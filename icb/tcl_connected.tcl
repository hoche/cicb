# This file is converted into a C string and embedded into the
# cicb executable.

# These are tcl commands that get run after connecting to a
# server.

set it "$icb_sys_datadir/system.icbrc"
if {[file exists $it]} {
  puts "Reading $it..."
  source $it
}

set it "~/.icb/icbrc"
if {[file exists $it]} {
  puts "Reading $it..."
  source $it
}

