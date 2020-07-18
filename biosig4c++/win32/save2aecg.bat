: Converts short-term ECG data into XML-based HL7v3 Annotated ECG format
:
:    Copyright (C) 2008,2020 Alois Schloegl <alois.schloegl@gmail.com>
:    This file is part of the BioSig at http://biosig.sourceforge.io/
echo off
echo REC2BIN is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo SAVE2AECG converts  short-term ECG data into XML-based HL7v3 Annotated ECG format
echo usage: save2aecg source dest
echo usage: save2aecg -f=FMT source dest
echo		FMT can be BDF,BIN,CFWB,EDF,GDF,HL7aECG,SCP_ECG
save2gdf.exe -f=HL7 %1 %2 %3 %4 %5 %6 %7

