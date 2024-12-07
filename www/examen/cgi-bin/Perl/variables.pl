#!/usr/bin/perl
use strict;
use warnings;

my @env_vars = sort keys %ENV;
my $count = scalar @env_vars;

print <<HTML;
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Variables de entorno CGI</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f0f0; }
        h1 { color: #333; text-align: center; }
        table { width: 80%; margin: 20px auto; border-collapse: collapse; }
        th, td { padding: 10px; border: 1px solid #ddd; }
        tr:nth-child(even) { background-color: #f9f9f9; }
        tr:hover { background-color: #f5f5f5; }
        th { background-color: #4CAF50; color: white; }
        .count { text-align: center; font-weight: bold; margin-top: 20px; }
    </style>
</head>
<body>
    <h1>Variables de entorno CGI</h1>
    <table>
        <tr><th>Variable</th><th>Valor</th></tr>
HTML

foreach my $key (@env_vars) {
    my $value = $ENV{$key};
    $value =~ s/&/&amp;/g;
    $value =~ s/</&lt;/g;
    $value =~ s/>/&gt;/g;
    print "<tr><td>$key</td><td>$value</td></tr>\n";
}

print <<HTML;
    </table>
    <div class="count">Total de variables de entorno: $count</div>
</body>
</html>
HTML