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
#include "compress_impl.h"
#include <chrono>
#include <iomanip>

namespace gr {
  namespace compress {
  class comma_numpunct : public std::numpunct<char>
  {
    protected:
      virtual char do_thousands_sep() const
      {
          return ',';
      }

      virtual std::string do_grouping() const
      {
          return "\03";
      }
  };

    compress::sptr
    compress::make(int itemsize,int compressionBlockSize, int numThreads, bool addHeader, bool verbose)
    {
      return gnuradio::get_initial_sptr
        (new compress_impl(itemsize, compressionBlockSize, numThreads, addHeader, verbose));
    }

    /*
     * The private constructor
     */
    compress_impl::compress_impl(int itemsize, int compressionBlockSize, int numThreads, bool addHeader, bool verbose)
      : gr::sync_block("compress",
              gr::io_signature::make(1, 1, itemsize),
              gr::io_signature::make(0, 0, 0))
    {
    	// The header is a uint64_t representing the original size

    	d_verbose = verbose;
        verboseCounter = 0;

    	d_addHeader = addHeader;
    	d_itemsize = itemsize;
    	d_numThreads = numThreads;
    	d_compressionBlockSize = compressionBlockSize;
    	// d_compressionBlockSize = 4096*40;

        pCompressor = new ZStdCompressor(numThreads);

        if (d_addHeader) {
        	compressedBuffer = new unsigned char[d_compressionBlockSize+sizeof(CompressionHeader)];
        }
        else {
            compressedBuffer = new unsigned char[d_compressionBlockSize];
        }
        compressedBufferSize = d_compressionBlockSize;

    	localWriteBuffer = new unsigned char[d_compressionBlockSize];
    	localWriteBufferSize = d_compressionBlockSize;
    	localWriteBufferNextIndex = 0;

        message_port_register_out(pmt::mp("compressed"));
    }

    /*
     * Our virtual destructor.
     */
    compress_impl::~compress_impl()
    {
    	bool retVal = stop();
    }

    bool compress_impl::stop() {

    	if (pCompressor) {
    		delete pCompressor;
    		pCompressor = NULL;
    	}

    	if (compressedBuffer) {
    		delete[] compressedBuffer;
    		compressedBuffer = NULL;
    		compressedBufferSize = 0;
    	}

    	if (localWriteBuffer) {
    		delete[] localWriteBuffer;
    		localWriteBuffer = NULL;
    		localWriteBufferSize = 0;
    	}

    	return true;
    }

    void compress_impl::timeCompression() {
    	std::locale comma_locale(std::locale(), new comma_numpunct());

    	// tell cout to use our new locale.
    	std::cout.imbue(comma_locale);
    	int queueSize = 64000;
    	if (queueSize > localWriteBufferSize) {
    		// This is the buffer that holds the block to be compressed.  Could be queue size or greater.
    		delete[] localWriteBuffer;
    		localWriteBuffer = new unsigned char[queueSize];
    		localWriteBufferSize = queueSize;
    	}

		// Check if we have enough space in our compression buffer
    	long worstCaseTargetBuffer = pCompressor->maxDestBufferSize(queueSize);

    	if (worstCaseTargetBuffer > compressedBufferSize) {
    		delete[] compressedBuffer;
    		if (d_addHeader)
        		compressedBuffer = new unsigned char[worstCaseTargetBuffer + sizeof(CompressionHeader)];
    		else
    			compressedBuffer = new unsigned char[worstCaseTargetBuffer];

    		compressedBufferSize = worstCaseTargetBuffer;
    	}

    	uint64_t actualCompressedSize;

    	unsigned char *pOutputBuffer=compressedBuffer;

    	if (d_addHeader) {
    		pOutputBuffer = &compressedBuffer[sizeof(CompressionHeader)];
    	}

    	actualCompressedSize = pCompressor->compress(pOutputBuffer,compressedBufferSize,localWriteBuffer,queueSize);

    	std::chrono::time_point<std::chrono::steady_clock> start, end;
    	std::chrono::duration<double> elapsed_seconds = end-start;

    	std::cout << "----------------------------------------------------------" << std::endl;
    	std::cout << "Test Case (all 0's) Compressed size: " << actualCompressedSize << std::endl;
    	float compressedRatio = ((float)actualCompressedSize / (float)queueSize * 100.0);
    	std::cout << "Compressed ratio: " << compressedRatio << "%, " << (100.0-compressedRatio) << "% compression." << std::endl;

    	int sizeofcomplex = sizeof(gr_complex);
    	int largeBlockSize = queueSize;
    	int complexSamples = largeBlockSize / (sizeofcomplex);
    	int iterations = 100;
    	float elapsed_time,throughput_original,throughput;

    	std::cout << "Testing compression only with " << largeBlockSize << " data block (equivilent to " << complexSamples << " complex samples)..." << std::endl;
    	std::cout << "Running with " << d_numThreads << " threads" << std::endl;

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (int i=0;i<iterations;i++) {
        	actualCompressedSize = pCompressor->compress(pOutputBuffer,compressedBufferSize,localWriteBuffer,queueSize);
    	}
    	end = std::chrono::steady_clock::now();

    	elapsed_seconds = end-start;

    	elapsed_time = elapsed_seconds.count()/(float)iterations;
    	throughput_original = largeBlockSize / elapsed_time;
    	float cs_throughput = complexSamples / elapsed_time;
    	float u8c_throughput = largeBlockSize / 2 / elapsed_time;

    	std::cout << "Run Time:   " << std::fixed << std::setw(11)
        << std::setprecision(6) << elapsed_time << " s" << std::endl;
    	std::cout << std::setprecision(2) << throughput_original << " Bps" << std::endl <<
    				cs_throughput << " float complex sps" << std::endl <<
    				u8c_throughput << " byte complex sps" << std::endl;
   }

