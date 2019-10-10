/*

    Copyright (C) 2011,2013,2015,2019 Alois Schloegl <alois.schloegl@ist.ac.at>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

 */

#include "mex.h"
#include <physicalunits.h>

#ifdef tmwtypes_h
  #if (MX_API_VER<=0x07020000)
    typedef int mwSize;
  #endif 
#endif 

// read numeric value and convert to uint16_t
uint16_t getUINT16(const mxArray *pm, size_t idx) {
	size_t n = mxGetNumberOfElements(pm);
	if (n == 0)   return(0);
	if (n <= idx) idx = 0;

	switch (mxGetClassID(pm)) {
/*
	case mxCHAR_CLASS:
	case mxLOGICAL_CLASS:
	case mxINT8_CLASS:
		return(*((int8_t*)mxGetData(pm) + idx));
	case mxUINT8_CLASS:
		return(*((uint8_t*)mxGetData(pm) + idx));
*/
	case mxDOUBLE_CLASS:
		return(*((double*)mxGetData(pm) + idx));
	case mxSINGLE_CLASS:
		return(*((float*)mxGetData(pm) + idx));
	case mxINT16_CLASS:
		return(*((int16_t*)mxGetData(pm) + idx));
	case mxUINT16_CLASS:
		return(*((uint16_t*)mxGetData(pm) + idx));
	case mxINT32_CLASS:
		return(*((int32_t*)mxGetData(pm) + idx));
	case mxUINT32_CLASS:
		return(*((uint32_t*)mxGetData(pm) + idx));
	case mxINT64_CLASS:
		return(*((int64_t*)mxGetData(pm) + idx));
	case mxUINT64_CLASS:
		return(*((uint64_t*)mxGetData(pm) + idx));
/*
	case mxFUNCTION_CLASS:
	case mxUNKNOWN_CLASS:
	case mxCELL_CLASS:
	case mxSTRUCT_CLASS:
*/
	default:
		return(0);
	}
	return(0);
}

