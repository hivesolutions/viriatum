# [Viriatum Web Server](http://viriatum.com) dump area

## Configuration Locations

    [location:colony]
    path = /colony
    handler = wsgi
    script_path = /root/repo_extra/colony/src/wsgi.py

    [location:automium]
    path = /automium
    handler = wsgi
    script_path = /usr/local/lib/python2.7/dist-packages/automium_web-0.1.0-py2.7.egg/automium_web.wsgi
