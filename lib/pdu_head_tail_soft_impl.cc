/* -*- c++ -*- */
/*
 * Copyright 2020 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pdu_head_tail_soft_impl.h"
#include <gnuradio/io_signature.h>

#include <algorithm>
#include <vector>

namespace gr {
namespace satellites {

pdu_head_tail_soft::sptr pdu_head_tail_soft::make(int mode, size_t num)
{
    return gnuradio::make_block_sptr<pdu_head_tail_soft_impl>(mode, num);
}

/*
 * The private constructor
 */
pdu_head_tail_soft_impl::pdu_head_tail_soft_impl(int mode, size_t num)
    : gr::block("pdu_head_tail_soft",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      d_mode(mode),
      d_num(num)
{
    message_port_register_out(pmt::mp("out"));
    message_port_register_in(pmt::mp("in"));
    set_msg_handler(pmt::mp("in"), [this](pmt::pmt_t msg) { this->msg_handler(msg); });
}

/*
 * Our virtual destructor.
 */
pdu_head_tail_soft_impl::~pdu_head_tail_soft_impl() {}

void pdu_head_tail_soft_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
}

int pdu_head_tail_soft_impl::general_work(int noutput_items,
                                     gr_vector_int& ninput_items,
                                     gr_vector_const_void_star& input_items,
                                     gr_vector_void_star& output_items)
{
    return 0;
}

void pdu_head_tail_soft_impl::msg_handler(pmt::pmt_t pmt_msg)
{
    std::vector<float> msg = pmt::f32vector_elements(pmt::cdr(pmt_msg));
    std::vector<float> cut_msg;
    auto num = std::min(d_num, msg.size());

    switch (d_mode) {
    case PDU_HEADTAIL_SOFT_HEAD:
        cut_msg = std::vector<float>(msg.begin(), msg.begin() + num);
        break;
    case PDU_HEADTAIL_SOFT_HEADMINUS:
        cut_msg = std::vector<float>(msg.begin(), msg.end() - num);
        break;
    case PDU_HEADTAIL_SOFT_TAIL:
        cut_msg = std::vector<float>(msg.end() - num, msg.end());
        break;
    case PDU_HEADTAIL_SOFT_TAILPLUS:
        cut_msg = std::vector<float>(msg.begin() + num, msg.end());
        break;
    default:
        throw "Invalid pdu_head_tail_soft mode";
        break;
    }

    message_port_pub(
        pmt::mp("out"),
        pmt::cons(pmt::car(pmt_msg), pmt::init_f32vector(cut_msg.size(), cut_msg)));
}

} /* namespace satellites */
} /* namespace gr */
