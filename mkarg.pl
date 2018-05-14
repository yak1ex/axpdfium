#!/usr/bin/perl
# Extract dependent libraries and objects

use strict;
use warnings;

while(<>) {
	chop;
	if(m,build ./pdfium_test.exe,) {
		s/.*link //; s/\|\|.*//;
		print join(";", map { s,obj/(third_party/)?,,; s,/,\\,gr } grep { $_ !~ m,/((pdfium_test|googletest|gtest)/|(test_support|image_diff)\.lib$), } split(/ /)),";comctl32.lib;%(AdditionalDependencies)\n";
	}
}
