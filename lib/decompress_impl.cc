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
#include "decompress_impl.h"
#include "ZStdCompressor.h" // Need compression header definition

namespace gr {
  namespace compress {

    decompress::sptr
    decompress::make(int itemsize, bool sendMsg, bool verbose)
    {
      return gnuradio::get_initial_sptr
        (new decompress_impl(itemsize,sendMsg,verbose));
    }

    /*
     * The private constructor
     */
    decompress_impl::decompress_impl(int itemsize, bool sendMsg, bool verbose)
      : gr::sync_block("decompress",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, itemsize))
    {
    	d_verbose = verbose;
        verboseCounter = 0;

    	d_itemsize = itemsize;
    	d_sendMsg = sendMsg;

    	d_compressionBlockSize = 64000;

    	localWriteBuffer = new unsigned char[d_compressionBlockSize];
    	localWriteBufferSize = d_compressionBlockSize;
    	localWriteBufferNextIndex = 0;

		message_port_register_in(pmt::mp("compressedin"));
        set_msg_handler(pmt::mp("compressedin"), boost::bind(&decompress_impl::compressedIn, this, _1) );
        message_port_register_out(pmt::mp("uncompressed"));

    }

    void decompress_impl::compressedIn(pmt::pmt_t msg) {
        gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function
		pmt::pmt_t inputMetadata = pmt::car(msg);
		pmt::pmt_t data = pmt::cdr(msg);
		size_t noutput_items = pmt::length(data);
		const unsigned char *samples;

		samples = (const unsigned char *)pmt::blob_data(data);

		long compressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("compressedsize"), pmt::mp(0)));
		long uncompressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("uncompressedsize"), pmt::mp(0)));
		bool hasHeader = pmt::to_bool(pmt::dict_ref(inputMetadata, pmt::mp("hasheader"), pmt::mp(false)));

		const unsigned char *pBuff;
		if (hasHeader)
			pBuff = &samples[sizeof(CompressionHeader)];
		else
			pBuff = samples;

		if (uncompressedSize > localWriteBufferSize) {
			delete [] localWriteBuffer;
	    	localWriteBuffer = new unsigned char[uncompressedSize];
	    	localWriteBufferSize = uncompressedSize;
		}

		size_t actualDecompressedSize = decompressor.decompress(localWriteBuffer,uncompressedSize,pBuff,compressedSize);

		if (actualDecompressedSize > uncompressedSize) {
			std::cerr << "Decompression ERROR: Decompressor returned a size (" << actualDecompressedSize <<
					") > expected decompressed size (" << uncompressedSize << ").  Not compressed data?" << std::endl;
			return;
		}

		if (d_verbose) {
			if (verboseCounter==0) {
				float compressedRatio = ((float)compressedSize / (float)uncompressedSize * 100.0);
				std::cout << "[Decompress] Compressed ratio: " << compressedRatio << "%, " << (100.0-compressedRatio) << "% compression." << std::endl;
			}

			verboseCounter++;

			if (verboseCounter > 1000)
				verboseCounter = 0;
		}

		// Queue it for local work
		for (long i=0;i<actualDecompressedSize; i++) {
			byteQueue.push(localWriteBuffer[i]);
		}

		if (d_sendMsg) {
			// Send decompressed data message
			pmt::pmt_t meta = pmt::make_dict();
			meta = pmt::dict_add(meta, pmt::mp("uncompressedsize"), pmt::from_long((long)actualDecompressedSize));

			/*
			pmt::pmt_t data_out(pmt::make_u8vector((long)actualDecompressedSize, 0));
			for (long i=0;i<actualDecompressedSize;i++) {
				pmt::u8vector_set(data_out,i,localWriteBuffer[i]);
			}
			*/
			pmt::pmt_t data_out(pmt::make_blob((const void *)localWriteBuffer,(long)actualDecompressedSize));
			pmt::pmt_t datapdu = pmt::cons( meta, data_out );

			message_port_pub(pmt::mp("uncompressed"),datapdu);
		}
    }

    /*
     * Our virtual destructor.
     */
    decompress_impl::~decompress_impl()
    {
    	bool retVal = stop();
    }

    bool decompress_impl::stop() {
    	if (localWriteBuffer) {
    		delete[] localWriteBuffer;
    		localWriteBuffer = NULL;
    		localWriteBufferSize = 0;
    	}

    	return true;
    }

    int
	decompress_impl::test(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items, size_t uncompressedSize, size_t compressedSize)
    {
        gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function
		const unsigned char *samples;

		samples = (const unsigned char *)input_items[0];
		const unsigned char *pBuff;
		pBuff = samples;

		if (uncompressedSize > localWriteBufferSize) {
			delete [] localWriteBuffer;
	    	localWriteBuffer = new unsigned char[uncompressedSize];
	    	localWriteBufferSize = uncompressedSize;
		}

		size_t actualDecompressedSize = decompressor.decompress(localWriteBuffer,uncompressedSize,pBuff,compressedSize);

		// Queue it for local work
		//unsigned char achar;

		if (actualDecompressedSize == 0) {
			std::cerr << "Decompression ERROR: Decompressor returned a size (" << actualDecompressedSize <<
					") > expected decompressed size (" << uncompressedSize << ").  Not compressed data?" << std::endl;
		}
		else {
			for (long i=0;i<actualDecompressedSize; i++) {
				byteQueue.push(localWriteBuffer[i]);
			}
		}

		return uncompressedSize;
    }

    int
    decompress_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
    	gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function
    	unsigned char *out = (unsigned char *) output_items[0];

    	long itemsAvailable = byteQueue.size() / d_itemsize;
    	long itemsProduced;

    	if (itemsAvailable >= noutput_items)
    		itemsProduced = noutput_items;
    	else
    		itemsProduced = itemsAvailable;

    	long noi = itemsProduced * d_itemsize;

    	for (long i=0;i<noi;i++) {
    		out[i] = byteQueue.front();
    		byteQueue.pop();
    	}

    	return itemsProduced;
    }

  } /* namespace compress */
} /* namespace gr */

