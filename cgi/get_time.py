"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import datetime

dt = datetime.datetime
time: str = dt.strftime(dt.now(), "%H:%M:%S")
response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Current Time</title>",
    "</head>",
    "<body>",
    f"<h1>{time}</h1>",
    "</body>",
    "</html>"
]
[print(i) for i in response_body]
