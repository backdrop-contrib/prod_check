<?php
// $Id$

/**
 * Build status page.
 */
function prod_monitor_status($id) {
  $site = _prod_monitor_get_site($id, TRUE);
  drupal_set_title(t('Production monitor status for') .' '. $site['url']);

  $functions = $site['settings']['functions'];
  $nodata = t('No data recieved yet.');

  $output = '';

  // Execute all functions per set as defined in the functions array.
  foreach ($functions as $set => $data) {
    if (isset($site['data'][$set])) {
      $output .= '<h2>'.t($data['title']).'</h2>'."\n";
      $output .= '<div class="description"><p><em>'.t($data['description']).'</em></p></div>'."\n";
      if (!empty($site['data'][$set])) {
        $output .= theme('status_report', $site['data'][$set]);
      }
      else  {
        $output .= '<p>'.$nodata.'</p><p>&nbsp;</p>';
      }
    }
  }

  if (empty($output)) {
    $output = '<p>'.$nodata.'</p><p>&nbsp;</p>';
  }

  $output .= drupal_get_form('_prod_monitor_update_data_form', $id, $site);

  return $output;
}

/**
 * Build settings form.
 */
function prod_monitor_overview_form(&$form_state, $edit = FALSE) {
  drupal_set_title(t('Production monitor settings'));
  $base = drupal_get_path('module', 'prod_monitor');
  drupal_add_css($base.'/css/prod-monitor.css');
  drupal_add_js($base.'/js/jquery.equalheights.js', 'module', 'header');
  drupal_add_js($base.'/js/prod-monitor.js', 'module', 'header');

  $form = array();

  $collapsed = FALSE;

  if (!$edit) {
    // Add new site situation.
    $sites = _prod_monitor_get_sites();
    $api_key = $url = '';
    $options = array();
    $button = t('Get settings');
    if (!empty($sites)) {
      $form['table'] = array(
        '#type' => 'markup',
        '#value' => _prod_monitor_overview_form_table($sites),
      );
      $collapsed = TRUE;
    }
    if (!empty($form_state['storage']['get_settings'])) {
      // Second step of add new site situation.
      $api_key = $form_state['values']['api_key'];
      $url = $form_state['values']['url'];
      $button = t('Add site');
      $collapsed = FALSE;
      $msg = TRUE;
    }
  }
  else {
    // Edit site situation.
    $site = _prod_monitor_get_site($edit);
    $url = $site['url'];
    $api_key = $site['settings']['api_key'];
    $options = $site['settings']['checks'];
    $button = t('Save site');
    $msg = FALSE;
  }

  // Add/edit form.
  $form['sites'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a site to monitor'),
    '#description' => t('You can add sites here that you wish to monitor.'),
    '#collapsible' => TRUE,
    '#collapsed' => $collapsed,
  );

  $form['sites']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('Website URL'),
    '#default_value' => $url,
    '#description' => t('Enter the <strong>full</strong> url of the website to be monitored. This site must be running the <em>Production check</em> module. This <strong>must</strong> include a protocol like <em>http://</em> or <em>https://</em>!'),
    '#size' => 60,
    '#required' => TRUE,
  );

  $form['sites']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('The website\'s API key'),
    '#default_value' => $api_key,
    '#description' => t('Enter the API key you have configured for this site using the <em>Production check</em> module.'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
  );

  // Only show on second step of add form or when editing.
  if (!empty($form_state['storage']['get_settings']) || $edit) {
    // Get the settings from the remote site. We always do this when the form is
    // displayed and don't store this locally. Logic here is that you won't be
    // editing these settings all that much.
    $functions = _prod_monitor_retrieve_functions($url, $api_key, $msg);
    // Parse the array of functions into a form.
    if ($functions) {
      // Save data to store in DB on submit.
      $form_state['storage']['functions'] = $functions;
      // Parse functions into form data
      $form['sites']['prod_check_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Configure what data you wish to monitor for this site.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );

      $form['sites']['prod_check_settings']['monitor_settings'] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="prod-check-settings">',
        '#suffix' => '</div>',
        '#tree' => TRUE,
      );

      $i = 1;
      foreach ($functions as $set => $data) {
        $rest = $i%2;
        $form['sites']['prod_check_settings']['monitor_settings'][$set] = array(
          '#type' => 'checkboxes',
          '#title' => t($data['title']),
          '#description' => t($data['description']),
          '#options' => $data['functions'],
          '#default_value' => array_keys($data['functions']),
          '#prefix' => '<div class="prod-check-settings '.(($rest) ? 'odd' : 'even').'">',
          '#suffix' => '</div>',
        );
        $i++;
      }
      if ($edit) {
        // Just to increase readability of the source here.
        $monitor_settings = &$form['sites']['prod_check_settings']['monitor_settings'];
        // Set default values to last saved state
        foreach (element_children($monitor_settings) as $set) {
          if (isset($options[$set])) {
            $monitor_settings[$set]['#default_value'] = $options[$set];
          }
          else {
            // No settings available, so uncheck all.
            $monitor_settings[$set]['#default_value'] = array();
          }
        }
      }
      $form['sites']['fetch'] = array(
        '#type' => 'checkbox',
        '#title' => t('Fetch data immediately'),
        '#default_value' => 0,
        '#description' => t('Will attempt to fetch the data immediately when the site has been added.'),
      );
    }
    else {
      // Error, so show retry button.
      $button = t('Retry');
    }
  }

  if ($edit) {
    $form['site_id'] = array(
      '#type' => 'value',
      '#value' => $edit,
    );
  }

  $form['sites']['submit'] = array(
    '#type' => 'submit',
    '#value' => $button,
  );

  return $form;
}

