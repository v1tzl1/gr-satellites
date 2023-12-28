/* -*- c++ -*- */
/*
 * Copyright 2016,2020 Daniel Estevez <daniel@destevez.net>.
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/io_signature.h>
#include <satellites/bpsk_snr_est.h>
#include <volk/volk.h>

namespace gr {
  namespace satellites {

    bpsk_snr_est::bpsk_snr_est(size_t block_size)
    : d_block_size(block_size),
      d_logger("bpsk_snr_est")
    {
        d_buffer = (float*) volk_malloc(block_size*sizeof(float), volk_get_alignment());
    }

    bpsk_snr_est::~bpsk_snr_est()
    {
        volk_free(d_buffer);
    }

    double bpsk_snr_est::snr_linear(const float* input) {
        float m2_tmp;
        float m4_tmp;
        
        // Compute M2
        volk_32f_s32f_power_32f(d_buffer, input, 2.0, d_block_size);                         // square each entry
        volk_32f_accumulator_s32f(&m2_tmp, d_buffer, d_block_size);                          // sum entries together
        const double m2 = static_cast<double>(m2_tmp) / static_cast<double>(d_block_size);   // scale entries

        // Compute M4
        volk_32f_s32f_power_32f(d_buffer, d_buffer, 2.0, d_block_size);                     // square entries again (they are already squared once from before)
        volk_32f_accumulator_s32f(&m4_tmp, d_buffer, d_block_size);                         // sum entries together
        const double m4 = static_cast<double>(m4_tmp) / static_cast<double>(d_block_size);  // scale entries
        
        const double radicand = 6.0*m2*m2 - 2*m4;
        const double shat = (radicand > 0.0) ? 0.5*sqrt(radicand) : 0.0;
        const double nhat = m2 - shat;
        d_logger.debug("M2={}, M4={}, S={}, N={:e}, SNR={:e}", m2, m4, shat, nhat, shat/nhat);

        return shat/nhat;
    }

  } /* namespace satellites */
} /* namespace gr */
