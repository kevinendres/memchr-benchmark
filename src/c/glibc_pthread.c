/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on strlen implementation by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se) and
   commentary by Jim Blandy (jimb@ai.mit.edu);
   adaptation to memchr suggested by Dick Karpinski (dick@cca.ucsf.edu),
   and implemented by Roland McGrath (roland@ai.mit.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <string.h>

#include <stddef.h>
#include <stdlib.h>

#include <limits.h>

#include <pthread.h>

#include <unistd.h>

#include "memchr.h"

#define THREAD_CNT 20

void *inner_loop(void *vargp);

typedef unsigned long int longword;

/* GLOBALS */
const unsigned char *char_ptr;
const longword *longword_ptr;
longword repeated_one;
longword repeated_c;
unsigned char c;
static size_t nsize_per_thread;
static longword *data_ptr;

/* Search no more than N bytes of S for C.  */
void *
MEMCHR_IMPL(void const *s, int c_in, size_t n)
{
  /* On 32-bit hardware, choosing longword to be a 32-bit unsigned
     long instead of a 64-bit uintmax_t tends to give better
     performance.  On 64-bit hardware, unsigned long is generally 64
     bits already.  Change this typedef to experiment with
     performance.  */
  // typedef unsigned long int longword;

  // const unsigned char *char_ptr;
  // const longword *longword_ptr;
  // longword repeated_one;
  // longword repeated_c;
  // unsigned char c;

  c = (unsigned char) c_in;

/* NO NEED TO HANDLE INITIAL BYTES IF MEMORY IS PRE-ALIGNED */

  /* Handle the first few bytes by reading one byte at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  // for (char_ptr = (const unsigned char *) s;
  //      n > 0 && (size_t) char_ptr % sizeof (longword) != 0;
  //      --n, ++char_ptr)
  //   if (*char_ptr == c)
  //     return (void *) char_ptr;
  char_ptr = (const unsigned char *) s;
  longword_ptr = (const longword *) char_ptr;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to any size longwords.  */

  /* Compute auxiliary longword values:
     repeated_one is a value which has a 1 in every byte.
     repeated_c has c in every byte.  */
  repeated_one = 0x01010101;
  repeated_c = c | (c << 8);
  repeated_c |= repeated_c << 16;
  if (0xffffffffU < (longword) -1)
    {
      repeated_one |= repeated_one << 31 << 1;
      repeated_c |= repeated_c << 31 << 1;
      if (8 < sizeof (longword))
	{
	  size_t i;

	  for (i = 64; i < sizeof (longword) * 8; i *= 2)
	    {
	      repeated_one |= repeated_one << i;
	      repeated_c |= repeated_c << i;
	    }
	}
    }
  
  //Multithreading
  char_ptr = NULL;    //overwritten if char is found, returns NULL otherwise

  pthread_t tid[THREAD_CNT];
  long myid[THREAD_CNT], i;
  nsize_per_thread = n / THREAD_CNT;

  data_ptr = (const longword *) malloc(5*sizeof(longword));
  *data_ptr = longword_ptr;
  *(data_ptr + 1) = repeated_c;
  *(data_ptr + 2) = repeated_one;
  *(data_ptr + 3) = nsize_per_thread;
  *(data_ptr + 4) = c;

  for (i = 0; i < THREAD_CNT; i++) {
    myid[i] = i;
    pthread_create(&tid[i], NULL, inner_loop, &myid[i]); 
  }
  
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_join(tid[i], NULL);
  }

  return char_ptr;
}


void *inner_loop(void *vargp)
{
  long myid = *((long *) vargp);
  const unsigned char *local_char_ptr;
  const longword *local_longword_ptr = *data_ptr + (*(data_ptr + 3) * myid);
  size_t local_n = *(data_ptr + 3);

  while (local_n >= sizeof (longword))
    {
      unsigned long int longword1 = *local_longword_ptr ^ repeated_c;

      if ((((longword1 - repeated_one) & ~longword1)
	   & (repeated_one << 7)) != 0)
	break;
      local_longword_ptr++;
      local_n -= sizeof (longword);
    }

  local_char_ptr = (const unsigned char *) local_longword_ptr;

  for (; local_n > 0; --local_n, ++local_char_ptr)
    {
      if (*local_char_ptr == c) {
        /* THREAD UNSAFE BEHAVIOR BELOW IF MORE THAN ONE OCCURANCE OF SEARCH CHAR
           NEED TO USE MUTEX TO LOCK */
        char_ptr = local_char_ptr;
        break;
      }
    }

  return NULL;
}  
