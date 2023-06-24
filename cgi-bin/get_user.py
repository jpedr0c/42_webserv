#!/usr/bin/python3
import cgi
import os

uploaded_user = cgi.FieldStorage()

# first_name = uploaded_user.getvalue("username")
# password = uploaded_user.getvalue("password")
# response_body = [
#     "HTTP/1.1 200 OK",
#     "Content-type: text/html;charset=utf-8\n",
#     "<html>",
#     "<head>",
#     "<title>Login | Webserv</title>",
#     "</head>",
#     "<body>",
#     f"<h2> Welcome, {first_name}! Your password is {password}",
#     "</body>",
#     "</html>"
# ]

# for i in response_body:
#     print(i)


first_name = uploaded_user.getvalue('first_name')
last_name = uploaded_user.getvalue('last_name')
print(os.environ["QUERY_STRING"], file=sys.stderr)
# print("HTTP/1.1 200 OK")
print("Content-type: text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("<html>")
print("<head>")
print("<h2>Hello %s %s</h2>" % (first_name, last_name))
print("</body>")
print("</html>")