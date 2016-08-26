
# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE


# Default values
option(ENABLE_PULSEGATEWAY "Enables pulse gateway" ON)
option(ENABLE_NETWORKGATEWAY "Enables network gateway" ON)
option(ENABLE_DEVICENODEGATEWAY "Enables device node gateway" ON)
option(ENABLE_DBUSGATEWAY "Enables DBus gateway" ON)
option(ENABLE_CGROUPSGATEWAY "Enables cgroups gateway" ON)

# Pulse is a special case since it has a dependency package
if(ENABLE_PULSEGATEWAY)
    pkg_check_modules(DEPENDENCIES REQUIRED libpulse)
endif(ENABLE_PULSEGATEWAY)
add_definitions(${DEPENDENCIES_CFLAGS})

# Enable or disable support for gateways compile-time.
SET(GATEWAY_SOURCES "")
SET(GATEWAY_HEADERS "")
macro(gateway_support flag strname hfile filename)
    if (${flag})
        MESSAGE(INFO " Compiling with gateway support for ${strname}")
        LIST(APPEND GATEWAY_SOURCES gateway/${filename})
        LIST(APPEND GATEWAY_HEADERS gateway/${hfile})
        ADD_DEFINITIONS(-D${flag})
    else(${flag})
        MESSAGE(INFO " Compiling without gateway support for ${strname}")
    endif(${flag})
endmacro(gateway_support flag filename hfile)

gateway_support(ENABLE_CGROUPSGATEWAY cgroups cgroupsgateway.h cgroupsgateway.cpp)
gateway_support(ENABLE_DBUSGATEWAY DBus dbusgateway.h dbusgateway.cpp)
gateway_support(ENABLE_DEVICENODEGATEWAY DeviceNode devicenodegateway.h devicenodegateway.cpp)
gateway_support(ENABLE_NETWORKGATEWAY network networkgateway.h networkgateway.cpp)
gateway_support(ENABLE_PULSEGATEWAY pulse pulsegateway.h pulsegateway.cpp)

SET(DEFAULT_GATEWAYS TRUE)
gateway_support(DEFAULT_GATEWAYS env envgateway.h envgateway.cpp)
gateway_support(DEFAULT_GATEWAYS file filegateway.h filegateway.cpp)
gateway_support(DEFAULT_GATEWAYS wayland waylandgateway.h waylandgateway.cpp)

add_library(softwarecontainerLib SHARED 
    container.cpp
    gateway/gateway.cpp
    generators.cpp
    generators.h
    libsoftwarecontainer.cpp
    ${GATEWAY_SOURCES}
    ${GATEWAY_HEADERS}
    ${PROJECT_SOURCE_DIR}/common/common.cpp
)

TARGET_LINK_LIBRARIES( softwarecontainerLib
    ${DEPENDENCIES_LIBRARIES}
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${IVIMainLoop_LIBRARIES}
    ${Jansson_LIBRARIES}
)

include_directories(${LIBSOFTWARECONTAINER_DIR}/include)

install(TARGETS softwarecontainerLib DESTINATION lib)
set_target_properties(softwarecontainerLib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})