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
    // this is to show the preview
    $form['snippet_preview_wrapper'] = array(
      '#prefix' => '<div id="snippet_preview">',
      '#suffix' => '</div>',
      '#markup' => '',
      );

    // adding parent element
    parent::edit_form($form, $form_state);

    if ($form_state['form type'] == 'clone') {
      $default_snippet = $this->load_item($form_state['original name']);
    }
    else {
      $default_snippet = $form_state['item'];
    }

    if ($form_state['op'] == 'edit') {
      $form['info']['admin_title']['#disabled'] = TRUE;
      $form['info']['name']['#disabled'] = TRUE;
    }

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#description' => t('Title for the textarea-exportible.'),
      '#default_value' => ($default_snippet->rid) ? $default_snippet->title_revision : $default_snippet->title,
      );

    $form['content'] = array(
      '#type' => 'text_format',
      '#title' => t('Description'),
      '#description' => t('Description of this snippet.'),
      '#default_value' => $default_snippet->content,
      '#format' => $default_snippet->content_format,
    );

    $form['preview'] = array(
      '#type' => 'button',
      '#title' => t('Preview'),
      '#limit_validation_errors' => array(),
      '#value' => t('Preview'),
      '#submit' => array('snippet_build_preview'),
      '#ajax' => array(
        'callback' => 'snippet_form_build_preview_callback',
        'wrapper' => 'snippet_preview',
        ),
      );

    $form['snippet_preview_wrapper'] = array(
      '#prefix' => '<div id="snippet_preview">',
      '#suffix' => '</div>',
      '#markup' => '',
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

    $snippet_revision = db_select('snippet_revision', 'sr')
                        ->fields('sr', array())
                        ->condition('name', $item_name)
                        ->condition('is_current', 1)
                        ->execute()->fetch();

    $snippet->content = !empty($snippet_revision->content) ? $snippet_revision->content : '' ;
    $snippet->content_format = !empty($snippet_revision->content_format) ? $snippet_revision->content_format : NULL ;
    $snippet->timestamp = !empty($snippet_revision->timestamp) ? $snippet_revision->timestamp : NULL ;
    $snippet->is_current = !empty($snippet_revision->is_current) ? $snippet_revision->is_current : NULL ;
    $snippet->rid = !empty($snippet_revision->rid) ? $snippet_revision->rid : NULL ;
    $snippet->title_revision = !empty($snippet_revision->title) ? $snippet_revision->title : NULL ;

    return $snippet;
  }

  /**
   * Called to save the final product from the edit form.
   */
  function edit_save_form($form_state) {
    $item = &$form_state['item'];
    // dpm($item);
    $export_key = $this->plugin['export']['key'];

    $operation_type = $form_state['op'];

    // if snippet is being added for the first time then make entry in snippet and snippet_revison
    // table to have complete information
    if ( $operation_type == 'add') {
      $result = ctools_export_crud_save($this->plugin['schema'], $item);
      _save_snippet($form_state['values']);
    }
    elseif ( $operation_type == 'edit') {
      $result = _save_snippet($form_state['values']);
    }

    if (@$result) {
      $message = str_replace('%title', check_plain($item->{$export_key}), $this->plugin['strings']['confirmation'][$form_state['op']]['success']);
      drupal_set_message($message);
    }
    else {
      $message = str_replace('%title', check_plain($item->{$export_key}), $this->plugin['strings']['confirmation'][$form_state['op']]['fail']);
      drupal_set_message($message, 'error');
    }
  }

  /**
   *
   *
   */
  function list_form_submit(&$form, &$form_state) {
    // Filter and re-sort the pages.
    $plugin = $this->plugin;
    $schema = ctools_export_get_schema($this->plugin['schema']);

    $prefix = ctools_export_ui_plugin_base_path($plugin);

    foreach ($this->items as $name => $item) {
      // Call through to the filter and see if we're going to render this
      // row. If it returns TRUE, then this row is filtered out.
      if ($this->list_filter($form_state, $item)) {
        continue;
      }

      // Note: Creating this list seems a little clumsy, but can't think of
      // better ways to do this.
      $allowed_operations = drupal_map_assoc(array_keys($plugin['allowed operations']));
      $not_allowed_operations = array('import');

      if ($item->{$schema['export']['export type string']} == t('Normal')) {
        $not_allowed_operations[] = 'revert';
      }
      elseif ($item->{$schema['export']['export type string']} == t('Overridden')) {
        $not_allowed_operations[] = 'delete';
      }
      else {
        $not_allowed_operations[] = 'revert';
        $not_allowed_operations[] = 'delete';
      }

      $not_allowed_operations[] = empty($item->disabled) ? 'enable' : 'disable';

      foreach ($not_allowed_operations as $op) {
        // Remove the operations that are not allowed for the specific
        // exportable.
        unset($allowed_operations[$op]);
      }

      $operations = array();

      foreach ($allowed_operations as $op) {
        $operations[$op] = array(
          'title' => $plugin['allowed operations'][$op]['title'],
          'href' => ctools_export_ui_plugin_menu_path($plugin, $op, $name),
        );
        if (!empty($plugin['allowed operations'][$op]['ajax'])) {
          $operations[$op]['attributes'] = array('class' => array('use-ajax'));
        }
        if (!empty($plugin['allowed operations'][$op]['token'])) {
          $operations[$op]['query'] = array('token' => drupal_get_token($op));
        }
      }

      $operations['snippets_revisions'] = array(
        'title' => t('Revisions'),
        'href' => SNIPPET_MENU_PREFIX . '/' . $name . '/revisions',
        );

      $this->list_build_row($item, $form_state, $operations);
    }

    // Now actually sort
    if ($form_state['values']['sort'] == 'desc') {
      arsort($this->sorts);
    }
    else {
      asort($this->sorts);
    }

    // Nuke the original.
    $rows = $this->rows;
    $this->rows = array();
    // And restore.
    foreach ($this->sorts as $name => $title) {
      $this->rows[$name] = $rows[$name];
    }
  }

  /**
   * Build a row based on the item.
   *
   * By default all of the rows are placed into a table by the render
   * method, so this is building up a row suitable for theme('table').
   * This doesn't have to be true if you override both.
   */
  function list_build_row($item, &$form_state, $operations) {
    // Set up sorting
    $name = $item->{$this->plugin['export']['key']};
    $schema = ctools_export_get_schema($this->plugin['schema']);

    // Note: $item->{$schema['export']['export type string']} should have already been set up by export.inc so
    // we can use it safely.
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$name] = empty($item->disabled) . $name;
        break;
      case 'title':
        $this->sorts[$name] = $item->{$this->plugin['export']['admin_title']};
        break;
      case 'name':
        $this->sorts[$name] = $name;
        break;
      case 'storage':
        $this->sorts[$name] = $item->{$schema['export']['export type string']} . $name;
        break;
    }

    $this->rows[$name]['data'] = array();
    $this->rows[$name]['class'] = !empty($item->disabled) ? array('ctools-export-ui-disabled') : array('ctools-export-ui-enabled');

    // If we have an admin title, make it the first row.
    if (!empty($this->plugin['export']['admin_title'])) {
      $this->rows[$name]['data'][] = array('data' => check_plain($item->{$this->plugin['export']['admin_title']}), 'class' => array('ctools-export-ui-title'));
    }
    $this->rows[$name]['data'][] = array('data' => check_plain($name), 'class' => array('ctools-export-ui-name'));

    $this->rows[$name]['data'][] = array('data' => check_plain($item->{$schema['export']['export type string']}), 'class' => array('ctools-export-ui-storage'));

    // To display whether this has any description
    $snippet = $this->load_item($name);
    $label = "No";
    if ($snippet->rid) {
      $label = 'Yes';
    }
    $this->rows[$name]['data'][] = array('data' => $label, 'class' => array('ctools-export-ui-title'));

    $ops = theme('links__ctools_dropbutton', array('links' => $operations, 'attributes' => array('class' => array('links', 'inline'))));

    $this->rows[$name]['data'][] = array('data' => $ops, 'class' => array('ctools-export-ui-operations'));

    // Add an automatic mouseover of the description if one exists.
    if (!empty($this->plugin['export']['admin_description'])) {
      $this->rows[$name]['title'] = $item->{$this->plugin['export']['admin_description']};
    }
  }

  /**
   * Provide the table header.
   *
   * If you've added columns via list_build_row() but are still using a
   * table, override this method to set up the table header.
   */
  function list_table_header() {
    $header = array();
    if (!empty($this->plugin['export']['admin_title'])) {
      $header[] = array('data' => t('Title'), 'class' => array('ctools-export-ui-title'));
    }

    $header[] = array('data' => t('Name'), 'class' => array('ctools-export-ui-name'));
    $header[] = array('data' => t('Storage'), 'class' => array('ctools-export-ui-storage'));
    $header[] = array('data' => t('Has Content/Revised'), 'class' => array('ctools-export-ui-name'));
    $header[] = array('data' => t('Operations'), 'class' => array('ctools-export-ui-operations'));

    return $header;
  }

}

