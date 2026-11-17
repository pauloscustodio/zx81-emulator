#!/usr/bin/env perl

use Modern::Perl;
use Path::Tiny;

@ARGV == 2 or die "usage: $0 file.bin array-name\n";
my ( $input_file, $array_name ) = @ARGV;

my $data  = path($input_file)->slurp_raw;
my @bytes = map { ord } split //, $data;

print "#include <cstdint>\n";
print "static const uint8_t ", $array_name, "[", scalar(@bytes), "] = {\n";
while (@bytes) {
    print "    ";
    for ( 1 .. 8 ) {
        last unless @bytes;
        printf "0x%02x, ", shift @bytes;
    }
    print "\n";
}
print "};\n";
