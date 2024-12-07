#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;
print $cgi->header;
print $cgi->start_html('Contador de visitas');

my $archivo_contador = 'contador.txt';
my $visitas = 1;

if (-e $archivo_contador) {
    open my $fh, '<', $archivo_contador or die "No se puede abrir el archivo: $!";
    $visitas = <$fh>;
    close $fh;
    $visitas++;
}

open my $fh, '>', $archivo_contador or die "No se puede escribir en el archivo: $!";
print $fh $visitas;
close $fh;

print "<h1>Bienvenido</h1>";
print "<p>Eres el visitante número $visitas</p>";

print $cgi->end_html;