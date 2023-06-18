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

/* unzip.c -- IO for uncompress .zip files using zlib
   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant

   Read unzip.h for more info
*/

/* Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced
in terms of compatibility with older software. The following is from the
original crypt.c. Code woven in by Terry Thorsen 1/2003.
*/
/*
  Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
  crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

  The encryption/decryption parts of this source code (as opposed to the
  non-echoing password parts) were originally written in Europe.  The
  whole source package can be freely distributed, including from the USA.
  (Prior to January 2000, re-export from the US was a violation of US law.)
 */

/*
  This encryption code is a direct transcription of the algorithm from
  Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
  file (appnote.txt) is distributed with the PKZIP program (even in the
  version without encryption capabilities).
 */

#define NOUNCRYPT

#include "unzip.hxx"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// NOLINTBEGIN
#include "zlib.h"
// NOLINTEND

#ifdef STDC
#	include <cstddef>
#	include <cstdlib>
#	include <cstring>
#endif
#ifdef NO_ERRNO_H
extern int errno;
#else
#	include <cerrno>
#endif

#ifndef local
#	define local static // NOLINT
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef CASESENSITIVITYDEFAULT_NO
#	if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES)
#		define CASESENSITIVITYDEFAULT_NO
#	endif
#endif

#ifndef UNZ_BUFSIZE
#	define UNZ_BUFSIZE (16384) // NOLINT
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#	define UNZ_MAXFILENAMEINZIP (256) // NOLINT
#endif

#ifndef ALLOC
#	define ALLOC(size) (malloc(size)) // NOLINT
#endif
#ifndef TRYFREE
// NOLINTNEXTLINE
#	define TRYFREE(p) \
		{ \
			if (p) \
				free(p); \
		}
#endif

#define SIZECENTRALDIRITEM (0x2e) // NOLINT
#define SIZEZIPLOCALHEADER (0x1e) // NOLINT

const char unz_copyright[] = // NOLINT
	" unzip 1.01 Copyright 1998-2004 Gilles Vollant - " // NOLINT
	"http://www.winimage.com/zLibDll"; // NOLINT

// NOLINT
/* unz_file_info_interntal contain internal info about a file in zipfile*/ // NOLINT
typedef struct unz_file_info_internal_s { // NOLINT
	uLong offset_curfile; /* relative offset of local header 4 bytes */ // NOLINT
} unz_file_info_internal; // NOLINT
						  // NOLINT

/* file_in_zip_read_info_s contain internal information about a file in zipfile, // NOLINT
	when reading and decompress it */ // NOLINT
typedef struct { // NOLINT
	char* read_buffer; /* internal buffer for compressed data */ // NOLINT
	z_stream stream; /* zLib stream structure for inflate */ // NOLINT
#ifdef HAVE_BZIP2 // NOLINT
	bz_stream bstream; /* bzLib stream structure for bziped */ // NOLINT
#endif // NOLINT
	// NOLINT
	uLong pos_in_zipfile; /* position in byte on the zipfile, for fseek*/ // NOLINT
	uLong stream_initialised; /* flag set if stream structure is initialised*/ // NOLINT
	// NOLINT
	uLong offset_local_extrafield; /* offset of the local extra field */ // NOLINT
	uInt size_local_extrafield; /* size of the local extra field */ // NOLINT
	uLong pos_local_extrafield; /* position in the local extra field in read*/ // NOLINT
	// NOLINT
	uLong crc32; /* crc32 of all data uncompressed */ // NOLINT
	uLong crc32_wait; /* crc32 we must obtain after decompress all */ // NOLINT
	uLong rest_read_compressed; /* number of byte to be decompressed */ // NOLINT
	uLong rest_read_uncompressed; /*number of byte to be obtained after decomp*/ // NOLINT
	zlib_filefunc_def z_filefunc; // NOLINT
	voidpf filestream; /* io structore of the zipfile */ // NOLINT
	uLong compression_method; /* compression method (0==store) */ // NOLINT
	uLong byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/ // NOLINT
	int raw; // NOLINT
} file_in_zip_read_info_s; // NOLINT
						   // NOLINT

/* unz_s contain internal information about the zipfile // NOLINT
 */ // NOLINT
typedef struct { // NOLINT
	zlib_filefunc_def z_filefunc; // NOLINT
	voidpf filestream; /* io structore of the zipfile */ // NOLINT
	unz_global_info gi; /* public global information */ // NOLINT
	uLong byte_before_the_zipfile; /* byte before the zipfile, (>0 for sfx)*/ // NOLINT
	uLong num_file; /* number of the current file in the zipfile*/ // NOLINT
	uLong pos_in_central_dir; /* pos of the current file in the central dir*/ // NOLINT
	uLong current_file_ok; /* flag about the usability of the current file*/ // NOLINT
	uLong central_pos; /* position of the beginning of the central dir*/ // NOLINT
	// NOLINT
	uLong size_central_dir; /* size of the central directory  */ // NOLINT
	uLong offset_central_dir; /* offset of start of central directory with // NOLINT
								 respect to the starting disk number */ // NOLINT
	// NOLINT
	unz_file_info cur_file_info; /* public info about the current file in zip*/ // NOLINT
	unz_file_info_internal cur_file_info_internal; /* private info about it*/ // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read; /* structure about the current // NOLINT
										file if we are decompressing it */ // NOLINT
	int encrypted; // NOLINT
#ifndef NOUNCRYPT // NOLINT
	unsigned long keys[3]; /* keys defining the pseudo-random sequence */ // NOLINT
	const unsigned long* pcrc_32_tab; // NOLINT
#endif // NOLINT
} unz_s; // NOLINT
		 // NOLINT
#ifndef NOUNCRYPT // NOLINT
#	include "crypt.h" // NOLINT
#endif // NOLINT
	   // NOLINT
/* =========================================================================== // NOLINT
	 Read a byte from a gz_stream; update next_in and avail_in. Return EOF // NOLINT
   for end of file. // NOLINT
   IN assertion: the stream s has been sucessfully opened for reading. // NOLINT
*/ // NOLINT
 // NOLINT
local int unzlocal_getByte OF((const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, int* pi)); // NOLINT
																										  // NOLINT

local int unzlocal_getByte(const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, int* pi) // NOLINT
{ // NOLINT
	unsigned char c; // NOLINT
	int err = (int)ZREAD(*pzlib_filefunc_def, filestream, &c, 1); // NOLINT
	if (err == 1) { // NOLINT
		*pi = (int)c; // NOLINT
		return UNZ_OK; // NOLINT
	} else { // NOLINT
		if (ZERROR(*pzlib_filefunc_def, filestream)) // NOLINT
			return UNZ_ERRNO; // NOLINT
		else // NOLINT
			return UNZ_EOF; // NOLINT
	} // NOLINT
} // NOLINT
  // NOLINT

/* =========================================================================== // NOLINT
   Reads a long in LSB order from the given gz_stream. Sets // NOLINT
*/ // NOLINT
local int unzlocal_getShort OF((const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, uLong* pX)); // NOLINT
																											 // NOLINT

