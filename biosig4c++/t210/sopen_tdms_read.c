/*

    Copyright (C) 2010-2019 Alois Schloegl <alois.schloegl@ist.ac.at>

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

References:
[1] TDMS File Format Internal Structure, Publish Date: Jun 23, 2014
    NI-Tutorial-5696-en.pdf downloaded from ni.com
[2] npTDMS package for python
    https://github.com/adamreeve/npTDMS.git

 */

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../biosig-dev.h"

typedef struct {
	uint32_t TDMSCONST;
	uint32_t toc_mask;
	uint32_t version;
	uint64_t nextSegmentOffset;
	uint64_t rawDataOffset;
	uint32_t numObjects;
	size_t   data_position;
	size_t   size;
} __attribute__((aligned(4),packed)) tdms_segment_t;
#define LEAD_SIZE 28

#define kTocMetaData 		(1L<<1)
#define kTocRawData 		(1L<<3)
#define kTocDAQmxRawData 	(1L<<7)
#define kTocInterleavedData 	(1L<<5)
#define kTocBigEndian 		(1L<<6)
#define kTocNewObjList		(1L<<2)

typedef enum {
	tdsTypeVoid,
	tdsTypeI8,
	tdsTypeI16,
	tdsTypeI32,
	tdsTypeI64,
	tdsTypeU8,
	tdsTypeU16,
	tdsTypeU32,
	tdsTypeU64,
	tdsTypeSingleFloat,
	tdsTypeDoubleFloat,
	tdsTypeExtendedFloat,
	tdsTypeSingleFloatWithUnit=0x19,
	tdsTypeDoubleFloatWithUnit,
	tdsTypeExtendedFloatWithUnit,
	tdsTypeString=0x20,
	tdsTypeBoolean=0x21,
	tdsTypeTimeStamp=0x44,
	tdsTypeFixedPoint=0x4F,
	tdsTypeComplexSingleFloat=0x08000c,
	tdsTypeComplexDoubleFloat=0x10000d,
	tdsTypeDAQmxRawData=0xFFFFFFFF
} tdsDataType;


/*
	defines single tdms object
*/
typedef struct {
	tdsDataType datatype;
	void *value;
	char *object_path;
} tdms_object_t;

// define object store
typedef struct {
	size_t numObjects;
	tdms_object_t *list;
} tdms_object_store_t;

// find element in object store
ssize_t find_element(tdms_object_store_t *objList, const char *key) {
	for (size_t k=0; k < objList->numObjects; k++) {
		if (!strcmp(objList->list[k].object_path, key)) return k;
	}
	return -1;
}

// add (or replace) element in object store
void add_element(tdms_object_store_t *objList, tdms_object_t element) {
	ssize_t idx = find_element(objList, element.object_path);
	if (idx < 0) {
		objList->numObjects++;
		objList->list = realloc(objList->list, objList->numObjects * sizeof(tdms_object_t));
		idx = objList->numObjects - 1;
	}
	memcpy(objList->list+idx, &element, sizeof(tdms_object_t));
}

