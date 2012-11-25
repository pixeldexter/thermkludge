#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;
use Pod::Usage;
use Data::Dumper;

my $config = {
	      pagesize => 64,
	      maxpages => 96,
};

GetOptions($config,
	   'input=s',
	   'output=s',
	   'pagesize=i',
	   'maxpages=i',
	  ) or pod2usage(255);

die "--input missing" unless exists $config->{input};

open (my $fh, "< $config->{input}") or die $!;
binmode $fh, ":bytes";

my $pagecount = 0;
while ( read($fh, my $page, $config->{pagesize}) ) {
#  my @bytes = pack 'C*', (split(//, $page));
  my @bytes = split //, $page;

  my $offset = $config->{pagesize} / 2 / 8;
  while ( $offset < $config->{pagesize} ) {
    my $time = ord($bytes[$offset + 0]) + (ord($bytes[$offset + 1]) << 8);
    my $data = ord($bytes[$offset + 2]) + (ord($bytes[$offset + 3]) << 8);
    $offset += 4;

    last if $time == 65535;
    printf "%d,%f\n", $time, $data / 16.0;
  }
  $pagecount++;
  last if $pagecount ge $config->{maxpages};
}


