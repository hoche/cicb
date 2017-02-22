# $Id: tcl_init.tcl,v 1.1 2003/06/18 21:13:33 felixlee Exp $

# This file is converted into a C string and embedded into the cicb
# executable.  It contains tcl commands which are run when cicb starts.

# icb_command_v1 is called to dispatch any /command the user types, other
# than /! and /tcl.  you can redefine this in your .icbrc, but the
# redefinition may get ignored in future versions.

proc icb_command_v1 {cmd argstr} {
  # split the args so special chars get quoted properly.
  # XXX note, consecutive spaces will make null args.  this behavior makes
  # "/m who a   b" work right, but "/cmd -opt   arg" work wrong.  sigh.
  set arglist [split $argstr { }]

  # exact command match?
  if {[info commands $cmd] != {}} {
    return [eval {$cmd} $arglist]
  }

  # see if it's a unique abbreviation.
  set cmds [info commands ${cmd}*]
  set n [llength $cmds]
  if {$n == 1} {
    return [eval {$cmds} $arglist]
  } elseif {1 < $n} {
    error "\"$cmd\" is ambiguous: [lsort $cmds]"
  }

  # otherwise, just punt.
  return -code continue
}


# Normal user bindings

# this is added even though it is not handled by tcl
c_usage add ! g "\[command\]" "shell out or run a unix command"

c_usage add m g "\[-permanent\] \[-clear\] nick message" "send a personal message"

c_usage add tcl g "tcl command" "pass command to tcl without quoting"
c_usage add oset g "\[variable\] \[value\]" "view or set an icb variable"
c_usage add ounset g "\[variable\]" "unset an icb variable"

# allow fset for those used to it and backward compatibility in rc files
proc fset {args} {
	if { [ length $args ] == 0 } then {
		oset
	} else { if { [ length $args ] == 1 } then {
		oset $args
	} else {
		set var [ range $args 0 0 ]
		set value [ range $args 1 1 ]
		oset $var $value
	}
	}
}
proc funset {var} {ounset $var}

c_usage add beep g "nick" "send someone a beep"
proc beep nick {s_beep $nick}

c_usage add boot m "nick" "kick someone out of your group"

c_usage add bug g "" "send mail to the author"
proc bug {} {c_shell "mail cicb_workers@obfuscation.org"}

c_usage add cancel m "nick" "cancel an invitation"

c_usage add commands i "" "list commands"
proc commands {} {c_usage list}

c_usage add cp "\[old\]" "\[new\]" "change your registration password"
proc cp {args} {s_chpw $args}

c_usage add echoback g "on|off" "turn on/off echoback"
proc echoback onoff {s_user echoback $onoff}

c_usage add group g "group" "change groups"
proc group group {s_group change $group}
proc g group {s_group change $group}

c_usage add hclear g "" "clear the \"tab\" history buffer"
proc hclear {} {c_hclear}

c_usage add hdel g "" "remove a nickname from the \"tab\" history buffer"
proc hdel {nick} {c_hdel $nick}

c_usage add help i "\[subject\]" "get help \[on subject\]"
proc help {args} {c_help $args}

c_usage add hush g "\[nick\]" "toggle hushing of a user"
proc hush {args} {c_hush $args}

c_usage add invite m "nick" "invite people to your group"

c_usage add log g "\[filename\]" "toggle session logging"
proc log {args}	{eval c_log $args}

#c_usage add message g "nick message" "send a personal message"
# /m gets directed to s_personal so we can record the nick for tab history,
# among other things.
proc m {args} {eval s_personal $args}
proc message {args} {eval s_personal $args}

c_usage add motd i "" "print the ICB message of the day"
proc motd {} {s_motd}

c_usage add new m "group" "create your own group"
proc new group {s_group create $group}

c_usage add nick g "newnick" "change your nickname"
proc nick newnick {s_user nick $newnick}

c_usage add pass m "\[nick\]" "pass moderation of your group"
proc pass {args} {s_group pass $args}

c_usage add display g "\[-personal\] \[-nickname nick\] \[-(no)log\] \[-(no)screen\] \[lines\]" "display past conversation"
proc display {args} {eval c_replay $args}
proc replay {args} {eval c_replay $args}

c_usage add quit g "" "exit icb"
proc quit {} {c_quit}

c_usage add read g "" "read stored personal messages on server"
proc read args {s_personal server read}

c_usage add register g "\[password\]" "register your nickname"
proc register args {s_register $args}

c_usage add send g "message" "send text to your group"
proc send {args} {eval s_send $args}

c_usage add status m "" "change or see the status of a group"

c_usage add time i "" "show the current time"

c_usage add topic m "topicname" "set your group's topic"

c_usage add version i "\[-s|-c\]" "display server and/or client version info"
proc version {args} {
	if { [ string compare $args "-c" ] !=0 } {
		s_version
	}
	if { [ string compare $args "-s" ] != 0 } {
		c_version
	}
}

c_usage add who i "\[.\] \[group\]" "list users"
proc who {args} {s_who long $args}

# Admin only commands
# Note: these are enforced in the server

proc a_su pass {s_user nick admin $pass}
proc a_drop args {s_admin drop $args}
proc a_shutdown {when args} {s_admin shutdown $when $args}
proc a_wall args {s_admin wall $args}


# These are forced abbrevs.  sigh.  it might be a good idea to use
# namespaces to isolate the icb commands.

# 'w' is necessary because of 'while'.
proc w {args} {eval who $args}

# 'p' is actually none of the client commands.
proc p {args} {eval s_register $args}
