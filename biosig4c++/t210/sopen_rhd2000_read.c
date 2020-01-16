/*
    Copyright (C) 2019,2020 Alois Schloegl <alois.schloegl@gmail.com>

    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.
 */

/*
References:
[1]  RHD2000 Application Note: Data File Formats, Intan TECHNOLOGIES, LLC
     Downloaded 2020-01-14 from
     http://www.intantech.com/files/Intan_RHD2000_data_file_formats.pdf
[2]  RHS2000 Data File Formats - Intan Tech
     http://www.intantech.com/files/Intan_RHS2000_data_file_formats.pdf
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <sys/stat.h>

#if !defined(__APPLE__) && defined (_LIBICONV_H)
 #define iconv		libiconv
 #define iconv_open	libiconv_open
 #define iconv_close	libiconv_close
#endif

#include "../gdftime.h"
#include "../biosig-dev.h"

#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define max(a,b)        (((a) > (b)) ? (a) : (b))

typedef struct {
	int32_t length;	// this is little endian as defined in spec, use le32toh() when using it
	char *string;
} QSTRING_T;

#ifdef __cplusplus
extern "C" {
#endif

/* check whether sufficient data of the header information has been read,
   reads more data if needed, advance position pointer pos to the end of the string;
*/
QSTRING_T *read_qstring(HDRTYPE* hdr, size_t *pos) {
	uint32_t len = leu32p(hdr->AS.Header + (*pos));
	*pos += 4;
	if (len==(uint32_t)(-1)) return NULL;

	// after each qstring at most 28 bytes are loaded before the next check for a qstring
	// This check is also needed when qstring is empty
	size_t SIZE = *pos+len+100;
	if (SIZE > hdr->HeadLen) {
		SIZE = max(SIZE, 2*hdr->HeadLen);	// always double size of header
		void *ptr = realloc(hdr->AS.Header, SIZE);
		if (ptr==NULL) {
			biosigERROR(hdr, B4C_MEMORY_ALLOCATION_FAILED, "Format Intan RHD2000 - memory allocation failed");
			return NULL;
		}
		hdr->AS.Header = (uint8_t*)ptr;
		hdr->HeadLen += ifread(hdr->AS.Header+hdr->HeadLen, 1, SIZE-hdr->HeadLen, hdr);
	}

	if ((*pos + len ) > hdr->HeadLen)
		biosigERROR(hdr, B4C_INCOMPLETE_FILE, "Format Intan RHD2000 - incomplete file");

	QSTRING_T *qString=(QSTRING_T *)(hdr->AS.Header+(*pos)-4);
	*pos += len;
	return qString;
}

size_t  convert_qstring_to_utf8(QSTRING_T *S, char **__restrict outbuf, size_t *__restrict outlen) {
	if (S==NULL) return 0;
	if (S->length==(-1)) return 0;
	iconv_t CD = iconv_open ("UTF-8", "UTF-16LE");
	size_t inlen = le32toh(S->length);
	char *inbuf = &(S->string);
	size_t iconv_res = iconv (CD, &inbuf, &inlen, outbuf, outlen);
	iconv_close (CD);
}


