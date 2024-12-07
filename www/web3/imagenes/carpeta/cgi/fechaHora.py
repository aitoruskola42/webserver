#!/usr/bin/env python
from datetime import datetime

print("<html><body>")
print("<h1>Fecha y hora actual</h1>")
print(f"<p>La fecha y hora actual es: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
print("</body></html>")