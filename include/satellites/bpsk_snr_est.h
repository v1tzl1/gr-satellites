/* -*- c++ -*- */
/*
 * Copyright 2016 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_SATELLITES_BPSK_SNR_EST_H
#define INCLUDED_SATELLITES_BPSK_SNR_EST_H

#include <satellites/api.h>
#include <gnuradio/logger.h>

namespace gr {
namespace satellites {

//! \brief SNR Estimator using 2nd and 4th-order moments.
/*! \ingroup snr_blk
 *
 *  This block is equivalent to the gr::digital::mpsk_snr_est_m2m4 block
 *  but it takes floats as input instead of gr_complex.
 * 
 *  An SNR estimator for BPSK signals that uses 2nd (M2) and 4th
 *  (M4) order moments. This estimator uses knowledge of the
 *  kurtosis of the signal (\f$k_a)\f$ and noise (\f$k_w\f$) for
 *  BPSK to make its estimation. We use Beaulieu's approximations here to BPSK
 *  signals and AWGN channels such that \f$k_a=1\f$ and \f$k_w=3\f$. These
 *  approximations significantly reduce the complexity of the
 *  calculations (and computations) required.
 *
 *  Reference:
 *  D. R. Pauluzzi and N. C. Beaulieu, "A comparison of SNR
 *  estimation techniques for the AWGN channel," IEEE
 *  Trans. Communications, Vol. 48, No. 10, pp. 1681-1691, 2000.
 */
class SATELLITES_API bpsk_snr_est
{
public:
    bpsk_snr_est(size_t block_size);
    ~bpsk_snr_est();

    double snr_linear(const float* input);
private:
    const size_t d_block_size;
    gr::logger d_logger;
    float* d_buffer;
};

} // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_BPSK_SNR_EST_H */
