/* -*- c++ -*- */
/*
 * Copyright 2022 Daniel Estevez <daniel@destevez.net>.
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/io_signature.h>
#include "correct_ambiguity_impl.h"
#include <volk/volk.h>

namespace {

inline bool same_sign(float a, float b) {
    return ((a >= 0.0f) && (b >= 0.0f)) | ((a < 0.0f) && (b < 0.0f));
}

}  /* namespace */

namespace gr {
namespace satellites {
using input_type = float;
using output_type = float;

correct_ambiguity::sptr
correct_ambiguity::make(std::string correlation_tag)
{
    return gnuradio::make_block_sptr<correct_ambiguity_impl>(correlation_tag);
}


/*
    * The private constructor
    */
correct_ambiguity_impl::correct_ambiguity_impl(std::string correlation_tag)
    : gr::sync_block("correct_ambiguity",
            gr::io_signature::make(1, 1, sizeof(input_type)),
            gr::io_signature::make(1, 1, sizeof(output_type))),
    d_correlation_key(pmt::intern(correlation_tag)),
    d_factor(1.0f)
{
    set_tag_propagation_policy(gr::block::tag_propagation_policy_t::TPP_ALL_TO_ALL);
}

/*
    * Our virtual destructor.
    */
correct_ambiguity_impl::~correct_ambiguity_impl()
{
}

int
correct_ambiguity_impl::work(int noutput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    std::deque<uint64_t> pos_sign_changes = this->get_flip_positions(static_cast<uint64_t>(noutput_items));

    const size_t num_total = static_cast<size_t>(noutput_items);
    size_t num_processed = 0lu;

    d_logger->trace("work(noutput_items={})", noutput_items);

    while (num_processed < num_total) {
        d_logger->trace("work loop: {}/{} samples processed, current sign is {:+.0f}.", num_processed, num_total, d_factor);
        size_t chunk_size;

        if (pos_sign_changes.empty()) {
            chunk_size = num_total-num_processed;
        } else {
            // reduce chunk to only include data until the next bit flip
            assert(pos_sign_changes.front() > num_processed);
            chunk_size = pos_sign_changes.front() - num_processed;
        }

        d_logger->trace("Copying {} items to offset {}", chunk_size, num_processed);
        volk_32f_s32f_multiply_32f(&out[num_processed], &in[num_processed], d_factor, chunk_size);
        num_processed += chunk_size;

        if (num_processed == pos_sign_changes.front()) {
            d_factor = -d_factor;
            pos_sign_changes.pop_front();
            d_logger->trace("Flipping sign to {:+.0f}", d_factor);
        }
    }

    assert(num_processed == num_total);
    
    // Tell runtime system how many output items we produced.
    return noutput_items;
}

std::deque<uint64_t> correct_ambiguity_impl::get_flip_positions(size_t window_size) {
    std::vector<gr::tag_t> tags;
    std::deque<uint64_t> pos_sign_changes;
    float current_sign = d_factor;
    const uint64_t num_read = nitems_read(0u);
    
    get_tags_in_window(tags, 0u, 0, window_size, d_correlation_key);
    
    d_logger->trace("Processing tags in window, current sign is {:+.0f}.", current_sign);
    for (const auto& tag : tags) {
        const float correlation = pmt::to_float(tag.value);
        d_logger->debug("  New correlation tag at {} with correlation {}", tag.offset, correlation);
        if (!same_sign(current_sign, correlation)) {
            assert(tag.offset >= num_read);
            pos_sign_changes.push_back(tag.offset - num_read);
            current_sign = -current_sign;
            d_logger->trace("    Correlation sign changed to {:+.0f}, adding flip at {}", current_sign, tag.offset);
        }
    }

    return pos_sign_changes;
}

} /* namespace satellites */
} /* namespace gr */
