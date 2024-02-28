/*
The MIT License (MIT)

Copyright (c) 2015-2024 Ivan Gagis <igagis@gmail.com>

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

/* ioapi.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant
*/

#include "ioapi.hxx"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <utki/config.hpp>

// NOLINTBEGIN
#include "zlib.h"
// NOLINTEND

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#	define SEEK_CUR 1
#endif

#ifndef SEEK_END
#	define SEEK_END 2
#endif

#ifndef SEEK_SET
#	define SEEK_SET 0
#endif

voidpf ZCALLBACK fopen_file_func OF((voidpf opaque, const char* filename, int mode));

uLong ZCALLBACK fread_file_func OF((voidpf opaque, voidpf stream, void* buf, uLong size));

uLong ZCALLBACK fwrite_file_func OF((voidpf opaque, voidpf stream, const void* buf, uLong size));

long ZCALLBACK ftell_file_func OF((voidpf opaque, voidpf stream));

long ZCALLBACK fseek_file_func OF((voidpf opaque, voidpf stream, long offset, int origin));

int ZCALLBACK fclose_file_func OF((voidpf opaque, voidpf stream));

int ZCALLBACK ferror_file_func OF((voidpf opaque, voidpf stream));

voidpf ZCALLBACK fopen_file_func(voidpf opaque, const char* filename, int mode)
{
	FILE* file = nullptr;
	const char* mode_fopen = nullptr;
	if ((mode & int(zlib_file_mode::zlib_filefunc_mode_readwritefilter)) ==
		int(zlib_file_mode::zlib_filefunc_mode_read))
		mode_fopen = "rb";
	else if (mode & int(zlib_file_mode::zlib_filefunc_mode_existing))
		mode_fopen = "r+b";
	else if (mode & int(zlib_file_mode::zlib_filefunc_mode_create))
		mode_fopen = "wb";

	if ((filename != nullptr) && (mode_fopen != nullptr)) {
#if CFG_COMPILER == CFG_COMPILER_MSVC
		fopen_s(&file, filename, mode_fopen);
#else
		file = fopen(filename, mode_fopen); // NOLINT
#endif
	}
	return file;
}

uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	uLong ret; // NOLINT
	ret = (uLong)fread(buf, 1, (size_t)size, (FILE*)stream); // NOLINT
	return ret;
}

uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	uLong ret; // NOLINT
	ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE*)stream); // NOLINT
	return ret;
}

long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream)
{
	long ret; // NOLINT
	ret = ftell((FILE*)stream); // NOLINT
	return ret;
}

long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, long offset, zlib_seek_relative origin)
{
	int fseek_origin = 0;
	long ret; // NOLINT
	switch (origin) {
		case zlib_seek_relative::zlib_filefunc_seek_cur:
			fseek_origin = SEEK_CUR;
			break;
		case zlib_seek_relative::zlib_filefunc_seek_end:
			fseek_origin = SEEK_END;
			break;
		case zlib_seek_relative::zlib_filefunc_seek_set:
			fseek_origin = SEEK_SET;
			break;
		default:
			return -1;
	}
	ret = 0;
	if (fseek((FILE*)stream, offset, fseek_origin) != 0) // NOLINT
		ret = -1;
	return ret;
}

int ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream)
{
	int ret; // NOLINT
	ret = fclose((FILE*)stream); // NOLINT
	return ret;
}

int ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream)
{
	int ret; // NOLINT
	ret = ferror((FILE*)stream); // NOLINT
	return ret;
}

void fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen_file = fopen_file_func;
	pzlib_filefunc_def->zread_file = fread_file_func;
	pzlib_filefunc_def->zwrite_file = fwrite_file_func;
	pzlib_filefunc_def->ztell_file = ftell_file_func;
	pzlib_filefunc_def->zseek_file = fseek_file_func;
	pzlib_filefunc_def->zclose_file = fclose_file_func;
	pzlib_filefunc_def->zerror_file = ferror_file_func;
	pzlib_filefunc_def->opaque = nullptr;
}
