/*
  This used to be an automatically generated file.
  Now it's an out-of-date file.
*/

#include <stddef.h>

char *helpkeys[] = 
{
	"groups",
	"hush",
	"input",
	"intro",
	"options",
	"primitives",
	"rules",
	"tcl",
	"variables",
	NULL
};

char *helptopics[] = 
{
	"Group concepts and commands.",
	"Using hush to silence annoying users.",
	"Input and editing characters",
	"Introduction to ICB..",
	"Runtime options and environment variables",
	"Primitives for building icb commands.",
	"Rules for ICB use.",
	"General information on Tcl.",
	"Setting icb customization variables.",
	NULL
};

char *ht_groups[] = 
{
"All ICB users are divided into \"groups\". A group may be thought of",
"as a room, where what you say out loud goes to everyone else in that",
"group.  In keeping with the CB analogy, you may think of a group as a",
"channel.",
"",
"Users can jump from group to group, and create their own new groups.",
"When the last person leaves a group, it disappears.",
"",
"Group Attributes",
"",
"Groups have three administrative attributes: moderatorship,",
"visibility, and volume.  The values of these attributes appear as",
"three letters in parentheses next to the group name.",
"",
"1. Moderatorship",
"",
"	A group may be public (p), moderated (m), or restricted (r).  A public",
"	group has no moderator; no one person has any more control over",
"	the group than any other.  A moderated group has a single ",
"	moderator, whose nickname appears in the group header and",
"	is starred in the group listing.  Only the moderator may change",
"	the group attributes, set the group topic, boot people out of the",
"	group, and issue invitations into the group.  A restricted group",
"	is a moderated group which no-one may enter without an invitation.",
"",
"2. Visibility",
"",
"	A group may be visible (v), secret (s), or invisible (i).  A visible",
"	group is one whose name and occupants are visible to everyone on",
"	ICB.  A secret group is one whose occupants are visible, but whose",
"	name is not; the group name shows up as '*Secret'.  This prevents",
"	anyone who does not know the group name from entering the group.",
"	An invisible group does not show up in the /w listing at all unless",
"	you are in that group.  However, it is counted in the summary",
"	statistics at the bottom of the /w listing.",
"	",
"3. Volume",
"",
"	A group may be quiet (q), normal (n), or loud (l).  In a quiet group,",
" 	no open messages are permitted.  If you wish to receive only personal",
"	messages, a quiet group is a good place to hang out.  In a normal",
"	group, open messages are permitted.  Loud is the same as normal,",
"	with one exception: if the group is restricted and someone without an",
"	invitation tries to enter the group, the moderator will receive a",
"	message indicating that fact.",
"",
"Creating your own group automatically makes you the moderator of that group.",
"Additionally, the moderator of any group may at any time decide to turn over",
"moderatorship of that group to you.  ",
"",
"Numeric groups are initially public, visible, and loud (pvl).  Named groups",
"are initially moderated, visible, and loud (mvl), with two exceptions.  If",
"the group name begins with a single period, the group is initially secret.",
"If the group names begins with two or more periods, the group is initially",
"invisible.",
"",
"Commands",
"",
"The following are the commands for manipulating groups.  Commands marked with",
"(m) may only be issued by the moderator if the group is moderated; otherwise",
"anyone may issue them.  Commands marked with (M) may only be issued by the",
"moderator; if there is no moderator, no-one may issue them.",
"",
"	group <groupname> 	change to a new group, creating it if it",
"				does not exist.",
"",
"(m)	topic [<string>] 	set or display the current topic string",
"",
"	status			display group attributes, topic, and invitation",
"				list",
"(m)	status [pmrqnlvsi]	set group attributes",
"(M)	status name <newname>	change the group's name",
"",
"	topic 			display the topic string",
"(m)	topic <string>		set the topic string",
"",
"(m)	invite [<nickname>]	invite someone to the group",
"",
"(M)	boot <nickname>		kick someone out of the group",
"",
"(M)	pass [<nickname>]	turn moderation over to someone else, or",
"				give up moderation completely",
NULL
};

