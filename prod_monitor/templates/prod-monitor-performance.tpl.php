<script type='text/javascript' src='http://www.google.com/jsapi'></script>

<?php
  foreach ($data as $module => $data_set) {
    $graph_title = $cols = $row_data = $row_time = '';
    $count = $i = 0;
?>
<script type='text/javascript'>
  google.load('visualization', '1', {'packages':['annotatedtimeline']});
  google.setOnLoadCallback(drawChart);
  function drawChart() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'Date');
<?php
    foreach ($data_set as $time => $params) {
      $graph_title = $params['title'];
      $row_time = date('Y, n, j, G, i, s', $time);
      $count = count($params['data']);
      $row_tmp = '';
      foreach ($params['data'] as $title => $row) {
        if ($i < $count) {
          $cols .= "    data.addColumn('number', '$title');\n";
          $i++;
        }
        // Cast empty strings to 0.
        $row_tmp .= (int) $row[0] . ', ';
      }
      $row_data .= "      [new Date($row_time), " . rtrim($row_tmp, ', ') . "],\n";
    }
    // TODO: how to handle massive amounts of data here? There's got to be a
    // beter way. Check the google API.
    print $cols;
    print "    data.addRows([\n$row_data    ]);\n";
?>
    var chart = new google.visualization.AnnotatedTimeLine(document.getElementById('<?php print $module ?>'));
    chart.draw(data, {displayAnnotations: false});
  }
</script>
<h2><?php print $graph_title ?></h2>
<div id="<?php print $module ?>" style="width: 960px; height: 300px;"></div>
<?php
  }
?>
