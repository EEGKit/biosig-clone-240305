: Converts data into ascii-Header and each channel into a separate binary data file
:
:    Copyright (C) 2008,2020 Alois Schloegl <alois.schloegl@gmail.com>
:    This file is part of the BioSig at https://biosig.sourceforge.io
echo off
echo REC2BIN is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo REC2BIN converts different biosignal data formats into the BIN format
echo usage: rec2bin source dest
echo usage: rec2bin -f=FMT source dest
echo		FMT can be BDF,BIN,CFWB,EDF,GDF,HL7aECG,SCP_ECG
save2gdf.exe -f=BIN %1 %2 %3 %4 %5 %6 %7


