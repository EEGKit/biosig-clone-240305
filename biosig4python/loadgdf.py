"""BIOSIG Toolbox for Python
    Copyright (C) 2020,2021 by Alois Schloegl <alois.schloegl@gmail.com>

    This function is part of the "BioSig for Python" repository
    (biosig4python) at https://biosig.sourceforge.io/

    BioSig is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


FUNCTIONS
===================
	sload


HEADER
===================
	TYPE:		type of file format
	VERSION:	GDF version number
	FileName:	name of opened file
	HeadLen:	length of header in bytes
	NS:		number of channels
	SPR:		samples per block (when different sampling rates are used, this is the LCM(CHANNEL[..].SPR)
	NRec:		number of records/blocks -1 indicates length is unknown.
	Dur:		Duration of each block in seconds expressed in the fraction Dur[0]/Dur[1]
	SampleRate:	Sampling rate
	IPaddr:		IP address of recording device (if applicable)
	T0:		starttime of recording


	data		last data read
	------------------------------
		block:		data block
		size:		size {rows, columns} of data block


	Patient:	Patient information
	-----------------------------------
		Name:		not recommended because of privacy protection
		Id:		identification code as used in hospital
		Weight:		weight in kilograms [kg] 0:unkown, 255: overflow
		Height:		height in centimeter [cm] 0:unkown, 255: overflow
		Birthday:	Birthday of Patient
		Age:		Age of Patient
		Headsize:	circumference, nasion-inion, left-right mastoid in millimeter
		Sex
		Handedness
		Smoking
		AlcoholAbuse
		DrugAbuse
		Medication
		Impairment
			Visual


	ID:		recording identification
	----------------------------------------
		Technician
		Hospital
		Equipment:	identfies this software


	LOC:		location of recording according to RFC1876
	----------------------------------------------------------
		VertPre
		HorizPre
		Size
		Version
		Latitude:	in degrees
		Longitude:	in degrees
		Altitude:	in metres


	ELEC:		position of electrodes; see also HDR.CHANNEL[k].XYZ
	-------------------------------------------------------------------
		REF:		XYZ position of reference electrode
		GND:		XYZ position of ground electrode


	EVENT:		EVENTTABLE
	--------------------------
		SampleRate:	for converting POS and DUR into seconds
		N:		number of events
		TYP:		defined at http://cvs.sourceforge.net/viewcvs.py/biosig/biosig/t200/eventcodes.txt?view=markup
		POS:		starting position [in samples]
		DUR:		duration [in samples]
		CHN:		channel number; 0: all channels


	FLAG:		flags
	---------------------
		OVERFLOWDETECTION:	overflow & saturation detection 0: OFF, !=0 ON
		UCAL:			UnCalibration  0: scaling  !=0: NO scaling - raw data return


	FILE:		File specific data
	----------------------------------
		FID:		file handle
		POS:		current reading/writing position in samples
		OPEN:		0: closed, 1:read, 2: write
		LittleEndian:	not in use


	AS:		internal variables
	----------------------------------
		PID:		patient identification
		RID:		recording identification
		spb:		total samples per block
		bpb:		total bytes per block
		bi:		not in use
		Header1:	not in use
		rawdata:	raw data block


	CHANNEL[k]:	channel specific data
	-------------------------------------
		Label:		Label of channel
		Transducer:	transducer e.g. EEG: Ag-AgCl electrodes
		PhysDim:	physical dimension
		PhysDimCode:	code for physical dimension
		PreFilt:	pre-filtering

		LowPass:	lowpass filter
		HighPass:	high pass
		Notch:		notch filter
		XYZ:		electrode position
		Impedance:	in Ohm

		PhysMin:	physical minimum
		PhysMax:	physical maximum
		DigMin:		digital minimum
		DigMax:		digital maximum

		GDFTYP:		data type
		SPR:		samples per record (block)
		bpr:		bytes per record (block)

		OnOff:		1: include, 0: exclude in sread
		Cal:		gain factor
		Off:		bias"""


import collections
import numpy
import os, sys
import struct
import subprocess


def skipbytes(fid,n):
	fid.read(n)

