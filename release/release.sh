#!/bin/bash 
# make a new release of the Biosig toolbox
#   
#  usage:
#    ./release.sh VERSION_NAME
#
#  for example:
#    ./release.sh 2.0.5-rc1
#
# Copyright (C) 2004,2008,2011,2012,2013,2016-2020 Alois Schloegl


## TODO: add freetb4matlab
## Tagging a release

B4OMversion=3.7.9

SRCDIR=/home/schloegl/src
MXEDIR=$SRCDIR"/mxe.github.schloegl"

## output directories ##
CWD=`pwd`
BIOSIG4M_DIR="$CWD/biosig4matlab-$B4OMversion"
BIOSIG4C_DIR="biosig-$1"

rm -rf ${BIOSIG4C_DIR}* BIOSIG4M_DIR

echo "== prepare space (delete previous attempts) =="
rm -rf biosig biosig-code $BIOSIG4M_DIR ${BIOSIG4C_DIR}* $BIOSIG4M_DIR/../NaN $BIOSIG4M_DIR/../tsa
rm  biosig-"$1"*.gz biosig-"$1"*.zip biosig4c++-"$1"*.gz  biosig4c++-"$1"*.zip $B4OM
rm -rf biosig4octmat-$B4OMversion.{tar.gz,zip}

echo "== clone biosig =="
git clone  $SRCDIR/biosig-code biosig-code
rm -rf biosig-code/sigviewer
rm -rf biosig-code/biosig4java
rm -rf biosig-code/biosig4c++/patches
rm -rf biosig-code/biosig4python
rm -rf biosig-code/bioprofeed
rm -rf biosig-code/release
### generate ./configure ###
sed -i '/^AC_INIT/ s/^.*$/AC_INIT([biosig], ['$1'])/g' biosig-code/configure.ac
(cd biosig-code && autoreconf -fi && ./configure)
### precompute all *.i and avoid need for gawk at build time
(cd biosig-code/biosig4c++ && \
	gawk -f eventcodes.awk   "../biosig4matlab/doc/eventcodes.txt" && \
	gawk -f annotatedECG.awk "../biosig4matlab/doc/11073-10102-AnnexB.txt" > "11073-10102-AnnexB.i" && \
	gawk -f units.awk        "../biosig4matlab/doc/units.csv" > "units.i" \
)


echo "==  make biosig-$1.src.tar.gz =="
VERSIONFILE=biosig-code/biosig4c++/VERSION
echo '# BioSig http://biosig.sf.net/' > $VERSIONFILE
echo -e '# Version:\tbiosig4c++/libbiosig\t' $1 >> $VERSIONFILE
echo -n -e '# Date:\t' >> $VERSIONFILE
date +%Y-%m-%d >> $VERSIONFILE


DESCRIPTION="biosig-code/biosig4c++/DESCRIPTION"
echo 'Name: BioSig' > $DESCRIPTION
echo 'Version:' $1 >> $DESCRIPTION
echo 'Date:' $(date +%Y-%m-%d) >> $DESCRIPTION
echo 'Author: Alois Schloegl <alois.schloegl@gmail.com>' >> $DESCRIPTION
echo 'Maintainer: Alois Schloegl' >> $DESCRIPTION
echo 'Title: BioSig' >> $DESCRIPTION
echo 'Description: A software library for biomedical signal processing.' >> $DESCRIPTION
echo 'Depends: octave (> 2.9.0),' >> $DESCRIPTION
echo 'License: GPL version 3 or later' >> $DESCRIPTION
echo 'Url: http://biosig.sf.net' >> $DESCRIPTION

rm -rf biosig-code/biosig4c++/todo.txt biosig-code/.git

# URL=https://sourceforge.net/projects/libb64/files/libb64/libb64/libb64-1.2.1.zip
# wget -nc $URL -O /tmp/$(basename $URL)
# unzip   /tmp/$(basename $URL) -d biosig-code/biosig4c++

### build "biosig4octave" package ###
mv biosig4c++/mex/{mexbiosig,biosig4octave}-$1.src.tar.gz ./

mv biosig-code biosig-$1

