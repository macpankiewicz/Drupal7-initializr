<?php

/**
 * @file Drush integration for the prepro module.
 */

/**
 * Implements hook_drush_command().
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function prepro_drush_command() {
  $items = array();
 
  $items['prepro'] = array(
    'description' => "Immediately processes a list of JS or CSS files according to the current prepro configuration.",
    'arguments' => array(
      'filepath1' => 'Path to file.',
      'filepath2' => 'Path to file.',
      '...' => 'Path to file.',
      'filepathN' => 'Path to file.',
    ),
    'drupal dependencies' => array('prepro'),
    'aliases' => array('ppr'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function prepro_drush_help($section) {
  switch ($section) {
    case 'drush:prepro':
      return dt("Immediately processes a list of JS or CSS files according to the current prepro configuration.");
      break;
  }
}

/**
 * Immediately processes a list of JS or CSS files according to the current
 * prepro configuration.
 *
 * NOTE: This command is designed to be used with file-watcher programs, e.g.
 * guard and guard-drush.
 */
function drush_prepro() {
  $files = func_get_args();

  // Convert filepaths to full paths, and remove those which do not exist.
  $files = array_filter(array_map('realpath', $files));
  if (empty($files)) {
    drush_log(dt('No valid files were found to process. Check permissions.'), 'error');
    return FALSE;
  }

  // Convert filepaths so they are relative to the Drupal root
  $basepath = realpath(DRUPAL_ROOT);
  $files = array_map('ltrim', preg_replace('%^' . preg_quote($basepath, '%') . '%', '', $files), array_fill(0, count($files), '/'));

  // Split files out into css and js types - we need to do our own matching
  // of extensions to work out which is what type.
  $items = array();
	$filetypes = _prepro_get_types();
  foreach ($files as $filepath) {

		preg_match('/([^\/]+?)\.([^\.\/]+)$/', $filepath, $match);
		$basename  = $name = $match[1]; # path/to/lol.sass => lol
		$extension = $ext = $match[2]; # path/to/lol.sass => sass

    foreach (array('css', 'js') as $target_ext) {

      if (isset($filetypes[$target_ext][$extension])) {
        $items[$target_ext][] = array(
          'type' => 'file',
          'data' => $filepath,
          // This forces prepro to recompile it immediately.
          'recompile' => TRUE,
        );
      }

    }
  }

  // No files were found that matched prepro configuration.
  if (!count(array_filter($items))) {
    drush_log(dt('No files were matched by current prepro configuration.'), 'warning');
    return FALSE;
  }

  // Process files!
  foreach (array('css', 'js') as $target_ext) {
    if (!empty($items[$target_ext])) {
      _prepro_process_assets($target_ext, $items[$target_ext]);
    }
  }

  return TRUE;
}
