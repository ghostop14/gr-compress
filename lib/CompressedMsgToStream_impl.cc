/* -*- c++ -*- */
/* 
 * Copyright 2019 ghostop14.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "CompressedMsgToStream_impl.h"
#include "ZStdCompressor.h" // Need compression header definition
#include <functional>
namespace gr {
  namespace compress {

    CompressedMsgToStream::sptr
    CompressedMsgToStream::make()
    {
      return gnuradio::get_initial_sptr
        (new CompressedMsgToStream_impl());
    }

    /*
     * The private constructor
     */
    CompressedMsgToStream_impl::CompressedMsgToStream_impl()
      : gr::sync_block("CompressedMsgToStream",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(char)))
    {

		message_port_register_in(pmt::mp("compressed"));
        set_msg_handler(pmt::mp("compressed"), std::bind(&CompressedMsgToStream_impl::compressedIn, this, std::placeholders::_1) );
    }

    /*
     * Our virtual destructor.
     */
    CompressedMsgToStream_impl::~CompressedMsgToStream_impl()
    {
    }

    void CompressedMsgToStream_impl::compressedIn(pmt::pmt_t msg) {
		pmt::pmt_t inputMetadata = pmt::car(msg);
		pmt::pmt_t data = pmt::cdr(msg);
		size_t noutput_items = pmt::length(data);
		const unsigned char *samples;

		samples = (const unsigned char *)pmt::blob_data(data);

		long compressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("compressedsize"), pmt::mp(0)));
		long uncompressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("uncompressedsize"), pmt::mp(0)));
		bool hasHeader = pmt::to_bool(pmt::dict_ref(inputMetadata, pmt::mp("hasheader"), pmt::mp(false)));

		// Need the header for transmission, so if it doesn't have one we'll need to build it.

		// Queue the data
		const unsigned char *pBuff;
		int dataLength;
		CompressionHeader cHeader;

		if (hasHeader) {
			pBuff = &samples[sizeof(CompressionHeader)];
			memcpy(&cHeader,&samples[0],sizeof(CompressionHeader));
			dataLength = noutput_items  - sizeof(CompressionHeader);  // This is how much data pBuff actually points to
		}
		else {
			pBuff = samples;
			cHeader.uncompressed_size = uncompressedSize;
			cHeader.compressed_size = compressedSize;
			dataLength = noutput_items; // No header so it was all data.
		}

		// Queue the header bytes
		unsigned char *pHeader;
		pHeader = (unsigned char *)&cHeader;

		// Lock our mutex to work with the queue
		gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function

        for (int i=0;i<sizeof(CompressionHeader);i++) {
			byteQueue.push(pHeader[i]);
		}

		// Queue the compressed data bytes.
		for (long i=0;i<dataLength; i++) {
			byteQueue.push(pBuff[i]);
		}
    }

    int
    CompressedMsgToStream_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      char *out = (char *) output_items[0];

      // Our data will be queued up in byteQueue.  At this point we just need to return it.
      long bytesToReturn = noutput_items;

      gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function
      if (byteQueue.size() < bytesToReturn)
    	  bytesToReturn = byteQueue.size();

      if (bytesToReturn == 0)
		return 0;

      for (long i=0;i<bytesToReturn;i++) {
    	  out[i] = byteQueue.front();
    	  byteQueue.pop();
      }
      // Tell runtime system how many output items we produced.
      return bytesToReturn;
    }

  } /* namespace compress */
} /* namespace gr */

