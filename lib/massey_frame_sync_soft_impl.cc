/* -*- c++ -*- */
/*
 * Copyright 2023 Daniel Estevez <daniel@destevez.net>.
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/io_signature.h>
#include "massey_frame_sync_soft_impl.h"

#include <volk/volk.h>

namespace {

uint8_t hexchar_to_uint(char c) {
    switch(c) {
        case '0': return 0u;
        case '1': return 1u;
        case '2': return 2u;
        case '3': return 3u;
        case '4': return 4u;
        case '5': return 5u;
        case '6': return 6u;
        case '7': return 7u;
        case '8': return 8u;
        case '9': return 9u;
        case 'A': return 10u;
        case 'B': return 11u;
        case 'C': return 12u;
        case 'D': return 13u;
        case 'E': return 14u;
        case 'F': return 15u;
        case 'a': return 10u;
        case 'b': return 11u;
        case 'c': return 12u;
        case 'd': return 13u;
        case 'e': return 14u;
        case 'f': return 15u;
        default: throw std::runtime_error("Syncword must be hexadecimal characters only");
    }
}

bool test_nibble(uint8_t byte, int pos) {
    assert(pos >= 0 && pos < 4);
    return ((byte >> (3-pos)) & 0x01) > 0u;
}

} /* namespace */

namespace gr {
  namespace satellites {

    using input_type = float;
    using output_type = float;

    massey_frame_sync_soft::sptr
    massey_frame_sync_soft::make(std::string syncword, size_t payload_len, float threshold)
    {
      return gnuradio::make_block_sptr<massey_frame_sync_soft_impl>(
        syncword, payload_len, threshold);
    }


    /*
     * The private constructor
     */
    massey_frame_sync_soft_impl::massey_frame_sync_soft_impl(std::string syncword, size_t payload_len, float threshold)
      : gr::sync_block("massey_frame_sync_soft",
              gr::io_signature::make(1, 1, sizeof(input_type)),
              gr::io_signature::make(1, 2, sizeof(output_type))),
        d_payload_len(payload_len),
        d_sync_len(syncword.size()*4lu),
        d_frame_len(d_payload_len+d_sync_len),
        d_threshold(threshold),
        d_tag_source(pmt::intern("massey_frame_sync_soft")),
        d_tag_key_score(pmt::intern("frame_score")),
        d_tag_key_correlation(pmt::intern("frame_correlation")),
        d_tag_key_snr(pmt::intern("frame_snr_db")),
        d_tag_key_offset(pmt::intern("frame_search_offset")),
        d_tag_key_length(pmt::intern("packet_len")),
        d_tag_length(pmt::from_uint64(d_frame_len)),
        d_correction_clamp_value(5.0),
        d_snr_est(d_frame_len)
    {
        const unsigned int alignment = volk_get_alignment();
        set_alignment(std::max(1, static_cast<int>(alignment / sizeof(output_type))));
        
        set_output_multiple(d_frame_len);

        d_sync_word = (float*) volk_malloc(d_sync_len*sizeof(float), alignment);
        d_score = (float*) volk_malloc(d_frame_len*sizeof(float), alignment);
        d_correlation = (float*) volk_malloc(d_frame_len*sizeof(float), alignment);
        d_correction = (float*) volk_malloc(d_frame_len*sizeof(float), alignment);
        d_buffer = (float*) volk_malloc((d_frame_len+d_sync_len)*sizeof(float), alignment);

        if (syncword.size() == 0lu) {
            throw std::runtime_error("syncword can not be empty");
        } else if (syncword.size() % 2lu != 0lu) {
            throw std::runtime_error("syncword must be a hexstring with an even number of characters");
        }

        // convert sync word from hex string to sequence of softbits (with magnitude 1)
        this->d_logger->trace("Reading in syncword {}:", syncword);
        for (size_t i=0lu; i<syncword.size(); i++) {
            const uint8_t byte = hexchar_to_uint(syncword[i]);
            this->d_logger->trace("0x{} = {:02d} = 0b{:04b}", syncword[i], byte, byte);
            for (int j=0; j<4; j++) {
                const size_t offset = 4u*i+j;
                d_sync_word[offset] = test_nibble(byte, j) ? -1.0f : 1.0f;
                this->d_logger->trace("{:2d}: {:d} => {:+.0f}", offset, test_nibble(byte, j), d_sync_word[offset]);
            }
        }
    }

    /*
     * Our virtual destructor.
     */
    massey_frame_sync_soft_impl::~massey_frame_sync_soft_impl()
    {
        volk_free(d_sync_word);
        volk_free(d_score);
        volk_free(d_buffer);
        volk_free(d_correction);
        volk_free(d_correlation);
    }

    int
    massey_frame_sync_soft_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        assert(noutput_items > 0);
        assert(static_cast<size_t>(noutput_items) % d_frame_len == 0lu);

        const size_t num_blocks = static_cast<size_t>(noutput_items) / d_frame_len;

