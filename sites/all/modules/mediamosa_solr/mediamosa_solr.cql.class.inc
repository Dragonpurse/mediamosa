<?php
/**
 * @file
 * Extended on the existing mediamosa_core_cql class.
 */

class mediamosa_solr_asset_cql_context extends mediamosa_asset_cql_context {
  // --------------------------------------------------------------------- Vars.
  /**
   * The search parameters, set for CQL parsing. Read only.
   *
   * @var array
   */
  public $parameters;

  // ------------------------------------------------------------------ Methods.
  /**
   * Returns the array containing mapping voor cql2solr.
   *
   * @return array
   */
  protected function mediamosa_core_cql_context_array_get() {
    static $mediamosa_context = NULL;
      mediamosa_watchdog::log_export(__FUNCTION__);

    if (!isset($mediamosa_context)) {
      $mediamosa_context = $this->m_a_mediamosa_context;
      $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEX_REFERENCES]['mediafile'] = 'mediamosa:mediafile';

      $replace = array(
        'mime_type' => 'mime_type',
        'acl_app' => 'mf_app_id_master',
        'is_master_slaved' => 'asset_is_master_slaved',
        'is_protected' => 'mf_is_protected',
        'acl_app' => 'mf_app_id_master',
        'acl_user' => 'mf_acl_user',
        'acl_group' => 'mf_acl_user_group',
        'acl_domain' => 'mf_acl_domain',
        'acl_realm' => 'mf_acl_realm',
        'acl_realm_prefix' => 'mf_acl_realm_prefix',
      );

      foreach ($mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES] as $context_set => &$lvl1) {
        foreach ($lvl1[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS] as $column_name => $column) {
          if (isset($replace[$column_name])) {
            $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'] = $replace[$column_name];
          }

          $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['table_for_sort'] = $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$column_name]['column'];
        }
      }

      $metadata_properties = mediamosa_asset_metadata_property::get_metadata_properties_full($this->app_ids);

      foreach ($metadata_properties as $name => $metadata_property) {
        $group_name_short = mediamosa_asset_metadata_property_group::property_group_get_context($metadata_property['propgroup_name']);

        switch ($metadata_property['propgroup_name']) {
          case 'qualified_dublin_core':
            $context_set = 'mediamosa:qdc';
            break;

          case 'dublin_core':
            $context_set = 'mediamosa:dc';
            break;

          case 'czp':
            $context_set = 'mediamosa:czp';
            break;

          case 'acl':
          case 'aut':
            $context_set = 'mediamosa:acl';
            break;

          default:
            $context_set = 'mediamosa:' . $metadata_property['propgroup_name'];
            break;
        }

        switch ($metadata_property['propdef_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $column = $group_name_short . '_' . $name . '|vi';
            $sort_column = $group_name_short . '_' . $name . '|svi';
            $type = mediamosa_sdk::TYPE_INT;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;

          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $column = $group_name_short . '_' . $name . '|vd';
            $sort_column = $group_name_short . '_' . $name . '|svd';
            $type = mediamosa_sdk::TYPE_DATETIME;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;

          default:
            assert(0);

          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $column = $group_name_short . '_' . $name . '|vc';
            $sort_column = $group_name_short . '_' . $name . '|svc';
            $type = mediamosa_sdk::TYPE_STRING;
            $table = mediamosa_asset_metadata_db::TABLE_NAME;
            break;
        }

        $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$name] = array(
          MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $sort_column,
          MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'unused',
          MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
          // Indication its an asset metadata field.
          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $metadata_property['propdef_id'],
        );
      }

      // Add search for mediafile technical metadata.
      // Get all the data needed for indexing.
      $mediafile_metadata_properties = module_invoke_all('mediafile_metadata_properties');
      $context_set = 'mediamosa:mediafile';
      foreach ($mediafile_metadata_properties as $name => &$info) {
        if (!empty($info['index'])) {
          $info += array('type' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR);
          $group_name_short = 'mf';
          $index = $group_name_short . '_' . $name;

          switch ($info['type']) {
            default:
            case mediamosa_asset_mediafile_metadata_db::VAL_INT:
              $column = $index . '|vi';
              $sort_column = $index . '|svi';
              $type = mediamosa_sdk::TYPE_INT;
              $table = mediamosa_asset_metadata_db::TABLE_NAME;
              break;

            case mediamosa_asset_mediafile_metadata_db::VAL_DATETIME:
              $column = $index . '|vd';
              $sort_column = $index . '|svd';
              $type = mediamosa_sdk::TYPE_DATETIME;
              $table = mediamosa_asset_metadata_db::TABLE_NAME;
              break;

            case mediamosa_asset_mediafile_metadata_db::VAL_CHAR:
              $column = $index . '|md';
              $sort_column = $index . '|svc';
              $type = mediamosa_sdk::TYPE_STRING;
              $table = mediamosa_asset_metadata_db::TABLE_NAME;
              break;
          }

          $mediamosa_context[MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES]['mediamosa'][MEDIAMOSA_CQL_CONTEXT_KEY_INDEXES][$context_set][MEDIAMOSA_CQL_CONTEXT_KEY_COLUMNS][$index] = array(
            MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN => $column,
            MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT => $sort_column,
            MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_ALIAS => 'unused',
            MEDIAMOSA_CQL_CONTEXT_KEY_TYPE => $type,
  //          // Indication its an asset metadata field.
  //          MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID => $metadata_property['propdef_id'],
          );
        }
      }
      mediamosa_watchdog::log_export($mediafile_metadata_properties);
    }

    return $mediamosa_context;
  }

  /**
   * Finalize the relation, index, search term
   *
   * @param mediamosa_cql_part $cql_part
   * @param mediamosa_cql_part_relation $cql_part_relation
   * @param array $result
   * @param string $str_index
   * @param bool $do_having
   *   Unused.
   *
   * @return string
   *   The generated Solr query string.
   *
   * @throws mediamosa_cql_exception
   * @throws mediamosa_solr_exception
   */
  public function mediamosa_core_cql_context_index2column_finalize(mediamosa_cql_part $cql_part, mediamosa_cql_part_relation $cql_part_relation, array &$result, $str_index, $do_having) {

    // Solr does not support having or needs it here.
    if ($do_having) {
      return '';
    }

    $cql_part_search_term = $cql_part->mediamosa_cql_part_search_term_get();
    $cql_part_boolean = $cql_part->mediamosa_cql_part_boolean_get();

    $pos = strrpos($str_index, '.');
    if ($pos !== FALSE) {
      $str_index = mediamosa_unicode::substr($str_index, $pos + 1);
    }

    $relation_modifiers = $cql_part_relation->mediamosa_cql_modifier_get_array();

    $index2column = $this->mediamosa_core_cql_context_index2column($str_index);
    if ($index2column === FALSE) {
      throw new mediamosa_cql_exception('index not found, use verify before you parse the cql string.', CQL_EXCEPTION_SEVERITY_HIGH);
    }

    // Solr does not support having.
    $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_USE_HAVING] = FALSE;

    // Special case when these keys are used.
    $acl_joins = array(
      'ACL_NAME_USER',
      'ACL_NAME_USER_GROUP',
      'ACL_GROUP_DOMAIN',
      'ACL_GROUP_REALM',
      'ACL_APP_MASTER_SLAVE',
    );
    if (!empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN]) && in_array($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN], $acl_joins)) {
      $result['is_acl_search'] = TRUE;
    }

    // Is case sensitive only?
    $is_case_sensitive = empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_CASE_SENSITIVE]) ? FALSE : TRUE;

    // For some special cases.
    if (isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN])) {
      $result['a_joins'][$str_index] = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_JOIN];
    }

    // Is asset metadata?
    $is_metadata = empty($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID]) ? FALSE : TRUE;

    $wildcard = '*';

    $str_table_column = $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];
    // Wildcard search.
    if (!isset($relation_modifiers[mediamosa_cql::STEM])) {
      $str_table_column_exact = $is_metadata ? $str_table_column . '_exact' : $str_table_column;
      $str_table_column_exact_cs = $is_metadata ? $str_table_column . '_exact_cs' : $str_table_column;
    }
    else {
      $str_table_column_exact = $str_table_column;
      $str_table_column_exact_cs = $str_table_column;
      // Stemming does not use wildcard chars.
      $wildcard = '';
    }

    if (isset($index2column['select_expr'])) {
      $result['a_select_expr'][$str_index] = $index2column['select_expr'];
    }

    // Get the search tern.
    $str_search_term = $cql_part_search_term->mediamosa_cql_part_str_get();

    // NULL values allowed.
