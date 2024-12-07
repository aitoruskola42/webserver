#!/usr/bin/perl
use strict;
use warnings;
use Time::Piece;

my $archivo_contador = 'contador.txt';
my $archivo_log = 'visitas_log.txt';

# Obtener información de la visita
my $ip = $ENV{REMOTE_ADDR} || 'Desconocida';
my $fecha = Time::Piece->new->strftime('%Y-%m-%d %H:%M:%S');

# Leer y actualizar el contador
my $visitas = leer_actualizar_contador($archivo_contador);

# Registrar la visita en el log
registrar_visita($archivo_log, $ip, $fecha);

# Generar HTML con estilos mejorados
imprimir_html($visitas, $ip, $fecha);

sub leer_actualizar_contador {
    my ($archivo) = @_;
    my $visitas = 1;

    if (-e $archivo) {
        open my $fh, '+<', $archivo or die "No se puede abrir el archivo: $!";
        $visitas = <$fh> + 1;
        seek $fh, 0, 0;
        print $fh $visitas;
        close $fh;
    } else {
        open my $fh, '>', $archivo or die "No se puede crear el archivo: $!";
        print $fh $visitas;
        close $fh;
    }

    return $visitas;
}

sub registrar_visita {
    my ($archivo, $ip, $fecha) = @_;
    open my $fh, '>>', $archivo or die "No se puede abrir el archivo de log: $!";
    print $fh "$fecha - IP: $ip\n";
    close $fh;
}

sub imprimir_html {
    my ($visitas, $ip, $fecha) = @_;
    print <<HTML;
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Contador de Visitas Mejorado</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; }
        .container { background-color: white; border-radius: 10px; padding: 20px; margin: 50px auto; max-width: 600px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .visitas { font-size: 24px; color: #007bff; }
        .info { font-size: 14px; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>¡Bienvenido a nuestro sitio!</h1>
        <p class="visitas">Eres el visitante número <strong>$visitas</strong></p>
        <p class="info">Tu IP: $ip</p>
        <p class="info">Fecha y hora de la visita: $fecha</p>
    </div>
</body>
</html>
HTML
}