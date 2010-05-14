<?php
/*
	$Id$

	This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
	Copyright (C) 2003 - 2006  Tilo Lutz
    Copyright (C) 2005 - 2010  Roland Gruber

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
* Manages Unix accounts for users and hosts.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
* @author Thomas Manninger
*/

/**
* Manages the object class "posixAccount" for users and hosts.
*
* @package modules
*/
class posixAccount extends baseModule implements passwordService {

	// Variables
	private static $SPLIT_DELIMITER = "###x##y##x###";
	
	/* These two variables keep an array of groups the user is also member of. */
	private $groups;
	private $groups_orig;

	private $createhomedir;
	private $lamdaemonServer;
	private $groupCache = null;
	private $clearTextPassword;

	/**
	 *  This function fills the error message array with messages.
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['minUID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Minimum UID number'), _("Minimum UID number is invalid!"));
		$this->messages['maxUID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number is invalid!"));
		$this->messages['minMachine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Minimum UID number'), _("Minimum UID number is invalid!"));
		$this->messages['maxMachine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number is invalid!"));
		$this->messages['cmp_UID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number must be greater than minimum UID number!"));
		$this->messages['cmp_Machine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number must be greater than minimum UID number!"));
		$this->messages['cmp_both'][0] = array('ERROR', _('UID ranges for Unix accounts'), _("The UID ranges for users and hosts overlap! This is a problem because LAM uses the highest UID in use + 1 for new accounts. Please set the minimum UID to equal values or use independent ranges."));
		$this->messages['homeDirectory'][0] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
		$this->messages['homeDirectory'][1] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
		$this->messages['homeDirectory'][2] = array('ERROR', _('Account %s:') . ' posixAccount_homedir', _('Homedirectory contains invalid characters.'));
		$this->messages['homeDirectory'][3] = array('INFO', _('Home directory'), _('Home directory changed. To keep home directory you have to run the following command as root: \'mv %s %s\''));
		$this->messages['uidNumber'][1] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
		$this->messages['uidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
		$this->messages['uidNumber'][3] = array('ERROR', _('ID-Number'), _('ID is already in use'));
		$this->messages['uidNumber'][4] = array('ERROR', _('Account %s:') . ' posixAccount_uid', _('UID must be a number. It has to be inside the UID range which is defined in your configuration profile.'));
		$this->messages['uidNumber'][5] = array('INFO', _('UID number'), _('UID number has changed. To keep file ownership you have to run the following command as root: \'find / -uid %s -exec chown %s {} \;\''));
		$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password fields.'));
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['userPassword'][4] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !')); // double "%" because of sprintf
		$this->messages['uid'][0] = array('INFO', _('UID'), _('UID has changed. Do you want to change home directory?'));
		$this->messages['uid'][1] = array('WARN', _('User name'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->messages['uid'][2] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][3] = array('WARN', _('Host name'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->messages['uid'][4] = array('ERROR', _('Host name'), _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][5] = array('WARN', _('User name'), _('User name in use. Selected next free user name.'));
		$this->messages['uid'][6] = array('WARN', _('Host name'), _('Host name in use. Selected next free host name.'));
		$this->messages['uid'][7] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][8] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][9] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name already exists!'));
		$this->messages['uid'][10] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Host name already exists!'));
		$this->messages['gidNumber'][0] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('LAM was unable to find a group with this name!'));
		$this->messages['gidNumber'][1] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('This GID number is invalid! Please provide either a number or a group name.'));
		$this->messages['gidNumber'][2] = array('INFO', _('GID number'), _('GID number has changed. To keep file ownership you have to run the following command as root: \'find / -gid %s -uid %s -exec chgrp %s {} \;\''));
		$this->messages['gecos'][0] = array('ERROR', _('Account %s:') . ' posixAccount_gecos', _('This gecos value is invalid!'));
		$this->messages['shell'][0] = array('ERROR', _('Account %s:') . ' posixAccount_shell', _('This login shell is invalid!'));
		$this->messages['passwordDisabled'][0] = array('ERROR', _('Account %s:') . ' posixAccount_passwordDisabled', _('This value can only be \"true\" or \"false\".'));
		$this->messages['cn'][0] = array('ERROR', _('Common name'), _('Please enter a valid common name!'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_cn', _('Please enter a valid common name!'));
		$this->messages['createhomediron'][0] = array('ERROR', _('Home directory'), _('Choose a server for the home directory!'));
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
		$return['icon'] = 'tux.png';
		// manages user and host accounts
		$return["account_types"] = array("user", "host");
		// user specific data
		if ($this->get_scope() == "user") {
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixAccount)", 'and' => "(!(uid=*$))");
			// module dependencies
			$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		}
		elseif ($this->get_scope() == "host") {
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixAccount)");
			// module dependencies
			$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		}
		// alias name
		$return["alias"] = _("Unix");
		// RDN attributes
		$return["RDN"] = array("uid" => "high", "cn" => "low");
		// managed object classes
		$return['objectClasses'] = array('posixAccount');
		// LDAP aliases
		$return['LDAPaliases'] = array('commonName' => 'cn', 'userid' => 'uid');
		// managed attributes
		$return['attributes'] = array('cn', 'uid', 'uidNumber', 'gidNumber', 'homeDirectory',
			'userPassword', 'loginShell', 'gecos');
		if ($this->get_scope() == "user") {
			// self service search attributes
			$return['selfServiceSearchAttributes'] = array('uid');
			// self service field settings
			$return['selfServiceFieldSettings'] = array('password' => _('Password'));
			// self service configuration settings
			$return['selfServiceSettings'] = array(
			array(
				array('kind' => 'text', 'text' => '<b>' . _("Password hash type") . ': &nbsp;</b>'),
				array('kind' => 'select', 'name' => 'posixAccount_pwdHash', 'size' => '1',
					'options' => array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), 'options_selected' => array('SSHA')),
				array('kind' => 'help', 'value' => 'pwdHash'))
			);
		}
		// profile checks
		$return['profile_checks']['posixAccount_homeDirectory'] = array('type' => 'ext_preg', 'regex' => 'homeDirectory',
		'error_message' => $this->messages['homeDirectory'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'posixAccount_homeDirectory' => 'homeDirectory',
			'posixAccount_loginShell' => 'loginShell'
		);
		// configuration options
		$return['config_options']['user'] = array(
			array(
				array('kind' => 'text', 'text' => '<b>' . _("Users") . ': &nbsp;</b>' . _('Minimum UID number') . "*: "),
				array('kind' => 'input', 'name' => 'posixAccount_minUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'text', 'text' => _('Maximum UID number') . "*: "),
				array('kind' => 'input', 'name' => 'posixAccount_maxUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				array('kind' => 'help', 'value' => 'minMaxUser'))
		);
		$return['config_options']['host'] = array(
		array(
			array('kind' => 'text', 'text' => '<b>' . _("Hosts") . ': &nbsp;</b>' . _('Minimum UID number') . "*: "),
			array('kind' => 'input', 'name' => 'posixAccount_minMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			array('kind' => 'text', 'value' => '&nbsp;'),
			array('kind' => 'text', 'text' => _('Maximum UID number') . "*: "),
			array('kind' => 'input', 'name' => 'posixAccount_maxMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			array('kind' => 'help', 'value' => 'minMaxHost'))
		);
		$return['config_options']['all'] = array(
			array(
				array('kind' => 'text', 'text' => '<b>' . _("Password hash type") . ': &nbsp;</b>'),
				array('kind' => 'select', 'name' => 'posixAccount_pwdHash', 'size' => '1',
					'options' => array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), 'options_selected' => array('SSHA')),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'help', 'value' => 'pwdHash')),
			array(
				array('kind' => 'text', 'text' => '<b>' . _('Set primary group as memberUid') . ":</b> "),
				array('kind' => 'input', 'name' => 'posixAccount_primaryGroupAsSecondary', 'type' => 'checkbox'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'text', 'value' => '&nbsp;'),
				array('kind' => 'help', 'value' => 'primaryGroupAsSecondary'))
		);
		// upload
		$return['upload_preDepends'] = array('inetOrgPerson');
		// user specific upload options
		if (($this->get_scope() == 'user') && isset($_SESSION['loggedIn']) && ($_SESSION['loggedIn'] === true)) {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_userName',
				'description' => _('User name'),
				'help' => 'uid',
				'example' => _('smiller'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller')
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uidNumber',
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group_upload',
				'example' => _('users'),
				'required' => true
			),
			array(
				'name' => 'posixAccount_additionalGroups',
				'description' => _('Additional groups'),
				'help' => 'addgroup_upload',
				'example' => _('group01,group02')
			),
			array(
				'name' => 'posixAccount_homedir',
				'description' => _('Home directory'),
				'help' => 'homeDirectory_upload',
				'example' => _('/home/smiller'),
				'default' => '/home/<i>&lt;posixAccount_userName&gt;</i>'
			),
			array(
				'name' => 'posixAccount_createHomeDir',
				'description' => _('Create home directory'),
				'help' => 'createhomedir',
				'example' => 'localhost',
				'values' => $_SESSION['config']->get_scriptServers()
			),
			array(
				'name' => 'posixAccount_shell',
				'description' => _('Login shell'),
				'help' => 'loginShell',
				'example' => '/bin/bash',
				'values' => implode(", ", getshells()),
				'default' => '/bin/bash'
			),
			array(
				'name' => 'posixAccount_password',
				'description' => _('Password'),
				'help' => 'userPassword',
				'example' => _('secret')
			),
			array(
				'name' => 'posixAccount_passwordDisabled',
				'description' => _('Lock password'),
				'help' => 'userPassword_lock',
				'example' => 'false',
				'values' => 'true, false',
				'default' => 'false'
			),
			array(
				'name' => 'posixAccount_gecos',
				'description' => _('Gecos'),
				'help' => 'gecos',
				'example' => _('Steve Miller,Room 2.14,123-123-1234,123-123-1234')
			)
			);
		}
		// host specific upload options
		elseif ($this->get_scope() == 'host') {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_hostName',
				'description' => _('Host name'),
				'help' => 'uid',
				'example' => _('pc01$'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uidNumber',
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group_upload',
				'example' => _('machines'),
				'required' => true
			),
			array(
				'name' => 'posixAccount_gecos',
				'description' => _('Gecos'),
				'help' => 'gecos',
				'example' => _('pc01,Room 2.34')
			)
			);
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'uid' => _('User name'),
			'uidNumber' => _('UID number'),
			'gidNumber' => _('GID number'),
			'gecos' => _('Gecos'),
			'primaryGroup' => _('Primary group'),
			'additionalGroups' => _('Additional groups'),
			'homeDirectory' => _('Home directory'),
			'loginShell' => _('Login shell'),
			'cn' => _('Common name'),
			'userPassword' => _('Password')
		);
		// help Entries
		$return['help'] = array(
			'primaryGroupAsSecondary' => array(
				'Headline' => _('Set primary group as memberUid'),
				'Text' => _('Usually, users are not added to groups as memberUid if they have this group as primary group. If your application ignores primary groups then you can select this option to override this behaviour.')
			),
			'minMaxUser' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for user IDs when creating new user accounts. The range should be different from that of machines. New user accounts will always get the highest number in use plus one.')
			),
			'minMaxHost' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for machine IDs when creating new accounts for hosts. The range should be different from that of users. New host accounts will always get the highest number in use plus one.')
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'uidNumber' => array(
				"Headline" => _("UID number"),
				"Text" => _("If empty UID number will be generated automaticly.")
			),
			'group_upload' => array(
				"Headline" => _("Primary group"),
				"Text" => _("The primary group for this account. You can insert a GID number or a group name.")
			),
			'addgroup_upload' => array(
				"Headline" => _("Additional groups"),
				"Text" => _("Here you can enter a list of additional group memberships. The group names are separated by commas.")
			),
			'homeDirectory_upload' => array(
				"Headline" => _("Home directory"),
				"Text" => _("Please enter the path to the user's home directory.")
			),
			'deletehomedir' => array(
				"Headline" => _("Home directory"),
				"Text" => _("Activating this checkbox will remove the user's home directory.")
			),
			'createhomedir' => array(
				"Headline" => _("Home directory"),
				"Text" => _("This will create the user's home directory on the specified server.")
			),
			'user' => array(
				'uid' => array(
					"Headline" => _("User name"),
					"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, .-_. If user name is already used user name will be expanded with a number. The next free number will be used. Warning: Older systems have problems with user names longer than 8 characters. You can not log in to Windows if user name is longer than 16 characters.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("User description. If left empty first and last name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"),
					"Text" => _("The primary group the user should be member of.")
				),
				'homeDirectory' => array(
					"Headline" => _("Home directory"),
					"Text" => _('$user and $group will be replaced with user name and primary group name.')
				),
				'userPassword' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
				),
				'userPassword_lock' => array(
					"Headline" => _("Account deactivated"),
					"Text" => _("If checked account will be deactivated by putting a \"!\" before the encrypted password.")
				),
				'loginShell' => array(
					"Headline" => _("Login shell"),
					"Text" => _("To disable login use /bin/false. The list of shells is read from lam/config/shells")
				),
				'addgroup' => array(
					"Headline" => _("Additional groups"),
					"Text" => _("Hold the CTRL-key to (de)select multiple groups."). ' '. _("Can be left empty.")
				),
				'cn' => array (
					"Headline" => _("Common name"),
					"Text" => _("This is the natural name of the user. If empty, the first and last name or user name is used.")
				)
			),
			'host' => array(
				'uid' => array(
					"Headline" => _("Host name"),
					"Text" => _("Host name of the host which should be created. Valid characters are: a-z,A-Z,0-9, .-_$. Host names are always ending with $. If last character is not $ it will be added. If host name is already used host name will be expanded with a number. The next free number will be used.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("Host description. If left empty host name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"),
					"Text" => _("The primary group the host should be member of.")
				),
				'description' => array (
					"Headline" => _("Description"),
					"Text" => _("Host description. If left empty host name will be used.")
				),
				'cn' => array (
					"Headline" => _("Common name"),
					"Text" => _("This is the natural name of the host. If empty, the host name will be used.")
				)
			)
		);

		return $return;
	}

	/**
	* Initializes the module after it became part of an accountContainer
	*
	* @param string $base the name of the accountContainer object ($_SESSION[$base])
	*/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->createhomedir=false;
		$this->groups = array();
		$this->groups_orig = array();
		$groups = $this->findGroups(); // list of all groupnames
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return;
		}
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (!isset($this->attributes['uid'][0]) || ($this->attributes['uid'][0] == '')) return false;
		if (!isset($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] == '')) return false;
		if (!isset($this->attributes['gidNumber'][0]) || ($this->attributes['gidNumber'][0] == '')) return false;
		if (!isset($this->attributes['homeDirectory'][0]) || ($this->attributes['homeDirectory'][0] == '')) return false;
		if (!isset($this->attributes['loginShell'][0]) || ($this->attributes['loginShell'][0] == '')) return false;
		return true;
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// get additional group memberships
		if (!isset($attr['uid'][0])) return;
		$dn_groups = $_SESSION['cache']->get_cache(array('memberUid', 'cn'), 'posixGroup', 'group');
		if (is_array($dn_groups)) {
			$DNs = array_keys($dn_groups);
			for ($i = 0; $i < sizeof($DNs); $i++) {
				if (isset($dn_groups[$DNs[$i]]['memberUid'][0])) {
					if (in_array($attr['uid'][0], $dn_groups[$DNs[$i]]['memberUid'])) $this->groups[] = $dn_groups[$DNs[$i]]['cn'][0];
				}
			}
		}
		$this->groups_orig = $this->groups;
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
	*/
	function save_attributes() {
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// Remove primary group from additional groups
		if (!isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
			|| ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] != 'true')) {
			for ($i=0; $i<count($this->groups); $i++) {
				if ($this->groups[$i] == $this->getGroupName($this->attributes['gidNumber'][0])) {
					unset($this->groups[$i]);
				}
			}
		}
		else {
			// add user as memberuid in primary group
			if (!in_array($this->getGroupName($this->attributes['gidNumber'][0]), $this->groups)) {
				$this->groups[] = $this->getGroupName($this->attributes['gidNumber'][0]);
			}
		}

		// Set additional group memberships
		if ($this->orig['uid'][0]!='' && $this->attributes['uid'][0]!=$this->orig['uid'][0]) {
			// remove old memberships
			$dn_groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN) {
				if (in_array($this->orig['uid'][0], $dn_groups[$DN])) {
					$return[$DN]['remove']['memberUid'][0] = $this->orig['uid'][0];
				}
			}
			// Add new memberships
			if (is_array($this->groups)) {
				foreach ($this->groups as $group) {
					$dn = $this->getGroupDN($group);
					$return[$dn]['add']['memberUid'][0] = $this->attributes['uid'][0];
				}
			}
		}
		else {
			// update groups.
			$add = array_delete($this->groups_orig, $this->groups);
			$remove = array_delete($this->groups, $this->groups_orig);
			$dn_cns = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
			// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
			$DNs = array_keys($dn_cns);
			foreach ($DNs as $DN) {
				if (is_array($add)) {
					if (in_array($dn_cns[$DN][0], $add)) $return[$DN]['add']['memberUid'][] = $this->attributes['uid'][0];
				}
				if (is_array($remove)) {
					if (in_array($dn_cns[$DN][0], $remove)) $return[$DN]['remove']['memberUid'][] = $this->attributes['uid'][0];
				}
			}
		}
		return $return;
	}

	/**
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 */
	public function postModifyActions($newAccount, $attributes) {
		// create home directory if needed
		if ($this->createhomedir) {
			$server = null;
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				if (isset($temp[1])) {
					if ($temp[1] == $this->lamdaemonServer) {
						$server = $temp[0];
						break;
					}
				}
				elseif ($temp[0] == $this->lamdaemonServer) {
					$server = $temp[0];
					break;
				}
			}
			$result = lamdaemon(implode(posixAccount::$SPLIT_DELIMITER, array($this->attributes['uid'][0], "home", "add", "0".$_SESSION['config']->get_scriptRights())), $server);
			// lamdaemon results
			if (is_array($result)) {
				$singleresult = explode(",", $result[0]);
				if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'INFO') || ($singleresult[0] == 'WARN')) {
					call_user_func_array('StatusMessage', $singleresult);
				}
				else {
					StatusMessage('ERROR', $result[0]);
				}
			}
		}
	}
	
	/**
	* Additional LDAP operations on delete.
	*
	* @return List of LDAP operations, same as for save_attributes()
	*/
	function delete_attributes() {
		$return = array();
		// remove memberUids if set
		$groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
		if (!is_array($groups)) return $return;
		$DNs = array_keys($groups);
		for ($i=0; $i<count($DNs); $i++) {
			if (in_array($this->attributes['uid'][0], $groups[$DNs[$i]])) $return[$DNs[$i]]['remove']['memberUid'][] = $this->attributes['uid'][0];
		}
		return $return;
	}

	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * An error message should be printed if the function returns false.
	 * 
	 * @return true, if no problems occured
	 */
	function preDeleteActions() {
		$return = true;
		if (isset($_POST['deletehomedir'])) {
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				$lamdaemonServers[$i] = $temp[0];
			}
			// try to delete directory on all servers
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$result = lamdaemon(implode(posixAccount::$SPLIT_DELIMITER, array($this->attributes['uid'][0], "home", "rem")), $lamdaemonServers[$i]);
				// lamdaemon results
				if (is_array($result)) {
					foreach ($result as $singleresult) {
						$singleresult = explode(",", $singleresult);
						if (is_array($singleresult)) {
							if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
								call_user_func_array('StatusMessage', $singleresult);
							}
						}
					}
				}
			}
		}
		return $return;
	}
	
	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		$groups = $this->findGroups(); // list of all groupnames
		if (count($groups)==0) {
			// abort if no groups were found
			return array();
		}
		$this->attributes['loginShell'][0] = $_POST['loginShell'];
		if (isset($_POST['gecos'])) $this->attributes['gecos'][0] = $_POST['gecos'];
		if ($this->orig['uid'][0]!='' && $_POST['uid']!=$this->attributes['uid'][0]) {
			$errors[] = $this->messages['uid'][0];
		}
		if (($this->orig['gidNumber'][0] != '') && ($_POST['gidNumber'] != $this->attributes['gidNumber'][0])) {
			$errorMessage = $this->messages['gidNumber'][2];
			$errorMessage[] = array($this->orig['gidNumber'][0], $this->orig['uidNumber'][0], $_POST['gidNumber']);
			$errors[] = $errorMessage;
		}
		if ($this->orig['uidNumber'][0]!='' && $_POST['uidNumber']!=$this->attributes['uidNumber'][0]) {
			$errorMessage = $this->messages['uidNumber'][5];
			$errorMessage[] = array($this->orig['uidNumber'][0], $_POST['uidNumber']);
			$errors[] = $errorMessage;
		}
		if (isset($_POST['homeDirectory']) && $this->orig['homeDirectory'][0]!='' && $_POST['homeDirectory']!=$this->attributes['homeDirectory'][0]) {
			$errorMessage = $this->messages['homeDirectory'][3];
			$errorMessage[] = array($this->orig['homeDirectory'][0], $_POST['homeDirectory']);
			$errors[] = $errorMessage;
		}
		// get list of DNS names or IPs
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			$temp = explode(":", $lamdaemonServers[$i]);
			if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
			else $lamdaemonServers[$i] = $temp[0];
		}
		if (isset($_POST['createhomedir'])) {
			$this->createhomedir = true;
	 		if (!in_array($_POST['createhomediron'], $lamdaemonServers)) {
	 			$errorMessage = $this->messages['createhomediron'][0];
	   			$errorMessage[] = array($this->orig['createhomediron'][0]);
				$errors[] = $errorMessage;
			}
			else {
				$this->lamdaemonServer = $_POST['createhomediron'];
			}
		}
		else $this->createhomedir = false;
		$this->attributes['homeDirectory'][0] = $_POST['homeDirectory'];
		// Load attributes
		if (isset($_POST['form_subpage_posixAccount_attributes_lockPassword'])) {
			$this->attributes['userPassword'][0] = pwd_disable($this->attributes['userPassword'][0]);
		}
		if (isset($_POST['form_subpage_posixAccount_attributes_unlockPassword'])) {
			$this->attributes['userPassword'][0] = pwd_enable($this->attributes['userPassword'][0]);
		}
		if (isset($_POST['form_subpage_posixAccount_attributes_removePassword'])) {
			unset($this->attributes['userPassword']);
		}
		$this->attributes['uid'][0] = $_POST['uid'];
		$this->attributes['cn'][0] = $_POST['cn'];
		if (!get_preg($this->attributes['cn'][0], 'cn')) {
			$errors[] = $this->messages['cn'][0];
		}
		$this->attributes['uidNumber'][0] = $_POST['uidNumber'];
		$this->attributes['gidNumber'][0] = $_POST['gidNumber'];
		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		if ($this->get_scope()=='user') {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		if ($this->get_scope()=='host') {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$dn_uids = $_SESSION['cache']->get_cache('uidNumber', 'posixAccount', array('user', 'host'));
		// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		if(is_array($dn_uids)) {
			foreach ($dn_uids as $uid) $uids[] = $uid[0];
			if (sizeof($uids) > 0) sort($uids, SORT_NUMERIC);
		}
		if ($this->attributes['uidNumber'][0]=='') {
			// No id-number given
			if ($this->orig['uidNumber'][0]=='') {
				// new account -> we have to find a free id-number
				$newUID = $this->getNextUIDs(1, $errors);
				if (is_array($newUID)) {
					$this->attributes['uidNumber'][0] = $newUID[0];
				}
				else {
					$errors[] = $this->messages['uidNumber'][3];
				}
			}
			else $this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
			// old account -> return id-number which has been used
		}
		else {
			// check manual ID
			if ($this->getAccountContainer()->isNewAccount || !isset($this->orig['uidNumber'][0]) || ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0])) {
				// check range
				if (!is_numeric($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] < $minID) || ($this->attributes['uidNumber'][0] > $maxID)) $errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
				// $uids is always an array but not if no entries were found
				if (is_array($uids)) {
					// id-number is in use and account is a new account
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					// id-number is in use, account is existing account and id-number is not used by itself
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
						$errors[] = $this->messages['uidNumber'][3];
						$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
					}
				}
			}
		}
		if ($this->get_scope()=='user') {
			if (($this->attributes['uid'][0] != $_POST['uid']) &&  !get_preg($_POST['uid'], '!upper'))
			$errors[] = $this->messages['uid'][1];
			if ( !get_preg($this->attributes['homeDirectory'][0], 'homeDirectory' ))
			$errors[] = $this->messages['homeDirectory'][0];
		}
		if ($this->get_scope()=='user') {
			$this->attributes['homeDirectory'][0] = str_replace('$group', $this->getGroupName($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
			if ($this->attributes['uid'][0] != '')
			$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
			if ($this->attributes['homeDirectory'][0] != $_POST['homeDirectory']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
			// Check if Username contains only valid characters
			if ( !get_preg($this->attributes['uid'][0], 'username'))
			$errors[] = $this->messages['uid'][2];
		}
		if ($this->get_scope()=='host') {
			// Check if Hostname contains only valid characters
			if ( !get_preg($this->attributes['uid'][0], 'hostname'))
			$errors[] = $this->messages['uid'][4];
			if (!$this->attributes['homeDirectory'][0]) {
				$this->attributes['homeDirectory'][0] = '/dev/null';
			}
			if (!$this->attributes['loginShell'][0]) {
				$this->attributes['loginShell'][0] = '/bin/false';
			}
		}
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ((sizeof(searchLDAPByAttribute('uid', $this->attributes['uid'][0], 'posixAccount', array('uid'), array('user', 'host'))) > 0) && ($this->orig['uid'][0]!='')) {
			$this->attributes['uid'][0] = $this->orig['uid'][0];
		}
		else {
			// Change uid to a new uid until a free uid is found
			while (sizeof(searchLDAPByAttribute('uid', $this->attributes['uid'][0], 'posixAccount', array('uid'), array('user', 'host'))) > 0) {
				if ($this->get_scope()=='host') $this->attributes['uid'][0] = substr($this->attributes['uid'][0], 0, -1);
				// get last character of username
				$lastchar = substr($this->attributes['uid'][0], strlen($this->attributes['uid'][0])-1, 1);
				// Last character is no number
				if ( !preg_match('/^([0-9])+$/', $lastchar)) {
					// Last character is no number. Therefore we only have to add "2" to it.
					if ($this->get_scope()=='host') {
						$this->attributes['uid'][0] = $this->attributes['uid'][0] . '2$';
					}
					else {
						$this->attributes['uid'][0] = $this->attributes['uid'][0] . '2';
					}
				}
				else {
					/* Last character is a number -> we have to increase the number until we've
					* found a groupname with trailing number which is not in use.
					*
					* $i will show us were we have to split groupname so we get a part
					* with the groupname and a part with the trailing number
					*/
					$i=strlen($this->attributes['uid'][0])-1;
					$mark = false;
					// Set $i to the last character which is a number in $account_new->general_username
					while (!$mark)
					if (preg_match('/^([0-9])+$/',substr($this->attributes['uid'][0], $i, strlen($this->attributes['uid'][0])-$i))) $i--;
					else $mark=true;
					// increase last number with one
					$firstchars = substr($this->attributes['uid'][0], 0, $i+1);
					$lastchars = substr($this->attributes['uid'][0], $i+1, strlen($this->attributes['uid'][0])-$i);
					// Put username together
					if ($this->get_scope()=='host') $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1)."$";
					else $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1);
				}
			}
		}
		// Show warning if LAM has changed username
		if ($this->attributes['uid'][0] != $_POST['uid']) {
			if ($this->get_scope()=='user') $errors[] = $this->messages['uid'][5];
			if ($this->get_scope()=='host') $errors[] = $this->messages['uid'][6];
		}
		$attributeList = array('gecos', 'homeDirectory');
		for ($i = 0; $i < sizeof($attributeList); $i++) {
			if (isset($this->attributes[$attributeList[$i]][0])) {
				$value = $this->attributes[$attributeList[$i]][0];
				$replacedValue = $this->checkASCII($value);
				if ($value != $replacedValue) {
					$this->attributes[$attributeList[$i]][0] = $replacedValue;
					$errors[] = array('WARN', $attributeList[$i], _('Changed value because only ASCII characters are allowed.'));
				}
			}
		}
		// Return error-messages
		return $errors;
	}

	/**
	* Checks if an attribute contains only ASCII charaters and replaces invalid characters.
	*
	* @param string $attribute attribute value
	* @return string attribute value with replaced non-ASCII characters
	*/
	function checkASCII($attribute) {
		// convert UTF8 to ASCII
		$convert = array ( 'ä' => 'ae', 'Ä' => 'Ae', 'ö' => 'oe', 'Ö' => 'Oe', 'ü' => 'ue', 'Ü' => 'Ue',
			'ß' => 'ss', 'é' => 'e', 'è' => 'e', 'ô' => 'o', 'ç' => 'c'
			);
		// replace special characters
		foreach ($convert as $key => $value) {
			$attribute = str_replace($key, $value, $attribute);
		}
		// remove remaining UTF-8 characters
		for ($c = 0; $c < strlen($attribute); $c++) {
			if (ord($attribute[$c]) > 127) {
				$attribute = str_replace($attribute[$c], "", $attribute);
				$c--;
			}
		}
		return $attribute;
	}

	/**
	* Processes user input of the group selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_group() {
		if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
			// Add new group
			$this->groups = @array_merge($this->groups, $_POST['addgroups']);
		}
		elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
			$this->groups = array_delete($_POST['removegroups'], $this->groups);
		}
		return array();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$groups = $this->findGroups(); // list of all group names
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return array();
		}
		$shelllist = getshells(); // list of all valid shells

		// set default values
		if (($this->attributes['uid'][0] == '') && ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null)) {
			// fill default value for user ID with first/last name
			$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if (isset($attrs['sn'][0])) {
				if (isset($attrs['givenName'])) {
					$this->attributes['uid'][0] = preg_replace('/[^a-z0-9_-]/', '', strtolower($attrs['givenName'][0][0] . $attrs['sn'][0]));
				}
				else {
					$this->attributes['uid'][0] = preg_replace('/[^a-z0-9_-]/', '', strtolower($attrs['sn'][0]));
				}
			}
		}
		if ($this->attributes['cn'][0] == '') {
			// set a default value for common name
			if (($this->get_scope() == 'host') && (substr($_POST['uid'], -1, 1) == '$')) {
				$this->attributes['cn'][0] = substr($_POST['uid'], 0, strlen($_POST['uid']) - 1);
			}
			elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
				if ($attrs['givenName'][0]) {
					$this->attributes['cn'][0] = $attrs['givenName'][0] . " " . $attrs['sn'][0];
				}
				elseif ($attrs['sn'][0]) {
					$this->attributes['cn'][0] = $attrs['sn'][0];
				}
				else {
					$this->attributes['cn'][0] = $_POST['uid'];
				}
			}
			else {
				$this->attributes['cn'][0] = $_POST['uid'];
			}
		}
		
		$userName = '';
		if (isset($this->attributes['uid'][0])) $userName = $this->attributes['uid'][0];
		$return[] = array (
			array('kind' => 'text', 'text' => _("User name").'*'),
			array('kind' => 'input', 'name' => 'uid', 'type' => 'text', 'size' => '30', 'maxlength' => '20', 'value' => $userName),
			array('kind' => 'help', 'value' => 'uid'));
		$commonName = '';
		if (isset($this->attributes['cn'][0])) $commonName = $this->attributes['cn'][0];
		$return[] = array (
			array('kind' => 'text', 'text' => _("Common name").'*'),
			array('kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '30', 'value' => $commonName),
			array('kind' => 'help', 'value' => 'cn'));
		$uidNumber = '';
		if (isset($this->attributes['uidNumber'][0])) $uidNumber = $this->attributes['uidNumber'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('UID number').'*'),
			array('kind' => 'input', 'name' => 'uidNumber', 'type' => 'text', 'size' => '30', 'maxlength' => '20', 'value' => $uidNumber),
			array('kind' => 'help', 'value' => 'uidNumber'));
		$gecos = '';
		if (isset($this->attributes['gecos'][0])) $gecos = $this->attributes['gecos'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Gecos')),
			array('kind' => 'input', 'name' => 'gecos', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $gecos),
			array('kind' => 'help', 'value' => 'gecos'));
		$return[] = array(
			array('kind' => 'text', 'text' => _('Primary group').'*' ),
			array('kind' => 'select', 'name' => 'gidNumber', 'descriptiveOptions' => true, 'options' => $groups, 'options_selected' => array($this->attributes['gidNumber'][0])),
			array('kind' => 'help', 'value' => 'gidNumber'));

		if ($this->get_scope()=='user') {
			$return[] = array(
				array('kind' => 'text', 'text' => _('Additional groups')),
				array('kind' => 'input', 'name' => 'form_subpage_posixAccount_group_open', 'type' => 'submit', 'value' => _('Edit groups')),
				array('kind' => 'help', 'value' => 'addgroup'));
			$return[] = array(
				array('kind' => 'text', 'text' => _('Home directory').'*'),
				array('kind' => 'input', 'name' => 'homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['homeDirectory'][0]),
				array('kind' => 'help', 'value' => 'homeDirectory'));
			if ($this->getAccountContainer()->isNewAccount && ($_SESSION['config']->get_scriptPath() != null) 
				&& ($_SESSION['config']->get_scriptPath() != '')) {
				// get list of lamdaemon servers
				$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
				for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
					$temp = explode(":", $lamdaemonServers[$i]);
					if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
					else $lamdaemonServers[$i] = $temp[0];
				}
				$return[] = array(
					array('kind' => 'text', 'text' => _('Create home directory')),
					array('kind' => 'table', 'value' => array(array(
						array('kind' => 'input', 'name' => 'createhomedir', 'type' => 'checkbox', 'checked' => $this->createhomedir),
						array('kind' => 'select', 'name' => 'createhomediron', 'options' => $lamdaemonServers, 'options_selected' => $this->lamdaemonServer),
					))),
					array('kind' => 'help', 'value' => 'createhomedir'));
			}
			if (count($shelllist)!=0) {
				$return[] = array(
					array('kind' => 'text', 'text' => _('Login shell').'*'),
					array('kind' => 'select', 'name' => 'loginShell', 'options' => $shelllist, 'options_selected' => array ($this->attributes['loginShell'][0])),
					array('kind' => 'help', 'value' => 'loginShell'));
			}
		}
		if (pwd_is_enabled($this->attributes['userPassword'][0])) {
			$lockOption = array('kind' => 'input', 'name' => 'form_subpage_posixAccount_attributes_lockPassword', 'type' => 'submit', 'value' => _('Lock password'));
		}
		else {
			$lockOption = array('kind' => 'input', 'name' => 'form_subpage_posixAccount_attributes_unlockPassword', 'type' => 'submit', 'value' => _('Unlock password'));
		}
		$return[] = array(
			array('kind' => 'text', 'text' => _('Password') ),
			array('kind' => 'table', 'value' => array(
				array($lockOption),
				array(
					array('kind' => 'input', 'name' => 'form_subpage_posixAccount_attributes_removePassword', 'type' => 'submit', 'value' => _('Remove password'))
				)
			)));
	return $return;
	}

	/**
	* Displays the delete homedir option for the delete page.
	*
	* @return meta HTML code
	*/
	function display_html_delete() {
		if ($this->get_scope() == 'user' && ($_SESSION['config']->get_scriptPath() != null)) {
			$return[] = array (
				array('kind' => 'text', 'text' => _('Delete home directory')),
				array('kind' => 'input', 'name' => 'deletehomedir', 'type' => 'checkbox'),
				array('kind' => 'help', 'value' => 'deletehomedir'));
		}
		return $return;
	}

	/**
	* Displays the group selection.
	*
	* @return array meta HTML code
	*/
	function display_html_group() {
		// load list with all groups
		$dn_groups = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			$groups[] = $dn_groups[$DN][0];
		}
		// remove groups the user is member of from grouplist
		$groups = array_delete($this->groups, $groups);
		// Remove primary group from grouplist
		$group = $this->getGroupName($this->attributes['gidNumber'][0]);
		$groups = array_flip($groups);
		unset ($groups[$group]);
		$groups = array_flip($groups);

		$return[] = array(
			array('kind' => 'fieldset', 'legend' => _("Additional groups"), 'value' => array(
				array(
					array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Selected groups"), 'value' => array(
						array (
							array('kind' => 'select', 'name' => 'removegroups', 'size' => '15', 'multiple' => true, 'options' => $this->groups)))),
						array('kind' => 'table', 'value' => array(
							array(
								array('kind' => 'input', 'type' => 'submit', 'name' => 'addgroups_button', 'value' => '<=', 'td' => array('align' => 'center'))),
							array(
								array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>', 'td' => array('align' => 'center'))),
							array(
								array ( 'kind' => 'help', 'value' => 'addgroup', 'td' => array('align' => 'center'))))),
						array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Available groups"), 'value' => array(
							array(
								array('kind' => 'select', 'name' => 'addgroups', 'size' => '15', 'multiple' => true, 'options' => $groups))))
		))));

		$return[] = array(
			array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_posixAccount_attributes_back'),
			array('kind' => 'text'),
			array('kind' => 'text'));
		return $return;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		$groupList = $this->findGroups();
		$groups = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groups[] = $groupList[$i][1];
		}
		if ($this->get_scope() == 'user') {
			$shelllist = getshells(); // list of all valid shells
			// primary Unix group
			$return[] = array(
				array('kind' => 'text', 'text' => _('Primary group') . ": "),
				array('kind' => 'select', 'name' => 'posixAccount_primaryGroup', 'options' => $groups, 'options_selected' => array(), 'size' => 1),
				array('kind' => 'help', 'value' => 'gidNumber', 'scope' => 'user'));
			// additional group memberships
			$return[] = array(
				array('kind' => 'text', 'text' => _('Additional groups') . ": "),
				array('kind' => 'select', 'name' => 'posixAccount_additionalGroup', 'options' => $groups,
					'options_selected' => array(), 'size' => 10, 'multiple' => true),
				array('kind' => 'help', 'value' => 'addgroup', 'scope' => 'user'));
			// home directory
			$return[] = array(
				array('kind' => 'text', 'text' => _('Home directory') . ": "),
				array('kind' => 'input', 'name' => 'posixAccount_homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => '/home/$user'),
				array('kind' => 'help', 'value' => 'homeDirectory', 'scope' => 'user'));
			// login shell
			$return[] = array(
				array('kind' => 'text', 'text' => _('Login shell') . ": "),
				array('kind' => 'select', 'name' => 'posixAccount_loginShell', 'options' => $shelllist, 'options_selected' => array("/bin/bash")),
				array('kind' => 'help', 'value' => 'loginShell', 'scope' => 'user'));
			// lamdaemon settings
			if (($_SESSION['config']->get_scriptPath() != null)) {
			$return[] = array(
				array('kind' => 'text', 'text' => _('Create home directory') . ": "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'posixAccount_createHomedir'),
				array('kind' => 'help', 'value' => 'createhomedir', 'scope' => 'user'));				
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
				else $lamdaemonServers[$i] = $temp[0];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Home directory server') . ": "),
				array('kind' => 'select', 'name' => 'posixAccount_homedirServer', 'options' => $lamdaemonServers),
				array('kind' => 'help', 'value' => 'createhomedir', 'scope' => 'user'));
			}
		}
		elseif ($this->get_scope() == 'host') {
			// primary Unix group
			$return[] = array(
				array('kind' => 'text', 'text' => _('Primary group') . ": "),
				array('kind' => 'select', 'name' => 'posixAccount_primaryGroup', 'options' => $groups, 'options_selected' => array(), 'size' => 1),
				array('kind' => 'help', 'value' => 'gidNumber', 'scope' => 'host'));
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
		// special profile options
		// GID
		if (isset($profile['posixAccount_primaryGroup'][0])) {
			$gid = $this->getGID($profile['posixAccount_primaryGroup'][0]);
			if ($gid != null) {
				$this->attributes['gidNumber'][0] = $gid;
			}
		}
		// other group memberships
		if (isset($profile['posixAccount_additionalGroup'][0])) {
			$this->groups = $profile['posixAccount_additionalGroup'];
		}
		// lamdaemon
		if (($this->get_scope() == 'user') && $this->getAccountContainer()->isNewAccount) {
			if (isset($profile['posixAccount_homedirServer'][0])) {
				$this->lamdaemonServer = $profile['posixAccount_homedirServer'][0];
			}
			if (isset($profile['posixAccount_createHomedir'][0]) && ($profile['posixAccount_createHomedir'][0] == 'true')) {
				$this->createhomedir = true;
			}
		}
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array(
			'posixAccount_uid' => array('<block><key>' . _('User name') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'),
			'posixAccount_cn' => array('<block><key>' . _('Common name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			'posixAccount_uidNumber' => array('<block><key>' . _('UID number') . '</key><value>' . $this->attributes['uidNumber'][0] . '</value></block>'),
			'posixAccount_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
			'posixAccount_gecos' => array('<block><key>' . _('Gecos') . '</key><value>' . $this->attributes['gecos'][0] . '</value></block>'),
			'posixAccount_primaryGroup' => array('<block><key>' . _('Primary group') . '</key><value>' . $this->getGroupName($this->attributes['gidNumber'][0]) . '</value></block>'),
			'posixAccount_additionalGroups' => array('<block><key>' . _('Additional groups') . '</key><value>' . implode(", ", $this->groups) . '</value></block>'),
			'posixAccount_homeDirectory' => array('<block><key>' . _('Home directory') . '</key><value>' . $this->attributes['homeDirectory'][0] . '</value></block>'),
			'posixAccount_loginShell' => array('<block><key>' . _('Login shell') . '</key><value>' . $this->attributes['loginShell'][0] . '</value></block>'),
		);
		if (isset($this->clearTextPassword)) {
			$return['posixAccount_userPassword'] = array('<block><key>' . _('Password') . '</key><value>' . $this->clearTextPassword . '</value></block>');
		}
		return $return;
	}

	/**
	* Checks input values of module settings.
	*
	* @param array $scopes list of account types which are used
	* @param array $options hash array containing the settings (array('option' => array('value')))
	* @return array list of error messages
	*/
	function check_configOptions($scopes, $options) {
		$return = array();
		// user settings
		if (in_array('user', $scopes)) {
			// min/maxUID are required, check if they are numeric
			if (!isset($options['posixAccount_minUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_minUID'][0])) {
				$return[] = $this->messages['minUID'][0];
			}
			if (!isset($options['posixAccount_maxUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_maxUID'][0])) {
				$return[] = $this->messages['maxUID'][0];
			}
			// minUID < maxUID
			if (isset($options['posixAccount_minUID'][0]) && isset($options['posixAccount_maxUID'][0])) {
				if ($options['posixAccount_minUID'][0] > $options['posixAccount_maxUID'][0]) {
					$return[] = $this->messages['cmp_UID'][0];
				}
			}
		}
		// host settings
		if (in_array('host', $scopes)) {
			// min/maxUID are required, check if they are numeric
			if (!isset($options['posixAccount_minMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_minMachine'][0])) {
				$return[] = $this->messages['minMachine'][0];
			}
			if (!isset($options['posixAccount_maxMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_maxMachine'][0])) {
				$return[] = $this->messages['maxMachine'][0];
			}
			// minUID < maxUID
			if (isset($options['posixAccount_minMachine'][0]) && isset($options['posixAccount_maxMachine'][0])) {
				if ($options['posixAccount_minMachine'][0] > $options['posixAccount_maxMachine'][0]) {
					$return[] = $this->messages['cmp_Machine'][0];
				}
			}
		}
		// check if user and host ranges overlap
		if (in_array('user', $scopes) && in_array('host', $scopes)) {
			if (isset($options['posixAccount_minUID'][0]) && isset($options['posixAccount_maxUID'][0]) &&
			isset($options['posixAccount_minMachine'][0]) && isset($options['posixAccount_maxMachine'][0])) {
				if (($options['posixAccount_minMachine'][0] > $options['posixAccount_minUID'][0]) &&
				($options['posixAccount_minMachine'][0] < $options['posixAccount_maxUID'][0])) {
					$return[] = $this->messages['cmp_both'][0];
				}
				if (($options['posixAccount_minUID'][0] > $options['posixAccount_minMachine'][0]) &&
				($options['posixAccount_minUID'][0] < $options['posixAccount_maxMachine'][0])) {
					$return[] = $this->messages['cmp_both'][0];
				}
			}
		}
		return $return;
	}

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
		$errors = array();
		$needAutoUID = array();
		// get list of existing users
		$dnUsers = $_SESSION['cache']->get_cache('uid', 'posixAccount', array('user', 'host'));
		$existingUsers = array();
		foreach ($dnUsers as $dn) {
			$existingUsers[] = $dn[0];
		}
		// get list of existing groups
		$groupList = $this->findGroups();
		$groupMap = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groupMap[$groupList[$i][1]] = $groupList[$i][0];
		}
		$existingGroups = array_keys($groupMap);
		// check input
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixAccount";
			// UID
			if ($rawAccounts[$i][$ids['posixAccount_uid']] == "") {
				// autoUID
				$needAutoUID[] = $i;
			}
			elseif (get_preg($rawAccounts[$i][$ids['posixAccount_uid']], 'digit')) {
				if ($this->get_scope() == 'user') {
					if (($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minUID'][0]) &&
						($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxUID'][0])) {
						$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
					}
					else {
						$errMsg = $this->messages['uidNumber'][4];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				elseif ($this->get_scope() == 'host') {
					if (($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minMachine'][0]) &&
						($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxMachine'][0])) {
						$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
					}
					else {
						$errMsg = $this->messages['uidNumber'][4];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
			}
			else {
				$errMsg = $this->messages['uidNumber'][4];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// GID number
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixAccount_group']];
			}
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'groupname')) {
				$gid = $groupMap[$rawAccounts[$i][$ids['posixAccount_group']]];
				if (is_numeric($gid)) {
					$partialAccounts[$i]['gidNumber'] = $gid;
				}
				else {
					$errMsg = $this->messages['gidNumber'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$errMsg = $this->messages['gidNumber'][1];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// GECOS
			if ($rawAccounts[$i][$ids['posixAccount_gecos']] != "") {
				if (get_preg($rawAccounts[$i][$ids['posixAccount_gecos']], 'gecos')) {
					$partialAccounts[$i]['gecos'] = $this->checkASCII($rawAccounts[$i][$ids['posixAccount_gecos']]);
				}
				else {
					$errMsg = $this->messages['gecos'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$gecos = "";
				if (($rawAccounts[$i][$ids['inetOrgPerson_firstName']] != "") && ($rawAccounts[$i][$ids['inetOrgPerson_lastName']] != "")) {
					$gecos = $rawAccounts[$i][$ids['inetOrgPerson_firstName']] . " " . $rawAccounts[$i][$ids['inetOrgPerson_lastName']];
					if ($rawAccounts[$i][$ids['inetOrgPerson_telephone']] != "") {
						$gecos = $gecos . ",," . $rawAccounts[$i][$ids['inetOrgPerson_telephone']];  // double "," because room is unknown
						if ($rawAccounts[$i][$ids['inetOrgPerson_fax']] != "") {
							$gecos = $gecos . "," . $rawAccounts[$i][$ids['inetOrgPerson_fax']];
						}
					}
				}
				if ($gecos != "") {
					$partialAccounts[$i]['gecos'] = $this->checkASCII($gecos);
				}
			}
			// user specific attributes
			if ($this->get_scope() == 'user') {
				// additional groups
				if ($rawAccounts[$i][$ids['posixAccount_additionalGroups']] != "") {
					$groups = explode(",", $rawAccounts[$i][$ids['posixAccount_additionalGroups']]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $existingGroups)) {
							$errors[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						}
					}
				}
				// user name
				if (in_array($rawAccounts[$i][$ids['posixAccount_userName']], $existingUsers)) {
					$errMsg = $this->messages['uid'][9];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][7];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// home directory
				if ($rawAccounts[$i][$ids['posixAccount_homedir']] == "") {
					$partialAccounts[$i]['homeDirectory'] = '/home/' . $partialAccounts[$i]['uid'];
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_homedir']], 'homeDirectory')) {
					$partialAccounts[$i]['homeDirectory'] = $rawAccounts[$i][$ids['posixAccount_homedir']];
				}
				else {
					$errMsg = $this->messages['homeDirectory'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// login shell
				if ($rawAccounts[$i][$ids['posixAccount_shell']] == "") {
					$partialAccounts[$i]['loginShell'] = '/bin/bash';
				}
				elseif (in_array($rawAccounts[$i][$ids['posixAccount_shell']], getshells())) {
					$partialAccounts[$i]['loginShell'] = $rawAccounts[$i][$ids['posixAccount_shell']];
				}
				else {
					$errMsg = $this->messages['shell'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				$pwd_enabled = true;
				// password enabled/disabled
				if ($rawAccounts[$i][$ids['posixAccount_passwordDisabled']] == "") {
					$pwd_enabled = true;
				}
				elseif (in_array($rawAccounts[$i][$ids['posixAccount_passwordDisabled']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['posixAccount_passwordDisabled']] == 'true') $pwd_enabled = false;
					else $pwd_enabled = true;
				}
				else {
					$errMsg = $this->messages['passwordDisabled'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// password
				if (($rawAccounts[$i][$ids['posixAccount_password']] != "") && (get_preg($rawAccounts[$i][$ids['posixAccount_password']], 'password'))) {
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixAccount_password']], $pwd_enabled, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				elseif ($rawAccounts[$i][$ids['posixAccount_password']] != "") {
					$errMsg = str_replace('%', '%%', $this->messages['userPassword'][4]); // double "%" because of later sprintf
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// cn
				if ($rawAccounts[$i][$ids['posixAccount_cn']] != "") {
					if (get_preg($rawAccounts[$i][$ids['posixAccount_cn']], 'cn')) {
						$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixAccount_cn']];
					}
					else {
						$errMsg = $this->messages['cn'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					if ($partialAccounts[$i]['givenName']) {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['givenName'] . " " . $partialAccounts[$i]['sn'];
					}
					elseif ($partialAccounts[$i]['sn']) {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['sn'];
					}
					else {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['uid'];
					}
				}
			}
			// host specific attributes
			elseif ($this->get_scope() == 'host') {
				// host name
				if (in_array($rawAccounts[$i][$ids['posixAccount_hostName']], $existingUsers)) {
					$errMsg = $this->messages['uid'][10];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_hostName']], 'hostname')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
				}
				else {
					$errMsg = $this->messages['uid'][8];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// description
				if ($rawAccounts[$i][$ids['posixAccount_description']] && ($rawAccounts[$i][$ids['posixAccount_description']] != '')) {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_description']];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
				}
				$partialAccounts[$i]['homeDirectory'] = '/dev/null';
				$partialAccounts[$i]['loginShell'] = '/bin/false';
			}
		}
		// fill in autoUIDs
		if (sizeof($needAutoUID) > 0) {
			$errorsTemp = array();
			$uids = $this->getNextUIDs(sizeof($needAutoUID), $errorsTemp);
			if (is_array($uids)) {
				for ($i = 0; $i < sizeof($needAutoUID); $i++) {
					$partialAccounts[$i]['uidNumber'] = $uids[$i];
				}
			}
			else {
				$errors[] = $this->messages['uidNumber'][2];
			}
		}
		return $errors;
	}

	/**
	* This function executes one post upload action.
	*
	* @param array $data array containing one account in each element
	* @param array $ids array(<column_name> => <column number>)
	* @param array $failed list of accounts which were not created successfully
	* @param array $temp variable to store temporary data between two post actions
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress'
	* <br>  'progress' => 0..100
	* <br>  'errors' => array (<array of parameters for StatusMessage>)
	* <br> )
	*/
	function doUploadPostActions($data, $ids, $failed, &$temp) {
		if (!checkIfWriteAccessIsAllowed()) {
			die();
		}
		// on first call generate list of ldap operations
		if (!isset($temp['counter'])) {
			$temp['groups'] = array();
			$temp['createHomes'] = array();
			$temp['counter'] = 0;
			$col = $ids['posixAccount_additionalGroups'];
			$col_home = $ids['posixAccount_createHomeDir'];
			// get list of existing groups
			$groupList = $this->findGroups();
			$groupMap = array();
			for ($i = 0; $i < sizeof($groupList); $i++) {
				$groupMap[$groupList[$i][0]] = $groupList[$i][1];
			}
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				if ($data[$i][$col] != "") {
					$groups = explode(",", $data[$i][$col]);
					if (isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
						&& ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] == 'true')) {
						if (get_preg($data[$i][$ids['posixAccount_group']], 'digit')) {
							if (!in_array($groupMap[$data[$i][$ids['posixAccount_group']]], $groups)) {
								$groups[] = $groupMap[$data[$i][$ids['posixAccount_group']]];
							}
						}
						else {
							if (!in_array($groupMap[$data[$i][$ids['posixAccount_group']]], $groups)) {
								$groups[] = $data[$i][$ids['posixAccount_group']];
							}
						}
					}
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $temp['groups'])) $temp['groups'][] = $groups[$g];
						$temp['members'][$groups[$g]][] = $data[$i][$ids['posixAccount_userName']];
					}
				}
				if ($data[$i][$col_home] != "") {
					$temp['createHomes'][] = $i;
				}
			}
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => array()
			);
		}
		// get DNs of groups
		elseif (!isset($temp['dn'])) {
			$temp['dn'] = array();
			$result = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
			$keys = array_keys($result);
			for ($i = 0; $i < sizeof($result); $i++) {
				$temp['dn'][$result[$keys[$i]][0]] = $keys[$i];
			}
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => array()
			);
		}
		// add users to groups
		elseif ($temp['counter'] < sizeof($temp['groups'])) {
			if (isset($temp['dn'][$temp['groups'][$temp['counter']]])) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $temp['dn'][$temp['groups'][$temp['counter']]], array('memberUID' => $temp['members'][$temp['groups'][$temp['counter']]]));
				$errors = array();
				if (!$success) {
					$errors[] = array(
						"ERROR",
						_("LAM was unable to modify group memberships for group: %s"),
						ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
						array($temp['groups'][$temp['counter']])
					);
				}
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes'])),
					'errors' => $errors
				);
			}
			else {
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups'] + sizeof($temp['createHomes']))),
					'errors' => array(array('ERROR', _('Unable to find group in LDAP.'), $temp['groups'][$temp['counter']]))
				);
			}
		}
		// create home directories
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']))) {
			$pos = $temp['createHomes'][$temp['counter'] - sizeof($temp['groups'])];
			$result = lamdaemon(implode(posixAccount::$SPLIT_DELIMITER, array($data[$pos][$ids['posixAccount_userName']], "home", "add", "0".$_SESSION['config']->get_scriptRights())),
				$data[$pos][$ids['posixAccount_createHomeDir']]);
			$errors = array();
			if (($result != false) && (sizeof($result) == 1)) {
				$parts = explode(",", $result[0]);
				if (in_array($parts[0], array('ERROR', 'WARN'))) { 
					$errors[] = $parts;
				}
			}
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes'])),
				'errors' => $errors
			);
		}
		// all groups are modified
		else {
			return array (
				'status' => 'finished',
				'progress' => 100,
				'errors' => array()
			);
		}
	}

	/**
	* Returns one or more free UID numbers.
	*
	* @param integer $count Number of needed free UIDs.
	* @param array $errors list of error messages where errors can be added
	* @return mixed Null if no UIDs are free else an array of free UIDs.
	*/
	function getNextUIDs($count, &$errors) {
		$ret = array();
		if ($this->get_scope() == "user") {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		else {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$dn_uids = $_SESSION['cache']->get_cache('uidNumber', 'posixAccount', array('user', 'host'));
		// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		$uids = array();
		if(is_array($dn_uids)) {
			foreach ($dn_uids as $uid) {
				if (($uid[0] <= $maxID) && ($uid[0] >= $minID)) $uids[] = $uid[0];  // ignore UIDs > maxID and UIDs < minID
			}
			sort ($uids, SORT_NUMERIC);
		}
		for ($i = 0; $i < $count; $i++) {
			if (count($uids) != 0) {
				// there already are some uids
				// store highest id-number
				$id = $uids[count($uids)-1];
				// Return minimum allowed id-number if all found id-numbers are too low
				if ($id < $minID) {
					$ret[] = $minID;
					$uids[] = $minID;
				}
				// return highest used id-number + 1 if it's still in valid range
				elseif ($id < $maxID) {
					$ret[] = $id + 1;
					$uids[] = $id + 1;
				}
				// find free numbers between existing ones
				else {
					$k = intval($minID);
					while (in_array($k, $uids)) $k++;
					if ($k > $maxID) return null;
					else {
						$ret[] = $k;
						$uids[] = $k;
						sort ($uids, SORT_NUMERIC);
					}
					// show warning message
					$errors[] = $this->messages['uidNumber'][2];
				}
			}
			else {
				// return minimum allowed id-number if no id-numbers are found
				$ret[] = $minID;
				$uids[] = $minID;
			}
		}
		return $ret;
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account (attribute names in lower case)
	 * @return array meta HTML
	 */
	function getSelfServiceOptions($fields, $attributes) {
		$return = array();
		if (in_array('password', $fields)) {
			$return['password'] = array(
				array('kind' => 'text', 'text' => _('New password')),
				array('kind' => 'input', 'name' => 'posixAccount_password', 'type' => 'password', 'size' => '30', 'maxlength' => '255'),
				array('kind' => 'text', 'text' => _('Reenter password')),
				array('kind' => 'input', 'name' => 'posixAccount_password2', 'type' => 'password', 'size' => '30', 'maxlength' => '255')
				);
		}
		return $return;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP commands which should be executed.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and LDAP commands (array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array());
		if (in_array('password', $fields)) {
			if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
				if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
					$return['messages'][] = $this->messages['userPassword'][0];
				}
				else {
					if (!get_preg($_POST['posixAccount_password'], 'password')) {
						$return['messages'][] = $this->messages['userPassword'][1];
					}
					else {
						$pwdPolicyResult = checkPasswordStrength($_POST['posixAccount_password']);
						if ($pwdPolicyResult === true) {
							$return['mod']['userPassword'][0] = pwd_hash($_POST['posixAccount_password'], true, $this->selfServiceSettings->moduleSettings['posixAccount_pwdHash'][0]);
							if (isset($attributes['shadowLastChange'][0])) {
								$return['mod']['shadowLastChange'][0] = intval(time()/3600/24);
							}
							$_SESSION['selfService_clientPasswordNew'] = $_POST['posixAccount_password'];
						}
						else {
							$return['messages'][] = array('ERROR', $pwdPolicyResult);
						}
					}
				}
			}
		}
		return $return;
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
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$this->clearTextPassword = $password;
		$this->attributes['userPassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		return array();
	}
	
	/**
	 * Returns the group ID of the given group.
	 *
	 * @param String $groupname group name
	 * @return String GID
	 */
	private function getGID($groupname) {
		$results = searchLDAPByAttribute('cn', $groupname, 'posixGroup', array('gidnumber'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['gidnumber'][0])) {
			return $results[0]['gidnumber'][0];
		}
		return null;
	}
	
	/**
	 * Returns the group name of the group with the given group ID.
	 *
	 * @param String $groupID group ID
	 * @return String group name
	 */
	private function getGroupName($groupID) {
		$results = searchLDAPByAttribute('gidNumber', $groupID, 'posixGroup', array('cn'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['cn'][0])) {
			return $results[0]['cn'][0];
		}
		return null;
	}
	
	/**
	 * Returns the group DN of the given group.
	 *
	 * @param String $groupname group name
	 * @return String DN
	 */
	private function getGroupDN($groupname) {
		$results = searchLDAPByAttribute('cn', $groupname, 'posixGroup', array('dn'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['dn'][0])) {
			return $results[0]['dn'];
		}
		return null;
	}
	
	/**
	 * Finds all existing LDAP groups.
	 *
	 * @return array groups array(array(cn, gidnumber), array(cn, gidnumber), ...)
	 */
	private function findGroups() {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$results = searchLDAPByAttribute(null, null, 'posixGroup', array('cn', 'gidnumber'), array('group'));
		$return = array();
		for ($i = 0; $i < sizeof($results); $i++) {
			if (isset($results[$i]['cn'][0]) && isset($results[$i]['gidnumber'][0])) {
				$return[] = array($results[$i]['gidnumber'][0], $results[$i]['cn'][0]);
			}
		}
		$this->groupCache = $return;
		return $return;
	}
	
}

?>
