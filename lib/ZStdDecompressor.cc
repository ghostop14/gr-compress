/*
 * ZStdDecompressor.cpp
 *
 *  Created on: Jun 6, 2019
 *      Author: mpiscopo
 */

#include "ZStdDecompressor.h"
#include <iostream>

namespace gr {
  namespace compress {

	ZStdDecompressor::ZStdDecompressor() {
		pCompressionContext = ZSTD_createDCtx();
	}

	size_t ZStdDecompressor::decompress(void* dst, size_t dstCapacity,
							const void* src, size_t srcSize) {
		/*! ZSTD_decompress() :
		 *  `compressedSize` : must be the _exact_ size of some number of compressed and/or skippable frames.
		 *  `dstCapacity` is an upper bound of originalSize to regenerate.
		 *  If user cannot imply a maximum upper bound, it's better to use streaming mode to decompress data.
		 *  @return : the number of bytes decompressed into `dst` (<= `dstCapacity`),
		 *            or an errorCode if it fails (which can be tested using ZSTD_isError()). */
		size_t retVal = ZSTD_decompressDCtx(pCompressionContext,dst,dstCapacity,src, srcSize);

		if (ZSTD_isError(retVal)) {
			std::cerr << "[ZStdDecompressor] An error was returned during decompression: " << ZSTD_getErrorName(retVal) << std::endl;
			return 0;
		}

		return retVal;
	}

	ZStdDecompressor::~ZStdDecompressor() {
		if (pCompressionContext) {
			ZSTD_freeDCtx(pCompressionContext);
			pCompressionContext = NULL;
		}
	}

} // namespace compress
} // namespace gr
