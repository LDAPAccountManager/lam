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

class sambaSamAccount extends baseModule {

	/**
	* Creates a new sambaSamAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function sambaSamAccount($scope) {
	// call parent constructor
	parent::baseModule($scope);
	}

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['homePath'][0] = array('ERROR', _('Home path'), _('Home path is invalid.'));
		$this->messages['homePath'][1] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
		$this->messages['logonScript'][0] = array('ERROR', _('Script path'), _('Script path is invalid!'));
		$this->messages['logonScript'][1] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'));
		$this->messages['workstations'][0] = array('ERROR', _('Samba workstations'), _('Please enter a comma separated list of host names!'));
		$this->messages['sambaLMPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['sambaLMPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['rid'][0] = array('ERROR', _('Special user'), _('There can be only one administrator per domain.'));
		$this->messages['rid'][1] = array('ERROR', _('Special user'), _('There can be only one guest per domain.'));
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
		if ($this->get_scope() == "host") {
			// this is a base module
			$return["is_base"] = true;
			// LDAP filter
			$return["ldap_filter"] = array('and' => '(uid=*$)', 'or' => "(objectClass=posixAccount)");
		}
		// alias name
		$return["alias"] = _('Samba 3');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// profile checks
		$return['profile_checks']['sambaSamAccount_smbhome'] = array(
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['homePath'][0]);
		$return['profile_checks']['sambaSamAccount_profilePath'] = array(
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['profilePath'][0]);
		$return['profile_checks']['sambaSamAccount_logonScript'] = array(
			'type' => 'ext_preg',
			'regex' => 'logonscript',
			'error_message' => $this->messages['logonScript'][0]);
		$return['profile_checks']['sambaSamAccount_userWorkstations'] = array(
			'type' => 'ext_preg',
			'regex' => 'unixhost',
			'error_message' => $this->messages['workstations'][0]);
		// available PDF fields
		$return['PDF_fields'] = array(
			'displayName',
			'uid',
			'sambaHomePath',
			'sambaHomeDrive',
			'sambaLogonScript',
			'sambaProfilePath',
			'sambaUserWorkstations',
			'sambaDomainName',
			'description',
			'sambaPrimaryGroupSID'
		);
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		$this->useunixpwd=false;
		// List of well known rids
		$this->rids = array ( _('Domain Admins') => 512, _('Domain Users') => 513, _('Domain Guests') => 514, _('Domain Computers') => 515, _('Domain Controllers') => 516,
			_('Domain Certificate Admins') => 517, _('Domain Schema Admins') => 518, _('Domain Enterprise Admins') => 519, _('Domain Policy Admins') => 520 );
		}

	// Variables
	// use unix password as samba password?
	var $useunixpwd;
	// Array of well known rids
	var $rids;


	/* $attribute['sambaLMPassword'] and $attribute['sambaNTPassword'] can't accessed directly because it's enrcypted
	* To read / write password function sambaLMPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function sambaLMPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			if ($newpassword=='')
				$this->attributes['sambaLMPassword'][0] = '';
			else $this->attributes['sambaLMPassword'][0] = base64_encode($_SESSION['ldap']->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->useunixpwd) return $_SESSION[$this->base]->module['posixAccount']->userPassword();
			if ($this->attributes['sambaLMPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION['ldap']->decrypt(base64_decode($this->attributes['sambaLMPassword'][0]));
				}
			else return '';
			}
		}

	function module_ready() {
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]=='') return false;
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]=='') return false;
		if ($this->attributes['uid'][0]=='') return false;
		return true;
		}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['sambaSID'][0] == '') return false;
		return true;
		}
	
	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'sambaUserWorkstations');
		}

	/*
	*/
	function get_help($id,$scope) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host description") . ".");
				break;
			}
		return false;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);
		// Delete password. We don't want to show an encrypted password because it makes no sense
		$this->sambaLMPassword('');
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
		/* Create sambaSID. Can't create it while loading attributes because
		* it's psssible uidNumber has changed
		*/
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomain[$i]->RIDbase;
				}
		$special = false;
		if ($this->attributes['sambaSID'][0] == $SID."-500") $special = true;
		if ($this->attributes['sambaSID'][0] == $SID."-501") $special = true;
		if (!$special) $this->attributes['sambaSID'][0] == $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2+$RIDbase);
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++)
			if ($this->attributes['sambaPrimaryGroupSID'][0] == $SID . "-" . $rids[$i])
				$wrid = true;
		if (!$wrid) $this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+$RIDbase+1;


		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);

		// unset password when needed
		if (isset($return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaNTPassword']);
		if (!isset($this->orig['sambaLMPassword'][0])) {
			// new account
			$return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword'][0] = lmPassword($this->sambaLMPassword());
			$return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword'][0] = ntPassword($this->sambaLMPassword());
			$return[$_SESSION[$this->base]->dn]['add']['sambaPwdLastSet'][0] = time();
			}
		else
			// edit existing account
			if ($this->sambaLMPassword()!='' && $this->attributes['sambaLMPassword'][0] != $this->orig['sambaLMPassword'][0]) {
				$return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword'][0] = lmPassword($this->sambaLMPassword());
				$return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword'][0] = ntPassword($this->sambaLMPassword());
				$return[$_SESSION[$this->base]->dn]['modify']['sambaPwdLastSet'][0] = time();
			}
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Save attributes
		$this->attributes['sambaDomainName'][0] = $post['sambaDomainName'];
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
				}
		$flag = "[";
		if ($post['sambaAcctFlagsD']) $flag .= "D";
		if ($post['sambaAcctFlagsX']) $flag .= "X";
		if ($post['sambaAcctFlagsN']) $flag .= "N";
		if ($post['sambaAcctFlagsS']) $flag .= "S";
		if ($post['sambaAcctFlagsH']) $flag .= "H";
		if ($post['sambaAcctFlagsW']) $flag .= "W";
		if ($post['sambaAcctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['sambaAcctFlags'][0] = $flag;

		if ($_SESSION[$this->base]->type=='host') {
			$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[_('Domain Computers')];
			if ($post['ResetSambaPassword']) {
				$this->sambaLMPassword($_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0]);
				$_SESSION[$this->base]->module['posixAccount']->userPassword('');
				}
			}

		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['sambaPwdCanChange'][0] = mktime($post['sambaPwdCanChange_h'], $post['sambaPwdCanChange_m'], $post['sambaPwdCanChange_s'],
				$post['sambaPwdCanChange_mon'], $post['sambaPwdCanChange_day'], $post['sambaPwdCanChange_yea']);
			$this->attributes['sambaPwdMustChange'][0] = mktime($post['sambaPwdMustChange_h'], $post['sambaPwdMustChange_m'], $post['sambaPwdMustChange_s'],
				$post['sambaPwdMustChange_mon'], $post['sambaPwdMustChange_day'], $post['sambaPwdMustChange_yea']);
			$this->attributes['sambaHomePath'][0] = stripslashes($post['sambaHomePath']);
			$this->attributes['sambaHomeDrive'][0] = $post['sambaHomeDrive'];
			$this->attributes['sambaLogonScript'][0] = stripslashes($post['sambaLogonScript']);
			$this->attributes['sambaProfilePath'][0] = stripslashes($post['sambaProfilePath']);
			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($post['sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[$rids[$i]];
					}
				}
			if (!$wrid) $this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+$RIDbase+1;

			if ($post['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;
			if (isset($post['sambaLMPassword'])) {
				if ($post['sambaLMPassword'] != $post['sambaLMPassword2']) {
					$triggered_messages['sambaLMPassword'][] = $this->messages['sambaLMPassword'][0];
					unset ($post['sambaLMPassword2']);
					}
				else {
						if ( !get_preg($this->sambaLMPassword(), 'password')) $triggered_messages['sambaLMPassword'][] = $this->messages['sambaLMPassword'][1];
							else $this->sambaLMPassword($post['sambaLMPassword']);
					}
			}
			if ($post['sambaSID']== _('Administrator')) {
				$this->attributes['sambaSID'][0] = $SID."-500";
				// Do a check if an administrator already exists
				if ($_SESSION['cache']->in_cache($SID."-500", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
					$triggered_messages['sambaSID'][] = $this->messages['rid'][0];
				}
			else if ($post['sambaSID']== _('Guest')) {
				$this->attributes['sambaSID'][0] = $SID."-501";
				// Do a check if an guest already exists
				if ($_SESSION['cache']->in_cache($SID."-501", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
					$triggered_messages['sambaSID'][] = $this->messages['rid'][1];
				}
			else if ($post['sambaSID']== "-") {
				$this->attributes['sambaSID'][0] = $SID."-". (($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2)+$RIDbase);
			}
			// Check values
			$this->attributes['sambaHomePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaHomePath'][0]);
			$this->attributes['sambaHomePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaHomePath'][0]);
			if ($this->attributes['sambaHomePath'][0] != stripslashes($post['sambaHomePath'])) $triggered_messages['sambaHomePath'][] = $this->messages['homePath'][1];
			$this->attributes['sambaLogonScript'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaLogonScript'][0]);
			$this->attributes['sambaLogonScript'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaLogonScript'][0]);
			if ($this->attributes['sambaLogonScript'][0] != stripslashes($post['sambaLogonScript'])) $triggered_messages['sambaLogonScript'][] = $this->messages['logonScript'][1];
			$this->attributes['sambaProfilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaProfilePath'][0]);
			$this->attributes['sambaProfilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaProfilePath'][0]);
			if ($this->attributes['sambaProfilePath'][0] != stripslashes($post['sambaProfilePath'])) $triggered_messages['sambaProfilePath'][] = $this->messages['profilePath'][1];
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC')))
				$triggered_messages['sambaHomePath'][] = $this->messages['homePath'][0];
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript')))
				$triggered_messages['sambaScriptPath'][] = $this->messages['logonScript'][0];
			if (!($this->attributes['sambaProfilePath'][0] == '') &&
				!(get_preg($this->attributes['sambaProfilePath'][0], 'UNC') xor get_preg($this->attributes['sambaProfilePath'][0], 'homeDirectory')))
					$triggered_messages['sambaProfilePath'][] = $this->messages['profilePath'][0];
			}
		else {
			$sambaHomePath = str_replace('$user', 'user', $this->attributes['sambaHomePath'][0]);
			$sambaHomePath = str_replace('$group', 'group', $sambaHomePath);
			$sambaLogonScript = str_replace('$user', 'user', $this->attributes['sambaLogonScript'][0]);
			$sambaLogonScript = str_replace('$group', 'group', $sambaLogonScript);
			$sambaProfilePath = str_replace('$user', 'user', $this->attributes['sambaProfilePath'][0]);
			$sambaProfilePath = str_replace('$group', 'group', $sambaProfilePath);
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC')))
					$triggered_messages[] = $this->messages['homePath'][0];
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript')))
				$triggered_messages[] = $this->messages['logonScript'][0];
			if ( (!$this->attributes['sambaProfilePath'][0]=='') && (!get_preg($this->attributes['sambaProfilePath'][0], 'UNC'))
				&& (!get_preg($this->attributes['sambaProfilePath'][0], 'UNC')))
					$triggered_messages[] = $this->messages['profilePath'][0];
			}
		if (count($triggered_messages)!=0) {
			$this->triggered_messages = $triggered_messages;
			return $triggered_messages;
		}
		else $this->triggered_messages = array();
		if ($post['sambaUserWorkstations'])  return 'sambaUserWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_sambaUserWorkstations($post) {
		// Load attributes
		if ($_SESSION[$this->base]->type=='user') {
			do { // X-Or, only one if() can be true
				if (isset($post['availableSambaUserWorkstations']) && isset($post['sambaUserWorkstations_add'])) { // Add workstations to list
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Add new // Add workstations
					$workstations = array_merge($workstations, $post['availableSambaUserWorkstations']);
					// remove doubles
					$workstations = array_flip($workstations);
					array_unique($workstations);
					$workstations = array_flip($workstations);
					// sort workstations
					sort($workstations);
					// Recreate workstation string
					$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
						}
					break;
					}
				if (isset($post['sambaUserWorkstations']) && isset($post['sambaUserWorkstations_remove'])) { // remove // Add workstations from list
					// Put all workstations in array
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Remove unwanted workstations from array
					$workstations = array_delete($post['sambaUserWorkstations'], $workstations);
					// Recreate workstation string
					$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
						}
					break;
					}
				} while(0);
			if ($post['attributes']) return 'attributes';
			}
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			$sambaDomainNames[] = $sambaDomains[$i]->name;
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name)
				$SID = $sambaDomains[$i]->SID;
			}
		$canchangedate = getdate($this->attributes['sambaPwdCanChange'][0]);
		$mustchangedate = getdate($this->attributes['sambaPwdMustChange'][0]);

		if ($_SESSION[$this->base]->type=='user') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_h', 'type' => 'hidden', 'value' => $canchangedate['hours']),
					1 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_m', 'type' => 'hidden', 'value' => $canchangedate['minutes']),
					2 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_s', 'type' => 'hidden', 'value' => $canchangedate['seconds']),
					3 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_h', 'type' => 'hidden', 'value' => $mustchangedate['hours']),
					4 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_m', 'type' => 'hidden', 'value' => $mustchangedate['minutes']),
					5 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_s', 'type' => 'hidden', 'value' => $mustchangedate['seconds']),
					6 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsU', 'type' => 'hidden', 'value' => 'true'));
			if ($post['sambaLMPassword2']!='') $password2 = $post['sambaLMPassword2'];
			else $password2 = $this->sambaLMPassword();
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $this->sambaLMPassword()));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
				2 => array ('kind' => 'help', 'value' => 'sambaLMPassword'));
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd),
					2 => array ('kind' => 'help', 'value' => 'useunixpwd'));
				}
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "N")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsN'));
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "X")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsX'));
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "D")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsD'));
			for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User can change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_day',
						'options' => $mday, 'options_selected' => array($canchangedate['mday'])),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_mon',
						'options' => $mon, 'options_selected' => array($canchangedate['mon'])),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_yea',
						'options' => $year, 'options_selected' => array($canchangedate['year']))))),
				2 => array ( 'kind' => 'help', 'value' => 'sambaPwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_day',
						'options' => $mday, 'options_selected' => array($mustchangedate['mday'])),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_mon',
						'options' => $mon, 'options_selected' => array($mustchangedate['mon'])),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_yea',
						'options' => $year, 'options_selected' => array($mustchangedate['year']))))),
				2 => array ( 'kind' => 'help', 'value' => 'sambaPwdMustChange' ));
			for ($i=90; $i>67; $i--) $drives[] = chr($i).':';
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home drive') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaHomeDrive', 'options' => $drives, 'options_selected' => array ($this->attributes['sambaHomeDrive'][0])),
				2 => array ( 'kind' => 'help', 'value' => 'sambaHomeDrive' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaHomePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaHomePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaHomePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Profile path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaProfilePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaProfilePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaProfilePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Logon script') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaLogonScript', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaLogonScript'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaLogonScript' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba workstations') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations', 'value' => _('Edit workstations')),
				2 => array ( 'kind' => 'help', 'value' => 'sambaUserWorkstations' ));

			$names = array_keys($this->rids);
			$wrid=false;
			for ($i=0; $i<count($names); $i++) {
				if ($this->attributes['sambaPrimaryGroupSID'][0]==$SID."-".$this->rids[$names[$i]]) {
					$selected[] = $names[$i];
					$wrid=true;
					}
				else $options[] = $names[$i];
				}
			if ($wrid) $options[] = $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
				else $selected[] = $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaPrimaryGroupSID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'sambaPrimaryGroupSID' ));
			// Display if group SID should be mapped to a well kown SID
			$wrid=false;
			$options = array();
			$selected = array();
			if ($this->attributes['sambaSID'][0]==$SID."-500") {
				$selected[] = _('Administrator');
				$wrid=true;
				}
			else $options[] = _('Administrator');
			if ($this->attributes['sambaSID'][0]==$SIM."-501") {
				$selected[] = _('Guest');
				$wrid=true;
				}
			else $options[] = _('Guest');
			if ($wrid) $options[] = "-";
				else $selected[] = "-";
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaSID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'sambaSID' ));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array($this->attributes['sambaDomainName'][0])),
			2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
		if ($_SESSION[$this->base]->type=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword'),
				2 => array ( 'kind' => 'help', 'value' => 'ResetSambaPassword' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => $this->attributes['sambaDomainName'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
			}
		return $return;
		}

	function display_html_delete($post) {
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_sambaUserWorkstations($post) {
		if ($_SESSION[$this->base]->type=='user') {
			// Get list of all hosts.
			$result = $_SESSION['cache']->get_cache('uid', 'sambaSamAccount', 'host');
			if (is_array($result)) {
				foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
				sort($availableUserWorkstations, SORT_STRING);
				$result = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$userWorkstations = explode (',', $result);
				$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);
				}

			$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Allowed workstations"), 'value' =>
				array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Allowed workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaUserWorkstations[]', 'size' => '15', 'multiple', 'options' => $userWorkstations)))),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_add',
					'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_remove', 'value' => '=>' )),
					2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'sambaUserWorkstations' )))),
				2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'availableSambaUserWorkstations[]', 'size' => '15', 'multiple', 'options' => $availableUserWorkstations))))
					))));

			$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'value' => _('Back') ),
				1 => array ( 'kind' => 'text'),
				2 => array ('kind' => 'text'));
			}
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
			// use Unix password as Samba password
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use unix password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_useunixpwd', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// use no password at all
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use no password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// password never expires
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Password does not expire') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// account deactivation
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Account is deactivated') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// letter of home drive
			$drives = array();
			for ($i = 90; $i > 67; $i--) $drives[] = chr($i) . ':';
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home drive') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaHomeDrive', 'options' => $drives, 'options_selected' => array('Z:')),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// path to home directory
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_smbhome', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// profile path
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Profile path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_profilePath', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// logon script
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Logon script') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_logonScript', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// allowed workstations
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Samba workstations') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_userWorkstations', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
			// domains
			$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array()),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
		}
		elseif ($_SESSION[$this->base]->type=='host') {
			// domains
			$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array()),
				2 => array('kind' => 'help', 'value' => 'TODO')
				);
		}
		return $return;
	}
	
	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array(	'sambaSamAccount_displayName' => array('<block><key>' . _('Display name') . '</key><value' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaSamAccount_uid' => array('<block><key>' . _('Username') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomePath' => array('<block><key>' . _('Home path') . '</key><value>' . $this->attributes['sambaHomeDrive'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomeDrive' => array('<block><key>' . _('Home drive') . '</key><value>' . $this->attributes['sambaHomePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaLogonScript' => array('<block><key>' . _('Logon script') . '</key><value>' . $this->attributes['sambaLogonScript'][0] . '</value></block>'),
							'sambaSamAccount_sambaProfilePath' => array('<block><key>' . _('Profile path') . '</key><value>' . $this->attributes['sambaProfilePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaUserWorkstations' => array('<block><key>' . _('Samba workstations') . '</key><value>' . $this->attributes['sambaUserWorkstations'][0] . '</value></block>'),
							'sambaSamAccount_sambaDomainName' => array('<block><key>' . _('Domain') . '</key><value>' . $this->attributes['sambaDomainName'][0] . '</value></block>'),
							'sambaSamAccount_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
							'sambaSamAccount_sambaPrimaryGroupSID' => array('<block><key>' . _('Windows group') . '</key><value>' . $this->attributes['sambaPrimaryGroupSID'][0] . '</value></block>'));
	}

}

?>
