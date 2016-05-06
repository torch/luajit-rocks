project(luarocks)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

SET(INSTALL_INCLUDE_SUBDIR "include" CACHE STRING "installation include subdirectory name")
IF(WIN32)
  SET(INSTALL_BIN_SUBDIR "." CACHE STRING "installation executable subdirectory name")
  SET(INSTALL_LIB_SUBDIR "." CACHE STRING "installation library subdirectory name")
  SET(INSTALL_LUA_PATH_SUBDIR "lua") # not editable
  SET(INSTALL_LUA_CPATH_SUBDIR ".") # not editable
  SET(INSTALL_LUAROCKS_ROCKS_SUBDIR "luarocks" CACHE STRING "installation luarocks rocks subdirectory name")
  SET(INSTALL_LUAROCKS_SYSCONF_SUBDIR "luarocks" CACHE STRING "installation luarocks sysconfig subdirectory name")
ELSE()
  SET(INSTALL_BIN_SUBDIR "bin" CACHE STRING "installation executable subdirectory name")
  SET(INSTALL_LIB_SUBDIR "lib" CACHE STRING "installation library subdirectory name")
  SET(INSTALL_LUA_PATH_SUBDIR "share/lua/${LUA_VERSION}") # not editable
  SET(INSTALL_LUA_LIB_SUBDIR "lib" CACHE STRING "installation lua lib subdirectory name")
  SET(INSTALL_LUA_CPATH_SUBDIR "${INSTALL_LUA_LIB_SUBDIR}/lua/${LUA_VERSION}") # not editable
  SET(INSTALL_LUAROCKS_ROCKS_SUBDIR "lib/luarocks/rocks" CACHE STRING "installation luarocks rocks subdirectory name")
  SET(INSTALL_LUAROCKS_SYSCONF_SUBDIR "etc/luarocks" CACHE STRING "installation luarocks sysconfig subdirectory name")
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
                      "${CMAKE_MODULE_PATH}")

IF(WIN32)

  SET(WINUTILS_DLL_FILES
    win32/tools/7z.dll win32/tools/libeay32.dll win32/tools/libiconv2.dll
    win32/tools/libintl3.dll win32/tools/libssl32.dll)

  INSTALL(FILES ${WINUTILS_DLL_FILES} DESTINATION "${INSTALL_BIN_SUBDIR}/tools/")

  SET(WINUTILS_EXE_FILES
    win32/tools/7z.exe
#    win32/tools/chmod.exe
    win32/tools/cp.exe
    win32/tools/find.exe
    win32/tools/ls.exe
    win32/tools/md5sum.exe
    win32/tools/mkdir.exe
    win32/tools/mv.exe
#    win32/tools/objdump.exe
    win32/tools/pwd.exe
#    win32/tools/rm.exe
    win32/tools/rmdir.exe
    win32/tools/test.exe
    win32/tools/uname.exe
    win32/tools/wget.exe)

  INSTALL(PROGRAMS ${WINUTILS_EXE_FILES} DESTINATION "${INSTALL_BIN_SUBDIR}/tools/")

  SET(WGET_EXECUTABLE_NAME "wget")
  SET(MD5_EXECUTABLE_NAME "md5sum")
  SET(UNAME_M "x64")
  SET(SHEBANG
"::rem:: --[[
@setlocal&  set PATH=${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_SUBDIR};%PATH% & set luafile=\"%~f0\" & if exist \"%~f0.bat\" set luafile=\"%~f0.bat\"
@${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_SUBDIR}/${LUA_EXE_NAME}.exe %luafile% %*&  exit /b ]]")

ELSE()
  FIND_PACKAGE(Wget)
  FIND_PACKAGE(MD5)

  IF(NOT WGET_FOUND)
    FIND_PROGRAM(CURL_EXECUTABLE curl)
    IF(CURL_EXECUTABLE)
      SET(WGET_EXECUTABLE ${CURL_EXECUTABLE})
      MESSAGE("-- curl found instead of wget :" ${WGET_EXECUTABLE})
    ELSE()
      MESSAGE(FATAL_ERROR "Downloader wget or curl not found")
    ENDIF()
  ENDIF()

  IF(NOT MD5_FOUND)
    MESSAGE(FATAL_ERROR "MD5 checker not found")
  ENDIF()

  GET_FILENAME_COMPONENT(WGET_EXECUTABLE_NAME "${WGET_EXECUTABLE}" NAME)
  GET_FILENAME_COMPONENT(MD5_EXECUTABLE_NAME "${MD5_EXECUTABLE}" NAME)

  IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE UNAME_M RETURN_VALUE val)
    IF("${val}" GREATER 0)
      SET(UNAME_M ${CMAKE_SYSTEM_PROCESSOR})
    ENDIF()
  ELSE()
    SET(UNAME_M ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF()

  SET(SHEBANG "#!${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_SUBDIR}/${LUA_EXE_NAME}")

ENDIF()

FILE(GLOB_RECURSE LUAROCKS_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src/luarocks" "${CMAKE_CURRENT_SOURCE_DIR}/src/luarocks/*.lua")

FOREACH(LUAROCKS_FILE ${LUAROCKS_FILES})
  GET_FILENAME_COMPONENT(_subpath "${LUAROCKS_FILE}" PATH)
  INSTALL(FILES "src/luarocks/${LUAROCKS_FILE}" DESTINATION "${INSTALL_LUA_PATH_SUBDIR}/luarocks/${_subpath}")
ENDFOREACH()

CONFIGURE_FILE("src/luarocks/site_config.lua.in" "${CMAKE_CURRENT_BINARY_DIR}/src/luarocks/site_config.lua")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/luarocks/site_config.lua" DESTINATION "${INSTALL_LUA_PATH_SUBDIR}/luarocks")

IF(WIN32)
  CONFIGURE_FILE("src/luarocks/config.lua.win.in" "${CMAKE_CURRENT_BINARY_DIR}/src/luarocks/config.lua")
ELSE()
  CONFIGURE_FILE("src/luarocks/config.lua.in" "${CMAKE_CURRENT_BINARY_DIR}/src/luarocks/config.lua")
ENDIF()
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/luarocks/config.lua" DESTINATION "${INSTALL_LUAROCKS_SYSCONF_SUBDIR}")

IF(WIN32)
  CONFIGURE_FILE("src/bin/luarocks.in" "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks.bat")
  CONFIGURE_FILE("src/bin/luarocks-admin.in" "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks-admin.bat")
  INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks.bat" DESTINATION "${INSTALL_BIN_SUBDIR}")
  INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks-admin.bat" DESTINATION "${INSTALL_BIN_SUBDIR}")
ELSE()
  CONFIGURE_FILE("src/bin/luarocks.in" "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks")
  CONFIGURE_FILE("src/bin/luarocks-admin.in" "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks-admin")
  INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks" DESTINATION "${INSTALL_BIN_SUBDIR}")
  INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/src/bin/luarocks-admin" DESTINATION "${INSTALL_BIN_SUBDIR}")
ENDIF()