void mexFunction(
    int           nlhs,           /* number of expected outputs */
    mxArray       *plhs[],        /* array of pointers to output arguments */
    int           nrhs,           /* number of inputs */
    const mxArray *prhs[]         /* array of pointers to input arguments */
) {

const char HELPTEXT[] = 
"PHYSICALUNITS converts PhysDim inte PhysDimCode and vice versa\n"
" according to Annex A of FEF Vital Signs Format [1] \n"
"\n" 
"   HDR = physicalunits(HDR); \n"
"	adds HDR.PhysDim or HDR.PhysDimCode, if needed \n"
"\n"
"   PhysDim = physicalunits(PhysDimCode);\n"
"	converts Code of PhysicalUnits into descriptive physical units\n"
"\n"
"   PhysDimCode = physicalunits(PhysDim);\n"
"	converts descriptive units into Code for physical units.\n"
"\n"
"   [..., scale] = physicalunits(...);\n"
"	scale contains the scaling factor of the decimal prefix\n"
"\n"
" Reference(s):\n"
" ISO/IEEE 11073-10101:2004\n"
"   Health Informatics - Point-of-care medical device communications - Part 10101: Nomenclature\n"
"   p.62-75. Table A.6.3: Vital signs units of measurements\n";

	if (nrhs<1) {
		mexPrintf( HELPTEXT);
		return; 
	}

    const mxArray *mxPhysDim_Input = NULL;
    const mxArray *mxPhysDimCode_Input = NULL;
    mxArray *mxPhysDim_Output = NULL;
    mxArray *mxPhysDimCode_Output = NULL;
    mxArray *mxScale = NULL;

    mwSize nDims = 0; 
    mwSize Numel = 1; 
    const mwSize *SZ   = NULL; 
	mwSize k;

    /*************************************************************
         Input
     *************************************************************/
    mxClassID Type = mxGetClassID(prhs[0]);
	/* process input arguments */
	switch (Type) {
	case mxSTRUCT_CLASS: {
		mxPhysDim_Input = mxGetField(prhs[0], 0, "PhysDim");
	        mxPhysDimCode_Input = mxGetField(prhs[0], 0, "PhysDimCode");
		break;
		}
	case mxINT16_CLASS:
	case mxINT32_CLASS:
	case mxINT64_CLASS:
	case mxUINT16_CLASS:
	case mxUINT32_CLASS:
	case mxUINT64_CLASS:
	case mxSINGLE_CLASS:
	case mxDOUBLE_CLASS: {
		mxPhysDimCode_Input = prhs[0];
		break;
		}
    //case mxCHAR_CLASS:
	case mxCELL_CLASS: {
		mxPhysDim_Input = prhs[0];
		break;
		}
	default: {
		mexPrintf("input arguments not supported\n");
		return;
		}
	}

    // Get dimensions of input argument and set dimensions of output
	if (mxPhysDimCode_Input && !mxPhysDim_Input) {
		nDims  = mxGetNumberOfDimensions(mxPhysDimCode_Input);
		SZ     = mxGetDimensions(mxPhysDimCode_Input);
		Numel  = mxGetNumberOfElements(mxPhysDimCode_Input);
	}
	else if (mxPhysDim_Input && !mxPhysDimCode_Input ) {
		nDims  = mxGetNumberOfDimensions(mxPhysDim_Input);
		SZ     = mxGetDimensions(mxPhysDim_Input);
		Numel  = mxGetNumberOfElements(mxPhysDim_Input);
	}

	else if (!mxPhysDim_Input && !mxPhysDimCode_Input) {
		mexPrintf("Warning %s(..): Neither PhysDim nor PhysDimCode defined\n",__FILE__);
		plhs[0] = mxDuplicateArray(prhs[0]);
		return;
	}
	else if (!mxIsCell(mxPhysDim_Input) ) {
		mexErrMsgIdAndTxt(__FILE__":PhysDim_not_a_cell_array", "ERROR: PhysDim must be a cell array, but is not.\n");
		return;
	}
	else {
		Numel = mxGetNumberOfElements(mxPhysDimCode_Input);
		if (Numel != mxGetNumberOfElements(mxPhysDim_Input)) {
			mexErrMsgIdAndTxt(__FILE__":Size_of_PhysDim_PhysDimCode_do_not_match", "ERROR: number of elements in PhysDim and PhysDimCode do not match.\n");
			return;
		}

		int errorFlag=0;
		for (k=0; k < Numel; k++) {
			uint16_t PDC1 = getUINT16(mxPhysDimCode_Input, k);

			const int STRLEN=63;
			char tmpstring[STRLEN+1];
			mxArray *tmpArr = mxGetCell(mxPhysDim_Input, k);
			mxGetString(tmpArr, tmpstring, STRLEN);
			uint16_t PDC2 = PhysDimCode(tmpstring);
			if (PDC1 != PDC2) {
				mexPrintf("ERROR %s(...): element %d doe not match %d~=%d, <%s>~=<%s>\n",__FILE__, k, PDC1,PDC2,PhysDim3(PDC1),tmpstring);
				errorFlag=1;
			}
		}
		if (errorFlag) {
			mexErrMsgIdAndTxt(__FILE__":some_elements_in_PhysDim_PhysDimCode_do_not_match", "ERROR: some elements in PhysDim and PhysDimCode do not match.\n");
			return;
		}
		return;
	}

	// allocate 2nd output argument
	if ((nlhs > 1) && (nrhs > 0)) {
		mxScale = mxCreateNumericArray(nDims, SZ, mxDOUBLE_CLASS, mxREAL);
		plhs[1] = mxScale;
	}

    /*************************************************************
         Conversion
     *************************************************************/
	if ( mxPhysDimCode_Input && !mxPhysDim_Input ) {
		mxPhysDim_Output = mxCreateCellArray(nDims, SZ);
		// Conversion: PhysDimCode -> PhysDim
		for (k=0; k < Numel; k++) {
		    uint16_t PDC = getUINT16(mxPhysDimCode_Input, k);
		    mxSetCell(mxPhysDim_Output, k, mxCreateString( PhysDim3( PDC )));
		    if (mxScale != NULL)
		        *((double*)mxGetData(mxScale)+k) = PhysDimScale(PDC);
		}
	}
	else if ( mxPhysDim_Input && !mxPhysDimCode_Input && mxIsCell(mxPhysDim_Input) ) {
		mxPhysDimCode_Output = mxCreateNumericArray(nDims, SZ, mxUINT16_CLASS, mxREAL);
		// Conversion: PhysDim -> PhysDimCode
		for (k=0; k < Numel; k++) {
		    const int STRLEN=63;
		    char tmpstring[STRLEN+1];

		    mxArray *tmpArr = mxGetCell(mxPhysDim_Input, k);
		    mxGetString(tmpArr, tmpstring, STRLEN);
		    uint16_t PDC = PhysDimCode(tmpstring);

		    *(((uint16_t*)mxGetData(mxPhysDimCode_Output))+k) = PDC;
		    if (mxScale != NULL)
		        *((double*)mxGetData(mxScale)+k) = PhysDimScale(PDC);
		}
	}
	else {
		mexPrintf("ERROR %s line %d\n",__FILE__,__LINE__);
		return;
	}

	/*************************************************************
		Output
	 *************************************************************/
	if ( mxIsStruct(prhs[0]) ) {
		plhs[0] = mxDuplicateArray(prhs[0]);
		if (mxPhysDimCode_Output) {
		     int n = mxAddField( plhs[0], "PhysDimCode");
		     mxSetFieldByNumber( plhs[0], 0, n, mxDuplicateArray(mxPhysDimCode_Output) );
		}
		if (mxPhysDim_Output) {
		     int n = mxAddField( plhs[0], "PhysDim");
		     mxSetFieldByNumber( plhs[0], 0, n, mxDuplicateArray(mxPhysDim_Output) );
		}
	}
	else if (mxPhysDim_Input && !mxPhysDimCode_Input) {
		plhs[0] = mxPhysDimCode_Output;
	}
	else if (mxPhysDimCode_Input && !mxPhysDim_Input) {
		plhs[0] = mxPhysDim_Output;
	}
	else {
		mexPrintf("ERROR %s line %d\n",__FILE__,__LINE__);
		return;
	}
}
