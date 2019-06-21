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
#include "StreamToDecompressionMsg_impl.h"

namespace gr {
  namespace compress {

    StreamToDecompressionMsg::sptr
    StreamToDecompressionMsg::make()
    {
      return gnuradio::get_initial_sptr
        (new StreamToDecompressionMsg_impl());
    }

    /*
     * The private constructor
     */
    StreamToDecompressionMsg_impl::StreamToDecompressionMsg_impl()
      : gr::sync_block("StreamToDecompressionMsg",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(0, 0, 0))
    {
    	headerValid = false;

    	localWriteBuffer = new unsigned char[64000];
    	localWriteBufferSize = 64000;

        message_port_register_out(pmt::mp("compressed"));
    }

    /*
     * Our virtual destructor.
     */
    StreamToDecompressionMsg_impl::~StreamToDecompressionMsg_impl()
    {
    	bool retVal = stop();
    }

    bool StreamToDecompressionMsg_impl::stop() {
    	if (localWriteBuffer) {
    		delete[] localWriteBuffer;
    		localWriteBuffer = NULL;
    		localWriteBufferSize = 0;
    	}

    	return true;
    }

    int
    StreamToDecompressionMsg_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
    	const char *in = (const char *) input_items[0];

    	gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function

    	// Queue the data
    	for (long i=0;i<noutput_items;i++) {
    		byteQueue.push(in[i]);
    	}

    	bool moreData = true;

    	while(moreData) {
        	// Look at the header if we have enough data for a complete message
        	// If not, just exit
        	if (!headerValid && (byteQueue.size() < sizeof(CompressionHeader)))
        		return noutput_items;  // we did consume the data.

        	// If we need the header and we do have enough data based on the header load it into a buffer for decryption
        	if (!headerValid) {
        		// We first need to pull the header off
        		unsigned char *pHeader;
        		pHeader = (unsigned char *)&currentHeader;
        		for (long i=0;i<sizeof(CompressionHeader);i++) {
        			pHeader[i] = byteQueue.front();
        			byteQueue.pop();
        		}
        		/*
            	std::cout << std::endl << "Header received: " << std::endl;
            	std::cout << "Compressed size:" << currentHeader.compressed_size << std::endl;
            	std::cout << "Uncompressed size:" << currentHeader.uncompressed_size << std::endl;
				*/

        		headerValid = true;
        	}

        	if (currentHeader.compressed_size > byteQueue.size()) {
        		return noutput_items; // we don't have enough yet
        	}

        	// We have enough, move queue data to a local buffer
        	if (currentHeader.compressed_size > localWriteBufferSize) {
        		delete[] localWriteBuffer;
        		localWriteBuffer = new unsigned char[currentHeader.compressed_size];
        		localWriteBufferSize = currentHeader.compressed_size;
        	}

        	for (long i=0;i<currentHeader.compressed_size;i++) {
        		localWriteBuffer[i] = byteQueue.front();
        		byteQueue.pop();
        	}

        	// Mark this header used
        	headerValid = false;

        	// send compressed message downstream
        	pmt::pmt_t meta = pmt::make_dict();
        	meta = pmt::dict_add(meta, pmt::mp("uncompressedsize"), pmt::from_long((long)currentHeader.uncompressed_size));
        	meta = pmt::dict_add(meta, pmt::mp("compressedsize"), pmt::from_long((long)currentHeader.compressed_size));
        	meta = pmt::dict_add(meta, pmt::mp("hasheader"), pmt::from_bool(false));

        	pmt::pmt_t data_out(pmt::make_blob((const void *)localWriteBuffer,(long)currentHeader.compressed_size));
        	pmt::pmt_t datapdu = pmt::cons( meta, data_out );

        	message_port_pub(pmt::mp("compressed"),datapdu);

        	// We'll keep processing our queue as long as we have enough for a message
        	// if we haven't returned already.

        	if (byteQueue.size() < sizeof(CompressionHeader))
        		moreData = false;  // this will cause the loop to kick out.
    	}

    	// Tell runtime system how many output items we produced.
    	return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

