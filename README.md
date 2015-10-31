CMake-based LuaJIT + Luarocks
=============================

# What's the point? #

We want to provide an easy to way to users for installing _recent_ versions
of LuaJIT (or Lua) and luarocks, with almost no efforts.

The provided LuaJIT (or Lua) and luarocks point to their respective git
repository. We did not make any changes, except the compilation and
installation processes.

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

Get a C compiler. For Windows, we recommend the
[Windows SDK](http://msdn.microsoft.com/en-us/windowsserver/bb980924.aspx). It
is free, it has no GUI, but it is just fine with CMake.

# Installation

```sh
git clone https://github.com/torch/luajit-rocks.git
cd luajit-rocks
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/your/prefix
```

Then under Unix systems:
```sh
make install
```

Under Windows:
```sh
nmake install
```

Note: we do not recommend (nor we support) installation under Cygwin.

## Additional CMake flags

  - If you prefer vanilla Lua 5.1 instead of LuaJIT, use `-DWITH_LUA51=ON`
  - If you prefer vanilla Lua 5.1 with reference counting instead of LuaJIT, use `-DWITH_LUA51RC=ON` (*experimental*)
  - If you prefer vanilla Lua 5.2 instead of LuaJIT, use `-DWITH_LUA52=ON`
  - If you prefer LuaJIT 2.1 instead of LuaJIT 2.0, use `-DWITH_LUAJIT21=ON`
