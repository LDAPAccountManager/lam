<?php
/*
$Id$

This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
Copyright (C) 2003 - 2006  Tilo Lutz
              2007 - 2012  Roland Gruber

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
* Manages Unix accounts for groups.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "posixGroup" for groups.
*
* @package modules
*/
class posixGroup extends baseModule implements passwordService {

	/** change GIDs of users and hosts? */
	private $changegids;
	/** specifies if the cn attribute should be managed by this module */
	protected $manageCnAttribute = true;
	/** specifies if the description attribute should be managed by this module */
	protected $manageDescriptionAttribute = true;
	/** cache for existing GID numbers */
	private $cachedGIDList = null;
	/** cache for existing users and their GIDs */
	private $cachedUserToGIDList = null;
	
	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$error_messages = array();
		$needAutoGID = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixGroup", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixGroup";
			if ($this->manageCnAttribute) {
				// group name
				if (get_preg($rawAccounts[$i][$ids['posixGroup_cn']], 'groupname')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixGroup_cn']];
				}
				else {
					$errMsg = $this->messages['cn'][3];
					array_push($errMsg, array($i));
					$error_messages[] = $errMsg;
				}
			}
			// GID
			if ($rawAccounts[$i][$ids['posixGroup_gid']] == "") {
				// autoGID
				$needAutoGID[] = $i;
			}
			elseif (get_preg($rawAccounts[$i][$ids['posixGroup_gid']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixGroup_gid']];
			}
			else {
				$errMsg = $this->messages['gidNumber'][8];
				array_push($errMsg, array($i));
				$error_messages[] = $errMsg;
			}
			if ($this->manageDescriptionAttribute) {
				// description (UTF-8, no regex check needed)
				if ($rawAccounts[$i][$ids['posixGroup_description']] == "") {
					$partialAccounts[$i]['description'] = $partialAccounts[$i]['cn'];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixGroup_description']];
				}
			}
			// group members
			if ($rawAccounts[$i][$ids['posixGroup_members']] != "") {
				if (get_preg($rawAccounts[$i][$ids['posixGroup_members']], 'usernameList')) {
					$partialAccounts[$i]['memberUid'] = explode(",", $rawAccounts[$i][$ids['posixGroup_members']]);
				}
				else {
					$errMsg = $this->messages['memberUID'][0];
					array_push($errMsg, $i);
					$error_messages[] =$errMsg;
				}
			}
			// password
			if ($rawAccounts[$i][$ids['posixGroup_password']] != "") {
				if (get_preg($rawAccounts[$i][$ids['posixGroup_password']], 'password')) {
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixGroup_password']], true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				else {
					$error_messages[] = $this->messages['userPassword'][1];
				}
			}
		}
		// fill in autoGIDs
		if (sizeof($needAutoGID) > 0) {
			$errorsTemp = array();
			$gids = $this->getNextGIDs(sizeof($needAutoGID), $errorsTemp);
			if (is_array($gids)) {
				for ($i = 0; $i < sizeof($needAutoGID); $i++) {
					$partialAccounts[$i]['gidNumber'] = $gids[$i];
				}
			}
			else {
				$error_messages[] = $this->messages['gidNumber'][2];
			}
		}
		return $error_messages;
	}

