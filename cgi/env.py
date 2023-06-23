"""
-> How to activate venv in Ubuntu
    - In the directory cgi, rum the command:
        source ./venv/bin/activate
"""

import os
response_body = ["HTTP/1.1 200 OK", "Content-type: text/html\n", "<html>", "<head></head>", "<body>"]
[print(i) for i in response_body]

for i in os.environ.keys():
    print("<b>%20s</b>: %s<br>" % (i, os.environ[i]))
print("</body>\n</html>")
