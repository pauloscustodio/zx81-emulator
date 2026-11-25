#!/usr/bin/env perl

use Modern::Perl;
use Path::Tiny;

@ARGV == 2 or die "usage: $0 file.lis vector-name\n";
my ( $input_file, $vector_name ) = @ARGV;

# collect locations from the list file
my @locs;
my @lines    = path($input_file)->lines( { chomp => 1 } );
my $filename = "";
for (@lines) {
    if (/^(\S+):$/) {
        $filename = $1;
        next;
    }

    if (/^\s+(\d+)\s+([0-9a-f]{4})/i) {
        my ( $line_num, $addr ) = ( $1, hex($2) );
        push @locs, [ $addr, $filename, $line_num ];
        next;
    }
}

# sort by addr
@locs = sort { $a->[0] <=> $b->[0] } @locs;

# create C++ table
say "#include <vector>";
say "struct LocEntry { int addr; const char* filename; int line_num; };";
say "static const std::vector<LocEntry> $vector_name = {";
for (@locs) {
    my ( $addr, $filename, $line_num ) = @$_;
    say "    { ", sprintf( "0x%04x", $addr ), ", \"$filename\", $line_num },";
}
say "};";
