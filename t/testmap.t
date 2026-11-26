#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;

my $headless = $ENV{ZX81_HEADLESS} // 0;
my @cmd      = ('zx81');
push @cmd, '-h' if $headless;

unlink "zx81.txt";
run_ok("@cmd -m t/input/testmap.map t/input/testmap.p");
check_text_file( "t/expected/testmap.txt", "zx81.txt" );

unlink_testfiles();
done_testing();
