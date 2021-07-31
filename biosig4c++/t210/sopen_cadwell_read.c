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

#include "../biosig.h"

/******************************************************************************
	read CADWELL file formats (EAS, EZ3, ARC)
 ******************************************************************************/
void sopen_cadwell_read(HDRTYPE* hdr) {	

	if (VERBOSE_LEVEL > 8) fprintf(stdout,"%s (line %d) %s(hdr)\n", __FILE__, __LINE__, __func__);

	if (hdr->TYPE==EAS) {
		/* 12 bit ADC ?
		   200 Hz 
		   77*800 samples	
		   EEGData section has a periodicity of 202*2 (404 bytes) 
			800samples*16channels*2byte=25600  = 0x6400)
		*/
		hdr->NS  = 16;  // so far all example files had 16 channels
		hdr->SPR  = 1;
		hdr->NRec = 0;
		unsigned lengthHeader0 = leu32p(hdr->AS.Header + 0x30);
		unsigned lengthHeader1 = leu32p(hdr->AS.Header + 0x34);	// this is varying amoung data sets - meaning unknown 	
		assert(lengthHeader0==0x0400);

		for (int k=0; k*0x20 < lengthHeader1; k++) {
			char *sectName =       hdr->AS.Header + 0x6c + k*0x20;
			size_t sectPos= leu32p(hdr->AS.Header + 0x7c + k*0x20);
			size_t sectN1 = leu32p(hdr->AS.Header + 0x80 + k*0x20);
			size_t sectN2 = leu32p(hdr->AS.Header + 0x84 + k*0x20);
			size_t sectN3 = leu32p(hdr->AS.Header + 0x88 + k*0x20);

			if (!sectPos
			 || memcmp("SctHdr\0\0", hdr->AS.Header+sectPos, 8)
			 || memcmp(hdr->AS.Header+sectPos+8, sectName, 16)) 
			{
				if (VERBOSE_LEVEL > 8) fprintf(stdout,"%s (line %d): break loop (0x%x %s)\n",__FILE__,__LINE__, sectPos, sectName);
				break;
			}

			uint64_t curSec, nextSec;
			int flag=1;
			do {
				curSec  = leu64p(hdr->AS.Header + sectPos + 24);
				nextSec = leu64p(hdr->AS.Header + sectPos + 32);

		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d): 0x%08x %s  0x%08lx 0x%08lx \n",__FILE__,__LINE__, sectPos, sectName, curSec, nextSec);
				if (flag && !strcmp(sectName,"EEGData")) {
		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d): 0x%08x %s  0x%08lx 0x%08lx \n",__FILE__,__LINE__, sectPos, sectName, curSec, nextSec);
					FILE *fid2=fopen("tmp.bin","w");
					fwrite(hdr->AS.Header + curSec+120, 1, nextSec-curSec-120,fid2);
					fclose(fid2);
					FILE *fid=fopen("tmp.asc","w");
					for (size_t k0=curSec+8*15; k0 < nextSec; k0+=2) {
						fprintf(fid,"%d\n",bei16p(hdr->AS.Header + curSec + k0+1));
					}
					fclose(fid);
					flag = 0;
				}
				sectPos = nextSec;
			} while (nextSec != (size_t)-1L);
		}
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format EAS(Cadwell): unsupported ");
	}

	else if (hdr->TYPE==EZ3) {
		hdr->VERSION = strtod((char*)hdr->AS.Header+21, NULL);
		// 16 bit ADC ? 
		// 250 Hz ?

		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d)0x%8x\n",__FILE__,__LINE__,hdr->HeadLen);

		uint32_t posH1  = leu32p(hdr->AS.Header + 0x10);
		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d)0x%8x\n",__FILE__,__LINE__,posH1);

		uint32_t posH1b = leu32p(hdr->AS.Header + 0x20);
		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d)0x%8x\n",__FILE__,__LINE__,posH1b);

		uint32_t posH2  = leu32p(hdr->AS.Header + 0x38);
		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d)0x%8x\n",__FILE__,__LINE__,posH2);

		uint32_t posH2b = leu32p(hdr->AS.Header + posH1 + 0x38);

		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d) 0x%08x 0x%08x 0x%08x 0x%08x \n",__FILE__,__LINE__,posH1,posH1b,posH2,posH2b);

		assert(posH1==posH1b);
		assert(posH2==posH2b);

		if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d)\n",__FILE__,__LINE__);

		// start date/time
		{
			char *tmp = hdr->AS.Header + 0x5c;
			struct tm t;
			t.tm_year = strtol(tmp,&tmp,10)-1900;
			t.tm_mon = strtol(tmp+1,&tmp,10)-1;
			t.tm_mday = strtol(tmp+1,&tmp,10);
			t.tm_hour = strtol(tmp+1,&tmp,10);
			t.tm_min = strtol(tmp+1,&tmp,10);
			t.tm_sec = strtol(tmp+1,&tmp,10);
			hdr->T0 = tm_time2gdf_time(&t);
		}

		uint32_t pos0=posH1+0x40;
		for (size_t k = posH1+0x40; k < posH2; k += 0x30) {
			char *tmp    = hdr->AS.Header + k + 1;
			uint32_t pos = leu32p(hdr->AS.Header + k + 0x28);
			if (tmp[0]=='\0') break;

			if (VERBOSE_LEVEL > 7) fprintf(stdout,"%s (line %d): Label=<%s> %10d(0x%08x) [sz=%d]\n",__FILE__,__LINE__, tmp, pos, pos, pos-pos0);
			pos0=pos;		
		}
	
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format EZ3(Cadwell): unsupported ");
	}
	else if (hdr->TYPE==ARC) {
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format ARC(Cadwell): unsupported ");
	}
}

