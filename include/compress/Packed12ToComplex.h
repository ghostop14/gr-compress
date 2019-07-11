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


#ifndef INCLUDED_COMPRESS_PACKED12TOCOMPLEX_H
#define INCLUDED_COMPRESS_PACKED12TOCOMPLEX_H

#include <compress/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
  namespace compress {

    /*!
     * \brief <+description of block+>
     * \ingroup compress
     *
     */
    class COMPRESS_API Packed12ToComplex : virtual public gr::sync_decimator
    {
     public:
      typedef boost::shared_ptr<Packed12ToComplex> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of compress::Packed12ToComplex.
       *
       * To avoid accidental use of raw pointers, compress::Packed12ToComplex's
       * constructor is in a private implementation
       * class. compress::Packed12ToComplex::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace compress
} // namespace gr

#endif /* INCLUDED_COMPRESS_PACKED12TOCOMPLEX_H */

