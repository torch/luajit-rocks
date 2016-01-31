# -*- cmake -*-

PROJECT(Lua)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

SET(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_MODULE_PATH}")

SET(INSTALL_INCLUDE_SUBDIR "include" CACHE STRING "installation include subdirectory name")
IF(WIN32)
  SET(INSTALL_BIN_SUBDIR "." CACHE STRING "installation executable subdirectory name")
  SET(INSTALL_LIB_SUBDIR "." CACHE STRING "installation library subdirectory name")
  SET(INSTALL_LUA_PATH_SUBDIR "lua") # not editable
  SET(INSTALL_LUA_CPATH_SUBDIR ".") # not editable
ELSE()
  SET(INSTALL_BIN_SUBDIR "bin" CACHE STRING "installation executable subdirectory name")
  SET(INSTALL_LIB_SUBDIR "lib" CACHE STRING "installation library subdirectory name")
  SET(INSTALL_LUA_PATH_SUBDIR "share/lua/5.1/" CACHE STRING "lua path subdirectory name")
  SET(INSTALL_LUA_LIB_SUBDIR "lib" CACHE STRING "installation lua lib subdirectory name")
  SET(INSTALL_LUA_CPATH_SUBDIR "${INSTALL_LUA_LIB_SUBDIR}/lua/5.1/" CACHE STRING "lua cpath subdirectory name")
ENDIF()

IF(UNIX)
  SET(LUA_ROOT "${CMAKE_INSTALL_PREFIX}")
ENDIF()

# Readline support
FIND_PACKAGE(Readline)
IF(READLINE_FOUND)
  SET(LUA_USE_READLINE 1)
  LIST(APPEND LIBS ${READLINE_LIBRARIES})
  INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
ENDIF(READLINE_FOUND)

INCLUDE(CheckLibraryExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)

CHECK_FUNCTION_EXISTS(_longjmp LUA_USE_ULONGJMP)
CHECK_SYMBOL_EXISTS(isatty unistd.h LUA_USE_ISATTY)
CHECK_SYMBOL_EXISTS(mkstemp stdlib.h LUA_USE_MKSTEMP)
CHECK_SYMBOL_EXISTS(popen stdio.h LUA_USE_POPEN)
CHECK_LIBRARY_EXISTS(m sin "" LUA_USE_LIBM)
IF(LUA_USE_LIBM)
  LIST(APPEND LIBS "m")
ENDIF()

IF(NOT WIN32)
  FIND_LIBRARY(DL_LIBRARY "dl")
  IF(DL_LIBRARY)
    SET(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
    LIST(APPEND LIBS ${DL_LIBRARY})
  ENDIF(DL_LIBRARY)
  CHECK_FUNCTION_EXISTS(dlopen LUA_USE_DLOPEN)
  IF(NOT LUA_USE_DLOPEN)
    MESSAGE(FATAL_ERROR "Cannot compile a useful lua.
Function dlopen() seems not to be supported on your platform.
Apparently you are not on a Windows platform as well.
So lua has no way to deal with shared libraries!")
  ENDIF(NOT LUA_USE_DLOPEN)
ELSE()
  SET(LUA_BUILD_AS_DLL 1)
ENDIF()

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
FIND_PACKAGE(Threads)
IF(THREADS_FOUND)
  LIST(APPEND LIBS ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR} src)
CONFIGURE_FILE(src/luaconf.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h src/lua.h src/lauxlib.h src/lualib.h
  DESTINATION "${INSTALL_INCLUDE_SUBDIR}")

SET(SRC_LIB
  src/lapi.c src/lcode.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c src/lgc.c src/llex.c src/lmem.c
  src/lobject.c src/lopcodes.c src/lparser.c src/lstate.c src/lstring.c src/ltable.c src/ltm.c
  src/lundump.c src/lvm.c src/lzio.c
  src/lauxlib.c src/lbaselib.c src/ldblib.c src/liolib.c
  src/lmathlib.c src/loslib.c src/ltablib.c src/lstrlib.c src/loadlib.c src/linit.c  
  ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h)

# Shared library and executables
ADD_LIBRARY(liblua SHARED ${SRC_LIB})
SET_TARGET_PROPERTIES(liblua PROPERTIES 
  PREFIX "lib" IMPORT_PREFIX "lib" OUTPUT_NAME "lua")
ADD_EXECUTABLE(lua src/lua.c ${SRC_LIB})
ADD_EXECUTABLE(luac src/luac.c src/print.c ${SRC_LIB})
TARGET_LINK_LIBRARIES(liblua ${LIBS})
TARGET_LINK_LIBRARIES(lua ${LIBS})
TARGET_LINK_LIBRARIES(luac ${LIBS})

# Install files
INSTALL(TARGETS lua luac liblua
  RUNTIME DESTINATION "${INSTALL_BIN_SUBDIR}"
  LIBRARY DESTINATION "${INSTALL_LIB_SUBDIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_SUBDIR}")

INSTALL(FILES src/lua.h ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h src/lualib.h src/lauxlib.h
  DESTINATION "${INSTALL_INCLUDE_SUBDIR}/lua")
