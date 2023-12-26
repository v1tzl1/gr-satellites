/* -*- c++ -*- */
/*
 * Copyright 2020 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_SATELLITES_PDU_HEAD_TAIL_SOFT_IMPL_H
#define INCLUDED_SATELLITES_PDU_HEAD_TAIL_SOFT_IMPL_H

#include <satellites/pdu_head_tail_soft.h>

#define PDU_HEADTAIL_SOFT_HEAD 0
#define PDU_HEADTAIL_SOFT_HEADMINUS 1
#define PDU_HEADTAIL_SOFT_TAIL 2
#define PDU_HEADTAIL_SOFT_TAILPLUS 3

namespace gr {
namespace satellites {

class pdu_head_tail_soft_impl : public pdu_head_tail_soft
{
private:
    int d_mode;
    size_t d_num;

public:
    pdu_head_tail_soft_impl(int mode, size_t num);
    ~pdu_head_tail_soft_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);

    void msg_handler(pmt::pmt_t pmt_msg);
};

} // namespace satellites
} // namespace gr

#endif /* INCLUDED_SATELLITES_PDU_HEAD_TAIL_SOFT_IMPL_H */
