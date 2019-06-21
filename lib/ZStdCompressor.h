/*
 * ZStdCompressor.h
 *
 *  Created on: Jun 6, 2019
 *      Author: mpiscopo
 */

#ifndef ZSTDCOMPRESSOR_H_
#define ZSTDCOMPRESSOR_H_

#include <compress/api.h>


#include <stdint.h>  // uint64_t definition
#include <zstd.h>

// #include "zstd/compress/zstdmt_compress.h" // ZSTDMT_ definitions
// #define USE_MT

namespace gr {
  namespace compress {

	class COMPRESS_API CompressionHeader {
	public:
		uint32_t uncompressed_size;
		uint32_t compressed_size;

		CompressionHeader() {
			uncompressed_size = compressed_size = 0;
		}
	};

	class COMPRESS_API ZStdCompressor {
	public:
		ZStdCompressor(int numThreads=1, int compressionLevel=3);
		virtual ~ZStdCompressor();

		size_t compress(void* dst, size_t dstCapacity,
						const void* src, size_t srcSize);

		size_t maxDestBufferSize(size_t inputSize);

		inline long getBlockSize() { return 64000; };

	protected:
#ifdef USE_MT
		ZSTDMT_CCtx *pCompressionContext;
#else
		ZSTD_CCtx *pCompressionContext;
#endif
		int d_numThreads;
		int d_compressionLevel;

	};

  } /* namespace compress */
} /* namespace gr */
#endif /* ZSTDCOMPRESSOR_H_ */
