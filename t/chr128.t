#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/chr128.bas", "t/input/chr128.p" );
run_ok("@cmd t/input/chr128.p");

unlink_testfiles();
done_testing();
