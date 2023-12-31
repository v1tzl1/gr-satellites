/* -*- c++ -*- */
/*
 * Copyright 2022 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_SATELLITES_CORRECT_AMBIGUITY_IMPL_H
#define INCLUDED_SATELLITES_CORRECT_AMBIGUITY_IMPL_H

#include <satellites/correct_ambiguity.h>

namespace gr {
namespace satellites {

class correct_ambiguity_impl : public correct_ambiguity
{
private:
    const pmt::pmt_t d_correlation_key;
    float d_factor;

    std::deque<uint64_t> get_flip_positions(size_t window_size);

    public:
    correct_ambiguity_impl(std::string correlation_tag);
    ~correct_ambiguity_impl();

    // Where all the action really happens
    int work(
            int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items
    );
};

} // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_CORRECT_AMBIGUITY_IMPL_H */