int sopen_intan_clp_read(HDRTYPE* hdr) {

	uint16_t NumADCs=0, NumChips=0, NumChan=0;

	float minor = leu16p(hdr->AS.Header+6);
	minor      *= (minor < 10) ? 0.1 : 0.01;
	hdr->VERSION = leu16p(hdr->AS.Header+4) + minor;

	uint16_t datatype=leu16p(hdr->AS.Header+8);
	switch (datatype) {
	case 1: NumADCs=leu16p(hdr->AS.Header+10);
		hdr->SampleRate = lef32p(hdr->AS.Header+24);
	case 0: break;
	default:
		// this should never ever occurs, because getfiletype checks this
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan CLP - datatype unknown");
		return -1;
	}

	size_t HeadLen=leu16p(hdr->AS.Header+10+(datatype*2));
	// read header
	if (HeadLen > hdr->HeadLen) {
		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, HeadLen+1);
		hdr->HeadLen  += ifread(hdr->AS.Header+HeadLen, 1, HeadLen - hdr->HeadLen, hdr);
	}
	hdr->AS.Header[hdr->HeadLen]=0;
	if (HeadLen > hdr->HeadLen) {
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan/CLP - file is too short");
		return -1;
	}
	ifseek(hdr, HeadLen, SEEK_SET);

	// read recording date and time
	size_t pos=12+(datatype*2);
	{
		struct tm t0;
		t0.tm_year = leu16p(hdr->AS.Header+pos);
		t0.tm_mon = leu16p(hdr->AS.Header+pos+2);
		t0.tm_mday = leu16p(hdr->AS.Header+pos+4);
		t0.tm_hour = leu16p(hdr->AS.Header+pos+6);
		t0.tm_min = leu16p(hdr->AS.Header+pos+8);
		t0.tm_sec = leu16p(hdr->AS.Header+pos+10);
		hdr->T0 = tm_time2gdf_time(&t0);
	}

	switch (datatype) {
	case 0:
		// If this is the standard data file (including clamp and measured data),
		/* TODO:
			read chips
			read settings
		*/
		break;
	case 1:
		// If this is the aux data file (including Digital Ins/Outs and ADC data)
	default:
		biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan CLP - datatype unknown");
		return -1;
	}

	switch (datatype) {
	case 0: {
		// read chips
		pos = 24;
		NumChips=leu16p(hdr->AS.Header+pos);
		NumChan=leu16p(hdr->AS.Header+pos+2);
		for (uint16_t k1 = 0; k1 < NumChips; k1++) {
		for (uint16_t k2 = 0; k2 < NumChan; k2++) {
			// read one header per chip
			//14*2+4+4+16+20+4
		}
			// read
		}
		pos += ((14*2+4+4+16+20+4) * NumChan + 8) * NumChips;
		// read settings

		hdr->NS    = 4;
		hdr->SPR   = 1;
		hdr->NRec  = -1;
		hdr->AS.bpb= 16;

		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
		// see below // strcpy(hdr->CHANNEL[0].Label,"Time");
		strcpy(hdr->CHANNEL[1].Label,"Clamp");
		strcpy(hdr->CHANNEL[2].Label,"TotalClamp");
		strcpy(hdr->CHANNEL[3].Label,"Measured");

		for (int k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			hc->Transducer[0]='\0';
			hc->OnOff=1;
			hc->GDFTYP=16;
			hc->DigMax=+1e9;
			hc->DigMin=-1e9;
			hc->Cal=1;
			hc->Off=0;
		}
		break;
	    }
	case 1: {
		hdr->NS    = NumADCs;
		hdr->SPR   = 1;
		hdr->NRec  = -1;
		hdr->AS.bpb= 4 + 2 + 2 + 2 * NumADCs;

		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
		for (int k=0; k < hdr->NS; k++) {
			CHANNEL_TYPE *hc = hdr->CHANNEL+k;
			sprintf(hdr->CHANNEL[3].Label, "ADC%d", k-2);
			hc->Transducer[0]='\0';
			hc->OnOff=1;
			hc->GDFTYP=4;
			hc->DigMax=65535.0;
			hc->DigMin=0;
			hc->Cal = (k < 3) ? 1.0 : 0.0003125;
			hc->Off = (k < 3) ? 0.0 : -10.24;
		}
		// see below // strcpy(hdr->CHANNEL[0].Label, "Time");
		strcpy(hdr->CHANNEL[1].Label,"DigitalIn");
		strcpy(hdr->CHANNEL[2].Label,"DigitalOut");
		break;
	    }
	default:
		;
	}

	hdr->CHANNEL[0].OnOff=2;	//uint32
	hdr->CHANNEL[0].GDFTYP=6;	//uint32
	hdr->CHANNEL[0].DigMax=ldexp(1l,32)-1;	//uint32
	hdr->CHANNEL[0].DigMin=0.0;	//uint32
	hdr->CHANNEL[0].Cal=1.0/hdr->SampleRate;	//uint32
	hdr->CHANNEL[0].PhysDimCode = 2176;	//uint32
	strcpy(hdr->CHANNEL[0].Label, "Time");

	hdr->AS.bpb = 0;
	for (int k = 0; k < hdr->NS; k++) {
		CHANNEL_TYPE *hc = hdr->CHANNEL+k;
		hc->PhysMax   = hc->DigMax * hc->Cal + hc->Off;
		hc->PhysMin   = hc->DigMin * hc->Cal + hc->Off;
		hc->LeadIdCode = 0;
		hc->PhysDimCode = 0;
		hc->TOffset   = 0;
		hc->LowPass   = NAN;
		hc->HighPass  = NAN;
		hc->Notch     = NAN;
		hc->Impedance = NAN;
		hc->fZ        = NAN;
		hc->SPR       = 1;
		memset(hc->XYZ, 0, 12);
		hc->bi        = hdr->AS.bpb;
		hdr->AS.bpb  += (GDFTYP_BITS[hc->GDFTYP]*(size_t)hc->SPR) >> 3;
	}

	biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan/CLP not supported");
	return -1;
}

