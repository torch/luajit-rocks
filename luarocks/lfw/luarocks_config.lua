local LFW_ROOT = config.LFW_ROOT
rocks_servers = {
   [[https://raw.githubusercontent.com/torch/rocks/master]],
   [[https://raw.githubusercontent.com/rocks-moonscript-org/moonrocks-mirror/master]]
}
rocks_trees = {
   { root = LFW_ROOT, rocks_dir = LFW_ROOT..[[\rocks]],
     bin_dir = LFW_ROOT, lua_dir = LFW_ROOT..[[\lua]],
     lib_dir = LFW_ROOT..[[\clibs]] }
}
variables.WRAPPER = LFW_ROOT..[[\rclauncher.c]]