char *ht_hush[] = 
{
"ICB has a large base of diverse users. It is inevitable that there",
"are going to be people who do not get along. Rather than censoring users,",
"I prefer to give people tools that allow them to reduce or negate the",
"problem.",
"",
"To deal with annoying users, the first line of defense is the facility",
"called \"hushing\". Use of the hush command enables you to block all",
"communications from a particular person to you. You won't receive any",
"group messgages, personal messages, beeps, or invitations from them.",
"",
"Hush acts a toggle, so if you decide that person doesn't bug you anymore,",
"you can reenable them by hushing them again. If you give the hush command",
"without a nickname, a list of currently hushed people will be printed.",
"",
"Hush operates on a nickname; a hushee may get around your hushlist by",
"changing his or her nickname.  You may prevent this by using the s_hush",
"(server hush) command; this command lets you define a set of users",
"to ignore based on their sites.  ",
"",
"Hush is the preferred way of dealing with problem users. Other means",
"include using the moderated group facility, and when all else fails,",
"complaining to the administration. If a user is persistent enough that",
"hush won't take care of them, see the rules section for what to do.",
NULL
};

char *ht_input[] = 
{
"EDITING CHARACTERS",
"",
"ICB uses the GNU readline library for input lines.  This library lets you",
"edit commands as you type them and retrieve previous commands that you have",
"typed.  Readline will emulate either of the vi or emacs editors; you may control",
"which by using the 'oset' command to set the 'editmode' variable to either",
"\"vi\" or \"emacs\".  The original default is picked up by the setting of your",
"EDITOR environment variable; if that variable is not set, vi is used.",
"",
"",
"NICKNAME HISTORY",
"",
"ICB remembers to whom you send messages, and makes those names",
"available by hitting the TAB key at the beginning of a line. This makes",
"sending personals very easy. Try it! If you strike the TAB key more",
"than once at the beginning of a line, it cycles through the list of",
"personals, up to the number specified by the \"history\" variable.",
"",
"If you have already typed a command and part of a nickname when you hit",
"TAB, it will attempt to complete the nickname from the list in your ",
"history.  If you have already typed a command, a nickname, and more text on the",
"line, then pressing TAB will cause the nickname to cycle through your history",
"without affecting the rest of your command line.",
"",
"You may selectively delete nicknames from your history with the \"hdel\"",
"command, or you may use the \"hclear\" to completely clear out this history list",
"and start over.",
"",
"If you set the value of the \"autodel\" variable to \"true\", then any nickname",
"that generates a \"not signed on\" error message will be automatically removed",
"from your history .",
"",
"",
"THE COMMAND CHARACTER",
"",
"All lines starting with the command character are assumed to be commands.",
"If you wish to send a line of text starting with the command character,",
"type it double at the beginning of the line.   The default command character",
"is '/'; you may change it by changing the value of the 'commandchar' variable.",
"",
"",
"TCL QUOTING",
"",
"Tcl has several characters with special meaning. For example, dollar",
"sign \"$\" is assumed to start a variable. So that users don't get",
"confused when things don't work as expected, all command input is",
"specially quoted to make Tcl ignore these special characters. If you",
"wish the line to be unquoted, use the \"tcl\" command, giving the line",
"you want unquoted as the argument. Tcl will operate on the rest of the",
"line at full functionality.",
NULL
};

char *ht_intro[] = 
{
"Welcome to ICB!",
"",
"ICB is an ongoing project to connect people together. It provides",
"a forum where people can meet and share information, ideas, or opinions;",
"conduct meetings; play games; or do just about anything made",
"possible with fast electronic communications.",
"",
"Getting started using ICB is easy. You run \"icb\" and after a few",
"seconds, you will be signed on in one of the default groups. A group is",
"a place where people can talk with one another. Chances are, there will",
"be other people there, and you'll see them talking. It might look",
"something like this:",
"",
"<Joe> hello x",
"<Amelia> hi x",
"",
"Usually when you first \"sign on\", people will say hello to you. Here,",
"Joe and Amelia said hi, assuming your nickname was \"x\".",
"",
"Everyone in ICB has a \"nickname\". These are used because many",
"login names are difficult to type, and don't tell you much about the",
"user. Some users have nicknames like \"Dr. X\". Others use their real",
"life names.",
"",
"Try a \"who\" command, to see who is signed on. You can do this",
"with the command:",
"",
"/who            (show everyone)",
"/who .          (show just your group)",
"",
"Note that the command started with a slash. All icb commands start with a",
"slash (although you can change that).",
"",
"To send a line of text to the group, just type what you want to say, and",
"hit RETURN. It will be sent to everyone in your group.",
"",
"Sooner or later, you're going to see a message that looks like this:",
"",
"<*Joe*> some text",
"",
"This means that Joe sent you a \"personal message\". It is meant only for",
"you. He sent it with a command that looks like this:",
"",
"/m x some text",
"",
"Still assuming your name was \"x\". You could reply with:",
"",
"/m joe Joe, you are being silly.",
"",
"Which would only go to Joe.",
"",
"I hope this is enough to get you started talking. In other help files,",
"you can learn more about groups, how to change groups, how to make your",
"own, etc. I suggest you read--or at least skim--all of the help files.",
"That way, you'll be able to get the most out of icb.",
"",
"As a final note, please read the rules. Not only do they say what is",
"tolerated and what is not (ICB is pretty liberal), but they also tell",
"you what to do if you run into trouble.",
"",
"Have fun, and feel free to send me suggestions and bug reports.",
"",
"Mark J. Reed",
"<mark@cad.gatech.edu>",
NULL
};

