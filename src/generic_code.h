/*
    Daala video codec
    Copyright (C) 2012 Daala project contributors

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef GENERIC_ENCODER_H_
#define GENERIC_ENCODER_H_

#include "entenc.h"
#include "entdec.h"

#define GENERIC_TABLES 12

typedef struct {
  unsigned char icdf[GENERIC_TABLES][16];
  unsigned char tot[GENERIC_TABLES];
  int increment;
} GenericEncoder;

int generic_decode(ec_dec *dec, GenericEncoder *model, int *ExQ4);

#endif
