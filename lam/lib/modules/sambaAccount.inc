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

/* Session variables which are used:
* $_SESSION['cacheAttributes']: This variable contains a list of attributes and their scope which should be cached
*
* Coockie variables which are used:
* $_COOKIE["IV"], $_COOKIE["Key"]: Needed to en/decrypt passwords.
*
* Variables in basearray which are no objects:
* type: Type of account. Can be user, group, host
* attributes: List of all attributes, how to get them and are theiy required or optional
* dn: current DN without uid= or cn=
* dn_orig: old DN if account was loaded with uid= or cn=

* External functions which are used
* account.inc: findgroups, incache, get_cache, array_delete, getshells
* ldap.inc: pwd_is_enabled, pwd_hash
*/

/* This class contains all sambaAccount LDAP attributes
* and funtioncs required to deal with sambaAccount
* sambaAccount can only be created when it should be added
* to an array.
* basearray is the same array sambaAccount should be added
* to. If basearray is not given the constructor tries to
* create an array with sambaAccount and all other required
* objects.
* Example: $user[] = new sambaAccount($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class sambaAccount {
	// Constructor
	function sambaAccount($base) {
		/* Return an error if sambaAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object in an accountContainer object first.'), E_USER_ERROR);
		$this->base = $base;
		// sambaAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() == 'host')) trigger_error(_('sambaAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['posixAccount'])) $_SESSION[$this->base]->modules['posixAccount'] = new posixAccount($this->base);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('sambaAccount');
		$_SESSION[$this->base]->add_attributes ('sambaAccount');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'sambaAccount';
		$this->useunixpwd=false;
		// List of well known rids
		$this->rids = array ( _('Domain Admins') => 512, _('Domain Users') => 513, _('Domain Guests') => 514, _('Domain Computers') => 515, _('Domain Controllers') => 516,
			_('Domain Certificate Admins') => 517, _('Domain Schema Admins') => 518, _('Domain Enterprise Admins') => 519, _('Domain Policy Admins') => 520 );
		}

	// Variables
	// name of accountContainer so we can read other classes in accuontArray
	var $base;
	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;
	// use unix password as samba password?
	var $useunixpwd;
	// Array of well known rids
	var $rids;

	/* $attribute['lmPassword'] and ntPassword can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function lmPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			$this->attributes['lmPassword'][0] = base64_encode($_SESSION[$_SESSION[$this->base]->ldap]->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->useunixpwd) return $_SESSION[$this->base]->module['posixAccount']->userPassword();
			if ($this->attributes['lmPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION[$_SESSION[$this->base]->ldap]->decrypt(base64_decode($this->attributes['lmPassword'][0]));
				}
			else return '';
			}
		}

	function get_alias($scope) {
		return _('sambaAccount');
		}

	function can_manage($scope) {
		if ($scope == "host") return true;
		elseif ($scope == "user") return true;
		else return false;
	}

	function is_base_module($scope) {
		return false;
	}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		if ($scope=='host') return array('require' => array('account'), 'conflict' => array() );
		if ($scope=='user') return array('require' => array('inetOrgPerson'), 'conflict' => array() );
		return -1;
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
		if ($this->attributes['rid'][0] == '') return false;
		return true;
		}	

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'userWorkstations');
		}

	/*
	*/
	function get_help($id) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host Description."));
				break;
			}
		return false;
		}

	/* This function returns all ldap attributes
	* which are part of sambaAccount and returns
	* also their values.
	*/
	function get_attributes() {
		$return['lmPassword'] = $this->lmPassword();
		return $this->attributes;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load attributes which are displayed
		// unset count entries
		unset ($attr['count']);
		$attributes = array_keys($attr);
		foreach ($attributes as $attribute) unset ($attr[$attribute]['count']);
		// unset double entries
		for ($i=0; $i<count($attr); $i++)
			if (isset($attr[$i])) unset($attr[$i]);
		foreach ($attributes as $attribute) {
			if (isset($this->attributes[$attribute])) {
				// decode as unicode
				$this->attributes[$attribute] = $attr[$attribute];
				for ($i=0; $i<count($this->attributes[$attribute]); $i++) {
					$this->attributes[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					$this->orig[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					}
				}
			}
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->attributes['objectClass'][0] = 'sambaAccount';
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
		$special = false;
		if ($this->attributes['rid'][0] == "500") $special = true;
		if ($this->attributes['rid'][0] == "501") $special = true;
		if ($this->attributes['rid'][0] == "515") $special = true;
		if (!$special) $this->attributes['rid'][0] == $_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2+1000;
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++)
			if ($this->attributes['primaryGroupID'][0] == $rids[$i])
				$wrid = true;
		if (!$wrid) $this->attributes['primaryGroupID'][0] = ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+1001;



		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// Set password
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['lmPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['lmPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['ntPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['ntPassword']);
		if (!isset($this->orig['lmPassword'][0])) {
			$return[$_SESSION[$this->base]->dn]['modify']['lmPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($this->lmPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['ntPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($this->lmPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['pwdLastSet'][0] = time();
			}
		if ($this->lmPassword()!='') {
			$return[$_SESSION[$this->base]->dn]['modify']['lmPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($this->lmPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['ntPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($this->lmPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['pwdLastSet'][0] = time();
			}
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		$this->attributes['domain'][0] = $post['domain'];
		// Start character
		$flag = "[";
		if ($post['acctFlagsD']) $flag .= "D";
		if ($post['acctFlagsX']) $flag .= "X";
		if ($post['acctFlagsN']) $flag .= "N";
		if ($post['acctFlagsS']) $flag .= "S";
		if ($post['acctFlagsH']) $flag .= "H";
		if ($post['acctFlagsW']) $flag .= "W";
		if ($post['acctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['acctFlags'][0] = $flag;

		if ($_SESSION[$this->base]->type=='host' && !$profile) {
			$this->attributes['primaryGroupID'][0] = $this->rids[_('Domain Computers')];
			if ($post['ResetSambaPassword']) {
				// *** fixme. What is the default password?
				$this->lmPassword('');
				$_SESSION[$this->base]->module['posixAccount']->userPassword('');
				}
			}

		// Check values
		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['pwdCanChange'][0] = mktime($post['pwdCanChange_h'], $post['pwdCanChange_m'], $post['pwdCanChange_s'],
				$post['pwdCanChange_mon'], $post['pwdCanChange_day'], $post['pwdCanChange_yea']);
			$this->attributes['pwdMustChange'][0] = mktime($post['pwdMustChange_h'], $post['pwdMustChange_m'], $post['pwdMustChange_s'],
				$post['pwdMustChange_mon'], $post['pwdMustChange_day'], $post['pwdMustChange_yea']);
			$this->attributes['smbHome'][0] = stripslashes($post['smbHome']);
			$this->attributes['homeDrive'][0] = $post['homeDrive'];
			$this->attributes['scriptPath'][0] = stripslashes($post['scriptPath']);
			$this->attributes['profilePath'][0] = stripslashes($post['profilePath']);
			if (!$profile) {
				$rids = array_keys($this->rids);
				$wrid = false;
				for ($i=0; $i<count($rids); $i++) {
					if ($post['primaryGroupID'] == $rids[$i]) {
						$wrid = true;
						$this->attributes['primaryGroupID'][0] = $this->rids[$rids[$i]];
						}
					}
				if (!$wrid) $this->attributes['primaryGroupID'][0] = ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+1001;

				if (isset($post['lmPassword'])) {
					if ($post['lmPassword'] != $post['lmPassword2']) {
						$errors['lmPassword'][] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
						unset ($post['lmPassword2']);
						}
						else $this->lmPassword($post['lmPassword']);
					}
				if ($post['rid']== _('Administrator')) {
					$this->attributes['rid'][0] = "500";
					// Do a check if an administrator already exists
					if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache("500", 'rid', 'user')!=$_SESSION[$this->base]->dn_orig)
						$errors['rid'][] = array('ERROR', _('Special user'), _('There can be only one administrator per domain.'));
					}
				if ($post['rid']== _('Guest')) {
					$this->attributes['rid'][0] = "501";
					// Do a check if an administrator already exists
					if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache("501", 'rid', 'user')!=$_SESSION[$this->base]->dn_orig)
						$errors['rid'][] = array('ERROR', _('Special user'), _('There can be only one guest per domain.'));
					}
				$this->attributes['smbHome'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['smbHome'][0]);
				$this->attributes['smbHome'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['smbHome'][0]);
				if ($this->attributes['smbHome'][0] != stripslashes($post['smbHome'])) $errors['smbHome'][] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
				$this->attributes['scriptPath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['scriptPath'][0]);
				$this->attributes['scriptPath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['scriptPath'][0]);
				if ($this->attributes['scriptPath'][0] != stripslashes($post['scriptPath'])) $errors['scriptPath'][] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'));
				$this->attributes['profilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['profilePath'][0]);
				$this->attributes['profilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['profilePath'][0]);
				if ($this->attributes['profiletPath'][0] != stripslashes($post['profilePath'])) $errors['profilePath'][] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
				if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
					$this->lmPassword())) $errors['lmPassword'][] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
				if ( (!$this->attributes['smbHome'][0]=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+)+$', $this->attributes['smbHome'][0])))
						$errors['smbHome'][] = array('ERROR', _('Home path'), _('Home path is invalid.'));
				if ( (!$this->attributes['scriptPath'][0]=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*'.
					'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*)*(([.][b][a][t])|([.][c][m][d]))$', $this->attributes['scriptPath'][0])))
					$errors['scriptPath'][] = array('ERROR', _('Script path'), _('Script path is invalid!'));
				if ( (!$this->attributes['profilePath'][0]=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $this->attributes['profilePath'][0]))
					&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $this->attributes['profilePath'][0])))
						$errors['profilePath'][] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
				}
			else {
				$smbHome = str_replace('$user', 'user', $this->attributes['smbHome'][0]);
				$smbHome = str_replace('$group', 'group', $smbHome);
				$scriptPath = str_replace('$user', 'user', $this->attributes['scriptPath'][0]);
				$scriptPath = str_replace('$group', 'group', $scriptPath);
				$profilePath = str_replace('$user', 'user', $this->attributes['profilePath'][0]);
				$profilePath = str_replace('$group', 'group', $profilePath);
				if ( (!$smbHome=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+)+$', $smbHome)))
						$errors['smbHome'][] = array('ERROR', _('Home path'), _('Home path is invalid.'));
				if ( (!$scriptPath=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*'.
					'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*)*(([.][b][a][t])|([.][c][m][d]))$', $scriptPath)))
					$errors['scriptPath'][] = array('ERROR', _('Script path'), _('Script path is invalid!'));
				if ( (!$profilePath=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $profilePath))
					&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $profilePath)))
						$errors['profilePath'][] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
				}

			if ($post['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;
			}

		if ((!$this->attributes['domain'][0]=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $this->attributes['domain'][0]))
			$errors['domain'][] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));

		if (is_array($errors)) return $errors;
		if ($post['userWorkstations'])  return 'userWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_userWorkstations($post, $profile=false) {
		// Load attributes
		if ($_SESSION[$this->base]->type=='user') {
			do { // X-Or, only one if() can be true
				if (isset($post['availableUserWorkstations']) && isset($post['userWorkstations_add'])) { // Add workstations to list
					$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Add new // Add workstations
					$workstations = array_merge($workstations, $post['availableUserWorkstations']);
					// remove doubles
					$workstations = array_flip($workstations);
					array_unique($workstations);
					$workstations = array_flip($workstations);
					// sort workstations
					sort($workstations);
					// Recreate workstation string
					$this->attributes['userWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
						}
					break;
					}
				if (isset($post['userWorkstations']) && isset($post['userWorkstations_remove'])) { // remove // Add workstations from list
					// Put all workstations in array
					$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Remove unwanted workstations from array
					$workstations = array_delete($post['userWorkstations'], $workstations);
					// Recreate workstation string
					$this->attributes['userWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
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
	function display_html_attributes($post, $profile=false) {
		if ($_SESSION[$this->base]->type=='user') {
			$canchangedate = getdate($this->attributes['pwdCanChange'][0]);
			$mustchangedate = getdate($this->attributes['pwdMustChange'][0]);
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_h', 'type' => 'hidden', 'value' => $canchangedate['hours']),
					1 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_m', 'type' => 'hidden', 'value' => $canchangedate['minutes']),
					2 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_s', 'type' => 'hidden', 'value' => $canchangedate['seconds']),
					3 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_h', 'type' => 'hidden', 'value' => $mustchangedate['hours']),
					4 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_m', 'type' => 'hidden', 'value' => $mustchangedate['minutes']),
					5 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_s', 'type' => 'hidden', 'value' => $mustchangedate['seconds']),
					6 => array ( 'kind' => 'input', 'name' => 'acctFlagsU', 'type' => 'hidden', 'value' => 'true'));

			if (!$profile) {
				if ($this->attributes['lmPassword'][0] != $this->orig['lmPassword'][0]) $password=$this->lmPassword();
					else $password='';
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba password') ),
					1 => array ( 'kind' => 'input', 'name' => 'lmPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password));
				if ($post['lmPassword2']!='') $password2 = $post['lmPassword2'];
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
					1 => array ( 'kind' => 'input', 'name' => 'lmPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
					2 => array ('kind' => 'help', 'value' => 'lmPassword'));
				}

			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd),
					2 => array ('kind' => 'help', 'value' => 'useunixpwd'));
				}
			$checked = false;
			if (strpos($this->attributes['acctFlags'][0], "N")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsN', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'acctFlagsN'));
			$checked = false;
			if (strpos($this->attributes['acctFlags'][0], "X")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsX', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'acctFlagsX'));
			$checked = false;
			if (strpos($this->attributes['acctFlags'][0], "D")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsD', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'acctFlagsD'));
			for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User can change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_day',
						'options' => $mday, 'options_selectd' => $canchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_mon',
						'options' => $mon, 'options_selectd' => $canchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_yes',
						'options' => $year, 'options_selectd' => $canchangedate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_day',
						'options' => $mday, 'options_selectd' => $mustchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_mon',
						'options' => $mon, 'options_selectd' => $mustchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_yes',
						'options' => $year, 'options_selectd' => $mustchangedate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdMustChange' ));
			for ($i=90; $i>67; $i--) $drives[] = chr($i).':';
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home drive') ),
				1 => array ( 'kind' => 'select', 'name' => 'homeDrive', 'options' => $drives, 'options_selected' => array ($this->attributes['homeDrive'][0])),
				2 => array ( 'kind' => 'help', 'value' => 'homeDrive' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'smbHome', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['smbHome'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'smbHome' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Profile path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'profilePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['profilePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'profilePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Script path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'scriptPath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['scriptPath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'scriptPath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba workstations') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'userWorkstations', 'value' => _('Edit workstations')),
				2 => array ( 'kind' => 'help', 'value' => 'userWorkstations' ));

			if (!$profile) {
				$names = array_keys($this->rids);
				$wrid=false;
				for ($i=0; $i<count($names); $i++) {
					if ($this->attributes['primaryGroupID'][0]==$this->rids[$names[$i]]) {
						$selected[] = $names[$i];
						$wrid=true;
						}
					else $options[] = $names[$i];
					}
				if ($wrid) $options[] = $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
					else $selected[] = $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
					1 => array ( 'kind' => 'select', 'name' => 'primaryGroupID', 'options' => $options, 'options_selected' => $selected),
					2 => array ( 'kind' => 'help', 'value' => 'primaryGroupID' ));
				// Display if group SID should be mapped to a well kown SID
				$wrid=false;
				if ($this->attributes['rid'][0]=="500") {
					$selected[] = _('Administrator');
					$wrid=true;
					}
				else $options[] = _('Administrator');
				if ($this->attributes['rid'][0]=="501") {
					$selected[] = _('Guest');
					$wrid=true;
					}
				else $options[] = _('Guest');
				if ($wrid) $options[] = _('Ordinary user');
					else $selected[] = _('Ordinary user');
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
					1 => array ( 'kind' => 'select', 'name' => 'rid', 'options' => $options, 'options_selected' => $selected),
					2 => array ( 'kind' => 'help', 'value' => 'rid' ));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'domain', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['domain'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'domain' ));
			}

		if ($_SESSION[$this->base]->type=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'acctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			if (!$profile) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
					1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword'),
					2 => array ( 'kind' => 'help', 'value' => 'ResetSambaPassword' ));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'domain', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['domain'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'domain' ));
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
	function display_html_userWorkstations($post) {
		if ($_SESSION[$this->base]->type=='user') {
			// Get list of all hosts.
			$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uid', 'sambaAccount', 'host');
			if (is_array($result)) {
				foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
				sort($availableUserWorkstations, SORT_STRING);
				$result = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$userWorkstations = explode (',', $result);
				$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);
				}

			$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Allowed workstations"), 'value' =>
				array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Allowed workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'userWorkstations[]', 'size' => '15', 'multiple', 'options' => $userWorkstations)))),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'userWorkstations_add',
					'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'userWorkstations_remove', 'value' => '=>' )),
					2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'userWorkstations' )))),
				2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'availableUserWorkstations[]', 'size' => '15', 'multiple', 'options' => $availableUserWorkstations))))
					))));

			$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'value' => _('Back') ),
				1 => array ( 'kind' => 'text'),
				2 => array ('kind' => 'text'));
			}

		return $return;
		}

	function get_profileOptions() {
		$return = array();
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'domain', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['domain'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'domain' ));
		return $return;
	}

	// checks if the values of a new or modified profile are valid
	// $scope: the account type (user, group, host, ...)
	// $options: a hash array (name => value) containing the options
	function check_profileOptions($scope, $options) {
		return array();
	}

}

?>
