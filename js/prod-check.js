/* $Id$ */

/* Prod monitor settings page styling */

$(document).ready(function() {
  $('#prod-check-settings').equalHeights('px');
  $('#prod-check-settings').equalWidths('px');
});

$('#prod-check-nagios').change(function() {
  $('#prod-check-settings').equalHeights('px');
  $('#prod-check-settings').equalWidths('px');
});
