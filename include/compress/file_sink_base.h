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


#ifndef INCLUDED_COMPRESS_FILE_SINK_BASE_H
#define INCLUDED_COMPRESS_FILE_SINK_BASE_H

#include <compress/api.h>
#include <boost/thread.hpp>
#include <cstdio>

namespace gr {
  namespace compress {

  class COMPRESS_API file_sink_base
  {
  protected:
    FILE        *d_fp;        // current FILE pointer
    FILE        *d_new_fp;    // new FILE pointer
    bool         d_updated;   // is there a new FILE pointer?
    bool         d_is_binary;
    boost::mutex d_mutex;
    bool         d_unbuffered;
    bool         d_append;

  protected:
    file_sink_base(const char *filename, bool is_binary, bool append);

  public:
    file_sink_base() {}
    ~file_sink_base();

    /*!
     * \brief Open filename and begin output to it.
     */
    bool open(const char *filename);

    /*!
     * \brief Close current output file.
     *
     * Closes current output file and ignores any output until
     * open is called to connect to another file.
     */
    void close();

    /*!
     * \brief if we've had an update, do it now.
     */
    void do_update();

    /*!
     * \brief turn on unbuffered writes for slower outputs
     */
    void set_unbuffered(bool unbuffered);
  };

  } // namespace compress
} // namespace gr

#endif /* INCLUDED_COMPRESS_COMPRESSEDFILESINK_H */

