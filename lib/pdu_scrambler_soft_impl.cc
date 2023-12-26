/* -*- c++ -*- */
/*
 * Copyright 2022 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of gr-satellites
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pdu_scrambler_soft_impl.h"
#include <gnuradio/io_signature.h>

namespace {

std::vector<float> convert_to_softbits(const std::vector<uint8_t> sequence_hard) {
    std::vector<float> sequence_soft(sequence_hard.size()*8lu);
    
    for (size_t i=0; i<sequence_hard.size(); i++) {
        for (size_t j=0lu; j<8lu; j++) {
            const uint8_t sequence_bit = (sequence_hard[i] >> (7lu-j)) & 0x01;
            sequence_soft.at(i*8lu+j) = sequence_bit ? -1.0f : 1.0f;
        }
    }
    return sequence_soft;
}

}  /* namespace */

namespace gr {
namespace satellites {

pdu_scrambler_soft::sptr pdu_scrambler_soft::make(const std::vector<uint8_t>& sequence)
{
    return gnuradio::make_block_sptr<pdu_scrambler_soft_impl>(sequence);
}

pdu_scrambler_soft_impl::pdu_scrambler_soft_impl(const std::vector<uint8_t>& sequence)
    : gr::block("pdu_scrambler_soft",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
                d_sequence(convert_to_softbits(sequence))
{
    message_port_register_out(pmt::mp("out"));
    message_port_register_in(pmt::mp("in"));
    set_msg_handler(pmt::mp("in"), [this](pmt::pmt_t msg) { this->msg_handler(msg); });
    
    printf("Input sequence (%lu bytes):\n", sequence.size());
    for (size_t i=0lu; i<sequence.size(); i++) {
        printf("  %3lu: 0x%02X\n", i, sequence[i]);
    }
    
    printf("Converted sequence (%lu bits):\n", d_sequence.size());
    for(size_t i=0lu; i<d_sequence.size(); i++) {
        printf("%3lu: %f\n", i, d_sequence[i]);
    }
}

pdu_scrambler_soft_impl::~pdu_scrambler_soft_impl() {}

void pdu_scrambler_soft_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required)
{
}

int pdu_scrambler_soft_impl::general_work(int noutput_items,
                                     gr_vector_int& ninput_items,
                                     gr_vector_const_void_star& input_items,
                                     gr_vector_void_star& output_items)
{
    return 0;
}

void pdu_scrambler_soft_impl::msg_handler(pmt::pmt_t pmt_msg)
{
    std::vector<float> msg = pmt::f32vector_elements(pmt::cdr(pmt_msg));

    if (msg.size() > d_sequence.size()) {
        d_logger->error("PDU longer than scrambler sequence; dropping");
    }

    for (size_t j = 0; j < msg.size(); ++j) {
        msg[j] *= d_sequence[j];
    }

    message_port_pub(pmt::mp("out"),
                     pmt::cons(pmt::car(pmt_msg), pmt::init_f32vector(msg.size(), msg)));
}

} /* namespace satellites */
} /* namespace gr */
