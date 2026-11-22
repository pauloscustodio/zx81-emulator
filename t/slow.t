#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/slow.bas", "t/input/slow.p" );
run_ok("@cmd -s t/input/slow.p");

unlink_testfiles();
done_testing();
