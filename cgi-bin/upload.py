#! /usr/bin/python3
import os
import cgi

uploaded = cgi.FieldStorage()
path = os.path


def check_file() -> str:
    if "image_input" in uploaded:
        uploaded_file = uploaded["image_input"]
        if not path.exists("./cgi-bin/tmp"):
            os.makedirs("./cgi-bin/tmp")
        file_path = "./cgi-bin/tmp/" + path.basename(uploaded_file.filename)
        open(file_path, "wb").write(uploaded_file.file.read())
        return "The file " + path.basename(uploaded_file.filename) + " was uploaded to " + os.getcwd() + "/cgi-bin/tmp"
    else:
        return "Error: uploading failed"


msg = check_file()
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
