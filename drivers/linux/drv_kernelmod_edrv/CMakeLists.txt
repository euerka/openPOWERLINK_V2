################################################################################
#
# CMake file for openPOWERLINK kernel stack (Linux kernel module)
#
# Copyright (c) 2013, SYSTEC electronik GmbH
# Copyright (c) 2016, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
# Copyright (c) 2016, Kalycito Infotech Private Limited
# Copyright (c) 2016, Romain Naour (Open Wide)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holders nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

PROJECT (oplkdrv_kernelmodule_edrv C)

###############################################################################
# edrvBuild
###############################################################################
MACRO(edrvBuild OPLK_EDRV)
    SET(MODULE_NAME "oplk${OPLK_EDRV}${MODULE_NAME_SUFFIX}")
    SET(MODULE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME})
    SET(MODULE_FILE ${MODULE_NAME}.ko)

    GET_PROPERTY(DRIVER_INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    FOREACH(NEXT_INCLUDE_DIR ${DRIVER_INCLUDE_DIRS})
        SET(MODULE_INCLUDES "${MODULE_INCLUDES} -I${NEXT_INCLUDE_DIR}")
    ENDFOREACH()
    ###############################################################################
    # Configure depending selected ethernet driver
    ###############################################################################
    IF(${OPLK_EDRV} STREQUAL "8139")

        SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=8139")
        SET(MODULE_OPLK8139_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-8139.c)
        SET(MODULE_OPLK8139_SOURCE_FILES ${MODULE_OPLK8139_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-linuxkernel.c)
        SET(MODULE_EDRV_SOURCE_FILES ${MODULE_OPLK8139_SOURCE_FILES})

    ELSEIF(${OPLK_EDRV} STREQUAL "82573")

        SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=82573")
        SET(MODULE_OPLK82573_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-82573.c)
        SET(MODULE_OPLK82573_SOURCE_FILES ${MODULE_OPLK82573_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-linuxkernel.c)
        SET(MODULE_EDRV_SOURCE_FILES ${MODULE_OPLK82573_SOURCE_FILES})

    ELSEIF(${OPLK_EDRV} STREQUAL "8255x")

        SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=8255")
        SET(MODULE_OPLK8255X_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-8255x.c)
        SET(MODULE_OPLK8255X_SOURCE_FILES ${MODULE_OPLK8255X_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-linuxkernel.c)
        SET(MODULE_EDRV_SOURCE_FILES ${MODULE_OPLK8255X_SOURCE_FILES})

    ELSEIF(${OPLK_EDRV} STREQUAL "i210")

        SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=210")
        SET(MODULE_OPLKI210_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-i210.c)
        SET(MODULE_OPLKI210_SOURCE_FILES ${MODULE_OPLKI210_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-i210.c)
        SET(MODULE_EDRV_SOURCE_FILES ${MODULE_OPLKI210_SOURCE_FILES})
        IF(CFG_OPLK_MN)
            SET(MODULE_DEFS "${MODULE_DEFS} -DEDRV_USE_TTTX=TRUE")
        ELSE()
            SET(MODULE_DEFS "${MODULE_DEFS} -DEDRV_USE_TTTX=FALSE")
        ENDIF()

    ELSEIF(${OPLK_EDRV} STREQUAL "8111")

        SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=8111")
        SET(MODULE_OPLK8111_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-8111.c)
        SET(MODULE_OPLK8111_SOURCE_FILES ${MODULE_OPLK8111_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-linuxkernel.c)
        SET(MODULE_EDRV_SOURCE_FILES ${MODULE_OPLK8111_SOURCE_FILES})

    ELSEIF(${OPLK_EDRV} STREQUAL "emacps")
        IF(CMAKE_SYSTEM_PROCESSOR MATCHES arm)
            SET(MODULE_DEFS "${MODULE_DEFS_GEN} -DCONFIG_EDRV=267200")
            # 267200 corresponds to Z7200 arch of Zynq family. (Z is represented by 26)
            SET(MODULE_EMACPS_SOURCE_FILES ${MODULE_SOURCE_FILES} ${EDRV_SOURCE_DIR}/edrv-emacps.c)
            # TODO: Interrrupt is not generated from ttc module from Linux, so the driver was updated to use hrestimer-linuxkernel instead
            # SET(MODULE_EMACPS_SOURCE_FILES ${MODULE_EMACPS_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-zynqttc.c)
            SET(MODULE_EMACPS_SOURCE_FILES ${MODULE_EMACPS_SOURCE_FILES} ${KERNEL_SOURCE_DIR}/timer/hrestimer-linuxkernel.c)
            SET(MODULE_EDRV_SOURCE_FILES ${MODULE_EMACPS_SOURCE_FILES})
        ELSE()
            MESSAGE(FATAL_ERROR "EmacPs supported only for ARM processor")
        ENDIF()
    ELSE()
        MESSAGE(FATAL_ERROR
                "No valid ethernet driver was specified during the edrvBuild() call")
    ENDIF()

    # Clear MODULE_OBJS content if already set by a previous edrv build.
    SET(MODULE_OBJS)

    # Make sure that MODULE_LINKED_SOURCE_FILES is empty to avoid issue during create_symlink.
    SET(MODULE_LINKED_SOURCE_FILES)

    # Link source files to do an out-of-source build
    FOREACH(MODULE_SOURCE_FILE ${MODULE_EDRV_SOURCE_FILES})
        GET_FILENAME_COMPONENT(MODULE_SOURCE_FILENAME ${MODULE_SOURCE_FILE} NAME)
        GET_FILENAME_COMPONENT(MODULE_SOURCE_BASENAME ${MODULE_SOURCE_FILE} NAME_WE)
        ADD_CUSTOM_COMMAND(OUTPUT ${MODULE_OUTPUT_DIR}/src/${MODULE_SOURCE_FILENAME}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${MODULE_OUTPUT_DIR}/src
                        COMMAND ${CMAKE_COMMAND} -E create_symlink ${MODULE_SOURCE_FILE} ${MODULE_OUTPUT_DIR}/src/${MODULE_SOURCE_FILENAME}
                        )
        SET(MODULE_OBJS "${MODULE_OBJS} src/${MODULE_SOURCE_BASENAME}.o")
        SET(MODULE_LINKED_SOURCE_FILES ${MODULE_LINKED_SOURCE_FILES} ${MODULE_OUTPUT_DIR}/src/${MODULE_SOURCE_FILENAME})
    ENDFOREACH()

    CONFIGURE_FILE(${TOOLS_DIR}/linux/Kbuild.in ${MODULE_OUTPUT_DIR}/Kbuild)

    ADD_CUSTOM_COMMAND(
        OUTPUT ${MODULE_OUTPUT_DIR}/${MODULE_FILE}
        COMMAND ${CMAKE_MAKE_PROGRAM} ${MAKE_KERNEL_CROSSCOMPILING_PARAMS} -C ${KERNEL_DIR} M=${MODULE_OUTPUT_DIR} modules
        WORKING_DIRECTORY ${MODULE_OUTPUT_DIR}
        DEPENDS ${MODULE_LINKED_SOURCE_FILES} ${TOOLS_DIR}/linux/Kbuild.in
        VERBATIM
    )

    ADD_CUSTOM_TARGET(
        ${MODULE_NAME}
        ALL
        DEPENDS ${MODULE_OUTPUT_DIR}/${MODULE_FILE}
        )

    ADD_CUSTOM_TARGET(
        module_${MODULE_NAME}_clean
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${KERNEL_DIR} M=${MODULE_OUTPUT_DIR} clean
        )

    INSTALL(FILES ${MODULE_OUTPUT_DIR}/${MODULE_FILE} DESTINATION ${PROJECT_NAME})

ENDMACRO()

# include cmake modules
INCLUDE(CMakeDependentOption)

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    CMAKE_MINIMUM_REQUIRED (VERSION 2.8.7)
ELSE()
    MESSAGE(FATAL_ERROR "Unsupported system ${CMAKE_SYSTEM_NAME} for this project!")
ENDIF()

MESSAGE(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "CMAKE_SYSTEM_PROCESSOR is ${CMAKE_SYSTEM_PROCESSOR}")

STRING(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_DIR)
STRING(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_PROCESSOR_DIR)

# Since 3.14 kernel Werror=date-time is automatically used if the compiler supports it.
SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -Wno-date-time")

################################################################################
# Configuration options

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF()

OPTION (CFG_OPLK_MN "Compile openPOWERLINK MN (Otherwise CN)" ON)
CMAKE_DEPENDENT_OPTION(CFG_INCLUDE_MN_REDUNDANCY "Compile MN redundancy functions into MN driver" OFF
                       "CFG_OPLK_MN" OFF)

SET(CFG_DEBUG_LVL "0xC0000000L" CACHE STRING "Debug Level for debug output")

SET(CFG_KERNEL_DIR "" CACHE PATH
        "Select the kernel directory to be used, if not specified, system kernel dir will be used!")

OPTION (CFG_POWERLINK_EDRV_82573 "Compile 82573 OPLK driver" ON)
OPTION (CFG_POWERLINK_EDRV_8139 "Compile 8139 OPLK driver" OFF)
OPTION (CFG_POWERLINK_EDRV_8255X "Compile 8255x OPLK driver" OFF)
OPTION (CFG_POWERLINK_EDRV_I210 "Compile i210 OPLK driver" OFF)
OPTION (CFG_POWERLINK_EDRV_8111 "Compile 8111 OPLK driver" OFF)
OPTION (CFG_POWERLINK_EDRV_EMACPS "Compile emacps OPLK driver" OFF)

################################################################################
# Set global directories
################################################################################
SET(OPLK_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
SET(CONTRIB_SOURCE_DIR ${OPLK_BASE_DIR}/contrib)
SET(STACK_SOURCE_DIR ${OPLK_BASE_DIR}/stack/src)
SET(COMMON_SOURCE_DIR ${OPLK_BASE_DIR}/stack/src/common)
SET(ARCH_SOURCE_DIR ${OPLK_BASE_DIR}/stack/src/arch/linuxkernel)
SET(KERNEL_SOURCE_DIR ${OPLK_BASE_DIR}/stack/src/kernel)
SET(EDRV_SOURCE_DIR ${OPLK_BASE_DIR}/stack/src/kernel/edrv)
SET(OPLK_INCLUDE_DIR ${OPLK_BASE_DIR}/stack/include)
SET(TOOLS_DIR ${OPLK_BASE_DIR}/tools)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    ${OPLK_BASE_DIR}/bin/${SYSTEM_NAME_DIR}/${SYSTEM_PROCESSOR_DIR}/
    CACHE PATH "openPOWERLINK kernel driver install prefix" FORCE
    )
ENDIF()

###############################################################################
# specifiy standard definitions
###############################################################################
IF(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL DEBUG)
    SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -D_DEBUG")
    #SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -D_DBG_TRACE_POINTS_")
    SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -DDEF_DEBUG_LVL=${CFG_DEBUG_LVL}")
ENDIF()

###############################################################################
# specifiy standard include directories
###############################################################################
INCLUDE_DIRECTORIES(${OPLK_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CONTRIB_SOURCE_DIR})
INCLUDE_DIRECTORIES(${COMMON_SOURCE_DIR}/circbuf)
INCLUDE_DIRECTORIES(${KERNEL_SOURCE_DIR}/errhnd)
INCLUDE_DIRECTORIES(${KERNEL_SOURCE_DIR}/dll)

###############################################################################
# specifiy source files
###############################################################################
SET(MODULE_SOURCE_FILES
    ${MODULE_SOURCE_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CONTRIB_SOURCE_DIR}/trace/trace-printk.c
    ${EDRV_SOURCE_DIR}/edrvcyclic.c
    ${KERNEL_SOURCE_DIR}/ctrl/ctrlk.c
    ${KERNEL_SOURCE_DIR}/ctrl/ctrlkcal-direct.c
    ${KERNEL_SOURCE_DIR}/dll/dllk.c
    ${KERNEL_SOURCE_DIR}/dll/dllkfilter.c
    ${KERNEL_SOURCE_DIR}/dll/dllkstatemachine.c
    ${KERNEL_SOURCE_DIR}/dll/dllkevent.c
    ${KERNEL_SOURCE_DIR}/dll/dllkframe.c
    ${KERNEL_SOURCE_DIR}/dll/dllknode.c
    ${KERNEL_SOURCE_DIR}/dll/dllkcal.c
    ${KERNEL_SOURCE_DIR}/dll/dllkcal-circbuf.c
    ${KERNEL_SOURCE_DIR}/event/eventk.c
    ${KERNEL_SOURCE_DIR}/event/eventkcal-linuxkernel.c
    ${KERNEL_SOURCE_DIR}/event/eventkcalintf-circbuf.c
    ${KERNEL_SOURCE_DIR}/nmt/nmtk.c
    ${KERNEL_SOURCE_DIR}/pdo/pdok.c
    ${KERNEL_SOURCE_DIR}/pdo/pdokcal.c
    ${KERNEL_SOURCE_DIR}/pdo/pdoklut.c
    ${KERNEL_SOURCE_DIR}/pdo/pdokcal-triplebufshm.c
    ${KERNEL_SOURCE_DIR}/pdo/pdokcalmem-linuxkernel.c
    ${KERNEL_SOURCE_DIR}/timesync/timesynck.c
    ${KERNEL_SOURCE_DIR}/timesync/timesynckcal-linuxkernel.c
    ${KERNEL_SOURCE_DIR}/errhnd/errhndk.c
    ${KERNEL_SOURCE_DIR}/errhnd/errhndkcal.c
    ${KERNEL_SOURCE_DIR}/errhnd/errhndkcal-local.c
    ${KERNEL_SOURCE_DIR}/veth/veth-linuxkernel.c
    ${COMMON_SOURCE_DIR}/circbuf/circbuffer.c
    ${COMMON_SOURCE_DIR}/circbuf/circbuf-linuxkernel.c
    ${COMMON_SOURCE_DIR}/bufalloc/bufalloc.c
    ${COMMON_SOURCE_DIR}/debugstr.c
    ${ARCH_SOURCE_DIR}/target-linuxkernel.c
    )

###############################################################################
# Configure depending selected mode
###############################################################################
IF(CFG_OPLK_MN)
    IF(CFG_INCLUDE_MN_REDUNDANCY)
         SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -DCONFIG_INCLUDE_NMT_RMN")
    ENDIF()
    SET(MODULE_DEFS_GEN "${MODULE_DEFS_GEN} -DCONFIG_MN")
    SET(MODULE_NAME_SUFFIX "mn")
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/proj/mn)
ELSE()
    SET(MODULE_NAME_SUFFIX "cn")
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/proj/cn)
ENDIF()

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.86|x86(_64)?)$")
    SET(MODULE_SOURCE_FILES ${MODULE_SOURCE_FILES} ${COMMON_SOURCE_DIR}/ami/amix86.c)
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES arm*)
    SET(MODULE_SOURCE_FILES ${MODULE_SOURCE_FILES} ${COMMON_SOURCE_DIR}/ami/amile.c)
ELSE()
    MESSAGE(FATAL_ERROR
            "ami: CMAKE_SYSTEM_PROCESSOR is set to ${CMAKE_SYSTEM_PROCESSOR}. Valid targets are (x86, x86_64, arm, armv7l, i686).")
ENDIF()

IF (CFG_KERNEL_DIR STREQUAL "")
    SET(KERNEL_DIR "/lib/modules/${CMAKE_SYSTEM_VERSION}/build")
ELSE()
    SET(KERNEL_DIR ${CFG_KERNEL_DIR})
ENDIF()

IF(CMAKE_CROSSCOMPILING)
    IF(DEFINED MAKE_KERNEL_ARCH)
        SET(MAKE_KERNEL_CROSSCOMPILING_PARAMS ARCH=${MAKE_KERNEL_ARCH})
    ELSE()
        MESSAGE(WARNING "ARCH is not set while cross-compiling !\n"
            "Set MAKE_KERNEL_ARCH with your target architecture.")
    ENDIF()

    IF(DEFINED MAKE_KERNEL_CROSS_COMPILE)
        SET(MAKE_KERNEL_CROSSCOMPILING_PARAMS
                ${MAKE_KERNEL_CROSSCOMPILING_PARAMS} CROSS_COMPILE=${MAKE_KERNEL_CROSS_COMPILE})
    ELSE()
        MESSAGE(WARNING "CROSS_COMPILE is not set while cross-compiling !\n"
            "Set MAKE_KERNEL_CROSS_COMPILE with your toolchain prefix")
    ENDIF()
ENDIF()

###############################################################################
# Build only selected kernel modules
###############################################################################

IF(CFG_POWERLINK_EDRV_82573)
    edrvBuild(82573)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplk82573${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_82573)

IF(CFG_POWERLINK_EDRV_8139)
    edrvBuild(8139)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplk8139${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_8139)

IF(CFG_POWERLINK_EDRV_8255X)
    edrvBuild(8255x)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplk8255x${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_8255X)

IF(CFG_POWERLINK_EDRV_I210)
    edrvBuild(i210)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplki210${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_I210)

IF(CFG_POWERLINK_EDRV_8111)
    edrvBuild(8111)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplk8111${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_8111)

IF(CFG_POWERLINK_EDRV_EMACPS)
    edrvBuild(emacps)
    SET(OPLK_CLEAN_TARGET ${OPLK_CLEAN_TARGET} module_oplkemacps${MODULE_NAME_SUFFIX}_clean)
ENDIF(CFG_POWERLINK_EDRV_EMACPS)

###############################################################################
# Clean all modules build directories
###############################################################################
ADD_CUSTOM_TARGET(
    module_clean
    COMMAND ${CMAKE_MAKE_PROGRAM} ${OPLK_CLEAN_TARGET}
    )

################################################################################
# add installation rules

INSTALL(FILES ${TOOLS_DIR}/linux/50-openPOWERLINK.rules DESTINATION ${PROJECT_NAME})
INSTALL(PROGRAMS ${TOOLS_DIR}/linux/plkload DESTINATION ${PROJECT_NAME})
INSTALL(PROGRAMS ${TOOLS_DIR}/linux/plkunload DESTINATION ${PROJECT_NAME})
INSTALL(PROGRAMS ${TOOLS_DIR}/linux/devices.txt DESTINATION ${PROJECT_NAME})
