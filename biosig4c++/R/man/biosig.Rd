#' \name{Biosig}
#' \title{Loads biomedical signal data}
#'
#' \Description{ 
   @param filename name of biosig data file
   @return Header (or Meta-) information of data file in JSON format 
   jsonHeader <- function(filename) jsonHeader(filename)

   @param channel indicates which channel should be loaded, (default 0: indicaes all channels)
   @inheritParams jsonHeader
   @return matrix containing sample values
   sload <- function(filename, channel=0) sload(filename, channel)

   Usage:
   # read header information
library(rjson)
HDR = fromJSON(jsonHeader("../data/Newtest17-256.bdf"))
   # read data
data = sload("../data/Newtest17-256.bdf")
plot((1:nrow(data))/HDR$Samplingrate,data[,1],"l",xlab="time [s]",ylab=HDR$CHANNEL[[1]]$Label)
#' }
