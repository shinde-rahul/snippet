<?php
/**
 * @file
 * Snippets
 *
 * Content type enable us to place the snippet as pane
 */
/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Snippet'),
  'description' => t('Snippet.'),

  // 'single' => TRUE means has no subtypes.
  'single' => TRUE,
  // Constructor.
  'content_types' => array('Snippet'),
  // Name of a function which will render the block.
  'render callback' => 'snippet_content_type_render',
  // The default context.
  'defaults' => array(),
  // This explicitly declares the config form. Without this line, the func would be
  // ctools_plugin_example_no_context_content_type_edit_form.
  'edit form' => 'snippet_content_type_edit_form',

  // Icon goes in the directory with the content type.
  'icon' => 'snippet_icon.png',
  'category' => array(t('Snippet'), -9),

);

/**
 * 'Edit form' callback for the content type.
 */
function snippet_content_type_edit_form($form, &$form_state) {
  // get the form state i.e. configuration values to load the default values
  $conf = $form_state['conf'];

  // Include the ctools helper 'export'
  ctools_include('export');

  // Loads all the 'snippet' those are there in database
  $options = ctools_export_crud_load_all('snippet');

  // Create an option array
  $snippet_options = array();
  foreach ($options as $key => $value) {
    $snippet_options[$key] = $value->title;
  }

  if (!count($snippet_options)) {
    drupal_set_message(t('There is no "Snippets" available.'), 'error');
  }

  // Form element : Select snippet
  $form['snippet'] = array(
    '#type' => 'select',
    '#title' => t('Snippets'),
    '#default_value' => $conf['snippet'],
    '#options' => $snippet_options,
    '#description' => t('Please select one of the snippet.')
  );

  $form['hide_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide snippet\'s title'),
    '#default_value' => $conf['hide_title'],
    '#description' => t('If checked, title will be hide.')
  );

  return $form;
}

/**
 * The submit form handler stores the data in $conf.
 */
function snippet_content_type_edit_form_submit(&$form, &$form_state) {
  $form_state['conf']['snippet'] = $form_state['values']['snippet'];
  $form_state['conf']['hide_title'] = $form_state['values']['hide_title'];
}

/**
 *
 * @param $subtype
 * @param $conf
 * @param $panel_args
 * @param $context
 */
function snippet_content_type_render($subtype, $conf, $panel_args, &$context) {
  // get the snippet
  $snippet = ctools_export_crud_load('snippet', $conf['snippet']);
  //get the revised data
  $snippet_revision = snippet_load($conf['snippet']);

  // prepare array for theme
  $variable['rid'] = $snippet_revision->rid;
  $variable['name'] = $snippet->name;
  $title = "";
  if(!$conf['hide_title']) {
    $override_title_text = trim($conf['override_title_text']);

    if (!$conf['override_title']) {
      $title = ($snippet_revision->rid) ? $snippet_revision->title_revision  : $snippet->title;
    }
    elseif (!empty($override_title_text)) {
      $title = $override_title_text;
    }
  }
  $variable['title'] = $title;// ($conf['override_title']) ? $conf['override_title_text'] : $snippet->title;
  $variable['content'] = $snippet_revision->content;

  $block = new stdClass();
  $block->content = theme('snippet_content_show', $variable);
  return $block;
}
