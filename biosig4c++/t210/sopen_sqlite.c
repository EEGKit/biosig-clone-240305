/*

    Copyright (C) 2021 Alois Schloegl <alois.schloegl@gmail.com>

    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/

    BioSig is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */


#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WITH_SQLITE3
#include <sqlite3.h>
#endif
#include "../biosig.h"


#ifdef WITH_SQLITE3
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	for(int i=0; i<argc; i++)
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	printf("\n");
	return 0;
}
#endif

int sopen_sqlite(HDRTYPE* hdr) {
#ifdef WITH_SQLITE3
	sclose(hdr);

	/* refererence to libsqlite3:
		https://www.sqlite.org/cintro.html
	*/
	char *zErrMsg = 0;
	int rc;
	sqlite3 *db;          /* SQLite database rc */
	sqlite3_blob *Blob;

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %d): %s(...)\n", __FILE__,__LINE__,__func__);

	sqlite3_initialize();
	rc = sqlite3_open(hdr->FileName, &db);
	if (rc) {
		biosigERROR(hdr, B4C_CANNOT_OPEN_FILE, "can not open (sqlite) file.");
		sqlite3_close(db);
		return;
	}

	if (VERBOSE_LEVEL > 7) {
		// sqlite3_prepare(), sqlite3_step(), sqlite3_column(), and sqlite3_finalize() 
		rc = sqlite3_exec(db, "SELECT * FROM sqlite_master;", callback, 0, &zErrMsg);
		if( rc != SQLITE_OK ) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}
	/* TODO:
		- Identify whether it is an Cadwell EZDATA file or note
	*/

	sqlite3_close(db);
	sqlite3_shutdown();

	biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format SQLite: not supported yet.");

#else	// WITH_SQLITE3

	biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "SOPEN(SQLite): - sqlite format not supported - libbiosig need to be recompiled with libsqlite3 support.");
#endif	// WITH_SQLITE3
}