#####################################
# SOURCE RELEASE
#####################################
if [[ 1 ]] ; then
	tar acvfz biosig-$1.src.tar.gz biosig-$1
	zip -r biosig-$1.src.zip biosig-$1
	#cp biosig-$1.src.tar.gz ~/L/deb/biosig_$1.orig.tar.gz
	#(cd ~/L/deb/ && tar xvf biosig$1.orig.tar.gz)

	echo "sha512sum:"
	sha512sum  biosig-$1.src.tar.gz | tee sha512.sum
	echo "sha256sum:"
	sha256sum  biosig-$1.src.tar.gz | tee sha256.sum
	echo "sha1sum:"
	sha1sum    biosig-$1.src.tar.gz | tee sha1.sum
	echo "md5sum:"
	md5sum  biosig-$1.src.tar.gz | tee md5.sum

	echo cp biosig-$1.* *.sum ~/L/tmp/
	echo cp biosig-$1.src.tar.gz ~/L/public_html/biosig/prereleases/
	echo cp biosig4octave-$1.src.tar.gz ~/L/tmp/
	echo cp biosig4octave-$1.src.tar.gz ~/L/public_html/biosig/prereleases/

	#
	#echo scp biosig4octmat-$B4OMversion.tar.gz schloegl@frs.sourceforge.net:"/home/frs/project/biosig/BioSig\ for\ Octave\ and\ Matlab/"
	#echo scp biosig4c++-$1.win.zip       schloegl@frs.sourceforge.net:"/home/frs/project/biosig/BioSig\ for\ C_C++/windows/"
	echo scp biosig-$1.src.tar.gz    schloegl@frs.sourceforge.net:"/home/frs/project/biosig"
fi

echo "== clone biosig =="
git clone $SRCDIR/biosig-code biosig-code
cp -rp biosig-code/biosig4matlab $BIOSIG4M_DIR

echo "== clone tsa+nan =="
cd $SRCDIR
hg clone $SRCDIR/octave-NaN  $BIOSIG4M_DIR/../NaN
hg clone $SRCDIR/octave-tsa  $BIOSIG4M_DIR/../tsa
cd $CWD


