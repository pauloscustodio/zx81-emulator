#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

run_ok(
    "perl tools/filepp-1.8.0/filepp -UROM -o $test.bas t/input/keyboard.bas");
make_p( "$test.bas", "$test.p" );
run_ok("@cmd -s $test.p");

unlink_testfiles();
done_testing();
