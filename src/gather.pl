#!perl  -w 

use strict;

while (1) {

	my $c = `bin/controller -c controller.conf --pairfile device-600194092466.pair --ip 192.168.1.231 --port 2030 --tcp --get --control 4`;

	my $str = '';
	foreach my $l ( split (/\n/, $c) )
	{

		if ( $l =~ m/^===    Control:    4/ ) {
			$str = $l;
		}
	}
	my ($adc) = ($str =~ m/ADC +(\d+)/);


	print " ## $str\n";
	print " ADC $adc\n";

	my $t = time();

	open FH, "+>>", "data.csv";
	print FH "$t,$adc\n";
	close FH;
	sleep(300);

	}