<?php
// $Id$

/**
 * Build status page.
 */
function prod_check_status() {
  drupal_set_title(t('Production check status'));

  $output = '';

  // Execute all functions per set as defined in the functions array in
  // _prod_check_functions().
  foreach (_prod_check_functions() as $set => $data) {
    $result = array();
    foreach ($data['functions'] as $function => $title) {
      $func = '_prod_check_' . $function;
      // $func() will execute the function with the name we composed above.
      $result = array_merge($result, $func());
    }
    $output .= '<h2>'.t($data['title']).'</h2>'."\n";
    $output .= '<div class="description"><p><em>'.t($data['description']).'</em></p></div>'."\n";
    $output .= theme('status_report', $result);
  }

  return $output;
}

/**
 * Build settings form.
 */
function prod_check_settings_form(&$form_state) {
  drupal_set_title(t('Production check settings'));
  $base = drupal_get_path('module', 'prod_check');
  drupal_add_css($base.'/css/prod-check.css');
  drupal_add_js($base.'/js/jquery.equalheights.js', 'module', 'header');
  drupal_add_js($base.'/js/prod-check.js', 'module', 'header');

  $form = array();

  // XMLRPC settings.
  $form['prod_check_enable_xmlrpc'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable XMLRPC API'),
    '#default_value' => variable_get('prod_check_enable_xmlrpc', 0),
    '#description' => t('Tick this box if you would like to the module to open up the XMLRPC api so that it can be queried externally to supply information to a base site for monitoring purposes.'),
    '#required' => FALSE,
  );

  $form['prod_check_xmlrpc_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API key'),
    '#default_value' => variable_get('prod_check_xmlrpc_key', ''),
    '#maxlength' => 128,
    '#size' => 60,
    '#description' => t('Enter a key here to ensure secure transfer of data over the API. Use a mixture of alphanumeric and special characters for increased security.'),
    '#required' => FALSE,
  );

  // Nagios settings.
  $form['prod_check_enable_nagios'] = array(
    '#type' => 'checkbox',
    '#title' =>   t('Enable Nagios integration'),
    '#description' => t('Tick this box if you want to enable integration with Nagios. The !link module is required for this to function.', array('!link' => l(t('Nagios'), 'http://drupal.org/project/nagios', array('attributes' => array('title' => t('Nagios')))))),
    '#default_value' => variable_get('prod_check_enable_nagios', 0),
    '#ahah' => array(
      'path' => 'admin/settings/prod-check/nagios',
      'wrapper' => 'prod-check-nagios',
      'effect' => 'fade',
    ),
    '#required' => FALSE,
  );

  // The #value here is necessary for the markup field to be rendered :-(
  $form['prod_check']['nagios'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="prod-check-nagios">',
    '#value' => '&nbsp;',
    '#suffix' => '</div>',
  );
  // Only show when the checkbox above is selected.
  // TODO: the OR part here is not ok, when disabling the checkbox, the variable_get will still return 1
  if ($form_state['values']['prod_check_enable_nagios'] == 1 || variable_get('prod_check_enable_nagios', 0)) {
    $form['prod_check']['nagios']['settings'] = _prod_check_functions_as_form();
    $options = variable_get('prod_check_nagios_checks', array());
    if (!empty($options)) {
      // Just to increase readability of the source here.
      $monitor_settings = &$form['prod_check']['nagios']['settings']['prod_check_settings']['monitor_settings'];
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
  }

  // Submit buttons.
  // Markup field for proper styling.
  $form['buttons'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="buttons">',
    '#value' => '&nbsp;',
    '#suffix' => '</div>',
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['buttons']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
  );

  return $form;
}

/**
 * Callback to add custom text field.
 */
function prod_check_enable_nagios() {
  $form = prod_check_ahah_helper();

  $nagios = $form['prod_check']['nagios'];
  // Remove the wrapper so we don't double it up.
  unset($nagios['#prefix'], $nagios['#suffix']);

  $output = drupal_render($nagios);

  // Final rendering callback.
  print drupal_json(array('status' => TRUE, 'data' => $output));
  exit();
}

/**
 * Does the very standard things that must be done in any normal callback.
 */
function prod_check_ahah_helper() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  return $form;
}

/**
 * Validation for settings form.
 */
function prod_check_settings_form_validate($form, &$form_state) {
  if (!empty($form_state['ahah_submission'])) {
    return;
  }

  if ($form_state['values']['prod_check_enable_xmlrpc']) {
    if (empty($form_state['values']['prod_check_xmlrpc_key'])) {
      form_set_error('prod_check_xmlrpc_key', t('When enabling the XPLRPC API, you <strong>must</strong> enter an API key!'));
    }
  }

  if ($form_state['values']['prod_check_enable_nagios']) {
    $checks = array();
    foreach ($form_state['values']['monitor_settings'] as $set => $data) {
      foreach ($data as $check => $value) {
        if ($value) {
          $checks[$set][] = $value;
        }
      }
    }
    if (empty($checks)) {
      form_set_error('monitor_settings', t('When enabling Nagios support, you <strong>must</strong> tick at least one of the checkboxes!'));
    }
  }
}

/**
 * Submit for settings form.
 */
function prod_check_settings_form_submit($form, &$form_state) {
  if (!empty($form_state['ahah_submission'])) {
    return;
  }
  switch ($form_state['values']['op']) {
    case t('Save configuration'):
      if ($form_state['values']['prod_check_enable_xmlrpc']) {
        variable_set('prod_check_enable_xmlrpc', $form_state['values']['prod_check_enable_xmlrpc']);
        variable_set('prod_check_xmlrpc_key', $form_state['values']['prod_check_xmlrpc_key']);
      }
      if ($form_state['values']['prod_check_enable_nagios']) {
        $checks = array();
        foreach ($form_state['values']['monitor_settings'] as $set => $data) {
          foreach ($data as $check => $value) {
            if ($value) {
              $checks[$set][] = $value;
            }
          }
        }
        variable_set('prod_check_enable_nagios', $form_state['values']['prod_check_enable_nagios']);
        variable_set('prod_check_nagios_checks', $checks);
      }
      break;
    case t('Reset to defaults'):
      variable_del('prod_check_enable_xmlrpc');
      variable_del('prod_check_xmlrpc_key');
      variable_del('prod_check_enable_nagios');
      variable_del('prod_check_nagios_checks');
      break;
  }
}

function prod_check_apc() {
  include(drupal_get_path('module', 'prod_check').'/includes/prod_check.apc.inc');
  exit;
}
