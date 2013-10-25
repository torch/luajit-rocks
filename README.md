CMake-based LuaJIT + Luarocks
=============================

# What the point? #

We want to provide an easy to way to users for installing _recent_ versions
of LuaJIT and luarocks, with almost no efforts.

The provided LuaJIT and luarocks point on their respective git
repository. We did not make any changes, except the compilation and
installation process.

In addition,
  - LuaJIT is compiled with RPATH support, such that there is no need to
    modify the system library path if it is installed in a non-standard location.
    (That is quite useful if you deal with several trees, e.g for debug reasons)
    
  - Luarocks will be installed at the same location than LuaJIT and will know
    about LuaJIT shared library location (mandatory for Windows installs). It will
    also not be confused if you have several LuaJIT+luarocks at different locations.
    
  - Luarocks will not use installed modules (such as luasocket, etc...), which
    is a workaround on a blatant annoying bug.
    
  - Luarocks will come installed with [Torch rocks repository](http://torch.github.io/rocks.html).
  
  - Luarocks comes with mandatory system command line tools under Windows.
  
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
