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

#include <gnuradio/thread/thread.h>
#include <gnuradio/io_signature.h>
#include "CompressedFileSource_impl.h"

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <stdio.h>

#include "ZStdCompressor.h" // Need the header definition

// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define	OUR_O_BINARY O_BINARY
#else
#define	OUR_O_BINARY 0
#endif
// should be handled via configure
#ifdef O_LARGEFILE
#define	OUR_O_LARGEFILE	O_LARGEFILE
#else
#define	OUR_O_LARGEFILE 0
#endif

namespace gr {
  namespace compress {

    CompressedFileSource::sptr
    CompressedFileSource::make(size_t itemsize, const char *filename, bool repeat)
    {
      return gnuradio::get_initial_sptr
        (new CompressedFileSource_impl(itemsize, filename, repeat));
    }

    /*
     * The private constructor
     */
    CompressedFileSource_impl::CompressedFileSource_impl(size_t itemsize, const char *filename, bool repeat)
      : gr::sync_block("CompressedFileSource",
		      io_signature::make(0, 0, 0),
		      io_signature::make(1, 1, itemsize)),
	d_itemsize(itemsize), d_fp(0), d_new_fp(0), d_repeat(repeat),
	d_updated(false), d_file_begin(true), d_repeat_cnt(0),
       d_add_begin_tag(pmt::PMT_NIL)
    {
    	open(filename, repeat);
    	do_update();

    	long d_compressionBlockSize = 64000;

    	localWriteBuffer = new unsigned char[d_compressionBlockSize];
    	localWriteBufferSize = d_compressionBlockSize;

    	compressedBuffer = new unsigned char[d_compressionBlockSize];
    	compressedBufferSize = d_compressionBlockSize;

    	std::stringstream str;
    	str << name() << unique_id();
    	_id = pmt::string_to_symbol(str.str());
    }

    /*
     * Our virtual destructor.
     */
    CompressedFileSource_impl::~CompressedFileSource_impl()
    {
    	bool retVal = stop();
    }

    bool CompressedFileSource_impl::stop() {
        if(d_fp) {
            fclose ((FILE*)d_fp);
            d_fp = NULL;
        }

        if(d_new_fp) {
          fclose ((FILE*)d_new_fp);
          d_new_fp = NULL;
        }

        if (localWriteBuffer) {
        	delete[] localWriteBuffer;
        	localWriteBuffer = NULL;
        	localWriteBufferSize = 0;
        }

        if (compressedBuffer) {
        	delete[] compressedBuffer;
        	compressedBuffer = NULL;
        	compressedBufferSize = 0;
        }
    	return true;
    }

    bool
    CompressedFileSource_impl::seek(long seek_point, int whence)
    {
      return fseek((FILE*)d_fp, seek_point *d_itemsize, whence) == 0;
    }


    void
    CompressedFileSource_impl::open(const char *filename, bool repeat)
    {
      // obtain exclusive access for duration of this function
      gr::thread::scoped_lock lock(fp_mutex);

      int fd;

      // we use "open" to use to the O_LARGEFILE flag
      if((fd = ::open(filename, O_RDONLY | OUR_O_LARGEFILE | OUR_O_BINARY)) < 0) {
	perror(filename);
	throw std::runtime_error("can't open file");
      }

      if(d_new_fp) {
	fclose(d_new_fp);
	d_new_fp = 0;
      }

      if((d_new_fp = fdopen (fd, "rb")) == NULL) {
	perror(filename);
	::close(fd);	// don't leak file descriptor if fdopen fails
	throw std::runtime_error("can't open file");
      }

      //Check to ensure the file will be consumed according to item size
      fseek(d_new_fp, 0, SEEK_END);
      int file_size = ftell(d_new_fp);
      rewind (d_new_fp);

      //Warn the user if part of the file will not be consumed.
      if(file_size % d_itemsize){
        GR_LOG_WARN(d_logger, "WARNING: File will not be fully consumed with the current output type");
      }

      d_updated = true;
      d_repeat = repeat;
    }

    void
    CompressedFileSource_impl::close()
    {
      // obtain exclusive access for duration of this function
      gr::thread::scoped_lock lock(fp_mutex);

      if(d_new_fp != NULL) {
	fclose(d_new_fp);
	d_new_fp = NULL;
      }
      d_updated = true;
    }

