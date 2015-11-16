LuaJIT-based Luarocks
=============================

# What's the point? #

We want to provide an easy to way to users for installing recent luarocks, with absolutely no efforts.

The provided luarocks points to their respective git repository. 
We did not make any changes, except the compilation and installation processes.

In addition,
  - Luarocks (or Lua) will be installed at the same location as LuaJIT and will know
    about LuaJIT shared library location (mandatory for Windows installs). It will
    also not be confused if you have several LuaJIT+luarocks at different locations.

  - Luarocks will come installed with [Torch rocks repository](http://htmlpreview.github.io/?https://github.com/torch/rocks/blob/master/index.html)
  
  - Luarocks comes with mandatory system command line tools under Windows.

  - Readline support for LuaJIT.
  
  - Experimental: Lua 5.1 with [reference counting](https://github.com/jjensen/luaplus51-all/).

# Pre-requisites

Install [CMake](http://cmake.org) on your system.

For Debian-based systens:
Using apt-get or dpkg, install .deb file from export/ directory.

Otherwise: 

!!! Switch to the 'master' branch of this repo and follow its README.md !!!