local int unzlocal_getShort(const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) // NOLINT
{ // NOLINT
	uLong x; // NOLINT
	int i = 0; // NOLINT
	int err; // NOLINT
	// NOLINT
	err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x = (uLong)i; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x += ((uLong)i) << 8; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		*pX = x; // NOLINT
	else // NOLINT
		*pX = 0; // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

local int unzlocal_getLong OF((const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, uLong* pX)); // NOLINT
																											// NOLINT

local int unzlocal_getLong(const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) // NOLINT
{ // NOLINT
	uLong x; // NOLINT
	int i = 0; // NOLINT
	int err; // NOLINT
	// NOLINT
	err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x = (uLong)i; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x += ((uLong)i) << 8; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x += ((uLong)i) << 16; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		err = unzlocal_getByte(pzlib_filefunc_def, filestream, &i); // NOLINT
	x += ((uLong)i) << 24; // NOLINT
	// NOLINT
	if (err == UNZ_OK) // NOLINT
		*pX = x; // NOLINT
	else // NOLINT
		*pX = 0; // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* My own strcmpi / strcasecmp */ // NOLINT
local int strcmpcasenosensitive_internal(const char* fileName1, const char* fileName2) // NOLINT
{ // NOLINT
	for (;;) { // NOLINT
		char c1 = *(fileName1++); // NOLINT
		char c2 = *(fileName2++); // NOLINT
		if ((c1 >= 'a') && (c1 <= 'z')) // NOLINT
			c1 -= 0x20; // NOLINT
		if ((c2 >= 'a') && (c2 <= 'z')) // NOLINT
			c2 -= 0x20; // NOLINT
		if (c1 == '\0') // NOLINT
			return ((c2 == '\0') ? 0 : -1); // NOLINT
		if (c2 == '\0') // NOLINT
			return 1; // NOLINT
		if (c1 < c2) // NOLINT
			return -1; // NOLINT
		if (c1 > c2) // NOLINT
			return 1; // NOLINT
	} // NOLINT
} // NOLINT
  // NOLINT
#ifdef CASESENSITIVITYDEFAULT_NO // NOLINT
#	define CASESENSITIVITYDEFAULTVALUE 2 // NOLINT
#else // NOLINT
#	define CASESENSITIVITYDEFAULTVALUE 1 // NOLINT
#endif // NOLINT
  // NOLINT
#ifndef STRCMPCASENOSENTIVEFUNCTION // NOLINT
#	define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal // NOLINT
#endif // NOLINT
  // NOLINT
/* // NOLINT
   Compare two filename (fileName1,fileName2). // NOLINT
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp) // NOLINT
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi // NOLINT
																or strcasecmp) // NOLINT
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system // NOLINT
		(like 1 on Unix, 2 on Windows) // NOLINT
 // NOLINT
*/ // NOLINT
extern int ZEXPORT
unz_string_filename_compare(const char* fileName1, const char* fileName2, int iCaseSensitivity) // NOLINT
{ // NOLINT
	if (iCaseSensitivity == 0) // NOLINT
		iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE; // NOLINT
	// NOLINT
	if (iCaseSensitivity == 1) // NOLINT
		return strcmp(fileName1, fileName2); // NOLINT
	// NOLINT
	return STRCMPCASENOSENTIVEFUNCTION(fileName1, fileName2); // NOLINT
} // NOLINT
  // NOLINT
#ifndef BUFREADCOMMENT // NOLINT
#	define BUFREADCOMMENT (0x400) // NOLINT
#endif // NOLINT
  // NOLINT
/* // NOLINT
  Locate the Central directory of a zipfile (at the end, just before // NOLINT
	the global comment) // NOLINT
*/ // NOLINT
local uLong unzlocal_SearchCentralDir OF((const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream)); // NOLINT
																											// NOLINT

local uLong unzlocal_SearchCentralDir(const zlib_filefunc_def* pzlib_filefunc_def, voidpf filestream) // NOLINT
{ // NOLINT
	unsigned char* buf; // NOLINT
	uLong uSizeFile; // NOLINT
	uLong uBackRead; // NOLINT
	uLong uMaxBack = 0xffff; /* maximum size of global comment */ // NOLINT
	uLong uPosFound = 0; // NOLINT
	// NOLINT
	if (ZSEEK(*pzlib_filefunc_def, filestream, 0, zlib_seek_relative::zlib_filefunc_seek_end) != 0) // NOLINT
		return 0; // NOLINT
	// NOLINT
	uSizeFile = ZTELL(*pzlib_filefunc_def, filestream); // NOLINT
	// NOLINT
	if (uMaxBack > uSizeFile) // NOLINT
		uMaxBack = uSizeFile; // NOLINT
	// NOLINT
	buf = (unsigned char*)ALLOC(BUFREADCOMMENT + 4); // NOLINT
	if (buf == nullptr) // NOLINT
		return 0; // NOLINT
	// NOLINT
	uBackRead = 4; // NOLINT
	while (uBackRead < uMaxBack) { // NOLINT
		uLong uReadSize, uReadPos; // NOLINT
		int i; // NOLINT
		if (uBackRead + BUFREADCOMMENT > uMaxBack) // NOLINT
			uBackRead = uMaxBack; // NOLINT
		else // NOLINT
			uBackRead += BUFREADCOMMENT; // NOLINT
		uReadPos = uSizeFile - uBackRead; // NOLINT
		// NOLINT
		uReadSize =
			((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ? (BUFREADCOMMENT + 4) : (uSizeFile - uReadPos); // NOLINT
		if (ZSEEK(*pzlib_filefunc_def, filestream, uReadPos, zlib_seek_relative::zlib_filefunc_seek_set) != 0) // NOLINT
			break; // NOLINT
		// NOLINT
		if (ZREAD(*pzlib_filefunc_def, filestream, buf, uReadSize) != uReadSize) // NOLINT
			break; // NOLINT
		// NOLINT
		for (i = (int)uReadSize - 3; (i--) > 0;) // NOLINT
			if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) && ((*(buf + i + 2)) == 0x05) // NOLINT
				&& ((*(buf + i + 3)) == 0x06)) // NOLINT
			{ // NOLINT
				uPosFound = uReadPos + i; // NOLINT
				break; // NOLINT
			} // NOLINT
		// NOLINT
		if (uPosFound != 0) // NOLINT
			break; // NOLINT
	} // NOLINT
	TRYFREE(buf); // NOLINT
	return uPosFound; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Open a Zip file. path contain the full pathname (by example, // NOLINT
	 on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer // NOLINT
	 "zlib/zlib114.zip". // NOLINT
	 If the zipfile cannot be opened (file doesn't exist or in not valid), the // NOLINT
	   return value is nullptr. // NOLINT
	 Else, the return value is a unzFile Handle, usable with other function // NOLINT
	   of this unzip package. // NOLINT
*/ // NOLINT
extern unzFile ZEXPORT unz_open_2(const char* path, zlib_filefunc_def* pzlib_filefunc_def) // NOLINT
{ // NOLINT
	unz_s us; // NOLINT
	unz_s* s; // NOLINT
	uLong central_pos, uL; // NOLINT
	// NOLINT
	uLong number_disk; /* number of the current dist, used for // NOLINT
						  spaning ZIP, unsupported, always 0*/ // NOLINT
	uLong number_disk_with_CD; /* number the the disk with central dir, used // NOLINT
								  for spaning ZIP, unsupported, always 0*/ // NOLINT
	uLong number_entry_CD; /* total number of entries in // NOLINT
							  the central dir // NOLINT
							  (same than number_entry on nospan) */ // NOLINT
	// NOLINT
	int err = UNZ_OK; // NOLINT
	// NOLINT
	if (unz_copyright[0] != ' ') // NOLINT
		return nullptr; // NOLINT
	// NOLINT
	if (pzlib_filefunc_def == nullptr) // NOLINT
		fill_fopen_filefunc(&us.z_filefunc); // NOLINT
	else // NOLINT
		us.z_filefunc = *pzlib_filefunc_def; // NOLINT
	// NOLINT
	us.filestream = (*(us.z_filefunc.zopen_file))( // NOLINT
		us.z_filefunc.opaque, // NOLINT
		path, // NOLINT
		int(zlib_file_mode::zlib_filefunc_mode_read) | int(zlib_file_mode::zlib_filefunc_mode_existing) // NOLINT
	); // NOLINT
	if (us.filestream == nullptr) // NOLINT
		return nullptr; // NOLINT
	// NOLINT
	central_pos = unzlocal_SearchCentralDir(&us.z_filefunc, us.filestream); // NOLINT
	if (central_pos == 0) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (ZSEEK(us.z_filefunc, us.filestream, central_pos, zlib_seek_relative::zlib_filefunc_seek_set) != 0) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* the signature, already checked */ // NOLINT
	if (unzlocal_getLong(&us.z_filefunc, us.filestream, &uL) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* number of this disk */ // NOLINT
	if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* number of the disk with the start of the central directory */ // NOLINT
	if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_disk_with_CD) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* total number of entries in the central dir on this disk */ // NOLINT
	if (unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.number_entry) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* total number of entries in the central dir */ // NOLINT
	if (unzlocal_getShort(&us.z_filefunc, us.filestream, &number_entry_CD) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if ((number_entry_CD != us.gi.number_entry) || (number_disk_with_CD != 0) || (number_disk != 0)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	/* size of the central directory */ // NOLINT
	if (unzlocal_getLong(&us.z_filefunc, us.filestream, &us.size_central_dir) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* offset of start of central directory with respect to the // NOLINT
		  starting disk number */ // NOLINT
	if (unzlocal_getLong(&us.z_filefunc, us.filestream, &us.offset_central_dir) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* zipfile comment length */ // NOLINT
	if (unzlocal_getShort(&us.z_filefunc, us.filestream, &us.gi.size_comment) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if ((central_pos < us.offset_central_dir + us.size_central_dir) && (err == UNZ_OK)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if (err != UNZ_OK) { // NOLINT
		ZCLOSE(us.z_filefunc, us.filestream); // NOLINT
		return nullptr; // NOLINT
	} // NOLINT
	// NOLINT
	us.byte_before_the_zipfile = central_pos - (us.offset_central_dir + us.size_central_dir); // NOLINT
	us.central_pos = central_pos; // NOLINT
	us.pfile_in_zip_read = nullptr; // NOLINT
	us.encrypted = 0; // NOLINT
	// NOLINT
	s = (unz_s*)ALLOC(sizeof(unz_s)); // NOLINT
	if (s != nullptr) { // NOLINT
		*s = us; // NOLINT
		unz_go_to_first_file((unzFile)s); // NOLINT
	} // NOLINT
	return (unzFile)s; // NOLINT
} // NOLINT
  // NOLINT

extern unzFile ZEXPORT unz_open(const char* path) // NOLINT
{ // NOLINT
	return unz_open_2(path, nullptr); // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Close a ZipFile opened with unzipOpen. // NOLINT
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see // NOLINT
  later), these files MUST be closed with unzipCloseCurrentFile before call // NOLINT
  unzipClose. return UNZ_OK if there is no problem. */ // NOLINT
extern int ZEXPORT unz_close(unzFile file) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	// NOLINT
	if (s->pfile_in_zip_read != nullptr) // NOLINT
		unz_close_current_file(file); // NOLINT
	// NOLINT
	ZCLOSE(s->z_filefunc, s->filestream); // NOLINT
	TRYFREE(s); // NOLINT
	return UNZ_OK; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Write info about the ZipFile in the *pglobal_info structure. // NOLINT
  No preparation of the structure is needed // NOLINT
  return UNZ_OK if there is no problem. */ // NOLINT
extern int ZEXPORT unz_get_global_info(unzFile file, unz_global_info* pglobal_info) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	*pglobal_info = s->gi; // NOLINT
	return UNZ_OK; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
   Translate date/time from Dos format to tm_unz (readable more easilty) // NOLINT
*/ // NOLINT
local void unzlocal_DosDateToTmuDate(uLong ulDosDate, tm_unz* ptm) // NOLINT
{ // NOLINT
	uLong uDate; // NOLINT
	uDate = (uLong)(ulDosDate >> 16); // NOLINT
	ptm->tm_mday = (uInt)(uDate & 0x1f); // NOLINT
	ptm->tm_mon = (uInt)((((uDate)&0x1E0) / 0x20) - 1); // NOLINT
	ptm->tm_year = (uInt)(((uDate & 0x0FE00) / 0x0200) + 1980); // NOLINT
	// NOLINT
	ptm->tm_hour = (uInt)((ulDosDate & 0xF800) / 0x800); // NOLINT
	ptm->tm_min = (uInt)((ulDosDate & 0x7E0) / 0x20); // NOLINT
	ptm->tm_sec = (uInt)(2 * (ulDosDate & 0x1f)); // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Get Info about the current file in the zipfile, with internal only info // NOLINT
*/ // NOLINT
local int unzlocal_GetCurrentFileInfoInternal // NOLINT
	OF((unzFile file, // NOLINT
		unz_file_info* pfile_info, // NOLINT
		unz_file_info_internal* pfile_info_internal, // NOLINT
		char* szFileName, // NOLINT
		uLong fileNameBufferSize, // NOLINT
		void* extraField, // NOLINT
		uLong extraFieldBufferSize, // NOLINT
		char* szComment, // NOLINT
		uLong commentBufferSize)); // NOLINT

// NOLINT
local int unzlocal_GetCurrentFileInfoInternal( // NOLINT
	unzFile file, // NOLINT
	unz_file_info* pfile_info, // NOLINT
	unz_file_info_internal* pfile_info_internal, // NOLINT
	char* szFileName, // NOLINT
	uLong fileNameBufferSize, // NOLINT
	void* extraField, // NOLINT
	uLong extraFieldBufferSize, // NOLINT
	char* szComment, // NOLINT
	uLong commentBufferSize // NOLINT
) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	unz_file_info file_info; // NOLINT
	unz_file_info_internal file_info_internal; // NOLINT
	int err = UNZ_OK; // NOLINT
	uLong uMagic; // NOLINT
	long lSeek = 0; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	if (ZSEEK( // NOLINT
			s->z_filefunc, // NOLINT
			s->filestream, // NOLINT
			s->pos_in_central_dir + s->byte_before_the_zipfile, // NOLINT
			zlib_seek_relative::zlib_filefunc_seek_set // NOLINT
		) // NOLINT
		!= 0) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	/* we check the magic */ // NOLINT
	if (err == UNZ_OK) { // NOLINT
		if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK) // NOLINT
			err = UNZ_ERRNO; // NOLINT
		else if (uMagic != 0x02014b50) // NOLINT
			err = UNZ_BADZIPFILE; // NOLINT
	} // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.version) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.version_needed) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.flag) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.compression_method) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.dosDate) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	unzlocal_DosDateToTmuDate(file_info.dosDate, &file_info.tmu_date); // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.crc) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.compressed_size) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.uncompressed_size) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_filename) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_file_extra) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.size_file_comment) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.disk_num_start) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &file_info.internal_fa) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info.external_fa) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &file_info_internal.offset_curfile) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	lSeek += file_info.size_filename; // NOLINT
	if ((err == UNZ_OK) && (szFileName != nullptr)) { // NOLINT
		uLong uSizeRead; // NOLINT
		if (file_info.size_filename < fileNameBufferSize) { // NOLINT
			*(szFileName + file_info.size_filename) = '\0'; // NOLINT
			uSizeRead = file_info.size_filename; // NOLINT
		} else // NOLINT
			uSizeRead = fileNameBufferSize; // NOLINT
		// NOLINT
		if ((file_info.size_filename > 0) && (fileNameBufferSize > 0)) // NOLINT
			if (ZREAD(s->z_filefunc, s->filestream, szFileName, uSizeRead) != uSizeRead) // NOLINT
				err = UNZ_ERRNO; // NOLINT
		lSeek -= uSizeRead; // NOLINT
	} // NOLINT
	// NOLINT
	if ((err == UNZ_OK) && (extraField != nullptr)) { // NOLINT
		uLong uSizeRead; // NOLINT
		if (file_info.size_file_extra < extraFieldBufferSize) // NOLINT
			uSizeRead = file_info.size_file_extra; // NOLINT
		else // NOLINT
			uSizeRead = extraFieldBufferSize; // NOLINT
		// NOLINT
		if (lSeek != 0) { // NOLINT
			if (ZSEEK(s->z_filefunc, s->filestream, lSeek, zlib_seek_relative::zlib_filefunc_seek_cur) == 0) // NOLINT
				lSeek = 0; // NOLINT
			else // NOLINT
				err = UNZ_ERRNO; // NOLINT
		} // NOLINT
		// NOLINT
		if ((file_info.size_file_extra > 0) && (extraFieldBufferSize > 0)) // NOLINT
			if (ZREAD(s->z_filefunc, s->filestream, extraField, uSizeRead) != uSizeRead) // NOLINT
				err = UNZ_ERRNO; // NOLINT
		lSeek += file_info.size_file_extra - uSizeRead; // NOLINT
	} else // NOLINT
		lSeek += file_info.size_file_extra; // NOLINT
	// NOLINT
	if ((err == UNZ_OK) && (szComment != nullptr)) { // NOLINT
		uLong uSizeRead; // NOLINT
		if (file_info.size_file_comment < commentBufferSize) { // NOLINT
			*(szComment + file_info.size_file_comment) = '\0'; // NOLINT
			uSizeRead = file_info.size_file_comment; // NOLINT
		} else // NOLINT
			uSizeRead = commentBufferSize; // NOLINT
		// NOLINT
		if (lSeek != 0) { // NOLINT
			if (ZSEEK(s->z_filefunc, s->filestream, lSeek, zlib_seek_relative::zlib_filefunc_seek_cur) == 0) { // NOLINT
				// lSeek=0; // NOLINT
			} else { // NOLINT
				err = UNZ_ERRNO; // NOLINT
			} // NOLINT
		} // NOLINT
		// NOLINT
		if ((file_info.size_file_comment > 0) && (commentBufferSize > 0)) // NOLINT
			if (ZREAD(s->z_filefunc, s->filestream, szComment, uSizeRead) != uSizeRead) // NOLINT
				err = UNZ_ERRNO; // NOLINT
		// lSeek+=file_info.size_file_comment - uSizeRead; // NOLINT
	} else { // NOLINT
		// lSeek+=file_info.size_file_comment; // NOLINT
	} // NOLINT
	// NOLINT
	if ((err == UNZ_OK) && (pfile_info != nullptr)) // NOLINT
		*pfile_info = file_info; // NOLINT
	// NOLINT
	if ((err == UNZ_OK) && (pfile_info_internal != nullptr)) // NOLINT
		*pfile_info_internal = file_info_internal; // NOLINT
	// NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Write info about the ZipFile in the *pglobal_info structure. // NOLINT
  No preparation of the structure is needed // NOLINT
  return UNZ_OK if there is no problem. // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_get_current_file_info( // NOLINT
	unzFile file, // NOLINT
	unz_file_info* pfile_info, // NOLINT
	char* szFileName, // NOLINT
	uLong fileNameBufferSize, // NOLINT
	void* extraField, // NOLINT
	uLong extraFieldBufferSize, // NOLINT
	char* szComment, // NOLINT
	uLong commentBufferSize // NOLINT
) // NOLINT
{ // NOLINT
	return unzlocal_GetCurrentFileInfoInternal( // NOLINT
		file, // NOLINT
		pfile_info, // NOLINT
		nullptr, // NOLINT
		szFileName, // NOLINT
		fileNameBufferSize, // NOLINT
		extraField, // NOLINT
		extraFieldBufferSize, // NOLINT
		szComment, // NOLINT
		commentBufferSize // NOLINT
	); // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Set the current file of the zipfile to the first file. // NOLINT
  return UNZ_OK if there is no problem // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_go_to_first_file(unzFile file) // NOLINT
{ // NOLINT
	int err = UNZ_OK; // NOLINT
	unz_s* s; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	s->pos_in_central_dir = s->offset_central_dir; // NOLINT
	s->num_file = 0; // NOLINT
	err = unzlocal_GetCurrentFileInfoInternal( // NOLINT
		file, // NOLINT
		&s->cur_file_info, // NOLINT
		&s->cur_file_info_internal, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0 // NOLINT
	); // NOLINT
	s->current_file_ok = (err == UNZ_OK); // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Set the current file of the zipfile to the next file. // NOLINT
  return UNZ_OK if there is no problem // NOLINT
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest. // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_go_to_next_file(unzFile file) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	int err; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	if (!s->current_file_ok) // NOLINT
		return UNZ_END_OF_LIST_OF_FILE; // NOLINT
	if (s->gi.number_entry != 0xffff) /* 2^16 files overflow hack */ // NOLINT
		if (s->num_file + 1 == s->gi.number_entry) // NOLINT
			return UNZ_END_OF_LIST_OF_FILE; // NOLINT
	// NOLINT
	s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename
		+ s->cur_file_info.size_file_extra // NOLINT
		+ s->cur_file_info.size_file_comment; // NOLINT
	s->num_file++; // NOLINT
	err = unzlocal_GetCurrentFileInfoInternal( // NOLINT
		file, // NOLINT
		&s->cur_file_info, // NOLINT
		&s->cur_file_info_internal, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0 // NOLINT
	); // NOLINT
	s->current_file_ok = (err == UNZ_OK); // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Try locate the file szFileName in the zipfile. // NOLINT
  For the iCaseSensitivity signification, see unzipStringFileNameCompare // NOLINT
 // NOLINT
  return value : // NOLINT
  UNZ_OK if the file is found. It becomes the current file. // NOLINT
  UNZ_END_OF_LIST_OF_FILE if the file is not found // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_locate_file(unzFile file, const char* szFileName, int iCaseSensitivity) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	int err; // NOLINT
	// NOLINT
	/* We remember the 'current' position in the file so that we can jump // NOLINT
	 * back there if we fail. // NOLINT
	 */ // NOLINT
	unz_file_info cur_file_infoSaved; // NOLINT
	unz_file_info_internal cur_file_info_internalSaved; // NOLINT
	uLong num_fileSaved; // NOLINT
	uLong pos_in_central_dirSaved; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	if (strlen(szFileName) >= UNZ_MAXFILENAMEINZIP) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	s = (unz_s*)file; // NOLINT
	if (!s->current_file_ok) // NOLINT
		return UNZ_END_OF_LIST_OF_FILE; // NOLINT
	// NOLINT
	/* Save the current state */ // NOLINT
	num_fileSaved = s->num_file; // NOLINT
	pos_in_central_dirSaved = s->pos_in_central_dir; // NOLINT
	cur_file_infoSaved = s->cur_file_info; // NOLINT
	cur_file_info_internalSaved = s->cur_file_info_internal; // NOLINT
	// NOLINT
	err = unz_go_to_first_file(file); // NOLINT
	// NOLINT
	while (err == UNZ_OK) { // NOLINT
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP + 1]; // NOLINT
		err = unz_get_current_file_info( // NOLINT
			file, // NOLINT
			nullptr, // NOLINT
			szCurrentFileName, // NOLINT
			sizeof(szCurrentFileName) - 1, // NOLINT
			nullptr, // NOLINT
			0, // NOLINT
			nullptr, // NOLINT
			0 // NOLINT
		); // NOLINT
		if (err == UNZ_OK) { // NOLINT
			if (unz_string_filename_compare(szCurrentFileName, szFileName, iCaseSensitivity) == 0) // NOLINT
				return UNZ_OK; // NOLINT
			err = unz_go_to_next_file(file); // NOLINT
		} // NOLINT
	} // NOLINT
	// NOLINT
	/* We failed, so restore the state of the 'current file' to where we // NOLINT
	 * were. // NOLINT
	 */ // NOLINT
	s->num_file = num_fileSaved; // NOLINT
	s->pos_in_central_dir = pos_in_central_dirSaved; // NOLINT
	s->cur_file_info = cur_file_infoSaved; // NOLINT
	s->cur_file_info_internal = cur_file_info_internalSaved; // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
/////////////////////////////////////////// // NOLINT
// Contributed by Ryan Haksi (mailto://cryogen@infoserve.net) // NOLINT
// I need random access // NOLINT
// // NOLINT
// Further optimization could be realized by adding an ability // NOLINT
// to cache the directory in memory. The goal being a single // NOLINT
// comprehensive file read to put the file I need in a memory. // NOLINT
*/ // NOLINT
 // NOLINT
/* // NOLINT
typedef struct unz_file_pos_s // NOLINT
{ // NOLINT
	uLong pos_in_zip_directory;   // offset in file // NOLINT
	uLong num_of_file;            // # of file // NOLINT
} unz_file_pos; // NOLINT
*/ // NOLINT
 // NOLINT
extern int ZEXPORT unz_get_file_pos(unzFile file, unz_file_pos* file_pos) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	// NOLINT
	if (file == nullptr || file_pos == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	if (!s->current_file_ok) // NOLINT
		return UNZ_END_OF_LIST_OF_FILE; // NOLINT
	// NOLINT
	file_pos->pos_in_zip_directory = s->pos_in_central_dir; // NOLINT
	file_pos->num_of_file = s->num_file; // NOLINT
	// NOLINT
	return UNZ_OK; // NOLINT
} // NOLINT
  // NOLINT

extern int ZEXPORT unz_go_to_file_pos(unzFile file, unz_file_pos* file_pos) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	int err; // NOLINT
	// NOLINT
	if (file == nullptr || file_pos == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	// NOLINT
	/* jump to the right spot */ // NOLINT
	s->pos_in_central_dir = file_pos->pos_in_zip_directory; // NOLINT
	s->num_file = file_pos->num_of_file; // NOLINT
	// NOLINT
	/* set the current file */ // NOLINT
	err = unzlocal_GetCurrentFileInfoInternal( // NOLINT
		file, // NOLINT
		&s->cur_file_info, // NOLINT
		&s->cur_file_info_internal, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0 // NOLINT
	); // NOLINT
	/* return results */ // NOLINT
	s->current_file_ok = (err == UNZ_OK); // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
// Unzip Helper Functions - should be here? // NOLINT
/////////////////////////////////////////// // NOLINT
*/ // NOLINT
 // NOLINT
/* // NOLINT
  Read the local header of the current zipfile // NOLINT
  Check the coherency of the local header and info in the end of central // NOLINT
		directory about this file // NOLINT
  store in *piSizeVar the size of extra info in local header // NOLINT
		(filename and size of extra field data) // NOLINT
*/ // NOLINT
local int unzlocal_CheckCurrentFileCoherencyHeader( // NOLINT
	unz_s* s, // NOLINT
	uInt* piSizeVar, // NOLINT
	uLong* poffset_local_extrafield, // NOLINT
	uInt* psize_local_extrafield // NOLINT
) // NOLINT
{ // NOLINT
	uLong uMagic, uData, uFlags; // NOLINT
	uLong size_filename; // NOLINT
	uLong size_extra_field; // NOLINT
	int err = UNZ_OK; // NOLINT
	// NOLINT
	*piSizeVar = 0; // NOLINT
	*poffset_local_extrafield = 0; // NOLINT
	*psize_local_extrafield = 0; // NOLINT
	// NOLINT
	if (ZSEEK( // NOLINT
			s->z_filefunc, // NOLINT
			s->filestream, // NOLINT
			s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, // NOLINT
			zlib_seek_relative::zlib_filefunc_seek_set // NOLINT
		) // NOLINT
		!= 0) // NOLINT
		return UNZ_ERRNO; // NOLINT
	// NOLINT
	if (err == UNZ_OK) { // NOLINT
		if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK) // NOLINT
			err = UNZ_ERRNO; // NOLINT
		else if (uMagic != 0x04034b50) // NOLINT
			err = UNZ_BADZIPFILE; // NOLINT
	} // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	/* // NOLINT
		else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion)) // NOLINT
			err=UNZ_BADZIPFILE; // NOLINT
	*/ // NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uFlags) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.compression_method)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if ((err == UNZ_OK) && (s->cur_file_info.compression_method != 0) && // NOLINT
		/* #ifdef HAVE_BZIP2 */ // NOLINT
		(s->cur_file_info.compression_method != Z_BZIP2ED) && // NOLINT
		/* #endif */ // NOLINT
		(s->cur_file_info.compression_method != Z_DEFLATED)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* date/time */ // NOLINT
		err = UNZ_ERRNO; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* crc */ // NOLINT
		err = UNZ_ERRNO; // NOLINT
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.crc) && ((uFlags & 8) == 0)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* size compr */ // NOLINT
		err = UNZ_ERRNO; // NOLINT
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.compressed_size) && ((uFlags & 8) == 0)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if (unzlocal_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* size uncompr */ // NOLINT
		err = UNZ_ERRNO; // NOLINT
	else if ((err == UNZ_OK) && (uData != s->cur_file_info.uncompressed_size) && ((uFlags & 8) == 0)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &size_filename) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	else if ((err == UNZ_OK) && (size_filename != s->cur_file_info.size_filename)) // NOLINT
		err = UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	*piSizeVar += (uInt)size_filename; // NOLINT
	// NOLINT
	if (unzlocal_getShort(&s->z_filefunc, s->filestream, &size_extra_field) != UNZ_OK) // NOLINT
		err = UNZ_ERRNO; // NOLINT
	*poffset_local_extrafield = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename; // NOLINT
	*psize_local_extrafield = (uInt)size_extra_field; // NOLINT
	// NOLINT
	*piSizeVar += (uInt)size_extra_field; // NOLINT
	// NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Open for reading data the current file in the zipfile. // NOLINT
  If there is no error and the file is opened, the return value is UNZ_OK. // NOLINT
*/ // NOLINT
extern int ZEXPORT
unz_open_current_file_3(unzFile file, int* method, int* level, int raw, const char* password) // NOLINT
{ // NOLINT
	int err = UNZ_OK; // NOLINT
	uInt iSizeVar; // NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	uLong offset_local_extrafield; /* offset of the local extra field */ // NOLINT
	uInt size_local_extrafield; /* size of the local extra field */ // NOLINT
#ifndef NOUNCRYPT // NOLINT
	char source[12]; // NOLINT
#else // NOLINT
	if (password != nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
#endif // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	if (!s->current_file_ok) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	if (s->pfile_in_zip_read != nullptr) // NOLINT
		unz_close_current_file(file); // NOLINT
	// NOLINT
	if (unzlocal_CheckCurrentFileCoherencyHeader(
			s,
			&iSizeVar,
			&offset_local_extrafield,
			&size_local_extrafield
		) // NOLINT
		!= UNZ_OK) // NOLINT
		return UNZ_BADZIPFILE; // NOLINT
	// NOLINT
	pfile_in_zip_read_info = (file_in_zip_read_info_s*)ALLOC(sizeof(file_in_zip_read_info_s)); // NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_INTERNALERROR; // NOLINT
	// NOLINT
	pfile_in_zip_read_info->read_buffer = (char*)ALLOC(UNZ_BUFSIZE); // NOLINT
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield; // NOLINT
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield; // NOLINT
	pfile_in_zip_read_info->pos_local_extrafield = 0; // NOLINT
	pfile_in_zip_read_info->raw = raw; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info->read_buffer == nullptr) { // NOLINT
		TRYFREE(pfile_in_zip_read_info); // NOLINT
		return UNZ_INTERNALERROR; // NOLINT
	} // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream_initialised = 0; // NOLINT
	// NOLINT
	if (method != nullptr) // NOLINT
		*method = (int)s->cur_file_info.compression_method; // NOLINT
	// NOLINT
	if (level != nullptr) { // NOLINT
		*level = 6; // NOLINT
		switch (s->cur_file_info.flag & 0x06) { // NOLINT
			case 6: // NOLINT
				*level = 1; // NOLINT
				break; // NOLINT
			case 4: // NOLINT
				*level = 2; // NOLINT
				break; // NOLINT
			case 2: // NOLINT
				*level = 9; // NOLINT
				break; // NOLINT
		} // NOLINT
	} // NOLINT
	// NOLINT
	if ((s->cur_file_info.compression_method != 0) && // NOLINT
		/* #ifdef HAVE_BZIP2 */ // NOLINT
		(s->cur_file_info.compression_method != Z_BZIP2ED) && // NOLINT
		/* #endif */ // NOLINT
		(s->cur_file_info.compression_method != Z_DEFLATED)) // NOLINT
	{ // NOLINT
	  // err=UNZ_BADZIPFILE; // NOLINT
	} // NOLINT
	// NOLINT
	pfile_in_zip_read_info->crc32_wait = s->cur_file_info.crc; // NOLINT
	pfile_in_zip_read_info->crc32 = 0; // NOLINT
	pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method; // NOLINT
	pfile_in_zip_read_info->filestream = s->filestream; // NOLINT
	pfile_in_zip_read_info->z_filefunc = s->z_filefunc; // NOLINT
	pfile_in_zip_read_info->byte_before_the_zipfile = s->byte_before_the_zipfile; // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream.total_out = 0; // NOLINT
	// NOLINT
	if ((s->cur_file_info.compression_method == Z_BZIP2ED) && (!raw)) { // NOLINT
#ifdef HAVE_BZIP2 // NOLINT
		pfile_in_zip_read_info->bstream.bzalloc = (void* (*)(void*, int, int))0; // NOLINT
		pfile_in_zip_read_info->bstream.bzfree = (free_func)0; // NOLINT
		pfile_in_zip_read_info->bstream.opaque = (voidpf)0; // NOLINT
		pfile_in_zip_read_info->bstream.state = (voidpf)0; // NOLINT
		// NOLINT
		pfile_in_zip_read_info->stream.zalloc = (alloc_func)0; // NOLINT
		pfile_in_zip_read_info->stream.zfree = (free_func)0; // NOLINT
		pfile_in_zip_read_info->stream.opaque = (voidpf)0; // NOLINT
		pfile_in_zip_read_info->stream.next_in = (voidpf)0; // NOLINT
		pfile_in_zip_read_info->stream.avail_in = 0; // NOLINT
		// NOLINT
		err = BZ2_bzDecompressInit(&pfile_in_zip_read_info->bstream, 0, 0); // NOLINT
		if (err == Z_OK) // NOLINT
			pfile_in_zip_read_info->stream_initialised = Z_BZIP2ED; // NOLINT
		else { // NOLINT
			TRYFREE(pfile_in_zip_read_info); // NOLINT
			return err; // NOLINT
		} // NOLINT
#else // NOLINT
		pfile_in_zip_read_info->raw = 1; // NOLINT
#endif // NOLINT
	} else if ((s->cur_file_info.compression_method == Z_DEFLATED) && (!raw)) { // NOLINT
		pfile_in_zip_read_info->stream.zalloc = (alloc_func)0; // NOLINT
		pfile_in_zip_read_info->stream.zfree = (free_func)0; // NOLINT
		pfile_in_zip_read_info->stream.opaque = (voidpf)0; // NOLINT
		pfile_in_zip_read_info->stream.next_in = (Bytef*)0; // NOLINT
		pfile_in_zip_read_info->stream.avail_in = 0; // NOLINT
		// NOLINT
		err = inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS); // NOLINT
		if (err == Z_OK) // NOLINT
			pfile_in_zip_read_info->stream_initialised = Z_DEFLATED; // NOLINT
		else { // NOLINT
			TRYFREE(pfile_in_zip_read_info); // NOLINT
			return err; // NOLINT
		} // NOLINT
		/* windowBits is passed < 0 to tell that there is no zlib header. // NOLINT
		 * Note that in this case inflate *requires* an extra "dummy" byte // NOLINT
		 * after the compressed stream in order to complete decompression and // NOLINT
		 * return Z_STREAM_END. // NOLINT
		 * In unzip, i don't wait absolutely Z_STREAM_END because I known the // NOLINT
		 * size of both compressed and uncompressed data // NOLINT
		 */ // NOLINT
	} // NOLINT
	pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size; // NOLINT
	pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size; // NOLINT
	// NOLINT
	pfile_in_zip_read_info->pos_in_zipfile =
		s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar; // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream.avail_in = (uInt)0; // NOLINT
	// NOLINT
	s->pfile_in_zip_read = pfile_in_zip_read_info; // NOLINT
	// NOLINT
	s->encrypted = 0; // NOLINT
	// NOLINT
#ifndef NOUNCRYPT // NOLINT
	if (password != nullptr) { // NOLINT
		int i; // NOLINT
		s->pcrc_32_tab = get_crc_table(); // NOLINT
		init_keys(password, s->keys, s->pcrc_32_tab); // NOLINT
		if (ZSEEK( // NOLINT
				s->z_filefunc, // NOLINT
				s->filestream, // NOLINT
				s->pfile_in_zip_read->pos_in_zipfile + s->pfile_in_zip_read->byte_before_the_zipfile, // NOLINT
				SEEK_SET // NOLINT
			) // NOLINT
			!= 0) // NOLINT
			return UNZ_INTERNALERROR; // NOLINT
		if (ZREAD(s->z_filefunc, s->filestream, source, 12) < 12) // NOLINT
			return UNZ_INTERNALERROR; // NOLINT
		// NOLINT
		for (i = 0; i < 12; i++) // NOLINT
			zdecode(s->keys, s->pcrc_32_tab, source[i]); // NOLINT
		// NOLINT
		s->pfile_in_zip_read->pos_in_zipfile += 12; // NOLINT
		s->encrypted = 1; // NOLINT
	} // NOLINT
#endif // NOLINT
	// NOLINT
	return UNZ_OK; // NOLINT
} // NOLINT
  // NOLINT

extern int ZEXPORT unz_open_current_file(unzFile file) // NOLINT
{ // NOLINT
	return unz_open_current_file_3(file, nullptr, nullptr, 0, nullptr); // NOLINT
} // NOLINT
  // NOLINT

extern int ZEXPORT unz_open_current_file_password(unzFile file, const char* password) // NOLINT
{ // NOLINT
	return unz_open_current_file_3(file, nullptr, nullptr, 0, password); // NOLINT
} // NOLINT
  // NOLINT

extern int ZEXPORT unz_open_current_file_2(unzFile file, int* method, int* level, int raw) // NOLINT
{ // NOLINT
	return unz_open_current_file_3(file, method, level, raw, nullptr); // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Read bytes from the current file. // NOLINT
  buf contain buffer where data must be copied // NOLINT
  len the size of buf. // NOLINT
 // NOLINT
  return the number of byte copied if somes bytes are copied // NOLINT
  return 0 if the end of file was reached // NOLINT
  return <0 with error code if there is an error // NOLINT
	(UNZ_ERRNO for IO error, or zLib error for uncompress error) // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_read_current_file(unzFile file, voidp buf, unsigned len) // NOLINT
{ // NOLINT
	int err = UNZ_OK; // NOLINT
	uInt iRead = 0; // NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	pfile_in_zip_read_info = s->pfile_in_zip_read; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info->read_buffer == nullptr) { // NOLINT
		return UNZ_END_OF_LIST_OF_FILE; // NOLINT
	} // NOLINT
	// NOLINT
	if (len == 0) { // NOLINT
		return 0; // NOLINT
	} // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream.next_out = (Bytef*)buf; // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream.avail_out = (uInt)len; // NOLINT
	// NOLINT
	if ((len > pfile_in_zip_read_info->rest_read_uncompressed) && (!(pfile_in_zip_read_info->raw))) // NOLINT
		pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed; // NOLINT
	// NOLINT
	if ((len > pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in) // NOLINT
		&& (pfile_in_zip_read_info->raw)) // NOLINT
		pfile_in_zip_read_info->stream.avail_out = // NOLINT
			(uInt)pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in; // NOLINT
	// NOLINT
	while (pfile_in_zip_read_info->stream.avail_out > 0) { // NOLINT
		if ((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed > 0))
		{ // NOLINT
			uInt uReadThis = UNZ_BUFSIZE; // NOLINT
			if (pfile_in_zip_read_info->rest_read_compressed < uReadThis) // NOLINT
				uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed; // NOLINT
			if (uReadThis == 0) // NOLINT
				return UNZ_EOF; // NOLINT
			if (ZSEEK( // NOLINT
					pfile_in_zip_read_info->z_filefunc, // NOLINT
					pfile_in_zip_read_info->filestream, // NOLINT
					pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile, // NOLINT
					zlib_seek_relative::zlib_filefunc_seek_set // NOLINT
				) // NOLINT
				!= 0) // NOLINT
				return UNZ_ERRNO; // NOLINT
			if (ZREAD( // NOLINT
					pfile_in_zip_read_info->z_filefunc, // NOLINT
					pfile_in_zip_read_info->filestream, // NOLINT
					pfile_in_zip_read_info->read_buffer, // NOLINT
					uReadThis // NOLINT
				) // NOLINT
				!= uReadThis) // NOLINT
				return UNZ_ERRNO; // NOLINT
				// NOLINT
#ifndef NOUNCRYPT // NOLINT
			if (s->encrypted) { // NOLINT
				uInt i; // NOLINT
				for (i = 0; i < uReadThis; i++) // NOLINT
					pfile_in_zip_read_info->read_buffer[i] = // NOLINT
						zdecode(s->keys, s->pcrc_32_tab, pfile_in_zip_read_info->read_buffer[i]); // NOLINT
			} // NOLINT
#endif // NOLINT
	   // NOLINT
			pfile_in_zip_read_info->pos_in_zipfile += uReadThis; // NOLINT
			// NOLINT
			pfile_in_zip_read_info->rest_read_compressed -= uReadThis; // NOLINT
			// NOLINT
			pfile_in_zip_read_info->stream.next_in = (Bytef*)pfile_in_zip_read_info->read_buffer; // NOLINT
			pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis; // NOLINT
		} // NOLINT
		// NOLINT
		if ((pfile_in_zip_read_info->compression_method == 0) || (pfile_in_zip_read_info->raw)) { // NOLINT
			uInt uDoCopy, i; // NOLINT
			// NOLINT
			if ((pfile_in_zip_read_info->stream.avail_in == 0)
				&& (pfile_in_zip_read_info->rest_read_compressed == 0)) // NOLINT
				return (iRead == 0) ? UNZ_EOF : iRead; // NOLINT
			// NOLINT
			if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in) // NOLINT
				uDoCopy = pfile_in_zip_read_info->stream.avail_out; // NOLINT
			else // NOLINT
				uDoCopy = pfile_in_zip_read_info->stream.avail_in; // NOLINT
			// NOLINT
			for (i = 0; i < uDoCopy; i++) // NOLINT
				*(pfile_in_zip_read_info->stream.next_out + i) = // NOLINT
					*(pfile_in_zip_read_info->stream.next_in + i); // NOLINT
			// NOLINT
			pfile_in_zip_read_info->crc32 = // NOLINT
				crc32(pfile_in_zip_read_info->crc32, pfile_in_zip_read_info->stream.next_out, uDoCopy); // NOLINT
			pfile_in_zip_read_info->rest_read_uncompressed -= uDoCopy; // NOLINT
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy; // NOLINT
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy; // NOLINT
			pfile_in_zip_read_info->stream.next_out += uDoCopy; // NOLINT
			pfile_in_zip_read_info->stream.next_in += uDoCopy; // NOLINT
			pfile_in_zip_read_info->stream.total_out += uDoCopy; // NOLINT
			iRead += uDoCopy; // NOLINT
		} else if (pfile_in_zip_read_info->compression_method == Z_BZIP2ED) { // NOLINT
#ifdef HAVE_BZIP2 // NOLINT
			uLong uTotalOutBefore, uTotalOutAfter; // NOLINT
			const Bytef* bufBefore; // NOLINT
			uLong uOutThis; // NOLINT
			// NOLINT
			pfile_in_zip_read_info->bstream.next_in = pfile_in_zip_read_info->stream.next_in; // NOLINT
			pfile_in_zip_read_info->bstream.avail_in = pfile_in_zip_read_info->stream.avail_in; // NOLINT
			pfile_in_zip_read_info->bstream.total_in_lo32 = pfile_in_zip_read_info->stream.total_in; // NOLINT
			pfile_in_zip_read_info->bstream.total_in_hi32 = 0; // NOLINT
			pfile_in_zip_read_info->bstream.next_out = pfile_in_zip_read_info->stream.next_out; // NOLINT
			pfile_in_zip_read_info->bstream.avail_out = pfile_in_zip_read_info->stream.avail_out; // NOLINT
			pfile_in_zip_read_info->bstream.total_out_lo32 = pfile_in_zip_read_info->stream.total_out; // NOLINT
			pfile_in_zip_read_info->bstream.total_out_hi32 = 0; // NOLINT
			// NOLINT
			uTotalOutBefore = pfile_in_zip_read_info->bstream.total_out_lo32; // NOLINT
			bufBefore = pfile_in_zip_read_info->bstream.next_out; // NOLINT
			// NOLINT
			err = BZ2_bzDecompress(&pfile_in_zip_read_info->bstream); // NOLINT
			// NOLINT
			uTotalOutAfter = pfile_in_zip_read_info->bstream.total_out_lo32; // NOLINT
			uOutThis = uTotalOutAfter - uTotalOutBefore; // NOLINT
			// NOLINT
			pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, bufBefore, (uInt)(uOutThis)); // NOLINT
			// NOLINT
			pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis; // NOLINT
			// NOLINT
			iRead += (uInt)(uTotalOutAfter - uTotalOutBefore); // NOLINT
			// NOLINT
			pfile_in_zip_read_info->stream.next_in = pfile_in_zip_read_info->bstream.next_in; // NOLINT
			pfile_in_zip_read_info->stream.avail_in = pfile_in_zip_read_info->bstream.avail_in; // NOLINT
			pfile_in_zip_read_info->stream.total_in = pfile_in_zip_read_info->bstream.total_in_lo32; // NOLINT
			pfile_in_zip_read_info->stream.next_out = pfile_in_zip_read_info->bstream.next_out; // NOLINT
			pfile_in_zip_read_info->stream.avail_out = pfile_in_zip_read_info->bstream.avail_out; // NOLINT
			pfile_in_zip_read_info->stream.total_out = pfile_in_zip_read_info->bstream.total_out_lo32; // NOLINT
			// NOLINT
			if (err == BZ_STREAM_END) // NOLINT
				return (iRead == 0) ? UNZ_EOF : iRead; // NOLINT
			if (err != BZ_OK) // NOLINT
				break; // NOLINT
#endif // NOLINT
		} else { // NOLINT
			uLong uTotalOutBefore, uTotalOutAfter; // NOLINT
			const Bytef* bufBefore; // NOLINT
			uLong uOutThis; // NOLINT
			int flush = Z_SYNC_FLUSH; // NOLINT
			// NOLINT
			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out; // NOLINT
			bufBefore = pfile_in_zip_read_info->stream.next_out; // NOLINT
			// NOLINT
			/* // NOLINT
			if ((pfile_in_zip_read_info->rest_read_uncompressed == // NOLINT
					 pfile_in_zip_read_info->stream.avail_out) && // NOLINT
				(pfile_in_zip_read_info->rest_read_compressed == 0)) // NOLINT
				flush = Z_FINISH; // NOLINT
			*/ // NOLINT
			err = inflate(&pfile_in_zip_read_info->stream, flush); // NOLINT
			// NOLINT
			if ((err >= 0) && (pfile_in_zip_read_info->stream.msg != nullptr)) // NOLINT
				err = Z_DATA_ERROR; // NOLINT
			// NOLINT
			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out; // NOLINT
			uOutThis = uTotalOutAfter - uTotalOutBefore; // NOLINT
			// NOLINT
			pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, bufBefore, (uInt)(uOutThis)); // NOLINT
			// NOLINT
			pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis; // NOLINT
			// NOLINT
			iRead += (uInt)(uTotalOutAfter - uTotalOutBefore); // NOLINT
			// NOLINT
			if (err == Z_STREAM_END) // NOLINT
				return (iRead == 0) ? UNZ_EOF : iRead; // NOLINT
			if (err != Z_OK) // NOLINT
				break; // NOLINT
		} // NOLINT
	} // NOLINT
	// NOLINT
	if (err == Z_OK) // NOLINT
		return iRead; // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Give the current position in uncompressed data // NOLINT
*/ // NOLINT
extern z_off_t ZEXPORT unztell(unzFile file) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	pfile_in_zip_read_info = s->pfile_in_zip_read; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	return (z_off_t)pfile_in_zip_read_info->stream.total_out; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  return 1 if the end of file was reached, 0 elsewhere // NOLINT
*/ // NOLINT
extern int ZEXPORT unzeof(unzFile file) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	pfile_in_zip_read_info = s->pfile_in_zip_read; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info->rest_read_uncompressed == 0) // NOLINT
		return 1; // NOLINT
	else // NOLINT
		return 0; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Read extra field from the current file (opened by unz_open_current_file) // NOLINT
  This is the local-header version of the extra field (sometimes, there is // NOLINT
	more info in the local-header version than in the central-header) // NOLINT
 // NOLINT
  if buf==nullptr, it return the size of the local extra field that can be read // NOLINT
 // NOLINT
  if buf!=nullptr, len is the size of the buffer, the extra header is copied in // NOLINT
	buf. // NOLINT
  the return value is the number of bytes copied in buf, or (if <0) // NOLINT
	the error code // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_get_local_extra_field(unzFile file, voidp buf, unsigned len) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	uInt read_now; // NOLINT
	uLong size_to_read; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	pfile_in_zip_read_info = s->pfile_in_zip_read; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	size_to_read =
		(pfile_in_zip_read_info->size_local_extrafield - pfile_in_zip_read_info->pos_local_extrafield); // NOLINT
	// NOLINT
	if (buf == nullptr) // NOLINT
		return (int)size_to_read; // NOLINT
	// NOLINT
	if (len > size_to_read) // NOLINT
		read_now = (uInt)size_to_read; // NOLINT
	else // NOLINT
		read_now = (uInt)len; // NOLINT
	// NOLINT
	if (read_now == 0) // NOLINT
		return 0; // NOLINT
	// NOLINT
	if (ZSEEK( // NOLINT
			pfile_in_zip_read_info->z_filefunc, // NOLINT
			pfile_in_zip_read_info->filestream, // NOLINT
			pfile_in_zip_read_info->offset_local_extrafield + pfile_in_zip_read_info->pos_local_extrafield, // NOLINT
			zlib_seek_relative::zlib_filefunc_seek_set // NOLINT
		) // NOLINT
		!= 0) // NOLINT
		return UNZ_ERRNO; // NOLINT
	// NOLINT
	if (ZREAD(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, buf, read_now)
		!= read_now) // NOLINT
		return UNZ_ERRNO; // NOLINT
	// NOLINT
	return (int)read_now; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Close the file in zip opened with unzipOpenCurrentFile // NOLINT
  Return UNZ_CRCERROR if all the file was read but the CRC is not good // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_close_current_file(unzFile file) // NOLINT
{ // NOLINT
	int err = UNZ_OK; // NOLINT
	// NOLINT
	unz_s* s; // NOLINT
	file_in_zip_read_info_s* pfile_in_zip_read_info; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	pfile_in_zip_read_info = s->pfile_in_zip_read; // NOLINT
	// NOLINT
	if (pfile_in_zip_read_info == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	// NOLINT
	if ((pfile_in_zip_read_info->rest_read_uncompressed == 0) && (!pfile_in_zip_read_info->raw)) { // NOLINT
		if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait) // NOLINT
			err = UNZ_CRCERROR; // NOLINT
	} // NOLINT
	// NOLINT
	TRYFREE(pfile_in_zip_read_info->read_buffer); // NOLINT
	pfile_in_zip_read_info->read_buffer = nullptr; // NOLINT
	if (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED) // NOLINT
		inflateEnd(&pfile_in_zip_read_info->stream); // NOLINT
#ifdef HAVE_BZIP2 // NOLINT
	else if (pfile_in_zip_read_info->stream_initialised == Z_BZIP2ED) // NOLINT
		BZ2_bzDecompressEnd(&pfile_in_zip_read_info->bstream); // NOLINT
#endif // NOLINT
	// NOLINT
	pfile_in_zip_read_info->stream_initialised = 0; // NOLINT
	TRYFREE(pfile_in_zip_read_info); // NOLINT
	// NOLINT
	s->pfile_in_zip_read = nullptr; // NOLINT
	// NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT

/* // NOLINT
  Get the global comment string of the ZipFile, in the szComment buffer. // NOLINT
  uSizeBuf is the size of the szComment buffer. // NOLINT
  return the number of byte copied or an error code <0 // NOLINT
*/ // NOLINT
extern int ZEXPORT unz_get_global_comment(unzFile file, char* szComment, uLong uSizeBuf) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	uLong uReadThis; // NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	// NOLINT
	uReadThis = uSizeBuf; // NOLINT
	if (uReadThis > s->gi.size_comment) // NOLINT
		uReadThis = s->gi.size_comment; // NOLINT
	// NOLINT
	if (ZSEEK(s->z_filefunc, s->filestream, s->central_pos + 22, zlib_seek_relative::zlib_filefunc_seek_set)
		!= 0) // NOLINT
		return UNZ_ERRNO; // NOLINT
	// NOLINT
	if (uReadThis > 0) { // NOLINT
		*szComment = '\0'; // NOLINT
		if (ZREAD(s->z_filefunc, s->filestream, szComment, uReadThis) != uReadThis) // NOLINT
			return UNZ_ERRNO; // NOLINT
	} // NOLINT
	// NOLINT
	if ((szComment != nullptr) && (uSizeBuf > s->gi.size_comment)) // NOLINT
		*(szComment + s->gi.size_comment) = '\0'; // NOLINT
	return (int)uReadThis; // NOLINT
} // NOLINT
  // NOLINT

/* Additions by RX '2004 */ // NOLINT
extern uLong ZEXPORT unz_get_offset(unzFile file) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return 0; // NOLINT
	s = (unz_s*)file; // NOLINT
	if (!s->current_file_ok) // NOLINT
		return 0; // NOLINT
	if (s->gi.number_entry != 0 && s->gi.number_entry != 0xffff) // NOLINT
		if (s->num_file == s->gi.number_entry) // NOLINT
			return 0; // NOLINT
	return s->pos_in_central_dir; // NOLINT
} // NOLINT
  // NOLINT

extern int ZEXPORT unz_set_offset(unzFile file, uLong pos) // NOLINT
{ // NOLINT
	unz_s* s; // NOLINT
	int err; // NOLINT
	// NOLINT
	if (file == nullptr) // NOLINT
		return UNZ_PARAMERROR; // NOLINT
	s = (unz_s*)file; // NOLINT
	// NOLINT
	s->pos_in_central_dir = pos; // NOLINT
	s->num_file = s->gi.number_entry; /* hack */ // NOLINT
	err = unzlocal_GetCurrentFileInfoInternal( // NOLINT
		file, // NOLINT
		&s->cur_file_info, // NOLINT
		&s->cur_file_info_internal, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0, // NOLINT
		nullptr, // NOLINT
		0 // NOLINT
	); // NOLINT
	s->current_file_ok = (err == UNZ_OK); // NOLINT
	return err; // NOLINT
} // NOLINT
  // NOLINT