	/**
	* Checks if the group which should be deleted is still used as primary group.
	*
	* @return List of LDAP operations, same as for save_attributes()
	*/
	function delete_attributes() {
		$return = array();
		$result = searchLDAPByFilter('(&(objectClass=posixAccount)(gidNumber=' . $this->attributes['gidNumber'][0] . '))', array('dn'), array('user', 'host'));
		if (sizeof($result) > 0) {
			$max = 5;
			if (sizeof($result) < 5) {
				$max = sizeof($result);
			}
			$users = array();
			for ($i = 0; $i < $max; $i++) {
				$users[] = getAbstractDN($result[$i]['dn']);
			}
			$message = $this->messages['primaryGroup'][0];
			$message[] = implode(', ', $users);
			$return[$this->getAccountContainer()->dn_orig]['errors'][] = $message;
		}
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 * 
	 * @see baseModule::get_metaData()
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// group name
		if ($this->manageCnAttribute) {
			$cn = '';
			if (isset($this->attributes['cn'][0])) {
				$cn = $this->attributes['cn'][0];
			}
			$cnInput = new htmlTableExtendedInputField(_("Group name"), 'cn', $cn, 'cn');
			$cnInput->setRequired(true);
			$cnInput->setFieldMaxLength(30);
			$return->addElement($cnInput, true);
		}
		// GID number
		$gidNumber = '';
		if (isset($this->attributes['gidNumber'][0])) {
			$gidNumber = $this->attributes['gidNumber'][0];
		}
		$gidNumberInput = new htmlTableExtendedInputField(_('GID number'), 'gidNumber', $gidNumber, 'gidNumber');
		$gidNumberInput->setFieldMaxLength(20);
		$gidNumberInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($gidNumberInput, true);
		// description
		$description = '';
		if (isset($this->attributes['description'][0])) {
			$description = $this->attributes['description'][0];
		}
		if ($this->manageDescriptionAttribute) {
			$return->addElement(new htmlTableExtendedInputField(_('Description'), 'description', $description, 'description'), true);
		}
		// group members
		$return->addElement(new htmlOutputText(_("Group members")));
		$return->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Edit members')));
		$return->addElement(new htmlHelpLink('members'), true);
		// password buttons
		if (checkIfWriteAccessIsAllowed() && isset($this->attributes['userPassword'][0])) {
			$return->addElement(new htmlOutputText(_('Password')));
			$pwdContainer = new htmlTable();
			if (pwd_is_enabled($this->attributes['userPassword'][0])) {
				$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
			}
			else {
				$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
			}
			$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
			$pwdContainer->colspan = 2;
			$return->addElement($pwdContainer, true);
		}
		if (isset($this->orig['gidNumber'][0]) && $this->attributes['gidNumber'][0]!=$this->orig['gidNumber'][0]) {
			$return->addElement(new htmlTableExtendedInputCheckbox('changegids', $this->changegids, _('Change GID number of users and hosts'), 'changegids'));
		}
		return $return;
	}