char *ht_options[] = 
{
"Runtime options for icb (these may be abbreviated):",
"",
"  -nickname nick	sign on with nickname nick.",
"",
"  -password pass	set password to pass (implies -clear).",
"",
"  -password -		prompt for password.",
"",
"  -group group		sign on into group group.",
"",
"  -server name		connect to server named name.",
"",
"  -host host		connect to server on host host.",
"",
"  -port port		try to connect to port port.",
"",
"  -list			list known servers, in order.",
"",
"  -clear		wipe args from command line.",
"",
"  -who			see who's on; don't sign on.",
"",
"  -restricted		restricted mode",
"",
"  -help			display help message.",
"",
"",
"ICB environment variables:",
"",
"    ICBNAME     Default nickname to use, instead of your login id.",
"",
"    SHELL       Shell to use for shellouts. If unspecified then /bin/sh",
"                is assumed.",
NULL
};

char *ht_primitives[] = 
{
"ICB provides a set of primitives for building your own commands. Most",
"of the \"normal\" commands are actually Tcl procedures that invoke these",
"primitives, which are C functions.",
"",
"The primitives are divided into two categories. The first one is client",
"functions performed internally. Most of these start with \"c_\", except for",
"\"fset\" and \"tcl\". The second category is functions that request some",
"action of the server, these start with \"s_\".",
"",
"fset    [variable [value]]",
"",
"    This sets and displays icb customization variables. Without a variable",
"    name, it displays all the variables on the users screen. With just",
"    a variable name, it prints out only that variable name. With a variable",
"    name and a value, the variable is set to that value. See the help",
"    section on variables.",
"",
"tcl     [tcl command]",
"",
"    When inputting text from the terminal, this flags the beginning of a",
"    tcl command that is not to be specially quoted. To avoid nasty surprises,",
"    text handed to tcl is normally quoted to remove the meaning of tcl",
"    special characters. This allows input without that quoting, thus",
"    giving tcl full power to do it's thing.",
"",
"c_hclear",
"",
"    Clears out the \"tab\" history buffer.",
"",
"c_help  [subject]",
"",
"    Display the help menu. If subject is given, display the help file for that",
"    subject.",
"",
"c_quit",
"",
"    Quit icb.",
"",
"c_rand range",
"",
"    Returns a random number from 1 to range.",
"",
"c_replay [lines]",
"",
"    Print lines from the review buffer. If lines is not given, the entire",
"    buffer is printed.",
"",
"c_shell [command]",
"",
"    Start up a shell or run a command under a subshell. If the SHELL environment",
"    variable is set, that shell is used.",
"",
"c_color [color] [fg] [bg]",
"",
"    set a color to a fg and bg color.",
"c_time",
"",
"    Print the current time.  Affected by the \"timedisplay\" variable.",
"",
"c_usage add|delete|list [name] [type args usage]\";",
"",
"    Manipulate usage lines. Sub functions are:",
"",
"        add name type args usage...",
"            Add a usage line. Name is the command name. Type is \"g\" for",
"            general commands, \"l\" for listing commands, \"m\" for moderator",
"            commands, \"a\" is for admin commands, and \"c\" is for user",
"            defined custom commands.",
"",
"        delete name",
"            Delete a usage line (not currently implemented).",
"",
"        list",
"            List all the usage lines. The \"commands\" command calls this.",
"",
"s_beep  nick",
"",
"    Send a beep to another user.",
"",
"s_group change|create|invite|pass|remove|status|topic args",
"",
"    Request a group function from the server. The sub functions are:",
"",
"        change group            Change groups.",
"        create newgroup         Create a new group.",
"        invite nick             Invite a user.",
"        pass   [nick]           Relinquish or pass moderatorship.",
"        remove nick             Remove a user from a group.",
"        status flags            Set or see the status for a group.",
"        topic  text...          Set the topic for this group.",
"",
"s_person nick text...",
"",
"    Send a personal message to a user.",
"",
"s_send  text...",
"",
"    Send a line of text to the group.",
"",
"s_user",
"",
"    Invoke a user function. The sub functions are:",
"",
"        nick    newnick q       Change your nickname.",
"",
"s_who   short|long [group]",
"",
"    Print a list of users in either short or long format. If group is",
"    given, only the users in that group are listed. Short and long are",
"    not yet implemented. Use \"long\".",
NULL
};

