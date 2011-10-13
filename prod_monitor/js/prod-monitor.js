(function ($) {
	Drupal.behaviors.prod_monitor_js = {
		attach: function(context) {

			/* Prod monitor settings page styling */

			$(document).ready(function() {
				$('#prod-check-settings').equalHeights('px');
				$('#prod-check-settings').equalWidths('px');
			});

		}
	}
})(jQuery);