	/**
	* Displays selections to add or remove users from current group.
	*
	* @return array meta HTML output
	*/
	function display_html_user() {
		$return = new htmlTable();
		if (!isset($this->attributes['memberUid'])) {
			$this->attributes['memberUid'] = array();
		}
		// load list with all users
		$userAndGIDs = $this->getUserAndGIDs();
		$users = array();
		foreach ($userAndGIDs as $user => $GID) {
			if (!in_array($user, $this->attributes['memberUid'])) {
				if ($this->attributes['gidNumber'][0] == $GID) {
					if (isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
						&& ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] == 'true')) {
						$users[] = $user;
					}
			}
			else {
				$users[] = $user;
			}
		}
	}
	$return->addElement(new htmlSubTitle(_("Group members")), true);
	
	$return->addElement(new htmlOutputText(_("Selected users")));
	$return->addElement(new htmlOutputText(''));
	$return->addElement(new htmlOutputText(_("Available users")));
	$return->addNewLine();
	
	$remGroups = array();
	if (isset($this->attributes['memberUid'])) {
		$remGroups = $this->attributes['memberUid'];
	}
	$remSelect = new htmlSelect('removeusers', $remGroups, null, 15);
	$remSelect->setMultiSelect(true);
	$remSelect->setTransformSingleSelect(false);
	$return->addElement($remSelect);
	$buttonContainer = new htmlTable();
	$buttonContainer->addElement(new htmlButton('addusers_button', 'back.gif', true), true);
	$buttonContainer->addElement(new htmlButton('removeusers_button', 'forward.gif', true), true);
	$buttonContainer->addElement(new htmlHelpLink('members'));
	$return->addElement($buttonContainer);
	$addSelect = new htmlSelect('addusers', $users, null, 15);
	$addSelect->setMultiSelect(true);
	$addSelect->setTransformSingleSelect(false);
	$return->addElement($addSelect);
	$return->addNewLine();

	// back button
	$return->addElement(new htmlSpacer(null, '10px'), true);
	$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')));
	
	return $return;
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'tux.png';
		// manages group accounts
		$return["account_types"] = array("group");
		if ($this->get_scope() == "group") {
			// this is a base module
			$return["is_base"] = true;
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixGroup)");
		}
		// alias name
		$return["alias"] = _('Unix');
		// RDN attribute
		$return["RDN"] = array("cn" => "normal");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('posixGroup');
		// LDAP aliases
		$return['LDAPaliases'] = array('commonName' => 'cn');
		// managed attributes
		$return['attributes'] = array('cn', 'gidNumber', 'userPassword', 'memberUid', 'description');
		// configuration options
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlSubTitle(_("Groups")), true);
		$minGidInput = new htmlTableExtendedInputField(_('Minimum GID number'), 'posixGroup_minGID', null, 'minMaxGID');
		$minGidInput->setRequired(true);
		$configContainer->addElement($minGidInput, true);
		$maxGidInput = new htmlTableExtendedInputField(_('Maximum GID number'), 'posixGroup_maxGID', null, 'minMaxGID');
		$maxGidInput->setRequired(true);
		$configContainer->addElement($maxGidInput, true);
		$return['config_options']['group'] = $configContainer;
		// configuration checks
		$return['config_checks']['group']['posixGroup_minGID'] = array (
			'type' => 'ext_preg',
			'regex' => 'digit',
			'required' => true,
			'required_message' => $this->messages['gidNumber'][5],
			'error_message' => $this->messages['gidNumber'][5]);
		$return['config_checks']['group']['posixGroup_maxGID'] = array (
			'type' => 'ext_preg',
			'regex' => 'digit',
			'required' => true,
			'required_message' => $this->messages['gidNumber'][6],
			'error_message' => $this->messages['gidNumber'][6]);
		$return['config_checks']['group']['cmpGID'] = array (
			'type' => 'int_greater',
			'cmp_name1' => 'posixGroup_maxGID',
			'cmp_name2' => 'posixGroup_minGID',
			'error_message' => $this->messages['gidNumber'][7]);
		// available PDF fields
		$return['PDF_fields'] = array(
			'gidNumber' => _('GID number'),
			'memberUid' => _('Group members')
		);
		if ($this->manageCnAttribute) {
			$return['PDF_fields']['cn'] = _('Group name');
		}
		if ($this->manageDescriptionAttribute) {
			$return['PDF_fields']['description'] = _('Description');
		}
		// upload fields
		$return['upload_columns'] = array(
		array(
			'name' => 'posixGroup_gid',
			'description' => _('GID number'),
			'help' => 'gidNumber',
			'example' => '2034'
		),
		array(
			'name' => 'posixGroup_members',
			'description' => _('Group members'),
			'help' => 'upload_members',
			'example' => _('user01,user02,user03')
		),
		array(
			'name' => 'posixGroup_password',
			'description' => _('Group password'),
			'help' => 'password',
			'example' => _('secret')
		)
		);
		if ($this->manageCnAttribute) {
			array_unshift($return['upload_columns'], 
				array(
					'name' => 'posixGroup_cn',
					'description' => _('Group name'),
					'help' => 'cn',
					'example' => _('adminstrators'),
					'required' => true,
					'unique' => true
				)
			);
		}
		if ($this->manageDescriptionAttribute) {
			array_unshift($return['upload_columns'],
				array(
				'name' => 'posixGroup_description',
				'description' => _('Group description'),
				'help' => 'description',
				'example' => _('Administrators group')
				)
			);
		}
		// help Entries
		$return['help'] = array(
			'gidNumber'	=> array(
				"Headline" => _("GID number"), 'attr' => 'gidNumber',
				"Text" => _("If empty GID number will be generated automaticly depending on your configuration settings.")
			),
			'description' => array(
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Group description. If left empty group name will be used.")
			),
			'members' => array(
				"Headline" => _("Group members"), 'attr' => 'memberUid',
				"Text" => _("Users who are member of the current group. Users who have set their primary group to this group will not be shown.")
			),
			'upload_members' => array(
				"Headline" => _("Group members"), 'attr' => 'memberUid',
				"Text" => _("Users who will become member of the current group. User names are separated by semicolons.")
			),
			'password' => array(
				"Headline" => _("Group password"), 'attr' => 'userPassword',
				"Text" => _("Sets the group password.")
			),
			'minMaxGID' => array(
				"Headline" => _("GID number"),
				"Text" => _("These are the minimum and maximum numbers to use for group IDs when creating new group accounts. New group accounts will always get the highest number in use plus one.")
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'cn' => array(
				"Headline" => _("Group name"), 'attr' => 'cn',
				"Text" => _("Group name of the group which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ . If group name is already used group name will be expanded with a number. The next free number will be used.")
			),
			'changegids' => array(
				"Headline" => _("Change GID number of users and hosts"),
				"Text" => _("The ID of this group was changed. You can update all user and host entries to the new group ID.")
			)
		);

		return $return;
	}


	/**
	* Returns a list of elements for the configuration.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all modules and active scopes
	* @return array configuration elements
	*/
	function get_configOptions($scopes, $allScopes) {
		$return = parent::get_configOptions($scopes, $allScopes);
		// display password hash option only if posixAccount module is not used
		if (!isset($allScopes['posixAccount'])) {
			$return[0]->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), array('SSHA'), _("Password hash type"), 'pwdHash'));
		}
		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$members = '';
		if (isset($this->attributes['memberUid'][0])) {
			$members = implode(', ', $this->attributes['memberUid']);
		}
		return array(
			get_class($this) . '_cn' => array('<block><key>' . _('Group name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			get_class($this) . '_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
			get_class($this) . '_memberUid' => array('<block><key>' . _('Group members') . '</key><value>' . $members . '</value></block>'),
			get_class($this) . '_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>')
		);
	}


	/** This functin will be called when the module will be loaded **/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->changegids=false;
	}


	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['gidNumber'][0] = array('INFO', _('GID number'), _('GID number has changed. Please select checkbox to change GID number of users and hosts.'));
		$this->messages['gidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
		$this->messages['gidNumber'][3] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
		$this->messages['gidNumber'][4] = array('ERROR', _('ID-Number'), _('ID is already in use'));
		$this->messages['gidNumber'][5] = array('ERROR', _('Minimum GID number'), _('Minimum GID number is invalid or empty!'));
		$this->messages['gidNumber'][6] = array('ERROR', _('Maximum GID number'), _('Maximum GID number is invalid or empty!'));
		$this->messages['gidNumber'][7] = array('ERROR', _('Maximum GID number'), _('Maximum GID number must be greater than minimum GID number!'));
		$this->messages['gidNumber'][8] = array('ERROR', _('Account %s:') . ' posixGroup_gid', _('GID number has to be a numeric value!'));
		$this->messages['cn'][0] = array('WARN', _('Group name'), _('You are using a capital letters. This can cause problems because Windows isn\'t case-sensitive.'));
		$this->messages['cn'][1] = array('WARN', _('Group name'), _('Group name in use. Selected next free group name.'));
		$this->messages['cn'][2] = array('ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][3] = array('ERROR', _('Account %s:') . ' posixGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['memberUID'][0] = array('ERROR', _('Account %s:') . ' posixGroup_members', _("This value must be a list of user names separated by semicolons."));
		$this->messages['primaryGroup'][0] = array('ERROR', _('There are still users who have this group as their primary group.'));
	}


	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('posixGroup', $objectClasses)) {
				return true;
			}
		}
		if ($this->manageCnAttribute && ($this->attributes['cn'][0] == '')) return false;
		if ($this->attributes['gidNumber'][0] == '') return false;
		return true;
	}


	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('posixGroup', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}


	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		if ($this->manageDescriptionAttribute) {
			$this->attributes['description'][0] = $_POST['description'];
		}
		if (isset($_POST['lockPassword'])) {
			$this->attributes['userPassword'][0] = pwd_disable($this->attributes['userPassword'][0]);
		}
		if (isset($_POST['unlockPassword'])) {
			$this->attributes['userPassword'][0] = pwd_enable($this->attributes['userPassword'][0]);
		}
		if (isset($_POST['removePassword'])) {
			unset($this->attributes['userPassword']);
		}
		if (isset($_POST['changegids'])) $this->changegids=true;
		else $this->changegids=false;
		if (!isset($this->attributes['gidNumber'][0]) || ($this->attributes['gidNumber'][0] != $_POST['gidNumber'])) {
			// Check if GID is valid. If none value was entered, the next useable value will be inserted
			// load min and max GID number
			$minID = intval($this->moduleSettings['posixGroup_minGID'][0]);
			$maxID = intval($this->moduleSettings['posixGroup_maxGID'][0]);
			$this->attributes['gidNumber'][0] = $_POST['gidNumber'];
			if ($this->attributes['gidNumber'][0]=='') {
				// No id-number given, find free GID
				if (!isset($this->orig['gidNumber'][0])) {
					$newGID = $this->getNextGIDs(1, $errors);
					if (is_array($newGID)) {
						$this->attributes['gidNumber'][0] = $newGID[0];
					}
					else {
						$errors[] = $this->messages['gidNumber'][3];
					}
				}
				else $this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
				// old account -> return id-number which has been used
			}
			else {
				$gids = $this->getGIDs();
				// Check manual ID
				if ($this->getAccountContainer()->isNewAccount || !isset($this->orig['gidNumber'][0]) || ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0])) {
					// check range
					if (($this->attributes['gidNumber'][0] < $minID) || ($this->attributes['gidNumber'][0] > $maxID) || !is_numeric($this->attributes['gidNumber'][0])) {
						$errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
						if (isset($this->orig['gidNumber'][0])) $this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
						else unset($this->attributes['gidNumber'][0]);
					}
					// $uids is allways an array but not if no entries were found
					if (is_array($gids)) {
						// id-number is in use and account is a new account
						if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]=='') {
							$errors[] = $this->messages['gidNumber'][4];
							unset($this->attributes['gidNumber'][0]);
						}
						// id-number is in use, account is existing account and id-number is not used by itself
						if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]!='' && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0]) ) {
							$errors[] = $this->messages['gidNumber'][4];
							$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
						}
					}
				}
			}
		}
		if ($this->manageCnAttribute && ($this->attributes['cn'][0]!=$_POST['cn'])) {
			$this->attributes['cn'][0] = $_POST['cn'];
			if (preg_match('/^[A-Z]+$/', $_POST['cn'])) {
				$errors[] = $this->messages['cn'][0];
			}
			// Check if Groupname contains only valid characters
			if ( !get_preg($this->attributes['cn'][0],'groupname'))
			$errors[] = $this->messages['cn'][2];
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			// Set username back to original name if new username is in use
			if ((sizeof(searchLDAPByAttribute('cn', $this->attributes['cn'][0], 'posixGroup', array('cn'), array('group'))) > 0) && ($this->orig['cn'][0]!='')) {
				$this->attributes['cn'][0] = $this->orig['cn'][0];
			}
			// Change gid to a new gid until a free gid is found
			else while (sizeof(searchLDAPByAttribute('cn', $this->attributes['cn'][0], 'posixGroup', array('cn'), array('group'))) > 0) {
				// get last character of username
				$lastchar = substr($this->attributes['cn'][0], strlen($this->attributes['cn'][0])-1, 1);
				// Last character is no number
				if ( !preg_match('/^([0-9])+$/', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				$this->attributes['cn'][0] = $this->attributes['cn'][0] . '2';
				else {
					/* Last character is a number -> we have to increase the number until we've
					* found a groupname with trailing number which is not in use.
					*
					* $i will show us were we have to split groupname so we get a part
					* with the groupname and a part with the trailing number
					*/
					$i=strlen($this->attributes['cn'][0])-1;
					$mark = false;
					// Set $i to the last character which is a number in $account_new->general_username
					while (!$mark) {
						if (preg_match('/^([0-9])+$/',substr($this->attributes['cn'][0], $i, strlen($this->attributes['cn'][0])-$i))) $i--;
						else $mark=true;
					}
					// increase last number with one
					$firstchars = substr($this->attributes['cn'][0], 0, $i+1);
					$lastchars = substr($this->attributes['cn'][0], $i+1, strlen($this->attributes['cn'][0])-$i);
					// Put username together
					$this->attributes['cn'][0] = $firstchars . (intval($lastchars)+1);
				}
			}
			// Show warning if lam has changed username
			if ($this->attributes['cn'][0] != $_POST['cn']) {
				$errors[] = $this->messages['cn'][1];
			}
			// show info when gidnumber has changed
			if (($this->orig['gidNumber'][0]!=$this->attributes['gidNumber'][0]) && $this->orig['gidNumber'][0]!='' && $_POST['gidNumber']!=$this->attributes['gidNumber'][0]) {
				$errors[] = $this->messages['gidNumber'][0];
			}
		}
		// Return error-messages
		return $errors;
	}


	/**
	* Processes user input of the user selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_user() {
		if (!isset($this->attributes['memberUid'])) $this->attributes['memberUid'] = array();
		if (isset($_POST['addusers']) && isset($_POST['addusers_button'])) { // Add users to list
			// Add new user
			$this->attributes['memberUid'] = @array_merge($this->attributes['memberUid'], $_POST['addusers']);
		}
		elseif (isset($_POST['removeusers']) && isset($_POST['removeusers_button'])) { // remove users from list
			$this->attributes['memberUid'] = array_delete($_POST['removeusers'], $this->attributes['memberUid']);
		}
		return array();
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
		// skip saving if account is based on another structural object class
		if (!$this->getAccountContainer()->isNewAccount && !in_array('posixGroup', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return array();
		}
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// Change gids of users and hosts?
		if ($this->changegids) {
			// find all accounts to change
			$result = searchLDAPByFilter('(&(objectClass=posixAccount)(gidNumber=' . $this->orig['gidNumber'][0] . '))', array('dn'), array('user', 'host'));
			if (sizeof($result) > 0) {
				for ($i = 0; $i < sizeof($result); $i++) {
					$return[$result[$i]['dn']]['modify']['gidNumber'][0] = $this->attributes['gidNumber'][0];
				}
			}
		}
		return $return;
	}


	/**
	* Returns one or more free GID numbers.
	*
	* @param integer $count Number of needed free GIDs.
	* @param array $errors list of error messages where errors can be added
	* @return mixed Null if no GIDs are free else an array of free GIDs.
	*/
	function getNextGIDs($count, &$errors) {
		$ret = array();
		$minID = intval($this->moduleSettings['posixGroup_minGID'][0]);
		$maxID = intval($this->moduleSettings['posixGroup_maxGID'][0]);
		$gidList = $this->getGIDs();
		$gids = array();
		foreach ($gidList as $gid) {
			if (($gid <= $maxID) && ($gid >= $minID)) $gids[] = $gid;  // ignore GIDs > maxID and GIDs < minID
		}
		for ($i = 0; $i < $count; $i++) {
			if (count($gids) != 0) {
				// there already are some GIDs
				// store highest id-number
				$id = $gids[count($gids)-1];
				// Return minimum allowed id-number if all found id-numbers are too low
				if ($id < $minID) {
					$ret[] = $minID;
					$gids[] = $minID;
				}
				// return highest used id-number + 1 if it's still in valid range
				elseif ($id < $maxID) {
					$ret[] = $id + 1;
					$gids[] = $id + 1;
				}
				// find free numbers between existing ones
				else {
					$k = intval($minID);
					while (in_array($k, $gids)) $k++;
					if ($k > $maxID) return null;
					else {
						$ret[] = $k;
						$gids[] = $k;
						sort ($gids, SORT_NUMERIC);
					}
					// show warning message
					$errors[] = $this->messages['gidNumber'][2];
				}
			}
			else {
				// return minimum allowed id-number if no id-numbers are found
				$ret[] = $minID;
				$gids[] = $minID;
			}
		}
		return $ret;
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		return true;
	}

	/**
	 * Specifies if this module supports to force that a user must change his password on next login.
	 * 
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange() {
		return false;
	}
	
	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$this->attributes['userPassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		return array();
	}
	
	/**
	 * Returns a list of existing GID numbers.
	 *
	 * @return array list of GID numbers
	 */
	private function getGIDs() {
		if ($this->cachedGIDList != null) {
			return $this->cachedGIDList;
		}
		$result = searchLDAPByAttribute('gidNumber', '*', 'posixGroup', array('gidNumber'), array('group'));
		$this->cachedGIDList = array();
		for ($i = 0; $i < sizeof($result); $i++) {
			$this->cachedGIDList[] = $result[$i]['gidnumber'][0];
		}
		sort($this->cachedGIDList, SORT_NUMERIC);
		return $this->cachedGIDList;
	}
	
	/**
	 * Returns a list of existing users and their GID numbers.
	 *
	 * @return array list in format array(uid => gidNumber)
	 */
	private function getUserAndGIDs() {
		if ($this->cachedUserToGIDList != null) {
			return $this->cachedUserToGIDList;
		}
		$result = searchLDAPByAttribute('gidNumber', '*', 'posixAccount', array('uid', 'gidNumber'), array('user'));
		$this->cachedUserToGIDList = array();
		for ($i = 0; $i < sizeof($result); $i++) {
			$this->cachedUserToGIDList[$result[$i]['uid'][0]] = $result[$i]['gidnumber'][0];
		}
		return $this->cachedUserToGIDList;
	}
	
}

?>
