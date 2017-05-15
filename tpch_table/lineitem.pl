 #!/usr/local/bin/perl

# print "Hi there!\n"; nation.tbl

  use strict;
  use warnings;
  
open my $in, "<", "$ARGV[0]"  or die ("$ARGV[0]: $!");
my @lines = <$in>; #read all the contents into the memory...
close $in;
chomp @lines;

  my @file_h;

  print "Geneated columns to files: \n";

   my $flag = 0;
   
for my $line (@lines) 
{

  #if ($line =~ /^\|/) 
  if ($line =~ /^\|\s*(\d|null)/) #whether the line begin with |, following with " ", and then 0-9 
  {
    my @values1 = split /\|/, $line;
	#print "$values[0]\n";
    for (my $index1 = 1; $index1 <= $#values1; $index1++) #foreach my $val (@values) # @#values returns the index of the last element of the array "values"
	{  
	   my $val = $values1[$index1];
	   if ($val =~ /^\s*(.*)\s*$/) #delete the beginning and ending spaces....
	   {
	     print {$file_h[$index1]} "$1\n"
	   } 
      # print "$val\n";
    } 
  }
  elsif ($line =~ /^\|\s*[a-zA-Z]/) #capture the column name, which is expressed in the sql program.
  {
    if ($flag > 0)
	{
	  print "$line\n";
	  last;
	}

    my @values = split /\|/, $line;
	#print "$values[0]\n";
    for (my $index = 1; $index <= $#values; $index++) #foreach my $val (@values) # @#values returns the index of the last element of the array "values"
	{  
	   my $val = $values[$index]; 
	   
	   if ($val =~ /^\s*(.*\w)\s*$/) #delete the beginning and ending spaces....
	   {
         open($file_h[$index], "+>", "$1.txt")
            || die "cannot open $1.txt: $!";	   
	     print "$1.txt\n";
	   }
      # print "$val\n";
    }
	
	$flag = 1;
  } 
  


  
}

#while (my $line = <$in>) 
#{
#    chomp $line;
#     my @values = split /\|/, $line;
#	 print "@values\n";
#    foreach my $val (@values) 
#	{
#       print "$val\n";
#    }
#}	
close $in;

#  my $data = 'Becky Alcorn,25,female,Melbourne';

#   my @values = split(',', $data);



  exit 0;