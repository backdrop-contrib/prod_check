<?php

/**
 * @file
 * Simple database connection check that can be placed anywhere within a Backdrop
 * installation. Does NOT need to be in the root where index.php resides!
 */

/**
 * Locate the actual Backdrop root. Based on drush_locate_root().
 */
function locate_root() {
  $drupal_root = FALSE;

  $start_path = isset($_SERVER['PWD']) ? $_SERVER['PWD'] : '';
  if (empty($start_path)) {
    $start_path = getcwd();
  }

  foreach (array(TRUE, FALSE) as $follow_symlinks) {
    $path = $start_path;
    if ($follow_symlinks && is_link($path)) {
      $path = realpath($path);
    }
    // Check the start path.
    if (valid_root($path)) {
      $drupal_root = $path;
      break;
    }
    else {
      // Move up dir by dir and check each.
      while ($path = shift_path_up($path)) {
        if ($follow_symlinks && is_link($path)) {
          $path = realpath($path);
        }
        if (valid_root($path)) {
          $drupal_root = $path;
          break 2;
        }
      }
    }
  }

  return $drupal_root;
}

/**
 * Based on the BackdropBoot*::valid_root() from Drush.
 */
function valid_root($path) {
  if (!empty($path) && is_dir($path) && file_exists($path . '/index.php')) {
    $candidate = 'core/includes/common.inc';
    if (file_exists($path . '/' . $candidate) && file_exists($path . 'core/misc/backdrop.js')) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Based on _drush_shift_path_up().
 */
function shift_path_up($path) {
  if (empty($path)) {
    return FALSE;
  }
  $path = explode('/', $path);
  // Move one directory up.
  array_pop($path);
  return implode('/', $path);
}

/**
 * Do the actual database connection check.
 */
define('BACKDROP_ROOT', locate_root());
require_once BACKDROP_ROOT . '/care/includes/bootstrap.inc';
backdrop_bootstrap(DRUPAL_BOOTSTRAP_DATABASE);

$result = db_query('SELECT COUNT(filename) FROM {system}')->fetchField();

if ($result) {
  $msg = 'OK';
  http_response_code(200);
}
else {
  http_response_code(500);
  $msg = 'NOK';
}

// Prevent caching of this page!
header("Cache-Control: no-cache, no-store, must-revalidate"); // HTTP 1.1.
header("Pragma: no-cache"); // HTTP 1.0.
header("Expires: 0"); // Proxies.

exit($msg);
