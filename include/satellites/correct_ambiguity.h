/* -*- c++ -*- */
/*
 * Copyright 2021 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_SATELLITES_CORRECT_AMBIGUITY_H
#define INCLUDED_SATELLITES_CORRECT_AMBIGUITY_H

#include <satellites/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace satellites {

    /*!
     * \brief Corrects BPSK ambiguity based on correlation tags.
     *
     * Copies the input softbits to the output stream. If a negative correlation is detected the
     * sign of the softbits will be inverted (i.e. multiplied with -1) while copying them to the
     * output.
     * 
     * Positive or negative correlation is detected by looking for stream tags with the provided
     * key and a float correlation value. If the sign of the float value is positive the softbits
     * are left unchanged, while if the correlation is negative, the softbit signs will be inverted.
     * 
     * Each output sample is treated according to the last received correlation tag (initially a
     * positive correlation is assumed), regardless of how "old" the last correlation tag is.
     * 
     * \ingroup satellites
     *
     */
    class SATELLITES_API correct_ambiguity : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<correct_ambiguity> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of satellites::correct_ambiguity.
       *
       * To avoid accidental use of raw pointers, satellites::correct_ambiguity's
       * constructor is in a private implementation
       * class. satellites::correct_ambiguity::make is the public interface for
       * creating new instances.
       * 
       * \param correlation_tag Look for correlation tags with this key.
       */
      static sptr make(std::string correlation_tag);
    };

  } // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_CORRECT_AMBIGUITY_H */
