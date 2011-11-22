<?php
/**
 * @file
 * Template that renders a google graph. Feel free to override it if you prefer
 * another graphing mechanism by placing it in your themes folder.
 *
 * Available variables:
 *  $data contains raw data fetched from the prod_monitor_performance table.
 *  $graphs contains preprocessed data to allow (more) easy output.
 *
 * TODO: use Drupal.behaviors to do all of this JS properly.
 */
?>
<script type='text/javascript' src='http://www.google.com/jsapi'></script>

<?php
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
        print "    data.addColumn('number', '$col');\n";
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
