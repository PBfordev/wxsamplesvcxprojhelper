#///////////////////////////////////////////////////////////////////////////////
#// Project:     wxSamples VCXProj Helper
#// Home:        https://github.com/PBfordev/wxsamplesvcxprojhelper
#// File Name:   data.h
#// Purpose:     Reading project files and writing command files
#// Author:      PB
#// Created:     2024-11-20
#// Copyright:   (c) 2024 PB
#// Licence:     wxWindows licence
#///////////////////////////////////////////////////////////////////////////////


cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(wxsamplesvcxprojhelper)

find_package(wxWidgets 3.2 COMPONENTS xml core base REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(SOURCES
  app.cpp
  data.h
  data.cpp
)
if(WIN32)
  list(APPEND SOURCES "app.rc")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

if(wxWidgets_USE_FILE)
  include(${wxWidgets_USE_FILE})
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})
if(MINGW) # work around the breaking change in wxWidgets 3.3
  target_link_libraries(${PROJECT_NAME} PRIVATE gdiplus msimg32)
endif()

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE wxUSE_RC_MANIFEST wxUSE_DPI_AWARE_MANIFEST=2) 
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE YES)

  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NON_CONFORMING_SWPRINTFS _SCL_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MP)
    target_link_options(${PROJECT_NAME} PRIVATE /MANIFEST:NO)
  else() # GCC or clang
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
  endif()
endif()