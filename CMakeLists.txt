cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

file(GLOB_RECURSE SRC
	#self written C++
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/*.h"
	"${CMAKE_SOURCE_DIR}/src/*.hpp"
	#non-compiled library .cpp files
	"${CMAKE_SOURCE_DIR}/libs/header/*.cpp"
)

project(main)
add_executable(${PROJECT_NAME} ${SRC})
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") 
# This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="./assets/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the project

#------------------------------ ABOVE IS STANDARD FOR EVERY PROJECT (except for project name) ------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_SOURCE_DIR}/libs/header"
	"${CMAKE_SOURCE_DIR}/libs/compiled/SDL2/include"
)

#linking all the libraries
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
