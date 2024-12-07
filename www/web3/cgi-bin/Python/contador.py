#!/usr/bin/env python
import time;
def read_counter():
    try:
        with open('counter.txt', 'r') as f:
            return int(f.read().strip())
    except FileNotFoundError:
        return 0

def write_counter(count):
    with open('counter.txt', 'w') as f:
        f.write(str(count))

count = read_counter() + 1
write_counter(count)

# Obtener la fecha y hora actual
fecha_actual = time.strftime("%d/%m/%Y")
hora_actual = time.strftime("%H:%M:%S")

print(f"""
<html>
<head>
    <meta charset="UTF-8">
    <title>Contador de Visitas Impresionante</title>
    <style>
        body {{ font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; }}
        .container {{ background-color: white; border-radius: 10px; padding: 20px; margin: 50px auto; max-width: 500px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }}
        h1 {{ color: #333; }}
        .count {{ font-size: 48px; color: #007bff; margin: 20px 0; }}
        .info {{ color: #666; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>¡Contador de Visitas Impresionante!</h1>
        <div class="count">{count}</div>
        <p class="info">Esta página ha sido visitada {count} veces.</p>
        <p class="info">Fecha: {fecha_actual}</p>
        <p class="info">Hora: {hora_actual}</p>
    </div>
</body>
</html>
""")