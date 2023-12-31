#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2018 Daniel Estevez <daniel@destevez.net>
#
# This file is part of gr-satellites
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import pmt
import numpy as np
try:
    from satellites import correct_ambiguity
except ImportError:
    import os
    import sys
    dirname, filename = os.path.split(os.path.abspath(__file__))
    sys.path.append(os.path.join(dirname, "bindings"))
    from satellites import correct_ambiguity

class qa_correct_ambiguity(gr_unittest.TestCase):
    def setUp(self):
        self.tag_key = 'frame_correlation'
        self.data = np.random.rand(16384)
        self.tags = [
            gr.python_to_tag((   42, pmt.intern(self.tag_key), pmt.from_float( 0.9), pmt.intern('test_src'))),
            gr.python_to_tag((   42, pmt.intern('something_else'), pmt.intern('ab'), pmt.intern('test_src'))),
            gr.python_to_tag((  500, pmt.intern(self.tag_key), pmt.from_float(-0.7), pmt.intern('test_src'))),
            gr.python_to_tag(( 2000, pmt.intern(self.tag_key), pmt.from_float(-0.8), pmt.intern('test_src'))),
            gr.python_to_tag(( 4000, pmt.intern(self.tag_key), pmt.from_float( 0.7), pmt.intern('test_src'))),
            gr.python_to_tag(( 4001, pmt.intern(self.tag_key), pmt.from_float(-0.7), pmt.intern('test_src'))),
            gr.python_to_tag(( 4002, pmt.intern(self.tag_key), pmt.from_float( 0.7), pmt.intern('test_src'))),
            gr.python_to_tag(( 4003, pmt.intern(self.tag_key), pmt.from_float( 0.7), pmt.intern('test_src'))),
            gr.python_to_tag(( 7500, pmt.intern('something_else'), pmt.intern('de'), pmt.intern('test_src'))),
            gr.python_to_tag((14000, pmt.intern(self.tag_key), pmt.from_float(-0.9), pmt.intern('test_src'))),
        ]
        self.expected_data = np.concatenate([
            self.data[0:500],
            -self.data[500:4000],
            self.data[4000:4001],
            -self.data[4001:4002],
            self.data[4002:14000],
            -self.data[14000:],
        ])
        
        self.source = blocks.vector_source_f(self.data, False, 1, self.tags)
        self.correction = correct_ambiguity(self.tag_key)
        self.sink = blocks.vector_sink_f()
        self.tag_debug = blocks.tag_debug(gr.sizeof_float, 'tag debug')
        self.tag_debug.set_display(False)
        self.tag_debug.set_save_all(True)
        
        self.tb = gr.top_block()
        self.tb.connect((self.source, 0), (self.correction, 0))
        self.tb.connect((self.correction, 0), (self.sink, 0))
        self.tb.connect((self.correction, 0), (self.tag_debug, 0))

    def tearDown(self):
        self.tb = None

    def test_correction(self):
        self.tb.start()
        self.tb.wait()

        received_tags = self.tag_debug.current_tags()

        np.testing.assert_almost_equal(self.sink.data(), self.expected_data)

        self.assertEqual(len(self.tags), len(received_tags))
        for tag_recv, tag_expected in zip(received_tags, self.tags):
            self.assertEqual(tag_recv.offset, tag_expected.offset)
            self.assertEqual(tag_recv.key, tag_expected.key)
            self.assertAlmostEqual(tag_recv.value, tag_expected.value)


if __name__ == '__main__':
    gr_unittest.run(qa_correct_ambiguity)
