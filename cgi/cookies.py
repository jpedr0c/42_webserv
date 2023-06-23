"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import os
from http import cookies

uploaded_cookies = cookies.BaseCookie()
print("HTTP1.1 200 OK\nContent-type: text/html;charset=utf-8\n")
if "HTTP-COOKIE" in os.environ:
    uploaded_cookies.load(os.environ["HTTP_COOKIE"])
    [print(i, morsel) for i, morsel in uploaded_cookies.items()]
else:
    print("Cookies have not yet been set")