/**
 * Helper function to theme all sites into a table
 */
function _prod_monitor_overview_form_table($sites) {
  $home = array('destination' => '/admin/reports/prod-monitor');

  // Set headers.
  $headers = array(
    t('URL'),
    t('Data'),
    t('Status'),
    t('Date added'),
    t('Last update'),
    array('data' => t('Actions'), 'colspan' => 5),
  );

  // Compose rows.
  $rows = array();
  foreach ($sites as $id => $site_info) {
    // Set view and flush links.
    $view = t('View');
    $flush = t('Flush');
    if ($site_info['data']) {
      $view = l(t('View'), 'admin/reports/prod-monitor/'.$id.'/view', array('attributes' => array('title' => t('View'))));
      $flush = l(t('Flush'), 'admin/reports/prod-monitor/'.$id.'/flush', array('query' => $home, 'attributes' => array('title' => t('Flush'))));
    }

    if (!empty($site_info['status'])) {
      $status = array('data' => '<strong>'.t(ucwords($site_info['status'])).'</strong>', 'class' => $site_info['status']);
    }
    else {
      $status = '';
    }

    // Actually compose the rows.
    $row = array(
      'data' => array(
        $site_info['url'],
        (!$site_info['data']) ? t('Not yet retrieved.') : t('Stored.'),
        $status,
        $site_info['added'],
        (!$site_info['lastupdate']) ? t('Not yet updated.') : $site_info['lastupdate'],
        /* Compose links. */
        $view,
        l(t('Edit'), 'admin/reports/prod-monitor/'.$id.'/edit', array('query' => $home, 'attributes' => array('title' => t('Edit')))),
        l(t('Fetch data'), 'admin/reports/prod-monitor/'.$id.'/fetch', array('attributes' => array('title' => t('Fetch & View')))),
        $flush,
        l(t('Delete'), 'admin/reports/prod-monitor/'.$id.'/delete', array('query' => $home, 'attributes' => array('title' => t('Delete')))),
      ),
      'class' => $site_info['status'],
    );
    $rows[] = $row;
  }

  return theme('table', $headers, $rows);
}

/**
 * Validation function
 */
function prod_monitor_overview_form_validate($form, &$form_state) {
  if (!preg_match('/^https?:\/\/.*/i', $form_state['values']['url'])) {
    form_set_error('url', t('The url must start with a valid protocol: either http:// or https://'));
  }
}

/**
 * Submit function
 */
