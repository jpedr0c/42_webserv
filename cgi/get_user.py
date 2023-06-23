"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import cgi

uploaded_user = cgi.FieldStorage()

first_name = uploaded_user.getvalue("first_name")
last_name = uploaded_user.getvalue("last_name")
response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Login | Webserv</title>",
    "</head>",
    "<body>",
    f"<h2> Welcome, {first_name + ' ' + last_name}",
    "</body>",
    "</html>"
]

[print(i) for i in response_body]
