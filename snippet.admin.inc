<?php
/**
 * @file
 * The admin inc file for snippet.
 * 
 */


/**
 * Form builder: Builds the snippet administration.
 */
function snippet_list_form() {

  // Build the table header.
  $header = array(
             'title' => array('data' => t('Title')),
             'name' => array('data' => t('Name')),
             'operations' => array('data' => t('Operations'))
             );
  
  // fetch all snippet
  ctools_include('export');
  $snippet = ctools_export_crud_load_all('snippet');
  
  // get the destination
  $destination = drupal_get_destination();
  
  // build the table rows
  $rows = array();
  foreach ($snippet as $key => $value) {
    $row = array();
    $operations = array();
    $rows[$value->name] = array(
      'title' => t($value->title),
      'name' =>  $value->name,
    );
    
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => SNIPPETS_LIST_MENU . "/$value->name/edit",
      'query' => $destination,
    );
    
    $operations['revert'] = array(
      'title' => t('revisions'),
      'href' => SNIPPETS_LIST_MENU . "/$value->name/revisions",
    );
    
    $rows[$value->name]['operations'] = array(
        'data' => array(
          '#theme' => 'links',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
  }
  
  // use the table theme to show the listing of existing snippet with operations
  $form['snippet'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No content available.'),
    );
  
  return $form;
}

/**
 * Form builder: Builds the snippet edit form.
 * @param $form
 * @param $form_state
 * @param $snippet
 */
function snippet_form($form, $form_state, $snippet) {
  // Some special stuff when previewing a snippet.
  if (isset($form_state['snippet_preview'])) {
    $form['#prefix'] = $form_state['snippet_preview'];
  }
  
  $form['rid'] = array(
    '#type' => 'hidden',
    '#value' => $snippet->rid,
  );
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $snippet->admin_title,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#disabled' => TRUE
  );
  
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#default_value' => $snippet->name,
    '#maxlength' => 255,
    '#disabled' => TRUE
  );
  
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('Title for the textarea-exportible.'),
    '#default_value' => $snippet->title,
    '#required' => TRUE,
    '#disabled' => TRUE
  );

  $form['content'] = array(
    '#type' => 'text_format',
    '#title' => t('Description'),
    '#description' => t('Description of this snippet.'),
    '#default_value' => $snippet->content,
    '#format' => $snippet->content_format,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  
  $form['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview'),
    '#submit' => array('snippet_form_build_preview'),
  );
  
  return $form;
}

/**
 * Helper function for outputting the preview above the form
 * @param $form
 * @param $form_state
 */
function snippet_form_build_preview($form, &$form_state) {
  // Display a preview of the snippet.
  if (!form_get_errors()) {
    $variable = array();
    $variable['rid'] = $form_state['values']['rid'];
    $variable['name'] = $form_state['values']['name'];
    $variable['title'] = $form_state['values']['title'];
    $variable['content'] = $form_state['values']['content']['value'];
    $variable['in_preview'] = 1;
    $form_state['snippet_preview'] = theme('snippet_content_show', $variable);
  }
  
  $form_state['rebuild'] = TRUE;
}


/**
 * 
 * @param $form
 * @param $form_state
 */
function snippet_form_submit(&$form, &$form_state) {
  $values = (object) $form_state['values'];
  
  $snippet_details = new stdClass();
  $snippet_details->name = $values->machine_name;
  $snippet_details->content = $values->content['value'];
  $snippet_details->content_format = $values->content['format'];
  
  $status = _snippet_save($snippet_details);
  // put success message
  if ($status) {
    drupal_set_message(t('Snippet content updated successfully.'), 'status');
  }
  
}

/**
 * Hepler function to save the snippet
 * @param $snippet
 */
