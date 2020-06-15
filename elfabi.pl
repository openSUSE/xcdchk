#! /usr/bin/perl -w

my $compare = 0;
my @files = ();
foreach (@ARGV) {
  if (/^-/) {
    $compare = 1 if ($_ eq '-c');
  } else {
    push @files, $_;
  }
}

foreach my $f (@files) {
  next unless -s $f;
  my @funcs = ();
  my @datas = ();
  my $symtabnum = 0;
  open F, "readelf -sW $f|" or next;
  while (<F>) {
      chomp;
      #    Num:    Value  Size Type    Bind   Vis      Ndx Name
      my @field = split;
      next unless @field;
      if ($field[0] eq 'Symbol') {
	  $symtabnum++;
	  last if $symtabnum > 1;
	  next;
      }
      next unless $field[0] =~ /^[0-9]+:$/;
      next if $field[6] eq 'UND';
      next if $field[4] eq 'LOCAL';
      my $name = $field[7];
      # a @@ means "default version".  As far as exporting is
      # concerned this is the same as simply exporting this version
      $name =~ s/\@\@/\@/;
      if ($field[3] eq 'FUNC') {
	  #print "func $field[7]\n";
	  push @funcs, $name;
      } elsif ($field[3] eq 'OBJECT') {
	  #print "data $field[7]\n";
	  # for data objects we append the size
	  $name .= "@".$field[2];
	  push @datas, $name;
      }
  }
  print "$f\n" if (@files >= 2);
  print "functions:\n";
  print join("\n", sort @funcs)."\n";
  print "data:\n";
  print join("\n", sort @datas)."\n";
  close F;
}
