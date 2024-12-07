#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;
print $cgi->header;
print $cgi->start_html('Generador de contraseñas');

sub generar_contrasena {
    my $longitud = shift || 12;
    my @chars = ('a'..'z', 'A'..'Z', 0..9, qw(! @ # $ % ^ & *));
    return join '', map { $chars[rand @chars] } 1..$longitud;
}

my $contrasena = generar_contrasena(16);

print "<h1>Tu nueva contraseña es:</h1>";
print "<p style='font-size: 24px; font-weight: bold;'>$contrasena</p>";

print $cgi->end_html;