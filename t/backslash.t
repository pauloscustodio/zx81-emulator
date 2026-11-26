#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p);

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

unlink "zx81.txt";
make_p( "t/input/backslash.bas", "t/input/backslash.p" );
push @cmd, 't/input/backslash.p';
ok system(@cmd) == 0, "Running @cmd";
check_text_file( "t/expected/backslash.txt", "zx81.txt" );

unlink_testfiles();
done_testing();
