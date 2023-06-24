#! /usr/bin/python3

import os
from http import cookies
# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
uploaded_cookies = cgi.FieldStorage()

# Get data from fields
key = uploaded_cookies.getvalue('key')
value  = uploaded_cookies.getvalue('value')
cookie = cookies.SimpleCookie()
cookie[key] = value
print("HTTP/1.1 204 OK")
print(cookie.output())
