<?php
// $Id$

//TODO: add fetch data function + query to get url of a site for better feedback

/**
 * Implementation of hook_drush_command().
 */
function prod_monitor_drush_command() {
  $items = array();

  $items['prod-monitor'] = array(
    'callback' => 'drush_prod_monitor_statusdetail',
    'description' => "Display the Production Monitor status page",
    'aliases' => array('pmon'),
    'arguments' => array(
      'id' => "ID of the site to view it's status in detail.",
    ),
  );
  $items['prod-monitor-fetch'] = array(
    'callback' => 'drush_prod_monitor_fetch',
    'description' => "Fetch the status information from a given remote site.",
    'aliases' => array('pmon-fe'),
  );
  $items['prod-monitor-flush'] = array(
    'callback' => 'drush_prod_monitor_flush',
    'description' => "Remove all fetched data for a given site.",
    'aliases' => array('pmon-fl'),
  );
  $items['prod-monitor-delete'] = array(
    'callback' => 'drush_prod_monitor_delsite',
    'description' => "Completemy remove a site and all its data.",
    'aliases' => array('pmon-rm'),
  );

  return $items;
}

function drush_prod_monitor_fetch() {
  $args = func_get_args();
  //_prod_monitor_retrieve_data($id, $site_info, $msg = FALSE);
}

/**
 * Flush data callback.
 */
function drush_prod_monitor_flush() {
  $args = func_get_args();
  for ($i = 0; $i < count($args); $i++) {
    if (!drush_confirm(dt('Do you really want to flush all data?'))) {
      drush_die('Aborting.');
    }
    $result = _prod_monitor_flush_data($args[$i]);
    if ($result === FALSE) {
      drush_print("\033[1;31m".dt('Error:')." \033[0m".dt('Unable to flush data!'));
    }
    else {
      drush_print(dt('Stored data successfully flushed.'));
    }
  }
}

/**
 * Delete site callback.
 */
function drush_prod_monitor_delsite() {
  $args = func_get_args();
  for ($i = 0; $i < count($args); $i++) {
    if (!drush_confirm(dt("Do you really want to remove the site and all it's data?"))) {
      drush_die('Aborting.');
    }
    $result = _prod_monitor_delete_data($args[$i]);
    if ($result === FALSE) {
      drush_print("\033[1;31m".dt('Error:')." \033[0m".dt('Unable to delete site!'));
    }
    else {
      drush_print(dt('Website successfully deleted.'));
    }
  }
}

/**
 * Status page callback
 */
function drush_prod_monitor_statusdetail() {
  $args = func_get_args();

  if (empty($args)) {
    _drush_prod_monitor_overview();
  }
  else if (is_numeric($args[0])) {
    _drush_prod_monitor_detail($args[0]);
  }
}

function _drush_prod_monitor_overview() {
  $sites = _prod_monitor_get_sites();

  // Map error codes to shell colours.
  $severity = array (
    'ok' => '1;32',
    'warning' => '1;33',
    'error' => '1;31',
  );

  $rows = array(array(
    dt('URL'),
    dt('ID'),
    dt('Data'),
    dt('Status'),
    dt('Date added'),
    dt('Last update'),
  ));
  
  // TODO: check why the colour coding messes up the tabs for the table
  foreach ($sites as $id => $site_info) {
    $rows[] = array(
      rtrim($site_info['url'], '/'),
      $id,
      (!$site_info['data']) ? dt('Not yet retrieved') : t('Stored'),
      "\033[".$severity[$site_info['status']].'m'.ucwords($site_info['status'])."\033[0m",
      $site_info['added'],
      (!$site_info['lastupdate']) ? dt('Not yet updated') : $site_info['lastupdate'],
    );
  }
  drush_print("\033[1m".dt('Production Monitor status')."\033[0m\n", 1);
  drush_print_table($rows, TRUE);
  drush_print(dt('Use drush prod-monitor [id] to view the details of a specific site.'));
}

function _drush_prod_monitor_detail($id) {
  $site = _prod_monitor_get_site($id, TRUE);
  if (!isset($site['url'])) {
    drush_print(dt('No site found with ID').' '.$id.'!');
    return;
  }
  $functions = $site['settings']['functions'];
  $nodata = dt('No data recieved yet.');
  $url = rtrim($site['url'], '/');
  // Map error codes to shell colours.
  $severity = array (
    REQUIREMENT_INFO => '1',
    REQUIREMENT_OK => '1;32',
    REQUIREMENT_WARNING => '1;33',
    REQUIREMENT_ERROR => '1;31',
  );
  $error = 0;

  foreach ($functions as $set => $data) {
    if (isset($site['data'][$set])) {
      $rows[] = array('');
      $rows[] = array("\033[1m".dt($data['title'])."\033[0m");
      if (!empty($site['data'][$set])) {
        foreach ($site['data'][$set] as $check => $result) {
          $rows[] = array(
            $result['title'],
            "\033[".$severity[$result['severity']].'m'.strip_tags($result['value'])."\033[0m",
          );
          if ($error < $result['severity']) {
            $error = $result['severity'];
          }
        }
      }
      else {
        $rows[] = array($nodata);
      }
    }
  }
  drush_print("\033[1m".dt('Production Check status for').' '.$url."\033[0m", 1);
  drush_print_table($rows);
  if ($error > 0) {
    // Would be cool if we could prefix the admin path with http://<host>/ so it
    // will become a clickable link in some terminals. Any deas?
    drush_print("\033[1m".dt('Some errors were reported!')."\033[0m ".dt('Check the full status page on')." \033[1m".'admin/reports/prod-monitor/'.$id.'/view'."\033[0m ".dt('for details.'));
  }
}
