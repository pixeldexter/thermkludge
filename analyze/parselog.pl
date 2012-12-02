#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;
use Data::Dumper;

my $config = {
	      pagesize => 64,
	      maxpages => 96,
	      sensors => 1,
};

GetOptions($config,
	   'input=s',
	   'output=s',
	   'pagesize=i',
	   'maxpages=i',
	   'sensors=i',
	  ) or pod2usage(255);

die "--input missing" unless exists $config->{input};

open (my $fh, "< $config->{input}") or die $!;
binmode $fh, ":bytes";

my $pagecount = 0;
while ( read($fh, my $page, $config->{pagesize}) ) {

  my $offset = $config->{pagesize} / 2 / 8;
  while ( $offset < $config->{pagesize} ) {
    # read time
    my $time = unpack("S", substr($page, $offset, 2));
    $offset += 2;
    last if $time == 65535;
    printf "%d", $time;

    # read samples
    for(my $i=0; $i!=$config->{sensors}; ++$i) {
      my $data = unpack("s", substr($page, $offset, 2));
      $offset += 2;
      printf ",%f", $data / 16.0;
    }
    printf "\n";
  }
  $pagecount++;
  last if $pagecount ge $config->{maxpages};
}


