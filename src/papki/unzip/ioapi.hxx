/*
The MIT License (MIT)

Copyright (c) 2015-2023 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

/* ioapi.h -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant
*/

#ifndef _ZLIBIOAPI_H
#define _ZLIBIOAPI_H

#define ZLIB_FILEFUNC_SEEK_CUR (1)
#define ZLIB_FILEFUNC_SEEK_END (2)
#define ZLIB_FILEFUNC_SEEK_SET (0)

#define ZLIB_FILEFUNC_MODE_READ (1)
#define ZLIB_FILEFUNC_MODE_WRITE (2)
#define ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#define ZLIB_FILEFUNC_MODE_EXISTING (4)
#define ZLIB_FILEFUNC_MODE_CREATE (8)

#ifndef ZCALLBACK

#	if (defined(WIN32) || defined(WINDOWS) || defined(_WINDOWS)) && defined(CALLBACK) && defined(USEWINDOWS_CALLBACK)
#		define ZCALLBACK CALLBACK
#	else
#		define ZCALLBACK
#	endif
#endif

#include "zconf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef voidpf(ZCALLBACK* open_file_func) OF((voidpf opaque, const char* filename, int mode));
typedef uLong(ZCALLBACK* read_file_func) OF((voidpf opaque, voidpf stream, void* buf, uLong size));
typedef uLong(ZCALLBACK* write_file_func) OF((voidpf opaque, voidpf stream, const void* buf, uLong size));
typedef long(ZCALLBACK* tell_file_func) OF((voidpf opaque, voidpf stream));
typedef long(ZCALLBACK* seek_file_func) OF((voidpf opaque, voidpf stream, uLong offset, int origin));
typedef int(ZCALLBACK* close_file_func) OF((voidpf opaque, voidpf stream));
typedef int(ZCALLBACK* testerror_file_func) OF((voidpf opaque, voidpf stream));

typedef struct zlib_filefunc_def_s {
	open_file_func zopen_file;
	read_file_func zread_file;
	write_file_func zwrite_file;
	tell_file_func ztell_file;
	seek_file_func zseek_file;
	close_file_func zclose_file;
	testerror_file_func zerror_file;
	voidpf opaque;
} zlib_filefunc_def;

void fill_fopen_filefunc OF((zlib_filefunc_def * pzlib_filefunc_def));

#define ZREAD(filefunc, filestream, buf, size) ((*((filefunc).zread_file))((filefunc).opaque, filestream, buf, size))
#define ZWRITE(filefunc, filestream, buf, size) ((*((filefunc).zwrite_file))((filefunc).opaque, filestream, buf, size))
#define ZTELL(filefunc, filestream) ((*((filefunc).ztell_file))((filefunc).opaque, filestream))
#define ZSEEK(filefunc, filestream, pos, mode) ((*((filefunc).zseek_file))((filefunc).opaque, filestream, pos, mode))
#define ZCLOSE(filefunc, filestream) ((*((filefunc).zclose_file))((filefunc).opaque, filestream))
#define ZERROR(filefunc, filestream) ((*((filefunc).zerror_file))((filefunc).opaque, filestream))

#ifdef __cplusplus
}
#endif

#endif
