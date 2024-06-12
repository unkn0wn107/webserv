import base64
import os
import sys
import json

print("Content-Type: text/html\r\n\r\n")

print("<html>")
print("<head>")
print("<title>Hello - CGI POST</title>")
print("</head>")
print("<body>")
print("<h2>Hello CGI POST</h2>")

print("<h3>Environment Variables:</h3>")
print("<pre>")
for var_name, var_value in os.environ.items():
    print(f"{var_name} = {var_value}")
print("</pre>")

# Get form data from POST request
try:
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
except ValueError:
    content_length = 0

if content_length > 0:
    post_data = sys.stdin.read(content_length)
    form_data = json.loads(post_data)
    
    # Print form data
    print("<h3>POST Data:</h3>")
    print("<pre>")
    for key, value in form_data.items():
        print(f"{key} = {value}")
    print("</pre>")

    if "name" in form_data:
        name = form_data["name"]
        print("<h3>Hello {}!</h3>".format(name))
    else:
        print("<h3>Error: name not provided!</h3>")
else:
    print("<h3>Error: no data received!</h3>")

print("</body>")
print("</html>")
