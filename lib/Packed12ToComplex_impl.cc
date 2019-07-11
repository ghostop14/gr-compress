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
#include "Packed12ToComplex_impl.h"

#define USHRT12_MAX 4095

namespace gr {
  namespace compress {

    Packed12ToComplex::sptr
    Packed12ToComplex::make()
    {
      return gnuradio::get_initial_sptr
        (new Packed12ToComplex_impl());
    }

    /*
     * The private constructor
     */
    Packed12ToComplex_impl::Packed12ToComplex_impl()
      : gr::sync_decimator("Packed12ToComplex",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), 3)
    {

    }

    /*
     * Our virtual destructor.
     */
    Packed12ToComplex_impl::~Packed12ToComplex_impl()
    {
    }

    int
    Packed12ToComplex_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const unsigned char *in = (const unsigned char *) input_items[0];
      float *out = (float *) output_items[0];

      long inBytes = noutput_items * 3;

      /* Packing Process:
       *
          out[outCtr++] = (char)((real & 0x0FF0) >> 4);
    	  out[outCtr++] = (char)((real & 0x000F) << 4) + (char)((imag & 0x0F00) >> 8);
    	  out[outCtr++] = (char)(imag & 0x00FF);
       *
       */
      for (long i=0;i<inBytes;i+=3) {
    	  // unpack bytes
    	  uint16_t real = ((uint16_t)in[i] << 4) + (uint16_t)((in[i+1] & 0xF0) >> 4);
    	  uint16_t imag = ((uint16_t)(in[i+1] & 0x0F) << 8) + (uint16_t)in[i+2];

    	  // convert to float
    	  *out++ = ((float)real / (float)USHRT12_MAX)*2.0 - 1.0;
    	  *out++ = ((float)imag / (float)USHRT12_MAX)*2.0 - 1.0;
      }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

