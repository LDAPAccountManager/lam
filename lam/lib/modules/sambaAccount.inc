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
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'sambaAccount\');'), E_USER_ERROR);
		$this->base = $base;
		// sambaAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() == 'host')) trigger_error(_('sambaAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['posixAccount'])) $_SESSION[$this->base]->add_objectClass('posixAccount');
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

	function get_alias() {
		return _('sambaAccount');
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

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'userWorkstations');
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
	function proccess_attributes($post) {
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

		if ($_SESSION[$this->base]->type=='host') {
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
					$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'), 'lmPassword');
					unset ($post['lmPassword2']);
					}
					else $this->lmPassword($post['lmPassword']);
				}
			if ($post['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;

			if ($post['rid']== _('Administrator')) {
				$this->attributes['rid'][0] = "500";
				// Do a check if an administrator already exists
				if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache("500", 'rid', 'user')!=$_SESSION[$this->base]->dn_orig)
					$errors[] = array('ERROR', _('Special user'), _('There can be only one administrator per domain.'), 'rid');
				}
			if ($post['rid']== _('Guest')) {
				$this->attributes['rid'][0] = "501";
				// Do a check if an administrator already exists
				if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache("501", 'rid', 'user')!=$_SESSION[$this->base]->dn_orig)
					$errors[] = array('ERROR', _('Special user'), _('There can be only one guest per domain.'), 'rid');
				}
			$this->attributes['smbHome'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['smbHome'][0]);
			$this->attributes['smbHome'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['smbHome'][0]);
			if ($this->attributes['smbHome'][0] != stripslashes($post['smbHome'])) $errors[] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'), 'smbHome');
			$this->attributes['scriptPath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['scriptPath'][0]);
			$this->attributes['scriptPath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['scriptPath'][0]);
			if ($this->attributes['scriptPath'][0] != stripslashes($post['scriptPath'])) $errors[] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'), 'scriptPath');
			$this->attributes['profilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['profilePath'][0]);
			$this->attributes['profilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['profilePath'][0]);
			if ($this->attributes['profiletPath'][0] != stripslashes($post['profilePath'])) $errors[] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'), 'profilePath');
			if ( (!$this->attributes['smbHome'][0]=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+)+$', $this->attributes['smbHome'][0])))
					$errors[] = array('ERROR', _('Home path'), _('Home path is invalid.'), 'smbHome');
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
				$this->lmPassword())) $errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'), 'lmPassword');
			if ( (!$this->attributes['scriptPath'][0]=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*'.
				'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*)*(([.][b][a][t])|([.][c][m][d]))$', $this->attributes['scriptPath'][0])))
				$errors[] = array('ERROR', _('Script path'), _('Script path is invalid!'), 'scriptPath');
			if ( (!$this->attributes['profilePath'][0]=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $this->attributes['profilePath'][0]))
				&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $this->attributes['profilePath'][0])))
					$errors[] = array('ERROR', _('Profile path'), _('Profile path is invalid!'), 'profilePath');
			}

		if ((!$this->attributes['domain'][0]=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $this->attributes['domain'][0]))
			$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'), 'domain');

		if (is_array($errors)) return $errors;
		if ($post['userWorkstations'])  return 'userWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_userWorkstations($post) {
		// Load attributes
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
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		if ($_SESSION[$this->base]->type=='user') {
			$canchangedate = getdate($this->attributes['pwdCanChange'][0]);
			$mustchangedate = getdate($this->attributes['pwdMustChange'][0]);
			echo	'<input name="pwdCanChange_h" type="hidden" value="'.$canchangedate['hours'].'">'.
				'<input name="pwdCanChange_m" type="hidden" value="'.$canchangedate['minutes'].'">'.
				'<input name="pwdCanChange_s" type="hidden" value="'.$canchangedate['seconds'].'">'.
				'<input name="pwdMustChange_h" type="hidden" value="'.$mustchangedate['hours'].'">'.
				'<input name="pwdMustChange_m" type="hidden" value="'.$mustchangedate['minutes'].'">'.
				'<input name="pwdMustChange_s" type="hidden" value="'.$mustchangedate['seconds'].'">'.
				'<input name="acctFlagsU" type="hidden" value="true">';
			echo "<table border=0 width=\"100%\">\n<tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Samba password') . "</td>\n";
			echo "<td><input name=\"lmPassword\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"" . $this->lmPassword() . "\"></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Repeat password') . "</td>\n";
			echo "<td><input name=\"lmPassword2\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"";
				if ($post['lmPassword2']!='') echo $post['lmPassword2'];
				else echo $this->lmPassword();
				echo "\"></td>\n";
			echo "<td></td>\n";
			echo "</tr>\n";
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]) {
				echo "<tr>\n";
				echo "<td>" . _('Use unix password') . "</td>\n";
				echo "<td><input name=\"useunixpwd\" type=\"checkbox\"";
					if ($this->useunixpwd) echo " checked ";
					echo "></td>\n";
				echo "<td><a href=\"../help.php?HelpNumber=428\" target=\"lamhelp\">" . _('Help') . "</a></td>";
				echo "</tr>\n";
				}
			echo "<tr>\n";
			echo "<td>" . _('Use no password') . "</td>\n";
			echo "<td><input name=\"acctFlagsN\" type=\"checkbox\"";
				if (strpos($this->attributes['acctFlags'][0], "N")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help' ) ."</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Password does not expire') . "</td>\n";
			echo "<td><input name=\"acctFlagsX\" type=\"checkbox\"";
				if (strpos($this->attributes['acctFlags'][0], "X")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=429\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('User can change password') . "</td>\n";
			echo "<td><select name=\"pwdCanChange_day\">";
					for ( $i=1; $i<=31; $i++ ) {
						if ($canchangedate['mday']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"pwdCanChange_mon\">";
					for ( $i=1; $i<=12; $i++ ) {
						if ($canchangedate['mon'] == $i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"pwdCanChange_yea\">";
					for ( $i=2003; $i<=2030; $i++ ) {
						if ($canchangedate['year']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=430\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('User must change password') . "</td>\n";
			echo "<td><select name=\"pwdMustChange_day\">";
					for ( $i=1; $i<=31; $i++ ) {
						if ($mustchangedate['mday']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"pwdMustChange_mon\">";
					for ( $i=1; $i<=12; $i++ ) {
						if ($mustchangedate['mon'] == $i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"pwdMustChange_yea\">";
					for ( $i=2030; $i>=2003; $i-- ) {
						if ($mustchangedate['year']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=431\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" .  _('Account is deactivated') . "</td>\n";
			echo "<td><input name=\"acctFlagsD\" type=\"checkbox\"";
				if (strpos($this->attributes['acctFlags'][0], "D")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=432\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home drive') . "</td>\n";
			echo "<td><select name=\"homeDrive\">";
				for ($i=90; $i>67; $i--)
					if ($this->attributes['homeDrive'][0]== chr($i).':') echo "<option selected>".chr($i).":</option>";
						else echo "<option>".chr($i).":</option>";
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=433\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home path') . "</td>\n";
			echo "<td><input name=\"smbHome\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['smbHome'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=437\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Profile path') . "</td>\n";
			echo "<td><input name=\"profilePath\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['profilePath'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=435\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Script path') . "</td>\n";
			echo "<td><input name=\"scriptPath\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['scriptPath'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=434\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Samba workstations') . "</td>\n";
			echo "<td><input name=\"userWorkstations\" type=\"submit\" value=\"" . _('Edit workstations') . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=436\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Windows group') . "</td>\n";
			echo "<td><select name=\"primaryGroupID\">";
				// Display if group SID should be mapped to a well kown SID
				$names = array_keys($this->rids);
				$wrid=false;
				for ($i=0; $i<count($names); $i++) {
					if ($this->attributes['primaryGroupID'][0]==$this->rids[$names[$i]]) {
						echo "<option selected>" . $names[$i] . "</option>";
						$wrid=true;
						}
					else echo "<option>" . $names[$i] . "</option>";
					}
				if ($wrid) echo "<option>" . $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]) . "</option>";
					else echo "<option selected>" . $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]) . "</option>";
				echo	"</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=464\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Special user') . "</td>\n";
			echo "<td><select name=\"rid\">";
				// Display if group SID should be mapped to a well kown SID
				$wrid=false;
				if ($this->attributes['rid'][0]=="500") {
					echo "<option selected>" . _('Administrator') . "</option>";
					$wrid=true;
					}
				else echo "<option>" . _('Administrator') . "</option>";
				if ($this->attributes['rid'][0]=="501") {
					echo "<option selected>" . _('Guest') . "</option>";
					$wrid=true;
					}
				else echo "<option>" . _('Guest') . "</option>";
				if ($wrid) echo "<option>" . _('Ordinary user') . "</option>";
					else echo "<option selected>" . _('Ordinary user') . "</option>";
				echo	"</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=XXX\" target=\"lamhelp\">" . _('Help-XX') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Domain') . "</td>\n";
			echo "<td><input name=\"domain\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['domain'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=438\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "</table>\n";
			}
		if ($_SESSION[$this->base]->type=='host') {
			echo '<input name="acctFlagsW" type="hidden" value="true">';
			echo "<table border=0 width=\"100%\">\n<tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Reset password') . "</td>\n";
			echo "<td><input name=\"ResetSambaPassword\" type=\"submit\" value=\"" . _('Reset password') . "\"></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Domain') . "</td>\n";
			echo "<td><input name=\"domain\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['domain'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=438\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "</table>\n";
			}
		return 0;
		}

	function display_html_delete($post) {
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_userWorkstations($post) {
		// Get list of all hosts.
		$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uid', 'sambaAccount', 'host');
		if (is_array($result)) {
			foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
			sort($availableUserWorkstations, SORT_STRING);
			$result = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
			$userWorkstations = explode (',', $result);
			$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);
			}
		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<tr>\n";
		echo "<td valign=\"top\">";
			echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\"><legend class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo _("Allowed workstations");
				echo "</legend>\n";
			// display all workstations the user is allowed to login
			if (count($userWorkstations)!=0) {
				echo "<select name=\"userWorkstations[]\" class=\"".$_SESSION[$this->base]->type."edit-bright\" size=15 multiple>\n";
				for ($i=0; $i<count($userWorkstations); $i++)
					if ($userWorkstations[$i]!='') echo "<option>".$userWorkstations[$i]."</option>\n";
				echo "</select>\n";
				}
			echo "</fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"userWorkstations_add\" value=\"<=\">";
			echo " ";
			echo "<input type=\"submit\" name=\"userWorkstations_remove\" value=\"=>\"><br><br>";
			echo "<a href=\""."../help.php?HelpNumber=436\" target=\"lamhelp\">"._('Help')."</a></td>\n";
		echo "<td valign=\"top\">";
			echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\"><legend class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo _('Available workstations');
				echo "</legend>\n";
			// Display all workstations without these the user is allowed to login
			if (count($availableUserWorkstations)!=0) {
				echo "<select name=\"availableUserWorkstations[]\" size=15 multiple class=\"".$_SESSION[$this->base]->type."edit-bright\">\n";
				foreach ($availableUserWorkstations as $temp) echo "<option>$temp</option>\n";
				echo "</select>\n";
				}
			echo "</fieldset></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td><input name=\"attributes\" type=\"submit\" value=\"" . _('Back') . "\"></td>\n";
		echo "<td></td>\n";
		echo "<td></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		}

	}

?>
