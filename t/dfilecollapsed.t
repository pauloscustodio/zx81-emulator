#!/usr/bin/env perl

BEGIN { use lib 't'; require 'testlib.pl'; }

use Modern::Perl;
use ZX81::Tape qw(make_p dump_p);

make_p( "t/input/dfilecollapsed.bas", "t/input/dfilecollapsed.p" );
run_ok("zx81 t/input/dfilecollapsed.p");

unlink_testfiles();
done_testing();
