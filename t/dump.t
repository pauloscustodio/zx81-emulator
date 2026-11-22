#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

make_p( "t/input/dump.bas", "t/input/dump.p" );
unlink "zx81.dump";
run_ok("@cmd t/input/dump.p");    # creates zx81.dump
ok -f "zx81.dump", "zx81.dump file created";
run_ok("@cmd zx81.dump");         # loads the dump file

unlink_testfiles();
done_testing();
