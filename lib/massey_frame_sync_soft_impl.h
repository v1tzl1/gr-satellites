/* -*- c++ -*- */
/*
 * Copyright 2023 Daniel Estevez <daniel@destevez.net>.
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_IMPL_H
#define INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_IMPL_H

#include <satellites/massey_frame_sync_soft.h>
#include <satellites/bpsk_snr_est.h>

#include <pmt/pmt.h>
#include <vector>

namespace gr {
  namespace satellites {

    class massey_frame_sync_soft_impl : public massey_frame_sync_soft
    {
     private:
        const size_t d_payload_len;
        const size_t d_sync_len;
        const size_t d_frame_len;
        const float d_threshold;
        const pmt::pmt_t d_tag_source;
        const pmt::pmt_t d_tag_key_score;
        const pmt::pmt_t d_tag_key_correlation;
        const pmt::pmt_t d_tag_key_snr;
        const pmt::pmt_t d_tag_key_offset;
        const pmt::pmt_t d_tag_key_length;
        const pmt::pmt_t d_tag_length;
        const float d_correction_clamp_value;
        float* d_sync_word;
        bpsk_snr_est d_snr_est;
        float* d_score;
        float* d_correction;
        float* d_correlation;
        float* d_buffer;

     public:
      massey_frame_sync_soft_impl(std::string syncword, size_t payload_len, float threshold);
      ~massey_frame_sync_soft_impl();

      // Where all the action really happens
      int work(
              int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
      );
    };

  } // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_IMPL_H */
