<!DOCTYPE html>
<html lang="en">
    <head>
        <title>Viriatum Listing</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

        <!-- css inclusion -->
        <link rel="stylesheet" href="/resources/css/layout.css" type="text/css" />

        <!-- favicon inclusion -->
        <link rel="shortcut icon" href="/resources/images/favicon.ico" />
    </head>
    <body>
        <div id="content-wrapper">
            <div class="content-header error">
                <div class="error-illustration"></div>
                <div class="error-content">
                    <div class="error-code">${out value=error_code /}</div>
                    <h1 class="error-description">${out value=error_message /}</h1>
                    <p>Maybe you've missed something, please double check the url value</p>
                </div>
                <div class="clear"></div>
            </div>
        </div>
    </body>
</html>
