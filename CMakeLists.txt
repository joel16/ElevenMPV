cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x4000 attribute 0x08 ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

project(ElevenMPV)
dolce_gen_libs(SceShellSvc_stubs
  SceShellSvc.yml
  LIB SceShellSvc_stub_weak)
dolce_gen_libs(SceLibc_stubs
  SceLibc.yml
  LIB SceLibc_stub_weak)
set(VITA_APP_NAME "Eleven Music Player")
set(VITA_TITLEID  "ELEVENMPV")
set(VITA_VERSION  "03.50")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -Wl,--no-wchar-size-warning,--no-enum-size-warning -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
  source/common.c
  source/config.c
  source/dirbrowse.c
  source/fs.c
  source/main.c
  source/textures.c
  source/touch.c
  source/motion.c
  source/utils.c
  source/audio/audio.c
  source/audio/flac.c
  source/audio/mp3.c
  source/audio/ogg.c
  source/audio/opus.c
  source/audio/vitaaudiolib.c
  source/audio/wav.c
  source/audio/xm.c
  source/audio/aac.c
  source/audio/at9.c
  source/menus/menu_audioplayer.c
  source/menus/menu_displayfiles.c
  source/menus/menu_settings.c
)

target_link_libraries(${PROJECT_NAME}
  ShellAudio
  vita2d_sys
  FLAC
  opusfile
  opus
  vorbisfile
  vorbis
  ogg
  xmp-lite
  m
  mpg123
  taihen_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  ScePower_stub
  SceShellSvc_stub
  SceSysmodule_stub
  SceTouch_stub
  SceFios2_stub
  SceShellSvc_stub_weak
  SceSystemGesture_stub
  SceIme_stub
  ScePvf_stub
  SceMotion_stub
  SceVshBridge_stub
  SceLibc_stub_weak
  SceNotificationUtil_stub
)

set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -d ATTRIBUTE=17338760 -s CATEGORY=gdc" -d PARENTAL_LEVEL=1)

dolce_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin)

add_executable(exit_module
  module/source/main.c
)

target_compile_options(exit_module PRIVATE -std=gnu99)

target_link_libraries(exit_module
  taihenForKernel_stub
  taihenModuleUtils_stub
  "-nostdlib"
)

dolce_create_self(exit_module.skprx exit_module CONFIG module/exit_module.yml UNSAFE)

dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/pic0.png sce_sys/pic0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE ${CMAKE_CURRENT_BINARY_DIR}/exit_module.skprx module/exit_module.skprx
  FILE res/texture.gxt texture.gxt
)