    void
    CompressedFileSource_impl::do_update()
    {
      if(d_updated) {
	gr::thread::scoped_lock lock(fp_mutex); // hold while in scope

	if(d_fp)
	  fclose(d_fp);

	d_fp = d_new_fp;    // install new file pointer
	d_new_fp = 0;
	d_updated = false;
       d_file_begin = true;
      }
    }

    void
    CompressedFileSource_impl::set_begin_tag(pmt::pmt_t val)
    {
      d_add_begin_tag = val;
    }

    int
    CompressedFileSource_impl::work(int noutput_items,
			   gr_vector_const_void_star &input_items,
			   gr_vector_void_star &output_items)
    {
      char *o = (char*)output_items[0];
      int i;
      int size = noutput_items;
      long noi = noutput_items * d_itemsize;

      do_update();       // update d_fp is reqd
      if(d_fp == NULL)
    	  throw std::runtime_error("work with file not open");

      gr::thread::scoped_lock lock(fp_mutex); // hold for the rest of this function

      if (byteQueue.size() >= noi) {
    	  // We had enough in the queue to satisfy the request.  Just return from queue.
    	  for (long i=0;i<noi;i++) {
    		  o[i] = byteQueue.front();
    		  byteQueue.pop();
    	  }

    	  return noutput_items;
      }


      while (byteQueue.size() < noi) {
          // If we're here, we need more data
    	  // Read just a header block and convert it to a CompressionHeader so we know how much we have to read next.
    	  i = fread(localWriteBuffer,1,sizeof(CompressionHeader),(FILE *)d_fp);

    	  if (i < sizeof(CompressionHeader)) {
    		  if (!d_repeat)
    			  return WORK_DONE;
    		  else {
    	    	  if(fseek ((FILE *) d_fp, 0, SEEK_SET) == -1) {
    	    		  fprintf(stderr, "[%s] fseek failed\n", __FILE__);
    	    		  exit(-1);
    	    	  }

    	    	  if (d_add_begin_tag != pmt::PMT_NIL) {
    	    		  d_file_begin = true;
    	    		  d_repeat_cnt++;
    	    	  }

    	    	  return 0;
    		  }
    	  }

    	  CompressionHeader *pHeader;
    	  pHeader = (CompressionHeader *)localWriteBuffer;
    	  uint64_t uncompressedSize = pHeader->uncompressed_size;
    	  uint64_t compressedSize = pHeader->compressed_size;

    	  // make sure our buffers are good.
    		if (uncompressedSize > localWriteBufferSize) {
    			delete [] localWriteBuffer;
    			localWriteBuffer = new unsigned char[uncompressedSize];
    			localWriteBufferSize = uncompressedSize;
    		}

    		if (compressedSize > compressedBufferSize) {
    			delete[] compressedBuffer;
    			compressedBuffer = new unsigned char[compressedSize];
    			compressedBufferSize = compressedSize;
    		}

    		// Read a compressed block.
    		size_t bytesRead = fread(compressedBuffer,1,compressedSize,(FILE *)d_fp);
    		size_t actualDecompressedSize;

    		if (bytesRead == compressedSize) {
    			// If we got a good block, decompress it and add it to the queue.
        		actualDecompressedSize = decompressor.decompress(localWriteBuffer,uncompressedSize,compressedBuffer,compressedSize);

        		// Queue it for local work
        		for (long i=0;i<actualDecompressedSize; i++) {
        			byteQueue.push(localWriteBuffer[i]);
        		}
    		}
    		else {
    			// If we're here, we were supposed to read a compressed block size, but we didn't get enough data.
    			// This could be a corrupt end-of-file or other issue.  So we'll just reset the file and ignore it.
      		  if (!d_repeat)
      			  return WORK_DONE;
      		  else {
      	    	  if(fseek ((FILE *) d_fp, 0, SEEK_SET) == -1) {
      	    		  fprintf(stderr, "[%s] fseek failed\n", __FILE__);
      	    		  exit(-1);
      	    	  }

      	    	  if (d_add_begin_tag != pmt::PMT_NIL) {
      	    		  d_file_begin = true;
      	    		  d_repeat_cnt++;
      	    	  }

      	    	  return 0;
      		  }
    		}
      }

      // If we're here we have enough data to satisfy the request.
	  // We had enough in the queue to satisfy the request.  Just return from queue.
	  for (long i=0;i<noi;i++) {
		  o[i] = byteQueue.front();
		  byteQueue.pop();
	  }

	  return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

