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
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'sambaSamAccount\');'), E_USER_ERROR);
		$this->base = $base;
		// sambaSamAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() == 'host')) trigger_error(_('sambaSamAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['posixAccount'])) $_SESSION[$this->base]->add_objectClass('posixAccount');
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('sambaSamAccount');
		$_SESSION[$this->base]->add_attributes ('sambaSamAccount');
		$this->alias = _('sambaSamAccount');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		// fixme *** do we have to unset module sambaSamAccount itself
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
	// Alias Name. This name is shown in the menu instead of sambaSamAccount
	var $alias;
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

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array('posixAccount');
		}

	function module_ready() {
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]=='') return false;
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]=='') return false;
		if ($this->attributes['uid'][0]=='') return false;
		return true;
		}

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
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$this->attributes['sambaLMPassword'][0] = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $newpassword, MCRYPT_MODE_ECB, $iv));
			return 0;
			}
		else {
			if ($this->useunixpwd) return $_SESSION[$this->base]->module['inetOrgPerson']->userPassword();
			if ($this->attributes['sambaLMPassword'][0]!='') {
				// Read existing password if set
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->attributes['sambaLMPassword'][0]), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				return $password;
				}
			else return '';
			}
		}



	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Load attributes
		$this->attributes['sambaDomainName'][0] = $post['form_sambaSamAccount_sambaDomainName'];
		// Get Domain SID from name
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomain[$i]->RIDbase;
				}

		$flag = "[";
		if ($post['form_sambaSamAccount_sambaAcctFlagsD']) $flag .= "D";
		if ($post['form_sambaSamAccount_sambaAcctFlagsX']) $flag .= "X";
		if ($post['form_sambaSamAccount_sambaAcctFlagsN']) $flag .= "N";
		if ($post['form_sambaSamAccount_sambaAcctFlagsS']) $flag .= "S";
		if ($post['form_sambaSamAccount_sambaAcctFlagsH']) $flag .= "H";
		if ($post['form_sambaSamAccount_sambaAcctFlagsW']) $flag .= "W";
		if ($post['form_sambaSamAccount_sambaAcctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['sambaAcctFlags'][0] = $flag;

		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['sambaPwdCanChange'][0] = mktime($post['form_sambaSamAccount_sambaPwdCanChange_h'], $post['form_sambaSamAccount_sambaPwdCanChange_m'], $post['form_sambaSamAccount_sambaPwdCanChange_s'],
				$post['form_sambaSamAccount_sambaPwdCanChange_mon'], $post['form_sambaSamAccount_sambaPwdCanChange_day'], $post['form_sambaSamAccount_sambaPwdCanChange_yea']);
			$this->attributes['sambaPwdMustChange'][0] = mktime($post['form_sambaSamAccount_sambaPwdMustChange_h'], $post['form_sambaSamAccount_sambaPwdMustChange_m'], $post['form_sambaSamAccount_sambaPwdMustChange_s'],
				$post['form_sambaSamAccount_sambaPwdMustChange_mon'], $post['form_sambaSamAccount_sambaPwdMustChange_day'], $post['form_sambaSamAccount_sambaPwdMustChange_yea']);
			$this->attributes['sambaHomePath'][0] = stripslashes($post['form_sambaSamAccount_sambaHomePath']);
			$this->attributes['sambaHomeDrive'][0] = $post['form_sambaSamAccount_sambaHomeDrive'];
			$this->attributes['sambaLogonScript'][0] = stripslashes($post['form_sambaSamAccount_sambaLogonScript']);
			$this->attributes['sambaProfilePath'][0] = stripslashes($post['form_sambaSamAccount_sambaProfilePath']);
			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($post['form_sambaSamAccount_sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[$rids[$i]];
					}
				}
			if (!$wrid) $this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+$RIDbase+1;

			if (isset($post['form_sambaSamAccount_sambaLMPassword'])) {
				if ($post['form_sambaSamAccount_sambaLMPassword'] != $post['form_sambaSamAccount_sambaLMPassword2']) {
					$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
					unset ($post['form_sambaSamAccount_sambaLMPassword2']);
					}
					else $this->sambaLMPassword($post['form_sambaSamAccount_sambaLMPassword']);
				}
			if ($post['form_sambaSamAccount_useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;
			}


		// Check values
		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['sambaHomePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaHomePath'][0]);
			$this->attributes['sambaHomePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaHomePath'][0]);
			if ($this->attributes['sambaHomePath'][0] != stripslashes($post['form_sambaSamAccount_sambaHomePath'])) $errors[] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
			$this->attributes['sambaLogonScript'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaLogonScript'][0]);
			$this->attributes['sambaLogonScript'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaLogonScript'][0]);
			if ($this->attributes['sambaLogonScript'][0] != stripslashes($post['form_sambaSamAccount_sambaLogonScript'])) $errors[] = array('INFO', _('Logon script'), _('Inserted user- or groupname in logon script.'));
			$this->attributes['sambaProfilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaProfilePath'][0]);
			$this->attributes['sambaProfilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaProfilePath'][0]);
			if ($this->attributes['sambaProfiletPath'][0] != stripslashes($post['form_sambaSamAccount_sambaProfilePath'])) $errors[] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+)+$', $this->attributes['sambaHomePath'][0])))
					$errors[] = array('ERROR', _('Home path'), _('Home path is invalid.'));
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
				$this->sambaLMPassword())) $errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*'.
				'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[?]|[?]|[?]|[?]|[?]|[?]|[?])*)*(([.][b][a][t])|([.][c][m][d]))$', $this->attributes['sambaLogonScript'][0])))
				$errors[] = array('ERROR', _('Script path'), _('Script path is invalid!'));
			if ( (!$this->attributes['sambaProfilePath'][0]=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $this->attributes['sambaProfilePath'][0]))
				&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $this->attributes['sambaProfilePath'][0])))
					$errors[] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
			}

		if ((!$this->attributes['sambaDomainName'][0]=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $this->attributes['sambaDomainName'][0]))
			$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));


		if (is_array($errors)) return $errors;
		if ($post['form_sambaSamAccount_sambaUserWorkstations'])  return 'sambaUserWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_sambaUserWorkstations($post) {
		// Load attributes
		do { // X-Or, only one if() can be true
			if (isset($post['form_sambaSamAccount_availableSambaUserWorkstations']) && isset($post['form_sambaSamAccount_sambaUserWorkstations_add'])) { // Add workstations to list
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++)
					if ($workstations[$i]=='') unset($workstations[$i]);
				$workstations = array_values($workstations);
				// Add new // Add workstations
				$workstations = array_merge($workstations, $post['form_sambaSamAccount_availableSambaUserWorkstations']);
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
			if (isset($post['form_sambaSamAccount_sambaUserWorkstations']) && isset($post['form_sambaSamAccount_sambaUserWorkstations_remove'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++)
					if ($workstations[$i]=='') unset($workstations[$i]);
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($post['form_sambaSamAccount_sambaUserWorkstations'], $workstations);
				// Recreate workstation string
				$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
					}
				break;
				}
			} while(0);
		if ($post['form_sambaSamAccount_attributes']) return 'attributes';
		return 0;
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
				for ($i=0; $i<count($this->attributes[$attribute]); $i++) $this->attributes[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
				}
			}
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->attributes['objectClass'][0] = 'sambaSamAccount';
		$this->orig = $this->attributes;
		$this->sambaLMPassword(''); // Remove old password so it won't displayed as hash
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


	/* This function returns all ldap attributes
	* which are part of sambaSamAccount and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->attributes;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		if ($_SESSION[$this->base]->type=='user') {
			$canchangedate = getdate($this->attributes['sambaPwdCanChange'][0]);
			$mustchangedate = getdate($this->attributes['sambaPwdMustChange'][0]);
			// Get Domain SID from name
			$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
			for ($i=0; $i<count($sambaDomains); $i++ ) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
				if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name)
					$SID = $sambaDomains[$i]->SID;
				}
			echo	'<input name="form_sambaSamAccount_sambaPwdCanChange_h" type="hidden" value="'.$canchangedate['hours'].'">'.
				'<input name="form_sambaSamAccount_sambaPwdCanChange_m" type="hidden" value="'.$canchangedate['minutes'].'">'.
				'<input name="form_sambaSamAccount_sambaPwdCanChange_s" type="hidden" value="'.$canchangedate['seconds'].'">'.
				'<input name="form_sambaSamAccount_sambaPwdMustChange_h" type="hidden" value="'.$mustchangedate['hours'].'">'.
				'<input name="form_sambaSamAccount_sambaPwdMustChange_m" type="hidden" value="'.$mustchangedate['minutes'].'">'.
				'<input name="form_sambaSamAccount_sambaPwdMustChange_s" type="hidden" value="'.$mustchangedate['seconds'].'">'.
				'<input name="form_sambaSamAccount_sambaAcctFlagsU" type="hidden" value="true">';
			echo "<table border=0 width=\"100%\">\n<tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Samba password') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaLMPassword\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"" . $this->sambaLMPassword() . "\"></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Repeat password') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaLMPassword2\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"";
				if ($post['form_sambaSamAccount_sambaLMPassword2']!='') echo $post['form_sambaSamAccount_sambaLMPassword2'];
				else echo $this->sambaLMPassword();
				echo "\"></td>\n";
			echo "<td></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Use unix password') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_useunixpwd\" type=\"checkbox\"";
				if ($this->useunixpwd) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=428\" target=\"lamhelp\">" . _('Help') . "</a></td>";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Use no password') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaAcctFlagsN\" type=\"checkbox\"";
				if (strpos($this->attributes['sambaAcctFlags'][0], "N")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help' ) ."</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Password does not expire') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaAcctFlagsX\" type=\"checkbox\"";
				if (strpos($this->attributes['sambaAcctFlags'][0], "X")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=429\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('User can change password') . "</td>\n";
			echo "<td><select name=\"form_sambaSamAccount_sambaPwdCanChange_day\">";
					for ( $i=1; $i<=31; $i++ ) {
						if ($canchangedate['mday']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"form_sambaSamAccount_sambaPwdCanChange_mon\">";
					for ( $i=1; $i<=12; $i++ ) {
						if ($canchangedate['mon'] == $i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"form_sambaSamAccount_sambaPwdCanChange_yea\">";
					for ( $i=2003; $i<=2030; $i++ ) {
						if ($canchangedate['year']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=430\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('User must change password') . "</td>\n";
			echo "<td><select name=\"form_sambaSamAccount_sambaPwdMustChange_day\">";
					for ( $i=1; $i<=31; $i++ ) {
						if ($mustchangedate['mday']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"form_sambaSamAccount_sambaPwdMustChange_mon\">";
					for ( $i=1; $i<=12; $i++ ) {
						if ($mustchangedate['mon'] == $i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select><select name=\"form_sambaSamAccount_sambaPwdMustChange_yea\">";
					for ( $i=2030; $i>=2003; $i-- ) {
						if ($mustchangedate['year']==$i) echo "<option selected>$i</option>";
							else echo "<option>$i</option>";
						}
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=431\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" .  _('Account is deactivated') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaAcctFlagsD\" type=\"checkbox\"";
				if (strpos($this->attributes['sambaAcctFlags'][0], "D")) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=432\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home drive') . "</td>\n";
			echo "<td><select name=\"form_sambaSamAccount_sambaHomeDrive\">";
				for ($i=90; $i>67; $i--)
					if ($this->attributes['sambaHomeDrive'][0]== chr($i).':') echo "<option selected>".chr($i).":</option>";
						else echo "<option>".chr($i).":</option>";
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=433\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home path') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaHomePath\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['sambaHomePath'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=437\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Profile path') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaProfilePath\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['sambaProfilePath'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=435\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Logon script') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaLogonScript\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"" . $this->attributes['sambaLogonScript'][0] . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=434\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Samba workstations') . "</td>\n";
			echo "<td><input name=\"form_sambaSamAccount_sambaUserWorkstations\" type=\"submit\" value=\"" . _('Edit workstations') . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=436\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Windows group') . "</td>\n";
			echo "<td><select name=\"form_sambaSamAccount_sambaPrimaryGroupSID\">";
				// Display if group SID should be mapped to a well kown SID
				$names = array_keys($this->rids);
				$wrid=false;
				for ($i=0; $i<count($names); $i++) {
					if ($this->attributes['sambaPrimaryGroupSID'][0]==$SID."-".$this->rids[$names[$i]]) {
						echo "<option selected>" . $names[$i] . "</option>";
						$wrid=true;
						}
					else echo "<option>" . $names[$i] . "</option>";
					}
				if ($wrid) echo "<option>" . getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]) . "</option>";
					else echo "<option selected>" . getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]) . "</option>";
				echo	"</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=464\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Domain') . "</td>\n";
			echo "<td><select name=\"form_sambaSamAccount_sambaDomainName\">";
				foreach ($sambaDomainNames as $domain) {
					if ($this->attributes['sambaDomainName'][0]==$domain) echo "<option selected>$domain</option>";
						else "<option>$domain</option>";
					}
				echo "</select></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=438\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "</table>\n";
			}
		else {

			}
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_sambaUserWorkstations($post) {
		// Get list of all hosts.
		$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uid', 'sambaSamAccount', 'host');
		if (is_array($result)) {
			foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
			sort($availableUserWorkstations, SORT_STRING);
			$result = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
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
				echo "<select name=\"form_sambaSamAccount_sambaUserWorkstations[]\" class=\"".$_SESSION[$this->base]->type."edit-bright\" size=15 multiple>\n";
				for ($i=0; $i<count($userWorkstations); $i++)
					if ($userWorkstations[$i]!='') echo "<option>".$userWorkstations[$i]."</option>\n";
				echo "</select>\n";
				}
			echo "</fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"form_sambaSamAccount_sambaUserWorkstations_add\" value=\"<=\">";
			echo " ";
			echo "<input type=\"submit\" name=\"form_sambaSamAccount_sambaUserWorkstations_remove\" value=\"=>\"><br><br>";
			echo "<a href=\""."../help.php?HelpNumber=436\" target=\"lamhelp\">"._('Help')."</a></td>\n";
		echo "<td valign=\"top\">";
			echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\"><legend class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo _('Available workstations');
				echo "</legend>\n";
			// Display all workstations without these the user is allowed to login
			if (count($availableUserWorkstations)!=0) {
				echo "<select name=\"form_sambaSamAccount_availableSambaUserWorkstations[]\" size=15 multiple class=\"".$_SESSION[$this->base]->type."edit-bright\">\n";
				foreach ($availableUserWorkstations as $temp) echo "<option>$temp</option>\n";
				echo "</select>\n";
				}
			echo "</fieldset></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td><input name=\"form_sambaSamAccount_attributes\" type=\"submit\" value=\"" . _('Back') . "\"></td>\n";
		echo "<td></td>\n";
		echo "<td></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		}


	}



?>
