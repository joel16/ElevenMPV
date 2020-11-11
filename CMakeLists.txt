cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

project(ElevenMPV)
set(VITA_APP_NAME "Eleven Music Player")
set(VITA_TITLEID  "GRVA00008")
set(VITA_VERSION  "03.80")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Os -Wl,--no-wchar-size-warning,--no-enum-size-warning -nostdlib")
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
  source/audio/at3.c
  source/audio/id3.c
  source/menus/menu_audioplayer.c
  source/menus/menu_displayfiles.c
  source/menus/menu_settings.c
  source/psp2_compat.c
  #source/fios2_common.c
)

target_link_libraries(${PROJECT_NAME}
  vita2d_sys
  opusfile
  opus
  vorbisfile
  vorbis
  ogg
  xmp-lite
  png16v2ds
  z
  ShellAudio
  SceLibKernel_stub
  SceSysmem_stub
  SceKernelThreadMgr_stub
  SceKernelModuleMgr_stub
  SceProcessMgr_stub
  SceIofilemgr_stub
  SceLibRng_stub
  taihen_stub
  SceAppMgr_stub
  SceSharedFb_stub
  SceAppUtil_stub
  SceAudio_stub
  SceCtrl_stub
  SceDisplayUser_stub
  SceDisplay_stub
  SceNotificationUtilProgress_stub
  SceGxm_stub
  SceGxmInternalForVsh_stub
  SceGxmInternal_stub
  ScePower_stub
  SceShellSvc_stub
  SceSysmodule_stub
  SceTouch_stub
  SceShellUtil_stub
  SceSystemGesture_stub
  SceIme_stub
  SceMotion_stub
  SceVshBridge_stub
  SceNotificationUtil_stub
  SceJpegArm_stub
  ScePvf_stub
  SceDbg_stub
  SceFios2_stub
  SceKernelDmacMgr_stub
  SceLibm_stub
  SceLibc_stub
  SceRtabi_stub
  SceAudiocodec_stub
  VarHelper_stub
  gcc
)

set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -d ATTRIBUTE=17338760 -s CATEGORY=gdc")
set(DOLCE_ELF_CREATE_FLAGS "${DOLCE_ELF_CREATE_FLAGS} -h 2097152")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x4000 attribute 0x10 ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

dolce_create_self(eboot.bin ${PROJECT_NAME} UNSAFE BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin)

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

dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  CONTENT_ID_LABEL "ELEVENMPVA000000"
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/pic0.png sce_sys/pic0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE ${CMAKE_CURRENT_BINARY_DIR}/exit_module.skprx module/kernel/exit_module.skprx
  FILE res/tex_common.gxt tex_common.gxt
  FILE res/tex_dirbrowse.gxt tex_dirbrowse.gxt
  FILE libs/lib/VarHelper.suprx module/user/VarHelper.suprx
)
