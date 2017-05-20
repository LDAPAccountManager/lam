<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
/*
$Id$

  This code is not yet part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2012 - 2017  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


/**
* Manages Puppet configuration options.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Puppet configuration options.
*
* @package modules
*/
class puppetClient extends baseModule {

	/** cache for classes */
	private $classCache = null;
	/** cache for variables */
	private $variablesCache = null;

	/**
	* Creates a new authorizedServiceObject object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}


	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('host'));
	}

	/**
    * Returns meta data that is interpreted by parent class
    *
    * @return array array with meta data
    */
    function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'puppet.png';
		// alias name
		$return["alias"] = _("Puppet");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('puppetClient');
		// managed attributes
		$return['attributes'] = array('environment', 'parentnode', 'puppetclass', 'puppetvar');
		// help Entries
		$return['help'] = array (
			'environment' => array (
				"Headline" => _('Environment'), 'attr' => 'environment',
				"Text" => _('Please enter the environment name for this node (e.g. production).')
			),
			'environmentList' => array (
				"Headline" => _('Environment'), 'attr' => 'environment',
				"Text" => _('Please enter the environment name for this node (e.g. production).') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'parentnode' => array (
				"Headline" => _('Parent node'), 'attr' => 'parentnode',
				"Text" => _('This is this node\'s parent. All classes and variables are inherited from this node.')
			),
			'puppetclass' => array (
				"Headline" => _('Classes'), 'attr' => 'puppetclass',
				"Text" => _('The list of configured Puppet classes for this node (e.g. ntp).')
			),
			'puppetclassList' => array (
				"Headline" => _('Classes'), 'attr' => 'puppetclass',
				"Text" => _('The list of configured Puppet classes for this node (e.g. ntp).') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'puppetvar' => array (
				"Headline" => _('Variables'), 'attr' => 'puppetvar',
				"Text" => _('Please enter your Puppet variables for this node (e.g. config_exim=true).')
			),
			'puppetvarList' => array (
				"Headline" => _('Variables'), 'attr' => 'puppetvar',
				"Text" => _('Please enter your Puppet variables for this node (e.g. config_exim=true).') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'autoAdd' => array(
				"Headline" => _('Automatically add this extension'),
				"Text" => _('This will enable the extension automatically if this profile is loaded.')
			),
			'predefinedClasses' => array(
				"Headline" => _('Predefined classes'),
				"Text" => _('These classes will be available as autocompletion hints when adding new classes.')
			),
			'predefinedEnvironments' => array(
				"Headline" => _('Predefined environments'),
				"Text" => _('These environments will be available as autocompletion hints when setting the environment.')
			),
			'enforceClasses' => array(
				"Headline" => _('Enforce classes'),
				"Text" => _('If set then only these classes are allowed. One class per line.')
			),
		);
		// upload fields
		$allowedClasses = '';
		if (!empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$allowedClasses = implode(', ', $this->moduleSettings['puppetClient_enforceClasses']);
		}
		$return['upload_columns'] = array(
			array(
				'name' => 'puppetClient_environment',
				'description' => _('Environment'),
				'help' => 'environmentList',
				'example' => 'production'
			),
			array(
				'name' => 'puppetClient_parentnode',
				'description' => _('Parent node'),
				'help' => 'parentnode',
				'example' => 'basenode'
			),
			array(
				'name' => 'puppetClient_puppetclass',
				'description' => _('Classes'),
				'help' => 'puppetclassList',
				'example' => 'ntp;exim',
				'values' => $allowedClasses
			),
			array(
				'name' => 'puppetClient_puppetvar',
				'description' => _('Variables'),
				'help' => 'puppetvarList',
				'example' => 'config_exim=true;config_exim_trusted_users=root'
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'environment' => _('Environment'),
			'parentnode' => _('Parent node'),
			'puppetclass' => _('Classes'),
			'puppetvar' => _('Variables'),
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('puppetClient_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		$profileEnvironment = new htmlTableExtendedInputField(_('Environment'), 'puppetClient_environment', null, 'environment');
		$autocompleteEnvironment = array();
		if (isset($this->moduleSettings['puppetClient_environments'])) {
			$autocompleteEnvironment = $this->moduleSettings['puppetClient_environments'];
		}
		$profileEnvironment->enableAutocompletion($autocompleteEnvironment);
		$profileContainer->addElement($profileEnvironment, true);
		$profileContainer->addElement(new htmlTableExtendedInputTextarea('puppetClient_puppetclass', '', 60, 5, _('Classes'), 'puppetclass'), true);
		$profileContainer->addElement(new htmlTableExtendedInputTextarea('puppetClient_puppetvar', '', 60, 5, _('Variables'), 'puppetvar'), true);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['puppetClient_environment'] = array('type' => 'ext_preg', 'regex' => 'ascii', 'error_message' => $this->messages['environment'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'puppetClient_environment' => 'environment',
		);
		return $return;
    }

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['environment'][0] = array('ERROR', _('The environment name may only contain ASCII characters.'));
		$this->messages['environment'][1] = array('ERROR', _('Account %s:') . ' puppetClient_environment', _('The environment name may only contain ASCII characters.'));
		$this->messages['puppetclass'][0] = array('ERROR', _('The class names may only contain ASCII characters.'));
		$this->messages['puppetclass'][1] = array('ERROR', _('Account %s:') . ' puppetClient_puppetclass', _('The class names may only contain ASCII characters.'));
		$this->messages['puppetclass'][2] = array('ERROR', _('Account %s:') . ' puppetClient_puppetclass', _('The class names contain invalid values.'));
		$this->messages['puppetvar'][0] = array('ERROR', _('The variables may only contain ASCII characters.'));
		$this->messages['puppetvar'][1] = array('ERROR', _('Account %s:') . ' puppetClient_puppetvar', _('The variables may only contain ASCII characters.'));
		$this->messages['parentnode'][0] = array('ERROR', _('Account %s:') . ' puppetClient_parentnode', _('Parent node not found.'));
	}

	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	* <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	*/
	function save_attributes() {
		if (!in_array('puppetClient', $this->attributes['objectClass']) && !in_array('puppetClient', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_puppetClient_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'puppetClient';
		}
		$return = new htmlTable();
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			$return->addElement(new htmlAccountPageButton('puppetClient', 'attributes', 'addObjectClass', _('Add Puppet extension')));
			return $return;
		}
		$this->initCache();
		// environment
		$autocompleteEnvironment = array();
		if (isset($this->moduleSettings['puppetClient_environments'])) {
			$autocompleteEnvironment = $this->moduleSettings['puppetClient_environments'];
		}
		$environments = array();
		if (isset($this->attributes['environment'][0])) {
			$environments = $this->attributes['environment'];
		}
		if (sizeof($environments) == 0) {
			$environments[] = '';
		}
		$environmentLabel = new htmlOutputText(_('Environment'));
		$environmentLabel->alignment = htmlElement::ALIGN_TOP;
		$return->addElement($environmentLabel);
		$environmentContainer = new htmlGroup();
		for ($i = 0; $i < sizeof($environments); $i++) {
			$environmentField = new htmlInputField('environment' . $i, $environments[$i]);
			$environmentField->enableAutocompletion($autocompleteEnvironment);
			$environmentContainer->addElement($environmentField);
			if ($i < (sizeof($environments) - 1)) {
				$environmentContainer->addElement(new htmlOutputText('<br>', false));
			}
			else {
				$environmentContainer->addElement(new htmlButton('addEnvironment', 'add.png', true));
			}
		}
		$return->addElement($environmentContainer);
		$environmentHelp = new htmlHelpLink('environment');
		$environmentHelp->alignment = htmlElement::ALIGN_TOP;
		$return->addElement($environmentHelp, true);
		// parent node
		$possibleParentNodes = $this->getPossibleParentNodes();
		array_unshift($possibleParentNodes, '-');
		$parentnode = '-';
		if (isset($this->attributes['parentnode'][0])) {
			$parentnode = $this->attributes['parentnode'][0];
		}
		$return->addElement(new htmlTableExtendedSelect('parentnode', $possibleParentNodes, array($parentnode), _('Parent node'), 'parentnode'), true);
		// classes
		if (empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$this->addMultiValueInputTextField($return, 'puppetclass', _('Classes'), false, null, false, $this->classCache);
		}
		else {
			$this->addMultiValueSelectField($return, 'puppetclass', _('Classes'), $this->moduleSettings['puppetClient_enforceClasses']);
		}
		// variables
		$this->addMultiValueInputTextField($return, 'puppetvar', _('Variables'), false, null, false, $this->variablesCache);

		$return->addElement(new htmlSpacer(null, '10px'),true);
		$remButton = new htmlAccountPageButton('puppetClient', 'attributes', 'remObjectClass', _('Remove Puppet extension'));
		$remButton->colspan = 4;
		$return->addElement($remButton);
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (isset($_POST['form_subpage_puppetClient_attributes_remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('puppetClient'), $this->attributes['objectClass']);
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return array();
		}
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			return array();
		}

		$errors = array();
		// environment
		$environmentCounter = 0;
		while (isset($_POST['environment' . $environmentCounter])) {
			$this->attributes['environment'][$environmentCounter] = $_POST['environment' . $environmentCounter];
			if (!get_preg($this->attributes['environment'][$environmentCounter], 'ascii')) $errors[] = $this->messages['environment'][0];
			if ($this->attributes['environment'][$environmentCounter] == '') {
				unset($this->attributes['environment'][$environmentCounter]);
			}
			$environmentCounter++;
		}
		if (isset($_POST['addEnvironment'])) {
			$this->attributes['environment'][] = '';
		}
		$this->attributes['environment'] = array_values($this->attributes['environment']);
		// parent node
		if (isset($this->attributes['parentnode'][0]) && ($_POST['parentnode'] == '-')) {
			unset($this->attributes['parentnode'][0]);
		}
		elseif ($_POST['parentnode'] != '-') {
			$this->attributes['parentnode'][0] = $_POST['parentnode'];
		}
		// classes
		if (empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$this->processMultiValueInputTextField('puppetclass', $errors, 'ascii');
		}
		else {
			$this->processMultiValueSelectField('puppetclass');
		}
		// variables
		$this->processMultiValueInputTextField('puppetvar', $errors, 'ascii');

		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("puppetClient", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "puppetClient";
			// parent node
			if ($rawAccounts[$i][$ids['puppetClient_parentnode']] != "") {
				if (!in_array($rawAccounts[$i][$ids['puppetClient_parentnode']], $this->getPossibleParentNodes())) {
					$messages[] = $this->messages['parentnode'][0];
				}
				else {
					$partialAccounts[$i]['parentnode'][0] = $rawAccounts[$i][$ids['puppetClient_parentnode']];
				}
			}
			// environment
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_environment', 'environment', 'ascii', $this->messages['environment'][1], $messages, '/;[ ]*/');
			// classes
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_puppetclass', 'puppetclass', 'ascii', $this->messages['puppetclass'][1], $messages, '/;[ ]*/');
			if (!empty($this->moduleSettings['puppetClient_enforceClasses'][0]) && !empty($partialAccounts[$i]['puppetclass'])) {
				$invalidClassFound = false;
				foreach ($partialAccounts[$i]['puppetclass'] as $singleClass) {
					if (!in_array($singleClass, $this->moduleSettings['puppetClient_enforceClasses'])) {
						$invalidClassFound = true;
						break;
					}
				}
				if ($invalidClassFound) {
					$msg = $this->messages['puppetclass'][2];
					$msg[] = $i;
					$messages[] = $msg;
				}
			}
			// variables
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_puppetvar', 'puppetvar', 'ascii', $this->messages['puppetvar'][1], $messages, '/;[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			return $return;
		}
		$this->addSimplePDFField($return, 'environment', _('Environment'));
		$this->addSimplePDFField($return, 'parentnode', _('Parent node'));
		if (isset($this->attributes['puppetclass'][0])) {
			$pdfTable = new PDFTable(_('Classes'));
			for ($i = 0; $i < sizeof($this->attributes['puppetclass']); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes['puppetclass'][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'puppetclass', $pdfTable);
		}
		if (isset($this->attributes['puppetvar'][0])) {
			$pdfTable = new PDFTable(_('Variables'));
			for ($i = 0; $i < sizeof($this->attributes['puppetvar']); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes['puppetvar'][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'puppetvar', $pdfTable);
		}
		return $return;
	}

	/**
	* {@inheritDoc}
	*/
	public function get_profileOptions($typeId) {
		$return = parent::get_profileOptions($typeId);
		$possibleParentNodes = $this->getPossibleParentNodes();
		array_unshift($possibleParentNodes, '-');
		$possibleParentNodes = array_values($possibleParentNodes);
		if (sizeof($possibleParentNodes) > 0) {
			$return->addElement(new htmlTableExtendedSelect('puppetClient_parentnode', $possibleParentNodes, array(), _('Parent node'), 'parentnode'), true);
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['puppetClient_addExt'][0]) && ($profile['puppetClient_addExt'][0] == "true")) {
			if (!in_array('puppetClient', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'puppetClient';
			}
		}
		// parent node
		if (isset($profile['puppetClient_parentnode'][0]) && ($profile['puppetClient_parentnode'][0] != "-")) {
			$this->attributes['parentnode'][0] = $profile['puppetClient_parentnode'][0];
		}
		// classes
		if (isset($profile['puppetClient_puppetclass'][0]) && ($profile['puppetClient_puppetclass'][0] != '')) {
			$this->attributes['puppetclass'] = $profile['puppetClient_puppetclass'];
		}
		// variables
		if (isset($profile['puppetClient_puppetvar'][0]) && ($profile['puppetClient_puppetvar'][0] != '')) {
			$this->attributes['puppetvar'] = $profile['puppetClient_puppetvar'];
		}
	}

	/**
	 * Reurns a list of valid parent nodes for this node.
	 *
	 * @return array parent nodes (e.g. array('node1', 'node2'))
	 */
	private function getPossibleParentNodes() {
		$possibleParentNodes = array();
		$searchResult = searchLDAPByAttribute('cn', '*', 'puppetClient', array('cn'), array('host'));
		$possibleParentNodes = array();
		for ($i = 0; $i < sizeof($searchResult); $i++) {
			if (!get_preg($searchResult[$i]['cn'][0], 'ascii')) {
				continue;
			}
			if (($this->getAccountContainer() == null) || $this->getAccountContainer()->isNewAccount
				|| (!isset($this->getAccountContainer()->attributes_orig['cn'][0])
					|| ($this->getAccountContainer()->attributes_orig['cn'][0] != $searchResult[$i]['cn'][0]))) {
				$possibleParentNodes[] = $searchResult[$i]['cn'][0];
			}
		}
		return $possibleParentNodes;
	}

	/**
	 * Loads cached data from LDAP such as classes etc.
	 */
	private function initCache() {
		if ($this->classCache != null) {
			return;
		}
		$attrs = array('puppetclass', 'puppetvar');
		$classes = array();
		$variables = array();
		$result = searchLDAPByFilter('(objectClass=puppetClient)', $attrs, array($this->get_scope()));
		foreach ($result as $attributes) {
			if (isset($attributes['puppetclass'])) {
				foreach ($attributes['puppetclass'] as $val) {
					$classes[] = $val;
				}
			}
			if (isset($attributes['puppetvar'])) {
				foreach ($attributes['puppetvar'] as $val) {
					$variables[] = $val;
				}
			}
		}
		$this->classCache = array_values(array_unique($classes));
		$this->variablesCache = array_values(array_unique($variables));
	}

	/**
	* Returns a list of configuration options.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The field names are used as keywords to load and save settings.
	* We recommend to use the module name as prefix for them (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all active account modules and their scopes (module => array(scopes))
	* @return mixed htmlElement or array of htmlElement
	*
	* @see htmlElement
	*/
	public function get_configOptions($scopes, $allScopes) {
		$return = parent::get_configOptions($scopes, $allScopes);
		// config options
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlTableExtendedInputTextarea('puppetClient_environments', "production\r\ntesting", 30, 5, _('Predefined environments'), 'predefinedEnvironments'), true);
		$configContainer->addElement(new htmlTableExtendedInputTextarea('puppetClient_enforceClasses', '', 30, 5, _('Enforce classes'), 'enforceClasses'), true);
		$return[] = $configContainer;
		return $return;
	}

}

?>
