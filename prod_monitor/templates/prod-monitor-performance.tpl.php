<script type='text/javascript' src='http://www.google.com/jsapi'></script>

<?php
  // TODO: move this first code block to the preprocess function?

  // This data holds all graphs per module, timestamp and per unit (MB, ms, ...).
  $graphs = array();
  foreach ($data as $module => $data_set) {
    // Counters to make sure we only add columns once.
    $count = $i = 0;
    foreach ($data_set as $time => $params) {
      // Store title for this modules graphs.
      $graphs[$module]['title'] = $params['title'];
      // Count all rows.
      $count = count($params['data']);
      foreach ($params['data'] as $title => $row) {
        if ($i < $count) {
          // Setup the columns for the graph in such a way that they are
          // organised by unit so we can draw one graph per unit.
          $graphs[$module][$row[1]]['cols'][] = "      data.addColumn('number', '$title');";
          $i++;
        }
        // Cast empty strings to 0 and store the data per timestamp and unit so
        // that we can draw one graph per unit.
        $graphs[$module][$row[1]]['rows'][$time][] = (int) $row[0];
      }
    }
  }

  // Output all graphs.
  foreach ($graphs as $module => $data) {
?>
<h2><?php print $data['title'] ?></h2>
<?php
    unset($data['title']);
    foreach ($data as $unit => $numbers) {
?>
<script type='text/javascript'>
  google.load('visualization', '1', {'packages':['annotatedtimeline']});
  google.setOnLoadCallback(drawChart);
  function drawChart() {
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'Date');
<?php
      // Add columns.
      foreach ($numbers['cols'] as $col) {
        print $col . "\n";
      }
?>
    data.addRows([
<?php
      // Add column data.
      foreach ($numbers['rows'] as $time => $row) {
        print '      [new Date(' . date('Y, n, j, G, i, s', $time) . '), ' . implode(', ', $row ) . "],\n";
      }
?>
    ]);

    var chart = new google.visualization.AnnotatedTimeLine(document.getElementById('<?php print $module . '-' . $unit; ?>'));
    chart.draw(data, {displayAnnotations: false});
  }
</script>
<div id="<?php print $module . '-' . $unit; ?>" style="width: 960px; height: 300px;"></div>
<p>&nbsp;</p>
<?php
    }
  }
?>
