#!/bin/bash 
#
# Generate vhdr header files from a template *.vhdr file
#   This is a simple way make flat binary files readable by Biosig 
#   tools (including SigViewer, Stimfit, etc). 
#   Add known sampling interval, number of channels, and data type in 
#   template file; optionally, you can also define channel labels, 
#   scaling factors and physical units, and then apply this to a large
#   set of flat binary files with same setting.
#   The template.vhdr (and template.vmrk) are self-explaining.
#
# Usage:
#   biosig_vhdr_generator.sh template.vhdr *.dat
#
#   *.dat file names are expanded and corresponding 
#   *.vhdr files are generated based on template.vhdr.
#   
#
#    Copyright (C) 2020 Alois Schloegl <alois.schloegl@gmail.com>
#    This file is part of BioSig http://biosig.sourceforge.io/
#
#    BioSig is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 3
#    of the License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TEMPLATE=$1
shift 
FILELIST=$*

for F in $FILELIST; do 
	OUTFILE="${F%.*}.vhdr"
	if [ -f "${OUTFILE}" ]; then 
		echo "File ${OUTFILE} exists already - delete it first before you can regenerate ${OUTFILE} "
	else
		# sed s/DataFile=.*$/DataFile=${F}/g $TEMPLATE > ${F%.*}.vhdr
		sed -e 's/DataFile=.*$/DataFile='${F}'/g' -e 's/MarkerFile=.*/MarkerFile='${F%.*}'.vmrk/g' $TEMPLATE > ${OUTFILE}
	fi
	# this would generate also Marker file from an template - but this is hardly useful
	# sed 's/DataFile=.*$/DataFile='${F}'/' ${TEMPLATE%.*}.vmrk >${F%.*}.vmrk
done 
