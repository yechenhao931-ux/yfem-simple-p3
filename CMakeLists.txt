cmake_minimum_required(VERSION 3.16)

project(yfem_serial_mfem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

foreach(example ex1 ex16)
  add_executable(${example} examples/${example}.cpp)
  target_link_libraries(${example} PRIVATE mfem)
endforeach()
