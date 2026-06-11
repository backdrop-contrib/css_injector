<?php
/**
 * @file css_injector.admin.inc
 * Administrative interface for CSS Injector.
 */

/**
 * Define Ace syntax highlighter version.
 */
define('CSS_INJECTOR_ACE_VERSION', '1.43.3');

/**
 * Form builder function for CSS Injector's main admin (rule overview) page.
 */
function css_injector_admin_form($form, &$form_state) {
  _css_injector_update_breadcrumb();

  $rules = _css_injector_load_rule(NULL, TRUE);
  $form = array();
  $form['#tree'] = TRUE;

  // Loop through each rule and define the fields.
  foreach ($rules as $crid => $rule) {
    $form['rules'][$crid]['#rule'] = $rule;
    // Add a checkbox to enable or disable each rule in addition to showing the
    // status of the rule.
    $form['enabled'][$crid] = array(
      '#type' => 'checkbox',
      '#default_value' => $rule['enabled'],
    );
    $form['rules'][$crid]['rule_actions'] = array(
      '#type' => 'dropbutton',
      '#links' => array(
        'edit_rule' => array(
          'title' => t('Edit Rule'),
          'href' => 'admin/config/development/css-injector/edit/' . $crid,
        ),
        'delete_rule' => array(
          'title' => t('Delete Rule'),
          'href' => 'admin/config/development/css-injector/delete/' . $crid,
        ),
      ),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  if (!empty($rules)) {
    $form['actions']['rewrite_files'] = array(
      '#type' => 'button',
      '#value' => t('Rewrite CSS files'),
      '#executes_submit_callback' => TRUE,
      '#submit' => array('css_injector_rewrite_css_files'),
    );
  }
  return $form;
}

/**
 * Submit button callback for the admin (rule overview) form.
 */
function css_injector_admin_form_submit($form, &$form_state) {
  // Load the config for rule definitions.
  $config = config('css_injector.rules_definition');

  // Get the Enabled state of each rule from the form.
  $rule_status = $form_state['values']['enabled'];

  // Loop through each rule and set the 'Enabled' config value.
  foreach ($rule_status as $crid => $status) {
    $config->set('rules.' . $crid . '.enabled', $status);
  }

  // Save the config and provide a message.
  $config->save();
  backdrop_set_message(t('Configuration saved.'));
}

/**
 * Rewrite CSS Files from Config.
 */
function css_injector_rewrite_css_files() {
  // Get the rules_css config.
  $config = config('css_injector.rules_css');
  $rules_css = $config->get('rules');

  // Check that one or more rules are defined.
  if (!empty($rules_css)) {
    // If rules exist, rewrite the file, over-writing if it exists.
    $file_count = 0;
    foreach ($rules_css as $rule_id => $rule) {
      // Check that CSS text is present.
      if (!empty($rule['css_text'])) {
        $result = file_unmanaged_save_data($rule['css_text'], _css_injector_rule_uri($rule_id), FILE_EXISTS_REPLACE);
        $file_count = $result ? ++$file_count : $file_count;
      }
    }
    if ($file_count > 0) {
      backdrop_set_message(t('!file_count files have been written or re-written.', array(
        '!file_count' => $file_count,
      )));
    }
    else {
      backdrop_set_message(t('No files have been written or re-written.'), 'warning');
    }

  }
  else {
    backdrop_set_message(t('No rules found.'), 'warning');
  }
}

/**
 * Theme function for the CSS Injector admin (rule overview) form.
 */
function theme_css_injector_admin_form($variables) {
  $form = $variables['form'];
  $headers = array(
    t('Enabled'),
    t('Title'),
    t('Location'),
    t('Actions'),
  );
  $rows = array();
  if (!empty($form['rules'])) {
    foreach (element_children($form['rules']) as $crid) {
      $rule = $form['rules'][$crid]['#rule'];
      $row = array(
        backdrop_render($form['enabled'][$crid]),
        check_plain($rule['title']),
        _css_injector_rule_uri($crid),
        backdrop_render($form['rules'][$crid]['rule_actions']),
      );
      $rows[] = $row;
    }
  }

  $form['rules'] = array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $rows,
    '#empty' => t('No CSS injection rules have been set up yet.'),
  );

  return backdrop_render_children($form);
}

/**
 * Form builder function for the CSS rule edit form.
 */
function css_injector_edit($form, $form_state, $crid = NULL) {
  _css_injector_update_breadcrumb();
  $module_path = backdrop_get_path('module', 'css_injector');
  $config_settings = config('css_injector.settings');

  if (isset($crid) && is_numeric($crid)) {
    $rule = _css_injector_load_rule($crid, TRUE);
    // Load the css text from config. It is stored separately for performance
    // reasons.
    $rule['css_text'] = config_get('css_injector.rules_css', 'rules.' . $crid . '.css_text');
  }
  else if (!isset($crid)) {
    $input = !empty($form_state['input']) ? $form_state['input'] : NULL;
    $rule = array(
      'title' => $input['title'] ?? '',
      'rule_type' => $input['rule_type'] ?? CSS_INJECTOR_PAGES_LISTED,
      'rule_themes' => $input['rule_themes'] ?? '',
      'rule_conditions' => $input['rule_conditions'] ?? '',
      'media' => $input['media'] ?? 'all',
      'preprocess' => $input['preprocess'] ?? 1,
      'css_text' => $input['css_text'] ?? '',
      'enabled' => $input['enabled'] ?? 1,
    );
  }

  if ($config_settings->get('css_injector_use_ace') && !module_exists('ace_editor')) {
    // Adding ace syntax highlighter.
    $form['#attached']['js'] = array(
      // Core Ace library.
      'https://cdnjs.cloudflare.com/ajax/libs/ace/' . CSS_INJECTOR_ACE_VERSION . '/ace.js' => array(
        'preprocess' => FALSE,
      ),
      // Theme for editor.
      'https://cdnjs.cloudflare.com/ajax/libs/ace/' . CSS_INJECTOR_ACE_VERSION . '/theme-chrome.js' => array(
        'preprocess' => FALSE,
      ),
      // CSS Mode Definition.
      'https://cdnjs.cloudflare.com/ajax/libs/ace/' . CSS_INJECTOR_ACE_VERSION . '/mode-css.js' => array(
        'preprocess' => FALSE,
      ),
      // Language tools to support autocomplete.
      'https://cdnjs.cloudflare.com/ajax/libs/ace/' . CSS_INJECTOR_ACE_VERSION . '/ext-language_tools.min.js' => array(
        'preprocess' => FALSE,
      ),
      // CSS Snippets to enable shortcuts to completing common statements.
      'https://cdnjs.cloudflare.com/ajax/libs/ace/' . CSS_INJECTOR_ACE_VERSION . '/snippets/css.min.js' => array(
        'preprocess' => FALSE,
      ),
      // Add settings.
      array(
        'data' => array(
          'cssInjector' => array(
            'enableLiveAutocompletion' => (bool) $config_settings->get('css_injector_ace_editor_autocomplete_live'),
          ),
        ),
        'type' => 'setting',
      ),
      // Add and configure the editor.
      $module_path . '/js/syntax_highlighter.js' => array(
        'preprocess' => FALSE,
      ),
    );

    $ace_editor_height = $config_settings->get('css_injector_ace_editor_height');
    $form['#attached']['css'] = array(
      $module_path . '/css/syntax_highlighter.css',
      array(
        'data' => '.css-injector-edit{--ace-editor-height:' . $ace_editor_height . ';}',
        'type' => 'inline',
      ),
    );
  }

  if (isset($crid)) {
    $form['crid'] = array(
      '#type' => 'value',
      '#value' => $crid,
    );
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $rule['title'],
    '#required' => TRUE,
  );

  $form['css_text'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#rows' => 10,
    '#default_value' => $rule['css_text'],
    '#required' => TRUE,
  );

  $ace_editor_markup = '<div class="ace-editor">';
  $ace_editor_markup .= '<div id="editor">' . $rule['css_text'] . '</div></div>';
  $ace_editor_markup .= '<div class="ace-editor-fullscreen help">';
  $ace_editor_markup .= t('Press F12 whilst focus is in the CSS code editor to switch to full screen mode. Press F11 or Esc to exit.');
  $ace_editor_markup .= '</div>';

  $form['css_text_ace'] = array(
    '#prefix' => '<span class="disable-ace">Disable syntax highlighter</span>',
    '#markup' => $ace_editor_markup,
  );

  // Get info of site themes.
  $themes = _css_injector_get_themes();
  $default_theme = config_get('system.core', 'theme_default');

  // Check the rule themes, using the default theme if not set.
  $rule_themes = $config_settings->get('css_injector_default_theme');
  $rule_themes = isset($rule_themes) ? $rule_themes : $default_theme;

  if (isset($rule['rule_themes']) && !empty($rule['rule_themes'])) {
    $rule_themes = $rule['rule_themes'];
  }

  $form['conditional']['rule_themes'] = array(
    '#type' => 'select',
    '#title' => 'Themes to show on',
    '#default_value' => $rule_themes,
    '#options' => $themes,
    '#description' => t('Select themes css will be applied to. @theme theme is selected by default.', array('@theme' => $themes[$default_theme])),
    '#multiple' => TRUE,
  );

  // XXX The php filter that allows embedded PHP code/snippets to be evaluated
  // has been removed from Backdrop core.  I will leave the code that uses it
  // in for now, until we feel sure it is never going to re-surface as
  // a contrib module.  If not, it should eventually be removed.
  // There has been a lot of discussion on the issue.
  // https://www.drupal.org/project/drupal/issues/1203886
  // - Vincent Stemen Sun Apr 22 2018.
  $php_access = (user_access('use PHP for settings') && module_exists('php'));

  $options = array(
    CSS_INJECTOR_PAGES_LISTED => t('Add on the following pages'),
    CSS_INJECTOR_PAGES_NOTLISTED => t('Add on all pages except the following pages'),
  );
  $description = t("Enter one page per line as Backdrop paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array(
    '%blog' => 'blog',
    '%blog-wildcard' => 'blog/*',
    '%front' => '<front>',
  ));

  if ($php_access) {
    $options[CSS_INJECTOR_PHP] = t('Add if the following PHP code outputs a nonzero value (PHP-mode, experts only).');
    $description .= ' ' . t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Backdrop site.', array('%php' => '<?php ?>'));
  }
  $form['conditional']['rule_type'] = array(
    '#type' => 'radios',
    '#title' => t('Add the CSS on specific pages'),
    '#options' => $options,
    '#default_value' => $rule['rule_type'],
  );
  $form['conditional']['rule_conditions'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#default_value' => $rule['rule_conditions'],
    '#description' => $description,
    '#states' => array(
      'required' => array(
        ':input[name="rule_type"]' => array('value' => CSS_INJECTOR_PAGES_LISTED),
      ),
    ),
  );
  $form['media'] = array(
    '#type' => 'select',
    '#title' => t('Media'),
    '#options' => array(
      'all' => t('All'),
      'screen' => t('Screen'),
      'print' => t('Print'),
    ),
    '#default_value' => $rule['media'],
  );

  $form['preprocess'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preprocess CSS'),
    '#default_value' => $rule['preprocess'],
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable rule'),
    '#default_value' => isset($rule['enabled']) ? $rule['enabled'] : 1,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('css_injector_edit_save'),
  );
  $form['actions']['save_and_continue'] = array(
    '#type' => 'submit',
    '#value' => t('Save and Continue Editing'),
    '#submit' => array('css_injector_edit_save_and_continue'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('css_injector_edit_cancel'),
    '#limit_validation_errors' => array(),
  );
  if (!empty($rule['crid'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#crid' => $rule['crid'],
      '#submit' => array('css_injector_edit_delete_button'),
      '#limit_validation_errors' => array(),
    );
  }

  return $form;
}

/**
 * Validation callback for the CSS rule edit form.
 */
function css_injector_edit_validate($form, &$form_state) {
  $directory = 'public://css_injector';
  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
    form_error($form, t('The directory %directory is not writable', array('%directory' => $directory)));
  }
  if ($form_state['values']['rule_type'] == CSS_INJECTOR_PAGES_LISTED && empty($form_state['values']['rule_conditions'])) {
    form_error($form['conditional']['rule_conditions'], t("If the rule type is 'Add on the following pages' then at least one page path must be entered."));
  }
}

/**
 * Submit button callback for the CSS rule edit form.
 */
function css_injector_edit_save($form, &$form_state) {

  $rule_form = $form_state['values'];
  $crid = !empty($rule_form['crid']) ? $rule_form['crid'] : NULL;

  if (is_null($crid)) {
    $all_rules = _css_injector_load_rule();
    $crid = !empty($all_rules) ? max(array_keys($all_rules)) + 1 : 1;
    $form_state['values']['crid'] = $crid;
  }
  $rule = array(
    'crid' => $crid,
    'title' => $rule_form['title'],
    'rule_type' => $rule_form['rule_type'],
    'rule_themes' => $rule_form['rule_themes'],
    'rule_conditions' => $rule_form['rule_conditions'],
    'media' => $rule_form['media'],
    'preprocess' => $rule_form['preprocess'],
    'enabled' => $rule_form['enabled'],
  );
  config_set('css_injector.rules_definition', 'rules.' . $crid, $rule);
  // Save the CSS text to config AND file. The config allows CSS to be deployed
  // using config methods and the file is saved to allow for CSS optimization.
  config_set('css_injector.rules_css', 'rules.' . $crid . '.css_text', $rule_form['css_text']);
  file_unmanaged_save_data($rule_form['css_text'], _css_injector_rule_uri($rule['crid']), FILE_EXISTS_REPLACE);

  backdrop_set_message(t('Your CSS injection rule %title was saved.', array('%title' => $rule['title'])));
  $form_state['redirect'] = 'admin/config/development/css-injector';
}

/**
 * Save and continue callback for the CSS rule edit form.
 */
function css_injector_edit_save_and_continue($form, &$form_state) {
  css_injector_edit_save($form, $form_state);
  $form_state['redirect'] = 'admin/config/development/css-injector/edit/' . $form_state['values']['crid'];
}

/**
 * Cancel callback for the CSS rule edit form.
 */
function css_injector_edit_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/config/development/css-injector';
}

/**
 * Delete button callback for the CSS rule edit form.
 * Redirects the user to the confirmation form.
 */
function css_injector_edit_delete_button($form, &$form_state) {
  $button = $form_state['triggering_element'];
  $crid = $button['#crid'];
  $form_state['redirect'] = 'admin/config/development/css-injector/delete/' . $crid;
}

/**
 * Menu callback -- ask for confirmation of rule deletion.
 */
function css_injector_delete_confirm($form, &$form_state, $crid) {
  _css_injector_update_breadcrumb();
  $form['crid'] = array(
    '#type' => 'value',
    '#value' => $crid,
  );

  $rule = _css_injector_load_rule($crid);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $rule['title'])),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/config/development/css-injector',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Execute node deletion.
 */
function css_injector_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    _css_injector_delete_rule($form_state['values']['crid']);
  }

  $form_state['redirect'] = 'admin/config/development/css-injector';
}
