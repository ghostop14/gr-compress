#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Test Compressed File Write2
# Generated: Mon Jun 17 10:44:12 2019
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import compress
import filerepeater
import sys
from gnuradio import qtgui


class test_compressed_file_write2(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Test Compressed File Write2")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Test Compressed File Write2")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "test_compressed_file_write2")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())


        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 600e3

        ##################################################
        # Blocks
        ##################################################
        self.filerepeater_file_repeater_ex_0 = filerepeater.file_repeater_ex(gr.sizeof_char*1, '/opt/tmp/Recordings/ATA/M3OH_y_small_104.8576MSPS.bc', 0, 0.0,False,0,0,False)
        self.compress_compress_0 = compress.compress(gr.sizeof_char,65536,4,False)
        self.compress_CompressedFileSink_0 = compress.CompressedFileSink(gr.sizeof_char, '/opt/tmp/Recordings/ATA/M3OH_y_small_104.8576MSPS.bc.gr.zstd', False, False)



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.compress_compress_0, 'compressed'), (self.compress_CompressedFileSink_0, 'compressedin'))
        self.connect((self.filerepeater_file_repeater_ex_0, 0), (self.compress_compress_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "test_compressed_file_write2")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate


def main(top_block_cls=test_compressed_file_write2, options=None):

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