//    $has_null = isset($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL]) ? $index2column[MEDIAMOSA_CQL_CONTEXT_KEY_NULL] : FALSE;

    $parts =  array();

    // Default is OR.
    $str_sub_parts_glue = ' OR ';
    $str_relation = $cql_part_relation->mediamosa_cql_part_str_get();
    $has_not = $cql_part->mediamosa_cql_part_has_not();

    $phrase_search = false;
    switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
      case mediamosa_sdk::TYPE_INT:
        break;

      case mediamosa_sdk::TYPE_DATETIME:
        $is_case_sensitive = TRUE;
        $wildcard = '';

        if ($str_relation !== 'within' && mediamosa_unicode::strtolower($str_search_term) !== 'null') {
          // Convert to DB Date.
          $str_search_term = $this->mediamosa_cql_str_date_2_db_date($str_search_term);
        }
        // Do NULL compare?
        if (mediamosa_unicode::strtolower($str_search_term) === 'null') {
          switch ($str_relation) {
            case '<>':
              $str_relation = 'IS NOT NULL';
              break;

            default:
              $str_relation = 'IS NULL';
              break;
          }
        }
        break;

      case mediamosa_sdk::TYPE_DOMAIN:
      case mediamosa_sdk::TYPE_REALM:
      case mediamosa_sdk::TYPE_STRING:
        // Do NULL compare?
        if (mediamosa_unicode::strtolower($str_search_term) === 'null') {
          switch ($str_relation) {
            case '<>':
              $str_relation = 'IS NOT NULL';
              break;

            default:
              $str_relation = 'IS NULL';
              break;
          }
        }
        $phrase_search = true;
        break;

      case mediamosa_sdk::TYPE_BOOL:
      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
        $is_case_sensitive = TRUE;
        $wildcard = '';

        $str_search_term = (mediamosa_unicode::strtolower($str_search_term) === 'true' ? 'TRUE' : 'FALSE');

        $bool_is_true = $str_search_term === 'TRUE';

        // Lets prevent usage of NOT.
        $bool_is_true = ($str_relation === '<>' ? !$bool_is_true : $bool_is_true);

        // Back into text.
        $str_search_term = $bool_is_true ? 'TRUE' : 'FALSE';

        // Prevent usage of <>.
        $str_relation = 'bool';
        break;

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
        throw new mediamosa_solr_exception('Type TYPE_JOB_PROGRESS is not supported by asset SOLR, job context.');
    }

    // Switch NOT value when <> is found.
    if ($str_relation == '<>') {
      $has_not = !$has_not;
      $str_relation = '=';
    }

    // Case sensitive fix (metadata).
    $str_table_column_exact = isset($relation_modifiers[mediamosa_cql::RESPECTCASE]) && $is_metadata ? $str_table_column_exact_cs : $str_table_column_exact;

    $boost = '';
    if (isset($relation_modifiers[mediamosa_cql::BOOST])) {
      $boost = '^' . $relation_modifiers[mediamosa_cql::BOOST]['value'];
    }

    switch ($str_relation) {
      case 'bool':
        $parts[$str_table_column_exact] = '{column}:' . mediamosa_solr_apache_solr_service::escape($str_search_term);
        break;

      case 'all':
        $str_sub_parts_glue = ' AND ';

      case '=':
      case 'any':
        if (!$is_case_sensitive && !$is_metadata) {
          $str_search_term = mediamosa_unicode::strtolower($str_search_term);
        }

        // Split whitespace up into words.
        $words = preg_split('/\s+/', $str_search_term);
        if (isset($relation_modifiers[mediamosa_cql::KEEPORDER])) {
          $keep_order_ids = array();
          foreach ($words as $word) {
            if (trim($word) === '') {
              continue;
            }
            $keep_order_ids[] = $this->mediamosa_cql_trim_anchor($word);
          }
          $this->mediamosa_core_set_keep_order($str_table_column, $keep_order_ids);
        }

        foreach ($words as $word) {
          if (trim($word) === '') {
            continue;
          }

          // Fix solr problem with wildcard on right.
          if (substr($word, 0, 1) === '*') {
            if (mediamosa_unicode::substr($word, -1) === '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) !== '\^' && mediamosa_unicode::substr($word, 0, 1) !== '^') {
              $words[] = substr($word, 1, -1);
            }
            else {
              $words[] = substr($word, 1);
            }
          }
          else {
            if (mediamosa_unicode::substr($word, -1) === '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) !== '\^' && mediamosa_unicode::substr($word, 0, 1) !== '^') {
              $words[] = substr($word, 0, -1);
            }
          }
        }

        $sub_parts = array();
        foreach ($words as $word) {
          if (trim($word) === '') {
            continue;
          }

          switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
            case mediamosa_sdk::TYPE_INT:
            case mediamosa_sdk::TYPE_UINT:
              // Cast to integer, as spaces will break solr.
              $sub_parts[] = strtr(
                '{column}:{value}{boost}',
                array(
                  '{value}' => (int) trim($word),
                  '{boost}' => $boost,
                )
              );
              break;

            default:
              // Remove trailing ^.
              if (mediamosa_unicode::substr($word, 0, 1) === '^' && mediamosa_unicode::substr($word, -1) === '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) !== '\^') {
                $sub_parts[] = strtr(
                  '{column}:{value}{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($word, 1, -1)),
                    '{boost}' => $boost,
                  )
                );
              }
              elseif (mediamosa_unicode::substr($word, 0, 1) === '^') {
                // Left ancor.
                $sub_parts[] = strtr(
                  '{column}:{value}{wildcard}{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($word, 1)),
                    '{wildcard}' => $wildcard,
                    '{boost}' => $boost,
                  )
                );
              }
              elseif (mediamosa_unicode::substr($word, -1) === '^' && mediamosa_unicode::substr(str_replace('\\', '', $word), -2) !== '\^') {
                $word = ltrim(mediamosa_unicode::substr($word, 0, -1), '*');
                // Right ancor.
                $sub_parts[] = strtr(
                  '{column}:{wildcard}{value}{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards($word),
                    '{wildcard}' => $wildcard,
                    '{boost}' => $boost,
                  )
                );
              }
              else {
                // No ancor search.
                $sub_parts[] = strtr(
                  '{column}:{wildcard}{value}{wildcard}{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards($word),
                    '{wildcard}' => '',
                    '{boost}' => $boost,
                  )
                );
              }
          }
        }

        $parts[$str_table_column] = (count($sub_parts) > 1 ? '(' : '') . implode($str_sub_parts_glue, $sub_parts) . (count($sub_parts) > 1 ? ')' : '');
        break;

      case '==':
      case 'exact':
      case 'adj':
        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_INT:
          case mediamosa_sdk::TYPE_UINT:
            // Cast to integer, as spaces will break solr.
            $parts[$str_table_column] = strtr(
              '{column}:{value}{boost}',
              array(
                '{value}' => (int) $str_search_term,
                '{boost}' => $boost,
              )
            );
            break;

          default:
            if (!$is_case_sensitive && !$is_metadata) {
              $str_search_term = mediamosa_unicode::strtolower($str_search_term);
            }

            if (!$is_metadata) {
              if ($cql_part_search_term->mediamosa_cql_contains_wildcards($str_search_term)) {
                $str_search_term = mediamosa_unicode::strtolower($str_search_term);
              }
            }

            // Remove trailing ^.
            if (mediamosa_unicode::substr($str_search_term, 0, 1) == '^' && mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
              // Add our search.
              $parts[$str_table_column_exact] = strtr(
                '{column}:{value}{boost}',
                array(
                  '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1, -1)),
                  '{boost}' => $boost,
                )
              );
            }
            elseif (mediamosa_unicode::substr($str_search_term, 0, 1) == '^') {
              // Left ancor.
              $parts[$str_table_column_exact] = strtr(
                '{column}:{value}{wildcard}{boost}',
                array(
                  '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 1)),
                  '{wildcard}' => $wildcard,
                  '{boost}' => $boost,
                )
              );
            }
            elseif (mediamosa_unicode::substr($str_search_term, -1) == '^' && mediamosa_unicode::substr(str_replace('\\', '', $str_search_term), -2) != '\^') {
              // Right ancor.
              $parts[$str_table_column_exact] = strtr(
                '{column}:{wildcard}{value}{boost}',
                array(
                  '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards(mediamosa_unicode::substr($str_search_term, 0, -1)),
                  '{wildcard}' => $wildcard,
                  '{boost}' => $boost,
                )
              );
            }
            else {
              if ($phrase_search) {
                $parts[$str_table_column] = strtr(
                  '{column}:"{value}"{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escapePhrase($str_search_term),
                    '{boost}' => $boost,
                  )
                );
              }
              else {
                $parts[$str_table_column] = strtr(
                  '{column}:{wildcard}{value}{wildcard}{boost}',
                  array(
                    '{value}' => mediamosa_solr_apache_solr_service::escape($str_search_term),
                    '{wildcard}' => $wildcard,
                    '{boost}' => $boost,
                  )
                );
              }
            }
        }
        break;

      case '<>':
        $has_not = !$has_not;
        $parts[$str_table_column] = strtr(
          '{column}:{value}{boost}',
          array(
            '{value}' => mediamosa_solr_apache_solr_service::escapeForWildcards($str_search_term),
            '{boost}' => $boost,
          )
        );
        break;

      case '>':
        $parts[$str_table_column] = strtr(
          '{column}:{{value} TO *}{boost}',
          array(
            '{value}' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
            '{boost}' => $boost,
          )
        );
        break;

      case '>=':
        $parts[$str_table_column] = strtr(
          '{column}:[{value} TO *]{boost}',
          array(
            '{value}' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
            '{boost}' => $boost,
          )
        );
        break;

      case '<':
        $parts[$str_table_column] = strtr(
          '{column}:{* TO {value}}{boost}',
          array(
            '{value}' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
            '{boost}' => $boost,
          )
        );
        break;

      case '<=':
        $parts[$str_table_column] = strtr(
          '{column}:[* TO {value}]{boost}',
          array(
            '{value}' => mediamosa_solr_apache_solr_service::escapeForSpace($str_search_term),
            '{boost}' => $boost,
          )
        );
        break;

      case 'within':
        // Split up.
        $values = explode(' ', $str_search_term);

        switch ($index2column[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE]) {
          case mediamosa_sdk::TYPE_DATETIME:
            $values[0] = $this->mediamosa_cql_str_date_2_db_date($values[0]);
            $values[1] = $this->mediamosa_cql_str_date_2_db_date($values[1]);
            break;

          default:
            break;
        }

        $parts[$str_table_column] = strtr('{column}:[{value1} TO {value2}]{boost}',
          array(
            '{value1}' => mediamosa_solr_apache_solr_service::escape($values[0]),
            '{value2}' => mediamosa_solr_apache_solr_service::escape($values[1]),
            '{boost}' => $boost,
          )
        );
        break;

      // Special cases.
      case 'IS NULL':
        $parts[$str_table_column] = '(+*:* -{column}:*)';
        break;

      case 'IS NOT NULL':
        $parts[$str_table_column] = '{column}:*';
        break;
    }

    if ($cql_part_boolean) {
      $str_boolean = $cql_part_boolean->mediamosa_cql_part_str_get();
      $str_boolean = ($str_boolean === 'NOT' ? ' AND ' : ' ' . $str_boolean . ' ');
    }

    // Allow other modules to change the syntax. This can add new parts to the
    // array.
    drupal_alter('mediamosa_solr_cql_parts', $parts, $this->parameters);

    foreach ($parts as $column => $part) {
      $parts[$column] = strtr($part, array('{column}' => str_replace('|', '_', $column)));
    }

    return ($has_not ? '(+*:* -' : '') . (count($parts) > 1 ? '(' : '') . implode (' OR ', $parts) . (count($parts) > 1 ? ')' : '') . ($has_not ? ')' : '') . (empty($str_boolean) ? '' : $str_boolean);
  }

  /**
   * Convert the CQL array result to a string.
   *
   * @param array $parse_result
   *   The parse result to convert to string.
   * @param string $outside
   *   The last conjuction type of our parent.
   * @param bool $first
   *   Is first call, or child call.
   *
   * @return string
   */
  protected function mediamosa_cql_parseresult_to_string(array $parse_result, $outside = null, $first = false) {
    $inside = array();
    foreach ($parse_result as &$item) {
      if (!is_string($item)) {
        continue;
      }

      $item = trim($item);

      foreach (array(' AND', ' OR') as $conjunction) {
        if (mediamosa_unicode::strlen($item) > $conjunction && mediamosa_unicode::substr($item, -mediamosa_unicode::strlen($conjunction)) === $conjunction) {
          $inside[] = $conjunction;
        }
      }
    }

    $inside = array_unique($inside);
    $inside = count($inside) > 1 ? null : reset($inside);

    foreach ($parse_result as &$item) {
      if (!is_string($item)) {
        $item = $this->mediamosa_cql_parseresult_to_string($item, $inside, false);
      }
    }

    $str_ql = trim(implode(' ', $parse_result));

    // The end of the string can not end with a conjuction.
    while (true) {
      if (mediamosa_unicode::substr($str_ql, -mediamosa_unicode::strlen(' OR')) === ' OR') {
        $str_ql = trim(mediamosa_unicode::substr($str_ql, 0, -mediamosa_unicode::strlen(' OR')));
      }
      elseif (mediamosa_unicode::substr($str_ql, -mediamosa_unicode::strlen(' AND')) === ' AND') {
        $str_ql = trim(mediamosa_unicode::substr($str_ql, 0, -mediamosa_unicode::strlen(' AND')));
      }
      elseif ($str_ql === 'OR' || $str_ql === 'AND') {
        $str_ql = '';
        break;
      }
      else {
        break;
      }
    }

    if (!$first && (is_null($outside) || is_null($inside) || $outside !== $inside) && $str_ql !== '') {
      $str_ql = '(' . ($outside === ' OR' ? '*:* AND ' : '') .  $str_ql . ')';
    }

    return $str_ql;
  }

  /**
   * Converts a any date into a db Solr date (YYYY-MM-DDTHH:MM:SS)
   *
   * Will convert to timezone set in CQL object.
   *
   * Overridden for fixing format problems with Solr.
   *
   * @param string $date
   *  The paramter to convert.
   * @param string $timezone
   *  Supply timezone for conversion.
   */
  protected function mediamosa_cql_str_date_2_db_date($date, $timezone = 'UTC', $format = 'Y-m-d\TH:i:s\Z') {
    return parent::mediamosa_cql_str_date_2_db_date($date, $timezone, $format);
  }
}

