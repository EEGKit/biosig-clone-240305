#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([biosig], [2.3.3])
AC_COPYRIGHT([Copyright (C) 2019-2021 Alois Schlögl])
AC_CONFIG_SRCDIR([biosig4c++/biosig.c])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([asound],  [snd_pcm_open])
AC_CHECK_LIB([b64],     [base64_encode_block], AC_SUBST(HAVE_LIBB64,     "1") )
AC_CHECK_LIB([cholmod], [cholmod_start],       AC_SUBST(HAVE_LIBCHOLMOD, "1") )
AC_CHECK_LIB([dcmdata], [dcmEnableOtherLongVRGeneration], AC_SUBST(HAVE_LIBDCMTK, "1") )
AC_CHECK_LIB([hdf5],    [H5Fopen],             AC_SUBST(HAVE_LIBHDF5,    "1") )
AC_CHECK_LIB([matio],   [Mat_Open],            AC_SUBST(HAVE_LIBMATIO,   "1") )
AC_CHECK_LIB([sqlite3], [sqlite3_open],        AC_SUBST(HAVE_SQLITE3,    "1") )
AC_CHECK_LIB([z],       [gzopen],              AC_SUBST(HAVE_LIBZ,       "1") )

# FIXME: Replace `main' with a function in `-lgdcmCommon':
#AC_CHECK_LIB([gdcmCommon], [main])
# FIXME: Replace `main' with a function in `-lgdcmDICT':
#AC_CHECK_LIB([gdcmDICT], [main])
# FIXME: Replace `main' with a function in `-lgdcmDSED':
#AC_CHECK_LIB([gdcmDSED], [main])
# FIXME: Replace `main' with a function in `-lgdcmIOD':
#AC_CHECK_LIB([gdcmIOD], [main])
# FIXME: Replace `main' with a function in `-lgdcmMEXD':
#AC_CHECK_LIB([gdcmMEXD], [main])
# FIXME: Replace `main' with a function in `-lgdcmMSFF':
#AC_CHECK_LIB([gdcmMSFF], [main])

AC_CHECK_LIB([iconv], [iconv_open])
AC_CHECK_HEADER([iconv.h], ,[AC_MSG_ERROR([can not find iconv.h])])
AC_CHECK_LIB([tinyxml] , [main],        AC_SUBST(HAVE_LIBTINYXML, "1")  [TEST_LIBS="$TEST_LIBS -ltinyxml"],   AC_MSG_WARN([libtinyxml is not installed.]))
# AC_CHECK_LIB([tinyxml2], [main],        AC_SUBST(HAVE_LIBTINYXML2, "1") [TEST_LIBS="$TEST_LIBS -ltinyxml2"],  AC_MSG_WARN([libtinyxml2 is not installed.]))

AC_PATH_PROG([JAVA],        [java],        [:])
AC_PATH_PROG([JAVAC],       [javac],       [:])
#
AC_PATH_PROG([MATHEMATICA], [mathematica], [:])
AC_PATH_PROG([MATHEMATICA_MPREP], [mprep], [:])
# MATLAB needs to contain full path, because it also used to find mex and mexext
#   readlink -f does not work on MacOSX, use "matlab -n" to extract the path
# AC_CHECK_PROG([MATLAB],     [matlab],      [$(readlink -f $(which matlab))],      [:])
AC_CHECK_PROG([MATLAB],     [matlab],      [$(matlab -n|awk '/^->  r   MATLAB              = / {printf("%s/bin/matlab",$5)}')],      [:])
AC_PATH_PROG([MKOCTFILE],   [mkoctfile],   [:])
AC_PATH_PROG([OCTAVE],      [octave],      [:])
# PHP
AC_PATH_PROG([PHP],         [php],         [:])
AC_PATH_PROG([PEAR],        [pear],        [:])
AC_PATH_PROG([PECL],        [pecl],        [:])
# PYTHON
AC_PATH_PROG([PYTHON],      [python],      [:])
AC_PATH_PROG([PYTHON2],     [python2],     [:])
AC_PATH_PROG([PYTHON3],     [python3],     [:])
# R
AC_PATH_PROG([R],           [R],           [:])

# FIXME: Replace `main' with a function in `-llibmex':
AC_CHECK_LIB([libmex], [main])
# FIXME: Replace `main' with a function in `-llibmx':
AC_CHECK_LIB([libmx], [main])

AC_CHECK_LIB([m],	 [round],		AC_SUBST(HAVE_LIBM,       "1") )

# Wolfram/Mathematica Mathlink interface
AC_CHECK_LIB([mathlink], [MLPutRealArray],	AC_SUBST(HAVE_MATHEMATICA,"1"))
AC_CHECK_LIB([uuid],     [uuid_generate],	AC_SUBST(HAVE_UUID,       "1"))

#AC_CHECK_LIB([pthread], [pthread_mutex_lock])
# FIXME: Replace `main' with a function in `-lssp':
#AC_CHECK_LIB([ssp], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm atexit bzero dup2 floor getcwd getdelim gethostbyname gethostname gettimeofday localtime_r memchr memmove memset mkdir modf pow putenv rmdir select setenv setlocale socket strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr strtol strtoul strtoumax tzset])

AC_CONFIG_FILES([Makefile
                 biosig4c++/Makefile
                 biosig4c++/examples/Makefile
                 biosig4c++/igor/Makefile
                 biosig4c++/java/Makefile
                 biosig4c++/magic/Makefile
                 biosig4c++/mex/Makefile
                 biosig4c++/mma/Makefile
                 biosig4c++/perl/Makefile
                 biosig4c++/php/Makefile
                 biosig4c++/python/Makefile
                 biosig4c++/python/setup.py
                 biosig4c++/python/README.md
                 biosig4c++/R/Makefile
                 biosig4c++/R/DESCRIPTION
                 biosig4c++/ruby/Makefile
                 biosig4c++/t240/Makefile
                 biosig4c++/t241/Makefile
                 biosig4c++/tcl/Makefile])

AC_OUTPUT
