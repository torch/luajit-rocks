PROJECT(luajit-rocks)

# We want release compilation by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

ADD_SUBDIRECTORY(luajit)
ADD_SUBDIRECTORY(luarocks)
