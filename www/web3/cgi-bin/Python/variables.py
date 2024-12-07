#!/usr/bin/env python
import os

print("<html><body>")
print("<h1>Variables de entorno CGI</h1>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}:</strong> {value}</li>")
print("</ul>")
print("</body></html>")