<html>
    <head>
        <script type="text/javascript" src="http://smoothiecharts.org/smoothie.js"></script>
        <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js"></script>
        <script type="text/javascript" src="status.js"></script>
        <script type="text/javascript">
            var random = new TimeSeries();

            setInterval(function() {
                        $.getJSON("status_json.php", function(data) {
                                    var items = [];
                                    var connections = data["connections"];
                                    random.append(new Date().getTime(), connections);
                                });
                    }, 2500);

            function createTimeline() {
                var chart = new SmoothieChart();
                chart.addTimeSeries(random, {
                            strokeStyle : "rgba(0, 255, 0, 1)",
                            fillStyle : "rgba(0, 255, 0, 0.2)",
                            lineWidth : 4
                        });
                chart.streamTo(document.getElementById("chart"), 500);
                $.getJSON("stats_json.php", function(data) {
                            var items = [];
                            var connections = data["connections"];
                            random.append(new Date().getTime(), connections);
                        });
            }
        </script>
        <title>Viriatum</title>
    </head>
    <body onload="createTimeline()">
        <div>
            Name: <?php echo(viriatum_name()); ?><br />
            Version: <?php echo(viriatum_version()); ?><br />
            Connections: <?php echo(viriatum_connections()); ?><br />
            Date: <?php echo(viriatum_compilation_date()); ?>, <?php echo(viriatum_compilation_time()); ?><br />
        </div>
        <canvas id="chart" width="100" height="100"></canvas>
    </body>
</html>
