PROJECT(luajit-rocks)

# We want release compilation by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

IF (NOT INSTALL_INCLUDE_SUBDIR)
 SET(INSTALL_INCLUDE_SUBDIR "include/luajit-2.0"
   CACHE STRING "installation include subdirectory name")
ENDIF(NOT INSTALL_INCLUDE_SUBDIR)

SET(LUA_EXE_NAME "luajit")

SET(LUA_VERSION 5.1)
ADD_SUBDIRECTORY(luarocks)
