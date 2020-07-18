: Converts short-term ECG data into EN1094 SCP-ECG format
:
:    Copyright (C) 2008,2020 Alois Schloegl <alois.schloegl@gmail.com>
:    This file is part of the BioSig at http://biosig.sourceforge.io/
echo off
echo SAVE2SCP is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo SAVE2SCO converts  short-term ECG data into EN1094 SCP-ECG format
echo usage: save2scp source dest
echo usage: save2scp -f=FMT source dest
echo		FMT can be BDF,BIN,CFWB,EDF,GDF,HL7aECG,SCP_ECG
save2gdf.exe -f=SCP_ECG %1 %2 %3 %4 %5 %6 %7