// read number of objects into object store
void read_tdms_objects(tdms_object_store_t O, void *ptr, uint32_t numObj) {

	// this function is under construction
	return;

	tdms_object_t element;

	// skip first 4 bytes, theise contains number of objects
			uint32_t numberOfObjects = leu32p(ptr);
			ptr += 4;
			char *pstr = NULL;
			for (uint32_t k = 0; k < numObj; k++) {
				uint32_t plen = leu32p(ptr);

				pstr  = realloc(pstr,plen+1);
				memcpy(pstr, ptr+4, plen);
				pstr[plen] = 0;

	if (VERBOSE_LEVEL > 8) fprintf(stdout,"%s (line %i): Object %d [%d]<%s>\n",__func__,__LINE__, k, plen, pstr);

				ptr += 4 + plen;
				uint32_t idx  = leu32p(ptr);
				ptr += 4;
				uint32_t numberOfProperties = leu32p(ptr);
				ptr += 4;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %i): Object %i/%i <%s> ((%d) has %i properties \n",__func__,__LINE__, k, numberOfObjects, pstr, idx, numberOfProperties);
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i/%i path='%s' rawDataIdx=0x%08x\n",__func__,__LINE__,   k, numberOfObjects, pstr, idx);

				char *propName = NULL;
				char *propVal  = NULL;
				for (uint32_t p = 0; p < numberOfProperties; p++) {
					// property name
					uint32_t plen = leu32p(ptr);
					propName  = realloc(propName,plen+1);
					memcpy(propName, ptr+4, plen);
					propName[plen] = 0;
					ptr += 4+plen;

	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i/%i <%s>\n",__func__,__LINE__, k, p, numberOfProperties, propName);

					// property type
					uint32_t propType = leu32p(ptr);
					ptr += 4;

					// property value
					int32_t val_i32;
					switch (propType) {
					case tdsTypeVoid:
					case tdsTypeI8:
					case tdsTypeI16:

					case tdsTypeI32: {
						// int32
						int32_t val = lei32p(ptr);
						ptr += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeI64:
					case tdsTypeU8:
					case tdsTypeU16:

					case tdsTypeU32: {
						// uint32
						uint32_t val = leu32p(ptr);
						ptr += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}

					case tdsTypeU64:
					case tdsTypeSingleFloat:
					case tdsTypeDoubleFloat:
					case tdsTypeExtendedFloat:
					case tdsTypeSingleFloatWithUnit: // =0x19,
					case tdsTypeDoubleFloatWithUnit:
					case tdsTypeExtendedFloatWithUnit:

					case tdsTypeString: 	// case 0x20:
						plen = leu32p(ptr);
						propVal  = realloc(propVal,plen+1);
						memcpy(propVal,ptr+4,plen);
						propVal[plen]=0;
						ptr += 4+plen;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(string)=<%s>\n",__func__,__LINE__, k, p,propName, propVal);
						break;

					case tdsTypeBoolean:

					case tdsTypeTimeStamp: // = 0x44
						ptr += 4+plen;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(string)=<%s>\n",__func__,__LINE__, k, p,propName, propVal);
						break;

					case tdsTypeFixedPoint: // 0x4F,
					case tdsTypeComplexSingleFloat: // =0x08000c,
					case tdsTypeComplexDoubleFloat: // =0x10000d,
					case tdsTypeDAQmxRawData: // =0xFFFFFFFF

					default:
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %d property %d <%s>type=0x%x not supported. Skip %d bytes\n",__func__,__LINE__, k, p,propName,propType, plen);
					}
				}


				switch (idx) {
				case 0xffffffff :	// no raw data
					break;
				case 0x00001269 :	// DAQmx Format Changing scaler
					break;
				case 0x00001369 :	// DAQmx Digital Line scaler
					break;
				case 0x00000000 : 	//
					;
				}
			}
	if (pstr) free(pstr);
}


EXTERN_C void sopen_tdms_read(HDRTYPE* hdr) {
/*
	this function will be called by the function SOPEN in "biosig.c"

	Input:
		char* Header	// contains the file content

	Output:
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/

	// read whole file into memory
	size_t count = hdr->HeadLen;
	while (!ifeof(hdr)) {
		size_t bufsiz = 2*count;
		hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, bufsiz+1);
		count  += ifread(hdr->AS.Header+count, 1, bufsiz-count, hdr);
	}
	hdr->AS.Header[count]=0;
	hdr->HeadLen = count;

	tdms_object_store_t ObjectStore;
	ObjectStore.numObjects=0;
	ObjectStore.list=NULL;

	/*
		Specification obtained from http://www.ni.com/white-paper/5696/en
		however, there are also TDMS data out there, that is based on an XML header
		and a separate binary file. This is somewhat confusing.
	 */
	fprintf(stderr,"%s (line %i): Format TDMS is very experimental\n",__func__,__LINE__);


	/***** Lead In *****/
	int currentSegment = 0;
	int currentSegmentTableSize = 16;

	// top level comprised of a single object that holds file-specific information like author or title.
	tdms_segment_t *segTable = malloc(currentSegmentTableSize * sizeof(tdms_segment_t));
	memcpy(segTable, hdr->AS.Header, LEAD_SIZE+4);

	hdr->FILE.LittleEndian    = !(segTable[0].toc_mask & kTocBigEndian);
	if ( (__BYTE_ORDER == __LITTLE_ENDIAN) != !(segTable[0].toc_mask & kTocBigEndian) ) {
		segTable[0].version           = bswap_32(segTable[0].version);
		segTable[0].nextSegmentOffset = bswap_64(segTable[0].nextSegmentOffset);
		segTable[0].rawDataOffset     = bswap_64(segTable[0].rawDataOffset);
		segTable[0].numObjects        = bswap_32(segTable[0].numObjects);
	}
	segTable[currentSegment].data_position = LEAD_SIZE + segTable[currentSegment].rawDataOffset;	// data position
	segTable[currentSegment].size = (segTable[currentSegment].toc_mask & kTocRawData) ?
					(segTable[currentSegment].nextSegmentOffset - segTable[currentSegment].rawDataOffset) : 0;

	switch (segTable[0].version) {
	case 4712: hdr->Version = 1.0; break;
	case 4713: hdr->Version = 2.0; break;
	default:
		biosigERROR(hdr,B4C_FORMAT_UNSUPPORTED,"This version of format TDMS is currently not supported");
	}

	read_tdms_objects(ObjectStore, hdr->AS.Header+LEAD_SIZE ,segTable[0].numObjects);

	size_t pos = LEAD_SIZE + segTable[currentSegment].nextSegmentOffset;

	// read table of indices for all segments
	while (++currentSegment) {
		// manage memory allocation of index table
		if (currentSegment >= currentSegmentTableSize) {
			currentSegmentTableSize *= 2;
			segTable = realloc(segTable, currentSegmentTableSize * sizeof(tdms_segment_t));
		}

		// read next segment
		memcpy(segTable + currentSegment, hdr->AS.Header + pos, LEAD_SIZE+4); 	// copy global, and 1 segment
		if ( (__BYTE_ORDER == __LITTLE_ENDIAN) != !(segTable[currentSegment].toc_mask & kTocBigEndian) ) {
			segTable[currentSegment].version           = bswap_32(segTable[currentSegment].version);
			segTable[currentSegment].nextSegmentOffset = bswap_64(segTable[currentSegment].nextSegmentOffset);
			segTable[currentSegment].rawDataOffset     = bswap_64(segTable[currentSegment].rawDataOffset);
			segTable[currentSegment].numObjects        = bswap_32(segTable[currentSegment].numObjects);
		}
		segTable[currentSegment].data_position = pos + LEAD_SIZE + segTable[currentSegment].rawDataOffset;
		segTable[currentSegment].size = (segTable[currentSegment].toc_mask & kTocRawData) ?
					(segTable[currentSegment].nextSegmentOffset - segTable[currentSegment].rawDataOffset) : -1;

		read_tdms_objects(ObjectStore, hdr->AS.Header+pos+LEAD_SIZE, segTable[currentSegment].numObjects);

		if (segTable[currentSegment].nextSegmentOffset == (uint64_t)(-1LL)) break;
		if (segTable[currentSegment].nextSegmentOffset == 0) break;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %d): %s\n\tLittleEndian=%d\n\tVersion=%d\n\t@beginSeg %ld\n\t@NextSegment %ld\n\t@rawDataOffset %ld\n",__func__,__LINE__, hdr->AS.Header,hdr->FILE.LittleEndian, segTable[currentSegment].version, pos,  segTable[currentSegment].nextSegmentOffset, segTable[currentSegment].rawDataOffset);

		pos += LEAD_SIZE + segTable[currentSegment].nextSegmentOffset;
	}

	if (VERBOSE_LEVEL > 7) {
		// show index table segTable
		fprintf(stdout, "#seg\tTMDs\tmask\tver\tnumObj\tnextSegOff\trawDataOffset\tDataPosition\tsize\n");
		char tmpstr[8]; tmpstr[4]=0;
		for (int k = 0; k < currentSegment; k++) {
			memcpy(tmpstr,&segTable[k].TDMSCONST,4);
			fprintf(stdout, "%d\t%04s\t0x%04x\t%d\t%d\t%9ld\t%9ld\t%9ld\t%ld\n", k,
				tmpstr, segTable[k].toc_mask, segTable[k].version, segTable[k].numObjects,
				segTable[k].nextSegmentOffset, segTable[k].rawDataOffset, segTable[k].data_position, segTable[k].size);
		}
	}

	uint64_t nextSegmentOffset = leu64p(hdr->AS.Header+12);
	uint64_t RawDataOffset     = leu64p(hdr->AS.Header+20);
	pos = 28;

	/***** Meta data *****/
	if (1) {
	//while (pos<RawDataOffset) {
			uint32_t numberOfObjects = leu32p(hdr->AS.Header+pos);
			pos += 4;
			char *pstr=NULL;
			for (uint32_t k=0; k < numberOfObjects; k++) {
				uint32_t plen = leu32p(hdr->AS.Header+pos);
	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %i): %i %i %i\n",__func__,__LINE__, (int)hdr->HeadLen, (int)pos,plen);
				pstr  = realloc(pstr,plen+1);
				memcpy(pstr,hdr->AS.Header+pos+4,plen);
				pstr[plen]=0;

				pos += 4+plen;
				uint32_t idx  = leu32p(hdr->AS.Header+pos);
				pos += 4;
				uint32_t numberOfProperties  = leu32p(hdr->AS.Header+pos);
				pos += 4;

	if (VERBOSE_LEVEL>8) fprintf(stdout,"%s (line %i): Object %i/%i <%s> ((%d) has %i properties \n",
				__func__,__LINE__, k,numberOfObjects,pstr,idx,numberOfProperties);

				char *propName=NULL;
				char *propVal=NULL;
				for (uint32_t p=0; p < numberOfProperties; p++) {
					// property name
					uint32_t plen = leu32p(hdr->AS.Header+pos);
					propName  = realloc(propName,plen+1);
					memcpy(propName,hdr->AS.Header+pos+4,plen);
					propName[plen]=0;
					pos += 4+plen;

					// property type
					uint32_t propType = leu32p(hdr->AS.Header+pos);
					pos += 4;

	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i/%i %d<%s> 0x%08x\n",__func__,__LINE__, k, p, numberOfProperties, plen,propName, propType);


					// property value
					int32_t val_i32;
					switch (propType) {
					case tdsTypeVoid:
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(void)\n",__func__,__LINE__, k, p,propName);
						break;
					case tdsTypeI8: {
						int8_t val = (int8_t)hdr->AS.Header[pos];
						pos += 1;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int8)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeI16: {
						int16_t val = lei16p(hdr->AS.Header+pos);
						pos += 2;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int16)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeI32: {
						// int32
						int32_t val = lei32p(hdr->AS.Header+pos);
						pos += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeI64: {
						// int64
						int64_t val = lei64p(hdr->AS.Header+pos);
						pos += 8;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int64)=%ld\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeU8: {
						uint8_t val = (uint8_t)hdr->AS.Header[pos];
						pos += 1;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(uint8)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeU16: {
						uint16_t val = leu16p(hdr->AS.Header+pos);
						pos += 1;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(uint16)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}

					case tdsTypeU32: {	// case 0x07:
						// uint32
						uint32_t val = leu32p(hdr->AS.Header+pos);
						pos += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(int32)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}

					case tdsTypeU64: {
						// uint64
						uint64_t val = leu64p(hdr->AS.Header+pos);
						pos += 8;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(uint64)=%ld\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeSingleFloat: {
						float val = lef32p(hdr->AS.Header+pos);
						pos += 4;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(float32)=%g\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeDoubleFloat: {
						// double
						double val = lef64p(hdr->AS.Header+pos);
						pos += 8;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(float64)=%ld\n",__func__,__LINE__, k, p,propName,val);
						break;
						}
					case tdsTypeExtendedFloat:{
						// double
						// double val = lef128p(hdr->AS.Header+pos);
						pos += 16;
						break;
						}
					case tdsTypeSingleFloatWithUnit: // =0x19,
					case tdsTypeDoubleFloatWithUnit:
					case tdsTypeExtendedFloatWithUnit:

					case tdsTypeString:	// case 0x20:
						plen = leu32p(hdr->AS.Header+pos);
						propVal  = realloc(propVal,plen+1);
						memcpy(propVal,hdr->AS.Header+pos+4,plen);
						propVal[plen]=0;
						pos += 4+plen;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(string)=<%s>\n",__func__,__LINE__, k, p,propName, propVal);
						break;

					case tdsTypeBoolean: {
						uint8_t val = (uint8_t)hdr->AS.Header[pos];
						pos += 1;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(boolean)=%i\n",__func__,__LINE__, k, p,propName,val);
						break;
						}

					case tdsTypeTimeStamp: {	//=0x44
						// NI Timestamps according to http://www.ni.com/product-documentation/7900/en/
						int64_t time_seconds = lei64p(hdr->AS.Header+pos);
						uint64_t time_fraction = leu64p(hdr->AS.Header+pos+8);
						gdftime_t T0 = ldexp((time_seconds+ldexp(time_fraction,-64))/(24.0*3600.0)+695422,32);
						pos += 16;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i timestamp %ld %.8g \n",__func__,__LINE__, k, time_seconds, ldexp(time_fraction,-64));
						break;
						}

					case tdsTypeFixedPoint: // 0x4F,
						pos += 8;
						break;

					case 0x61:
						//plen = leu32p(hdr->AS.Header+pos);
						pos += 16;
						break;

					case tdsTypeComplexSingleFloat: { // =0x08000c,
						float val[2];
						val[0] = lef32p(hdr->AS.Header+pos);
						val[1] = lef32p(hdr->AS.Header+pos);
						pos += 16;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(float64)=%g + i%h\n",__func__,__LINE__, k, p, propName, val[0], val[1]);
						break;
						}
					case tdsTypeComplexDoubleFloat: {// =0x10000d,
						double val[2];
						val[0] = lef64p(hdr->AS.Header+pos);
						val[1] = lef64p(hdr->AS.Header+pos);
						pos += 16;
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i property %i <%s>(float64)=%g + i%h\n",__func__,__LINE__, k, p, propName, val[0], val[1]);
						break;
						}
					case tdsTypeDAQmxRawData: { // =0xFFFFFFFF
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %d property %d <%s>type=0x%x not supported. Skip %d bytes\n",__func__,__LINE__, k, p,propName,propType, plen);
						break;
						}

					default:
	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %d property %d <%s>type=0x%x not supported. Skip %d bytes\n",__func__,__LINE__, k, p,propName,propType, plen);
					}
				}


	if (VERBOSE_LEVEL>6) fprintf(stdout,"%s (line %i): object %i/%i path='%s' rawDataIdx=0x%08x\n",__func__,__LINE__, k, numberOfObjects, pstr, idx);

				switch (idx) {
				case 0xffffffff :	// no raw data
					break;
				case 0x00001269 :	// DAQmx Format Changing scaler
					break;
				case 0x00001369 :	// DAQmx Digital Line scaler
					break;
				case 0x00000000 : 	//
					;
				}
			}
	}

	if (segTable) free(segTable);
	biosigERROR(hdr,B4C_FORMAT_UNSUPPORTED,"Format TDMS is currently not supported");
}

