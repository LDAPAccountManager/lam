<?php
use \LAM\TYPES\TypeManager;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2017  Roland Gruber

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
* Manages PyKota group accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages PyKota group accounts.
*
* @package modules
*/
class pykotaGroup extends baseModule {

	/** cache for cn attribute */
	private $cnCache = null;
	/** cache for pykotaGroupName attribute */
	private $pykotaGroupNameCache = null;
	/** list of limit options label => value */
	private $limitOptions;

	/**
	 * Returns if this module also manages the structural object class pykotaObject.
	 * This is overridden by a submodule that must provide the structural object class.
	 *
	 * @return boolean structural usage
	 */
	public function isStructural() {
		return false;
	}

	/**
	* Creates a new pykotaGroup object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		$this->limitOptions = array(
			_('Quota') => 'quota',
			_('Balance') => 'balance',
			_('No quota') => 'noquota',
			_('Free printing') => 'nochange',
			_('Deny printing') => 'noprint',
		);
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = $this->isStructural();
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('group'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'printerBig.png';
		// alias name
		$return["alias"] = _("PyKota");
		// this is a base module
		$return["is_base"] = $this->isStructural();
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=pykotaGroup)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('pykotaGroup');
		// managed attributes
		$return['attributes'] = array('pykotaLimitBy', 'pykotaGroupName');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("Group name of the group which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ .")
			),
			'description' => array (
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Group description.")
			),
			'pykotaGroupName' => array(
				"Headline" => _("PyKota group name"), 'attr' => 'pykotaGroupName',
				"Text" => _("Group name that is used for PyKota.")
			),
			'pykotaLimitBy' => array(
				"Headline" => _("Limit type"), 'attr' => 'pykotaLimitBy',
				"Text" => _("Specifies the type of limit for printing if any. Please note that in contrast to \"Free printing\" the option \"No quota\" includes accounting.")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
		);
		// profile options
		$profileContainer = new htmlTable();
		$pykotaLimitByProfileOption = new htmlTableExtendedSelect('pykotaGroup_pykotaLimitBy', $this->limitOptions, array(), _('Limit type'), 'pykotaLimitBy');
		$pykotaLimitByProfileOption->setHasDescriptiveElements(true);
		$pykotaLimitByProfileOption->setSortElements(false);
		$profileContainer->addElement($pykotaLimitByProfileOption, true);
		if (!$this->isStructural()) {
			$profileContainer->addElement(new htmlTableExtendedInputCheckbox('pykotaGroup_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		}
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings']['pykotaGroup_pykotaLimitBy'] = 'pykotaLimitBy';
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'pykotaGroup_pykotaGroupName',
				'description' => _('PyKota group name'),
				'help' => 'pykotaGroupName',
				'example' => _('adminstrators'),
				'unique' => true,
			)
		);
		$return['upload_columns'][] = array(
			'name' => 'pykotaGroup_pykotaLimitBy',
			'description' => _('Limit type'),
			'help' => 'pykotaLimitBy',
			'example' => _('Quota'),
			'default' => _('Quota'),
			'values' => implode(', ', array_keys($this->limitOptions))
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'pykotaGroupName' => _('PyKota group name'),
			'pykotaLimitBy' => _('Limit type'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' pykotaGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][2] = array('ERROR', _('Group name'), _('Group name already exists!'));
		$this->messages['cn'][3] = array('ERROR', _('Account %s:') . ' pykotaGroup_cn', _('Group name already exists!'));
		$this->messages['pykotaGroupName'][0] = array('ERROR', _('PyKota group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaGroupName'][1] = array('ERROR', _('Account %s:') . ' pykotaGroup_pykotaGroupName', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaGroupName'][2] = array('ERROR', _('PyKota group name'), _('Group name already exists!'));
		$this->messages['pykotaGroupName'][3] = array('ERROR', _('Account %s:') . ' pykotaGroup_pykotaGroupName', _('Group name already exists!'));
		$this->messages['pykotaLimitBy'][0] = array('ERROR', _('Account %s:') . ' pykotaGroup_pykotaLimitBy', _('Please enter a valid limit type.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if ($this->isStructural() || (isset($this->attributes['objectClass']) && in_array('pykotaGroup', $this->attributes['objectClass']))) {
			// cn
			if ($this->isUnixOrGonInactive($modules)) {
				$this->addSimpleInputTextField($container, 'cn', _('Group name'), true);
			}
			// pykotaGroupName
			$this->addSimpleInputTextField($container, 'pykotaGroupName', _('Pykota group name'));
			// limit by
			$limitOption = 'quota';
			if (!empty($this->attributes['pykotaLimitBy'][0])) {
				$limitOption = $this->attributes['pykotaLimitBy'][0];
			}
			$limitSelect = new htmlTableExtendedSelect('pykotaLimitBy', $this->limitOptions, array($limitOption), _('Limit type'), 'pykotaLimitBy');
			$limitSelect->setHasDescriptiveElements(true);
			$limitSelect->setSortElements(false);
			$container->addElement($limitSelect, true);
			// description
			if ($this->isUnixOrGonInactive($modules)) {
				$this->addMultiValueInputTextField($container, 'description', _('Description'), false, null, true);
			}
			// remove button
			if (!$this->isStructural()) {
				$container->addElement(new htmlSpacer(null, '20px'), true);
				$remButton = new htmlButton('remObjectClass', _('Remove PyKota extension'));
				$remButton->colspan = 5;
				$container->addElement($remButton);
			}
		}
		else {
			// add button
			$container->addElement(new htmlButton('addObjectClass', _('Add PyKota extension')));
		}
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		if (isset($_POST['addObjectClass'])) {
			if (!isset($this->attributes['objectClass'])) {
				$this->attributes['objectClass'] = array();
			}
			if (!in_array('pykotaGroup', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaGroup';
			}
			if (!isset($this->attributes['pykotaGroupName'][0])) {
				$this->attributes['pykotaGroupName'][0] = $this->getCurrentGroupName();
			}
			return $errors;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('pykotaGroup'), $this->attributes['objectClass']);
			$attrs = array('pykotaLimitBy', 'pykotaGroupName');
			if ($this->isUnixOrGonInactive($modules)) {
				$attrs[] = 'description';
			}
			if ($this->isUnixOrGonInactive($modules)) {
				$attrs[] = 'cn';
			}
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('pykotaGroup', $this->attributes['objectClass'])) {
			return $errors;
		}
		// cn
		if ($this->isUnixOrGonInactive($modules)) {
			if (isset($_POST['cn']) && ($_POST['cn'] != '')) {
				if (!get_preg($_POST['cn'], 'groupname')) {
					$errors[] = $this->messages['cn'][0];
				}
				else {
					$this->attributes['cn'][0] = $_POST['cn'];
					if ((!isset($this->orig['cn'][0]) || ($this->attributes['cn'][0] != $this->orig['cn'][0]))
						&& $this->cnExists($_POST['cn'])) {
						$errors[] = $this->messages['cn'][2];
					}
				}
			}
			elseif (isset($this->attributes['cn'][0])) {
				unset($this->attributes['cn'][0]);
			}
		}
		// PyKota group name
		if (!empty($_POST['pykotaGroupName'])) {
			if (!get_preg($_POST['pykotaGroupName'], 'groupname')) {
				$errors[] = $this->messages['pykotaGroupName'][0];
			}
			else {
				$this->attributes['pykotaGroupName'][0] = $_POST['pykotaGroupName'];
				if ((!isset($this->orig['pykotaGroupName'][0]) || ($this->attributes['pykotaGroupName'][0] != $this->orig['pykotaGroupName'][0]))
					&& $this->pykotaGroupNameExists($_POST['pykotaGroupName'])) {
					$errors[] = $this->messages['pykotaGroupName'][2];
				}
			}
		}
		else {
			$this->attributes['pykotaGroupName'][0] = $this->getCurrentGroupName();
		}
		// limit by
		$this->attributes['pykotaLimitBy'][0] = $_POST['pykotaLimitBy'];
		// description
		if ($this->isUnixOrGonInactive($modules)) {
			$this->processMultiValueInputTextField('description', $errors);
		}
		return $errors;
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
		if (!in_array('pykotaGroup', $this->attributes['objectClass']) && !in_array('pykotaGroup', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	public function getManagedAttributes($typeId) {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		if ($this->isUnixOrGonInactive($typeManager->getConfiguredType($typeId)->getModules())) {
			$attrs[] = 'cn';
			$attrs[] = 'description';
		}
		return $attrs;
	}

	/**
	* This function is used to check if all settings for this module have been made.
	*
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This function tells LAM if it can create/modify the LDAP account. If your module needs any
	* additional input then set this to false. The user will be notified that your module needs
	* more input.<br>
	* This method's return value defaults to true.
	*
	* @return boolean true, if settings are complete
	*/
	public function module_complete() {
		if (in_array('pykotaGroup', $this->attributes['objectClass'])) {
			// require cn
			$cn = $this->getCurrentGroupName();
			return !empty($cn);
		}
		return true;
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
		if (isset($profile['pykotaGroup_addExt'][0]) && ($profile['pykotaGroup_addExt'][0] == "true")) {
			if (!in_array('pykotaGroup', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaGroup';
			}
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	function get_uploadColumns($selectedModules, &$type) {
		$return = parent::get_uploadColumns($selectedModules, $type);
		if ($this->isUnixOrGonInactive($selectedModules)) {
			$return[] = array(
				'name' => 'pykotaGroup_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('adminstrators'),
				'required' => true,
			);
			$return[] = array(
				'name' => 'pykotaGroup_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Administrators group'),
			);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('pykotaGroup', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaGroup';
			}
			if ($this->isStructural() && !in_array('pykotaObject', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaObject';
			}
			// cn
			if ($this->isUnixOrGonInactive($selectedModules) && !empty($rawAccounts[$i][$ids['pykotaGroup_cn']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaGroup_cn']], 'groupname')) {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				elseif ($this->cnExists($rawAccounts[$i][$ids['pykotaGroup_cn']])) {
					$errMsg = $this->messages['cn'][3];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['pykotaGroup_cn']];
				}
			}
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaGroup_description', 'description');
			// PyKota group name
			if (!empty($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']], 'groupname')) {
					$errMsg = $this->messages['pykotaGroupName'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				elseif ($this->pykotaGroupNameExists($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']])) {
					$errMsg = $this->messages['pykotaGroupName'][3];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaGroupName'] = $rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']];
				}
			}
			// limit by
			if (!empty($rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']])) {
				if (isset($this->limitOptions[$rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']]])) {
					$partialAccounts[$i]['pykotaLimitBy'] = $this->limitOptions[$rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']]];
				}
				else {
					$errMsg = $this->messages['pykotaLimitBy'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['pykotaLimitBy'] = 'quota';
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfFields()
	 */
	public function get_pdfFields($typeId) {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->isUnixOrGonInactive($modules)) {
			$fields['cn'] = _('Common name');
			$fields['description'] = _('Description');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'pykotaGroupName', _('PyKota group name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$limitByOptions = array_flip($this->limitOptions);
		$limitByValue = '';
		if (!empty($this->attributes['pykotaLimitBy'][0]) && isset($limitByOptions[$this->attributes['pykotaLimitBy'][0]])) {
			$limitByValue = $limitByOptions[$this->attributes['pykotaLimitBy'][0]];
		}
		$this->addPDFKeyValue($return, 'pykotaLimitBy', _('Limit type'), $limitByValue);
		return $return;
	}

	/**
	 * Checks if the active modules contain Unix or group of names.
	 *
	 * @param string[] $modules modules
	 * @return bool not active
	 */
	private function isUnixOrGonInactive($modules) {
		if (in_array('posixGroup', $modules)
			|| in_array('groupOfNames', $modules)
			|| in_array('groupOfUniqueNames', $modules)) {
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * Returns if the given cn already exists.
	 *
	 * @param String $cn cn attribute value
	 * @return boolean cn exists
	 */
	private function cnExists($cn) {
		if ($this->cnCache == null) {
			$this->loadGroupNameCache();
		}
		return in_array($cn, $this->cnCache);
	}

	/**
	 * Returns if the given pykotaGroupName already exists.
	 *
	 * @param String $pykotaGroupName pykotaGroupName attribute value
	 * @return boolean pykotaGroupName exists
	 */
	private function pykotaGroupNameExists($pykotaGroupName) {
		if ($this->pykotaGroupNameCache == null) {
			$this->loadGroupNameCache();
		}
		return in_array($pykotaGroupName, $this->pykotaGroupNameCache);
	}

	/**
	 * Loads the list of group names into the cache.
	 */
	private function loadGroupNameCache() {
		$results = searchLDAPByFilter('(objectClass=pykotaGroup)', array('cn', 'pykotaGroupName'), array($this->get_scope()));
		$this->cnCache = array();
		$this->pykotaGroupNameCache = array();
		foreach ($results as $result) {
			if (isset($result['cn'][0])) {
				$this->cnCache[] = $result['cn'][0];
			}
			if (isset($result['pykotagroupname'][0])) {
				$this->pykotaGroupNameCache[] = $result['pykotagroupname'][0];
			}
		}
	}

	/**
	 * Returns the current group name (cn) of this account.
	 *
	 * @return String group name
	 */
	private function getCurrentGroupName() {
		if (!empty($this->attributes['cn'][0])) {
			return $this->attributes['cn'][0];
		}
		if ($this->getAccountContainer()->getAccountModule('posixGroup') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('posixGroup');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		if ($this->getAccountContainer()->getAccountModule('groupOfNames') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('groupOfNames');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		if ($this->getAccountContainer()->getAccountModule('groupOfUniqueNames') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('groupOfUniqueNames');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		return '';
	}

}


?>
