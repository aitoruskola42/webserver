#!/usr/bin/perl
print "Content-type: text/html\n\n";
print "<html><body>";
print "<h1>Variables de entorno CGI</h1>";
print "<table border='1'>";
foreach my $key (sort keys %ENV) {
    print "<tr><td>$key</td><td>$ENV{$key}</td></tr>";
}
print "</table>";
print "</body></html>";