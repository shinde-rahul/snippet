<?php
/**
 * @file
 * snippet_ctools_export_ui.inc
 *
 * Define this Export UI plugin
 */
$plugin = array(
  'schema' => 'snippet',  // As defined in hook_schema().
  'access' => 'administer snippet',  // Define a permission users must have to access these pages.
  
  'export' => array(
    'admin_title' => "admin_title",
  ),

  // Define the menu item.
  'menu' => array(
    'menu item' => 'snippet',
    'menu title' => 'Snippet: Add',
    'menu description' => 'Create a snippet that will be used as a pane.',
    
  ),

  // Define user interface texts.
  'title singular' => t('preset'),
  'title plural' => t('presets'),
  'title singular proper' => t('snippet preset'),
  'title plural proper' => t('snippet presets'),
  
  'handler' => 'snippet_ctools_export_ui',
);