int sopen_rhs2000_read(HDRTYPE* hdr) {
		int gdftyp = 4;
		char bufstr[1000];
		size_t outlen=1000;
		char *tmpstr=bufstr;

		size_t c= convert_qstring_to_utf8((QSTRING_T *)(hdr->AS.Header+0x64), &tmpstr, &outlen);

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) [%u] <%s> %d\n",__FILE__,__LINE__,__func__,hdr->HeadLen,bufstr,outlen);

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) [%u]\n",__FILE__,__LINE__,__func__,hdr->HeadLen);

		// 8 bytes
		float minor = leu16p(hdr->AS.Header+6);
		minor      *= (minor < 10) ? 0.1 : 0.01;
		hdr->VERSION = leu16p(hdr->AS.Header+4) + minor;

		// +38 = 46 bytes
		hdr->NS = 1;
		hdr->SampleRate = lef32p(hdr->AS.Header+8);

		float HighPass = ( leu16p(hdr->AS.Header+12) ? lef32p(hdr->AS.Header+14) : 0.0 );
		      HighPass = max( HighPass, lef32p(hdr->AS.Header+18) );
		float LowPass = lef32p(hdr->AS.Header+26);

		// +10 = 56 bytes
		const int ListNotch[] = {0,50,60};
		uint16_t tmp = leu16p(hdr->AS.Header+46);
		if (tmp>2) tmp=0;
		float Notch = ListNotch[tmp];
		float fZ_desired   = lef32p(hdr->AS.Header+46); // desired impedance test frequency
		float fZ_actual    = lef32p(hdr->AS.Header+50); // actual impedance test frequency

		// +4 = 60 bytes
		// +12 = 72 bytes

		size_t pos = 72;
		QSTRING_T *note1 = read_qstring(hdr, &pos);
		QSTRING_T *note2 = read_qstring(hdr, &pos);
		QSTRING_T *note3 = read_qstring(hdr, &pos);

		uint16_t flag_DC_amplifier_data_saved = leu16p(hdr->AS.Header+pos);
		pos += 2;
		uint16_t flag_Board_Mode = leu16p(hdr->AS.Header+pos);
		pos += 2;

		QSTRING_T *ReferenceChannelName = read_qstring(hdr, &pos);

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...)  %d %d\n", __FILE__, __LINE__, __func__, pos, hdr->HeadLen);

		uint16_t numberOfSignalGroups = leu16p(hdr->AS.Header+pos);
		pos += 2;
		uint16_t NS = 0;
		// read all signal groups
		for (int nsg=0; nsg<numberOfSignalGroups; nsg++) {
			QSTRING_T *SignalGroupName = read_qstring(hdr, &pos);
			QSTRING_T *SignalGroupPrefix = read_qstring(hdr, &pos);
			uint16_t flag_SignalGroupEnabled = leu16p(hdr->AS.Header+pos);
			pos += 2;
			uint16_t NumberOfChannelsInSignalGroup = leu16p(hdr->AS.Header+pos);
			pos += 2;
			uint16_t NumberOfAmplifierChannelsInSignalGroup = leu16p(hdr->AS.Header+pos);
			pos += 2;

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) group=%u %u+%u %d\n", __FILE__, __LINE__, __func__, nsg, NS, NumberOfChannelsInSignalGroup, pos);

		//if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) group=%u %s %s\n",__FILE__,__LINE__,__func__,nsg,SignalGroupName->string,SignalGroupPrefix->string);

			if (flag_SignalGroupEnabled) {
				hdr->CHANNEL = (CHANNEL_TYPE*) realloc(hdr->CHANNEL, (1+NS+NumberOfChannelsInSignalGroup) * sizeof(CHANNEL_TYPE));
				for (unsigned k = 0; k < NumberOfChannelsInSignalGroup; k++) {
					QSTRING_T *NativeChannelName = read_qstring(hdr, &pos);
					QSTRING_T *customChannelName = read_qstring(hdr, &pos);
					pos += 4;
					int16_t SignalType = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t ChannelEnabled = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t ChipChannel = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t CommandStream = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t BoardStream = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t SpikeScopeVoltageTriggerMode = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t SpikeScopeVoltageThreshold = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t SpikeScopeDigitalTriggerChannel = lei16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t SpikeScopeDigitalTriggerEdgePolarity = lei16p(hdr->AS.Header+pos);
					pos += 2;
					float ElectrodeImpedanceMagnitude = lef32p(hdr->AS.Header+pos);
					pos += 4;
					float ElectrodeImpedancePhase = lef32p(hdr->AS.Header+pos);
					pos += 4;

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) group=%u %u %d %d\n",__FILE__,__LINE__,__func__,nsg,k,ChannelEnabled,pos);

					if (ChannelEnabled) {
						NS++;	// first channel is Time channel
						CHANNEL_TYPE *hc = hdr->CHANNEL + NS;

//		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) %u %2u %3u [%c %c]\n",__FILE__,__LINE__,__func__,nsg,k,NS,((char*)customChannelName->string)[0],((char*)customChannelName->string)[1]);
						size_t outlen = MAX_LENGTH_LABEL+1;
						char *outbuf = hc->Label;
						size_t c = convert_qstring_to_utf8(NativeChannelName, &outbuf, &outlen);

#ifdef MAX_LENGTH_PHYSDIM
						strcpy(hc->PhysDim,"?");
#endif
						hc->OnOff  = 1;
						hc->SPR    = hdr->SPR;
						hc->GDFTYP = 4; // uint16
						hc->bi     = 4-2+NS*2;
						hc->bi8    = hc->bi << 3;
						hc->LeadIdCode = 0;
						hc->DigMin = 0.0;
						hc->DigMax = ldexp(2,16)-1;
						hc->Off    = 0.0;
						hc->Cal    = 1.0 / hdr->SampleRate;
						hc->PhysDimCode = 0; // [?]

						hc->PhysMin = -1.0;
						hc->PhysMax = +1.0;

						hc->bufptr = NULL;
						hc->TOffset = 0;
						hc->LowPass = LowPass;
						hc->HighPass = HighPass;
						hc->Notch = Notch;
						hc->XYZ[0] = NAN;
						hc->XYZ[1] = NAN;
						hc->XYZ[2] = NAN;
						hc->Impedance = ElectrodeImpedanceMagnitude;

		if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %u); %s(...) %u %2u %3u Label<%s>: %d\n",__FILE__,__LINE__,__func__,nsg,k,NS,hc->Label,c);

					}
				}
			}
		}
		{	// channel 0 - time channel
						CHANNEL_TYPE *hc = hdr->CHANNEL+0;
						hc->OnOff = 2;
						strcpy(hc->Label, "time");
						strcpy(hc->Transducer, "");
#ifdef MAX_LENGTH_PHYSDIM
						strcpy(hc->PhysDim,"s");
#endif
						hc->SPR = hdr->SPR;
						hc->GDFTYP  = 6; // uint32
						hc->bi      = 0;
						hc->bufptr  = NULL;

						hc->OnOff  = 1;
						hc->SPR    = hdr->SPR;
						hc->GDFTYP = 4; // uint16
						hc->bi     = 4-2+NS*2;
						hc->bi8    = hc->bi << 3;
						hc->LeadIdCode = 0;
						hc->DigMin = 0.0;
						hc->DigMax = ldexp(2,32)-1;
						hc->Off    = 0.0;
						hc->Cal    = 1.0/hdr->SampleRate;
						hc->PhysDimCode = 2176; // [s]
						hc->PhysMin = 0;
						hc->PhysMax = hc->DigMax*hc->Cal;

						hc->bufptr = NULL;
						hc->TOffset = 0;
						hc->LowPass = 0;
						hc->HighPass = INFINITY;
						hc->Notch = 0;
						hc->XYZ[0] = NAN;
						hc->XYZ[1] = NAN;
						hc->XYZ[2] = NAN;
						hc->Impedance = NAN;
		}

		hdr->HeadLen = pos;
		hdr->SPR = 128;
		hdr->NRec = -1;
		hdr->NS = NS;
		hdr->AS.bpb = (4+2*NS)*hdr->SPR;
		ifseek(hdr, hdr->HeadLen, SEEK_SET);

		struct stat FileBuf;
		if (stat(hdr->FileName,&FileBuf)==0) hdr->FILE.size = FileBuf.st_size;
		hdr->NRec = (hdr->FILE.size - hdr->HeadLen) / hdr->AS.bpb;

		return 0;
}

