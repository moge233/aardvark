cmake_minimum_required(VERSION 3.10)

project(aardvark VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "/media/matt/HDD1/buildroot-2025.02.10/output/host/bin/aarch64-buildroot-linux-gnu-gcc")
set(CMAKE_CXX_STANDARD 20)

set(
	SOURCES
	main.cpp
	drivers/src/led.cpp
	drivers/src/usbtmc.cpp
	platform/src/blockingqueue.cpp
	platform/src/commandinterface.cpp
	platform/src/commandmessage.cpp
	platform/src/errors.cpp
	platform/src/logicaldevice.cpp
	platform/src/lua.cpp
	platform/src/scriptprocessor.cpp
	platform/src/status.cpp
)
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

# target_compile_definitions(aardvark PRIVATE DAEMONIZE)

target_include_directories(
	aardvark
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/drivers/inc
	${CMAKE_CURRENT_SOURCE_DIR}/platform/inc
	/media/matt/HDD1/buildroot_apps/lua-5.4.6/src
)

target_link_libraries(aardvark PRIVATE -lstdc++)
target_link_libraries(aardvark PRIVATE m)
target_link_libraries(aardvark PRIVATE lua)