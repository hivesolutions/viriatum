# Dump area

This is the area where there are no boundaries between useful and stupidity.
Just throw random content to this section.

## Config Locations

Examples of location sections that can be used to confgiure wsgi for example for colony or automium.

    [location:colony]
    path = /colony
    handler = wsgi
    script_path = /root/repo_extra/colony/src/wsgi.py

    [location:automium]
    path = /automium
    handler = wsgi
    script_path = /usr/local/lib/python2.7/dist-packages/automium_web-0.1.0-py2.7.egg/automium_web.wsgi
