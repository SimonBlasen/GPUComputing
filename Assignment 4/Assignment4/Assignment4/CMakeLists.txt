cmake_minimum_required (VERSION 2.8.3) 
project (GPUComputing) 

# Add our modules to the path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")


include(CheckCXXCompilerFlag)
if (WIN32)
else (WIN32)
    #set (EXTRA_COMPILE_FLAGS "-Wall -Werror")
    set (EXTRA_COMPILE_FLAGS "-Wall")
    CHECK_CXX_COMPILER_FLAG(-std=c++11 HAS_CXX_11)
    if (HAS_CXX_11)
        set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -std=c++11")
        message(STATUS "Enabling C++11 support")
    else(HAS_CXX_11)
        message(WARNING "No C++11 support detected, build will fail.")
    endif()
    set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}")
endif (WIN32)

# Include support for changing the working directory in Visual Studio
include(ChangeWorkingDirectory)

# Search for OpenCL and add paths
find_package( OpenCL REQUIRED )

include_directories( ${OPENCL_INCLUDE_DIRS} )

# Include Common module
add_subdirectory (../Common ${CMAKE_BINARY_DIR}/Common)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "fwrwere")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "aegegherh")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "aegegherh")
set(GLFW_INSTALL OFF CACHE BOOL "fasdfa")

add_subdirectory(glfw)
include_directories(glfw/include)

add_subdirectory(glew)
include_directories(glew/include)

# Define source files for this assignment
set(sources
  clothsim.cl
  ParticleSystem.cl
  Scan.cl
  CClothSimulationTask.cpp
  CParticleSystemTask.cpp
  CAssignment4.cpp
  CTriMesh.cpp
  CGLTexture.cpp
  GLCommon.cpp
  main.cpp
  HLSL.cpp
  forcefield.frag
  mesh.frag
  meshtextured.frag
  particles.frag
  CClothSimulationTask.h
  CParticleSystemTask.h
  CAssignment4.h
  CTriMesh.h
  CGLTexture.h
  GLCommon.h
  HLSL.h
  HLSLEx.h
  forcefield.vert
  mesh.vert
  meshtextured.vert
  particles.vert
  )
ADD_EXECUTABLE (Assignment 
	${sources}
	)

  # Link required libraries
  set(GLEW_LIBRARIES "glew")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(GLEW_LIBRARIES "${GLEW_LIBRARIES}64")
  else()
	set(GLEW_LIBRARIES "${GLEW_LIBRARIES}32")
  endif()

  set(GLU_LIBRARIES "GLU")
  if(WIN32)
    set(GLU_LIBRARIES "${GLU_LIBRARIES}32")
  endif()
target_link_libraries(Assignment ${OPENCL_LIBRARIES} glfw ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${GLU_LIBRARIES})
target_link_libraries(Assignment GPUCommon)


if (WIN32)
	add_custom_command(TARGET Assignment POST_BUILD COMMAND  ${CMAKE_COMMAND} -E $<1:copy_if_different> $<0:echo> $<TARGET_FILE_DIR:${GLEW_LIBRARIES}>/${GLEW_LIBRARIES}.dll $<1:$<TARGET_FILE_DIR:Assignment>> )
	change_workingdir(Assignment ${CMAKE_SOURCE_DIR})
endif()
