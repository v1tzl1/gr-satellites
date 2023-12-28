/* -*- c++ -*- */
/*
 * Copyright 2023 Daniel Estevez <daniel@destevez.net>.
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_H
#define INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_H

#include <satellites/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace satellites {

    /*!
     * \brief BPSK (softbit) frame synchronization according to James Masseys "Optimum Frame Synchronization" paper
     *
     * This block uses a M2M4 moment SNR estimator for the energy correction. See \ref bpsk_snr_est.
     * 
     * \ingroup satellites
     */
    class SATELLITES_API massey_frame_sync_soft : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<massey_frame_sync_soft> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of satellites::massey_frame_sync_soft.
       *
       * \param syncword String of hexadecimal representation of the sync marker, e.g. "1ACFFC1D" for the standard CCSDS ASM.
       * \param payload_len Number of payload bits
       * \param threshold Threshold for frame detection. If positive, potential frames with a score less than the threshold value are ignored. If the threshold is zero or negative, the block will tag the most likely frame location for every block.
       * \param strip_asm If false the generated tags will be attatched to the first symbol of the detected ASM and have a length of ASM length + payload_length. If true the tag will be attached to the first payload bit and have a length of payload_length.
       * \param tag_name Name for the tag that should be attached. The value of that tag will be a long unsigned with the payload_length
       */
      static sptr make(std::string syncword, size_t payload_len, float threshold, bool strip_asm, std::string tag_name);
    };

  } // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_H */
