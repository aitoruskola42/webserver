#!/usr/bin/perl
use strict;
use warnings;

sub generar_contrasena {
    my ($longitud, $incluir_mayusculas, $incluir_numeros, $incluir_simbolos) = @_;
    my @chars = ('a'..'z');
    push @chars, ('A'..'Z') if $incluir_mayusculas;
    push @chars, (0..9) if $incluir_numeros;
    push @chars, qw(! @ # $ % ^ & * - _ + =) if $incluir_simbolos;
    return join '', map { $chars[rand @chars] } 1..$longitud;
}

my $longitud = 16;
my $contrasena = generar_contrasena($longitud, 1, 1, 1);
my $fuerza = length($contrasena) * 4;

print <<HTML;
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Generador de Contraseñas Seguras</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            background-color: white;
            padding: 2rem;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            text-align: center;
        }
        h1 {
            color: #333;
        }
        .password {
            font-size: 24px;
            font-weight: bold;
            background-color: #e9e9e9;
            padding: 10px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .strength {
            font-weight: bold;
            color: #4CAF50;
        }
        .copy-btn {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Generador de Contraseñas Seguras</h1>
        <p>Tu nueva contraseña es:</p>
        <div class="password" id="password">$contrasena</div>
        <button class="copy-btn" onclick="copiarContrasena()">Copiar al portapapeles</button>
        <p>Longitud: $longitud caracteres</p>
        <p>Fuerza estimada: <span class="strength">$fuerza bits</span></p>
        <p><small>Esta contraseña incluye letras mayúsculas, minúsculas, números y símbolos.</small></p>
    </div>

    <script>
    function copiarContrasena() {
        var passwordText = document.getElementById("password").innerText;
        navigator.clipboard.writeText(passwordText).then(function() {
            alert("Contraseña copiada al portapapeles");
        }, function(err) {
            console.error('Error al copiar: ', err);
        });
    }
    </script>
</body>
</html>
HTML