class mediamosa_solr_cql extends mediamosa_core_cql {

  /**
   * Needed for the search parameters.
   *
   * @param array $parameters
   *   The search parameters.
   * @param bool $cql_store_stats
   *   Store stats about the search query.
   * @param array $app_ids
   *   The app IDs used.
   *
   * @return string
   *   The solr query.
   */
  public static function parse_asset_solr($parameters, $cql_store_stats, array $app_ids) {
    // Create the CQL object.
    $mediamosa_cql = new mediamosa_solr_asset_cql_context($app_ids);
    $mediamosa_cql->parameters = $parameters;
    return $mediamosa_cql->mediamosa_core_parse_cql($parameters['cql'], $cql_store_stats);
  }

  /**
   * Overruled for Solr.
   *
   * @param string $str_cql
   *   The CQL string to process.
   * @param bool $cql_store_stats
   *   Store stats about the search query.
   * @param array $app_ids
   *   The app IDs used.
   *
   * @return string
   *   The query string translated from CQL.
   */
  public static function parse_asset($str_cql, $cql_store_stats, array $app_ids) {
    throw new mediamosa_solr_exception('Use parse_asset_solr() instead.');
  }

  /**
   * Verify the CQL syntax.
   *
   * @param string $cql
   *   The CQL string.
   * @param array $app_ids
   *   The application IDs.
   * @param string $error_text
   *   The error text, in case of error.
   *
   * @return bool
   *   Returns TRUE when no errors where found, FALSE otherwise.
   */
  public static function verify_asset($cql, array $app_ids, &$error_text, &$mediamosa_cql = NULL) {
    if (!isset($mediamosa_cql)) {
      $mediamosa_cql = new mediamosa_solr_asset_cql_context($app_ids);
    }

    $result = $mediamosa_cql->mediamosa_cql_verify($cql);
    if (!$result) {
      $error_text = $mediamosa_cql->mediamosa_cql_last_exception_get()->getMessage();
    }
    return $result;
  }
}