function _save_snippet($values) {
  // need to set  is_current to 0 before setting up the new one
  _snippet_revision_reset_current($values['name']);

  $revision = new stdClass();
  $revision->name = $values['name'];
  $revision->title = $values['title'];
  $revision->content = $values['content']['value'];
  $revision->content_format = $values['content']['format'];
  $revision->timestamp = strtotime('now');
  $revision->is_current = 1;

  $status = drupal_write_record('snippet_revision', $revision);
  return $status;
}

/**
 *
 * @param $name
 */
function _snippet_revision_reset_current($name) {
  $set_is_current = db_update('snippet_revision')
                    ->fields(array(
                      'is_current' => 0,
                    ))
                    ->condition('name', $name)
                    ->execute();
  return $set_is_current;
}


/**
 * Helper function for outputting the preview above the form
 * @param $form
 * @param $form_state
 */
function snippet_form_build_preview_callback($form, &$form_state) {
  // Display a preview of the snippet.
  if (!form_get_errors()) {
    $variable = array();
    $variable['rid'] = $form_state['values']['rid'];
    $variable['name'] = $form_state['values']['name'];
    $variable['title'] = $form_state['values']['title'];
    $variable['content'] = $form_state['values']['content']['value'];
    $variable['in_preview'] = 1;
    return $form_state['snippet_preview_wrapper'] = theme('snippet_content_show', $variable);
  }
}

function snippet_build_preview($form, &$form_state){
  $form_state['rebuild'] = TRUE;
}
