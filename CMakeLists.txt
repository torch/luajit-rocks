PROJECT(luajit-rocks)

# We want release compilation by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

OPTION(WITH_LUA51 "Use Lua 5.1 instead of LuaJIT" OFF)

IF(WITH_LUA51)
  SET(LUA_EXE_NAME lua)
  ADD_SUBDIRECTORY(lua-5.1)
ELSE()
  SET(LUA_EXE_NAME luajit)
  ADD_SUBDIRECTORY(luajit)
ENDIF()

ADD_SUBDIRECTORY(luarocks)