##### LOAD GDF data - as produced by biosig2gdf #####
def loadgdf(FileName, chan=-1):
	GDFTYP_BYTE = numpy.array([1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0, 0, 0, 0, 0, 4, 8, 16])
	HDR         = collections.namedtuple('HDRTYPE',['HeadLen'])

	converter   = 'biosig2gdf'
	if (os.name=='nt'):
		converter = 'biosig2gdf.exe'
	proc = subprocess.Popen([converter, FileName], stdout=subprocess.PIPE)
	fid = proc.stdout

	# read H1 - fixed header #
	version = fid.read(8)
	HDR.VERSION=float(version[4:]);
	if (version[0:4] != b'GDF ') and (HDR.VERSION>2.0) and (HDR.VERSION <= 3.0):
		print("file format not supported")
		print(version[0:4])
		return -1
	HDR.TYPE=version[0:4];
	skipbytes(fid, 184-8)
	HDR.HeadLen = struct.unpack('<H',fid.read(2))[0]*256

	skipbytes(fid, 236-184-2)
	HDR.NRec = struct.unpack('<Q',fid.read(8))[0]
	HDR.Dur  = struct.unpack('<d',fid.read(8))[0]
	HDR.NS   = struct.unpack('<H',fid.read(2))[0]
	HDR.tzmin= struct.unpack('<h',fid.read(2))[0]	# timezone in minutes east of UTC
	HDR.SPR  = 1
	if (chan < 0):
		chan = slice(0,HDR.NS);

	# read H2 - variable header, channel information
	HDR.Label=[]
	for k in range(HDR.NS):
		HDR.Label.append(struct.unpack('<16s',fid.read(16))[0].strip(b'\0').decode())
	HDR.Transducer=[]
	for k in range(HDR.NS):
		HDR.Transducer.append(struct.unpack('<80s',fid.read(80))[0].strip(b'\0').decode())

	skipbytes(fid, (102-96)*HDR.NS)
	HDR.PhysDimCode = numpy.array(struct.unpack('<'+str(HDR.NS)+'H',fid.read(2*HDR.NS))[0:HDR.NS])
	HDR.PhysMin = numpy.array(struct.unpack('<'+str(HDR.NS)+'d',fid.read(8*HDR.NS))[0:HDR.NS])
	HDR.PhysMax = numpy.array(struct.unpack('<'+str(HDR.NS)+'d',fid.read(8*HDR.NS))[0:HDR.NS])
	HDR.DigMin  = numpy.array(struct.unpack('<'+str(HDR.NS)+'d',fid.read(8*HDR.NS))[0:HDR.NS])
	HDR.DigMax  = numpy.array(struct.unpack('<'+str(HDR.NS)+'d',fid.read(8*HDR.NS))[0:HDR.NS])

	skipbytes(fid, (200-102-34)*HDR.NS)
	HDR.Toffset = numpy.array(struct.unpack('<'+str(HDR.NS)+'f',fid.read(4*HDR.NS))[0:HDR.NS])
	HDR.LowPass = numpy.array(struct.unpack('<'+str(HDR.NS)+'f',fid.read(4*HDR.NS))[0:HDR.NS])
	HDR.HighPass= numpy.array(struct.unpack('<'+str(HDR.NS)+'f',fid.read(4*HDR.NS))[0:HDR.NS])
	HDR.Notch   = numpy.array(struct.unpack('<'+str(HDR.NS)+'f',fid.read(4*HDR.NS))[0:HDR.NS])
	HDR.spr     = numpy.array(struct.unpack('<'+str(HDR.NS)+'L',fid.read(4*HDR.NS))[0:HDR.NS])
	HDR.GDFTYP  = numpy.array(struct.unpack('<'+str(HDR.NS)+'L',fid.read(4*HDR.NS))[0:HDR.NS])

	for v in numpy.unique(HDR.spr[chan]):
		HDR.SPR = numpy.lcm(HDR.SPR, v)
	HDR.SampleRate = HDR.SPR / HDR.Dur

	HDR.BPB     = sum(GDFTYP_BYTE[HDR.GDFTYP] * HDR.spr)
	# bi          <- c(0, cumsum(ceiling(GDFTYP_BYTE[HDR.GDFTYP+1]) * HDR.spr))
	# spb         = [0,numpy.cumsum(HDR.spr)]

	HDR.Cal = (HDR.PhysMax - HDR.PhysMin) / (HDR.DigMax - HDR.DigMin)
	HDR.Off =  HDR.PhysMin - HDR.Cal * HDR.DigMin
	HDR.EventTablePos = HDR.HeadLen + HDR.BPB * HDR.NRec

	### read H3 - optional header ###
	skipbytes(fid, HDR.HeadLen - 256 - 224*HDR.NS)

	### read data section ###
	D = fid.read(HDR.BPB*HDR.NRec)
	nSamples=HDR.NS*HDR.SPR*HDR.NRec
	print(nSamples)
	# codes according to https://docs.python.org/3/library/struct.html
	if (all(HDR.GDFTYP==0)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'c',D)[0:nSamples])
	elif (all(HDR.GDFTYP==1)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'b',D)[0:nSamples])
	elif (all(HDR.GDFTYP==2)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'B',D)[0:nSamples])
	elif (all(HDR.GDFTYP==3)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'h',D)[0:nSamples])
	elif (all(HDR.GDFTYP==4)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'H',D)[0:nSamples])
	elif (all(HDR.GDFTYP==5)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'l',D)[0:nSamples])
	elif (all(HDR.GDFTYP==6)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'L',D)[0:nSamples])
	elif (all(HDR.GDFTYP==7)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'q',D)[0:nSamples])
	elif (all(HDR.GDFTYP==8)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'Q',D)[0:nSamples])

	elif (all(HDR.GDFTYP==16)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'f',D)[0:nSamples])
	elif (all(HDR.GDFTYP==17)):
		D = numpy.array(struct.unpack('<'+str(nSamples)+'d',D)[0:nSamples])
	else:
		print("data types are not the same for all channels")
		return -1

	D = numpy.reshape(D,(HDR.NS,HDR.SPR*HDR.NRec))
	if (all(HDR.spr[chan]==1)):
		for ch in range(HDR.NS):
			print(ch)
			D[ch,:] = D[ch,:] * HDR.Cal[ch] + HDR.Off[ch]
	else:
		print("this file is currently not supported (channels have different sampling rates)")
		return -1
	HDR.data = D;

	# read event table


	return(HDR)

