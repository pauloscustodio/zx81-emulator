#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

unlink "zx81.txt";
make_p( "t/input/copy.bas", "t/input/copy.p" );
run_ok("@cmd t/input/copy.p");
check_text_file( "t/expected/copy.txt", "zx81.txt" );

unlink_testfiles();
done_testing();