char *ht_rules[] = 
{
"In general, ICB is a \"free speech\" system. Users are encouraged to",
"air their viewpoints and to comment freely on the viewpoints of others.",
"",
"Nonetheless, there are rules designed to \"keep the peace\" and in",
"general protect people from abuse.",
"",
"ICB usage may be reduced, suspended, or revoked for:",
"",
"        Using lewd or offensive words in nicknames, visible group",
"        names, or topics.",
"",
"        Harassing another user. Harassment is defined as behavior",
"        towards another with the intent purpose of annoying them.",
"",
"        Sending files or \"pictures\" through ICB to users that",
"        don't want to see them.",
"",
"        Falsifying your identity to the server. Why is this bad?",
"        Because it is my only way of contacting people so I can",
"        resolve various problems.",
"",
"        Any behavior whose purpose is to reduce the functionality of",
"        ICB, violate the security thereof, or see or allow others",
"        to see any ICB messages that they would not normally be",
"        permitted to see. Please don't mess it up for others.",
"",
"What is \"lewd\" or \"offensive\" is up to the ICB server administrator. If",
"you have doubts, don't use it. Harassment is also largely a matter of",
"opinion.",
"",
"Violations are usually straighted out via friendly email. If that fails",
"to work, a lot of things can happen. In some cases in the past, remote",
"administrations have been contacted, and even whole sites have been",
"locked out. ",
"",
"If you find someone violating the rules, please contact the ICB",
"server administrator. The address is available with the \"version\" command.",
NULL
};

char *ht_tcl[] = 
{
"Tcl, pronounced \"tickle\", stands for \"Tool Command Language\". It is",
"basically a shell that you extend with your own commands to make an",
"application. Tcl has been incorporated into icb, or icb has been",
"incorporated into Tcl, depending on how you look at it.",
"",
"The net result is that now users have a very flexible command parser.",
"They can rename or delete commands, define their own commands and",
"abbreviations, and write Tcl programs to do things.",
"",
"Tcl is too complicated to cover here. The user is referred to the Tcl",
"reference document, included in the icb distribution in the \"docs\"",
"directory. The rest of the Tcl documentation, which deals with how",
"to interface Tcl with C programs, is in the official Tcl release,",
"available via anonymous ftp from ucbvax.berkeley.edu.",
"",
"See the help topic for info on the fnrc file, which has some sample",
"uses of Tcl.",
NULL
};

