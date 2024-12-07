import cgi

def main(): # Línea en blanco para separar encabezados del cuerpo

    form = cgi.FieldStorage()
    num1 = float(form.getvalue("num1", 0))
    num2 = float(form.getvalue("num2", 0))
    operation = form.getvalue("operation", "add")

    if operation == "add":
        result = num1 + num2
        operation = "+"
    elif operation == "subtract":
        result = num1 - num2
    elif operation == "multiply":
        result = num1 * num2
        operation = "*"
    elif operation == "divide":

        result = num1 / num2 if num2 != 0 else "Error: División por cero"
        operation = "/"
    else:
        result = "Operación no válida"

    # Generar respuesta HTML
    response = f"""
    <html>
    <head>
        <title>Resultado de la Calculadora</title>
        <style>
            body {{
                font-family: Arial, sans-serif;
                background-color: #f4f4f4;
                margin: 0;
                padding: 20px;
            }}
            .container {{
                max-width: 600px;
                margin: auto;
                background: white;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }}
            h1 {{
                color: #333;
            }}
            p {{
                font-size: 18px;
                color: #555;
            }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>Resultado</h1>
            <p>El resultado de <strong>{num1}</strong> <strong>{operation}</strong> <strong>{num2}</strong> es: <strong>{result}</strong></p>
        </div>
    </body>
    </html>
    """
    print(response)

if __name__ == "__main__":
    main()