    const unsigned char * compress_impl::getCompressedBuffer() {
    	return compressedBuffer;
    }

    int
    compress_impl::test(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function

        const unsigned char *in = (const unsigned char *) input_items[0];

        long noi = noutput_items * d_itemsize;

        // Add the bytes that just came in to the queue
        long queueSize = localWriteBufferNextIndex + noi;
        if (queueSize > localWriteBufferSize) {
        	unsigned char *newBuff = new unsigned char[queueSize];
        	// If we had data, copy it to the new buffer
        	if (localWriteBufferNextIndex > 0)
        		memcpy(newBuff,localWriteBuffer,localWriteBufferNextIndex);

        	// delete old and swap
        	delete[] localWriteBuffer;
        	localWriteBuffer = newBuff;
        	localWriteBufferSize = queueSize;
        }

        memcpy(&localWriteBuffer[localWriteBufferNextIndex],in,noi);
        localWriteBufferNextIndex = queueSize;

        if (queueSize >= d_compressionBlockSize) {
    		// Check if we have enough space in our compression buffer
        	long worstCaseTargetBuffer = pCompressor->maxDestBufferSize(queueSize);

        	if (worstCaseTargetBuffer > compressedBufferSize) {
        		delete[] compressedBuffer;
        		if (d_addHeader)
            		compressedBuffer = new unsigned char[worstCaseTargetBuffer + sizeof(CompressionHeader)];
        		else
        			compressedBuffer = new unsigned char[worstCaseTargetBuffer];

        		compressedBufferSize = worstCaseTargetBuffer;
        	}

        	uint64_t actualCompressedSize;

        	unsigned char *pOutputBuffer=compressedBuffer;

        	if (d_addHeader) {
        		pOutputBuffer = &compressedBuffer[sizeof(CompressionHeader)];
        	}

        	actualCompressedSize = pCompressor->compress(pOutputBuffer,compressedBufferSize,localWriteBuffer,queueSize);

        	// This is only used for testing.
        	compressedSize = actualCompressedSize;

        	// We compress it all.
        	localWriteBufferNextIndex = 0;

        	// Now build the message
			pmt::pmt_t meta = pmt::make_dict();
			meta = pmt::dict_add(meta, pmt::mp("uncompressedsize"), pmt::from_long((long)queueSize));
			meta = pmt::dict_add(meta, pmt::mp("compressedsize"), pmt::from_long((long)actualCompressedSize));
			meta = pmt::dict_add(meta, pmt::mp("hasheader"), pmt::from_bool(d_addHeader));

			long vecSize = actualCompressedSize;

			if (d_addHeader) {
        		// Header is:
        		// uint64_t uncompressed_size
        		// uint64_t compressed_size
        		CompressionHeader *pHeader;
        		pHeader = (CompressionHeader *)compressedBuffer;
        		pHeader->uncompressed_size = queueSize;
        		pHeader->compressed_size = actualCompressedSize;
				// Note our vector is just a little longer.
				vecSize += sizeof(CompressionHeader);
			}


			pmt::pmt_t data_out(pmt::make_blob((const void *)compressedBuffer,(long)vecSize));
			pmt::pmt_t datapdu = pmt::cons( meta, data_out );

			// message_port_pub(pmt::mp("compressed"),datapdu);
        }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    int
    compress_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        gr::thread::scoped_lock guard(d_mutex);	// hold mutex for duration of this function

        const unsigned char *in = (const unsigned char *) input_items[0];

        long noi = noutput_items * d_itemsize;

        // Add the bytes that just came in to the queue
        long queueSize = localWriteBufferNextIndex + noi;
        if (queueSize > localWriteBufferSize) {
        	unsigned char *newBuff = new unsigned char[queueSize];
        	// If we had data, copy it to the new buffer
        	if (localWriteBufferNextIndex > 0)
        		memcpy(newBuff,localWriteBuffer,localWriteBufferNextIndex);

        	// delete old and swap
        	delete[] localWriteBuffer;
        	localWriteBuffer = newBuff;
        	localWriteBufferSize = queueSize;
        }

        memcpy(&localWriteBuffer[localWriteBufferNextIndex],in,noi);
        localWriteBufferNextIndex = queueSize;

        if (queueSize >= d_compressionBlockSize) {
    		// Check if we have enough space in our compression buffer
        	long worstCaseTargetBuffer = pCompressor->maxDestBufferSize(queueSize);

        	if (worstCaseTargetBuffer > compressedBufferSize) {
        		delete[] compressedBuffer;
        		if (d_addHeader)
            		compressedBuffer = new unsigned char[worstCaseTargetBuffer + sizeof(CompressionHeader)];
        		else
        			compressedBuffer = new unsigned char[worstCaseTargetBuffer];

        		compressedBufferSize = worstCaseTargetBuffer;
        	}

        	uint64_t actualCompressedSize;

        	unsigned char *pOutputBuffer=compressedBuffer;

        	if (d_addHeader) {
        		pOutputBuffer = &compressedBuffer[sizeof(CompressionHeader)];
        	}

        	actualCompressedSize = pCompressor->compress(pOutputBuffer,compressedBufferSize,localWriteBuffer,queueSize);
        	// std::cout << "Calling compress with " << queueSize << " bytes. Compressed size=" << actualCompressedSize << std::endl;


			if (d_verbose) {
				if (verboseCounter==0) {
					float compressedRatio = ((float)actualCompressedSize / (float)queueSize * 100.0);
					std::cout << "[Compress] Compressed ratio: " << compressedRatio << "%, " << (100.0-compressedRatio) << "% compression." << std::endl;
				}

				verboseCounter++;

				if (verboseCounter > 1000)
					verboseCounter = 0;
			}

        	// We compress it all.
        	localWriteBufferNextIndex = 0;

        	// Now build the message
			pmt::pmt_t meta = pmt::make_dict();
			meta = pmt::dict_add(meta, pmt::mp("uncompressedsize"), pmt::from_long((long)queueSize));
			meta = pmt::dict_add(meta, pmt::mp("compressedsize"), pmt::from_long((long)actualCompressedSize));
			meta = pmt::dict_add(meta, pmt::mp("hasheader"), pmt::from_bool(d_addHeader));

			long vecSize = actualCompressedSize;

			if (d_addHeader) {
        		// Header is:
        		// uint64_t uncompressed_size
        		// uint64_t compressed_size
        		CompressionHeader *pHeader;
        		pHeader = (CompressionHeader *)compressedBuffer;
        		pHeader->uncompressed_size = queueSize;
        		pHeader->compressed_size = actualCompressedSize;
				// Note our vector is just a little longer.
				vecSize += sizeof(CompressionHeader);
			}
			// Blob is much faster
			pmt::pmt_t data_out(pmt::make_blob((const void *)compressedBuffer,(long)vecSize));
			pmt::pmt_t datapdu = pmt::cons( meta, data_out );

			message_port_pub(pmt::mp("compressed"),datapdu);
        }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

