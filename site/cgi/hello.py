import cgi
import cgitb

# Enable debugging
cgitb.enable()

print("Content-Type: text/html")    # HTML is following
print()                             # blank line, end of headers

form = cgi.FieldStorage()           # parse form data

print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("</head>")
print("<body>")
print("<h2>Hello CGI Program</h2>")

if "name" in form:
    name = form["name"].value
    print("<h3>Hello {}!</h3>".format(name))
else:
    print("<h3>Error: name not provided!</h3>")

print("</body>")
print("</html>")
