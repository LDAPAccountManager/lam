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


class posixGroup extends baseModule {

	// Variables
	// Use a unix password?
	var $userPassword_no;
	// Lock account?
	var $userPassword_lock;
	// change gids of users and hosts?
	var $changegids;

	/**
	* Creates a new posixGroup object.
	*
	* @param string $scope account type
	*/
	function posixGroup($scope) {
		// load error messages
		$this->load_errorMessages();
		// call parent constructor
		parent::baseModule($scope);
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
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixGroup", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixGroup";
			if (eregi(".*", $rawAccounts[$i][$ids['posixGroup_cn']])) {  // TODO use real regex for group name
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixGroup_cn']];
			}
			if ($rawAccounts[$i][$ids['posixGroup_gid']] == "") {
				// TODO autoGID
				$partialAccounts[$i]['gidNumber'] = 42;
			}
			elseif (eregi(".*", $rawAccounts[$i][$ids['posixGroup_gid']])) {  // TODO use real regex for group name
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixGroup_gid']];
			}
			if ($rawAccounts[$i][$ids['posixGroup_description']] == "") {
				$partialAccounts[$i]['description'] = $partialAccounts[$i]['cn'];
			}
			elseif (eregi(".*", $rawAccounts[$i][$ids['posixGroup_description']])) {  // TODO use real regex for group name
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixGroup_description']];
			}
			if ($rawAccounts[$i][$ids['posixGroup_members']] != "") {
				if (eregi(".*", $rawAccounts[$i][$ids['posixGroup_members']])) {  // TODO use real regex for group name
					$partialAccounts[$i]['memberUid'] = explode(",", $rawAccounts[$i][$ids['posixGroup_members']]);
				}
				else {
					$errors[] = array(); // TODO error message
				}
			}
			if ($rawAccounts[$i][$ids['posixGroup_password']] != "") {
				if (eregi(".*", $rawAccounts[$i][$ids['posixGroup_password']])) {  // TODO use real regex for group name
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixGroup_password']], true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				else {
					$errors[] = array(); // TODO error message
				}
			}
		}
		return $errors;
	}


	function delete_attributes($post) {
		if ($_SESSION['cache']->in_cache($this->attributes['gidNumber'][0], 'gidNumber', 'user'))
		$return[$_SESSION[$this->base]->dn]['errors'][] = array ('ERROR', _('Primary groupmembers'), _('There are still primary members in group.'));
		return $return;
	}


	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		if ($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) $password=$this->userPassword();
		else if ($this->attributes['userPassword'][0] != '') $password=$post['userPassword'];
		if (!$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Groupname").'*' ),
			1 => array ( 'kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '20', 'maxlength' => '20', 'value' => $this->attributes['cn'][0]),
			2 => array ('kind' => 'help', 'value' => 'cn'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('GID number').'*' ),
			1 => array ( 'kind' => 'input', 'name' => 'gidNumber', 'type' => 'text', 'size' => '6', 'maxlength' => '6', 'value' => $this->attributes['gidNumber'][0]),
			2 => array ('kind' => 'help', 'value' => 'gidNumber'));
		}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
		1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['description'][0]),
		2 => array ('kind' => 'help', 'value' => 'description'));
		if (!$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Group members").'*' ),
			1 => array ( 'kind' => 'input', 'name' => 'adduser', 'type' => 'submit', 'value' => _('Edit groups')),
			2 => array ('kind' => 'help', 'value' => 'adduser'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password') ),
			1 => array ( 'kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password),
			2 => array ( 'kind' => 'input', 'name' => 'genpass', 'type' => 'submit', 'value' => _('Generate password')));
			if ($post['userPassword2']!='') $password2 = $post['userPassword2'];
			 else $password2 = $password;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
			1 => array ( 'kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
			2 => array ('kind' => 'help', 'value' => 'userPassword'));
		}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
		1 => array ( 'kind' => 'input', 'name' => 'userPassword_no', 'type' => 'checkbox', 'checked' => $this->userPassword_no),
		2 => array ('kind' => 'help', 'value' => 'userPassword_no'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Lock password') ),
		1 => array ( 'kind' => 'input', 'name' => 'userPassword_lock', 'type' => 'checkbox', 'checked' => $this->userPassword_lock),
		2 => array ('kind' => 'help', 'value' => 'userPassword_lock'));
		if ($this->attributes['gidNumber'][0]!=$this->orig['gidNumber'][0] && $this->orig['gidNumber'][0]!='' && !$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Change GID number of users and hosts') ),
			1 => array ( 'kind' => 'input', 'name' => 'changegids', 'type' => 'checkbox', 'checked' => $this->changegids),
			2 => array ('kind' => 'help', 'value' => 'changegids'));
		}
		return $return;
	}


	function display_html_delete($post) {
		// Get list of primary groupmembers.
		return 0;
	}


	function display_html_user($post, $profile=false) {
		// load list with all groups
		$dn_users = $_SESSION['cache']->get_cache('uid', 'posixAccount', 'user');
		if (is_array($dn_users)) {
			foreach ($dn_users as $user) $users[] = $user[0];
			// sort groups
			sort($users, SORT_STRING);
			// remove groups the user is member of from grouplist
			$users = array_delete($this->attributes['memberUid'], $users);
			// Remove primary group from grouplist
			$users_dn = $_SESSION['cache']->get_cache('gidNumber', 'posixAccount', 'user');
			$DNs = array_keys($users_dn);
			for ($i=0; $i<count($DNs); $i++) {
				if ($users_dn[$DNs[$i]][0]==$this->attributes['gidNumber'][0]) {
					$thisuser = substr($DNs[$i], 4, strpos($DNs[$i], ",")-4);
					if (in_array($thisuser, $users)) {
						$users = @array_flip($users);
						unset($users[$thisuser]);
						$users = @array_flip($users);
					}
				}
			}
			// sort users
			sort($users);
		}
		$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Group members"), 'value' =>
		array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Selected users"), 'value' =>
		array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'removeusers[]', 'size' => '15', 'multiple', 'options' => $this->attributes['memberUid'])))),
		1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'addusers_button',
		'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removeusers_button', 'value' => '=>' )),
		2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'adduser' )))),
		2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available users"), 'value' =>
		array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'addusers[]', 'size' => '15', 'multiple', 'options' => $users))))
		))));

		$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'toattributes' ,'type' => 'submit', 'value' => _('Back') ),
		1 => array ( 'kind' => 'text'),
		2 => array ('kind' => 'text'));
		return $return;
	}


	/** This function returns all ldap attributes
	* which are part of posixGroup and returns
	* also their values.
	*/
	function get_attributes() {
		$return = $this->attributes;
		$return['userPassword'] = $this->userPassword();
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
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array('inetOrgPerson', 'account', 'sambaDomain'));
		// configuration options
		$return['config_options']['group'] = array(
		array(
		0 => array('kind' => 'text', 'text' => '<b>' . _('Minimum GID number') . " *: </b>"),
		1 => array('kind' => 'input', 'name' => 'posixGroup_minGID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
		2 => array('kind' => 'text', 'value' => '&nbsp;'),
		3 => array('kind' => 'text', 'text' => '<b>' . _('Maximum GID number') . " *: </b>"),
		4 => array('kind' => 'input', 'name' => 'posixGroup_maxGID', 'type' => 'text', 'size' => '10', 'maxlength' => '255'),
		5 => array('kind' => 'help', 'value' => 'minMaxGID'))
		);
		$return['config_options']['all'] = array(
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
		'posixGroup_pwdHash' => _("Password hash type"),
		)
		);
		// configuration checks
		$return['config_checks']['group']['posixGroup_minGID'] = array(
			'type' => 'regex',
			'regex' => '^[0-9]+$',
			'required' => true,
			'required_message' => $this->errormessages['gidNumber'][5],
			'error_message' => $this->errormessages['gidNumber'][5]);
		$return['config_checks']['group']['posixGroup_maxGID'] = array(
			'type' => 'regex',
			'regex' => '^[0-9]+$',
			'required' => true,
			'required_message' => $this->errormessages['gidNumber'][6],
			'error_message' => $this->errormessages['gidNumber'][6]);
		$return['config_checks']['group']['cmpGID'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'posixGroup_maxGID',
			'cmp_name2' => 'posixGroup_minGID',
			'error_message' => $this->errormessages['gidNumber'][7]);
		// available PDF fields
		$return['PDF_fields'] = array(	'cn',
		'gidNumber',
		'memberUid',
		'description');
		// upload fields
		$return['upload_columns'] = array(
		array(
		'name' => 'posixGroup_cn',
		'description' => _('Group name'),
		'help' => 'cn',
		'example' => _('adminstrators'),
		'required' => true
		),
		array(
		'name' => 'posixGroup_gid',
		'description' => _('GID number'),
		'help' => 'gidNumber',
		'example' => _('2034'),
		'required' => false
		),
		array(
		'name' => 'posixGroup_description',
		'description' => _('Group description'),
		'help' => 'description',
		'example' => _('Administrators group'),
		'required' => false
		),
		array(
		'name' => 'posixGroup_members',
		'description' => _('Group members'),
		'help' => 'upload_members',
		'example' => _('user01,user02,user03'),
		'required' => false
		),
		array(
		'name' => 'posixGroup_password',
		'description' => _('Group password'),
		'help' => 'password',
		'example' => _('secret'),
		'required' => false
		)
		);
		// help Entries
		$return['help'] = array(
		'cn' =>	array(
			"ext" => "FALSE",
			"Headline" => _("Groupname"),
			"Text" => _("Group name of the group which should be created. Valid characters are: a-z,0-9, .-_. Lam does not allow a number as first character because groupadd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. If groupname is already used groupname will be expanded with a number. The next free number will be used.")),
		'gidNumber'	=> array(
			"ext" => "FALSE",
			"Headline" => _("GID number"),
			"Text" => _("If empty GID number will be generated automaticly depending on your configuration settings.")),
		'description' => array(
			"ext" => "FALSE",
			"Headline" => _("Description"),
			"Text" => _("Group description. If left empty group name will be used.")),
		'members' => array(
			"ext" => "FALSE",
			"Headline" => _("Group members"),
			"Text" => _("Users also being member of the current group."). ' '. _("Can be left empty.")),
		'upload_members' => array(
			"ext" => "FALSE",
			"Headline" => _("Group members"),
			"Text" => _("Users also being member of the current group. Users are separated by semicolons.")),
		'password' => array(
			"ext" => "FALSE",
			"Headline" => _("Group password"),
			"Text" => _("Sets the group password.")),
		'userPassword_no' => array(
			"ext" => "FALSE",
			"Headline" => _("Use no password"),
			"Text" => _("If checked no password will be used.")),
		/*'userPassword_lock'	=>	*/
		'minMaxGID' => array(
			"ext" => "FALSE",
			"Headline" => _("GID number"),
			"Text" => _("These are the minimum and maximum numbers to use for group IDs when creating new group accounts. New group accounts will always get the highest number in use plus one.")),
		'pwdHash' => array(
			"ext" => "FALSE",
			"Headline" => _("Password hash type"),
			"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of an user password. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords."))
		);

		return $return;
	}


	/*
	* (non-PHPDoc)
	* @see baseModule#get_pdfEntries
	*/
	function get_pdfEntries($account_type = "group") {
		return array(	'posixGroup_cn' => array('<block><key>' . _('Groupname') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
		'posixGroup_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
		'posixGroup_memberUid' => array('<block><key>' . _('Group members') . '</key><value>' . $this->attributes['memberUid'][0] . '</value></block>'),
		'posixGroup_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}


	/** This functin will be called when the module will be loaded **/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->changegids=false;
	}


	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);
		return 0;
	}


	/** this functin fills the error message array with messages
	**/
	function load_errorMessages() {
		$this->errormessages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->errormessages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->errormessages['gidNumber'][0] = array('INFO', _('GID number'), _('GID number has changed. Please select checkbox to change GID number of users and hosts.'));
		$this->errormessages['gidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
		$this->errormessages['gidNumber'][3] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
		$this->errormessages['gidNumber'][4] = array('ERROR', _('ID-Number'), _('ID is already in use'));
		$this->errormessages['gidNumber'][5] = array('ERROR', _('Minimum GID number'), _('Minimum GID number is invalid or empty!'));
		$this->errormessages['gidNumber'][6] = array('ERROR', _('Maximum GID number'), _('Maximum GID number is invalid or empty!'));
		$this->errormessages['gidNumber'][7] = array('ERROR', _('Maximum GID number'), _('Maximum GID number must be greater than minimum GID number!'));
		$this->errormessages['cn'][0] = array('WARN', _('Groupname'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->errormessages['cn'][1] = array('WARN', _('Groupname'), _('Groupname in use. Selected next free groupname.'));
		$this->errormessages['cn'][2] = array('ERROR', _('Groupname'), _('Groupname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		} 


	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['cn'][0] == '') return false;
		if ($this->attributes['gidNumber'][0] == '') return false;
		return true;
	}


	function module_ready() {
		return true;
	}


	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'user');
	}


	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['description'][0] = $post['description'];
		if ($post['userPassword_no']) $this->userPassword_no=true;
		else $this->userPassword_no=false;
		if ($post['userPassword_lock']) $this->userPassword_lock=true;
		else $this->userPassword_lock=false;
		If (!$profile) {
			if ($post['changegids']) $this->changegids=true;
			else $this->changegids=false;
			if ($post['genpass']) {
					$this->userPassword(genpasswd());
					$post['userPassword2'] = '';
				}
				else {
					if (isset($post['userPassword'])) {
						if ($post['userPassword'] != $post['userPassword2']) {
							$errors['userPassword'][] = $this->errormessages['userPassword'][0];
						}
						else $this->userPassword($post['userPassword']);
						if (!get_preg($this->userPassword(), 'password'))
							$errors['userPassword'][] = $this->errormessages['userPassword'][1];
					}
				}
			if ($this->attributes['gidNumber'][0]!=$post['gidNumber'] || ($this->errors['gidNumber'][0]='ERROR')) {
				// Check if GID is valid. If none value was entered, the next useable value will be inserted
				// load min and may uidNumber
				$minID = intval($this->moduleSettings['posixGroup_minGID'][0]);
				$maxID = intval($this->moduleSettings['posixGroup_maxGID'][0]);
				$dn_gids = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', '*');
				// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
				if(is_array($dn_gids)) {
					foreach ($dn_gids as $gid) $gids[] = $gid[0];
					sort ($gids, SORT_NUMERIC);
				}
				$this->attributes['gidNumber'][0]=$post['gidNumber'];
					if ($this->attributes['gidNumber'][0]=='') {
					// No id-number given
					if ($this->orig['gidNumber'][0]=='') {
						// new account -> we have to find a free id-number
						if (count($gids)!=0) {
							// There are some uids
							// Store highest id-number
							$id = $gids[count($gids)-1];
							// Return minimum allowed id-number if all found id-numbers are too low
							if ($id < $minID) $this->attributes['gidNumber'][0] = $minID;
							// Return higesht used id-number + 1 if it's still in valid range
							if ($id < $maxID) $this->attributes['gidNumber'][0] = $id+1;
							/* If this function is still running we have to fid a free id-number between
							* the used id-numbers
							*/
							$i = intval($minID);
							while (in_array($i, $gids)) $i++;
							if ($i>$maxID)
							$errors['gidNumber'][] = $this->errormessages['gidNumber'][3];
							else {
								$this->attributes['gidNumber'][0] = $i;
								$errors['gidNumber'][] = $this->errormessages['gidNumber'][2];
							}
						}
						else $this->attributes['gidNumber'][0] = $minID;
						// return minimum allowed id-number if no id-numbers are found
					}
					else $this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
					// old account -> return id-number which has been used
				}
				else {
					// Check manual ID
					// id-number is out of valid range
					if ( ($this->attributes['gidNumber'][0]!=$post['gidNumber']) && ($this->attributes['gidNumber'][0] < $minID || $this->attributes['gidNumber'][0] > $maxID)) $errors['gidNumber'][] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
					// $uids is allways an array but not if no entries were found
					if (is_array($gids)) {
						// id-number is in use and account is a new account
						if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]=='') $errors['gidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
						// id-number is in use, account is existing account and id-number is not used by itself
						if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]!='' && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0]) ) {
							$errors['gidNumber'][] = $this->errormessages['gidNumber'][4];
							$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
						}
					}
				}
			}
			if ($this->attributes['cn'][0]!=$post['cn'] || ($this->errors['cn'][0]='ERROR')) {
				$this->attributes['cn'][0] = $post['cn'];
				if (($this->attributes['cn'][0] != $post['cn']) &&  ereg('[A-Z]$', $post['cn']))
				$errors['cn'][] = $this->errormessages['cn'][0];
				// Check if Groupname contains only valid characters
				if ( !get_preg($this->attributes['cn'][0],'groupname'))
				$errors['cn'][] = $this->errormessages['cn'][2];
				// Create automatic useraccount with number if original user already exists
				// Reset name to original name if new name is in use
				// Set username back to original name if new username is in use
				if ($_SESSION['cache']->in_cache($this->attributes['cn'][0],'cn', '*')!=false && ($this->orig['cn'][0]!='')) {
					$this->attributes['cn'][0] = $this->orig['cn'][0];
				}
				// Change gid to a new gid until a free gid is found
				else while ($_SESSION['cache']->in_cache($this->attributes['cn'][0], 'cn', '*')) {
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
					$errors['cn'][] = $this->errormessages['cn'][0];
				}
			// show info when gidnumber has changed
			if (($this->orig['gidNumber'][0]!=$this->attributes['gidNumber'][0]) && $this->orig['gidNumber'][0]!='' && $post['gidNumber']!=$this->attributes['gidNumber'][0])
			$errors['gidNumber'][] = $this->errormessages['gidNumber'][0];
			}
		}
		// Return error-messages
		if (count($errors)!=0) {
			$this->errors = $errors;
			return $errors;
		}
		else $this->errors = array();
		// Go to additional group page when no error did ocour and button was pressed
		if ($post['adduser'])  return 'user';
		return 0;
	}


	/* Write variables into object and do some regexp checks
	*/
	function proccess_user($post, $profile=false) {
		do { // X-Or, only one if() can be true
		if (isset($post['addusers']) && isset($post['addusers_button'])) { // Add groups to list
		// Add new user
		$this->attributes['memberUid'] = @array_merge($this->attributes['memberUid'], $post['addusers']);
		// remove doubles
		$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
		array_unique($this->attributes['memberUid']);
		$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
		// sort groups
		sort($this->attributes['memberUid']);
		break;
		}
		if (isset($post['removeusers']) && isset($post['removeusers_button'])) { // remove groups from list
		$this->attributes['memberUid'] = array_delete($post['removeusers'], $this->attributes['memberUid']);
		break;
		}
		} while(0);
		if (isset($post['adduser_button']) || isset($post['removeuser_button'])) return 'user';
		if ($post['toattributes']) return 'attributes';
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
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]);
			}
			else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]));
		}
		else {
			if (($this->attributes['userPassword'][0] != $this->orig['userPassword'][0] && $this->userPassword()!='' ) || $this->userPassword_no) {
				// Write new password
				if ($this->userPassword_no) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash('', !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]);
				else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash($this->userPassword(), !$this->userPassword_lock, $this->moduleSettings['posixGroup_pwdHash'][0]));
			}
			else { // No new password but old password
			// (un)lock password
			if ($this->userPassword_lock == pwd_is_enabled($this->orig['userPassword'][0])) {
				// Split old password hash in {CRYPT} and password-hash
				$i = 0;
				while ($this->orig['userPassword'][0]{$i} != '}') $i++;
				$passwd = substr($this->orig['userPassword'][0], $i+1 );
				$crypt = substr($this->orig['userPassword'][0], 0, $i+1 );
				// remove trailing ! from password hash
				if ($passwd{0} == '!') $passwd = substr($passwd, 1);
				// Write new password
				if ($this->userPassword_lock) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode("$crypt!$passwd");
				else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode("$crypt$passwd");
			}
			}
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
				$result = $_SESSION['cache']->get_cache('gidNumber', 'posixAccount', '*');
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
				$result = $_SESSION['cache']->get_cache('primaryGroupID', 'sambaAccount', '*');
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
				$result = $_SESSION['cache']->get_cache('sambaPrimaryGroupSID', 'sambaSamAccount', '*');
				if (is_array($result)) {
					$DNs = array_keys($result);
					for ($i=0; $i<count($DNs); $i++) {
						// Get Domain SID from name
						$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
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


	/** $attribute['userPassword'] can't accessed directly because it's enrcypted
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
}

?>
