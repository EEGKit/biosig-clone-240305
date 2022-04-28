/*
//-------------------------------------------------------------------
//   C-MEX implementation of ACCOVF - this function is part of the NaN-toolbox. 
//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//
// accovf: auto- and crosscovariance function 
// usage:
//	[Sxx,Nxx,Sxy,Nxy,lag] = accovf(X,Y,maxlag);
//	[...                ] = accovf(X,Y,maxlag,tix);
//	[Sxx,Nxx,lag] = accovf(X,maxlag,tix);
//	[Sxx,Nxx,lag] = accovf(X,maxlag);
//      Sxx./Nxx : auto-covariance function
//      Sxy./Nxy : cross-covariance function
	
       The purpose is speeding up these computations: 
	
        for k = -maxlag : maxlag,
                Rxy(k+1+maxlag) = mean(X(ixtrain-k) .* Y(ixtrain));
                Rxx(k+1+maxlag) = mean(X(ixtrain-k) .* X(ixtrain));
        end; 


//    Copyright (C) 2022 Alois Schloegl <alois.schloegl@gmail.com>
//    This function is part of the NaN-toolbox
//    http://pub.ist.ac.at/~schloegl/matlab/NaN/
//
//-------------------------------------------------------------------
*/

#ifdef __GNUC__ 
        #include <stdint.h>
#endif
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mex.h"
// #include "matrix.h"

/*#define NO_FLAG*/

/* 
   math.h has isnan() defined for all sizes of floating point numbers, 
   but c++ assumes isnan(double), causing possible conversions for float and long double 
 */
#define ISNAN(a) (a!=a)

#ifndef typeof
#define typeof __typeof__
#endif

