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
//    '#description' => t('Please select one of the snippet <br /> or simply select "Add new snippet" to add new snippet.'),
    '#description' => t('Please select one of the snippet.')
  );
  return $form;
}

/**
 * Check for the entered is a duplicate or not
 * If name is already exist then set an error
 * @param $element
 * @param $form_state
 * @param $form
 */
function _snippet_duplicate_check($element, &$form_state, $form) {
  ctools_include('export');
    $preset = ctools_export_crud_load('snippet', $form_state['values']['machine_name']);
    if ($preset) {
      form_set_error('name', 'Snippet already exists, snippet names have to be unique.');
    }
}

/**
 * The submit form handler stores the data in $conf.
 */
function snippet_content_type_edit_form_submit(&$form, &$form_state) {
  $form_state['conf']['snippet'] = $form_state['values']['snippet'];
}

/**
 * 
 * @param $subtype
 * @param $conf
 * @param $panel_args
 * @param $context
 */
function snippet_content_type_render($subtype, $conf, $panel_args, &$context) {
  $snippet = ctools_export_crud_load('snippet', $conf['snippet']);
  $snippet_revision = snippet_load($conf['snippet']);
  $variable['rid'] = $snippet->rid;
  $variable['name'] = $snippet->name;
  $variable['title'] = ($conf['override_title']) ? $conf['override_title_text'] : $snippet->title;
  $variable['content'] = $snippet_revision->content;
  
  $block = new stdClass();
  $block->content = theme('snippet_content_show', $variable);
  return $block;
}