int sopen_rhd2000_read(HDRTYPE* hdr) {

		float minor = leu16p(hdr->AS.Header+6);
		minor      *= (minor < 10) ? 0.1 : 0.01;
		hdr->VERSION = leu16p(hdr->AS.Header+4) + minor;

		hdr->NS = 1;
		hdr->SampleRate = lef32p(hdr->AS.Header+8);

		float HighPass = ( leu16p(hdr->AS.Header+12) ? lef32p(hdr->AS.Header+14) : 0.0 );
		      HighPass = max( HighPass, lef32p(hdr->AS.Header+18) );
		float LowPass = lef32p(hdr->AS.Header+22);

		const int ListNotch[] = {0,50,60};
		uint16_t tmp = leu16p(hdr->AS.Header+34);
		if (tmp>2) tmp=0;
		float Notch = ListNotch[tmp];
		float fZ_desired   = lef32p(hdr->AS.Header+40); // desired impedance test frequency
		float fZ_actual    = lef32p(hdr->AS.Header+44); // actual impedance test frequency

		size_t pos = 48;
		QSTRING_T *note1 = read_qstring(hdr, &pos);
		QSTRING_T *note2 = read_qstring(hdr, &pos);
		QSTRING_T *note3 = read_qstring(hdr, &pos);

		uint16_t numberTemperatureSensors = leu16p(hdr->AS.Header+pos);
		pos += 2;

		int boardMode = leu16p(hdr->AS.Header+pos);
		pos += 2;
		float PhysMin=0.;
		float PhysMax=1.;
		float Cal=1, DigOff=0;
		switch (boardMode) {
		case 0: PhysMax=3.3; Cal=0.000050354; break;
		case 1: PhysMin=-5.0; PhysMax=5.0; Cal = 0.00015259; break;
		case 13: PhysMin=-10.24; PhysMax=10.24; Cal = 0.0003125; break;
		default:
			fprintf(stderr,"%s (line %d): Intan/RHD2000 - unknown Boardmode %d\n", __func__,__LINE__,boardMode);
			// boardMode unknown
		};

		QSTRING_T *referenceChannelName = read_qstring(hdr, &pos);

		uint16_t numberOfSignalGroups = leu16p(hdr->AS.Header+pos);
		pos += 2;

		hdr->NS = 1;
		uint32_t chan = 1 ;
		hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));
		CHANNEL_TYPE *hc = hdr->CHANNEL;
		strcpy(hc->Label,"Time");
		hc->Transducer[0] = 0;
		hc->OnOff  = 2;
		hc->GDFTYP = 5; //int32_t
		hc->DigMin = -ldexp(1,31);
		hc->DigMax = ldexp(1,31)-1;
		hdr->SPR   = (hdr->Version < 2.0) ? 60 : 128;

		for (uint16_t k = 0; k<numberOfSignalGroups; k++) {
			QSTRING_T *groupName = read_qstring(hdr, &pos);
			QSTRING_T *groupNamePrefix = read_qstring(hdr, &pos);

			uint16_t enabled = leu16p(hdr->AS.Header+pos);
			pos += 2;
			uint16_t numberChannelsInGroup = leu16p(hdr->AS.Header+pos);
			pos += 2;
			uint16_t numberAmplifierChannels = leu16p(hdr->AS.Header+pos);
			pos += 2;

			if (enabled && (numberChannelsInGroup > 0)) {
				hdr->NS += numberChannelsInGroup;
				hdr->CHANNEL = (CHANNEL_TYPE*)realloc(hdr->CHANNEL, hdr->NS * sizeof(CHANNEL_TYPE));

				for (; chan < hdr->NS; chan++); {
					CHANNEL_TYPE *hc = hdr->CHANNEL + chan;

					QSTRING_T *NativeChannelName = read_qstring(hdr, &pos);
					size_t outlen = MAX_LENGTH_LABEL+1;
					char *outbuf = hc->Label;
					size_t c = convert_qstring_to_utf8(NativeChannelName, &outbuf, &outlen);

					QSTRING_T *CustomChannelName = read_qstring(hdr, &pos);
					outlen = MAX_LENGTH_LABEL+1;
					outbuf = hc->Label;
					c = convert_qstring_to_utf8(CustomChannelName, &outbuf, &outlen);

					uint16_t customOrder = leu16p(hdr->AS.Header+pos);
					pos += 2;
					uint16_t nativeOrder = leu16p(hdr->AS.Header+pos);
					pos += 2;
					uint16_t signalType = leu16p(hdr->AS.Header+pos);
					pos += 2;
					uint16_t channelEnabled = leu16p(hdr->AS.Header+pos);
					hc->OnOff = channelEnabled;
					pos += 2;
					uint16_t chipChannel = leu16p(hdr->AS.Header+pos);
					pos += 2;
					uint16_t boardStream = leu16p(hdr->AS.Header+pos);
					pos += 2;

					uint16_t triggerMode = leu16p(hdr->AS.Header+pos);
					pos += 2;
					int16_t voltageThreshold = leu16p(hdr->AS.Header+pos); // uV
					pos += 2;
					uint16_t triggerChannel = leu16p(hdr->AS.Header+pos);
					pos += 2;
					float ImpedanceMagnitude = lef32p(hdr->AS.Header+pos); // Ohm
					pos += 4;
					float ImpedancePhase = lef32p(hdr->AS.Header+pos); 	// degree
					pos += 4;

					// translate into biosig HDR channel structure
					hc->GDFTYP = 4; 	// uint16_t
					hc->DigMin = 0;
					hc->DigMax = 0xffff;
					hc->SPR    = (signalType<3) ? 60 : 128;
					switch (signalType) {
					case 0: hc->SPR = hdr->SPR;
						hc->Cal = 0.195;
						hc->PhysMin = (hc->DigMin-32768) * hc->Cal;
						hc->PhysMax = (hc->DigMin-32768) * hc->Cal;
						break;
					case 1: hc->SPR = hdr->SPR/4;
						hc->Cal = 0.0000374;
						hc->PhysMin = hc->DigMin * hc->Cal;
						hc->PhysMax = hc->DigMin * hc->Cal;
						break;
					case 2: hc->SPR = 1;
						hc->Cal = 0.0000748;
						hc->PhysMin = hc->DigMin * hc->Cal;
						hc->PhysMax = hc->DigMin * hc->Cal;
						break;
					case 3: // depends on board mode, range is computed above
						hc->SPR = hdr->SPR;
						hc->PhysMin = PhysMin;
						hc->PhysMax = PhysMax;
						hc->Cal = 0.0000748;
						break;
					case 4: hc->SPR = hdr->SPR;
						hc->PhysMin = hc->DigMin;
						hc->PhysMax = hc->DigMax;
						break;
					case 5: hc->SPR = hdr->SPR;
						hc->PhysMin = hc->DigMin;
						hc->PhysMax = hc->DigMax;
						break;
					default:
						;
					}
					hc->Off = hc->PhysMin - hc->DigMin * hc->Cal;
					hc->Transducer[0]=0;

				if (VERBOSE_LEVEL >7)
					fprintf(stdout, "%s (line %d): Intan/RHD2000:  #%d %d %s\n",__FILE__,__LINE__, chan, hc->OnOff, hc->Label);

					if (!(chipChannel<32) || !(signalType<6)) {
						biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan RHD2000 - not conformant to specification");
						return -1;
					}
				}
			}
		}
		hdr->HeadLen = pos;

	biosigERROR(hdr, B4C_FORMAT_UNSUPPORTED, "Format Intan RHD2000 not supported");
	return -1;
}

#ifdef __cplusplus
}
#endif
