#
#    Copyright (C) 2016,2019 Alois Schloegl <alois.schloegl@gmail.com>
#    This file is part of the "BioSig for C/C++" repository
#    (biosig4c++/libbiosig) at http://biosig.sf.net/
#
#    This program is free software; you can redistribute it and/or
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


dyn.load("sload.so")

#' Loads biomedical signal data
#'
#' @param filename name of biosig data file
#' @param channel indicates which channel should be loaded, (default 0: indicaes all channels)
#' @return matrix containing sample values
sload <- function(filename, channel=0) {
	result <- .Call("sload", filename, channel=0)
	return(result)
}

#' @param filename name of biosig data file
#' @return Header (or Meta-) information of data file in JSON format
jsonHeader <- function(filename) {
	result <- .Call("jsonHeader", filename)
	return(result)
}

# Usage:
#   # read header information
#   library(rjson)
#   HDR = fromJSON(jsonHeader("../data/Newtest17-256.bdf"))
#
#   # read data
#   data = sload("../data/Newtest17-256.bdf")
#   plot((1:nrow(data))/HDR$Samplingrate,data[,1],"l",xlab="time [s]",ylab=HDR$CHANNEL[[1]]$Label)

