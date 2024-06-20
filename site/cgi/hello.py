import base64
import os
import sys
from urllib.parse import parse_qsl

print("Content-Type: text/html\r\n\r\n")

print("<html>")
print("<head>")
print("<title>Hello - CGI Python Script</title>")
print("</head>")
print("<body>")
print("<h2>Hello CGI</h2>")

print("<h3>Environment Variables:</h3>")
print("<pre>")
for var_name, var_value in os.environ.items():
    print(f"{var_name} = {var_value}")
print("</pre>")

# Get form data from QUERY_STRING environment variable
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    form_pairs = parse_qsl(query_string)  # Parse query string into key-value pairs
    form_dict = dict(form_pairs)  # Convert key-value pairs into a dictionary

    # Print query strings
    print("<h3>Query Strings:</h3>")
    print("<pre>")  # Use <pre> tag to preserve formatting
    for key, value in form_pairs:
        print(f"{key} = {value}")
    print("</pre>")

    # Check if name is in the form data
    if "name" in form_dict:
        name = form_dict["name"]
        print("<h3>Hello {}!</h3>".format(name))
    else:
        print("<h3>btw what's your name ?!</h3>")
else:
    print("<h3>Error: no data received as query string!</h3>")

print("</body>")
print("</html>")
