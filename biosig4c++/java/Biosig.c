/*

    Copyright (C) 2018,2019 Alois Schloegl <alois.schloegl@gmail.com>
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


#include <stdio.h>
#include <biosig.h>
#include "Biosig.h"
/* Header for class Biosig */

#ifndef _Included_biosig
#define _Included_biosig
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     Biosig
 * Method:    hdr2ascii
 * Signature: ()V
 */
JNIEXPORT jstring JNICALL Java_Biosig_hdr2ascii (JNIEnv *env, jobject, jstring filename) {
     HDR_STRUCT *hdr = sopen((const char*)env->GetStringUTFChars(filename, 0),"r", NULL);
     char *json=NULL;
     asprintf_hdr2json(&json,hdr);
     sclose(hdr);
     destructHDR(hdr);
     printf("hdr2ascii: Hello World! \n%s\n", json);
     return env->NewStringUTF(json);
}

/*
 * Class:     Biosig
 * Method:    isTTY
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_Biosig_isTTY (JNIEnv *env, jclass) {
     printf("isTTY: Hello World!\n");

}
/*
 * Class:     Biosig
 * Method:    getTTYName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_Biosig_getTTYName (JNIEnv *env, jclass) {
     printf("getTTYName: Hello World!\n");
}

#ifdef __cplusplus
}
#endif
#endif
