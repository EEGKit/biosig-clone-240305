/*

    Copyright (C) 2000,2005,2007-2020 Alois Schloegl <alois.schloegl@gmail.com>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/


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

 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include "biosig.h"

#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define max(a,b)        (((a) > (b)) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif
int savelink(const char* filename);
#ifdef __cplusplus
}
#endif

#ifdef WITH_PDP
void sopen_pdp_read(HDRTYPE *hdr);
#endif

extern const uint16_t GDFTYP_BITS[];


int compare_uint16 (const void* a, const void* b) {
   return ( (int)*(uint16_t*)a - (int)*(uint16_t*)b );
}

int main(int argc, char **argv) {

    HDRTYPE 	*hdr;
    size_t 	count, k1, ne=0;
    char 	*source, *dest, *tmpstr;
    biosig_options_type   biosig_options;
    biosig_options.free_text_event_limiter = "";	// default is no delimiter

    enum FileFormat SOURCE_TYPE; 		// type of file format
    struct {
	enum FileFormat TYPE;
	float VERSION;
    } TARGET;
    TARGET.TYPE=GDF;
    TARGET.VERSION=-1.0;

    FILE*       fid=stdout;
    int		COMPRESSION_LEVEL=0;
    int		status;
    uint16_t	k;
    uint16_t	chansel = 0;
    int 	TARGETSEGMENT=1; 	// select segment in multi-segment file format EEG1100 (Nihon Kohden)
    int 	VERBOSE	= 0;
    char	FLAG_ANON = 1;
    char	FLAG_CSV = 0;
    char	FLAG_JSON = 0;
    char	FLAG_DYGRAPH = 0;
    char	*argsweep = NULL;
    double	t1=0.0, t2=1.0/0.0;
    uint16_t    *CHANLIST = NULL;
    uint16_t    LenChanList = 0;	// number of entries in CHANLIST

#ifdef CHOLMOD_H
    char *rrFile = NULL;
    int   refarg = 0;
#endif

	source = NULL;
	dest   = NULL;

    for (k=1; k<argc; k++) {
    	if (!strcmp(argv[k],"-v") || !strcmp(argv[k],"--version") ) {
		fprintf(stderr,"biosig2gdf (BioSig4C++) v%i.%i.%i\n", BIOSIG_VERSION_MAJOR, BIOSIG_VERSION_MINOR, BIOSIG_PATCHLEVEL);
		fprintf(stderr,"Copyright (C) 2006-2020 by Alois Schloegl and others\n");
		fprintf(stderr,"This file is part of BioSig http://biosig.sf.net - the free and\n");
		fprintf(stderr,"open source software library for biomedical signal processing.\n\n");
		fprintf(stderr,"BioSig is free software; you can redistribute it and/or modify\n");
		fprintf(stderr,"it under the terms of the GNU General Public License as published by\n");
		fprintf(stderr,"the Free Software Foundation; either version 3 of the License, or\n");
		fprintf(stderr,"(at your option) any later version.\n\n");
	}

    	else if (!strcmp(argv[k],"-h") || !strcmp(argv[k],"--help") ) {
		fprintf(stderr,"\nusage: biosig2gdf [OPTIONS] SOURCE\n");
		fprintf(stderr,"\nusage: biosig2gdf [OPTIONS] SOURCE DEST\n");
		fprintf(stderr,"  SOURCE is the source file \n");
		fprintf(stderr,"      SOURCE can be also network file bscs://<hostname>/ID e.g. bscs://129.27.3.99/9aebcc5b4eef1024 \n");
		fprintf(stderr,"  DEST is the destination file \n");
		fprintf(stderr,"\n  Supported OPTIONS are:\n");
		fprintf(stderr,"   -v, --version\n\tprints version information\n");
		fprintf(stderr,"   -h, --help   \n\tprints this information\n");
#ifdef CHOLMOD_H
		fprintf(stderr,"   -r, --ref=MM  \n\trereference data with matrix file MM. \n\tMM must be a 'MatrixMarket matrix coordinate real general' file.\n");
#endif
		fprintf(stderr, "   -a, --anon=yes   (default)\n"
				"\tanonymized data processing - personalize data (name, and birthday) is not processed but ignored.\n"
				"\tThe patient can be still identified with the unique patient identifier (and an external database).\n"
				"\tThis is for many cases sufficient (e.g. for research etc.). This mode can be turn off with\n"
				"   -n, --anon=no\n"
				"\tThis will process personal information like name and birthday. One might want to use this mode\n"
				"\twhen converting personalized patient data and no unique patient identifier is available.\n"
				"\tIt's recommended to pseudonize the data, or to use the patient identifier instead of patient name and birthday.\n"
		);
		fprintf(stderr, "   -c <chanlist> (default: 0)\n"
				"\tThis flag is in preparation and not working yet.\n"
				"\t<chanlist> is a comma-separated list of channel numbers.\n"
				"\tIf <chanlist> contains any number smaller than 1 or larger than the number \n"
				"\t  of available channels, all channels are loaded.\n"
		);
		fprintf(stderr,"   --free-text-event-limiter=\";\"\n\tfree text of events limited to first occurrence of \";\" (only EDF+/BDF+ format)\n");
		fprintf(stderr,"   -s=#\tselect target segment # (in the multisegment file format EEG1100)\n");
		fprintf(stderr,"   -SWEEP=ne,ng,ns\n\tsweep selection of HEKA/PM files\n\tne,ng, and ns select the number of experiment, the number of group, and the sweep number, resp.\n");
		fprintf(stderr,"   -VERBOSE=#, verbosity level #\n\t0=silent [default], 9=debugging\n");
		fprintf(stderr,"   --chan=CHAN\n\tselect channel CHAN (0: all channels, 1: first channel, etc.)\n");
		fprintf(stderr,"\n\n");
		return(0);
	}

    	else if (!strncmp(argv[k],"-VERBOSE",2)) {
	    	VERBOSE = argv[k][strlen(argv[k])-1]-48;
#ifndef NDEBUG
		// then VERBOSE_LEVEL is not a constant but a variable
		VERBOSE_LEVEL = VERBOSE;
#endif
		}
    	else if (!strncasecmp(argv[k],"-SWEEP=",7))
	    	argsweep = argv[k]+6;

	else if (!strcasecmp(argv[k],"-a") || !strcasecmp(argv[k],"--anon") )
		FLAG_ANON = 1;

	else if (!strcasecmp(argv[k],"-n") || !strcasecmp(argv[k],"--anon=no") )
		FLAG_ANON = 0;

	else if (!strcasecmp(argv[k],"-c") ) {
		char *CS  = strdup(argv[++k]);
		char *CS0 = CS;
		CHANLIST  = realloc(CHANLIST,sizeof(uint16_t)*strlen(CS));
		LenChanList = 0;
		while (isdigit(*CS)) {
			CHANLIST[LenChanList++] = strtol(CS,&CS,10);
			if (*CS==0) break;
			if (*CS != ',') {
				LenChanList = 0;
				break;
			}
			CS++;
		}
		CHANLIST[LenChanList]=0xffff;
		free(CS0);
		qsort(CHANLIST, LenChanList, sizeof(uint16_t), compare_uint16);
	}
	else if (!strncmp(argv[k],"--free-text-event-limiter=",26))
		biosig_options.free_text_event_limiter = strstr(argv[k],"=") + 1;

#ifdef CHOLMOD_H
    	else if ( !strncmp(argv[k],"-r=",3) || !strncmp(argv[k],"--ref=",6) )	{
    	        // re-referencing matrix
		refarg = k;
	}
#endif

	else if (!strncmp(argv[k],"-s=",3))  {
    		TARGETSEGMENT = atoi(argv[k]+3);
	}

	else if (argv[k][0]=='[' && argv[k][strlen(argv[k])-1]==']' && (tmpstr=strchr(argv[k],',')) ) {
		t1 = strtod(argv[k]+1,NULL);
		t2 = strtod(tmpstr+1,NULL);
		if (VERBOSE_LEVEL>7) fprintf(stderr,"[%f,%f]\n",t1,t2);
	}

	else {
		break;
	}

	if (VERBOSE_LEVEL>7)
		fprintf(stderr,"%s (line %i): biosig2gdf: arg%i = <%s>\n",__FILE__,__LINE__, k, argv[k]);

    }   // end of for (k=1; k<argc; k++)

	switch (argc - k) {
	case 0:
		fprintf(stderr,"biosig2gdf: missing file argument\n");
		fprintf(stderr,"usage: biosig2gdf [options] SOURCE\n");
		fprintf(stderr," for more details see also biosig2gdf --help \n");
		exit(-1);
	case 1:
		source = argv[k];
		break;
	case 2:
		source = argv[k];
		dest   = argv[k+1];
		fid    = fopen(dest,"w");
		break;
	default:
		fprintf(stderr,"biosig2gdf: extra arguments %d-%d (%s, etc) ignored\n",argc,k,argv[k+1]);
	}

	if (VERBOSE_LEVEL<0) VERBOSE=1; // default
	if (VERBOSE_LEVEL>7) fprintf(stderr,"%s (line %i): BIOSIG2GDF %s started \n",__FILE__,__LINE__, source);

	tzset();
	hdr = constructHDR(0,0);
	// hdr->FLAG.OVERFLOWDETECTION = FlagOverflowDetection;
	hdr->FLAG.UCAL = 1;
	hdr->FLAG.TARGETSEGMENT = TARGETSEGMENT;
	hdr->FLAG.ANONYMOUS = FLAG_ANON;

	if (argsweep) {
		k = 0;
		do {
			hdr->AS.SegSel[k++] = strtod(argsweep+1, &argsweep);
		} while (argsweep[0]==',' && (k < 5) );
	}
	hdr = sopen_extended(source, "r", hdr, &biosig_options);

#ifdef WITH_PDP
	if (hdr->AS.B4C_ERRNUM) {
		biosigERROR(hdr, 0, NULL);  // reset error
		sopen_pdp_read(hdr);
	}
#endif
	// HEKA2ITX hack
        if (TARGET.TYPE==ITX) {
	if (hdr->TYPE==HEKA) {
		// hack: HEKA->ITX conversion is already done in SOPEN
		dest = NULL;
	}
	else {
                fprintf(stderr,"error: only HEKA->ITX is supported - source file is not HEKA file");
		biosigERROR(hdr, B4C_UNSPECIFIC_ERROR, "error: only HEKA->ITX is supported - source file is not HEKA file");
	}
	}

		if ((status=serror2(hdr))) {
		destructHDR(hdr);
		exit(status);
	}

	t1 *= hdr->SampleRate / hdr->SPR;
	t2 *= hdr->SampleRate / hdr->SPR;
	if (isnan(t1)) t1 = 0.0;
	if (t2+t1 > hdr->NRec) t2 = hdr->NRec - t1;

	if ( ( t1 - floor (t1) ) || ( t2 - floor(t2) ) ) {
		fprintf(stderr,"ERROR BIOSIG2GDF: cutting from parts of blocks not supported; t1 (%f) and t2 (%f) must be a multiple of block duration %f\n", t1,t2,hdr->SPR / hdr->SampleRate);
		biosigERROR(hdr, B4C_UNSPECIFIC_ERROR, "blocks must not be split");
	}

	if ((status=serror2(hdr))) {
		destructHDR(hdr);
		exit(status);
	}
	sort_eventtable(hdr);

	/*******************************************
		Channel selection
	 *******************************************/
	uint16_t NS2=0; // number of channels (w/o hidden channels)

	for (k=0; k < hdr->NS; k++) {
		NS2 += (hdr->CHANNEL[k].OnOff > 0);
	}
	for (k = 0; k < LenChanList; k++) {
		// check whether all arguments specify a valid channel number
		if ((CHANLIST[k] < 1) || (NS2 < CHANLIST[k])) {
			LenChanList = 0;	// in case of an invalid channel, all channels are selected
			break;
		}
	}

	if (LenChanList > 0) {
		fprintf(stderr, "argument -c <chanlist> is currently not supported, the argument is ignored ");
		LenChanList=0;
	}
	if (LenChanList > 0) {
		// all entries in argument -c specify valid channels
		int chan=0;
		int k1 = 0;
		for (k=0; k<hdr->NS; k++)
		if (hdr->CHANNEL[k].OnOff > 0) {
			chan++;	// count
			while (CHANLIST[k1] < chan) k1++; // skip double entries in CHANLIST

			if      (CHANLIST[k1]==chan) {
				hdr->CHANNEL[k].OnOff = 1;
				k1++;
				fprintf(stderr,"-- %d  %d  %d \n",k,k1,chan);
			}
			else if (CHANLIST[k1] > chan)
				hdr->CHANNEL[k].OnOff = 0;
		}
	}

    	for (k=0; k<hdr->NS; k++) {
		if ( (hdr->CHANNEL[k].OnOff > 0) && hdr->CHANNEL[k].SPR ) {
			if ((hdr->SPR/hdr->CHANNEL[k].SPR)*hdr->CHANNEL[k].SPR != hdr->SPR)
				 fprintf(stderr,"Warning: channel %i might be decimated!\n",k+1);
    		};
	}
	if (CHANLIST) {free(CHANLIST); CHANLIST=NULL;}

	hdr->FLAG.OVERFLOWDETECTION = 0;
	hdr->FLAG.UCAL = 1;
	hdr->FLAG.ROW_BASED_CHANNELS = 1;

	if (VERBOSE_LEVEL>7)
		fprintf(stderr,"%s (line %i): SREAD [%f,%f].\n",__FILE__,__LINE__,t1,t2);

	if (hdr->NRec <= 0) {
		// in case number of samples is not known
		count = sread(NULL, t1, (size_t)-1, hdr);
		t2 = count;
	}
 	else {
		if (t2+t1 > hdr->NRec) t2 = hdr->NRec - t1;
		count = sread(NULL, t1, t2, hdr);
	}

	biosig_data_type* data = hdr->data.block;

	if ((status=serror2(hdr))) {
		destructHDR(hdr);
		exit(status);
	};

	if (hdr->FILE.OPEN) {
		sclose(hdr);
		free(hdr->AS.Header);
		hdr->AS.Header = NULL;
		if (VERBOSE_LEVEL>7) fprintf(stderr,"%s (line %i): file closed\n",__FILE__,__LINE__);
	}

	SOURCE_TYPE = hdr->TYPE;

	hdr->TYPE = GDF;
	hdr->VERSION = 3.0;

	hdr->FILE.COMPRESSION = COMPRESSION_LEVEL;

	/*******************************************
	 convert to unique sampling rate and unique data type
	 *******************************************/

	uint16_t gdftyp=0;
	if (1) {
		uint16_t nbits=0;
		int flag_fnbits=0;
		int flag_snbits=0;
		int flag_unbits=0;
		int flag_signed=0;

    		for (k=0; k < hdr->NS; k++) {
		    	if (hdr->CHANNEL[k].OnOff && hdr->CHANNEL[k].SPR) {
				hdr->CHANNEL[k].SPR = hdr->SPR;

				uint16_t gt = hdr->CHANNEL[k].GDFTYP;
				if ((15<gt) && (gt<20))
					if (flag_fnbits < GDFTYP_BITS[gt]) flag_fnbits=GDFTYP_BITS[gt];
				if (((gt<15) && !(gt%2)) || (511<gt))
					if (flag_unbits < GDFTYP_BITS[gt]) flag_unbits=GDFTYP_BITS[gt];
				if (((gt<15) &&  (gt%2)) || ((255<gt) && (gt<511)))
					if (flag_snbits < GDFTYP_BITS[gt]) flag_snbits=GDFTYP_BITS[gt];
			}
		}

		if      (flag_snbits>flag_unbits) { nbits=flag_snbits; flag_signed=1;}
		else if (flag_snbits<flag_unbits) { nbits=flag_unbits; flag_signed=0;}
		else    {nbits = flag_unbits+1; flag_signed=1;}

		if      (flag_fnbits==32)  gdftyp=16;
		else if (flag_fnbits==64)  gdftyp=17;
		else if (flag_fnbits==128) gdftyp=18;
		else if (nbits>64)         gdftyp=8-flag_signed;
		else if (nbits>32)         gdftyp=8-flag_signed;
		else if (nbits>16)         gdftyp=6-flag_signed;
		else if (nbits>8)          gdftyp=4-flag_signed;
		else                       gdftyp=2-flag_signed;

    		hdr->SPR  = 1;
	    	hdr->NRec = hdr->data.size[1];
		typeof(hdr->NS) NS=0;
    		for (k=0; k<hdr->NS; k++) {
		    	if (hdr->CHANNEL[k].OnOff) {
				NS++;
				hdr->CHANNEL[k].GDFTYP = gdftyp;
	    			hdr->CHANNEL[k].SPR    = hdr->SPR;
			}
		}
		hdr->AS.bpb = NS*GDFTYP_BITS[gdftyp];
    	}

	/*********************************
		Write data
	 *********************************/

	hdr->FLAG.ANONYMOUS = FLAG_ANON;

	/*
	  keep header data from previous file, in might contain optional data
	  (GDF Header3, EventDescription, hdr->SCP.SectionX, etc. )
	  and might still be referenced and needed.
	*/
	void *tmpmem = hdr->AS.Header;
	hdr->AS.Header = NULL;
	size_t filesize=0;

	//  open_gdfwrite(hdr,stdout);
	// encode header
	hdr->TYPE=GDF;
	hdr->VERSION=3.0;
	struct2gdfbin(hdr);

	// write header into buffer
	filesize += fwrite (hdr->AS.Header, 1, hdr->HeadLen, fid);

	count = 0;
	char BLK[0x10000];
	unsigned spb = (0x10000 / hdr->AS.bpb) * hdr->AS.bpb / GDFTYP_BITS[gdftyp];
	while (count < hdr->data.size[0]*hdr->data.size[1]) {
		if (count+spb > hdr->data.size[0]*hdr->data.size[1])
			spb = hdr->data.size[0] * hdr->data.size[1] - count;

		switch (gdftyp) {
		case 1: {
			int8_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) data[k]=(int8_t)hdr->data.block[k+count];
			break;
			}
		case 2: {
			uint8_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) data[k]=(int8_t)hdr->data.block[k+count];
			break;
			}
		case 3: {
			int16_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) lei16a((int16_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 4: {
			uint16_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) leu16a((uint16_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 5: {
			int32_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) lei32a((int32_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 6: {
			uint32_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) leu32a((uint32_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 7: {
			int64_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) lei64a((int64_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 8: {
			uint64_t *data = (void*)&BLK;
			for (int k=0; k < spb; k++) leu64a((uint64_t)hdr->data.block[k+count], data+k);
			break;
			}
		case 16: {
			float *data = (void*)&BLK;
			for (int k=0; k < spb; k++) lef32a((float)hdr->data.block[k+count], data+k);
			break;
			}
		case 17: {
			double *data = (void*)&BLK;
			for (int k=0; k < spb; k++) lef64a((double)hdr->data.block[k+count], data+k);
			break;
			}
		default: {
			}
		}
		count += fwrite(BLK, GDFTYP_BITS[gdftyp]/8, spb, fid);
	}

	filesize += count*GDFTYP_BITS[gdftyp]/8;

	// write event table into buffer
	size_t len3 = hdrEVT2rawEVT(hdr);
	filesize += fwrite(hdr->AS.rawEventData, 1, len3, fid);
	if (fid != stdout) fclose(fid);

	status = serror2(hdr);
	destructHDR(hdr);
	exit(status);
}

