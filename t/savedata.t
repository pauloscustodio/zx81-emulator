#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/savedata.bas", "t/input/savedata.p" );
unlink "savedata.dat";
run_ok("@cmd t/input/savedata.p");    # creates savedata.dat
my $saved_data = join '', 
        map {chr(ord($_) - 0x26 + ord('a'))}
        split '', path("savedata.dat")->slurp_raw;
is $saved_data, "hello";

unlink "savedata.dat";
unlink_testfiles();
done_testing();
