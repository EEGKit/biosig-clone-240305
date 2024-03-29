cmake_minimum_required (VERSION 2.6)
project (libbiosig)

option (USE_ZLIB "Add support for compressed files." OFF)
option (USE_SUITESPARSE "Add support for automatic re-referencing." OFF)

# Version number
set (libbiosig_VERSION_MAJOR 1)
set (libbiosig_VERSION_MINOR 8)
set (libbiosig_VERSION_PATCH 4)

find_program (GAWK NAMES gawk)

add_custom_target (eventcodes
  COMMAND ${GAWK} -f eventcodes.awk extern/eventcodes.txt
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS eventcodes.awk extern/eventcodes.txt
)

add_custom_target (units
  COMMAND ${GAWK} -f units.awk extern/units.csv > "units.i"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS units.awk extern/units.csv
)

add_custom_target (annexb
  COMMAND ${GAWK} -f annotatedECG.awk extern/11073-10102-AnnexB.txt > "11073-10102-AnnexB.i"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS annotatedECG.awk extern/11073-10102-AnnexB.txt
)

set (headers
  t210/abfheadr.h
  XMLParser/tinyxml.h
  XMLParser/tinystr.h
  biosig.h
  gdftime.h
  physicalunits.h
)

set (sources
  t210/sopen_abf_read.c
  t210/sopen_alpha_read.c
  t210/sopen_axg_read.c
  t210/sopen_cadwell_read.c
  t210/sopen_cfs_read.c
  t210/sopen_heka_read.c
  t210/sopen_igor.c
  t210/sopen_scp_read.c
  t210/scp-decode.cpp
  t220/crc4scp.c
  t220/sopen_scp_write.c
  t230/sopen_hl7aecg.cpp
  test0/sandbox.c
  XMLParser/tinystr.cpp
  XMLParser/tinyxml.cpp
  XMLParser/tinyxmlerror.cpp
  XMLParser/tinyxmlparser.cpp
  biosig.c
  gdftime.c
  mdc_ecg_codes.c
  physicalunits.c
)

if (WIN32)
  list (APPEND sources
    win32/getdelim.c
    win32/getline.c
    win32/getlogin.c
  )
endif ()

if (APPLE)
  list (APPEND sources
    win32/getdelim.c
    win32/getline.c
  )
endif ()

# Cannot have same name for static and shared library, 
# so we'll correct that later on
add_library (biosigstatic STATIC
  ${headers}
  ${sources}
)
set_target_properties(biosigstatic PROPERTIES OUTPUT_NAME biosig)

add_library (biosig2static STATIC
  ${headers}
  biosig2.h
  biosig-dev.h
  ${sources}
  biosig2.c
)
set_target_properties(biosig2static PROPERTIES OUTPUT_NAME biosig2)

add_library (biosigshared SHARED
  ${headers}
  ${sources}
)
set_target_properties(biosigshared PROPERTIES OUTPUT_NAME biosig)
if (APPLE)
  set_target_properties(biosigshared PROPERTIES
     INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
     )
endif ()

add_library (biosig2shared SHARED
  ${headers}
  biosig2.h
  biosig-dev.h
  ${sources}
  biosig2.c
)
set_target_properties(biosig2shared PROPERTIES OUTPUT_NAME biosig2)
if (APPLE)
  set_target_properties(biosigshared PROPERTIES
     INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
     )
endif ()

add_dependencies (biosigstatic eventcodes units annexb)
add_dependencies (biosigshared eventcodes units annexb)
add_dependencies (biosig2static eventcodes units annexb)
add_dependencies (biosig2shared eventcodes units annexb)

FIND_LIBRARY( ICONV_LIBRARY NAMES iconv )
MARK_AS_ADVANCED( ICONV_LIBRARY )
target_link_libraries( biosigstatic ${ICONV_LIBRARY} )
target_link_libraries( biosigshared ${ICONV_LIBRARY} )
target_link_libraries( biosig2static ${ICONV_LIBRARY} )
target_link_libraries( biosig2shared ${ICONV_LIBRARY} )

if (USE_ZLIB)
  add_definitions (-D=WITH_ZLIB)
  find_package( ZLIB REQUIRED )
    if ( ZLIB_FOUND )
      include_directories( ${ZLIB_INCLUDE_DIRS} )
      target_link_libraries( biosigstatic ${ZLIB_LIBRARIES} )
      target_link_libraries( biosigshared ${ZLIB_LIBRARIES} )
      target_link_libraries( biosig2static ${ZLIB_LIBRARIES} )
      target_link_libraries( biosig2shared ${ZLIB_LIBRARIES} )
    endif( ZLIB_FOUND )
endif ()

if (USE_SUITESPARSE)
  add_definitions (-D=WITH_CHOLMOD)
  FIND_LIBRARY( CHOLMOD_LIBRARY NAMES cholmod )
  MARK_AS_ADVANCED( CHOLMOD_LIBRARY )
  target_link_libraries( biosigstatic ${CHOLMOD_LIBRARY} )
  target_link_libraries( biosigshared ${CHOLMOD_LIBRARY} )
  target_link_libraries( biosig2static ${CHOLMOD_LIBRARY} )
  target_link_libraries( biosig2shared ${CHOLMOD_LIBRARY} )
endif ()

add_definitions (-D=WITHOUT_NETWORK)

install (TARGETS biosigstatic DESTINATION lib)
install (TARGETS biosigshared DESTINATION lib)
install (TARGETS biosig2static DESTINATION lib)
install (TARGETS biosig2shared DESTINATION lib)
install (FILES  biosig.h DESTINATION include)
install (FILES  biosig2.h DESTINATION include)
install (FILES  biosig-dev.h DESTINATION include)
install (FILES  gdftime.h DESTINATION include)
