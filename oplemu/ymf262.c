/*
 * Yamaha YMF262 (OPL3) emulator
 * Copyright (C) 2002 Volker Gietz <talphir@web.de>
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * NOTES:
 * Right now we have a waveform and envelope generator in
 * (what I think is) microprocessor style.
 */

#include <stdlib.h>
#include <math.h>

#include "ymf262.h"

/***** Defines *****/

/*
 * inline definitions for certain systems. Supported systems at this time
 * are: gcc, MSVC
 */
#ifdef __GCC__
#	define INLINE	__inline
#elif defined(_MSC_VER)
#	define INLINE	__inline
#else
#	define INLINE
#endif

/* pi */
#define PI	3.141592653

/* Boolean values */
#define TRUE	1
#define FALSE	0

/***** Global variables *****/

/* table of the first quarter of a sine wave */
static int16 sine[512];

/***** Implementation *****/

static INLINE int16 get_sine(int32 phi)
/* Sine readout function. Virtually extends the sine table to 2048 entries. */
{
  if(phi & 0x40000000) 
    phi ^= 0x3FFFFFFF;				// symmetry to 90 and 270 dgs

  return sine[(phi >> 21) & 511];		// map to 512 entries
}

static INLINE int32 phasor_get(YMF262 *opl)
/* Phase accumulating saw wave generator. */
{
  return opl->phasor_phi += opl->phasor_omega;
}

static int16 waveform_get(YMF262 *opl, uint8 op, int32 phi)
/*
 * OPL2 waveform generator. Four waveforms are selectable:
 *     __        __        __  __    _   _
 *    /  \      /  \___   /  \/  \  / |_/ |_
 *        \__/      
 *
 *     0:sine   1:sine>0  2:|sine|  3:chopped
 */
{
  int16	y = get_sine(phi);

  // use upper two bits to mangle waveform
  if(opl->op[op].waveform == 3 && phi & 0x40000000) y = 0;
  return (phi & 0x80000000) ? ((opl->op[op].waveform & 1) ? 0 :
			       ((opl->op[op].waveform & 2) ? y : -y)) : y;
}

static uint32 env_get(YMF262 *opl, uint8 op)
/*
 * Returns current ADSR envelope level for operator 'op' and calculates
 * the next one.
 */
{
  uint32	out = opl->op[op].env_level;

  /* env_level *= 1 - 1/(2^shift) */
  opl->op[op].env_level -= (opl->op[op].env_level >> opl->op[op].env_shift);

  return out;
}

static uint8 env_finished(YMF262 *opl, uint8 op)
/* Returns whether the current ADSR run for operator 'op' has finished. */
{
  /* OPL has only 24(?) bits so "quite zero" is OK. */
  return !(opl->op[op].env_level >> 8);
}

static void keyon(YMF262 *opl, uint8 op)
/* Set ADSR key on for operator 'op'. */
{
  if(opl->op[op].attack) return;	/* am already in attack */
  opl->op[op].attack = TRUE;

  /* take over at current level */
  opl->op[op].env_level = ~(opl->op[op].env_level + opl->op[op].bias);
  opl->op[op].env_shift = opl->op[op].arate;	/* use attack rate */
}

static void keyoff(YMF262 *opl, uint8 op)
/* Set ADSR key off for operator 'op'. */
{
  if(opl->op[op].attack) {
    opl->op[op].env_level = ~opl->op[op].env_level;	/* cancel inversion */
    opl->op[op].attack = FALSE;
  } else {
    opl->op[op].env_level += opl->op[op].bias;	/* take over at actual level */
    opl->op[op].bias = 0;			/* and let go to zero */
  }

  opl->op[op].env_shift = opl->op[op].rrate;	/* use release rate */
}

static int32 adsr_get(YMF262 *opl, uint8 op)
/* Get current ADSR level for operator 'op'. */
{
  uint32	level = env_get(opl, op);	/* get new level */

  if(opl->op[op].attack) {
    if(env_finished(opl, op)) {		/* time to go from attack to decay */
      opl->op[op].attack = FALSE;
      opl->op[op].bias = opl->op[op].suslevel;	/* bias is now sustain level */
      /* leave room for the bias */
      opl->op[op].env_level = ~opl->op[op].suslevel;
      opl->op[op].env_shift = opl->op[op].drate;	/* use decay rate */
    }

    return ~level;		/* if attack: level goes up */
  }

  return level + opl->op[op].bias;	/* no attack: level goes down */
}

static void one_time_init(void)
/* One-time global variable initialization procedure. */
{
  static uint8	one_time = FALSE;	/* one-time init flag */
  uint32	i;

  if(one_time) return;	/* Return immediately, if already initialized */
  one_time = TRUE;

  /* Initialize the global sine array */
  for(i = 0; i < 512; i++)
    sine[i] = (int16)(32767.0 * sin(i / 1024.0 * PI));
}

/***** Exported functions *****/

YMF262 *ymf262_create(uint8 channels, uint8 bits, uint32 rate)
{
  YMF262	*opl = (YMF262 *)malloc(sizeof(YMF262));

  /* One-time initialization procedure */
  one_time_init();

  /* Reset data */
  memset(opl, 0, sizeof(YMF262));

  /* Set configuration */
  opl->cfg_channels = channels;
  opl->cfg_bits = bits;
  opl->cfg_rate = rate;

  return opl;
}

void ymf262_destroy(YMF262 *opl)
{
  /* Free YMF262 data structure itself */
  free(opl);
}

void ymf262_render(YMF262 *opl, void *buffer, uint32 length)
{
}

void ymf262_write(YMF262 *opl, uint8 set, uint8 index, uint8 data)
{
}

uint8 ymf262_readstatus(YMF262 *opl)
{
  return opl->status;
}
