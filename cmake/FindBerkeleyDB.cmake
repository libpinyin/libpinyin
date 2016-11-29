# -*- cmake -*-

# - Find BerkeleyDB
# Find the BerkeleyDB includes and library
# This module defines
#  DB_INCLUDE_DIR, where to find db.h, etc.
#  DB_LIBRARIES, the libraries needed to use BerkeleyDB.
#  DB_FOUND, If false, do not try to use BerkeleyDB.
# also defined, but not for general use are
#  DB_LIBRARY, where to find the BerkeleyDB library.

FIND_PATH(DB_INCLUDE_DIR db.h
  /usr/local/include/db4
  /usr/local/include
  /usr/include/db4
  /usr/include
  )

SET(DB_NAMES ${DB_NAMES} db)
FIND_LIBRARY(DB_LIBRARY
  NAMES ${DB_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (DB_LIBRARY AND DB_INCLUDE_DIR)
  SET(DB_LIBRARIES ${DB_LIBRARY})
  SET(DB_FOUND "YES")
ELSE (DB_LIBRARY AND DB_INCLUDE_DIR)
  SET(DB_FOUND "NO")
ENDIF (DB_LIBRARY AND DB_INCLUDE_DIR)


IF (DB_FOUND)
  IF (NOT DB_FIND_QUIETLY)
    MESSAGE(STATUS "Found BerkeleyDB: ${DB_LIBRARIES}")
  ENDIF (NOT DB_FIND_QUIETLY)
ELSE (DB_FOUND)
  IF (DB_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find BerkeleyDB library")
  ENDIF (DB_FIND_REQUIRED)
ENDIF (DB_FOUND)

# Deprecated declarations.
SET (NATIVE_DB_INCLUDE_PATH ${DB_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_DB_LIB_PATH ${DB_LIBRARY} PATH)

MARK_AS_ADVANCED(
  DB_LIBRARY
  DB_INCLUDE_DIR
  )
