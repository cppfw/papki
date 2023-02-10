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

#ifndef ZLIBIOAPI_H
#define ZLIBIOAPI_H

enum class zlib_seek_relative {
	zlib_filefunc_seek_set,
	zlib_filefunc_seek_cur,
	zlib_filefunc_seek_end,
};

enum class zlib_file_mode {
	zlib_filefunc_mode_read = 1,
	zlib_filefunc_mode_write = 2,
	zlib_filefunc_mode_readwritefilter = 3,
	zlib_filefunc_mode_existing = 4,
	zlib_filefunc_mode_create = 8,
};

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

using open_file_func = voidpf(ZCALLBACK*) OF((voidpf opaque, const char* filename, int mode));
using read_file_func = uLong(ZCALLBACK*) OF((voidpf opaque, voidpf stream, void* buf, uLong size));
using write_file_func = uLong(ZCALLBACK*) OF((voidpf opaque, voidpf stream, const void* buf, uLong size));
using tell_file_func = long(ZCALLBACK*) OF((voidpf opaque, voidpf stream));
using seek_file_func = long(ZCALLBACK*) OF((voidpf opaque, voidpf stream, long offset, zlib_seek_relative origin));
using close_file_func = int(ZCALLBACK*) OF((voidpf opaque, voidpf stream));
using testerror_file_func = int(ZCALLBACK*) OF((voidpf opaque, voidpf stream));

using zlib_filefunc_def = struct zlib_filefunc_def_s {
	open_file_func zopen_file;
	read_file_func zread_file;
	write_file_func zwrite_file;
	tell_file_func ztell_file;
	seek_file_func zseek_file;
	close_file_func zclose_file;
	testerror_file_func zerror_file;
	voidpf opaque;
};

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
