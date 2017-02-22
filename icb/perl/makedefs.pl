# Copyright (c) 1990 by Carrick Sean Casey.
# For copying and distribution information, see the file COPYING.

# This perl script takes a file of procedure definitions as arguments,
# and outputs C code containing the procs files encoded as strings.

# Usage: perl defmake.pl < def_procs > defprocs.c

print "/*\
	THIS IS AN AUTOMATICALLY GENERATED FILE\
\
	If you edit this file, your changes may be lost later.\
*/\n\n";

print "char *defprocs[] = \n{\n";
while (<>) {
	chop;
	s/\s\s*/ /g;			# compress whitespace
	if (/^#/ || /^$/ || /^\s*$/) {	# ignore comments or blanks
		next;
	}
	s/\\/\\\\/g;			# backslash backslashes
	s/"/\\"/g;			# backslash quotes
	if (/^proc .* {$/) {
		print "\t\"$_";
		$loopcnt = 0;
		while(<>) {
			chop;
			if (/^#/ || /^$/ || /^\s*$/) {
				last;
			}
			s/\\/\\\\/g;
			s/"/\\"/g;
			if ($loopcnt++ > 0) {
				print "\\n\\\n\t$_";
			} else {
				print "\\\n\t$_";
			}
		}
		print "\",\n";
	} else {
		print "\t\"$_\",\n";
	}
}
print "\t(char *) 0\n};\n\n";
