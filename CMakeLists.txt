cmake_minimum_required(VERSION 3.16)

project(yfem_serial_mfem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMAKE_BUILD_TYPE 可从命令行传入，例如：
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# 未指定时默认使用 Debug。
set(YFEM_BUILD_TYPES Debug Release)

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
else()
  set(_yfem_build_type_valid FALSE)

  foreach(_build_type IN LISTS YFEM_BUILD_TYPES)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _requested_build_type)
    string(TOLOWER "${_build_type}" _known_build_type)

    if(_requested_build_type STREQUAL _known_build_type)
      set(CMAKE_BUILD_TYPE "${_build_type}" CACHE STRING "Choose the type of build." FORCE)
      set(_yfem_build_type_valid TRUE)
      break()
    endif()
  endforeach()

  if(NOT _yfem_build_type_valid)
    message(FATAL_ERROR
      "Unsupported CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}'. "
      "Supported values: ${YFEM_BUILD_TYPES}")
  endif()
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${YFEM_BUILD_TYPES})
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 清空 CMake 默认配置 flags，统一通过 target_compile_options 设置。
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "Flags used by the CXX compiler during Debug builds." FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "Flags used by the CXX compiler during Release builds." FORCE)

file(GLOB_RECURSE MFEM_SERIAL_SOURCES CONFIGURE_DEPENDS
  general/*.cpp
  linalg/*.cpp
  mesh/*.cpp
  fem/*.cpp)

foreach(src IN LISTS MFEM_SERIAL_SOURCES)
  if (src MATCHES "/vtkhdf" OR
      src MATCHES "/particleset")
    list(REMOVE_ITEM MFEM_SERIAL_SOURCES "${src}")
  endif()
endforeach()

add_library(mfem STATIC ${MFEM_SERIAL_SOURCES})
target_include_directories(mfem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

function(yfem_apply_compile_options target_name)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    #target_compile_options(${target_name} PRIVATE -Wall -Wextra)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_compile_options(${target_name} PRIVATE -O0 -g)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${target_name} PRIVATE -O3)
      target_compile_definitions(${target_name} PRIVATE NDEBUG)
    endif()
  elseif(MSVC)
    target_compile_options(${target_name} PRIVATE /W3)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_compile_options(${target_name} PRIVATE /Od /Zi)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${target_name} PRIVATE /O2)
      target_compile_definitions(${target_name} PRIVATE NDEBUG)
    endif()
  endif()
endfunction()

yfem_apply_compile_options(mfem)

foreach(example ex1 ex16)
  add_executable(${example} examples/${example}.cpp)
  target_link_libraries(${example} PRIVATE mfem)
  yfem_apply_compile_options(${example})
endforeach()

# Data factory: batch transient-heat FEM runs exported as graph time-series
# for training a GNN surrogate.
add_executable(gen_heat_dataset tools/gen_heat_dataset.cpp)
target_link_libraries(gen_heat_dataset PRIVATE mfem)
yfem_apply_compile_options(gen_heat_dataset)
