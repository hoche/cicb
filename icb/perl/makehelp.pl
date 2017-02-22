#!/local/bin/perl

# Copyright (c) 1990 by Carrick Sean Casey.
# For copying and distribution information, see the file COPYING.

# This perl script takes a list of help files as arguments, and outputs
# C code containing the help files encoded as definitions.

# Usage: perl makehelp.pl helpfile ... > file.c

# Line 1 should have a keyword, followed by whitespace, followed by a
#  sentence describing the topic.
# Line 2 should be blank.
# Lines 3 and on should contain the help text to be printed.

# Example:

# Porridge	Description of porridge.
#
# Porridge is an aqueous goo of strange coloration. Often a focal point
# for fairy tales, it is rumored to have had hallucinogenic properties.
# Soon after ingesting this bland substance, storybook characters never
# failed to embark upon strange surreal adventures, often accompanied by
# talking animals.
#
# See also: LSD, Nancy Reagan, "The Diary of Janice Goldilocks".
#
# eval "exec perl $*"
#	if $running_under_some_shell;

if ($#ARGV < 0) {
	print stderr "usage: makehelp helpfile ...\n";
	exit(-1); 
}

#
#	get the keys, topics, and pathnames
#

$x = 0;
foreach $path (@ARGV) {
	if (!open (fp, "$path")) {
		print stderr "makehelp: can't open help file \"$path\"\n";
		exit(-1);
	}
	$_ = <fp>;
	# scan off the key and topic and save in array
	if (/(\S*)\s*(.*)/) {
		@keys[$x] = "$1\001$2\001$path";
		$x++;
	} else {
		print stderr "can't figure out topic line in \"$path\"\n";
		exit(-1);
	}
	close(fp);
}

#
#	sort by key (sort of)
#

@keys = sort(@keys);

#
#	write the header
#

print "/*\
	THIS IS AN AUTOMATICALLY GENERATED FILE\
\
	If you edit this file, your changes may be lost later.\
*/\n\n";
print "#include <stdio.h>\n\n";

#
#	write out the keys section
#

print "char *helpkeys[] = \n{\n";
$oldkey = "";
foreach (@keys) {
	($key) = split('\001');
	if ($key eq $oldkey) {
		print stderr "makehelp: key \"$key\" is in two or more files\n";
		exit(-1);
	} else {
		print "\t\"$key\",\n";
		$oldkey = $key;
	}
}
print "\tNULL\n};\n\n";

#
#	write out the topics section
#

print "char *helptopics[] = \n{\n";
foreach (@keys) {
	($key, $topic) = split('\001');
	print "\t\"$topic\",\n";
}
print "\tNULL\n};\n\n";


#
#	write out the text sections
#
#
foreach (@keys) {
	($key, $topic, $path) = split('\001');
	print "char *ht_$key[] = \n{\n";
	open (fp, "$path") || die "can't open help file \"$path\"";
	$line = <fp>; $line = <fp>;

	# process the text
	while ($line = <fp>) {
		chop($line);
		# fix quotes
		$line =~ s/"/\\"/g;
		print "\"$line\"";
		if (!eof(fp)) {
			print ",\n";
		} else {
			print ",\nNULL\n};\n\n";
		}
	}
}


#
#	write out the text dictionary section
#

print "char **helptexts[] = \n{\n";
foreach (@keys) {
	($key,$topic) = split('\001');
	print "\tht_$key,\n";
}
print "\tNULL\n};\n\n";

