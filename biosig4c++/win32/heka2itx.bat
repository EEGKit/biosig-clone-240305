: Conversion of HEKA/Patchmaster data into ITX (Igor Text) format
:
:    Copyright (C) 2008, 2011,2020 Alois Schloegl <alois.schloegl@gmail.com>
:    This file is part of the BioSig http://biosig.sourceforge.io/
echo off
echo HEKA2ITX is part of BioSig http://biosig.sf.net and licensed with GNU GPL v3.
echo HEKA2ITX converts HEKA files into ITX (IgorProText) files
echo usage: heka2itx source dest
echo usage: heka2itx -SWEEP=ne,ng,ns source dest
echo          selects sweep ns from group ng from experiment ne.
echo          use 0 as wildcard selecting all sweeps fullfilling the criteria
save2gdf.exe -f=ITX %1 %2 %3 %4 %5 %6 %7
