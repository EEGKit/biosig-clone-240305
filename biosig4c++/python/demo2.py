####### Demo for Python interface to BioSig" #####################
###
###  Copyright (C) 2009,2016 Alois Schloegl <alois.schloegl@ist.ac.at>
###  This file is part of the "BioSig for Python" repository
###  at http://biosig.sf.net/
###
##############################################################

# download and extract 
#   http://www.biosemi.com/download/BDFtestfiles.zip 
# into /tmp/
# then run this demo 
#
# on linux you can run instead  
#   make test 

import sys
import biosig
import numpy as np
import matplotlib.pyplot as plt
import json

def printf(format, *args):
    sys.stdout.write(format % args)

## read header 
FILENAME="/home/schloegl/data/test/cfs/BaseDemo/Leak.cfs"
FILENAME="/home/schloegl/data/test/cfs/20190417_A0AA.dat"
FILENAME="/home/schloegl/data/test/cfs/minis.dat"
FILENAME="data/Newtest17-256.bdf"

HDR=biosig.header(FILENAME)
#print HDR
## extracting header fields
H=json.loads(HDR)
print(H["Filename"])
print(H["TYPE"])
print(H["Samplingrate"])
Fs=H["Samplingrate"]
NS=len(H["CHANNEL"])	# number of channels
T0=H["StartOfRecording"]
print(Fs,NS,T0)

### read and display data ###
A=biosig.data(FILENAME)
NS=np.size(A,1)		# number of channels

fig = plt.figure()
ax  = fig.add_subplot(111)
h   = plt.plot(np.arange(np.size(A,0))/H["Samplingrate"],A[:,0]);
ax.set_xlabel('time [s]')
plt.show()


for chan in list(range(NS)):
	name=H["CHANNEL"][chan]["Label"]	# name of channel
	unit=H["CHANNEL"][chan]["PhysicalUnit"]	# units of channel
	printf("#%d:\t%s\t[%s]\n", chan, name, unit)

### get all sweeps, breaks-in-recording
selpos=[0]
if hasattr(H,"EVENT"):
    for k in list(range(len(H["EVENT"]))):
        if (H["EVENT"][k]["TYP"] == '0x7ffe'):
            selpos.append(round(H["EVENT"][k]["POS"]*H["Samplingrate"]))

selpos.append(np.size(A,0))

### data from channel m and sweep c can be obtained by:
m=0	# channel
c=0	# segment, trace, sweep number
d = A[selpos[c]:selpos[c+1]-1,m]

### display data
fig = plt.figure()
k=0
for m in list(range(NS)):
	for c in list(range(len(selpos)-1)):
		k = k+1
		ax  = fig.add_subplot(NS,len(selpos)-1,k)
		d   = A[selpos[c]:selpos[c+1]-1,m]
		h   = plt.plot(np.arange(np.size(d,0))/H["Samplingrate"],d)

plt.show()

