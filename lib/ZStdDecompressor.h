/*
 * ZStdDecompressor.h
 *
 *  Created on: Jun 6, 2019
 *      Author: mpiscopo
 */

#ifndef ZSTDDECOMPRESSOR_H_
#define ZSTDDECOMPRESSOR_H_

#include <compress/api.h>

#include "zstd.h"

namespace gr {
  namespace compress {

	class COMPRESS_API ZStdDecompressor {
	public:
		ZStdDecompressor();
		virtual ~ZStdDecompressor();

		size_t decompress(void* dst, size_t dstCapacity,
						const void* src, size_t srcSize);

	protected:
		ZSTD_DCtx *pCompressionContext;

	};

  } // namespace compress
} // namespace gr
#endif /* ZSTDDECOMPRESSOR_H_ */
