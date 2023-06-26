#! /usr/bin/python3
import os
import cgi

uploaded = cgi.FieldStorage()
path = os.path


def check_file() -> str:
    if "image_input" in uploaded:
        uploaded_file = uploaded["image_input"]
        if not path.exists("./cgi/tmp"):
            os.makedirs("./cgi/tmp")
        file_path = "./cgi/tmp/" + path.basename(uploaded_file.filename)
        open(file_path, "wb").write(uploaded_file.file.read())
        return os.getcwd() + "/cgi/tmp/" + path.basename(uploaded_file.filename)
    else:
        return "Error: uploading failed"


image_path = check_file()
response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Current Time</title>",
    '<script src="https://cdn.tailwindcss.com"></script>',
    "</head>",
    '<body class="bg-slate-900 h-screen w-screen flex justify-center items-center text-slate-100">',
    '<div class="flex flex-col space-y-7 flex justify-center items-center p-10">',
    '<img src="https://private-user-images.githubusercontent.com/54289589/248842975-c2933c77-44dd-4b2b-ba4f-dbbdd3cc0a41.svg?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJrZXkiOiJrZXkxIiwiZXhwIjoxNjg3NzkxNDQzLCJuYmYiOjE2ODc3OTExNDMsInBhdGgiOiIvNTQyODk1ODkvMjQ4ODQyOTc1LWMyOTMzYzc3LTQ0ZGQtNGIyYi1iYTRmLWRiYmRkM2NjMGE0MS5zdmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBSVdOSllBWDRDU1ZFSDUzQSUyRjIwMjMwNjI2JTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDIzMDYyNlQxNDUyMjNaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT0zZjJlYzExNzM1YTVjMmYwYTkwODY2YzlhMmVlNmIwYjM0NWJkMWQ3MTRkZmJhOWJiNGVkZjM5ZjM5ZGQyMTYxJlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCZhY3Rvcl9pZD0wJmtleV9pZD0wJnJlcG9faWQ9MCJ9.9JDZTA29ECovTWAHtsQL_9323zEBw6_9MXmKvtSezuE" class="w-96" />',
    '<div class="flex flex-col justify-center items-center space-y-3">',
    '<h1 class="font-bold text-3xl text-slate-400">Image uplodad to:</h1>',
    f'<code class="font-bold text-sm bg-slate-800 text-purple-400 p-3 rounded-md md:text-xl">{image_path}</code>',
    "</div>",
    '<a href="../index.html"class="py-2 px-7 text-center ring-2 ring-purple-500 rounded-md">Ir para página inicial</a>',
    "</div>",
    "</body>",
    "</html>",
]
[print(i) for i in response_body]
