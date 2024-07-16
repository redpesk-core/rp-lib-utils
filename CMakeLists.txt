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
	VERSION 0.1.1
	DESCRIPTION  "Programming utilities"
	HOMEPAGE_URL "https://github.com/redpesk-core/rp-lib-utils"
	LANGUAGES C)

include(FindPkgConfig)
include(GNUInstallDirs)

set(LIBRP_UTILS_VERSION "${PROJECT_VERSION}")

###########################################################################
# TUNING

option(NO_STATIC_LIB         "don't produce static library")
option(NO_SHARED_LIB         "don't produce shared library")
option(WITHOUT_JSON_C        "disable json-c extensions")
option(WITHOUT_JSON_NLOHMANN "disable nlohmann extensions")
option(WITHOUT_SYSTEMD       "disable systemd support")
option(WITHOUT_CURL          "disable CURL support")
option(WITHOUT_YAML          "disable YAML support")
option(WITHOUT_PROCFS        "disable procfs dependent features")

###########################################################################

add_subdirectory(src)
