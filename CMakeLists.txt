cmake_minimum_required(VERSION 3.16)
project(xrealAirLinuxDriver C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LIBEVDEV REQUIRED)
find_package(LIBUSB REQUIRED)

set(VITURE_LIB_NAME viture_sdk)

execute_process(COMMAND git submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(modules/xrealInterfaceLibrary/interface_lib)

add_library (
	${VITURE_LIB_NAME}
        STATIC
	IMPORTED)

set_target_properties(
    ${VITURE_LIB_NAME} PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libviture_one_sdk.a
)

add_executable(xrealAirLinuxDriver
		src/buffer.c
		src/config.c
		src/driver.c
		src/files.c
		src/imu.c
		src/ipc.c
		src/multitap.c
		src/outputs.c
		src/plugins.c
		src/runtime_context.c
		src/sideview_plugin.c
		src/state.c
		src/strings.c
		src/virtual_display_plugin.c
		src/viture.c
		src/xreal.c
)

target_include_directories(xrealAirLinuxDriver
		SYSTEM BEFORE PRIVATE
		${LIBEVDEV_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/modules/xrealInterfaceLibrary/interface_lib/include
		${LIBUSB_INCLUDE_DIR}
)

target_link_libraries(xrealAirLinuxDriver
		PRIVATE ${LIBEVDEV_LIBRARIES} nrealAirLibrary Threads::Threads m ${VITURE_LIB_NAME} ${LIBUSB_LIBRARY}
)