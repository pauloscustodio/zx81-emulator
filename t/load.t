#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/hello.bas", "t/input/hello.p" );
make_p( "t/input/load.bas",  "t/input/load.p" );
run_ok("@cmd t/input/load.p");    # loads HELLO and runs it

unlink_testfiles();
done_testing();
