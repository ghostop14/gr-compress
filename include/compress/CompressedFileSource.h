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


#ifndef INCLUDED_COMPRESS_COMPRESSEDFILESOURCE_H
#define INCLUDED_COMPRESS_COMPRESSEDFILESOURCE_H

#include <compress/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace compress {

    /*!
     * \brief <+description of block+>
     * \ingroup compress
     *
     */
    class COMPRESS_API CompressedFileSource : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<CompressedFileSource> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of compress::CompressedFileSource.
       *
       * To avoid accidental use of raw pointers, compress::CompressedFileSource's
       * constructor is in a private implementation
       * class. compress::CompressedFileSource::make is the public interface for
       * creating new instances.
       */
      static sptr make(size_t itemsize, const char *filename, bool repeat = false);
      /*!
       * \brief seek file to \p seek_point relative to \p whence
       *
       * \param seek_point	sample offset in file
       * \param whence	one of SEEK_SET, SEEK_CUR, SEEK_END (man fseek)
       */

      virtual bool seek(long seek_point, int whence) = 0;

      /*!
       * \brief Opens a new file.
       *
       * \param filename	name of the file to source from
       * \param repeat	repeat file from start
       */
      virtual void open(const char *filename, bool repeat) = 0;

      /*!
       * \brief Close the file handle.
       */
      virtual void close() = 0;

      /*!
       * \brief Add a stream tag to the first sample of the file if true
       */
      virtual void set_begin_tag(pmt::pmt_t val) = 0;
    };

  } // namespace compress
} // namespace gr

#endif /* INCLUDED_COMPRESS_COMPRESSEDFILESOURCE_H */

