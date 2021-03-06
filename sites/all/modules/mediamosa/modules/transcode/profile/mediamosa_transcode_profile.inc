<?php
/**
 * @file
 * MediaMosa transcode profile function.
 */

/**
 * Show the listing of the current transcode profiles.
 */
function _mediamosa_transcode_profile_list() {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps for the drop down listing.
  $apps = array(0 => t('Global profile (no owner)'));
  $apps = array_merge($apps, mediamosa_app::collect_allowed_apps());

  // Only allow either all apps (admin) or limited to global and owned.
  $app_ids = array_keys($apps);

  $header = array(
    array('data' => t('Transcode profile id'), 'field' => 'tp.' . mediamosa_transcode_profile_db::ID),
    array('data' => t('Profile'), 'field' => 'tp.' . mediamosa_transcode_profile_db::PROFILE),
    array('data' => t('Is Default'), 'field' => 'tp.' . mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    array('data' => t('Tool'), 'field' => 'tp.' . mediamosa_transcode_profile_db::TOOL),
    array('data' => t('App owner'), 'field' => 'tp.' . mediamosa_transcode_profile_db::APP_ID),
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
  $query->fields('tp');

  // Limit apps?
  if (!empty($app_ids)) {
    $query->condition(mediamosa_transcode_profile_db::APP_ID, $app_ids, 'IN');
  }

  $transcode_profiles = $query->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute()->fetchAll();

  // Collect the node IDs.
  $nids = array();
  foreach ($transcode_profiles as $transcode_profile) {
    $nids[] = $transcode_profile[mediamosa_app_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($transcode_profiles as $transcode_profile) {
    $actions = array();
    if (node_access('update', $nodes[$transcode_profile[mediamosa_transcode_profile_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $nodes[$transcode_profile[mediamosa_transcode_profile_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID] . '/delete', $l_options);
    }

    $owner = ($transcode_profile[mediamosa_transcode_profile_db::APP_ID] == 0) ? 'global profile' : $transcode_profile[mediamosa_transcode_profile_db::APP_ID];    $rows[] = array(
      'transcode_profile_id' => $transcode_profile[mediamosa_transcode_profile_db::ID],
      'title' => array('data' => l($transcode_profile[mediamosa_transcode_profile_db::PROFILE], 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID]), 'field' => $transcode_profile[mediamosa_transcode_profile_db::PROFILE]),
      'description' => $transcode_profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE],
      'tool' => $transcode_profile[mediamosa_transcode_profile_db::TOOL],
      'owner' => $owner,
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = array();
  $output[] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no transcode profiles. Click on the <em>Add</em> tab to create one.')));
  $output[] = theme('pager');
  return implode('', $output);
}
