#! /usr/bin/python3
import os
from http import cookies
# Import modules for CGI handling 
import cgi, cgitb 

uploaded_cookies = cgi.FieldStorage()
key = uploaded_cookies.getvalue("key")
value = uploaded_cookies.getvalue("value")
response_body = ["HTTP1.1 200 OK\n", "Content-type: text/html;charset=utf-8\n"]

cookie = cookies.SimpleCookie()
if "HTTP_COOKIE" in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])
if key in cookie:
    response_body.append(f"The value of cookie {key} is {cookie[key].value}")
else:
    response_body.append("Cookie was not found!")

[print(i) for i in response_body]