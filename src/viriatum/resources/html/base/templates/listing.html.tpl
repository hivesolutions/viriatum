<!DOCTYPE html>
<html lang="en">
    <head>
        <title>Viriatum / Listing</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />

        <!-- css inclusion -->
        <link rel="stylesheet" href="/resources/css/layout.css" type="text/css" />

        <!-- favicon inclusion -->
        <link rel="shortcut icon" href="/resources/images/favicon.ico" />
        <link rel="icon" href="/resources/images/favicon-2x.png" sizes="32x32" type="image/png" />
        <link rel="icon" href="/resources/images/favicon.svg" type="image/svg+xml" />
    </head>
    <body>
        <div id="header-wrapper">
            <div class="header-body">
                <div class="branding simple">
                    <a href="/"></a>
                </div>
            </div>
        </div>
        <div id="content-wrapper">
            <table class="table file-listing" cellpadding="0" cellspacing="0">
                <thead>
                    <th class="label left name" colspan="2">Name</th>
                    <th class="label right date">Last Modified</th>
                    <th class="label right size">Size</th>
                </thead>
                <tbody>
                    <tr class="current-folder">
                        <td class="icon arrow-left"><span></span></td>
                        <td colspan="3"><a href="../">${out value=folder_path /}</a></td>
                    </tr>
                    ${foreach item=entry from=entries}
                        <tr>
                            ${if item=entry.type value=1 operator=eq}
                                <td class="icon folder"></td>
                            ${/if}
                            ${if item=entry.type value=2 operator=eq}
                                <td class="icon text"></td>
                            ${/if}
                            ${if item=entry.type value=3 operator=eq}
                                <td class="icon link"></td>
                            ${/if}
                            <td class="left name"><a href="${out value=entry.name /}">${out value=entry.name /}</a></td>
                            <td class="right date">${out value=entry.time /}</td>
                            <td class="right size">${out value=entry.size_string /}</td>
                        </tr>
                    ${/foreach}
                </tbody>
                <tfoot>
                    <tr>
                        <td colspan="4" class="label center">${out value=items /} items</td>
                    </tr>
                </tfoot>
            </table>
        </div>
        <div id="footer-wrapper">
            <div class="footer-body">
                <div class="separator-horizontal"></div>
            </div>
        </div>
    </body>
</html>
