# - Find CBLAS (includes and library)
#
# This module defines
#  CBLAS_INCLUDE_DIR
#  CBLAS_LIBRARIES
#  CBLAS_FOUND
#
# Also defined, but not for general use are
#  CBLAS_LIBRARY, where to find the library.

set(BLAS_FIND_REQUIRED true)

#if (CBLAS_INC_DIR)
#    find_path(CBLAS_INCLUDE_DIR cblas.h gsl_cblas.h
#        PATHS ${CBLAS_INC_DIR} NO_DEFAULT_PATH)
#else()
#    find_path(CBLAS_INCLUDE_DIR cblas.h gsl_cblas.h
#        PATHS
#        /usr/include/atlas
#        /usr/local/include/atlas
#        /usr/include
#        /usr/local/include
#        /usr/include/gsl)
#endif()

set(CBLAS_NAMES ${CBLAS_NAMES} cblas gslcblas)

if (CBLAS_LIB_DIR)
    find_library(CBLAS_LIBRARY
        NAMES ${CBLAS_NAMES}
        PATHS ${CBLAS_LIB_DIR} NO_DEFAULT_PATH)
else()
    find_library(CBLAS_LIBRARY
        NAMES ${CBLAS_NAMES}
        PATHS
        /usr/lib64/atlas
        /usr/lib/atlas
        /usr/local/lib64/atlas
        /usr/local/lib/atlas
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib)
endif()

#if (CBLAS_LIBRARY AND CBLAS_INCLUDE_DIR)
#   set(CBLAS_LIBRARIES ${CBLAS_LIBRARY})
#endif (CBLAS_LIBRARY AND CBLAS_INCLUDE_DIR)
if (CBLAS_LIBRARY)
   set(CBLAS_LIBRARIES ${CBLAS_LIBRARY})
endif (CBLAS_LIBRARY)




# handle the QUIETLY and REQUIRED arguments and set CBLAS_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleCompat)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(CBLAS DEFAULT_MSG CBLAS_LIBRARY CBLAS_LIBRARIES CBLAS_INCLUDE_DIR)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CBLAS DEFAULT_MSG CBLAS_LIBRARY CBLAS_LIBRARIES)

# Deprecated declarations.
#set(NATIVE_CBLAS_INCLUDE_PATH ${CBLAS_INCLUDE_DIR} )
#GET_FILENAME_COMPONENT(NATIVE_CBLAS_LIB_PATH ${CBLAS_LIBRARY} PATH)

# Hide in the cmake cache
#mark_as_advanced(CBLAS_LIBRARY CBLAS_INCLUDE_DIR)
mark_as_advanced(CBLAS_LIBRARY)
