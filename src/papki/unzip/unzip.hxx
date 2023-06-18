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

/* unzip.h -- IO for uncompress .zip files using zlib
   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant

   This unzip package allow extract file from .ZIP file, compatible with
  PKZip 2.04g WinZip, InfoZip tools and compatible.

   Multi volume ZipFile (span) are not supported.
   Encryption compatible with pkzip 2.04g only supported
   Old compressions used by old PKZip 1.x are not supported


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


*/

/* for more info about .ZIP format, see
	  http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
	  http://www.info-zip.org/pub/infozip/doc/
   PkWare has also a specification at :
	  ftp://ftp.pkware.com/probdesc.zip
*/

#ifndef unzip_HXX
#define unzip_HXX

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTBEGIN
#ifndef _ZLIB_H
#	include "zlib.h"
#endif
// NOLINTEND

#ifndef _ZLIBIOAPI_H
#	include "ioapi.hxx"
#endif

#ifdef HAVE_BZIP2
#	include "bzlib.h"
#endif

#define Z_BZIP2ED 12 // NOLINT

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
	from (void*) without cast */
typedef struct TagunzFile__ {
	int unused;
} unzFile__;

typedef unzFile__* unzFile;
#else
using unzFile = voidp;
#endif

#define UNZ_OK (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO (Z_ERRNO)
#define UNZ_EOF (0)
#define UNZ_PARAMERROR (-102)
#define UNZ_BADZIPFILE (-103)
#define UNZ_INTERNALERROR (-104)
#define UNZ_CRCERROR (-105)

/* tm_unz contain date/time info */
using tm_unz = struct tm_unz_s {
	uInt tm_sec; /* seconds after the minute - [0,59] */
	uInt tm_min; /* minutes after the hour - [0,59] */
	uInt tm_hour; /* hours since midnight - [0,23] */
	uInt tm_mday; /* day of the month - [1,31] */
	uInt tm_mon; /* months since January - [0,11] */
	uInt tm_year; /* years - [1980..2044] */
};

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
using unz_global_info = struct unz_global_info_s {
	uLong number_entry; /* total number of entries in
			   the central dir on this disk */
	uLong size_comment; /* size of the global comment of the zipfile */
};

/* unz_file_info contain information about a file in the zipfile */
using unz_file_info = struct unz_file_info_s {
	uLong version; /* version made by                 2 bytes */
	uLong version_needed; /* version needed to extract       2 bytes */
	uLong flag; /* general purpose bit flag        2 bytes */
	uLong compression_method; /* compression method              2 bytes */
	uLong dosDate; /* last mod file date in Dos fmt   4 bytes */
	uLong crc; /* crc-32                          4 bytes */
	uLong compressed_size; /* compressed size                 4 bytes */
	uLong uncompressed_size; /* uncompressed size               4 bytes */
	uLong size_filename; /* filename length                 2 bytes */
	uLong size_file_extra; /* extra field length              2 bytes */
	uLong size_file_comment; /* file comment length             2 bytes */

	uLong disk_num_start; /* disk number start               2 bytes */
	uLong internal_fa; /* internal file attributes        2 bytes */
	uLong external_fa; /* external file attributes        4 bytes */

	tm_unz tmu_date;
};

extern int ZEXPORT unz_string_filename_compare OF((const char* fileName1, const char* fileName2, int iCaseSensitivity));
/*
   Compare two filename (fileName1,fileName2).
   If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
   If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
								or strcasecmp)
   If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
	(like 1 on Unix, 2 on Windows)
*/

extern unzFile ZEXPORT unz_open OF((const char* path));
/*
  Open a Zip file. path contain the full pathname (by example,
	 on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
	 "zlib/zlib113.zip".
	 If the zipfile cannot be opened (file don't exist or in not valid), the
	   return value is NULL.
	 Else, the return value is a unzFile Handle, usable with other function
	   of this unzip package.
*/

extern unzFile ZEXPORT unz_open_2 OF((const char* path, zlib_filefunc_def* pzlib_filefunc_def));
/*
   Open a Zip file, like unz_open, but provide a set of file low level API
	  for read/write the zip file (see ioapi.h)
*/

extern int ZEXPORT unz_close OF((unzFile file));
/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unz_open_current_file (see later),
	these files MUST be closed with unzipCloseCurrentFile before call
  unzipClose. return UNZ_OK if there is no problem. */

