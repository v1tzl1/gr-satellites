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
       * \brief Perform frame syncrhonization according to James Masseys paper "Optimum Frame Syncrhonization"
       *
       * This block operates on blocks of \ref packet_len bits (length of sync word and payload length).
       * For each input symbol in the block, the normalized correlation with the sync word and the normalized
       * score (correlation minus correction term) is computed.
       * 
       * 
       * After score computation the position within the search block with the maximum score is searched. Depending
       * on the \ref threshold parameter there is a detected frame for every block, or only if the score exceeds a
       * theshold.
       * 
       * For each detected frame a couple of stream tags are attached to the output streams:
       *  - frame_correlation (float): Normalized correlation coefficient
       *  - frame_score (float): Normalized correlation minus the normalized correction term
       *  - frame_snr_db (float): Average estimated signal to noise ratio in dB over the search range.
       *    The search range has the same length than \ref packet_len, but is only aligned with
       *    the frame (and thus reflects the average frame SNR), if the frame is detected with offset
       *    0 in the search range.
       *  - frame_search_offset (uint64): Offset beteen search range and start of frame.
       *  - packet_len (uint64): Packet length in bits (sync word length + payload length)
       *
       * The input data is copied to the first output stream. If the optional second output stream is connected it will
       * contain the computed score for each input symbol.
       * 
       * \param syncword String of hexadecimal representation of the sync marker, e.g. "1ACFFC1D" for the standard
       *                 CCSDS ASM.
       * \param payload_len Number of payload bits
       * \param threshold Threshold for frame detection. If positive, potential frames with an absolute normalized 
       *                  **correlation** (not score) value of less than the threshold value are ignored. If the
       *                  threshold is zero or negative, the block will always tag the most likely frame location
       *                  within the search range.
       */
      static sptr make(std::string syncword, size_t payload_len, float threshold);
    };

  } // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_MASSEY_FRAME_SYNC_SOFT_H */
