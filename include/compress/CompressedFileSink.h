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


#ifndef INCLUDED_COMPRESS_COMPRESSEDFILESINK_H
#define INCLUDED_COMPRESS_COMPRESSEDFILESINK_H

#include <compress/api.h>
#include <gnuradio/sync_block.h>
#include "file_sink_base.h"

namespace gr {
  namespace compress {

    /*!
     * \brief <+description of block+>
     * \ingroup compress
     *
     */

      class COMPRESS_API CompressedFileSink : virtual public gr::sync_block, virtual public file_sink_base
    {
     public:
      typedef std::shared_ptr<CompressedFileSink> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of compress::CompressedFileSink.
       *
       * To avoid accidental use of raw pointers, compress::CompressedFileSink's
       * constructor is in a private implementation
       * class. compress::CompressedFileSink::make is the public interface for
       * creating new instances.
       */
      static sptr make(size_t itemsize, const char *filename, bool unbuffered, bool append=false);
    };

  } // namespace compress
} // namespace gr

#endif /* INCLUDED_COMPRESS_COMPRESSEDFILESINK_H */

