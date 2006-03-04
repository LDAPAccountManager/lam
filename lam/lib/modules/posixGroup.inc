<?php
/*
$Id$

This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
Copyright (C) 2003 - 2006  Tilo Lutz

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
class posixGroup extends baseModule {

	// Variables
	// Use a unix password?
	var $userPassword_nopassword;
	// Use invalid password, '*', e.g. * for services
	var $userPassword_invalid;
	// Lock password
	var $userPassword_lock;
	// change gids of users and hosts?
	var $changegids;


	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		$error_messages = array();
		$needAutoGID = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixGroup", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixGroup";
			// group name
			if (get_preg($rawAccounts[$i][$ids['posixGroup_cn']], 'groupname')) {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixGroup_cn']];
			}
			else {
				$errMsg = $this->messages['cn'][3];
				array_push($errMsg, array($i));
				$error_messages[] = $errMsg;
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
			// description (UTF-8, no regex check needed)
			if ($rawAccounts[$i][$ids['posixGroup_description']] == "") {
				$partialAccounts[$i]['description'] = $partialAccounts[$i]['cn'];
			}
			else {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixGroup_description']];
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
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixGroup_password']], true, $this->moduleSettings['posixGroup_pwdHash'][0]);
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


	function delete_attributes($post) {
		$data = $_SESSION['cache']->get_cache('gidNumber', 'posixAccount', 'user');
		$DNs = array_keys($data);
		$found = false;
		for ($i = 0; $i < sizeof($DNs); $i++) {
			if ($data[$DNs[$i]][0] == $this->attributes['gidNumber'][0]) {
				$found = true;
				break;
			}
		}
		if ($found) {
			$return[$_SESSION[$this->base]->dn]['errors'][] = $this->messages['primaryGroup'][0];
		}
		return $return;
	}


	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		// check password format if called the first time
		if (!isset($this->userPassword_invalid)) {
			if ($this->attributes['userPassword'][0]) {
				if ($this->attributes['userPassword'][0] == '*') $this->userPassword_invalid = true;
				else $this->userPassword_invalid = false;
				if (pwd_is_enabled($this->attributes['userPassword'][0])) $this->userPassword_lock = false;
				else $this->userPassword_lock = true;
			}
			else $this->userPassword_nopassword = true;
		}
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _("Group name").'*'),
			1 => array('kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '20', 'maxlength' => '20', 'value' => $this->attributes['cn'][0]),
			2 => array('kind' => 'help', 'value' => 'cn'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('GID number').'*'),
			1 => array('kind' => 'input', 'name' => 'gidNumber', 'type' => 'text', 'size' => '6', 'maxlength' => '6', 'value' => $this->attributes['gidNumber'][0]),
			2 => array('kind' => 'help', 'value' => 'gidNumber'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Description')),
			1 => array('kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['description'][0]),
			2 => array ('kind' => 'help', 'value' => 'description'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _("Group members")),
			1 => array('kind' => 'input', 'name' => 'form_subpage_posixGroup_user_open', 'type' => 'submit', 'value' => _('Edit members')),
			2 => array ('kind' => 'help', 'value' => 'members'));
		if ($_SESSION[$this->base]->isNewAccount) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Password')),
				1 => array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
				2 => array('kind' => 'input', 'name' => 'genpass', 'type' => 'submit', 'value' => _('Generate password')));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Repeat password')),
				1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
				2 => array('kind' => 'help', 'value' => 'password'));
		}
		else {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Password') ),
				1 => array('kind' => 'input', 'name' => 'form_subpage_posixGroup_password_open', 'type' => 'submit', 'value' => _('Change password')));
		}
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Set no password')),
			1 => array('kind' => 'input', 'name' => 'userPassword_nopassword', 'type' => 'checkbox', 'checked' => $this->userPassword_nopassword),
			2 => array('kind' => 'help', 'value' => 'userPassword_no'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Invalid password')),
			1 => array('kind' => 'input', 'name' => 'userPassword_invalid', 'type' => 'checkbox', 'checked' => $this->userPassword_invalid),
			2 => array('kind' => 'help', 'value' => 'userPassword_invalid'));
		if ($_SESSION[$this->base]->isNewAccount || isset($this->attributes['userPassword'][0])) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Lock password')),
				1 => array('kind' => 'input', 'name' => 'userPassword_lock', 'type' => 'checkbox', 'checked' => $this->userPassword_lock),
				2 => array('kind' => 'help', 'value' => 'userPassword_lock'));
		}
		if ($this->attributes['gidNumber'][0]!=$this->orig['gidNumber'][0] && $this->orig['gidNumber'][0]!='')
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Change GID number of users and hosts')),
			1 => array('kind' => 'input', 'name' => 'changegids', 'type' => 'checkbox', 'checked' => $this->changegids, 'value' => true),
			2 => array('kind' => 'help', 'value' => 'changegids'));
		return $return;
	}


	/**
	* Displays selections to add or remove users from current group.
	*
	* @param array $post HTTP-POST
	* @return array meta HTML output
	*/
	function display_html_user(&$post) {
		// load list with all users
		$dn_users = $_SESSION['cache']->get_cache(array('uid', 'gidNumber'), 'posixAccount', 'user');
		$users = array();
		if (is_array($dn_users)) {
			$DNs = array_keys($dn_users);
			for ($i = 0; $i < sizeof($DNs); $i++) {
				// users who can be added have a uid and gidNumber
				if (isset($dn_users[$DNs[$i]]['uid'][0]) && isset($dn_users[$DNs[$i]]['gidNumber'][0]) &&
				// are not already member
				!in_array($dn_users[$DNs[$i]]['uid'][0], $this->attributes['memberUid']) &&
				// and do not have this group as their primary group
				!($this->attributes['gidNumber'][0] == $dn_users[$DNs[$i]]['gidNumber'][0])) {
					$users[] = $dn_users[$DNs[$i]]['uid'][0];
				}
			}
		}
		$return[] = array(
			0 => array('kind' => 'fieldset', 'legend' => _("Group members"), 'value' =>	array (
				0 => array(
					0 => array('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Selected users"), 'value' => array (
						0 => array(
							0 => array ( 'kind' => 'select', 'name' => 'removeusers', 'size' => '15', 'multiple' => true, 'options' => $this->attributes['memberUid'])))),
					1 => array('kind' => 'table', 'value' => array(
						0 => array(
							0 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addusers_button', 'value' => '<=', 'td' => array('align' => 'center'))),
							1 => array(
								0 => array('kind' => 'input', 'type' => 'submit', 'name' => 'removeusers_button', 'value' => '=>', 'td' => array('align' => 'center'))),
							2 => array(
								0 => array('kind' => 'help', 'value' => 'adduser', 'td' => array('align' => 'center'))))),
					2 => array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Available users"), 'value' => array(
						0 => array(
							0 => array('kind' => 'select', 'name' => 'addusers', 'size' => '15', 'multiple' => true, 'options' => $users))))
		))));

		$return[] = array(
			0 => array('kind' => 'input', 'name' => 'form_subpage_posixGroup_attributes_back' ,'type' => 'submit', 'value' => _('Back') ),
			1 => array('kind' => 'text'),
			2 => array('kind' => 'text'));
		return $return;
	}

	/**
	* Displays the password changing dialog.
	*
	* @param array $post HTTP-POST
	* @return array meta HTML code
	*/
	function display_html_password(&$post) {
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Password') ),
			1 => array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'password'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Repeat password')),
			1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ""));
		$return[] = array(
			0 => array('kind' => 'table', 'value' => array(
				0 => array(
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Ok'), 'name' => 'form_subpage_posixGroup_attributes_submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_posixGroup_attributes_back'),
					2 => array('kind' => 'text')))));
		return $return;
	}
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
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
		// configuration options
		$return['config_options']['group'] = array(
			array(
				0 => array('kind' => 'text', 'text' => '<b>' . _('Minimum GID number') . " *: </b>"),
				1 => array('kind' => 'input', 'name' => 'posixGroup_minGID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				2 => array('kind' => 'text', 'value' => '&nbsp;'),
				3 => array('kind' => 'text', 'text' => '<b>' . _('Maximum GID number') . " *: </b>"),
				4 => array('kind' => 'input', 'name' => 'posixGroup_maxGID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				5 => array('kind' => 'help', 'value' => 'minMaxGID')),
			array(
				0 => array('kind' => 'text', 'text' => '<b>' . _("Password hash type") . ': &nbsp;</b>'),
				1 => array('kind' => 'select', 'name' => 'posixGroup_pwdHash', 'size' => '1',
				'options' => array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), 'options_selected' => array('SSHA')),
				2 => array('kind' => 'text', 'value' => '&nbsp;'),
				3 => array('kind' => 'text', 'value' => '&nbsp;'),
				4 => array('kind' => 'text', 'value' => '&nbsp;'),
				5 => array('kind' => 'help', 'value' => 'pwdHash'))
		);
		// configuration descriptions
		$return['config_descriptions'] = array(
			'legend' => _("GID ranges for Unix groups"),
			'descriptions' => array(
				'posixGroup_minGID' => _("Minimum GID number for Unix groups"),
				'posixGroup_maxGID' => _("Maximum GID number for Unix groups"),
				'posixGroup_pwdHash' => _("Password hash type for Unix groups"),
			)
		);
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
			'cn',
			'gidNumber',
			'memberUid',
			'description'
		);
		// upload fields
		$return['upload_columns'] = array(
		array(
			'name' => 'posixGroup_cn',
			'description' => _('Group name'),
			'help' => 'cn',
			'example' => _('adminstrators'),
			'required' => true,
			'unique' => true
		),
		array(
			'name' => 'posixGroup_gid',
			'description' => _('GID number'),
			'help' => 'gidNumber',
			'example' => '2034'
		),
		array(
			'name' => 'posixGroup_description',
			'description' => _('Group description'),
			'help' => 'description',
			'example' => _('Administrators group')
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
		// help Entries
		$return['help'] = array(
			'cn' =>	array(
				"Headline" => _("Group name"),
				"Text" => _("Group name of the group which should be created. Valid characters are: a-z,0-9, .-_. LAM does not allow a number as first character because groupadd also does not allow it. LAM does not allow capital letters A-Z because it can cause several problems. If group name is already used group name will be expanded with a number. The next free number will be used.")
			),
			'gidNumber'	=> array(
				"Headline" => _("GID number"),
				"Text" => _("If empty GID number will be generated automaticly depending on your configuration settings.")
			),
			'description' => array(
				"Headline" => _("Description"),
				"Text" => _("Group description. If left empty group name will be used.")
			),
			'members' => array(
				"Headline" => _("Group members"),
				"Text" => _("Users who are member of the current group.")
			),
			'upload_members' => array(
				"Headline" => _("Group members"),
				"Text" => _("Users who will become member of the current group. User names are separated by semicolons.")
			),
			'password' => array(
				"Headline" => _("Group password"),
				"Text" => _("Sets the group password.")
			),
			'userPassword_no' => array(
				"Headline" => _("Use no password"),
				"Text" => _("If checked no password will be used.")
			),
			'userPassword_lock' => array(
				"Headline" => _("Account deactivated"),
				"Text" => _("If checked account will be deactivated by putting a \"!\" before the encrypted password.")
			),
			'userPassword_invalid' => array(
				"Headline" => _("Invalid password"),
				"Text" => _("This will set an invalid password which prevents logins with this account.")
			),
			'minMaxGID' => array(
				"Headline" => _("GID number"),
				"Text" => _("These are the minimum and maximum numbers to use for group IDs when creating new group accounts. New group accounts will always get the highest number in use plus one.")
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			)
		);

		return $return;
	}


	/**
	* Returns the PDF entries for this module.
	* 
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(
		'posixGroup_cn' => array('<block><key>' . _('Group name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
		'posixGroup_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
		'posixGroup_memberUid' => array('<block><key>' . _('Group members') . '</key><value>' . implode(', ', $this->attributes['memberUid']) . '</value></block>'),
		'posixGroup_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}


	/** This functin will be called when the module will be loaded **/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->changegids=false;
	}


	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['userPassword'][3] = array('ERROR', _('Password'), _('You cannot use this password options at the same time.'));
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
		$this->messages['primaryGroup'][0] = array('ERROR', _('Primary group'), _('There are still users who have this group as their primary group.'));
		} 


	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$_SESSION[$this->base]->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $_SESSION[$this->base]->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('posixGroup', $objectClasses)) {
				return true;
			}
		}
		if ($this->attributes['cn'][0] == '') return false;
		if ($this->attributes['gidNumber'][0] == '') return false;
		return true;
	}


	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		if (!$_SESSION[$this->base]->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $_SESSION[$this->base]->attributes_orig['objectClass'];
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
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		$this->attributes['description'][0] = $post['description'];

		if (($post['userPassword_lock'] && $post['userPassword_invalid']) || ($post['userPassword_nopassword'] && $post['userPassword_invalid'])) {
			// found invalid password parameter combination
			$triggered_messages['userPassword'][] = $this->messages['userPassword'][3];
		}
		else {
			if ($post['userPassword_nopassword']) {
				$this->userPassword_nopassword=true;
				$this->userPassword_invalid=false;
				$this->attributes['userPassword'][0] = '';
				$post['userPassword2'] = '';
				if ($post['userPassword_lock'])
					$this->userPassword_lock=true;
				else $this->userPassword_lock=false;
			}
			else {
				$this->userPassword_nopassword=false;
				if ($post['userPassword_invalid']) {
					$this->userPassword_invalid=true;
					$this->userPassword_lock=false;
					$post['userPassword2'] = '';
				}
				else {
					$this->userPassword_invalid=false;
					if ($post['genpass']) $this->attributes['userPassword'][0] = genpasswd();
					elseif ($_SESSION[$this->base]->isNewAccount) {
						if ($post['userPassword'] != $post['userPassword2'])
							$triggered_messages['userPassword'][] = $this->messages['userPassword'][0];
						else $this->attributes['userPassword'][0] = $post['userPassword'];
						if (!get_preg($this->attributes['userPassword'][0], 'password'))
							$triggered_messages['userPassword'][] = $this->messages['userPassword'][1];
					}
					if ($post['userPassword_lock']) $this->userPassword_lock=true;
					else $this->userPassword_lock=false;
				}
			}
		if ($post['changegids']) $this->changegids=true;
		else $this->changegids=false;
		if ($this->attributes['gidNumber'][0]!=$post['gidNumber'] || ($this->triggered_messages['gidNumber'][0]='ERROR')) {
			// Check if GID is valid. If none value was entered, the next useable value will be inserted
			// load min and max GID number
			$minID = intval($this->moduleSettings['posixGroup_minGID'][0]);
			$maxID = intval($this->moduleSettings['posixGroup_maxGID'][0]);
			$dn_gids = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
			// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
			if(is_array($dn_gids) && (sizeof($dn_gids) > 0)) {
				foreach ($dn_gids as $gid) $gids[] = $gid[0];
				sort ($gids, SORT_NUMERIC);
			}
			$this->attributes['gidNumber'][0]=$post['gidNumber'];
				if ($this->attributes['gidNumber'][0]=='') {
				// No id-number given, find free GID
				if ($this->orig['gidNumber'][0]=='') {
					$newGID = $this->getNextGIDs(1, $triggered_messages);
					if (is_array($newGID)) {
						$this->attributes['gidNumber'][0] = $newGID[0];
					}
					else {
						$triggered_messages['gidNumber'][] = $this->messages['gidNumber'][3];
					}
				}
				else $this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
				// old account -> return id-number which has been used
			}
			else {
				// Check manual ID
				// id-number is out of valid range
				if ( ($this->attributes['gidNumber'][0]!=$post['gidNumber']) && ($this->attributes['gidNumber'][0] < $minID || $this->attributes['gidNumber'][0] > $maxID)) $triggered_messages['gidNumber'][] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
				// $uids is allways an array but not if no entries were found
				if (is_array($gids)) {
					// id-number is in use and account is a new account
					if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]=='') $triggered_messages['gidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					// id-number is in use, account is existing account and id-number is not used by itself
					if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]!='' && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0]) ) {
						$triggered_messages['gidNumber'][] = $this->messages['gidNumber'][4];
						$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
					}
				}
			}
		}
		if ($this->attributes['cn'][0]!=$post['cn'] || ($this->triggered_messages['cn'][0]='ERROR')) {
			$this->attributes['cn'][0] = $post['cn'];
			if (($this->attributes['cn'][0] != $post['cn']) &&  ereg('[A-Z]$', $post['cn']))
				$triggered_messages['cn'][] = $this->messages['cn'][0];
			// Check if Groupname contains only valid characters
			if ( !get_preg($this->attributes['cn'][0],'groupname'))
			$triggered_messages['cn'][] = $this->messages['cn'][2];
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			// Set username back to original name if new username is in use
			if ($_SESSION['cache']->in_cache($this->attributes['cn'][0],'cn', 'group')!=false && ($this->orig['cn'][0]!='')) {
				$this->attributes['cn'][0] = $this->orig['cn'][0];
			}
			// Change gid to a new gid until a free gid is found
			else while ($_SESSION['cache']->in_cache($this->attributes['cn'][0], 'cn', 'group')) {
				// get last character of username
				$lastchar = substr($this->attributes['cn'][0], strlen($this->attributes['cn'][0])-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
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
						if (ereg('^([0-9])+$',substr($this->attributes['cn'][0], $i, strlen($this->attributes['cn'][0])-$i))) $i--;
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
			if ($this->attributes['cn'][0] != $post['cn']) {
				$triggered_messages['cn'][] = $this->messages['cn'][0];
			}
		// show info when gidnumber has changed
		if (($this->orig['gidNumber'][0]!=$this->attributes['gidNumber'][0]) && $this->orig['gidNumber'][0]!='' && $post['gidNumber']!=$this->attributes['gidNumber'][0])
		$triggered_messages['gidNumber'][] = $this->messages['gidNumber'][0];
		}
		}
		// Return error-messages
		if (count($triggered_messages)!=0) {
			$this->triggered_messages = $triggered_messages;
			return $triggered_messages;
		}
		else $this->triggered_messages = array();
	}


	/**
	* Processes user input of the user selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_user(&$post) {
		if (isset($post['addusers']) && isset($post['addusers_button'])) { // Add users to list
			// Add new user
			$this->attributes['memberUid'] = @array_merge($this->attributes['memberUid'], $post['addusers']);
			// remove duplicates
			$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
			array_unique($this->attributes['memberUid']);
			$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
			// sort users
			sort($this->attributes['memberUid']);
		}
		elseif (isset($post['removeusers']) && isset($post['removeusers_button'])) { // remove users from list
			$this->attributes['memberUid'] = array_delete($post['removeusers'], $this->attributes['memberUid']);
		}
	}


	/**
	* Processes user input of the password page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_password(&$post) {
		if ($post['form_subpage_posixGroup_attributes_back']) return;
		$messages = array();
		if ($post['userPassword'] != $post['userPassword2']) {
			$messages['userPassword'][] = $this->messages['userPassword'][0];
			if (!get_preg($post['userPassword'], 'password'))
				$messages['userPassword'][] = $this->messages['userPassword'][1];
			}
			else {
				$this->attributes['userPassword'][0] = $post['userPassword'];
				$this->userPassword_invalid = false;
				$this->userPassword_lock = false;
				$this->userPassword_nopassword = false;
			}
		if (sizeof($messages) > 0) return $messages;
	}
	
	/* This function returns an array with 3 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* modify are attributes which have to been modified in ldap entry
	*/
	function save_attributes() {
		// skip saving if account is based on another structural object class
		if (!$_SESSION[$this->base]->isNewAccount && !in_array('posixGroup', $_SESSION[$this->base]->attributes_orig['objectClass'])) {
			return array();
		}
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// unset password when needed
		if (isset($return[$_SESSION[$this->base]->dn]['add']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['userPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['userPassword']);
		// Set unix password
		if (isset($this->orig['userPassword'][0])) {
			// use no password, do nothing
			if ($this->userPassword_nopassword) {}
			// invalid, use '*' as password
			elseif ($this->userPassword_invalid)
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = '*';
			// password changed
			elseif (($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) && $this->attributes['userPassword'][0] != '')
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash($this->attributes['userPassword'][0], !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]);
			// lock account if required
			elseif ($this->userPassword_lock && (pwd_disable($this->orig['userPassword'][0]) != $this->orig['userPassword'][0]))
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_disable($this->orig['userPassword'][0]);
			// unlock password if required
			elseif (!$this->userPassword_lock && (pwd_enable($this->orig['userPassword'][0]) != $this->orig['userPassword'][0]))
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_enable($this->orig['userPassword'][0]);
			// password has not changed
			else
				$return[$_SESSION[$this->base]->dn]['notchanged']['userPassword'][0] = $this->orig['userPassword'][0];
		}
		else {
			// New user or no old password set
			if ($this->userPassword_nopassword) // use no password
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]);
			else if ($this->userPassword_invalid) // use '*' as password
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = '*';
			else if ($this->attributes['userPassword'][0] != '') // set password if set
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = pwd_hash($this->attributes['userPassword'][0], !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]);
		}

		// Remove primary group from users from memberUid
		$users_dn = $_SESSION['cache']->get_cache('gidNumber', 'posixAccount', 'user');
		if (is_array($users_dn)) {
			$DNs = array_keys($users_dn);
			for ($i=0; $i<count($DNs); $i++) {
				if ($users_dn[$DNs[$i]][0]==$this->attributes['gidNumber'][0]) {
					$thisuser = substr($DNs[$i], 4, strpos($DNs[$i], ",")-4);
					if (@in_array($thisuser, $this->attribtues['memberUid'])) {
						$this->attribtues['memberUid'] = @array_flip($this->attribtues['memberUid']);
						unset($this->attribtues['memberUid'][$thisuser]);
						$this->attribtues['memberUid'] = @array_flip($this->attribtues['memberUid']);
					}
				}
			}
		}


		// Change gids of users and hosts?
		if ($this->changegids) {
			// get gidNumber
			$line=-1;
			for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME 'posixAccount'")) $line = $i;
			}
			if ($line!=-1) {
				$result = $_SESSION['cache']->get_cache('gidNumber', 'posixAccount', array('user', 'host'));
				if (is_array($result)) {
					$DNs = array_keys($result);
					for ($i=0; $i<count($DNs); $i++)
					if ($result[$DNs[$i]][0] == $this->orig['gidNumber'][0]) $return[$DNs[$i]]['modify']['gidNumber'][0] = $this->attributes['gidNumber'][0];
				}
			}
			// change primaryGroupID
			$line=-1;
			for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME 'sambaAccount'")) $line = $i;
			}
			if ($line!=-1) {
				$result = $_SESSION['cache']->get_cache('primaryGroupID', 'sambaAccount', array('user', 'host'));
				if (is_array($result)) {
					$DNs = array_keys($result);
					for ($i=0; $i<count($DNs); $i++) {
						if ($result[$DNs[$i]][0] == $this->orig['gidNumber'][0]*2+1001 ) $return[$DNs[$i]]['modify']['PrimaryGroupID'][0] = $this->attributes['gidNumber'][0]*2+1001;
					}
				}
			}
			// change sambaPrimaryGroupSID
			$line=-1;
			for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME 'sambaSamAccount'")) $line = $i;
			}
			if ($line!=-1) {
				$result = $_SESSION['cache']->get_cache('sambaPrimaryGroupSID', 'sambaSamAccount', array('user', 'host'));
				if (is_array($result)) {
					$DNs = array_keys($result);
					for ($i=0; $i<count($DNs); $i++) {
						// Get Domain SID from name
						$sambaDomains = search_domains();
						// Get Domain-SID from group SID
						$domainSID = substr($result[$DNs[$i]], 0, strrpos($result[$DNs[$i]], "-"));
						for ($i=0; $i<count($sambaDomains); $i++ )
						if ($domainSID==$sambaDomains[$i]->SID)
						$RIDbase = $sambaDomains[$i]->RIDbase;
						if ($result[$DNs[$i]][0] == $SID . "-" . $this->orig['gidNumber'][0]*2+1+$RIDbase ) $return[$DNs[$i]]['modify']['sambaPrimaryGroupSID'][0] = $SID . "-" . $this->attributes['gidNumber'][0]*2+1+$RIDbase;
					}
				}
			}
		}
		return $return;
	}


	/**
	* Returns one or more free GID numbers.
	*
	* @param integer $count Number of needed free GIDs.
	* @param array $triggered_messages list of error messages where errors can be added
	* @return mixed Null if no GIDs are free else an array of free GIDs. 
	*/
	function getNextGIDs($count, &$triggered_messages) {
		$ret = array();
		$minID = intval($this->moduleSettings['posixGroup_minGID'][0]);
		$maxID = intval($this->moduleSettings['posixGroup_maxGID'][0]);
		$dn_gids = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
		// get_cache will return an array ( dn1 => array(gidnumber1), dn2 => array(gidnumber2), ... )
		$gids = array();
		if(is_array($dn_gids)) {
			foreach ($dn_gids as $gid) {
				if (($gid[0] <= $maxID) && ($gid[0] >= $minID)) $gids[] = $gid[0];  // ignore GIDs > maxID and GIDs < minID
			}
			sort ($gids, SORT_NUMERIC);
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
					$triggered_messages['gidNumber'][] = $this->messages['gidNumber'][2];
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

}
	
?>
