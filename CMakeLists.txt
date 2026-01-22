#CMake template for C++ projects with libraries
cmake_minimum_required(VERSION 3.16)

project(main)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT SWITCH)
	# set(CMAKE_CXX_FLAGS_DEBUG "/O2")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2")

	#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
endif()

file(GLOB_RECURSE SRC
	#self written C++
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/*.h"
	"${CMAKE_SOURCE_DIR}/src/*.hpp"
	#non-compiled library .cpp files
	"${CMAKE_SOURCE_DIR}/libs/header/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC})

if (SWITCH)
	target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=\"/switch/2d-ecs-engine/assets/\")
	target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		ASSETS_PATH=\"$<IF:$<CONFIG:Debug>,${CMAKE_CURRENT_SOURCE_DIR}/assets/,./assets/>\"
	)
endif()

# for shipping rename and zip the Release folder in /build

#------------------------------ ABOVE IS STANDARD FOR EVERY PROJECT (except for project name) ------------------------------

#.h files of libraries
target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_SOURCE_DIR}/libs/header"
)

if (SWITCH)
	find_package(SDL2 REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(freetype REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
	pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

	#FIXME: Lua 5.1 is the latest version provided by devkitPro
	target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_INCLUDE_DIRS}
		${SDL2_IMAGE_INCLUDE_DIRS})
	target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_CFLAGS_OTHER}
		${SDL2_IMAGE_CFLAGS_OTHER})
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 ${SDL2_MIXER_LIBRARIES}
		${SDL2_IMAGE_LIBRARIES} SDL2_ttf lua5.1 freetype)

	nx_generate_nacp(${PROJECT_NAME}.nacp
		NAME "Jumpy Avian"
		AUTHOR "EnesAkkusci"
		VERSION "v1.0"
	)

	nx_create_nro(${PROJECT_NAME}
		NACP ${PROJECT_NAME}.nacp
		ICON ${CMAKE_CURRENT_SOURCE_DIR}/SwitchIcon.jpg
	)
else()
	#linking all the libraries
	target_include_directories(${PROJECT_NAME} PUBLIC
		"${CMAKE_SOURCE_DIR}/libs/compiled/SDL2/include"
	)
	
	target_link_libraries(${PROJECT_NAME} PRIVATE 
		"${CMAKE_SOURCE_DIR}/libs/compiled/SDL2/lib/x64/*.lib"
		"${CMAKE_SOURCE_DIR}/libs/compiled/lua/liblua54.a"
	)

	#precompiled dynamic library dlls
	file(GLOB ALL_DLLS
		"${CMAKE_SOURCE_DIR}/libs/compiled/SDL2/lib/x64/*.dll"
		"${CMAKE_SOURCE_DIR}/libs/compiled/lua/lua54.dll"
	)
	#place the dlls next to the executable
	foreach(dll IN LISTS ALL_DLLS)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${dll}" $<TARGET_FILE_DIR:${PROJECT_NAME}>
	)
	endforeach()

	# place the assets folder next to the .exe only in Release builds
	if (CMAKE_BUILD_TYPE STREQUAL "Release" OR "$<CONFIG>" STREQUAL "Release")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_directory
				"${CMAKE_SOURCE_DIR}/assets"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
	)
	endif()
endif()
