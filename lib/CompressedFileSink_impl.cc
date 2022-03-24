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
#include <stdexcept>
#include "ZStdCompressor.h"
#include <functional>
#include "CompressedFileSink_impl.h"

namespace gr {
  namespace compress {

    CompressedFileSink::sptr
    CompressedFileSink::make(size_t itemsize, const char *filename, bool unbuffered, bool append)
    {
      return gnuradio::get_initial_sptr
        (new CompressedFileSink_impl(itemsize, filename, unbuffered, append));
    }

    /*
     * The private constructor
     */
    CompressedFileSink_impl::CompressedFileSink_impl(size_t itemsize, const char *filename, bool unbuffered, bool append)
      : gr::sync_block("CompressedFileSink",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0, 0, 0)),
		        file_sink_base(filename, true, append),
		        d_itemsize(itemsize)

    {
    	d_unbuffered = unbuffered;
    	open(filename);
    	do_update();

		message_port_register_in(pmt::mp("compressedin"));
        set_msg_handler(pmt::mp("compressedin"), std::bind(&CompressedFileSink_impl::compressedIn, this, std::placeholders::_1) );
    }


    /*
     * Our virtual destructor.
     */
    CompressedFileSink_impl::~CompressedFileSink_impl()
    {
    	bool retVal = stop();
    }

    bool CompressedFileSink_impl::stop() {
    	close();
    	return true;
    }

    void CompressedFileSink_impl::compressedIn(pmt::pmt_t msg) {
        gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function

		pmt::pmt_t inputMetadata = pmt::car(msg);
		pmt::pmt_t data = pmt::cdr(msg);
		size_t noutput_items = pmt::length(data);
		const uint8_t *inbuf;

		// inbuf = pmt::u8vector_elements(data,noutput_items);
		inbuf = (const uint8_t *)pmt::blob_data(data);

		long compressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("compressedsize"), pmt::mp(0)));
		long uncompressedSize = pmt::to_long(pmt::dict_ref(inputMetadata, pmt::mp("uncompressedsize"), pmt::mp(0)));
		bool hasHeader = pmt::to_bool(pmt::dict_ref(inputMetadata, pmt::mp("hasheader"), pmt::mp(false)));

		const unsigned char *pBuff;
		if (hasHeader)
			pBuff = &inbuf[sizeof(CompressionHeader)];
		else
			pBuff = inbuf;

		// Now looks just like file sink
        long nwritten = 0;

        if(!d_fp) {
          return;         // drop output on the floor
        }

        if (!hasHeader) {
        	// Need to write the header for the block first.
        	CompressionHeader hdr;
        	hdr.uncompressed_size = uncompressedSize;
        	hdr.compressed_size = compressedSize;

        	fwrite((const void *)&hdr,sizeof(CompressionHeader),1,d_fp);
        }

        while(nwritten < noutput_items) {
          int count = fwrite(inbuf, d_itemsize, noutput_items - nwritten, d_fp);
          if(count == 0) {
            if(ferror(d_fp)) {
              std::stringstream s;
              s << "file_sink write failed with error " << fileno(d_fp) << std::endl;
              throw std::runtime_error(s.str());
            }
            else { // is EOF
              break;
            }
          }
          nwritten += count;
          inbuf += count * d_itemsize;
        }

        if(d_unbuffered)
          fflush (d_fp);
    }

    int
    CompressedFileSink_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

