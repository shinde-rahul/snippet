<?php
/**
 * @file
 * The admin inc file for snippet.
 *
 */

/**
 * Helper function to set all reviosion to 0
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
  drupal_set_title(t('Revisions for %title', array('%title' => $snippet->admin_title)), PASS_THROUGH);

  $header = array(t('Revision'), array('data' => t('Operations'), 'colspan' => 2));
  $snippet_revisions = db_select('snippet_revision', 'sr')
                      ->fields('sr', array())
                      ->condition('name', $snippet->name)
                      ->orderBy('is_current', 'DESC')
                      ->orderBy('rid', 'DESC')
                      ->execute()->fetchAll();

  $rows = array();
  $revert_permission = FALSE;

  if (user_access('edit snippet')) {
    $revert_permission = TRUE;
  }

  // if only a version available then don't show revert option
  if (count($snippet_revisions) == 1) {
    $revert_permission = FALSE;
  }

  foreach ($snippet_revisions as $revision) {
    $row = array();
    $operations = array();
    $row[] = array('data' => t('!date', array('!date' => l(format_date($revision->timestamp, 'short'), SNIPPET_MENU_PREFIX . "/$snippet->name/revisions/$revision->rid/view"  ))));

    if ($revert_permission) {
      $operations[] = l(t('revert'), SNIPPET_MENU_PREFIX . "/$snippet->name/revisions/$revision->rid/revert");
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
 * View snippet revision
 * @param $snippet
 */
function snippet_revision_view($snippet) {
  drupal_set_title(t('Revision of %title from %date', array('%title' => $snippet->admin_title, '%date' => format_date($snippet->timestamp))), PASS_THROUGH);
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
  watchdog('snippet content', 'Snippets reverted %title revision %revision.', array( '%title' => $snippet_revision->admin_title, '%revision' => $snippet_revision->rid));
  drupal_set_message(t('Snippets %title has been reverted back to the revision from %revision-date.', array( '%title' => $snippet_revision->admin_title, '%revision-date' => format_date($snippet_revision->timestamp))));
  $form_state['redirect'] = SNIPPETS_LIST_MENU . "/$snippet_revision->name/revisions";
}

