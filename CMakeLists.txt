# General project settings
project(libautopin+_linuxC C)
cmake_minimum_required(VERSION 2.6)

# Configuration options
option(STATIC "Build a static library" ON)

# Find pthread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Could not find pthread library")
endif (NOT CMAKE_USE_PTHREADS_INIT)

# Project include path
include_directories(src)

# Generating the Documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
  add_custom_command(
     OUTPUT htmldoc
     COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
     DEPENDS Doxyfile src/libautopin+_linuxC.c src/libautopin+_linuxC.h src/libautopin+_msg.h
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_target(
     doc
     DEPENDS htmldoc
  )

  add_custom_target(
    cleandoc
    COMMAND ${CMAKE_COMMAND} -E remove_directory doc/html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif (DOXYGEN_FOUND)

# Compile
if (STATIC)
  add_library(autopin+_linuxC STATIC src/libautopin+_linuxC.c)
else (STATIC)
  add_library(autopin+_linuxC SHARED src/libautopin+_linuxC.c)
endif (STATIC)

target_link_libraries(autopin+_linuxC ${CMAKE_THREAD_LIBS_INIT})
