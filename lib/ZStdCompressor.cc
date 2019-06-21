/*
 * ZStdCompressor.cpp
 *
 *  Created on: Jun 6, 2019
 *      Author: mpiscopo
 */

#include "ZStdCompressor.h"
#include <iostream>

namespace gr {
  namespace compress {

ZStdCompressor::ZStdCompressor(int numThreads, int compressionLevel) {
	size_t errcode;

#ifdef USE_MT
	pCompressionContext = ZSTDMT_createCCtx(numThreads);
#else
	pCompressionContext = ZSTD_createCCtx();
	// Set number of threads
	errcode = ZSTD_CCtx_setParameter(pCompressionContext, ZSTD_c_nbWorkers,numThreads);

	if (ZSTD_isError(errcode)) {
		std::cerr << "[ZStdCompressor] Error setting threads.  Error: " << ZSTD_getErrorName(errcode) << std::endl;
	}
#endif

	if (pCompressionContext==NULL) {
		std::cerr<<"[ZStdCompressor] Error creating compression context." << std::endl;
		exit(1);
	}

	// set compression level

	errcode = ZSTD_CCtx_setParameter(pCompressionContext, ZSTD_c_compressionLevel,compressionLevel);

	if (ZSTD_isError(errcode)) {
		std::cerr << "[ZStdCompressor] Error setting threads.  Error: " << ZSTD_getErrorName(errcode) << std::endl;
	}

	d_numThreads = numThreads;
	d_compressionLevel = compressionLevel;
}

size_t ZStdCompressor::maxDestBufferSize(size_t inputSize) {
	return ZSTD_compressBound(inputSize);
}

size_t ZStdCompressor::compress(void* dst, size_t dstCapacity,
        				const void* src, size_t srcSize) {

	/*! ZSTD_compress() :
	 *  Compresses `src` content as a single zstd compressed frame into already allocated `dst`.
	 *  Hint : compression runs faster if `dstCapacity` >=  `ZSTD_compressBound(srcSize)`.
	 *  @return : compressed size written into `dst` (<= `dstCapacity),
	 *            or an error code if it fails (which can be tested using ZSTD_isError()). */
#ifdef USE_MT
	return ZSTDMT_compressCCtx(pCompressionContext,dst,dstCapacity,src, srcSize,d_compressionLevel);
#else
	// return ZSTD_compressCCtx(pCompressionContext,dst,dstCapacity,src, srcSize,d_compressionLevel);
	return ZSTD_compress2(pCompressionContext,dst,dstCapacity,src, srcSize);
#endif
}

ZStdCompressor::~ZStdCompressor() {
	if (pCompressionContext) {
#ifdef USE_MT
		ZSTDMT_freeCCtx(pCompressionContext);
#else
		ZSTD_freeCCtx(pCompressionContext);
#endif
		pCompressionContext = NULL;
	}
}

  } /* namespace compress */
} /* namespace gr */
