<?php
// http://compass-style.org/reference/compass/helpers/font-files/#font-files
function sassy_compass__font_files() {
  $args = func_get_args();

  foreach ($args as $k=>$v) {
    if (is_object($v)) {
      $args->$k = $v->value;
    }
  }

  $output = array();
  while (count($args)) {
    $url = array_shift($args);
    $format = array_shift($args);
    $output[] = "font_url($url) format($format)";
  }

  return new SassString(implode(', ', $output));
}