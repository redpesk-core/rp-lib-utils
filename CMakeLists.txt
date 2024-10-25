###########################################################################
# Copyright (C) 2015-2024 IoT.bzh Company
#
# Author: José Bollo <jose.bollo@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
# 
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################

cmake_minimum_required(VERSION 3.13)

project(rp-lib-utils
	VERSION 0.2.0
	DESCRIPTION  "Programming utilities"
	HOMEPAGE_URL "https://github.com/redpesk-core/rp-lib-utils"
	LANGUAGES C)

# Detect the platform reliably
if(ZEPHYR_BASE)
    if (NOT CONFIG_AFB_BINDER)
      return()
    endif()
    SET(WITH_ZEPHYR YES)
endif()

include(FindPkgConfig)
include(GNUInstallDirs)

set(LIBRP_UTILS_VERSION "${PROJECT_VERSION}")

option(NO_LIBUUID "Dont use libuuid" OFF)

if (WITH_ZEPHYR)
  set(NO_JSON_C  ON)
  set(NO_YAML    ON)
  set(NO_CURL    ON)
  set(NO_SYSTEMD ON)
  set(NO_LIBUUID ON)
  set(NO_SHARED_LIB ON)

  #set(WITHOUT_PROCFS ON)
  #set(WITH_DIRENT OFF)

  add_definitions(-DWITH_DIRENT=0)
  add_definitions(-DWITH_ZEPHYR=1)
  add_definitions(-DVERBOSE_WITH_SYSLOG=1)
  find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
endif()

add_subdirectory(src)
