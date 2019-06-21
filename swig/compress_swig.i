/* -*- c++ -*- */

#define COMPRESS_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "compress_swig_doc.i"

%{
#include "compress/compress.h"
#include "compress/decompress.h"
#include "compress/CompressedFileSink.h"
#include "compress/CompressedFileSource.h"
#include "compress/CompressedMsgToStream.h"
#include "compress/StreamToDecompressionMsg.h"
%}


%include "compress/compress.h"
GR_SWIG_BLOCK_MAGIC2(compress, compress);
%include "compress/decompress.h"
GR_SWIG_BLOCK_MAGIC2(compress, decompress);
%include "compress/CompressedFileSink.h"
GR_SWIG_BLOCK_MAGIC2(compress, CompressedFileSink);
%include "compress/CompressedFileSource.h"
GR_SWIG_BLOCK_MAGIC2(compress, CompressedFileSource);
%include "compress/CompressedMsgToStream.h"
GR_SWIG_BLOCK_MAGIC2(compress, CompressedMsgToStream);
%include "compress/StreamToDecompressionMsg.h"
GR_SWIG_BLOCK_MAGIC2(compress, StreamToDecompressionMsg);
