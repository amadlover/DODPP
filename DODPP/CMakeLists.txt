cmake_minimum_required (VERSION 3.8)

add_compile_options ($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_compile_options ($<$<CXX_COMPILER_ID:MSVC>:/GR->)


add_definitions (-DUNICODE -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)

include_directories ($ENV{VULKAN_SDK}/Include)
include_directories (${CMAKE_SOURCE_DIR}/External/stb)

if (NOT CMAKE_CL_64)
	link_directories ($ENV{VULKAN_SDK}/Lib32)
endif ()

if (CMAKE_CL_64)
	link_directories ($ENV{VULKAN_SDK}/Lib)
endif ()


set (HPP error.hpp game.hpp log.hpp graphics.hpp types.hpp vulkan_interface.hpp utils.hpp compute.hpp actor_vert.hpp actor_frag.hpp vk_utils.hpp)
set (CPP main.cpp game.cpp log.cpp graphics.cpp types.cpp vulkan_interface.cpp utils.cpp compute.cpp vk_utils.cpp)

add_executable (DODPP WIN32 ${HPP} ${CPP})
target_link_libraries (DODPP vulkan-1)
