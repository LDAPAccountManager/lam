<?php
/*
	$Id$
	
	This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
	Copyright (C) 2003  Tilo Lutz
	
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

class posixAccount extends baseModule {

	/**
	* Creates a new posixAccount object.
	*/
	function posixAccount($scope) {
		// call parent constructor
		parent::baseModule($scope);
	}

	function dynamic_Message($attribute, $id) {
		switch ($attribute) {
			case 'homeDirectory': switch ($id) {
				case 2: return array('INFO', _('Home directory'), sprintf(_('Home directory changed. To keep home directory you have to run the following command as root: \'mv %s %s\''), $this->orig['homeDirectory'][0], $this->attributes['homeDirectory'][0]));
				break;
			}
			break;
			case 'gidNumber': switch ($id) {
				case 0: return array('INFO', _('GID number'), sprintf(_('GID number has changed. To keep file ownership you have to run the following command as root: \'find / -gid %s -uid %s -exec chgrp %s {} \;\''), $this->orig['gidNumber'][0], $this->orig['uidNumber'][0], $_SESSION['cache']->getgid($this->attribtues['gidNumber'][0])));
				break;
			}
			break;
			case 'uidNumber': switch ($id) {
				case 0: return array('INFO', _('UID number'), sprintf(_('UID number has changed. To keep file ownership you have to run the following command as root: \'find / -uid %s -exec chown %s {} \;\''), $this->orig['uidNumber'][0], $this->attributes['uidNumber'][0]));
				break;
			}
			break;
		}
	}

	/** this functin fills the error message array with messages
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
		$this->messages['uidNumber'][1] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
		$this->messages['uidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
		$this->messages['uidNumber'][3] = array('ERROR', _('ID-Number'), _('ID is already in use'));
		$this->messages['uidNumber'][4] = array('ERROR', _('Account %s:') . ' posixAccount_uid', _('UID must be a number. It has to be inside the UID range which is defined in your configuration profile.'));
		$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['userPassword'][3] = array('ERROR', _('Password'), _('You cannot use this password options at the same time.'));
		$this->messages['userPassword'][4] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['uid'][0] = array('INFO', _('UID'), _('UID has changed. Do you want to change home directory?'));
		$this->messages['uid'][1] = array('WARN', _('Username'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->messages['uid'][2] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][3] = array('WARN', _('Hostname'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->messages['uid'][4] = array('ERROR', _('Hostname'), _('Hostname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ ! Hostname must end with $ !'));
		$this->messages['uid'][5] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
		$this->messages['uid'][6] = array('WARN', _('Hostname'), _('Hostname in use. Selected next free hostname.'));
		$this->messages['uid'][7] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][8] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Hostname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['gidNumber'][0] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('LAM was unable to find a group with this name!'));
		$this->messages['gidNumber'][1] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('This GID number is invalid! Please provide either a number or a group name.'));
		$this->messages['gecos'][0] = array('ERROR', _('Account %s:') . ' posixAccount_gecos', _('This gecos value is invalid!'));
		$this->messages['shell'][0] = array('ERROR', _('Account %s:') . ' posixAccount_shell', _('This login shell is invalid!'));
		$this->messages['passwordDisabled'][0] = array('ERROR', _('Account %s:') . ' posixAccount_passwordDisabled', _('This value can only be \"true\" or \"false\".'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
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
			// module dependencies
			$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		}
		// alias name
		$return["alias"] = _("Unix");
		// RDN attributes
		$return["RDN"] = array("uid" => "normal", "cn" => "low");
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
			0 => array('kind' => 'text', 'text' => '<b>' . _("Users") . ': &nbsp;</b>' . _('Minimum UID number') . ": "),
			1 => array('kind' => 'input', 'name' => 'posixAccount_minUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			2 => array('kind' => 'text', 'value' => '&nbsp;'),
			3 => array('kind' => 'text', 'text' => _('Maximum UID number') . ": "),
			4 => array('kind' => 'input', 'name' => 'posixAccount_maxUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			5 => array('kind' => 'help', 'value' => 'minMaxUser'))
		);
		$return['config_options']['host'] = array(
		array(
			0 => array('kind' => 'text', 'text' => '<b>' . _("Hosts") . ': &nbsp;</b>' . _('Minimum UID number') . ": "),
			1 => array('kind' => 'input', 'name' => 'posixAccount_minMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			2 => array('kind' => 'text', 'value' => '&nbsp;'),
			3 => array('kind' => 'text', 'text' => _('Maximum UID number') . ": "),
			4 => array('kind' => 'input', 'name' => 'posixAccount_maxMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
			5 => array('kind' => 'help', 'value' => 'minMaxHost'))
		);
		$return['config_options']['all'] = array(
		array(
			0 => array('kind' => 'text', 'text' => '<b>' . _("Password hash type") . ': &nbsp;</b>'),
			1 => array('kind' => 'select', 'name' => 'posixAccount_pwdHash', 'size' => '1',
				'options' => array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), 'options_selected' => array('SSHA')),
			2 => array('kind' => 'text', 'value' => '&nbsp;'),
			3 => array('kind' => 'text', 'value' => '&nbsp;'),
			4 => array('kind' => 'text', 'value' => '&nbsp;'),
			5 => array('kind' => 'help', 'value' => 'pwdHash'))
		);
		// configuration descriptions
		$return['config_descriptions'] = array(
			'legend' => _("UID ranges for Unix accounts"),
			'descriptions' => array(
			'posixAccount_minUID' => _("Minimum UID number for Unix accounts (users)"),
			'posixAccount_maxUID' => _("Maximum UID number for Unix accounts (users)"),
			'posixAccount_minMachine' => _("Minimum UID number for Unix accounts (hosts)"),
			'posixAccount_maxMachine' => _("Maximum UID number for Unix accounts (hosts)"),
			'posixAccount_pwdHash' => _("Password hash type for Unix accounts"),
		)
		);
		// upload
		$return['upload_preDepends'] = array('inetOrgPerson');
		// user specific upload options
		if ($this->scope == 'user') {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_userName',
				'description' => _('User name'),
				'help' => 'userName', // TODO
				'example' => _('smiller'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uid', // TODO
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group', // TODO
				'example' => _('users'),
				'required' => true
			),
			array(
				'name' => 'posixAccount_additionalGroups',
				'description' => _('Additional groups'),
				'help' => 'additionalGroups', // TODO
				'example' => _('group01,group02')
			),
			array(
				'name' => 'posixAccount_homedir',
				'description' => _('Home directory'),
				'help' => 'homedir', // TODO
				'example' => _('/home/smiller'),
				'default' => '/home/<i>&lt;posixAccount_userName&gt;</i>'
			),
			array(
				'name' => 'posixAccount_shell',
				'description' => _('Login shell'),
				'help' => 'shell', // TODO
				'example' => '/bin/bash',
				'values' => implode(", ", getshells()),
				'default' => '/bin/bash'
			),
			array(
				'name' => 'posixAccount_password',
				'description' => _('Password'),
				'help' => 'password', // TODO
				'example' => _('secret')
			),
			array(
				'name' => 'posixAccount_passwordDisabled',
				'description' => _('Lock password'),
				'help' => 'passwordDisabled', // TODO
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
		elseif ($this->scope == 'host') {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_hostName',
				'description' => _('Host name'),
				'help' => 'hostName', // TODO
				'example' => _('pc01$'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uid', // TODO
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group', // TODO
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
		if ($_SESSION['loggedIn']) {
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('inetOrgPerson', $modules)) {
				$return['upload_columns'][] =
					array(
						'name' => 'posixAccount_description',
						'description' => _('Description'),
						'help' => 'description'
					);
			}
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'uid',
			'uidNumber',
			'gidNumber',
			'gecos',
			'primaryGroup',
			'additionalGroups',
			'homeDirectory',
			'userPassword',
			'loginShell'
		);
		// help Entries
		$return['help'] = array(
			'minMaxUser' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for user IDs when creating new user accounts. The range should be different from that of machines. New user accounts will always get the highest number in use plus one.')
			),
			'minMaxHost' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for machine IDs when creating new accounts for Samba hosts. The range should be different from that of users. New host accounts will always get the highest number in use plus one.')
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'uidNumber' => array(
				"Headline" => _("UID number"),
				"Text" => _("If empty UID number will be generated automaticly.")
			),
			'user' => array(
				'uid' => array(
					"Headline" => _("Username"),
					"Text" => _("Username of the user who should be created. Valid characters are: a-z,0-9, .-_. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. If username is already used username will be expanded with a number. The next free number will be used. Warning: Older systems have problems with usernames longer than 8 characters. You can not log in to Windows if username is longer than 16 characters.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("User description. If left empty sur- and give name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"),
					"Text" => _("The Primary Group the user should be member of.")
				),
				'homeDirectory' => array(
					"Headline" => _("Home directory"),
					"Text" => _('$user and $group are replaced with username or primary groupname.')
				),
				'userPassword' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
				),
				'userPassword_no' => array(
					"Headline" => _("Use no password"),
					"Text" => _("If checked no password will be used.")
				),
				/*'userPassword_lock'	=>*/
				'loginShell' => array(
					"Headline" => _("Login shell"),
					"Text" => _("To disable login use /bin/false. List of shells is read from lam/config/shells")
				),
				'addgroup' => array(
					"Headline" => _("Additional groups"),
					"Text" => _("Hold the CTRL-key to (de)select multiple groups."). ' '. _("Can be left empty.")
				)
			),
			'host' => array(
				'uid' => array(
					"Headline" => _("Host name"),
					"Text" => _("Host name of the host which should be created. Valid characters are: a-z,0-9, .-_$. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. Hostnames are always ending with $. If last character is not $ it will be added. If hostname is already used hostname will be expanded with a number. The next free number will be used.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("Host description. If left empty host name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"),
					"Text" => _("The Primary group the host should be member of.")
				)
			)
		);

		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
		$this->createhomedir=false;
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return;
		}
	}

	// Variables
	// Use a unix password?
	var $userPassword_nopassword;
	// Use invalid password, '*', e.g. * for services
	var $userPassword_invalid;
	// Lock password
	var $userPassword_lock;
	/* These two variables keep an array of groups the
	* user is also member of.
	*/
	var $groups;
	var $groups_orig;
	var $createhomedir;

	/* $attribute['userPassword'] can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function userPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			if ($newpassword=='')
				$this->attributes['userPassword'][0] = '';
			else $this->attributes['userPassword'][0] = base64_encode($_SESSION['ldap']->encrypt($newpassword));
			return 0;
		}
		else {
			if ($this->attributes['userPassword'][0]!='') // Read existing password if set
				return $_SESSION['ldap']->decrypt(base64_decode($this->attributes['userPassword'][0]));
			else return '';
		}
	}

	function module_ready() {
		return true;
	}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['uid'][0] == '') return false;
		if ($this->attributes['uidNumber'][0] == '') return false;
		if ($this->attributes['gidNumber'][0] == '') return false;
		if ($this->attributes['homeDirectory'][0] == '') return false;
		if ($this->attributes['loginShell'][0] == '') return false;
		return true;
	}

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'group');
	}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		parent::load_attributes($attr);

		// get all additional groupmemberships
		$dn_groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
		if (is_array($dn_groups)) {
			if (!is_array($this->groups)) $this->groups = array();
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN) {
				if (in_array($attr['uid'][0], $dn_groups[$DN])) {
					$this->groups[] = substr($DN, 3, strpos($DN, ',')-3);
				}
			}
			sort($this->groups);
			$this->groups_orig = $this->groups;
		}
		// Delete password. We don't want to show an encrypted password because it makes no sense
		$this->userPassword('');
		return 0;
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
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// fill gecos with correct attributes frm inetOrgPerson when they are set
		// TODO do we have t use 7bit ascci only?
		// TODO change gecos evertime something in inetOrgPerson has changes
		if (isset($_SESSION[$this->base]->modules['inetOrgPerson'])) {
			if (isset($return[$_SESSION[$this->base]->dn]['add']['gecos']))
				$return[$_SESSION[$this->base]->dn]['add']['gecos'] = $_SESSION[$this->base]->modules['inetOrgPerson']->attributes['givenName'][0] .
					" " . $_SESSION[$this->base]->modules['inetOrgPerson']->attributes['sn'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['roomNumber'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['telephoneNumber'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['homePhone'][0] ;
			if (isset($return[$_SESSION[$this->base]->dn]['modify']['gecos']))
				$return[$_SESSION[$this->base]->dn]['modify']['gecos'] = $_SESSION[$this->base]->modules['inetOrgPerson']->attributes['givenName'][0] .
					" " . $_SESSION[$this->base]->modules['inetOrgPerson']->attributes['sn'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['roomNumber'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['telephoneNumber'][0] . "," .
					$_SESSION[$this->base]->modules['inetOrgPerson']->attributes['homePhone'][0] ;
		}
		// unset password when needed
		if (isset($return[$_SESSION[$this->base]->dn]['add']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['userPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['userPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['userPassword']);
		// Set unix password
		if (isset($this->orig['userPassword'][0])) {
			if ($this->userPassword_nopassword) // use no password
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
			else if ($this->userPassword_invalid) // use '*' as password
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = '*';
			else if (($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) && $this->userPassword()!='') // set password if set
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
			else if ($this->userPassword_lock && (pwd_disable($this->orig['userPassword'][0]) != $this->orig['userPassword'][0])) // lock account if required
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_disable($this->orig['userPassword'][0]);
			else // password hasn't changed
				$return[$_SESSION[$this->base]->dn]['notchanged']['userPassword'][0] = $this->orig['userPassword'][0];
		}
		else {
			// New user or no old password set
			if ($this->userPassword_nopassword) // use no password
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
			else if ($this->userPassword_invalid) // use '*' as password
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = '*';
			else if ($this->userPassword()!='') // set password if set
				$return[$_SESSION[$this->base]->dn]['add']['userPassword'][0] = pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
		}

		// Remove primary group from additional groups
		for ($i=0; $i<count($this->groups); $i++) {
			if ($this->groups[$i]==$_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0])) unset($this->groups[$i]);
		}

		// Set additional group memberships
		if ($this->orig['uid'][0]!='' && $this->attributes['uid'][0]!=$this->orig['uid'][0]) {
			// remove old memberships
			$dn_groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN)
			if (in_array($this->orig['uid'][0], $dn_groups[$DN]))
			$return[$DN]['remove']['memberUid'][0] = $this->orig['uid'][0];
			// Add new memberships
			if (is_array($this->groups))
			foreach ($this->groups as $group) {
				$dn = $_SESSION['ldap']->in_cache ($group, 'cn', 'group');
				$return[$dn]['add']['memberUid'][0] = $this->attributes['uid'][0];
			}
		}
		else {
			if (is_array($this->groups)) {
				// There are some additional groups defined
				if (is_array($this->groups_orig)) {
					//There are some old groups.
					$add = array_delete($this->groups_orig, $this->groups);
					$remove = array_delete($this->groups, $this->groups_orig);
					$dn_cns = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (is_array($add))
						if (in_array($dn_cns[$DN][0], $add)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'][0];
						if (is_array($remove))
						if (in_array($dn_cns[$DN][0], $remove)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'][0];
					}
					// primary group mut also be removed if it has changed after setting additional groups
					if (in_array($_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]), $this->groups_orig)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'];
				}
				else {
					// Add user to every group
					$dn_cns = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (in_array($dn_cns[$DN][0], $this->groups)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'][0];
					}
				}
			}
			else {
				if (is_array($this->groups_orig)) {
					//There are some old groups which have to be removed
					$dn_cns = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (in_array($dn_cns[$DN][0], $this->orig['groups'])) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'][0];
					}
				}
			}
		}
		// fixme TODO lamdeamon without DN
		if ($this->createhomedir) $return[$_SESSION[$this->base]->dn]['lamdaemon']['command'][] = $this->attributes['uid'][0] . " home add";
		return $return;
	}

	function delete_attributes($post) {
		$return = array();
		// remove memberUids if set
		$groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($groups);
		for ($i=0; $i<count($DNs); $i++) {
			if (in_array($this->attributes['uid'][0], $groups[$DNs[$i]])) $return[$DNs[$i]]['remove']['memberUid'][] = $this->attributes['uid'][0];
		}
		if ($post['deletehomedir']) $return[$_SESSION[$this->base]->dn_orig]['lamdaemon']['command'][] = $this->attributes['uid'][0] . " home rem";
		return $return;
	}

	/* Write variables into object and do some regexp checks
	*/
	function process_attributes(&$post) {
		$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
		if (count($groups)==0) {
			return array("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
		}
		$this->attributes['homeDirectory'][0] = $post['homeDirectory'];
		$this->attributes['loginShell'][0] = $post['loginShell'];
		if (isset($post['gecos'])) $this->attributes['gecos'][0] = $post['gecos'];
		if ($post['createhomedir']) $this->createhomedir = true;
		else $this->createhomedir = false;
		if ($this->orig['uid'][0]!='' && $post['uid']!=$this->attributes['uid'][0])
			$triggered_messages['uid'][] = $this->messages['uid'][0];
		if ($this->orig['gidNumber'][0]!='' && $_SESSION['cache']->getgid($post['gidNumber'])!=$this->attributes['gidNumber'][0])
			$triggered_messages['gidNumber'][] = $this->dynamic_Message('gidNumber',0);
		if ($this->orig['uidNumber'][0]!='' && $post['uidNumber']!=$this->attributes['uidNumber'][0])
			$triggered_messages['uidNumber'][] = $this->dynamic_Message('uidNumber',0);
		if (isset($post['homeDirectory']) && $this->orig['homeDirectory'][0]!='' && $post['homeDirectory']!=$this->attributes['homeDirectory'][0])
			$triggered_messages['homeDirectory'][] = $this->dynamic_Message('homeDirectory',0);
		// Load attributes
		$this->attributes['uid'][0] = $post['uid'];
		$this->attributes['cn'][0] = $this->attributes['uid'][0];
		$this->attributes['uidNumber'][0] = $post['uidNumber'];
		$this->attributes['gidNumber'][0] = $_SESSION['cache']->getgid($post['gidNumber']);
		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		if ($_SESSION[$this->base]->type=='user') {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		if ($_SESSION[$this->base]->type=='host') {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$dn_uids = $_SESSION['cache']->get_cache('uidNumber', 'posixAccount', '*');
		// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		if(is_array($dn_uids)) {
			foreach ($dn_uids as $uid) $uids[] = $uid[0];
			sort ($uids, SORT_NUMERIC);
		}
		if ($this->attributes['uidNumber'][0]=='') {
			// No id-number given
			if ($this->orig['uidNumber'][0]=='') {
				// new account -> we have to find a free id-number
				$newUID = $this->getNextUIDs(1, $triggered_messages);
				if (is_array($newUID)) {
					$this->attributes['uidNumber'][0] = $newUID[0];
				}
				else {
					$triggered_messages['uidNumber'][] = $this->messages['uidNumber'][3];
				}
			}
			else $this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
			// old account -> return id-number which has been used
		}
		else {
			// Check manual ID
			// id-number is out of valid range
			if ( ($this->attributes['uidNumber'][0]!=$post['uidNumber']) && ($this->attributes['uidNumber'][0] < $minID || $this->attributes['uidNumber'][0] > $maxID)) $triggered_messages['uidNumber'][] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
			// $uids is allways an array but not if no entries were found
			if (is_array($uids)) {
				// id-number is in use and account is a new account
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $triggered_messages['uidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
					$triggered_messages['uidNumber'][] = $this->messages['uidNumber'][3];
					$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
				}
			}
		}
		if ($_SESSION[$this->base]->type=='user') {
			if (($this->attributes['uid'][0] != $post['uid']) &&  !get_preg($post['uid'], '!upper'))
			$triggered_messages['uid'][] = $this->messages['uid'][1];
			if ( !get_preg($this->attributes['homeDirectory'][0], 'homeDirectory' ))
			$triggered_messages['homeDirecotry'][] = $this->messages['homeDirectory'][0];
		}
			if (($post['userPassword_lock'] && $post['userPassword_invalid']) || ($post['userPassword_nopassword'] && $post['userPassword_invalid'])) {
			// found invalid password parameter combination
			$triggered_messages['userPassword'][] = $this->messages['userPassword'][3];
		}
		else {
			if ($post['userPassword_nopassword']) {
				$this->userPassword_nopassword=true;
				$this->userPassword_invalid=false;
				$this->userPassword('');
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
					$this->userPassword('');
					$post['userPassword2'] = '';
				}
				else {
					$this->userPassword_invalid=false;
					if ($post['genpass']) $this->userPassword(genpasswd());
					elseif ($_SESSION[$this->base]->isNewAccount) {
						if ($post['userPassword'] != $post['userPassword2'])
							$triggered_messages['userPassword'][] = $this->messages['userPassword'][0];
						else $this->userPassword($post['userPassword']);
						if (!get_preg($this->userPassword(), 'password'))
							$triggered_messages['userPassword'][] = $this->messages['userPassword'][1];
					}
					if ($post['userPassword_lock']) $this->userPassword_lock=true;
					else $this->userPassword_lock=false;
				}
			}
			if ($_SESSION[$this->base]->type=='user') {
				$this->attributes['homeDirectory'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
				if ($this->attributes['uid'][0] != '')
				$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
				if ($this->attributes['homeDirectory'][0] != $post['homeDirectory']) $triggered_messages['homeDirecotry'][] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
				// Check if Username contains only valid characters
				if ( !get_preg($this->attributes['uid'][0], 'username') && !$profile)
				$triggered_messages['uid'][] = $this->messages['uid'][2];
			}
			if ($_SESSION[$this->base]->type=='host') {
				// Check if Hostname contains only valid characters
				if ( !get_preg($this->attributes['uid'][0], 'hostname'))
				$triggered_messages['uid'][] = $this->messages['uid'][4];
			}
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			// Set username back to original name if new username is in use
			if ($_SESSION['cache']->in_cache($this->attributes['uid'][0],'uid', '*') && ($this->orig['uid'][0]!=''))
			$this->attributes['uid'][0] = $this->orig['uid'][0];
			// Change uid to a new uid until a free uid is found
			else
			while ($_SESSION['cache']->in_cache($this->attributes['uid'][0], 'uid', '*')) {
				if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = substr($this->attributes['uid'][0], 0, -1);
				// get last character of username
				$lastchar = substr($this->attributes['uid'][0], strlen($this->attributes['uid'][0])-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = $this->attributes['uid'][0] . '2$';
				else $this->attributes['uid'][0] = $this->attributes['uid'][0] . '2';
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
					if (ereg('^([0-9])+$',substr($this->attributes['uid'][0], $i, strlen($this->attributes['uid'][0])-$i))) $i--;
					else $mark=true;
					// increase last number with one
					$firstchars = substr($this->attributes['uid'][0], 0, $i+1);
					$lastchars = substr($this->attributes['uid'][0], $i+1, strlen($this->attributes['uid'][0])-$i);
					// Put username together
					if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1)."$";
					else $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1);
				}
			}
			// Show warning if lam has changed username
			if ($this->attributes['uid'][0] != $post['uid']) {
				if ($_SESSION[$this->base]->type=='user')
				$triggered_messages['uid'][] = $this->messages['uid'][5];
			if ($_SESSION[$this->base]->type=='host')
				$triggered_messages['uid'][] = $this->messages['uid'][6];
			}
			if ($_SESSION[$this->base]->isNewAccount && !get_preg($this->userPassword(), 'password'))
			$triggered_messages['userPassword'][] = $this->messages['userPassword'][1];
		}
	
	$temp = $this->input_check();
	// TODO is this really OK?
	if (is_array($temp)) $triggered_messages = array_merge_recursive($triggered_messages, $temp);
	// Return error-messages
	if (count($triggered_messages)!=0) {
		$this->triggered_messages = $triggered_messages;
		return $triggered_messages;
	}
	else $this->triggered_messages = array();
	// Go to additional group page when no error did ocour and button was pressed
	if ($post['addgroup'])  return 'group';
	if ($post['changepass'])  return 'password';
	return 0;
	}

	/* Write variables into object and do some regexp checks
	*/
	function process_group(&$post) {
		do { // X-Or, only one if() can be true
		if (isset($post['addgroups']) && isset($post['addgroups_button'])) { // Add groups to list
		// Add new group
		$this->groups = @array_merge($this->groups, $post['addgroups']);
		// sort groups
		sort($this->groups);
		break;
		}
		if (isset($post['removegroups']) && isset($post['removegroups_button'])) { // remove groups from list
		$this->groups = array_delete($post['removegroups'], $this->groups);
		break;
		}
		} while(0);
	if (isset($post['addgroups_button']) || isset($post['removegroups_button'])) return 'group';
	if ($post['back']) return 'attributes';
	return 0;
	}

	/**
	* Sets a new password.
	*
	* @param $post HTTP POST
	*/
	function process_password(&$post) {
		if ($post['back']) return 'attributes';
		$messages = array();
		if ($post['userPassword'] != $post['userPassword2']) {
			$messages['userPassword'][] = $this->messages['userPassword'][0];
			if (!get_preg($post['userPassword'], 'password'))
				$messages['userPassword'][] = $this->messages['userPassword'][1];
			}
			else $this->userPassword($post['userPassword']);
		if (sizeof($messages) > 0) return $messages;
		else return 'attributes';
	}
	
	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return;
		}
		$shelllist = getshells(); // list of all valid shells

		$return[] = array (
			0 => array('kind' => 'text', 'text' => _("Username").'*'),
			1 => array('kind' => 'input', 'name' => 'uid', 'type' => 'text', 'size' => '20', 'maxlength' => '20', 'value' => $this->attributes['uid'][0]),
			2 => array('kind' => 'help', 'value' => 'uid'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('UID number').'*'),
			1 => array('kind' => 'input', 'name' => 'uidNumber', 'type' => 'text', 'size' => '6', 'maxlength' => '6', 'value' => $this->attributes['uidNumber'][0]),
			2 => array('kind' => 'help', 'value' => 'uidNumber'));
		if (!isset($_SESSION[$this->base]->modules['inetOrgPerson'])) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Gecos')),
				1 => array('kind' => 'input', 'name' => 'gecos', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['gecos'][0]),
				2 => array('kind' => 'help', 'value' => 'gecos'));
		}
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Primary group').'*' ),
			1 => array('kind' => 'select', 'name' => 'gidNumber', 'options' => $groups, 'options_selected' => array($_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]))),
			2 => array('kind' => 'help', 'value' => 'gidNumber'));

		if ($_SESSION[$this->base]->type=='user') {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Additional groups')),
				1 => array('kind' => 'input', 'name' => 'addgroup', 'type' => 'submit', 'value' => _('Edit groups')),
				2 => array('kind' => 'help', 'value' => 'addgroup'));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home directory').'*'),
				1 => array('kind' => 'input', 'name' => 'homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['homeDirectory'][0]),
				2 => array('kind' => 'help', 'value' => 'homeDirectory'));
			if ($this->orig['homeDirectory']=='' && isset($_SESSION['config']->scriptPath)) {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Create home directory')),
					1 => array('kind' => 'input', 'name' => 'createhomedir', 'type' => 'checkbox', 'checked' => $this->createhomedir),
					2 => array('kind' => 'help', 'value' => 'createhomedir'));
				}
			if ($_SESSION[$this->base]->isNewAccount) {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Password') ),
					1 => array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $this->userPassword()),
					2 => array('kind' => 'input', 'name' => 'genpass', 'type' => 'submit', 'value' => _('Generate password')));
				if ($post['userPassword2']!='') $password2 = $post['userPassword2'];
				else $password2 = $this->userPassword();
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Repeat password')),
					1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
					2 => array('kind' => 'help', 'value' => 'userPassword'));
			}
			else {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Password') ),
					1 => array('kind' => 'input', 'name' => 'changepass', 'type' => 'submit', 'value' => _('Change password')));
			}
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Set no password')),
				1 => array('kind' => 'input', 'name' => 'userPassword_nopassword', 'type' => 'checkbox', 'checked' => $this->userPassword_nopassword),
				2 => array('kind' => 'help', 'value' => 'userPassword_nopassword'));
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
			if (count($shelllist)!=0) {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Login shell').'*'),
					1 => array('kind' => 'select', 'name' => 'loginShell', 'options' => $shelllist, 'options_selected' => array ($this->attributes['loginShell'][0])),
					2 => array('kind' => 'help', 'value' => 'loginShell'));
			}
		}
	return $return;
	}

	function display_html_delete(&$post) {
		if ($this->scope == 'user' && isset($_SESSION['config']->scriptPath)) {
			$return[] = array (
				0 => array('kind' => 'text', 'text' => _('Delete home directory')),
				1 => array('kind' => 'input', 'name' => 'deletehomedir', 'type' => 'checkbox'),
				2 => array('kind' => 'help', 'value' => 'deletehomedir'));
		}
		return $return;
	}

	/**
	* Displays the group selection.
	*
	* @param array $post HTTP-POST
	* @return array meta HTML code
	*/
	function display_html_group(&$post) {
		// load list with all groups
		$dn_groups = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN)
		$groups[] = substr($DN, 3, strpos($DN, ',')-3);
		// remove groups the user is member of from grouplist
		$groups = array_delete($this->groups, $groups);
		// Remove primary group from grouplist
		$group = $_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]);
		$groups = array_flip($groups);
		unset ($groups[$group]);
		$groups = array_flip($groups);
		// sort groups
		sort($groups, SORT_STRING);

		$return[] = array(
			0 => array('kind' => 'fieldset', 'legend' => _("Additional groups"), 'value' => array(
				0 => array(
					0 => array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Selected groups"), 'value' => array(
						0 => array (
							0 => array('kind' => 'select', 'name' => 'removegroups', 'size' => '15', 'multiple' => true, 'options' => $this->groups)))),
						1 => array('kind' => 'table', 'value' => array(
							0 => array(
								0 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addgroups_button', 'value' => '<=', 'td' => array('align' => 'center'))),
							1 => array(
								0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>', 'td' => array('align' => 'center'))),
							2 => array(
								0 => array ( 'kind' => 'help', 'value' => 'addgroup', 'td' => array('align' => 'center'))))),
						2 => array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Available groups"), 'value' => array(
							0 => array(
								0 => array('kind' => 'select', 'name' => 'addgroups', 'size' => '15', 'multiple' => true, 'options' => $groups))))
		))));

		$return[] = array(
			0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'back'),
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
			2 => array('kind' => 'help', 'value' => 'userPassword'));
		if ($post['userPassword2']!='') $password2 = $post['userPassword2'];
		else $password2 = $this->userPassword();
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Repeat password')),
			1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ""));
		$return[] = array(
			0 => array('kind' => 'table', 'value' => array(
				0 => array(
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Submit'), 'name' => 'submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'back'),
					2 => array('kind' => 'text')))));
		return $return;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		if ($this->scope == 'user') {
			$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
			$shelllist = getshells(); // list of all valid shells
			// primary Unix group
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Primary group') . ": "),
			1 => array('kind' => 'select', 'name' => 'posixAccount_primaryGroup', 'options' => $groups, 'options_selected' => array(), 'size' => 1),
			2 => array('kind' => 'help', 'value' => 'gidNumber', 'scope' => 'user'));
			// additional group memberships
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Additional groups') . ": "),
			1 => array('kind' => 'select', 'name' => 'posixAccount_additionalGroup', 'options' => $groups,
			'options_selected' => array(), 'size' => 10, 'multiple' => true),
			2 => array('kind' => 'help', 'value' => 'addgroup', 'scope' => 'user'));
			// home directory
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Home directory') . ": "),
			1 => array('kind' => 'input', 'name' => 'posixAccount_homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => '/home/$user'),
			2 => array('kind' => 'help', 'value' => 'homeDirectory', 'scope' => 'user'));
			// login shell
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Login shell') . ": "),
			1 => array('kind' => 'select', 'name' => 'posixAccount_loginShell', 'options' => $shelllist, 'options_selected' => array("/bin/bash")),
			2 => array('kind' => 'help', 'value' => 'loginShell', 'scope' => 'user'));
			// do not set password
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Set no password') . ": "),
			1 => array('kind' => 'input', 'name' => 'posixAccount_userPassword_no', 'type' => 'checkbox', 'checked' => false),
			2 => array('kind' => 'help', 'value' => 'posixAccount_userPassword_no', 'scope' => 'user'));
			// disable account
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Lock password') . ": "),
			1 => array('kind' => 'input', 'name' => 'posixAccount_userPassword_lock', 'type' => 'checkbox', 'checked' => false),
			2 => array('kind' => 'help', 'value' => 'posixAccount_userPassword_lock', 'scope' => 'user'));
		}
		elseif ($this->scope == 'host') {
			$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
			// primary Unix group
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Primary group') . ": "),
			1 => array('kind' => 'select', 'name' => 'posixAccount_primaryGroup', 'options' => $groups, 'options_selected' => array(), 'size' => 1),
			2 => array('kind' => 'help', 'value' => 'gidNumber', 'scope' => 'host'));
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
			$gid = $_SESSION['cache']->getgid($profile['posixAccount_primaryGroup'][0]);
			if (isset($gid)) {
				$this->attributes['gidNumber'][0] = $gid;
			}
		}
		// other group memberships
		if (isset($profile['posixAccount_additionalGroup'][0])) {
			$this->groups = $profile['posixAccount_additionalGroup'];
		}
		// no password
		if (isset($profile['posixAccount_userPassword_no'][0])) {
			$this->userPassword_nopassword = $profile['posixAccount_userPassword_no'][0];
		}
		// locked password
		if (isset($profile['posixAccount_userPassword_lock'][0])) {
			$this->userPassword_lock = $profile['posixAccount_userPassword_lock'][0];
		}
	}

	/*
	* (non-PHPDoc)
	* @see baseModule#get_pdfEntries
	*/
	function get_pdfEntries($account_type = "user") {
		return array(	'posixAccount_uid' => array('<block><key>' . _('Username') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'),
		'posixAccount_uidNumber' => array('<block><key>' . _('UID number') . '</key><value>' . $this->attributes['uidNumber'][0] . '</value></block>'),
		'posixAccount_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
		'posixAccount_gecos' => array('<block><key>' . _('Gecos') . '</key><value>' . $this->attributes['gecos'][0] . '</value></block>'),
		'posixAccount_primaryGroup' => array('<block><key>' . _('Primary group') . '</key><value>' . $_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]) . '</value></block>'),
		'posixAccount_additionalGroups' => array('<block><key>' . _('Additional groups') . '</key><value>' . '</value></block>'),
		'posixAccount_homeDirectory' => array('<block><key>' . _('Home directory') . '</key><value>' . $this->attributes['homeDirectory'][0] . '</value></block>'),
		'posixAccount_userPassword' => array('<block><key>' . _('Password') . '</key><value>' . $this->attributes['userPassword'][0] . '</value></block>'),
		'posixAccount_loginShell' => array('<block><key>' . _('Login shell') . '</key><value>' . $this->attributes['loginShell'][0] . '</value></block>'),
		);
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
			if (!isset($options['posixAccount_minUID'][0]) || !ereg('^[0-9]+$', $options['posixAccount_minUID'][0])) {
				$return[] = $this->messages['minUID'][0];
			}
			if (!isset($options['posixAccount_maxUID'][0]) || !ereg('^[0-9]+$', $options['posixAccount_maxUID'][0])) {
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
			if (!isset($options['posixAccount_minMachine'][0]) || !ereg('^[0-9]+$', $options['posixAccount_minMachine'][0])) {
				$return[] = $this->messages['minMachine'][0];
			}
			if (!isset($options['posixAccount_maxMachine'][0]) || !ereg('^[0-9]+$', $options['posixAccount_maxMachine'][0])) {
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
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		$triggered_messages = array();
		$needAutoUID = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixAccount";
			// UID
			if ($rawAccounts[$i][$ids['posixAccount_uid']] == "") {
				// autoUID
				$needAutoUID[] = $i;
			}
			elseif (get_preg($rawAccounts[$i][$ids['posixAccount_uid']], 'digit')) {
				if (($this->get_scope() == 'user') &&
				($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minUID'][0]) &&
				($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxUID'][0])) {
					$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
				}
				elseif (($this->get_scope() == 'host') &&
				($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minMachine'][0]) &&
				($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxMachine'][0])) {
					$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
				}
			}
			else {
				$errMsg = $this->messages['uidNumber'][4];
				array_push($errMsg, array($i));
				$triggered_messages[] = $errMsg;
			}
			// GID number
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixAccount_group']];
			}
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'groupname')) {
				$partialAccounts[$i]['gidNumber'] = 42;
				$gid = $_SESSION['cache']->getgid($rawAccounts[$i][$ids['posixAccount_group']]);
				if (is_numeric($gid)) {
					$partialAccounts[$i]['gidNumber'] = $gid;
				}
				else {
					$errMsg = $this->messages['gidNumber'][0];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
				}
			}
			else {
				$errMsg = $this->messages['gidNumber'][1];
				array_push($errMsg, array($i));
				$triggered_messages[] = $errMsg;
			}
			// GECOS // TODO fill default values
			if (($rawAccounts[$i][$ids['posixAccount_gecos']] != "") && (get_preg($rawAccounts[$i][$ids['posixAccount_gecos']], 'gecos'))) {
				$partialAccounts[$i]['gecos'] = $rawAccounts[$i][$ids['posixAccount_gecos']];
			}
			else {
				$errMsg = $this->messages['gecos'][0];
				array_push($errMsg, array($i));
				$triggered_messages[] = $errMsg;
			}
			// user specific attributes
			if ($this->scope == 'user') {
				// user name
				if (get_preg($rawAccounts[$i][$ids['posixAccount_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][7];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
				}
				// password
				if (($rawAccounts[$i][$ids['posixAccount_password']] != "") && (get_preg($rawAccounts[$i][$ids['posixAccount_password']], 'password'))) {
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixAccount_password']], $pwd_enabled, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				elseif ($rawAccounts[$i][$ids['posixAccount_password']] != "") {
					$errMsg = $this->messages['userPassword'][4];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
				}
				// description
				$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
				if (!in_array('inetOrgPerson', $modules)) {
					if ($rawAccounts[$i][$ids['posixAccount_description']] && ($rawAccounts[$i][$ids['posixAccount_description']] != '')) { 
						$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_description']];
					}
					else {
						$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
					}
				}
			}
			// host specific attributes
			elseif ($this->scope == 'host') {
				// host name
				if (get_preg($rawAccounts[$i][$ids['posixAccount_hostName']], 'hostname')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
				}
				else {
					$errMsg = $this->messages['uid'][8];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
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
				$triggered_messages[] = $this->messages['uidNumber'][2];
			}
		}
		return $triggered_messages;
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
		// on first call generate list of ldap operations
		if (!isset($temp['counter'])) {
			$temp['groups'] = array();
			$temp['counter'] = 0;
			$col = $ids['posixAccount_additionalGroups'];
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				if ($data[$i][$col] != "") {
					$groups = explode(",", $data[$i][$col]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $temp['groups'])) $temp['groups'][] = $groups[$g];
						$temp['members'][$groups[$g]][] = $data[$i][$ids['posixAccount_userName']];
					}
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
				$success = @ldap_mod_add($_SESSION['ldap']->server, $temp['dn'][$temp['groups'][$temp['counter']]], array('memberUID' => $temp['members'][$temp['groups'][$temp['counter']]]));
				$errors = array();
				if (!$success) {
					$errors[] = array(
						"ERROR",
						_("LAM was unable to modify group memberships for group: %s"),
						ldap_errno($_SESSION[ldap]->server) . ": " . ldap_error($_SESSION[ldap]->server),
						array($temp['groups'][$temp['counter']])
					);
				}
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / sizeof($temp['groups']),
					'errors' => $errors
				);
			}
			else {
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / sizeof($temp['groups']),
					'errors' => array(array('ERROR', _('Unable to find group in LDAP.'), $temp['groups'][$temp['counter']]))
				);
			}
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
	* @param array $triggered_messages list of error messages where errors can be added
	* @return mixed Null if no UIDs are free else an array of free UIDs. 
	*/
	function getNextUIDs($count, &$triggered_messages) {
		$ret = array();
		if ($this->scope == "user") {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		else {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$dn_uids = $_SESSION['cache']->get_cache('uidNumber', 'posixAccount', '*');
		// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		$uids = array();
		if(is_array($dn_uids)) {
			foreach ($dn_uids as $uid) {
				if (($uid[0] < $maxID) && ($uid[0] > $minID)) $uids[] = $uid[0];  // ignore UIDs > maxID and UIDs < minID
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
					$triggered_messages['uidNumber'][] = $this->messages['uidNumber'][2];
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

}

?>
