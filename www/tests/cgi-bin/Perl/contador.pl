#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;
print $cgi->start_html('Visitor Counter');

my $archivo_contador = 'contador.txt';
my $visitas = 1;

if (-e $archivo_contador) {
    open my $fh, '<', $archivo_contador or die "file can't be opened: $!";
    $visitas = <$fh>;
    close $fh;
    $visitas++;
}

open my $fh, '>', $archivo_contador or die "file can't be opened: $!";
print $fh $visitas;
close $fh;

print "<h1>Welcome!</h1>";
print "<p>You are the visitor number $visitas</p>";

print $cgi->end_html;