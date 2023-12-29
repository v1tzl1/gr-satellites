#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: MOVE-II decoder (postprocessing)
# Author: Martin Lülf
# Description: Decode MOVE-II baseband recording (which needs to be dopplercorrected)
# GNU Radio version: 3.10.7.0

from gnuradio import blocks
import pmt
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import gr, pdu
from gnuradio import ldpc_toolbox
import satellites
import satellites.components.demodulators




class move2_postprocessing(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "MOVE-II decoder (postprocessing)", catch_exceptions=True)

        ##################################################
        # Blocks
        ##################################################

        self.satellites_pdu_scrambler_soft_0 = satellites.pdu_scrambler([255, 18, 112, 3, 89, 176, 14, 61, 113, 52, 201, 181, 229, 237, 98, 115, 90, 233, 190, 51, 76, 69, 253, 124, 80, 8, 143, 17, 41, 179, 87, 141, 127, 9, 184, 129, 44, 88, 135, 158, 56, 154, 228, 218, 242, 118, 177, 57, 173, 116, 223, 25, 166, 162, 126, 62, 40, 132, 199, 136, 148, 217, 171, 198, 191, 4, 220, 64, 22, 172, 67, 79, 28, 77, 114, 109, 121, 187, 216, 156, 86, 186, 239, 12, 83, 81, 63, 31, 20, 194, 99, 68, 202, 236, 85, 227, 95, 2, 110, 32, 11, 214, 161, 39, 142, 38, 185, 182, 188, 93, 108, 78, 43, 221, 119, 134, 169, 168, 159, 15, 10, 225, 49, 34, 101, 246, 170, 241, 47, 1, 55, 144, 5, 235, 208, 19, 71, 147, 92, 91, 222, 46, 54, 167, 149, 238, 59, 195, 84, 212, 207, 7, 133, 240, 24, 145, 50, 123, 213, 248, 151, 128, 27, 200, 130, 117, 232, 137, 163, 73, 174, 45, 111, 23, 155, 211, 74, 247, 157, 97, 42, 234, 231, 131, 66, 120, 140, 72, 153, 189, 106, 252, 75, 192, 13, 100, 193, 58, 244, 196, 209, 36, 215, 150, 183, 139, 205, 105, 165, 251, 206, 48, 21, 245, 243, 65, 33, 60, 70, 164, 204, 94, 53, 254, 37, 224, 6, 178, 96, 29, 122, 226, 104, 146, 107, 203, 219, 197, 230, 180, 210, 125, 103, 152, 138, 250, 249, 160, 16, 30, 35, 82, 102, 175, 26, 255])
        self.satellites_massey_frame_sync_soft_0 = satellites.massey_frame_sync_soft('49E0DCC7', 2048, -1.0, True, 'packet_len')
        self.satellites_bpsk_demodulator_0 = satellites.components.demodulators.bpsk_demodulator(baudrate = 12.5e3, samp_rate = 25e3, f_offset = 0, differential = False, manchester = False, iq = True, options="")
        self.pdu_tagged_stream_to_pdu_1 = pdu.tagged_stream_to_pdu(gr.types.float_t, 'packet_len')
        self.pdu_pdu_to_tagged_stream_0 = pdu.pdu_to_tagged_stream(gr.types.float_t, 'packet_len')
        self.ldpc_toolbox_ldpc_decoder_0 = ldpc_toolbox.ldpc_decoder('/home/mluelf/Downloads/AR4JA_r12_k1024n.a', 'Phif32', "", 2048, 1024, 200)
        self.blocks_vector_to_stream_0 = blocks.vector_to_stream(gr.sizeof_char*1, 1024)
        self.blocks_stream_to_vector_0 = blocks.stream_to_vector(gr.sizeof_float*1, 2048)
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_ff(1)
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, '', False, 0, 0)
        self.blocks_file_source_0.set_begin_tag(pmt.PMT_NIL)
        self.blocks_file_sink_1 = blocks.file_sink(gr.sizeof_float*1, '', False)
        self.blocks_file_sink_1.set_unbuffered(False)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, '', False)
        self.blocks_file_sink_0.set_unbuffered(False)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.pdu_tagged_stream_to_pdu_1, 'pdus'), (self.satellites_pdu_scrambler_soft_0, 'in'))
        self.msg_connect((self.satellites_pdu_scrambler_soft_0, 'out'), (self.pdu_pdu_to_tagged_stream_0, 'pdus'))
        self.connect((self.blocks_file_source_0, 0), (self.satellites_bpsk_demodulator_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.pdu_tagged_stream_to_pdu_1, 0))
        self.connect((self.blocks_stream_to_vector_0, 0), (self.ldpc_toolbox_ldpc_decoder_0, 0))
        self.connect((self.blocks_vector_to_stream_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.ldpc_toolbox_ldpc_decoder_0, 0), (self.blocks_vector_to_stream_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.blocks_stream_to_vector_0, 0))
        self.connect((self.satellites_bpsk_demodulator_0, 0), (self.satellites_massey_frame_sync_soft_0, 0))
        self.connect((self.satellites_massey_frame_sync_soft_0, 1), (self.blocks_file_sink_1, 0))
        self.connect((self.satellites_massey_frame_sync_soft_0, 0), (self.blocks_multiply_const_vxx_0, 0))





def main(top_block_cls=move2_postprocessing, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    tb.wait()


if __name__ == '__main__':
    main()