#####################################
# ADD BINARIES
#####################################
echo "== add binaries =="
cp $SRCDIR/biosig-code/biosig4matlab/install.m $BIOSIG4M_DIR/../biosig_installer.m
cp $SRCDIR/biosig-code/biosig4c++/mex/*.mex* $BIOSIG4M_DIR/t200_FileAccess/
cp $SRCDIR/octave-NaN/src/*.mex* $BIOSIG4M_DIR/../NaN/src/
cp $SRCDIR/octave-tsa/src/*.mex* $BIOSIG4M_DIR/../tsa/src/
rm $BIOSIG4M_DIR/../NaN/src/*.mex $BIOSIG4M_DIR/../tsa/src/*.mex $BIOSIG4M_DIR/../biosig/t200_FileAccess/*.mex
cd $CWD;
	mv biosig4matlab-$B4OMversion biosig
	tar acvfz biosig4octmat-$B4OMversion.tar.gz biosig tsa NaN biosig_installer.m ;
	zip -r biosig4octmat-$B4OMversion.zip biosig tsa NaN biosig_installer.m ;

mkdir -p $BIOSIG4C_DIR/{include,share/man,share/mathematica,share/python,share/R}
mkdir -p $BIOSIG4C_DIR-{Windows-64bit,Windows-32bit,$(uname -s)-$(uname -m)}/{include,share/man,bin,lib/pkgconfig,matlab,mathematica,python,R}

### shared, platform independent files ###
cp biosig-code/biosig4c++/*.h                    $BIOSIG4C_DIR/include/
cp biosig-code/biosig4c++/doc/*.1                $BIOSIG4C_DIR/share/man
cp -r biosig-code/biosig4matlab                  $BIOSIG4C_DIR/share/matlab
cp -r biosig-code/biosig4c++/mma/{*.nb,*.m}      $BIOSIG4C_DIR/share/mathematica/
cp -r biosig-code/biosig4c++/python/*.py         $BIOSIG4C_DIR/share/python/
cp -r biosig-code/biosig4python/loadgdf.py       $BIOSIG4C_DIR/share/python/
cp -r biosig-code/biosig4c++/R/{loadgdf.r,README}  $BIOSIG4C_DIR/share/R/


### GNU/Linux ###
PLATFORM=$(uname -s)-$(uname -m)
cp biosig-code/biosig4c++/*.h                            $BIOSIG4C_DIR-$PLATFORM/include/
cp $SRCDIR/biosig-code/biosig4c++/lib*                   $BIOSIG4C_DIR-$PLATFORM/lib/
cp $SRCDIR/biosig-code/biosig4c++/{bin2rec,rec2bin,heka2itx,*.exe,R/loadgdf.r} \
					                 $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4python/loadgdf.py          $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4c++/mex/{*.cpp,*.mexglx*}  $BIOSIG4C_DIR-$PLATFORM/matlab/
cp $SRCDIR/biosig-code/biosig4c++/win32/README           $BIOSIG4C_DIR-$PLATFORM/
rm $BIOSIG4C_DIR/$PLATFORM/matlab/*.mex		         ## remove octave binaries
cp -r $SRCDIR/biosig-code/biosig4c++/mma/Linux-x86-64/*  $BIOSIG4C_DIR-$PLATFORM/mathematica/
cp biosig-code/biosig4c++/python/{*.c,*.h,setup.py,README.md}  $BIOSIG4C_DIR-$PLATFORM/python/
cp -rp $BIOSIG4C_DIR/share				 $BIOSIG4C_DIR-$PLATFORM/
tar chvfz biosig-$1-$PLATFORM.tar.gz ./$BIOSIG4C_DIR-$PLATFORM
zip -r biosig-$1-$PLATFORM.zip ./$BIOSIG4C_DIR-$PLATFORM


### Windows 64bit ###
PLATFORM=Windows-64bit
cp biosig-code/biosig4c++/*.h                            $BIOSIG4C_DIR-$PLATFORM/include/
cp $MXEDIR/usr/x86_64-w64-mingw32.static/bin/{save2gdf,physicalunits,biosig_fhir,biosig2gdf,sigviewer,stimfit}.exe \
							 $BIOSIG4C_DIR-$PLATFORM/bin
cp $MXEDIR/usr/x86_64-w64-mingw32.static/bin/lib{b64,z,lapack,blas,iberty,biosig,tinyxml,iconv,physicalunits}.dll \
                                                         $BIOSIG4C_DIR-$PLATFORM/bin
cp $MXEDIR/usr/x86_64-w64-mingw32.static/lib/lib{b64,z,lapack,blas,iberty,biosig,tinyxml,iconv,physicalunits}.a \
                                                         $BIOSIG4C_DIR-$PLATFORM/lib
cp $MXEDIR/usr/x86_64-w64-mingw32.static/lib/pkgconfig/{libbiosig,zlib}.pc \
                                                         $BIOSIG4C_DIR-$PLATFORM/lib/pkgconfig/
cp -rp $BIOSIG4C_DIR/share				 $BIOSIG4C_DIR-$PLATFORM/
cp $SRCDIR/biosig-code/biosig4c++/mex/{*.cpp,*.mexw64}   $BIOSIG4C_DIR-$PLATFORM/matlab/
rm $BIOSIG4C_DIR-$PLATFORM/matlab/*.mex		 ## remove octave binaries
cp -r $SRCDIR/biosig-code/biosig4c++/mma/Windows-x86-64/*  $BIOSIG4C_DIR-$PLATFORM/mathematica/
# loadgdf for Python and R
cp $SRCDIR/biosig-code/biosig4python/loadgdf.py          $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4python/loadgdf.py          $BIOSIG4C_DIR-$PLATFORM/python/
cp $SRCDIR/biosig-code/biosig4c++/python/{*.c,*.h,setup.py,README.md}  $BIOSIG4C_DIR-$PLATFORM/python/
(cd $BIOSIG4C_DIR-$PLATFORM/python/ && ln ../bin/biosig2gdf.exe)
cp $SRCDIR/biosig-code/biosig4c++/R/loadgdf.r            $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4c++/R/{loadgdf.r,README}   $BIOSIG4C_DIR-$PLATFORM/R/
(cd $BIOSIG4C_DIR-$PLATFORM/R/ && ln ../bin/biosig2gdf.exe)
tar chvfz biosig-$1-$PLATFORM.tar.gz ./$BIOSIG4C_DIR-$PLATFORM
zip -r biosig-$1-$PLATFORM.zip ./$BIOSIG4C_DIR-$PLATFORM


### Windows 32bit ###
PLATFORM=Windows-32bit
cp biosig-code/biosig4c++/*.h                            $BIOSIG4C_DIR-$PLATFORM/include/
cp $MXEDIR/usr/i686-w64-mingw32.static/bin/{save2gdf,physicalunits,biosig_fhir,biosig2gdf,sigviewer,stimfit}.exe \
							 $BIOSIG4C_DIR-$PLATFORM/bin
cp $MXEDIR/usr/i686-w64-mingw32.static/lib/lib{b64,z,lapack,blas,iberty,biosig,tinyxml,iconv,physicalunits}.dll \
                                                         $BIOSIG4C_DIR-$PLATFORM/bin
cp $MXEDIR/usr/i686-w64-mingw32.static/bin/lib{b64,z,lapack,blas,iberty,biosig,tinyxml,iconv,physicalunits}.a \
                                                         $BIOSIG4C_DIR-$PLATFORM/lib
cp $MXEDIR/usr/i686-w64-mingw32.static/lib/pkgconfig/{libbiosig,zlib}.pc \
                                                         $BIOSIG4C_DIR-$PLATFORM/lib/pkgconfig/
cp -rp $BIOSIG4C_DIR/share				 $BIOSIG4C_DIR-$PLATFORM/
cp $SRCDIR/biosig-code/biosig4c++/mex/{*.cpp,*.mexw32}   $BIOSIG4C_DIR-$PLATFORM/matlab/
rm $BIOSIG4C_DIR-$PLATFORM/matlab/*.mex		 ## remove octave binaries
cp -r $SRCDIR/biosig-code/biosig4c++/mma/Windows/*       $BIOSIG4C_DIR-$PLATFORM/mathematica/
# loadgdf for Python and R
cp $SRCDIR/biosig-code/biosig4python/loadgdf.py          $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4python/loadgdf.py          $BIOSIG4C_DIR-$PLATFORM/python/
cp $SRCDIR/biosig-code/biosig4c++/python/{*.c,*.h,setup.py,README.md}  $BIOSIG4C_DIR-$PLATFORM/python/
(cd $BIOSIG4C_DIR-$PLATFORM/python/ && ln ../bin/biosig2gdf.exe)
cp $SRCDIR/biosig-code/biosig4c++/R/loadgdf.r            $BIOSIG4C_DIR-$PLATFORM/bin/
cp $SRCDIR/biosig-code/biosig4c++/R/{loadgdf.r,README}   $BIOSIG4C_DIR-$PLATFORM/R/
(cd $BIOSIG4C_DIR-$PLATFORM/R/ && ln ../bin/biosig2gdf.exe)
tar chvfz biosig-$1-$PLATFORM.tar.gz ./$BIOSIG4C_DIR-$PLATFORM
zip -r biosig-$1-$PLATFORM.zip ./$BIOSIG4C_DIR-$PLATFORM


echo "sha512sum:"
sha512sum biosig4octmat-$B4OMversion.{tar.gz,zip} {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1{.src,-Windows*,-$(uname -s)-$(uname -m)}.{tar.gz,zip} | tee sha512.sum
echo "sha256sum:"
sha256sum biosig4octmat-$B4OMversion.{tar.gz,zip} {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1{.src,-Windows*,-$(uname -s)-$(uname -m)}.{tar.gz,zip} | tee sha256.sum
echo "sha1sum:"
sha1sum biosig4octmat-$B4OMversion.{tar.gz,zip} {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1{.src,-Windows*,-$(uname -s)-$(uname -m)}.{tar.gz,zip} | tee sha1.sum
echo "md5sum:"
md5sum biosig4octmat-$B4OMversion.{tar.gz,zip} {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1{.src,-Windows*,-$(uname -s)-$(uname -m)}.{tar.gz,zip} | tee md5.sum

echo cp biosig4octmat-$B4OMversion.{tar.gz,zip} {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1*.{tar.gz,zip} *.sum ~/L/tmp/
echo cp {mexbiosig,biosig4octave}-$1.src.tar.gz biosig-$1*.{zip,tar.gz} ~/L/public_html/biosig/prereleases/
#
echo scp biosig4octmat-$B4OMversion.tar.gz schloegl@frs.sourceforge.net:"/home/frs/project/biosig/BioSig\ for\ Octave\ and\ Matlab/"
echo scp biosig-$1-{win32,win64,$(uname -m)}.zip       schloegl@frs.sourceforge.net:"/home/frs/project/biosig/BioSig\ for\ C_C++/windows/"
echo scp biosig-$1.src.tar.gz    schloegl@frs.sourceforge.net:"/home/frs/project/biosig/BioSig\ for\ C_C\+\+/src/"

