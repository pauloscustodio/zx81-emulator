#!/usr/bin/env perl

use Modern::Perl;
use Path::Tiny;

@ARGV == 2 or die "usage: $0 file.map vector-name\n";
my ( $input_file, $vector_name ) = @ARGV;

# collect constants from the map file
my @consts;
my @lines = path($input_file)->lines( { chomp => 1 } );
for (@lines) {
    next if /^(__head|__size|__tail)/;

    my @fields = split /\s*[=;,]\s*/, $_;
    die "invalid line from map file: $_" if @fields < 8;

    my $name             = $fields[0];
    my $value_hex        = $fields[1];
    my $source_file_line = $fields[7];

    $value_hex =~ /^\$([0-9a-f]+)$/i or die "invalid hex value: $value_hex";
    my $value = hex($1);
    $value = -1 - ( 0xffffffff ^ $value ) if ( $value & 0x80000000 );
    my $value_fmt = $value < 10 ? $value : sprintf( "0x%04x", $value );

    my ( $source_file, $line_num ) = split /:/, $source_file_line;

    push @consts, [ $value, $name, $value_fmt, $source_file, $line_num ];
}

# sort by value
@consts = sort { $a->[0] <=> $b->[0] } @consts;

# create C++ table
say "#include \"symbols.h\"";
say "#include <vector>";
say "static const std::vector<Symbol> $vector_name = {";
for (@consts) {
    my ( $value, $name, $value_fmt, $source_file, $line_num ) = @$_;
    say "    { \"$name\", $value_fmt, \"$source_file\", $line_num },";
}
say "};";
