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

/* This class contains all sambaSamAccount LDAP attributes
* and funtioncs required to deal with sambaSamAccount
* sambaSamAccount can only be created when it should be added
* to an array.
* basearray is the same array sambaSamAccount should be added
* to. If basearray is not given the constructor tries to
* create an array with sambaSamAccount and all other required
* objects.
* Example: $user[] = new sambaSamAccount($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class sambaSamAccount {
	// Constructor
	function sambaSamAccount($base) {
		/* Return an error if sambaSamAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object in an accountContainer object first.'), E_USER_ERROR);
		$this->base = $base;
		// sambaSamAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() == 'host')) trigger_error(_('sambaSamAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['posixAccount'])) $_SESSION[$this->base]->modules['posixAccount'] = new posixAccount($this->base);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('sambaSamAccount');
		$_SESSION[$this->base]->add_attributes ('sambaSamAccount');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'sambaSamAccount';
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

	/* $attribute['sambaLMPassword'] and sambaNTPassword can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function sambaLMPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			$this->attributes['sambaLMPassword'][0] = base64_encode($_SESSION[$_SESSION[$this->base]->ldap]->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->useunixpwd) return $_SESSION[$this->base]->module['posixAccount']->userPassword();
			if ($this->attributes['sambaLMPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION[$_SESSION[$this->base]->ldap]->decrypt(base64_decode($this->attributes['sambaLMPassword'][0]));
				}
			else return '';
			}
		}

	function get_alias($scope) {
		return _('sambaSamAccount');
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
		if ($scope=='host') return array('require' => array('posixAccount'), 'conflict' => array() );
		if ($scope=='user') return array('require' => array('posixAccount'), 'conflict' => array() );
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
	* which are part of posixAccount and returns
	* also their values.
	*/
	function get_attributes() {
		$return = $this->attributes;
		$return['sambaLMPassword'] = $this->sambaLMPassword();
		return $return;
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
		$this->attributes['objectClass'][0] = 'sambaSamAccount';
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
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
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
		// Set password
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']);
		if (!isset($this->orig['sambaLMPassword'][0])) {
			$return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($this->sambaLMPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($this->sambaLMPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['sambaPwdLastSet'][0] = time();
			}
		if ($this->sambaLMPassword()!='') {
			$return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($this->sambaLMPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword'][0] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($this->sambaLMPassword()));
			$return[$_SESSION[$this->base]->dn]['modify']['sambaPwdLastSet'][0] = time();
			}
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['sambaDomainName'][0] = $post['sambaDomainName'];
		// Get Domain SID from name
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
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
				// *** fixme. What is the default password?
				$this->sambaLMPassword('');
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

			if (isset($post['sambaLMPassword']) && !$profile) {
				if ($post['sambaLMPassword'] != $post['sambaLMPassword2']) {
					$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'), 'sambaLMPassword');
					unset ($post['sambaLMPassword2']);
					}
					else $this->sambaLMPassword($post['sambaLMPassword']);
				}
			if ($post['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;

			if (!$profile) {
				if ($post['sambaSID']== _('Administrator')) {
					$this->attributes['sambaSID'][0] = $SID."-500";
					// Do a check if an administrator already exists
					if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($SID."-500", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
						$errors['sambaSID'][] = array('ERROR', _('Special user'), _('There can be only one administrator per domain.'));
					}
				if ($post['sambaSID']== _('Guest')) {
					$this->attributes['sambaSID'][0] = $SID."-501";
					// Do a check if an administrator already exists
					if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($SID."-501", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
						$errors['sambaSID'][] = array('ERROR', _('Special user'), _('There can be only one guest per domain.'));
					}
				// Check values
				$this->attributes['sambaHomePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaHomePath'][0]);
				$this->attributes['sambaHomePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaHomePath'][0]);
				if ($this->attributes['sambaHomePath'][0] != stripslashes($post['sambaHomePath'])) $errors['sambaHomePath'][] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
				$this->attributes['sambaLogonScript'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaLogonScript'][0]);
				$this->attributes['sambaLogonScript'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaLogonScript'][0]);
				if ($this->attributes['sambaLogonScript'][0] != stripslashes($post['sambaLogonScript'])) $errors['sambaLogonScript'][] = array('INFO', _('Logon script'), _('Inserted user- or groupname in logon script.'));
				$this->attributes['sambaProfilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaProfilePath'][0]);
				$this->attributes['sambaProfilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaProfilePath'][0]);
				if ($this->attributes['sambaProfiletPath'][0] != stripslashes($post['sambaProfilePath'])) $errors['sambaProfilePath'][] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
				if ( (!$this->attributes['sambaHomePath'][0]=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+)+$', $this->attributes['sambaHomePath'][0])))
						$errors['sambaHomePath'][] = array('ERROR', _('Home path'), _('Home path is invalid.'));
				if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
					$this->sambaLMPassword())) $errors['sambaLMPassword'][] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
				if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*'.
					'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*)*(([.][b][a][t])|([.][c][m][d]))$', $this->attributes['sambaLogonScript'][0])))
					$errors['sambaScriptPath'][] = array('ERROR', _('Script path'), _('Script path is invalid!'));
				if ( (!$this->attributes['sambaProfilePath'][0]=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $this->attributes['sambaProfilePath'][0]))
					&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $this->attributes['sambaProfilePath'][0])))
						$errors['sambaProfilePath'][] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
				}
			else {
				$sambaHomePath = str_replace('$user', 'user', $this->attributes['sambaHomePath'][0]);
				$sambaHomePath = str_replace('$group', 'group', $sambaHomePath);
				$sambaLogonScript = str_replace('$user', 'user', $this->attributes['sambaLogonScript'][0]);
				$sambaLogonScript = str_replace('$group', 'group', $sambaLogonScript);
				$sambaProfilePath = str_replace('$user', 'user', $this->attributes['sambaProfilePath'][0]);
				$sambaProfilePath = str_replace('$group', 'group', $sambaProfilePath);
				if ( (!$this->attributes['sambaHomePath'][0]=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+)+$', $this->attributes['sambaHomePath'][0])))
						$errors[] = array('ERROR', _('Home path'), _('Home path is invalid.'), 'sambaHomePath');
				if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
					$this->sambaLMPassword())) $errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'), 'sambaLMPassword');
				if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*'.
					'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*)*(([.][b][a][t])|([.][c][m][d]))$', $this->attributes['sambaLogonScript'][0])))
					$errors[] = array('ERROR', _('Script path'), _('Script path is invalid!'), 'sambaScriptPath');
				if ( (!$this->attributes['sambaProfilePath'][0]=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $this->attributes['sambaProfilePath'][0]))
					&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $this->attributes['sambaProfilePath'][0])))
						$errors[] = array('ERROR', _('Profile path'), _('Profile path is invalid!'), 'sambaProfilePath');
				}
			}
		if (is_array($errors)) return $errors;
		if ($post['sambaUserWorkstations'])  return 'sambaUserWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_sambaUserWorkstations($post, $profile=false) {
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
	function display_html_attributes($post, $profile=false) {
		// Get Domain SID from name
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
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
			if (!$profile) {
				if ($this->attributes['lmPassword'][0] != $this->orig['lmPassword'][0]) $password=$this->sambaLMPassword();
					else $password='';
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba password') ),
					1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password));
				if ($post['sambaLMPassword2']!='') $password2 = $post['sambaLMPassword2'];
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
					1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
					2 => array ('kind' => 'help', 'value' => 'sambaLMPassword'));
				}
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd),
					2 => array ('kind' => 'help', 'value' => 'useunixpwd'));
				}
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "N")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsN'));
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "X")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsX'));
			$checked = false;
			if (strpos($this->attributes['sambaScctFlags'][0], "D")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => $checked),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsD'));
			for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User can change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_day',
						'options' => $mday, 'options_selectd' => $canchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_mon',
						'options' => $mon, 'options_selectd' => $canchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_yes',
						'options' => $year, 'options_selectd' => $canchangedate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'sambaPwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_day',
						'options' => $mday, 'options_selectd' => $mustchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_mon',
						'options' => $mon, 'options_selectd' => $mustchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_yes',
						'options' => $year, 'options_selectd' => $mustchangedate['year'])))),
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

			if (!$profile) {
				$names = array_keys($this->rids);
				$wrid=false;
				for ($i=0; $i<count($names); $i++) {
					if ($this->attributes['sambaPrimaryGroupSID'][0]==$SID."-".$this->rids[$names[$i]]) {
						$selected[] = $names[$i];
						$wrid=true;
						}
					else $options[] = $names[$i];
					}
				if ($wrid) $options[] = $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
					else $selected[] = $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPrimaryGroupSID', 'options' => $options, 'options_selected' => $selected),
					2 => array ( 'kind' => 'help', 'value' => 'sambaPrimaryGroupSID' ));
				// Display if group SID should be mapped to a well kown SID
				$wrid=false;
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
				if ($wrid) $options[] = _('Ordinary user');
					else $selected[] = _('Ordinary user');
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
					1 => array ( 'kind' => 'select', 'name' => 'sambaSID', 'options' => $options, 'options_selected' => $selected),
					2 => array ( 'kind' => 'help', 'value' => 'sambaSID' ));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array($this->attributes['sambaDomainName'][0])),
				2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
			}
		if ($_SESSION[$this->base]->type=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			if (!$profile) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
					1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword'),
					2 => array ( 'kind' => 'help', 'value' => 'ResetSambaPassword' ));
				}
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
	function display_html_sambaUserWorkstations($post, $profile=false) {
		if ($_SESSION[$this->base]->type=='user') {
			// Get list of all hosts.
			$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uid', 'sambaSamAccount', 'host');
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

	function get_profileOptions($scope) {
		$return = array();
		// TODO fill in correct groups
		$return[] = array(0 => array('kind' => 'text', 'text' => _('Domain')),
			1 => array('kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames),
			2 => array('kind' => 'help', 'value' => 'sambaDomainName' ));
		return $return;
	}

}

?>
