cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ElevenMPV)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Eleven Music Player")
set(VITA_TITLEID  "ELEVENMPV")
set(VITA_VERSION  "02.10")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

FUNCTION(ADD_RESOURCES out_var)
  SET(result)
  FOREACH(in_f ${ARGN})
    SET(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
    GET_FILENAME_COMPONENT(out_dir ${out_f} DIRECTORY)
    ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
      COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_f}
      DEPENDS ${in_f}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Building resource ${out_f}"
      VERBATIM
      )
    LIST(APPEND result ${out_f})
  ENDFOREACH()
  SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()

file(GLOB res_files RELATIVE
  ${CMAKE_SOURCE_DIR}
  res/*.png
)
add_resources(elevenmp_res ${res_files})

include_directories(
  libs/include
  include
  include/audio
  include/menus
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libs/lib
)

add_executable(${PROJECT_NAME}
  ${elevenmp_res}
  source/common.c
  source/config.c
  source/dirbrowse.c
  source/fs.c
  source/main.c
  source/status_bar.c
  source/textures.c
  source/touch.c
  source/utils.c
  source/audio/audio.c
  source/audio/flac.c
  source/audio/mp3.c
  source/audio/ogg.c
  source/audio/opus.c
  source/audio/vitaaudiolib.c
  source/audio/wav.c
  source/audio/xm.c
  source/menus/menu_audioplayer.c
  source/menus/menu_displayfiles.c
  source/menus/menu_settings.c
)

target_link_libraries(${PROJECT_NAME}
  vita2d
  freetype
  png
  jpeg
  z
  c
  FLAC
  opusfile
  opus
  vorbisfile
  vorbis
  ogg
  xmp-lite
  m
  mpg123
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceAudiodec_stub
  SceCommonDialog_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  ScePower_stub
  SceRtc_stub
  SceShellSvc_stub
  SceSysmodule_stub
  SceTouch_stub
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE res/Roboto-Regular.ttf Roboto-Regular.ttf
)
