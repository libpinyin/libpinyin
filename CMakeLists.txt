## Copyright (C) 2011 BYVoid
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

######## Project settings
cmake_minimum_required(VERSION 2.8)
set (PACKAGE_NAME libpinyin)
project (${PACKAGE_NAME} CXX C)
enable_testing()

######## Package information
set (PACKAGE_URL http://http://code.google.com/p/libpinyin)
set (PACKAGE_BUGREPORT http://code.google.com/p/libpinyin/issues/entry)
set (LIBPINYIN_VERSION_MAJOR 0)
set (LIBPINYIN_VERSION_MINOR 2)
set (LIBPINYIN_VERSION_REVISION 99)
set (LIBPINYIN_BINARY_VERSION 0.3)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (version_suffix .Debug)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

set (
    LIBPINYIN_VERSION
    ${LIBPINYIN_VERSION_MAJOR}.${LIBPINYIN_VERSION_MINOR}.${LIBPINYIN_VERSION_REVISION}${version_suffix}
)

######## Validation

include(CheckIncludeFileCXX)
check_include_file_cxx(locale.h HAVE_LOCALE_H)
check_include_file_cxx(libintl.h HAVE_LIBINTL_H)
check_include_file_cxx(stdlib.h HAVE_STDLIB_H)
check_include_file_cxx(string.h HAVE_STRING_H)
check_include_file_cxx(sys/time.h HAVE_SYS_TIME_H)
check_include_file_cxx(unistd.h HAVE_UNISTD_H)

include(CheckFunctionExists)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(malloc HAVE_MALLOC)
check_function_exists(memcmp HAVE_MEMCMP)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(realloc HAVE_REALLOC)
check_function_exists(setlocale HAVE_SETLOCALE)
check_function_exists(stat HAVE_STAT)

include(CheckTypeSize)
check_type_size(size_t SIZE_OF_SIZE_T)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(GLIB2 REQUIRED)
find_package(BerkeleyDB REQUIRED)

######## Windows

if (WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
endif (WIN32)

######## Directory

set (DIR_PREFIX ${CMAKE_INSTALL_PREFIX})
set (DIR_LIBRARY ${DIR_PREFIX}/${CMAKE_SHARED_LIBRARY_PREFIX})
set (DIR_LIBRARY_STATIC ${DIR_PREFIX}/${CMAKE_STATIC_LIBRARY_PREFIX})
set (DIR_INCLUDE ${DIR_PREFIX}/include)
set (DIR_SHARE ${DIR_PREFIX}/share)
set (DIR_BIN ${DIR_PREFIX}/bin)
set (DIR_ETC ${DIR_PREFIX}/etc)

if (DEFINED CMAKE_INSTALL_LIBDIR)
    set (DIR_LIBRARY ${CMAKE_INSTALL_LIBDIR})
    set (DIR_LIBRARY_STATIC ${CMAKE_INSTALL_LIBDIR})
endif (DEFINED CMAKE_INSTALL_LIBDIR)

if (DEFINED SHARE_INSTALL_PREFIX)
    set (DIR_SHARE ${SHARE_INSTALL_PREFIX})
endif (DEFINED SHARE_INSTALL_PREFIX)

if (DEFINED INCLUDE_INSTALL_DIR)
    set (DIR_INCLUDE ${INCLUDE_INSTALL_DIR})
endif (DEFINED INCLUDE_INSTALL_DIR)

if (DEFINED SYSCONF_INSTALL_DIR)
    set (DIR_ETC ${SYSCONF_INSTALL_DIR})
endif (DEFINED SYSCONF_INSTALL_DIR)

set (DIR_SHARE_LIBPINYIN ${DIR_SHARE}/libpinyin)
set (DIR_INCLUDE_LIBPINYIN ${DIR_INCLUDE}/libpinyin-${LIBPINYIN_BINARY_VERSION})

######## Configuration

configure_file(
    libpinyin.pc.in
    libpinyin.pc
    @ONLY
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/libpinyin.pc
    DESTINATION
        ${DIR_LIBRARY}/pkgconfig
)

######## Definition

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(
        -O0
        -g3
    )
endif (CMAKE_BUILD_TYPE MATCHES Debug)

include_directories(
    ${GLIB2_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/storage
    ${PROJECT_SOURCE_DIR}/src/lookup
)

######## Subdirectories

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(utils)