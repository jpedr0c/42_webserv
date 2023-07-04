#!/usr/bin/python3

import cgi

form = cgi.FieldStorage()

if "username" in form:
    first_name = form.getvalue("username")
else:
    first_name = "Visitor"

if "password" in form:
    password = form.getvalue("password")
else:
    password = "default"

first_name = form.getvalue("username")
password = form.getvalue("password")

response_body = [
    "HTTP/1.1 200 OK",
    "Content-type: text/html;charset=utf-8\n",
    "<html>",
    "<head>",
    "<title>Welcome</title>",
    '<script src="https://cdn.tailwindcss.com"></script>',
    "</head>",
    '<body class="bg-black h-screen w-screen flex justify-center items-center">',
    '<div class="flex flex-col space-y-7 flex justify-center items-center p-10">',
    '<div class="flex flex-col justify-center items-center space-y-1">',
    f'<h1 class="bg-gradient-to-r from-rose-500 to-purple-600 bg-clip-text text-7xl leading-loose font-extrabold text-transparent">Welcome back, {first_name}</h1>',
    f'<p class="font-bold text-2xl text-gray-300">Your password is: {password}</p>',
    '<p class="font-bold text-xs text-gray-300">*LGPD que se lasque</p>',
    "</div>",
    '<div class="flex bg-gradient-to-r from-rose-500 to-purple-600 p-1 rounded-lg hover:scale-105 transition-all duration-300">',
    '<a href="../login.html" class="px-7 bg-black py-3 font-bold text-center rounded-md text-slate-100">Back to login</a>',
    "</div>",
    "</div>",
    "</body>",
    "</html>",
]
[print(i) for i in response_body]
