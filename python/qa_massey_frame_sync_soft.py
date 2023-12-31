#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2021-2022 Daniel Estevez <daniel@destevez.net>
#
# This file is part of gr-satellites
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import numpy as np
import pmt

try:
    from satellites import massey_frame_sync_soft
except ImportError:
    import os
    import sys
    dirname, filename = os.path.split(os.path.abspath(__file__))
    sys.path.append(os.path.join(dirname, "bindings"))
    from satellites import massey_frame_sync_soft


ccsds_asm_float = np.asarray([
    +1.0, +1.0, +1.0, -1.0,   -1.0, +1.0, -1.0, +1.0,  # 0x1A => +++- -+-+
    -1.0, -1.0, +1.0, +1.0,   -1.0, -1.0, -1.0, -1.0,  # 0xCF => --++ ----
    -1.0, -1.0, -1.0, -1.0,   -1.0, -1.0, +1.0, +1.0,  # 0xFC => ---- --++
    +1.0, +1.0, +1.0, -1.0,   -1.0, -1.0, +1.0, -1.0,  # 0x1D => +++- --+-
])
ccsds_asm_str = '1ACFFC1D'
ccsds_asm = (ccsds_asm_float, ccsds_asm_str)


class qa_massey_frame_sync_soft(gr_unittest.TestCase):
    def run_test(self, asm_float, asm_str, payload_len, num_payloads, num_prefix, num_suffix, snr_db, flip_data):
        sign = -1.0 if flip_data else 1.0
        data_in = sign * np.concatenate([
            np.sign(np.random.rand(num_prefix)),
            *[np.concatenate([asm_float, np.sign(np.random.rand(payload_len))]) for _ in range(num_payloads)],
            np.sign(np.random.rand(num_suffix)),
        ])
        noise_std = np.sqrt(10**(-snr_db/10))
        noise_in = noise_std*np.random.rand(len(data_in))

        tb = gr.top_block()
        frame_sync = massey_frame_sync_soft(asm_str, payload_len, -1.0)
        vector_source = blocks.vector_source_f(data_in+noise_in, False, 1, [])
        tag_debug_data = blocks.tag_debug(gr.sizeof_float*1, 'signal_stream')
        tag_debug_data.set_display(False)
        tag_debug_score = blocks.tag_debug(gr.sizeof_float*1, 'score_stream')
        tag_debug_score.set_display(False)
        
        tb.connect((vector_source, 0), (frame_sync, 0))
        tb.connect((frame_sync, 0), (tag_debug_data, 0))
        tb.connect((frame_sync, 1), (tag_debug_score, 0))

        frame_len = len(asm_float) + payload_len
        expected_asm_positions = [num_prefix+i*frame_len for i in range(num_payloads)]

        expected_correlation_sign = -1.0 if flip_data else 1.0

        tb.run()

        for dbg_block in (tag_debug_data, tag_debug_score):
            tags = dbg_block.current_tags()
            tags_correlation = [t for t in tags if pmt.to_python(t.key) == 'frame_correlation']
            tags_length = [t for t in tags if pmt.to_python(t.key) == 'packet_len']
            
            self.assertEqual(num_payloads, len(tags_length))
            self.assertEqual(num_payloads, len(tags_correlation))
            
            for i, (tag_corr, tag_len) in enumerate(zip(tags_correlation, tags_length)):
                self.assertEqual(tag_corr.offset, expected_asm_positions[i])
                self.assertEqual(tag_len.offset, expected_asm_positions[i])

                self.assertAlmostEqual(np.sign(pmt.to_python(tag_corr.value)), expected_correlation_sign)
                self.assertEqual(pmt.to_python(tag_len.value), frame_len)

    def test_001(self):
        self.run_test(*ccsds_asm, payload_len=223, num_payloads=7, num_prefix=42, num_suffix=15, snr_db=10, flip_data=False)

    def test_001(self):
        self.run_test(*ccsds_asm, payload_len=223, num_payloads=7, num_prefix=42, num_suffix=15, snr_db=10, flip_data=True)


if __name__ == '__main__':
    gr_unittest.run(qa_massey_frame_sync_soft)
