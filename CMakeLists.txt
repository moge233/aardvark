cmake_minimum_required(VERSION 3.10)

project(aardvark VERSION 1.0 LANGUAGES C CXX)

option(
	DISPLAY "Build with display support" OFF
)

option(
	DAEMONIZE "Daemonize the process" ON
)

option(
	BUILD_WITH_M4_SUPPORT "Include rpmsg to communicate with m4 core" OFF
)

set(CMAKE_CXX_COMPILER "/media/matt/HDD1/buildroot-2025.02.10/output/host/bin/aarch64-buildroot-linux-gnu-gcc")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "/media/matt/HDD1/buildroot-2025.02.10/output/host/bin/aarch64-buildroot-linux-gnu-gcc")
set(CMAKE_C_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(-Wall)

set(
	SOURCES
	main.cpp
	drivers/src/led.cpp
	drivers/src/usbtmc.cpp
	drivers/rpcgen/vxi11_xdr.c
	drivers/src/vxi11.cpp
	platform/src/circularbuffer.cpp
	platform/src/commandinterface.cpp
	platform/src/commandmessage.cpp
	platform/src/endpoint.cpp
	platform/src/errors.cpp
	platform/src/lua.cpp
	platform/src/monitor.cpp
	platform/src/osalthread.cpp
	platform/src/scriptprocessor.cpp
	platform/src/status.cpp
)

if (BUILD_WITH_M4_SUPPORT)
	list(
		APPEND
		SOURCES
		drivers/src/rpmsg.cpp
	)
endif()

set(
	LUA_SRC
	/media/matt/HDD1/buildroot_apps/lua-5.4.6/src
)

add_library(lua STATIC IMPORTED)
set_target_properties(
	lua
	PROPERTIES
	IMPORTED_LOCATION /media/matt/HDD1/buildroot_apps/lua-5.4.6/src/liblua.a
	IMPORTED_INCLUDE_DIRECTORIES /media/matt/HDD1/buildroot_apps/lua-5.4.6/src
)

add_executable(aardvark ${SOURCES})

if (DAEMONIZE)
	target_compile_definitions(aardvark PUBLIC DAEMONIZE)
endif()

if (DISPLAY)
	target_compile_definitions(
		aardvark
		PUBLIC
		BUILD_WITH_DISPLAY
	)
	target_include_directories(
		aardvark
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/display/inc
	)
	target_link_directories(
		aardvark
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/display/build
	)
endif()

if (BUILD_WITH_M4_SUPPORT)
	target_compile_definitions(
		aardvark
		PUBLIC
		BUILD_WITH_M4
	)
endif()

target_include_directories(
	aardvark
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/drivers/inc
	${CMAKE_CURRENT_SOURCE_DIR}/drivers/rpcgen
	${CMAKE_CURRENT_SOURCE_DIR}/platform/inc
	/media/matt/HDD1/buildroot_apps/lua-5.4.6/src
	/media/matt/HDD1/buildroot-2025.02.10/output/host/include/tirpc
)

target_link_libraries(aardvark PRIVATE -lstdc++)
target_link_libraries(aardvark PRIVATE m)
target_link_libraries(aardvark PRIVATE lua)
if (DISPLAY)
	target_link_libraries(aardvark PRIVATE display)
endif()
target_link_libraries(aardvark PRIVATE udev)
target_link_libraries(aardvark PRIVATE tirpc)