        auto in = static_cast<const input_type*>(input_items[0]);
        auto out_signal = static_cast<output_type*>(output_items[0]);

        const bool optional_output = output_items.size() >= 1lu;
        
        uint32_t max_index;
        float frame_score;
        float frame_correlation;
        this->d_logger->trace("noutput_items={:d}, optional_output={}, num_blocks={:d}", noutput_items, optional_output, num_blocks);

        for (size_t i=0lu; i<num_blocks; i++) {
            this->d_logger->trace("iteration {}/{}, copying data into buffer", i, num_blocks);
            std::memcpy(d_buffer, &in[i*d_frame_len], d_frame_len*sizeof(input_type));
            std::memcpy(&d_buffer[d_frame_len], &in[i*d_frame_len], d_sync_len*sizeof(input_type));  // repeat the first L symbols to simulate wrapping around

            const double snr_linear = d_snr_est.snr_linear(d_buffer);
            const double snr_db = 10.0f*log10(snr_linear);
            this->d_logger->debug("SNR estimated to {:f}={:e}={:.3f} dB", snr_linear, snr_linear, snr_db);

            bool clamped = false;

            for (size_t j=0lu; j<d_frame_len; j++) {
                volk_32f_x2_dot_prod_32f(&d_correlation[j], &d_buffer[j], d_sync_word, d_sync_len);
                d_correction[j] = (snr_linear != 0.0f) ? 1.0f/snr_linear*log(cosh(d_buffer[j]*snr_linear)) : 0.0f;
                
                if (!std::isfinite(d_correction[j])) {
                    clamped = true;
                    d_correction[j] = d_correction_clamp_value;
                }
            }
            // normalize correlation
            volk_32f_s32f_multiply_32f(d_correlation, d_correlation, 1.0f/static_cast<float>(d_sync_len), d_frame_len);

            if (clamped) {
                d_logger->debug("At least one correction value was non-finite (SNR too high?). Clamping corrections to {}", d_correction_clamp_value);
            }


            for (size_t j=0lu; j<d_frame_len; j++) {
                float correction_sum;
                volk_32f_accumulator_s32f(&correction_sum, d_correction, d_sync_len);
                correction_sum = correction_sum / static_cast<float>(d_sync_len);

                d_score[j] = abs(d_correlation[j]) - correction_sum;
                this->d_logger->trace("symbol {:2d}/{}: correlation={}, correction={}, score={}", j, d_frame_len, d_correlation[j], correction_sum, d_score[j]);
            }

            // copy output
            this->d_logger->trace("Copying input data to output(s)");
            std::memcpy(&out_signal[i*d_frame_len], d_buffer, d_frame_len*sizeof(input_type));
            if (optional_output) {
                auto out_score = static_cast<output_type*>(output_items[1]);
                memcpy(&out_score[i*d_frame_len], d_score, d_frame_len*sizeof(input_type));
            }

            // Find max. frame
            volk_32f_index_max_32u(&max_index, d_score, d_frame_len);
            frame_score = d_score[max_index];
            frame_correlation = d_correlation[max_index];

            if (d_threshold > 0.0 && abs(frame_correlation) < d_threshold) {
                // Threshold value is not reached, ignore frame
                this->d_logger->debug("Potential frame at offset {} has a score of {} and a correlation of {}/{}, which is below the threshold, ignoring frame", max_index, frame_score, frame_correlation, d_threshold);
                continue;
            }

            // create output tag(s)
            const uint64_t offset = nitems_read(0) + i*d_frame_len + max_index;
            this->d_logger->debug("Found frame at at offset {} with score {}, creating tags at {}", max_index, frame_score, offset);
            add_item_tag(0, offset, d_tag_key_score, pmt::from_float(frame_score), d_tag_source);
            add_item_tag(0, offset, d_tag_key_correlation, pmt::from_float(frame_correlation), d_tag_source);
            add_item_tag(0, offset, d_tag_key_snr, pmt::from_float(snr_db), d_tag_source);
            add_item_tag(0, offset, d_tag_key_offset, pmt::from_uint64(max_index), d_tag_source);
            add_item_tag(0, offset, d_tag_key_length, d_tag_length, d_tag_source);
            
            if (optional_output) {
                add_item_tag(1, offset, d_tag_key_score, pmt::from_float(frame_score), d_tag_source);
                add_item_tag(1, offset, d_tag_key_correlation, pmt::from_float(frame_correlation), d_tag_source);
                add_item_tag(1, offset, d_tag_key_snr, pmt::from_float(snr_db), d_tag_source);
                add_item_tag(1, offset, d_tag_key_offset, pmt::from_uint64(max_index), d_tag_source);
                add_item_tag(1, offset, d_tag_key_length, d_tag_length, d_tag_source);
            }
        }
        
        // Tell runtime system how many output items we produced.
        return static_cast<int>(num_blocks*d_frame_len);
    }

  } /* namespace satellites */
} /* namespace gr */
