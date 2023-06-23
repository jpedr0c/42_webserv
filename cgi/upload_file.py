"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import os
import cgi

uploaded = cgi.FieldStorage()
path = os.path


def check_file() -> str:
    if "filename" in uploaded:
        uploaded_file = uploaded["filename"]
        if not path.exists("./cgi/tmp"):
            os.makedirs("./cgi/tmp")
        open(os.getcwd() + "./tmp" + path.basename(uploaded_file.filename), "wb").write(uploaded_file.file.read())
        return "The file " + path.basename(uploaded_file.filename) + " was uploaded to " + os.getcwd() + "/cgi/tmp"
    else:
        return "Error: uploading failed"


msg: str = check_file()
response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: image/png;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Images | Webserv</title>",
    "</head>",
    "<body>",
    f"<h1>{msg}</h1>",
    "<p>This is the response body.</p>",
    "</body>",
    "</html>"
]
[print(i) for i in response_body]
