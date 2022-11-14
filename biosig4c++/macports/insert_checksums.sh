#! /bin/bash

BSVERSION="2.3.1"
MPDIR=`pwd`

wget http://downloads.sourceforge.net/project/biosig/BioSig%20for%20C_C%2B%2B/src/biosig-${BSVERSION}.src.tar.gz
RMD160=`openssl rmd160 -r biosig-${BSVERSION}.src.tar.gz | awk '{print $1;}'`
SHA256=`openssl sha256 -r biosig-${BSVERSION}.src.tar.gz | awk '{print $1;}'`

echo "rmd160:" ${RMD160}
echo "sha256:" ${SHA256}

GSED=`which gsed`
if [ "${GSED}" = "" ]
then
    GSED=`which sed`
fi

${GSED} 's/RMD160/'${RMD160}'/g' ${MPDIR}/science/libbiosig/Portfile.in > ${MPDIR}/science/libbiosig/Portfile
${GSED} -i 's/SHA256/'${SHA256}'/g' ${MPDIR}/science/libbiosig/Portfile
${GSED} -i 's/BSVERSION/'${BSVERSION}'/g' ${MPDIR}/science/libbiosig/Portfile

sudo portindex
sudo port uninstall libbiosig
sudo port clean --all libbiosig