void mexFunction(int POutputCount,  mxArray* POutput[], int PInputCount, const mxArray *PInputs[]) {

    	double 		*X0=NULL, *Y0=NULL, *W=NULL;
    	double	 	*CC;
    	double 		*NN = NULL;

    	size_t		rX,cX,rY,cY;
    	size_t    	i; 
	char	 	flag_isNaN = 0;
        int             ACC_LEVEL;

	/*********** check input arguments *****************/

	// check for proper number of input and output arguments
	if ((PInputCount <= 0) || (PInputCount > 4)) {
	        mexPrintf("usage:\n\t[Sxx,Nxx,Sxy,Nxy,lag] = accovf_mex(X, Y, lag)\n");
	        mexPrintf(        "\t[Sxx,Nxx,Sxy,Nxy,lag] = accovf_mex(X, Y, lag, tix)\n");
	        // mexPrintf("\t[Sxx,Nxx] = accovf_mex(X,lag)\n");
	        // mexPrintf("\t[Sxx,Nxx] = accovf_mex(X,lag,tix)\n");
	        return;
	}
	if (POutputCount > 5)
	        mexErrMsgTxt("accovf_mex.mex has no more then 5 output arguments.");

	// get 1st argument
	if(mxIsDouble(PInputs[0]) && !mxIsComplex(PInputs[0]) && !mxIsSparse(PInputs[0]) )
		X0  = mxGetPr(PInputs[0]);
	else 	
		mexErrMsgTxt("First argument must be non-sparse REAL/DOUBLE.");
		
	// get 2nd argument
	if (mxIsDouble(PInputs[1]) && !mxIsComplex(PInputs[1]) && !mxIsSparse(PInputs[1]))
		Y0  = mxGetPr(PInputs[1]);
	else 	
		mexErrMsgTxt("Second argument must be non-sparse REAL/DOUBLE.");

	long MAXLAG=1000; // DEFAULT
	rX = mxGetM(PInputs[0]);		
	cX = mxGetN(PInputs[0]);		
	rY = mxGetM(PInputs[1]);		
	cY = mxGetN(PInputs[1]);
	int argix_lag=2; //default
	double *tix = NULL;
	if (rY==1 && cY==1) {
		Y0 = NULL;
		rY = 0; 
		cY = 0;	
		argix_lag=1;
	}
	if (PInputCount > argix_lag) {
		MAXLAG = (long)(*(double*)mxGetPr(PInputs[argix_lag]));
	}
	
	if (cY>0 && rX != rY)
		mexErrMsgTxt("number of rows in X and Y do not match");
	if (cX != 1 || cY > 1)
		mexErrMsgTxt("number of columns must be 1");

	if (PInputCount > (argix_lag+1)) {
		tix = (double*)mxGetPr(PInputs[argix_lag+1]);
		rY  = mxGetM(PInputs[argix_lag+1]);		
		rY *= mxGetN(PInputs[argix_lag+1]);
	}

	/*********** create output arguments *****************/
	const mwSize DIMS[2] = {2*MAXLAG+1, 1};

	POutput[0] = mxCreateDoubleMatrix(DIMS[0], DIMS[1], mxREAL);
	POutput[1] = mxCreateDoubleMatrix(DIMS[0], DIMS[1], mxREAL);
	double *Sxx = mxGetPr(POutput[0]);
	double *Nxx = mxGetPr(POutput[1]);

	POutput[2] = mxCreateDoubleMatrix(DIMS[0], DIMS[1], mxREAL);
	POutput[3] = mxCreateDoubleMatrix(DIMS[0], DIMS[1], mxREAL);
	POutput[4] = mxCreateDoubleMatrix(DIMS[0], DIMS[1], mxREAL);

	double *Sxy = mxGetPr(POutput[2]);
	double *Nxy = mxGetPr(POutput[3]);
	double *LAG = mxGetPr(POutput[4]);
	
	for (long k=-MAXLAG; k<=MAXLAG; k++) {
		size_t ix = k+MAXLAG;
		Sxx[ix] = 0.0;	
		Nxx[ix] = 0;	
		Sxy[ix] = 0.0;	
		Nxy[ix] = 0;	
		LAG[ix] = k;	
	}

	uint32_t *nxx = (uint32_t*)mxMalloc((2*MAXLAG+1)*sizeof(uint32_t));
	uint32_t *nxy = (uint32_t*)mxMalloc((2*MAXLAG+1)*sizeof(uint32_t));
	memset(nxx,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
	memset(nxy,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
	
	if (tix==NULL) {
	//	[Sxx,Nxx,Sxy,Nxy,lag] = accovf(X,Y,maxlag);
		for (size_t t=0; t < rX; t++) {
			if ( (t % (1<<30) == 0) ) {
				for (long k=0; k<=2*MAXLAG; k++) {
					Nxx[k] += nxx[k]; 
					Nxy[k] += nxy[k];
				}
				memset(nxx,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
				memset(nxy,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
			}	

		for (long k=0; k<=2*MAXLAG; k++) {
			long k0 = k-MAXLAG;
			if ( (t-k0) < 0) continue;
			if ( (t-k0) >= rX ) continue;
			double a = X0[t-k0] * X0[t];
			double b = X0[t-k0] * Y0[t];
			if (a==a) {
				Sxx[k] += a;
				nxx[k] += 1;
			}
			if (b==b) {
				Sxy[k] += b;
				nxy[k] += 1;
			}
		}
		}
	}
	else {	
	//	[Sxx,Nxx,Sxy,Nxy,lag] = accovf(X,Y,maxlag,tix);
		for (size_t l=0; l < rY; l++) {
			if ( (l % (1<<30) == 0) ) {
				for (long k=0; k<=2*MAXLAG; k++) {
					Nxx[k] += nxx[k]; 
					Nxy[k] += nxy[k];
				}
				memset(nxx,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
				memset(nxy,0,(2*MAXLAG+1)*sizeof(uint32_t)); 
			}	
			
		size_t t = (size_t)tix[l];
		for (long k=0; k<=2*MAXLAG; k++) {
			long k0 = k-MAXLAG;
			if ( (t-k0) < 0) continue;
			if ( (t-k0) >= rX ) continue;
			double a = X0[t-k0] * X0[t];
			double b = X0[t-k0] * Y0[t];
			if (a==a) {
				Sxx[k] += a;
				nxx[k] += 1;
			}
			if (b==b) {
				Sxy[k] += b;
				nxy[k] += 1;
			}
		}
		}
	}
	
	for (long k=0; k<=2*MAXLAG; k++) {
		Nxx[k] += nxx[k]; 
		Nxy[k] += nxy[k];
	}
	if (nxx) mxFree(nxx);
	if (nxy) mxFree(nxy);

}

