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
#include "ComplexToPacked12_impl.h"

#define USHRT12_MAX 4095

namespace gr {
  namespace compress {

    ComplexToPacked12::sptr
    ComplexToPacked12::make()
    {
      return gnuradio::get_initial_sptr
        (new ComplexToPacked12_impl());
    }

    /*
     * The private constructor
     */
    ComplexToPacked12_impl::ComplexToPacked12_impl()
      : gr::sync_interpolator("ComplexToPacked12",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(char)), 3)
    {

    }

    /*
     * Our virtual destructor.
     */
    ComplexToPacked12_impl::~ComplexToPacked12_impl()
    {
    }

    int
    ComplexToPacked12_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0];
      unsigned char *out = (unsigned char *) output_items[0];

      // 3 for the interpolation to bytes, *2 to split the complex
      long numFloats= noutput_items / 3 * 2;

      for (long i=0;i<numFloats;i+=2) {
    	  // convert to 12-bit
    	  // Shift scale from -1->1 to 0->2 to pack in unsigned correctly
    	  // use int32_t just in case our number gets too big until we min it.
    	  int32_t real = (int32_t)lrintf( (in[i]+1.0) / 2.0 * (float)USHRT12_MAX );
    	  int32_t imag = (int32_t)lrintf( (in[i+1]+1.0) / 2.0 * (float)USHRT12_MAX );

    	  real = std::min(real,(int32_t)USHRT12_MAX);
    	  imag = std::min(imag,(int32_t)USHRT12_MAX);

    	  // pack into 3 bytes
    	  *out++ = (unsigned)((real & 0x0FF0) >> 4);
    	  *out++ = (unsigned)((real & 0x000F) << 4) + (unsigned)((imag & 0x0F00) >> 8);
    	  *out++ = (unsigned)(imag & 0x00FF);
      }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace compress */
} /* namespace gr */