char *ht_variables[] = 
{
"ICB has several variables you can set to customize the way icb works. This",
"is done using the \"oset\" command. There are three ways to use the oset",
"command:",
"",
"oset            Display the value of all variables.",
"oset var        Display the value of a particular variable.",
"oset var value  Set the value of a variable.",
"ounset var	Unset a variable.",
"",
"The variables and their effect are listed below. Boolean (bool) fields",
"are either \"true\" or \"false\". Setting them to \"true\", \"on\", \"yes\" \"y\",",
"or \"1\", sets them to true. Setting them to \"false\", \"off\", \"no\" \"n\",",
"or \"0\", sets them to false. \"Nneg\" fields are nonnegative integers.",
"A \"char\" field will use the first character only of an argument.",
"",
"ICB VARIABLES",
"",
"autodel		(boolean)		default: false",
"",
"    If true, whenever you receive an error from the server indicating that",
"    a nickname is not signed on, that nickname will be deleted from your",
"    \"tab\" history.",
"",
"autoreg		(boolean)		default: false",
"",
"   If true, whenever the server sends a message indicating that you must",
"   send your password to register your nickname, the client will prompt you",
"   for your password and send it to the server.",
"",
"asyncread		(boolean)		default: false",
"",
"    When false, messages will be paused while you type."
"    When true, you will receive messages even while typing."
"",
"beeps           (boolean)               default: true",
"",
"    If true, a received beep will make your terminal beep (with a ^G).",
"    If false, you will still get the beep message, but no actual beep.",
"",
"bufferlines     (nneg)                  default: 250",
"",
"    ICB keeps a buffer of the messages that have gone by. You can review",
"    this buffer with the replay command.",
"",
"colorize        (boolean)               default: false",
"",
"    If true, then use ANSI color sequences to colorize output.",
"    Colors can be changed with /c_color.",
"",
"commandchar     (char)                  default: '/'",
"",
"    Commandchar is the character used at the beginning of a line to",
"    prefix an icb command. Note that the backslash character always",
"    works and can't be changed, so if you set it and forget it, use",
"    backslash. (Joe Isuzu alert: this is a lie, backslash doesn't work).",
"",
"cute		(boolean)		default: true",
"",
"    If true, then ICB will generate a random cute message every time you",
"    hit RETURN on a blank line.",
"",
"editmode	(string)		default: ${EDITOR:-vi}",
"",
"    This variable may be set to either \"vi\", for vi-style input editing,",
"    or \"emacs\", for emacs-style input editing.  The default is determined",
"    from the environment variable EDITOR.  If EDITOR is not set, the",
"    default is vi.",
"",
"groupblanks     (boolean)               default: false",
"",
"    In a who command, this causes a blank line to be printed before",
"    each group.",
"",
"history         (nneg)                  default: 15",
"",
"    This controls the number of nicknames recallable with the history",
"    key, usually TAB.",
"",
"logfile         (string)                default: icb.log",
"",
"    The default filename used for session logs.",
"",
"logreplay	(boolean)		default: false",
"",
"   If true, then the output of the /replay or /display command will go",
"   into the logfile, if logging is active.  The value of this variable may",
"   be overridden for a single replay command by specifying -log or -nolog.",
"",
"pagesize        (integer)               default: -1)",
"",
"    If this is nonzero then icb will pause and print a [=More=] prompt",
"    every pagesize lines.  Any value less than zero means icb will",
"    query your terminal for its current height.",
"",
"pagewidth       (integer)               default: -1",
"",
"   If this is nonzero, then icb will split long output lines.",
"   Any value less than zero means icb will query your terminal for",
"   its current width.",
"",
"personalto	(string)		default: \"\" ",
"",
"    If this is a nonempty string, then any text typed that is not a ",
"    command will be sent as a personal message to the nickname contained",
"    in this variable.  /ounset will set this variable to the empty string,",
"    so that non-command text will go out as a public message to your",
"    group.",
" ",
"    If you specify the -permanent flag on the /message command, this",
"    variable will be set to the target of that /m.  The -clear flag of",
"    /message will unset this variable.",
"",
"pauseonshell    (boolean)               default: false",
"",
"    When pauseonshell is true, icb will ask you to hit return when a",
"    shell command is finished. Since a large amount of icb output may be",
"    waiting for your screen, this gives you a chance to see the output",
"    of your command.",
"",
"restricted      (boolean)               default: false",
"",
"    Restricted shuts off the ability to run subshell commands or to",
"    read or write files. It makes it icb reasonably secure for use as",
"    a shell.",
"",
"tabreply	(boolean)		default: false",
"",
"    If false, only people you /m will be added to the <tab> history."
"    If true, people who sent you personal messages will be added too.",
"",
"timedisplay     (string)                default: \"normal\"",
"",
"    This controls how time is printed by /c_time and /time.",
"        \"cute\"      means use English.",
"        \"normal\"    means use a 12-hour clock (%I:%M%p).",
"        \"military\"  means use a 24-hour clock (%H:%M).",
"    Any other value is used directly as a format string for strftime()",
"",
"verifyquit      (boolean)               default: false",
"",
"    If this is true, you are asked to verify anytime you quit icb.",
NULL
};

char **helptexts[] = 
{
	ht_groups,
	ht_hush,
	ht_input,
	ht_intro,
	ht_options,
	ht_primitives,
	ht_rules,
	ht_tcl,
	ht_variables,
	NULL
};

