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
 */

#ifndef H_YMF262
#define H_YMF262

#ifdef __cplusplus
extern "C" {
#endif

  typedef signed long		int32;
  typedef signed short		int16;
  typedef signed char		int8;
  typedef unsigned long		uint32;
  typedef unsigned short	uint16;
  typedef unsigned char		uint8;

  typedef struct {
    /* Emulator configuration */
    uint8	cfg_channels, cfg_bits;
    uint32	cfg_rate;

    /* Phasor */
    int32	phasor_phi, phasor_omega;

    /* OPL3 status register */
    uint8	status;

    /* 36 operators */
    struct {
      /* ADSR */
      uint32	env_level, env_shift, arate, drate, rrate, suslevel, bias;
      uint8	attack;

      uint8	waveform;
    } op[36];

    /* 18 channels */
    struct {
    } channel[18];
  } YMF262;

  YMF262 *ymf262_create(uint8 channels, uint8 bits, uint32 rate);
  /*
   * Create and initialize a YMF262 data structure. The emulated YMF262
   * will output audio data using 'channels' audio channels, 'bits'
   * length samples and 'rate' Hz sampling rate.
   *
   * Returns a pointer to the initialized structure, or NULL if an
   * error occured.
   */

  void ymf262_destroy(YMF262 *);
  /* Free the memory of the passed YMF262 data structure. */

  void ymf262_render(YMF262 *, void *buffer, uint32 length);
  /*
   * Render audio data of a YMF262 data structure to a sample buffer,
   * pointed to by 'buffer', with length 'length' bytes.
   */

  void ymf262_write(YMF262 *, uint8 set, uint8 index, uint8 data);
  /*
   * Writes to the OPL3 registers. 'set' determines whether to write to
   * the primary or secondary register set (i.e. 0x888 and 0x389 or
   * 0x38a and 0x38b). 'index' is written to the index and 'data' to the
   * data register, respectively.
   */

  uint8 ymf262_readstatus(YMF262 *);
  /* Returns the contents of the OPL3 status register. */

#ifdef __cplusplus
}
#endif

#endif