extern int ZEXPORT unz_get_global_info OF((unzFile file, unz_global_info* pglobal_info));
/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */

extern int ZEXPORT unz_get_global_comment OF((unzFile file, char* szComment, uLong uSizeBuf));
/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/

/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

extern int ZEXPORT unz_go_to_first_file OF((unzFile file));
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

extern int ZEXPORT unz_go_to_next_file OF((unzFile file));
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

extern int ZEXPORT unz_locate_file OF((unzFile file, const char* szFileName, int iCaseSensitivity));

/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unz_string_filename_compare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/

/* ****************************************** */
/* Ryan supplied functions */
/* unz_file_info contain information about a file in the zipfile */
using unz_file_pos = struct unz_file_pos_s {
	uLong pos_in_zip_directory; /* offset in zip file directory */
	uLong num_of_file; /* # of file */
};

extern int ZEXPORT unz_get_file_pos(unzFile file, unz_file_pos* file_pos);

extern int ZEXPORT unz_go_to_file_pos(unzFile file, unz_file_pos* file_pos);

/* ****************************************** */

extern int ZEXPORT unz_get_current_file_info
	OF((unzFile file,
		unz_file_info* pfile_info,
		char* szFileName,
		uLong fileNameBufferSize,
		void* extraField,
		uLong extraFieldBufferSize,
		char* szComment,
		uLong commentBufferSize));
/*
  Get Info about the current file
  if pfile_info!=NULL, the *pfile_info structure will contain somes info about
		the current file
  if szFileName!=NULL, the filemane string will be copied in szFileName
			(fileNameBufferSize is the size of the buffer)
  if extraField!=NULL, the extra field information will be copied in extraField
			(extraFieldBufferSize is the size of the buffer).
			This is the Central-header version of the extra field
  if szComment!=NULL, the comment string of the file will be copied in szComment
			(commentBufferSize is the size of the buffer)
*/

/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

extern int ZEXPORT unz_open_current_file OF((unzFile file));
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

extern int ZEXPORT unz_open_current_file_password OF((unzFile file, const char* password));
/*
  Open for reading data the current file in the zipfile.
  password is a crypting password
  If there is no error, the return value is UNZ_OK.
*/

extern int ZEXPORT unz_open_current_file_2 OF((unzFile file, int* method, int* level, int raw));
/*
  Same than unz_open_current_file, but open for read raw the file (not uncompress)
	if raw==1
  *method will receive method of compression, *level will receive level of
	 compression
  note : you can set level parameter as NULL (if you did not want known level,
		 but you CANNOT set method parameter as NULL
*/

extern int ZEXPORT unz_open_current_file_3 OF((unzFile file, int* method, int* level, int raw, const char* password));
/*
  Same than unz_open_current_file, but open for read raw the file (not uncompress)
	if raw==1
  *method will receive method of compression, *level will receive level of
	 compression
  note : you can set level parameter as NULL (if you did not want known level,
		 but you CANNOT set method parameter as NULL
*/

extern int ZEXPORT unz_close_current_file OF((unzFile file));
/*
  Close the file in zip opened with unz_open_current_file
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/

extern int ZEXPORT unz_read_current_file OF((unzFile file, voidp buf, unsigned len));
/*
  Read bytes from the current file (opened by unz_open_current_file)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
	(UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

extern z_off_t ZEXPORT unztell OF((unzFile file));
/*
  Give the current position in uncompressed data
*/

extern int ZEXPORT unzeof OF((unzFile file));
/*
  return 1 if the end of file was reached, 0 elsewhere
*/

extern int ZEXPORT unz_get_local_extra_field OF((unzFile file, voidp buf, unsigned len));
/*
  Read extra field from the current file (opened by unz_open_current_file)
  This is the local-header version of the extra field (sometimes, there is
	more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
	buf.
  the return value is the number of bytes copied in buf, or (if <0)
	the error code
*/

/***************************************************************************/

/* Get the current file offset */
extern uLong ZEXPORT unz_get_offset(unzFile file);

/* Set the current file offset */
extern int ZEXPORT unz_set_offset(unzFile file, uLong pos);

#ifdef __cplusplus
}
#endif

#endif /* _unz_H */
