/*
    Copyright (C) 2016,2019 Alois Schloegl <alois.schloegl@gmail.com>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++/libbiosig) at http://biosig.sf.net/

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

References:
    https://cran.r-project.org/doc/manuals/R-exts.html

 */

#include <R.h>
#include <Rdefines.h>
#include <R_ext/Rdynload.h>

#include <biosig.h>

SEXP sload(SEXP filename, SEXP channels) {

	// sanity check of input
	if(!isString(filename) || length(filename) != 1)
		error("filename is not a single string");

	// Open file
	HDRTYPE *hdr = sopen(CHAR(asChar(filename)), "r", NULL);
	if (serror2(hdr)) return R_NilValue;

	long NS = biosig_get_number_of_channels(hdr);
	size_t SPR = biosig_get_number_of_samples(hdr);

	// allocate memory for results
	SEXP result = PROTECT(allocMatrix(REALSXP, SPR, NS));

	// read data from file and write into result
	int status = sread(REAL(result), 0, SPR, hdr);
	if (serror2(hdr)) {
		destructHDR(hdr);
		Free(result);
		UNPROTECT(1);
		return R_NilValue;
	}

	// close file, and cleanup memory to avoid any leaks
	destructHDR(hdr);
	UNPROTECT(1);
	return result;
}

SEXP jsonHeader (SEXP filename) {
	// sanity check of input
	if(!isString(filename) || length(filename) != 1)
		error("filename is not a single string");

	// Open file
	HDRTYPE *hdr = sopen(CHAR(asChar(filename)), "r", NULL);
	if (serror2(hdr)) return R_NilValue;

	// convert header to json-string
	SEXP result    = R_NilValue;
	char *str      = NULL;
	asprintf_hdr2json(&str, hdr);
	if (str != NULL) {
		result = PROTECT(allocVector(STRSXP, strlen(str)+1));
		SET_STRING_ELT(result, 0, mkChar(str));
		free(str);
	}

	// close file, and cleanup memory to avoid any leaks
	destructHDR(hdr);
	UNPROTECT(1);
	return result;
}
