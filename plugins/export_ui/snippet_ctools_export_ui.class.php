<?php
/**
 * 
 */
class snippet_ctools_export_ui extends ctools_export_ui {
  
  /**
   * 
   * @param $form
   * @param $form_state
   */
  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);
    if ($form_state['form type'] == 'clone'){
      $default_snippet = $this->load_item($form_state['original name']);
    }
    else {
      $default_snippet = $form_state['item'];
    }

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#description' => t('Title for the textarea-exportible.'),
      '#default_value' => $default_snippet->title,
      '#required' => true,
      );
  }
  
  /**
   * 
   * @param $form
   * @param $form_state
   */
  function edit_form_validate(&$form, &$form_state) {
    $op = $form_state['op'];
    switch ($op) {
      case 'add':
        //Check if name already exists
        ctools_include('export');
        $preset = ctools_export_crud_load($form_state['plugin']['schema'], $form_state['values']['name']);
        if($preset) {
          form_set_error('name', 'snippet already exists, snippet names have to be unique.');
        }
        break;
    }
  }
  
  /**
   * 
   * @param $item_name
   */
  function load_item($item_name) {
    $snippet = ctools_export_crud_load($this->plugin['schema'], $item_name);
    return $snippet;
  }
}
