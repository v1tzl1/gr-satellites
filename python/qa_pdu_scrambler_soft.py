#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2022 Daniel Estevez <daniel@destevez.net>
#
# This file is part of gr-satellites
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, blocks, gr_unittest
import numpy as np
import pmt

# bootstrap satellites module, even from build dir
try:
    import python as satellites
except ImportError:
    pass
else:
    import sys
    sys.modules['satellites'] = satellites

from satellites import pdu_scrambler_soft


class qa_pdu_scrambler_soft(gr_unittest.TestCase):
    def _test(self, data_in, data_out_expected, sequence):
        self.tb = gr.top_block()
        self.dbg = blocks.message_debug()
        pdu = pmt.cons(pmt.PMT_NIL, pmt.init_f32vector(len(data_in), data_in))
        self.scrambler = pdu_scrambler_soft(sequence)
        self.tb.msg_connect((self.scrambler, 'out'), (self.dbg, 'store'))
        self.scrambler.to_basic_block()._post(pmt.intern('in'), pdu)
        self.scrambler.to_basic_block()._post(
            pmt.intern('system'),
            pmt.cons(pmt.intern('done'), pmt.from_long(1)))

        self.tb.run()

        data_out= pmt.f32vector_elements(pmt.cdr(self.dbg.get_message(0)))
        np.testing.assert_allclose(data_out, data_out_expected)

        self.tb = None
    
    def test_pdu_scrambler_soft_001(self):
        data = np.random.rand(256)
        self._test(
            data_in=data,
            data_out_expected=data,
            sequence=[0x00,]*32
        )
    
    def test_pdu_scrambler_soft_002(self):
        data = np.random.rand(256)
        self._test(
            data_in=data,
            data_out_expected=-data,
            sequence=[0xFF,]*32
        )
    
    def test_pdu_scrambler_soft_003(self):
        self._test(
            data_in=np.asarray([
                 1.0,  0.9,  0.8,  0.7,  0.6,  0.5,  0.4,  0.3,
                -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3
            ]),
            data_out_expected=np.asarray([
                 1.0,  0.9,  0.8,  0.7,  0.6,  0.5,  0.4, -0.3,
                -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3
            ]),
            sequence=[0x01, 0x00]
        )
    
    def test_pdu_scrambler_soft_004(self):
        self._test(
            data_in=np.asarray([
                 1.0,  0.9,  0.8,  0.7,  0.6,  0.5,  0.4,  0.3,
                -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3
            ]),
            data_out_expected=np.asarray([
                -1.0, -0.9, -0.8, -0.7,  0.6,  0.5,  0.4,  0.3,
                -1.0, -0.9, -0.8, -0.7,  0.6,  0.5,  0.4,  0.3
            ]),
            sequence=[0xF0, 0x0F]
        )


if __name__ == '__main__':
    gr_unittest.run(qa_pdu_scrambler_soft)