function prod_monitor_overview_form_submit($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t('Get settings'):
    case t('Retry'):
        // Make sure the storage is not empty so we go to step 2
        $form_state['storage']['get_settings'] = TRUE;
        $form_state['rebuild'] = TRUE;
      break;

    case t('Add site'):
    case t('Save site'):
      // Prevent from ending on step 2 again.
      unset($form_state['storage']['get_settings']);
      $site = new stdClass();

      // Edit situation, so force an update.
      if (isset($form_state['values']['site_id']) && is_numeric($form_state['values']['site_id'])) {
        $update = array('id');
        $site->id = $form_state['values']['site_id'];
      }
      else {
        // Add situation, insert.
        $update = NULL;
        $site->added = time();
      }

      $site->url = $form_state['values']['url'];
      $checks = array();
      foreach ($form_state['values']['monitor_settings'] as $set => $data) {
        foreach ($data as $check => $value) {
          if ($value) {
            $checks[$set][] = $value;
          }
        }
      }
      $site->settings = serialize(
        array(
          'api_key' => $form_state['values']['api_key'],
          'functions' => $form_state['storage']['functions'],
          'checks' => $checks,
        )
      );

      $result = drupal_write_record('prod_monitor_sites', $site, $update);

      if ($result) {
        drupal_set_message(t('Website %url correctly saved.', array('%url' => $site->url)));
        if ($form_state['values']['fetch']) {
          $site_info = _prod_monitor_get_site($site->id, TRUE);
          _prod_monitor_retrieve_data($site->id, $site_info, TRUE);
          drupal_goto('admin/reports/prod-monitor/'.$site->id.'/view');
        }
      }
      else {
        drupal_set_message(t('Website %url not saved! Please try again.', array('%url' => $site->url)), 'error');
      }
      break;
  }
}

/**
 * Callback to fetch site data
 */
function prod_monitor_fetch_data($id) {
  $site_info = _prod_monitor_get_site($id, TRUE);
  _prod_monitor_retrieve_data($id, $site_info, TRUE);
  drupal_goto('admin/reports/prod-monitor/'.$id.'/view');
}

/**
 * Form to delete a site's data
 */
function prod_monitor_flush_form(&$form_state, $id) {
  $form = array();

  $form['site_id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $site = _prod_monitor_get_site($id);

  $form['url'] = array(
    '#type' => 'value',
    '#value' => $site['url'],
  );

  return confirm_form($form, t('Are you sure you wish to delete all fetched data for %url?', array('%url' => $site['url'])), 'admin/reports/prod-monitor');
}

/**
 * Form to delete a feature's recorded data
 */
function prod_monitor_flush_form_submit($form, &$form_state) {
  $id = $form_state['values']['site_id'];
  $url = $form_state['values']['url'];

  if ($id) {
    $result = _prod_monitor_flush_data($id);
    if ($result === FALSE) {
      drupal_set_message(t('Unable to flush data for %url!', array('%url' => $url)), 'error');
    }
    else {
      drupal_set_message(t('Stored data for %url successfully flushed.', array('%url' => $url)));
      drupal_goto('admin/reports/prod-monitor');
    }
  }
}

/**
 * Form to delete a feature
 */
function prod_monitor_delete_form(&$form_state, $id) {
  $form = array();

  $form['site_id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $site = _prod_monitor_get_site($id);

  $form['url'] = array(
    '#type' => 'value',
    '#value' => $site['url'],
  );

  return confirm_form($form, t('Are you sure you wish to delete the website %url?', array('%url' => $site['url'])), 'admin/reports/prod-monitor');
}

/**
 * Delete a feature
 */
function prod_monitor_delete_form_submit($form, &$form_state) {
  $id = $form_state['values']['site_id'];
  $url = $form_state['values']['url'];

  if ($id) {
    $result = _prod_monitor_delete_site($id);
    if ($result) {
      drupal_set_message(t('Website %url successfully deleted.', array('%url' => $url)));
      drupal_goto('admin/reports/prod-monitor');
    }
    else {
      drupal_set_message(t('Unable to delete %url!', array('%url' => $url)), 'error');
    }
  }
}

function _prod_monitor_update_data_form($form_state, $id, $site_info) {
  $form['site_id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $form['site_info'] = array(
    '#type' => 'value',
    '#value' => $site_info,
  );

  // Markup field for proper styling.
  $form['buttons'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="buttons">',
    '#value' => '&nbsp;',
    '#suffix' => '</div>',
  );

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Fetch data now'),
  );

  return $form;
}

function _prod_monitor_update_data_form_submit($form, &$form_state) {
  _prod_monitor_retrieve_data($form_state['values']['site_id'], $form_state['values']['site_info'], TRUE);
}
