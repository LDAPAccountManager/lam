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
	$this->messages['cmp_both'][0] = array('ERROR', _('UID ranges'), _("The UID ranges for users and hosts overlap! This is a problem because LAM uses the highest UID in use + 1. Please set the minimum UID to equal values or use independent ranges."));
	$this->messages['homeDirectory'][0] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));	
	$this->messages['homeDirectory'][1] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));	
	$this->messages['uidNumber'][1] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
	$this->messages['uidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
	$this->messages['uidNumber'][3] = array('ERROR', _('ID-Number'), _('ID is already in use'));
 	$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.')); 
	$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
	$this->messages['uid'][0] = array('INFO', _('UID'), _('UID has changed. Do you want to change home directory?'));
	$this->messages['uid'][1] = array('WARN', _('Username'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
	$this->messages['uid'][2] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
	$this->messages['uid'][3] = array('WARN', _('Hostname'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
	$this->messages['uid'][4] = array('ERROR', _('Hostname'), _('Hostname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ ! Hostname must end with $ !'));
	$this->messages['uid'][5] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
	$this->messages['uid'][6] = array('WARN', _('Hostname'), _('Hostname in use. Selected next free hostname.'));
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
			// this is a base module
			$return["is_base"] = true;
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixAccount)", 'and' => "(!(uid=*$))");
			// module dependencies
			$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		}
		elseif ($this->get_scope() == "host") {
			// module dependencies
			$return['dependencies'] = array('depends' => array('account'), 'conflicts' => array());
		}
		// alias name
		$return["alias"] = _("Unix");
		// profile checks
		$return['profile_checks']['posixAccount_homeDirectory'] = array('type' => 'regex_i', 'regex' => 'homeDirectory',
			'error_message' => $this->messages['homeDirectory'][0]);
		// configuration options
		$return['config_options']['user'] = array(
			array(
				0 => array('kind' => 'text', 'text' => '<b>' . _("Users") . ': &nbsp;</b>' . _('Minimum UID number') . ": "),
				1 => array('kind' => 'input', 'name' => 'posixAccount_minUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				2 => array('kind' => 'text', 'value' => '&nbsp;'),
				3 => array('kind' => 'text', 'text' => _('Maximum UID number') . ": "),
				4 => array('kind' => 'input', 'name' => 'posixAccount_maxUID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				5 => array('kind' => 'help', 'value' => 'TODO'))
		);
		$return['config_options']['host'] = array(
			array(
				0 => array('kind' => 'text', 'text' => '<b>' . _("Hosts") . ': &nbsp;</b>' . _('Minimum UID number') . ": "),
				1 => array('kind' => 'input', 'name' => 'posixAccount_minMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				2 => array('kind' => 'text', 'value' => '&nbsp;'),
				3 => array('kind' => 'text', 'text' => _('Maximum UID number') . ": "),
				4 => array('kind' => 'input', 'name' => 'posixAccount_maxMachine', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
				5 => array('kind' => 'help', 'value' => 'TODO'))
		);
		$return['config_options']['all'] = array(
			array(
				0 => array('kind' => 'text', 'text' => '<b>' . _("Password hash type") . ': &nbsp;</b>'),
				1 => array('kind' => 'select', 'name' => 'posixAccount_pwdHash', 'size' => '1',
					'options' => array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN"), 'options_selected' => array('SSHA')),
				2 => array('kind' => 'text', 'value' => '&nbsp;'),
				3 => array('kind' => 'text', 'value' => '&nbsp;'),
				4 => array('kind' => 'text', 'value' => '&nbsp;'),
				5 => array('kind' => 'help', 'value' => 'TODO'))
		);
		// configuration descriptions
		$return['config_descriptions'] = array(
			'legend' => _("UID ranges for Unix accounts"),
			'descriptions' => array(
				'posixAccount_minUID' => _("Minimum UID number for Unix accounts (users)"),
				'posixAccount_maxUID' => _("Maximum UID number for Unix accounts (users)"),
				'posixAccount_minMachine' => _("Minimum UID number for Unix accounts (hosts)"),
				'posixAccount_maxMachine' => _("Maximum UID number for Unix accounts (hosts)"),
				'posixAccount_pwdHash' => _("Password hash type"),
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(	'uid',
													'uidNumber',
													'gidNumber',
													'gecos',
													'primaryGroup',
													'additionalGroups',
													'homeDirectory',
													'userPassword',
													'loginShell');
		// help Entries
		$return['help'] = array (	'user'						=>	array(	'uid'				=>	array ("ext" => "FALSE", "Headline" => _("Username"), "Text" => _("Username of the user who should be created. Valid characters are: a-z,0-9, .-_. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. If username is already used username will be expanded with a number. The next free number will be used. Warning: Older systems have problems with usernames longer than 8 characters. You can not log in to Windows if username is longer than 16 characters.")),
																								'gecos'			=>	array ("ext" => "FALSE", "Headline" => _("Gecos"), "Text" => _("User description. If left empty sur- and give name will be used.")),
																								'gidNumber'	=>	array ("ext" => "FALSE", "Headline" => _("Primary group"), "Text" => _("The Primary Group the user should be member of."))),
											'host'						=>	array(	'uid'				=>	array ("ext" => "FALSE", "Headline" => _("Host name"), "Text" => _("Host name of the host which should be created. Valid characters are: a-z,0-9, .-_$. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. Hostnames are always ending with $. If last character is not $ it will be added. If hostname is already used hostname will be expanded with a number. The next free number will be used.")),
																								'gecos'			=>	array ("ext" => "FALSE", "Headline" => _("Gecos"), "Text" => _("Host description. If left empty host name will be used.")),
																								'gidNumber'	=>	array ("ext" => "FALSE", "Headline" => _("Primary group"), "Text" => _("The Primary group the host should be member of."))),
											'uidNumber'				=>	array ("ext" => "FALSE", "Headline" => _("UID number"), "Text" => _("If empty UID number will be generated automaticly.")),
											'addgroup'					=>	array ("ext" => "FALSE", "Headline" => _("Additional groups"), "Text" => _("Hold the CTRL-key to (de)select multiple groups."). ' '. _("Can be left empty.")),
											'homeDirectory'			=>	array ("ext" => "FALSE", "Headline" => _("Home directory"), "Text" => _("$user and $group are replaced with username or primary groupname.")),
											/*'userPassword'		=>*/
											'userPassword_no'	=>	array ("ext" => "FALSE", "Headline" => _("Use no password"), "Text" => _("If checked no password will be used.")),
											/*'userPassword_lock'	=>*/
											'loginShell'					=>	array ("ext" => "FALSE", "Headline" => _("Login shell"), "Text" => _("To disable login use /bin/false. List of shells is read from lam/config/shells")));	
		
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
		if (count($groups)==0) trigger_error(_('No groups found in ldap.'), E_USER_WARNING);
		$this->createhomedir=false;
		}

	// Variables
	// Use a unix password?
	var $userPassword_no;
	// Lock account?
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
			$this->attributes['userPassword'][0] = base64_encode($_SESSION['ldap']->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->attributes['userPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION['ldap']->decrypt(base64_decode($this->attributes['userPassword'][0]));
				}
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

	/* This function returns all ldap attributes
	* which are part of posixAccount and returns
	* also their values.
	*/
	function get_attributes() {
		$return = $this->attributes;
		$return['userPassword'] = $this->userPassword();
		return $return;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);

		// get all additional groupmemberships
		$dn_groups = $_SESSION['cache']->get_cache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			if (in_array($attr['uid'][0], $dn_groups[$DN])) {
				$this->groups[] = substr($DN, 3, strpos($DN, ',')-3);
				}
			}
		$this->groups_orig = $this->groups;
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

		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		// Set unix password
		if (count($this->orig['userPassword'])==0) {
			// New user or no old password set
			if ($this->userPassword_no) {
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]));
			}
		else {
			if (($this->attributes['userPassword'][0] != $this->orig['userPassword'][0] && $this->userPassword()!='' ) || $this->userPassword_no) {
				// Write new password
				if ($this->userPassword_no) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]);
					else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixAccount_pwdHash'][0]));
				}
			else { // No new password but old password
				// (un)lock password
				if ($this->userPassword_lock == pwd_is_enabled($this->orig['userPassword'][0])) {
					if ($this->userPassword_lock) {
						$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_disable($this->orig['userPassword'][0]));
					}
					else {
						$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_enable($this->orig['userPassword'][0]));
					}
				}
			}
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
	function proccess_attributes($post, $profile=false) {
		if ($this->orig['uid'][0]!='' && $post['uid']!=$this->attributes['uid'][0])
			$errors['uid'][] = $this->messages['uid'][0];
		if ($this->orig['gidNumber'][0]!='' && $_SESSION['cache']->getgid($post['gidNumber'])!=$this->attributes['gidNumber'][0])
			$errors['gidNumber'][] = $this->dynamic_Message('gidNumber',0);
		if ($this->orig['uidNumber'][0]!='' && $post['uidNumber']!=$this->attributes['uidNumber'][0])
			$errors['uidNumber'][] = $this->dynamic_Message('uidNumber',0);
		if (isset($post['homeDirectory']) && $this->orig['homeDirectory'][0]!='' && $post['homeDirectory']!=$this->attributes['homeDirectory'][0])
			$errors['homeDirectory'][] = $this->dynamic_Message('homeDirectory',0);

		// Load attributes
		$this->attributes['uid'][0] = $post['uid'];
		$this->attributes['cn'][0] = $this->attributes['uid'][0];
		$this->attributes['uidNumber'][0] = $post['uidNumber'];
		$this->attributes['gidNumber'][0] = $_SESSION['cache']->getgid($post['gidNumber']);
		$this->attributes['homeDirectory'][0] = $post['homeDirectory'];
		$this->attributes['loginShell'][0] = $post['loginShell'];
		$this->attributes['gecos'][0] = $post['gecos'];
		if ($post['createhomedir']) $this->createhomedir = true;
			else $this->createhomedir = false;
		if ($post['userPassword_no']) $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($post['userPassword_lock']) $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if (!$profile) {
			if ($post['genpass']) {
					$this->userPassword(genpasswd());
					$post['userPassword2'] = '';
				}
				else {
					if (isset($post['userPassword'])) {
						if ($post['userPassword'] != $post['userPassword2']) {
							$errors['userPassword'][] = $this->messages['userPassword'][0];
						}
						else $this->userPassword($post['userPassword']);
						if (!get_preg($this->userPassword(), 'password'))
							$errors['userPassword'][] = $this->messages['userPassword'][1];
					}
				}
			
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
			foreach ($dn_uids as $uid) $uids[] = $uid[0];
			if(is_array($uids)) sort ($uids, SORT_NUMERIC);
			if ($this->attributes['uidNumber'][0]=='') {
				// No id-number given
				if ($this->orig['uidNumber'][0]=='') {
					// new account -> we have to find a free id-number
					if (count($uids)!=0) {
						// There are some uids
						// Store highest id-number
						$id = $uids[count($uids)-1];
						// Return minimum allowed id-number if all found id-numbers are too low
						if ($id < $minID) $this->attributes['uidNumber'][0] = $minID;
						// Return higesht used id-number + 1 if it's still in valid range
						if ($id < $maxID) $this->attributes['uidNumber'][0] = $id+1;
						/* If this function is still running we have to fid a free id-number between
						* the used id-numbers
						*/
						$i = intval($minID);
						while (in_array($i, $uids)) $i++;
						if ($i>$maxID)
							$errors['uidNumber'][] = $this->messages['uidNumber'][1];
							else {
								$this->attributes['uidNumber'][0] = $i;
								$errors['uidNumber'][] = $this->messages['uidNumber'][2];
								}
						}
					else $this->attributes['uidNumber'][0] = $minID;
					// return minimum allowed id-number if no id-numbers are found
					}
				else $this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
				// old account -> return id-number which has been used
				}
			else {
				// Check manual ID
				// id-number is out of valid range
				if ( ($this->attributes['uidNumber'][0]!=$post['uidNumber']) && ($this->attributes['uidNumber'][0] < $minID || $this->attributes['uidNumber'][0] > $maxID)) $errors['uidNumber'][] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
				// $uids is allways an array but not if no entries were found
				if (is_array($uids)) {
					// id-number is in use and account is a new account
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $errors['uidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					// id-number is in use, account is existing account and id-number is not used by itself
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
						$errors['uidNumber'][] = $this->messages['uidNumber'][3];
						$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
						}
					}
				}
			}
		if ($_SESSION[$this->base]->type=='user') {
			if (($this->attributes['uid'][0] != $post['uid']) &&  !get_preg($post['uid'], '!upper') && !$profile)
				$errors['uid'][] = $this->messages['uid'][1];
			// Check if Homedir is valid
			if (!$profile) {
					$this->attributes['homeDirectory'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
				if ($this->attributes['uid'][0] != '')
					$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
				if ($this->attributes['homeDirectory'][0] != $post['homeDirectory']) $errors['homeDirecotry'][] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
				}
			if ( !get_preg($this->attributes['homeDirectory'][0], 'homeDirectory' ))
				$errors['homeDirecotry'][] = $this->messages['homeDirectory'][0];
			// Check if Username contains only valid characters
			if ( !get_preg($this->attributes['uid'][0], 'username') && !$profile)
				$errors['uid'][] = $this->messages['uid'][2];
			}

		if ($_SESSION[$this->base]->type=='host' && !$profile) {
			if (($this->attributes['uid'][0] != $post['uid']) &&  !get_preg($post['uid'], '!upper'))
				$errors['uid'][] = $this->messages['uid'][3];
			// Check if Hostname contains only valid characters
			if ( !get_preg($this->attributes['uid'][0], 'hostname'))
				$errors['uid'][] = $this->messages['uid'][4];
			}

		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if (!$profile) {
			if ($_SESSION['cache']->in_cache($this->attributes['uid'][0],'uid', '*')!=false && ($this->orig['uid'][0]!='')) {
				$this->attributes['uid'][0] = $this->orig['uid'][0];
				}
			// Change uid to a new uid until a free uid is found
			else while ($_SESSION['cache']->in_cache($this->attributes['uid'][0], 'uid', '*')) {
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
					while (!$mark) {
						if (ereg('^([0-9])+$',substr($this->attributes['uid'][0], $i, strlen($this->attributes['uid'][0])-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($this->attributes['uid'][0], 0, $i+1);
					$lastchars = substr($this->attributes['uid'][0], $i+1, strlen($this->attributes['uid'][0])-$i);
					// Put username together
					if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1)."$";
						else $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1);
					}
				}
		// Show warning if lam has changed username
		if ($_SESSION[$this->base]->type=='user')
			if ($this->attributes['uid'][0] != $post['uid']) {
				$errors['uid'][] = $this->messages['uid'][5];
				}
		if ($_SESSION[$this->base]->type=='host')
			if ($this->attributes['uid'][0] != $post['uid']) {
				$errors['uid'][] = $this->messages['uid'][6];
				}
		if (!get_preg($this->userPassword(), 'password'))
				$errors['userPassword'][] = $this->messages['userPassword'][1];
				}
		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		if ($post['addgroup'])  return 'group';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_group($post, $profile=false) {
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

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
		$shelllist = getshells(); // list of all valid shells

		if (!$profile) {
			if ($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) $password=$this->userPassword();
			else if ($this->attributes['userPassword'][0] != '') $password=$post['userPassword'];
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Username").'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'uid', 'type' => 'text', 'size' => '20', 'maxlength' => '20', 'value' => $this->attributes['uid'][0]),
				2 => array ('kind' => 'help', 'value' => 'uid'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('UID number').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'uidNumber', 'type' => 'text', 'size' => '6', 'maxlength' => '6', 'value' => $this->attributes['uidNumber'][0]),
				2 => array ('kind' => 'help', 'value' => 'uidNumber'));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Gecos') ),
			1 => array ( 'kind' => 'input', 'name' => 'gecos', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['gecos'][0]),
			2 => array ('kind' => 'help', 'value' => 'gecos'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Primary group').'*' ),
			1 => array ( 'kind' => 'select', 'name' => 'gidNumber', 'options' => $groups, 'options_selected' =>
				array ($_SESSION['cache']->getgrnam($this->attributes['gidNumber'][0]))),
			2 => array ('kind' => 'help', 'value' => 'gidNumber'));

		if ($_SESSION[$this->base]->type=='user') {
			if (!$profile) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Additional groups') ),
					1 => array ( 'kind' => 'input', 'name' => 'addgroup', 'type' => 'submit', 'value' => _('Edit groups')),
					2 => array ('kind' => 'help', 'value' => 'addgroup'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home directory').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['homeDirectory'][0]),
				2 => array ('kind' => 'help', 'value' => 'homeDirectory'));
			if (!$profile) {
				if ($this->orig['homeDirectory']=='' && isset($_SESSION['config']->scriptPath)) {
					$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Create home directory') ),
						1 => array ( 'kind' => 'input', 'name' => 'createhomedir', 'type' => 'checkbox', 'checked' => $this->createhomedir),
						2 => array ('kind' => 'help', 'value' => 'createhomedir'));
					}
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password') ),
					1 => array ( 'kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password),
					2 => array ( 'kind' => 'input', 'name' => 'genpass', 'type' => 'submit', 'value' => _('Generate password')));
				if ($post['userPassword2']!='') $password2 = $post['userPassword2'];
				 else $password2 = $password;
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
					1 => array ( 'kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
					2 => array ('kind' => 'help', 'value' => 'userPassword'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Set no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'userPassword_no', 'type' => 'checkbox', 'checked' => $this->userPassword_no),
				2 => array ('kind' => 'help', 'value' => 'userPassword_no'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Lock password') ),
				1 => array ( 'kind' => 'input', 'name' => 'userPassword_lock', 'type' => 'checkbox', 'checked' => $this->userPassword_lock),
				2 => array ('kind' => 'help', 'value' => 'userPassword_lock'));
			if (count($shelllist)!=0)
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Login shell').'*' ),
					1 => array ( 'kind' => 'select', 'name' => 'loginShell', 'options' => $shelllist, 'options_selected' =>
						array ($this->attributes['loginShell'][0])),
					2 => array ('kind' => 'help', 'value' => 'loginShell'));
			}
		return $return;
		}

	function display_html_delete($post) {
		if ($_SESSION[$this->base]->type=='user' && isset($_SESSION['config']->scriptPath)) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Delete home directory') ),
				1 => array ( 'kind' => 'input', 'name' => 'deletehomedir', 'type' => 'checkbox'),
				2 => array ('kind' => 'help', 'value' => 'deletehomedir'));
			}
		return $return;
		}

	function display_html_group($post, $profile=false) {
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
		
		$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Additional groups"), 'value' =>
			array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Selected groups"), 'value' =>
				array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'removegroups[]', 'size' => '15', 'multiple', 'options' => $this->groups)))),
			1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'addgroups_button',
				'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>' )),
				2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'addgroup' )))),
			2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available groups"), 'value' =>
				array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'addgroups[]', 'size' => '15', 'multiple', 'options' => $groups))))
				))));

		$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'back' ),
			1 => array ( 'kind' => 'text'),
			2 => array ('kind' => 'text'));
		return $return;
		}


	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		if ($_SESSION[$this->base]->type=='user') {
			$groups = $_SESSION['cache']->findgroups(); // list of all groupnames
			$shelllist = getshells(); // list of all valid shells
			// primary Unix group
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Primary group') . ": "),
				1 => array('kind' => 'select', 'name' => 'posixAccount_primaryGroup', 'options' => $groups, 'options_selected' => array(), 'size' => 1),
				2 => array('kind' => 'help', 'value' => 'TODO'));
			// additional group memberships
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Additional groups') . ": "),
				1 => array('kind' => 'select', 'name' => 'posixAccount_additionalGroup', 'options' => $groups,
					'options_selected' => array(), 'size' => 10, 'multiple' => true),
				2 => array('kind' => 'help', 'value' => 'TODO'));
			// home directory
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Home directory') . ": "),
				1 => array('kind' => 'input', 'name' => 'posixAccount_homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => '/home/$user'),
				2 => array('kind' => 'help', 'value' => 'TODO'));
			// login shell
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Login shell') . ": "),
				1 => array('kind' => 'select', 'name' => 'posixAccount_loginShell', 'options' => $shelllist, 'options_selected' => array("/bin/bash")),
				2 => array('kind' => 'help', 'value' => 'TODO'));
			// do not set password
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Set no password') . ": "),
				1 => array('kind' => 'input', 'name' => 'posixAccount_userPassword_no', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'TODO'));
			// disable account
			$return[] = array(0 => array('kind' => 'text', 'text' => _('Lock password') . ": "),
				1 => array('kind' => 'input', 'name' => 'posixAccount_userPassword_lock', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'TODO'));
		}
		return $return;
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
							'posixAccount_loginShell' => array('<block><key>' . _('Login Shell') . '</key><value>' . $this->attributes['loginShell'][0] . '</value></block>'),
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

}

?>
