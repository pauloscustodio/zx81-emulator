#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

path("loaddata.dat")
    ->spew_raw( join '',
    map { chr( ord($_) - ord('a') + 0x26 ) } split //, "hello" );
make_p( "t/input/loaddata.bas", "t/input/loaddata.p" );
unlink "zx81.txt";
run_ok("@cmd t/input/loaddata.p");    # reads loaddata.dat, writes zx81.txt
is path("zx81.txt")->slurp_raw, "HELLO";

unlink "loaddata.dat", "zx81.txt";
unlink_testfiles();
done_testing();
