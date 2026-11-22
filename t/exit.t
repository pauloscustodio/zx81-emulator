#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/exit.bas", "t/input/exit.p" );
capture_ok( "@cmd t/input/exit.p", "t/expected/exit.txt" );

unlink_testfiles();
done_testing();
