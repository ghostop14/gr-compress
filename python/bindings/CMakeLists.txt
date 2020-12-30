# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT compress_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT compress_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND compress_python_files
    ComplexToPacked12_python.cc
    compress_python.cc
    CompressedFileSink_python.cc
    CompressedFileSource_python.cc
    CompressedMsgToStream_python.cc
    decompress_python.cc
    Packed12ToComplex_python.cc
    StreamToDecompressionMsg_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(compress 
   ../..
   gr::compress
   "${compress_python_files}")

install(TARGETS compress_python DESTINATION ${GR_PYTHON_DIR}/compress COMPONENT pythonapi)
