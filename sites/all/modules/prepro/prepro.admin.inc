<?php

/**
 * Callback for Prepro settings page - compiles a table of filetype-specific settings
 * and any preprocessor-specific settings available.
 */
function prepro_admin_form($form_id, &$form_state) {
  $types = _prepro_get_types();
  $settings = _prepro_get_settings();
  $selected = array(); # keep track of chosen preprocs for deciding wether a fieldset is open by default
  $rows = array();
  foreach ($types as $format => $processors) {
    $proc_list = array();
    foreach ($processors as $name => $proc) {
      $proc_list[$name] = $proc['label'];
    }
    $settings['filetypes'] += array($format => array());
    $settings['filetypes'][$format] += array(
      'preprocessor' => current(array_keys($proc_list)),
      'cache' => 'onload',
      'cache_location' => 'public://prepro/'
    );
    $selected[] = $settings['filetypes'][$format]['preprocessor'];
    $rows[] = array(
      'name' => '.' . $format,
      'processor' => array('data' => array(
        '#name' => 'filetypes[' . $format . '][preprocessor]',
        '#type' => 'select',
        '#options' => $proc_list,
        '#value' => $settings['filetypes'][$format]['preprocessor'],
      )),
      'cache' => array('data' => array(
        '#name' => 'filetypes[' . $format . '][cache]',
        '#type' => 'select',
        '#options' => array(
          'cache' => 'Cached, reprocessed on cache-clear',
          'onload' => 'Uncached, reprocessed on every page load',
        ),
        '#value' => $settings['filetypes'][$format]['cache'],
      )),
      'cache_location' => array('data' => array(
        '#name' => 'filetypes[' . $format . '][cache_location]',
        '#type' => 'textfield',
        '#value' => $settings['filetypes'][$format]['cache_location']
      )),
    );
  }

  $form = array();
  $form['filetypes'] = array(
    '#theme' => 'table',
    '#header' => array('Format', 'Pre-processor', 'Caching', 'Cache Location'),
    '#rows' => $rows,
    '#empty' => 'There are no modules offering to preprocess JS/CSS files'
  );

  foreach (module_implements('preprocessor_settings_form') as $module) {
    $procs = module_invoke($module, 'preprocessor_info');
    $form[$module] = array();
    foreach ($procs as $proc => $info) {
      # use the "retrieve" and "prepare" funcs to get the form and allow the usual form handler functions
      $form_id = $module . '_preprocessor_settings_form';
      $settings['additional'] += array($proc => array());
      $form_state = array('prepro' => array('local' => $settings['additional'][$proc], 'global' => $settings, 'processor' => $proc)) + form_state_defaults();
      $result = drupal_retrieve_form($form_id, $form_state);
      // drupal_prepare_form($form_id, $result, $form_state);
      // if we got the form, alter the form element names to make saving v simple.
      if ($result) {
        _prepro_admin_form_alter_name($result, $proc);
      } else {
        // otherwise, remove the compiler from the "selected" list
        foreach ($selected as $i => $v) {
          if ($v == $proc) {
            unset ($selected[$i]);
          }
        }
        // and change the label a wee bit.
        $info['label'] .= ' (no settings)';
        $result = array();//
      }

      // add the result to the page.
      $form[$module][$proc] = array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => !in_array($proc, $selected),
        '#title' => $info['label'],
        '#name' => $proc,
        'fields' => $result
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Settings',
  );

  return $form;
}

/**
 * Helper: alters the #name attribute of all inputs to make them specific
 * to the preprocessor (for example 'hello' to 'additional[sassy][hello]')
 */
function _prepro_admin_form_alter_name(&$array, $proc) {
  foreach ($array as $key => $value) {
    if ($key{0} != '#') {
      if (!isset($value['#name'])) {
        $value['#name'] = 'additional[' . $proc . '][' . $key . ']';
      }
      else {
        $value['#name'] = preg_replace('/^([^\[]+)(\[.+)?/', 'additional[' . $proc . '][$1]$2', $value['#name']);
      }
      if (is_array($value)) {
        _prepro_admin_form_alter_name($value, $proc);
      }
    }
    $array[$key] = $value;
  }
}

/**
 * Submit handler for prepro admin form.
 */
function prepro_admin_form_submit($form, $form_state) {
  $settings = _prepro_get_settings(array(
    'additional' => $form_state['input']['additional'],
    'filetypes' => $form_state['input']['filetypes']
  ));
  foreach ($settings['filetypes'] as $ext => $type) {
    if (!file_prepare_directory($type['cache_location'], FILE_MODIFY_PERMISSIONS ^ FILE_CREATE_DIRECTORY)) {
      drupal_set_message(t('The cache location for :ext files (:dir) can either not be created or is not writable.', array(':ext' => $ext, ':dir' => $type['cache_location'])), 'warning');
    }
  }
  variable_set('prepro', $settings);
  drupal_set_message('Prepro settings have been saved');
}