function _snippet_save($snippet) {
  // need to set  is_current to 0 before setting up the new one
  _snippet_revision_set_current($snippet->name);
  
  $revision = new stdClass();
  $revision->name = $snippet->name;
  $revision->content = $snippet->content;
  $revision->content_format = $snippet->content_format;
  $revision->timestamp = strtotime('now');
  $revision->is_current = 1;
  
  $status = drupal_write_record('snippet_revision', $revision);
  return $status;
}

/**
 * 
 * @param $name
 */
function _snippet_revision_set_current($name) {
  $set_is_current = db_update('snippet_revision')
                    ->fields(array(
                      'is_current' => 0,
                    ))
                    ->condition('name', $name)
                    ->execute();
  return $set_is_current;
}


/**
 * Generate an overview table of older revisions of a node.
 */
function snippet_revision_overview($snippet) {
  drupal_set_title(t('Revisions for %title', array('%title' => $snippet->title)), PASS_THROUGH);

  $header = array(t('Revision'), array('data' => t('Operations'), 'colspan' => 2));
  $snippet_revisions = db_select('snippet_revision', 'sr')
                      ->fields('sr', array())
                      ->condition('name', $snippet->name)
                      ->orderBy('is_current', 'DESC')
                      ->orderBy('rid', 'DESC')
                      ->execute()->fetchAll();

  $rows = array();
  $revert_permission = FALSE;
  
  if (user_access('edit snippet content')) {
    $revert_permission = TRUE;
  }
  
  // if only a version available then don't show revert option
  if (count($snippet_revisions) == 1) {
    $revert_permission = FALSE;
  }
  
  foreach ($snippet_revisions as $revision) {
    $row = array();
    $operations = array();
    $row[] = array('data' => t('!date', array('!date' => l(format_date($revision->timestamp, 'short'), SNIPPETS_LIST_MENU . "/$snippet->name/revisions/$revision->rid/view"  ))));
    
    if ($revert_permission) {
      $operations[] = l(t('revert'), SNIPPETS_LIST_MENU . "/$snippet->name/revisions/$revision->rid/revert");
    }
    $rows[] = array_merge($row, $operations);
  }
  
  $build['snippet_revisions_table'] = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => $header,
  );
  return $build;
}

/**
 * View the revision 
 * @param $snippet
 */
function snippet_revision_view($snippet) {
  drupal_set_title(t('Revision of %title from %date', array('%title' => $snippet->title, '%date' => format_date($snippet->timestamp))), PASS_THROUGH);
  $html = " <div id = 'snippet'>
              <h2>$snippet->title</h2>
              <div class = 'content' >
                $snippet->content
              </div>
            </div>";
  return $html;
}


/**
 * Ask for confirmation of the reversion to prevent against CSRF attacks.
 */
function snippet_revision_revert_confirm($form, $form_state, $revision) {
  $form['#revision'] = $revision;
  return confirm_form($form, 
                      t('Are you sure you want to revert to the revision from %revision-date?', 
                         array(
                          '%revision-date' => format_date($revision->timestamp))), 
                          SNIPPETS_LIST_MENU . "/$revision->rid/revisions", 
                          '', t('Revert'), t('Cancel'));
}

/**
 * 
 * @param $form
 * @param $form_state
 */
function snippet_revision_revert_confirm_submit($form, &$form_state) {
  $snippet_revision = $form['#revision'];
  
  _snippet_revision_set_current($snippet_revision->name);
  
  $revision = new stdClass();
  $revision->rid = $snippet_revision->rid;
  $revision->is_current = 1;
  
  $status = drupal_write_record('snippet_revision', $revision, 'rid');
  watchdog('snippet content', 'Snippets reverted %title revision %revision.', array( '%title' => $snippet_revision->title, '%revision' => $snippet_revision->rid));
  drupal_set_message(t('Snippets %title has been reverted back to the revision from %revision-date.', array( '%title' => $snippet_revision->title, '%revision-date' => format_date($snippet_revision->timestamp))));
  $form_state['redirect'] = SNIPPETS_LIST_MENU . "/$snippet_revision->name/revisions";
}

