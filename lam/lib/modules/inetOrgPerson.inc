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

/*
* Variables in basearray which are no objects:
* type: Type of account. Can be user, group, host
* attributes: List of all attributes, how to get them and are theiy required or optional
* dn: current DN without uid= or cn=
* dn_orig: old DN if account was loaded with uid= or cn=

* External functions which are used
* account.inc: array_delete
*/

// *** fixme, start session if not yet done
// *** fixme set language if not yet done
include_once('../ldap.inc');
include_once('../account.inc');

/* This class contains all inetOrgPerson LDAP attributes
* and funtioncs required to deal with inetOrgPerson
* inetOrgPerson can only be created when it should be added
* to an array.
* basearray is the same array inetOrgPerson should be added
* to. If basearray is not given the constructor tries to
* create an array with inetOrgPerson and all other required
* objects.
* Example: $user[] = new inetOrgPerson($user);
*
*/

class inetOrgPerson {
	// Constructor
	function inetOrgPerson(&$basearray=false) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$basearray) die _('Please create a new object with $array[] = new posixAccount($array);');
		// Check if $basearray is an array
		if (!is_array($basearray)) die _('Please create a new object with $array[] = new posixAccount($array);');
		// posixAccount is only a valid objectClass for user and host
		if !($basearray->get_type() == 'user') die _('inetOrgPerson can only be used for users.');
		/* Create a reference to basearray so we can read all other modules
		* php will avaois recousrion itself
		*/
		$this->base = &$basearray;
		// Add attributes which should be cached
		$_SESSION['cache']->add_cache(array ('user' => array('cn', 'uid'), 'host' => array('cn', 'uid') ));
		// Add Array with all attributes and type
		$basearray->add_attributes ('inetOrgPerson');

		// Add account type to object
		$orig = array(  'uid' => '', 'uidNumber' => '', 'gidNumber' => '', 'homeDirectory' => '', 'loginShell' => '', 'gecos' => '',
			'description' => '', 'enc_userPassword' => '', 'groups' => array() );
		$this->alias = _('inetOrgPerson');
		}

	// Variables
	// Alias Name. This name is shown in the menu instead of posixAccount
	var $alias;
	// reference to base-array so we can read other classes in basearray
	var $base;


	// Use a unix password?
	var $userPassword_no;
	// Lock account?
	var $userPassword_lock;
	// Array with all groups the user should also be member of
	var $groups;
	// LDAP attributes
	// These attributes have to be set in ldap
	var $uid;
	var $uidNumber;
	var $gidNumber;
	var $homeDirectory;
	// These attributes doesn't have to be set in ldap
	var $loginShell;
	var $gecos;
	var $description;
	/* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function userPassword($newpassword='') {
		// Read existing password if set
		if ($newpassword='') {
			if ($this->enc_userPassword != '') {
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->enc_userPassword), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				return $password;
				}
			else return '';
			}
		// Write new password
		else {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$this->enc_userPassword = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $newpassword, MCRYPT_MODE_ECB, $iv));
			return 0;
			}
		}

	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		if ($this->base['type']=='user') return array('inetOrgPerson');
		if ($this->base['type']=='host') return array('account');
		// return error if unsupported type is used
		return -1;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes() {
		// Load attributes
		$this->uid = $_POST['form_posixAccount_uid'];
		if ($this->base['type']=='user') $this->uid &= $this->base['inetOrgPerson']->cn;
		if ($this->base['type']=='host') $this->uid &= $this->base['account']->cn;
		$this->uidNumber = $_POST['form_posixAccount_uidNumber'];
		$this->gidNumber = getgrnam($_POST['form_posixAccount_gidNumber']);
		$this->homeDirectory = $_POST['form_posixAccount_homeDirectory'];
		$this->loginShell = $_POST['form_posixAccount_loginShell'];
		$this->gecos = $_POST['form_posixAccount_gecos'];
		$this->description = $_POST['form_posixAccount_description'];
		if ($_POST['form_posixAccount_userPassword_no']; $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($_POST['form_posixAccount_userPassword_lock']; $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if (isset($_POST['form_posixAccount_userPassword'])) {
			if ($_POST['form_posixAccount_userPassword'] != $_POST['form_posixAccount_userPassword2']) {
				$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
				unset ($_POST['form_posixAccount_userPassword2']);
				}
				else $this->userPassword($_POST['form_posixAccount_userPassword']);
			}
		if ($_POST['form_posixAccount_genpass']) $this->userPassword(genpasswd());

		// Check if Username contains only valid characters
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*$', $this->uid))
			$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));

		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// *** fixme make incache modularized. Incache will return the found attribute
		// Set username back to original name if new username is in use
		if (incache($this->uid,'uid', '*')!=$this->orig['uid'] && ($this->orig['uid']!='')) $this->uid = $this->orig['uid'];
		// Change uid to a new uid until a free uid is found
		while (incache($this->uid, 'uid', '*')) {
			// Remove "$" at end of hostname if type is host
			if ($this->base['type']=='host') $this->uid = substr($this->uid, 0, $this->uid-1);
			// get last character of username
			$lastchar = substr($this->uid, strlen($this->uid)-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				if ($this->base['type']=='host') $this->uid = $this->uid . '2$';
				 else $this->uid = $this->uid . '2';
			 else {
				/* Last character is a number -> we have to increase the number until we've
				* found a groupname with trailing number which is not in use.
				*
				* $i will show us were we have to split groupname so we get a part
				* with the groupname and a part with the trailing number
				*/
			 	$i=strlen($this->uid)-1;
				$mark = false;
				// Set $i to the last character which is a number in $account_new->general_username
			 	while (!$mark) {
					if (ereg('^([0-9])+$',substr($this->uid, $i, strlen($this->uid)-$i))) $i--;
						else $mark=true;
					}
				// increase last number with one
				$firstchars = substr($this->uid, 0, $i+1);
				$lastchars = substr($this->uid, $i+1, strlen($this->uid)-$i);
				// Put username together
				$this->uid = $firstchars . (intval($lastchars)+1);
				// Add $ name if type is host
				if ($this->base['type']=='host') $this->uid .= '$';
				}
			}
		// Show warning if lam has changed username
		if ($this->uid != $_POST['form_posixAccount_uid']) $errors[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));

		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		if ($this->base['type']=='user') {
			$minID = intval($_SESSION['config']->get_minUID());
			$maxID = intval($_SESSION['config']->get_maxUID());
			}
		else {
			$minID = intval($_SESSION['config']->get_minMachine());
			$maxID = intval($_SESSION['config']->get_maxMachine());
			}
		// *** fixme create getcache function
		$dn_uids = getcache('uidNumber', 'posixAccount', '*');
		// getcache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		foreach ($dn_uids as $uid) $uids[] = $uid[0];
		if(is_array($uids)) sort ($uids, SORT_NUMERIC);
		if ($this->uidNumber=='') {
			// No id-number given
			if ($this->orig['uidNumber']=='') {
				// new account -> we have to find a free id-number
				if (count($uids)!=0) {
					// There are some uids
					// Store highest id-number
					$id = $uids[count($uids)-1];
					// Return minimum allowed id-number if all found id-numbers are too low
					if ($id < $minID) $this->uidNumber = $minID;
					// Return higesht used id-number + 1 if it's still in valid range
					if ($id < $maxID) $this->uidNumber = $id+1;
					/* If this function is still running we have to fid a free id-number between
					* the used id-numbers
					*/
					$i = intval($minID);
					while (in_array($i, $uids)) $i++;
					if ($i>$maxID)
						$errors[] = array('ERROR', _('ID-Number'), _('No free ID-Number!')))));
						else {
							$this->uidNumber = $i;
							$errors[] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
							}
					}
				else $this->uidNumber = $minID;
				// return minimum allowed id-number if no id-numbers are found
				}
			else $this->uidNumber = $this->orig['uidNumber'];
			// old account -> return id-number which has been used
			}
		else {
			// Check manual ID
			// id-number is out of valid range
			if ( $this->uidNumber < $minID || $this->uidNumber > $maxID) $errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
			// $uids is allways an array but not if no entries were found
			if (is_array($uids)) {
				// id-number is in use and account is a new account
				if ((in_array($this->uidNumber, $uids)) && $this->orig['uidNumber']=='') $errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->uidNumber, $uids)) && $this->orig['uidNumber']!='' && ($this->orig['uidNumber'] != $this->uidNumber) ) {
					$errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					$this->uidNumber = $this->orig['uidNumber'];
					}
				}
			}

		// Check if Homedir is valid
		$this->homeDirectory = str_replace('$group', getgrnam($this->gidNumber), $this->homeDirectory);
		if ($this->uid != '')
			$this->homeDirectory = str_replace('$user', $this->uid, $this->homeDirectory);
		if ($this->homeDirectory != $_POST['form_posixAccount_homeDirectory']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
		if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $this->homeDirectory ))
			$errors[] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
		// Check if Name-length is OK. minLength=3, maxLength=20
		if ( !ereg('.{3,20}', $this->uid)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
		// Check if Name starts with letter
		if ( !ereg('^([a-z]|[A-Z]).*$', $this->uid))
			$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));
		// Check if password is OK
		if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $this->userPassword()))
			$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		if ($_POST['form_posixAccount_addgroup'])  return 'group';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function process_groups() {
		do { // X-Or, only one if() can be true
			if (isset($_POST['form_posixAccount_addgroups']) && isset($_POST['form_posixAccount_addgroups_button'])) { // Add groups to list
				// Add new group
				$this->groups = @array_merge($this->groups, $_POST['allgroups']);
				// remove doubles
				$this->groups = @array_flip($this->groups);
				array_unique($this->groups);
				$this->groups = @array_flip($this->groups);
				// sort groups
				sort($this->groups);
				break;
				}
			if (isset($_POST['form_posixAccount_removegroups']) && isset($_POST['form_posixAccount_removegroups_button'])) { // remove groups from list
				$this->groups = array_delete($_POST['form_posixAccount_removegroups'], $this->groups);
				break;
				}
			} while(0);
		if (isset($_POST['form_posixAccount_addgroups_button']) || isset($_POST['form_posixAccount_removegroups_button'])) return 'group';
		if ($_POST['form_posixAccount_toattributes'] return 'attributes';
		return 0;
		}


	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load attributes which are displayed
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->cn = $attr['cn'][0];
		$this->orig['cn'] = $attr['cn'][0];
		$this->uid = $attr['uid'][0];
		$this->orig['uid'] = $attr['uid'][0];
		$this->uidNumber = $attr['uidNumber'][0];
		$this->orig['uidNumber'] = $attr['uidNumber'][0];
		$this->gidNumber = $attr['gidNumber'][0];
		$this->orig['gidNumber'] = $attr['gidNumber'][0];
		$this->homeDirectory = $attr['homeDirectory'][0];
		$this->orig['homeDirectory'] = $attr['homeDirectory'][0];
		if (isset($attr['loginShell'][0])) {
			$this->loginShell = $attr['loginShell'][0];
			$this->orig['loginShell'] = $attr['loginShell'][0];
			}
		if (isset($attr['gecos'][0])) {
			$this->gecos = $attr['gecos'][0];
			$this->orig['gecos'] = $attr['gecos'][0];
			}
		if (isset($attr['description'][0])) {
			$this->gecos = $attr['description'][0];
			$this->orig['description'] = $attr['description'][0];
			}
		if (isset($attr['userPassword'][0])) {
			$this->orig['enc_userPassword'] = $attr['userPassword'][0];
			}
		$this->userPassword_lock=!pwd_is_enabled($attr['userPassword'][0]);
		// get all additional groupmemberships
		$dn_groups = getcache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			if (in_array($attr['uid'], $dn_groups[$DN]))
				$this->groups[] = substr($DN, 3, strpos($DN, ',')-1);
			}
		$this->orig['groups'] = $this->groups;
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
		/* Exmaples
		* Add new attribute
		* if ($this->cn!='' && $this->orig['cn']=='') $return[$this->base['dn']]['add']['cn'] = $this->cn;
		* Modify existing attribute
		* if ($this->cn!='' && $this->orig['cn']!='') $return[$this->base['dn']]['modify']['cn'] = $this->cn;
		* Remove existing attribute
		* if ($this->cn=='' && $this->orig['cn']!='') $return[$this->base['dn']]['remove']['cn'] = $this->cn;
		*/

		// Get list off all attributes
		$attributes = $this->orig;
		// Remove attributes which are not as easy to set
		unset ($attributes['enc_userPassword']);
		unset ($attributes['groups']);
		// Get list of all "easy" attributes
		$attr_names = array_keys($attributes);
		foreach ($attr_names as $attr_name) {
			if ($this->$attr_name!='' && $this->orig[$attr_name]=='') $return[$this->base['dn']]['add'][$attr_name] = $this->cn;
			if ($this->$attr_name!='' && $this->orig[$attr_name]!='') $return[$this->base['dn']]['modify'][$attr_name] = $this->cn;
			if ($this->$attr_name=='' && $this->orig[$attr_name]!='') $return[$this->base['dn']]['remove'][$attr_name] = $this->cn;
			}

		// Set unix password
		if ($this->orig['enc_userPassword']=='') {
			// New user or no old password set
			if ($this->userPassword_no) $return[$this->base['dn']]['modify']['userPassword'] = pwd_hash ('', !$this->userPassword_lock);
				else $return[$this->base['dn']]['modify']['userPassword'] = pwd_hash ($this->userPassword(), !$this->userPassword_lock);
			}
		else {
			if ($this->userPassword()!='' || $this->userPassword_no) {
				// Write new password
				if ($this->userPassword_no) $return[$this->base['dn']]['modify']['userPassword'] = pwd_hash ('', !$this->userPassword_lock);
					else $return[$this->base['dn']]['modify']['userPassword'] = pwd_hash ($this->userPassword(), !$this->userPassword_lock);
				}
			else { // No new password but old password
				// (un)lock password
				if ($this->userPassword_lock == pwd_is_enabled($this->orig['enc_userPassword'])) {
					// Split old password hash in {CRYPT} and password-hash
					$i = 0;
					while ($this->orig['enc_userPassword']{$i} != '}') $i++;
					$passwd = substr($this->orig['enc_userPassword'], $i+1 );
					$crypt = substr($this->orig['enc_userPassword'], 0, $i+1 );
					// remove trailing ! from password hash
					if ($passwd{0} == '!') $passwd = substr($passwd, 1);
					// Write new password
					if ($this->userPassword_lock) $return[$this->base['dn']]['modify']['userPassword'] = "$crypt!$passwd";
					else $return[$this->base['dn']]['modify']['userPassword'] = "$crypt$passwd";
					}
				}
			}
		// Set additional group memberships
		if (is_array($this->groups)) {
			// There are some additional groups defined
			if (is_array($this->orig['groups']) {
				//There are some old groups.
				$add = array_delete($this->orig['groups'], $this->groups);
				$remove = array_delete($this->groups, $this->orig['groups']);
				$dn_cns = getcache('cn', 'posixGroup', 'group');
				// getcache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
				$DNs = array_keys($dn_cns);
				foreach ($DNs as $DN) {
					if (in_array($dn_cns[$DN], $add)) $return[$DN]]['add']['memberUid'] = $this->uid;
					if (in_array($dn_cns[$DN], $remove)) $return[$DN]]['remove']['memberUid'] = $this->uid;
					}
				}
			else {
				// Add user to every group
				$dn_cns = getcache('cn', 'posixGroup', 'group');
				// getcache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
				$DNs = array_keys($dn_cns);
				foreach ($DNs as $DN) {
					if (in_array($dn_cns[$DN], $this->groups)) $return[$DN]]['add']['memberUid'] = $this->uid;
					}
				}
			}
		else {
			if (is_array($this->orig['groups'])) {
				//There are some old groups which have to be removed
				$dn_cns = getcache('cn', 'posixGroup', 'group');
				// getcache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
				$DNs = array_keys($dn_cns);
				foreach ($DNs as $DN) {
					if (in_array($dn_cns[$DN], $this->orig['groups'])) $return[$DN]]['remove']['memberUid'] = $this->uid;
					}
				}
			}
		}

	/* This function returns all ldap attributes
	* which are part of posixAccount and returns
	* also their values.
	*/
	function get_attributes() {
		if ($userPassword_no) $return['userPassword'] = '';
			else $return['userPassword'] = $this->userPassword();
		$return['cn'] = $this->cn;
		$return['uid'] = $this->uid;
		$return['uidNumber'] = $this->uidNumber;
		$return['gidNumber'] = $this->gidNumber;
		$return['homeDirectory'] = $this->homeDirectory;
		$return['loginShell'] = $this->loginShell;
		$return['gecos'] = $this->gecos;
		$return['description'] = $this->description;
		// Not really ldap attributes but return values may be required
		$return['groups'] = $this->groups;
		if ($userPassword_lock) $return['userPasswordLocked'] = true;
			else $return['userPasswordLocked'] = false;
		return $return;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes() {
		$groups = findgroups(); // list of all groupnames
		$shelllist = getshells(); // list of all valid shells

		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo '<td>' . _('Username') . "*</td>\n";
		echo "<td><input name=\"form_posixAccount_uid\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"$this->uid\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=400\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('UID number') . "</td>\n";
		echo "<td><input name=\"form_posixAccout_uidNumber\" type=\"text\" size=\"6\" maxlength=\"6\" value=\"$this->uidNumber\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=401\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Primary group') . "*</td>\n";
		echo "<td><select name=\"form_posixAccount_group\">";
		// loop trough existing groups
		foreach ($groups as $group)
			if (getgrnam($this->gidNumber) == $group) echo "<option selected> $group </option>\n";
				else echo "<option> $group </option>\n";
		echo "</select></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=406\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if ($this->base['type']=='user') {
			echo "<tr>\n";
			echo "<td>" . _('Additional groups') . "</td>\n";
			echo "<td><input name=\"form_posixAccount_addgroup\" type=\"submit\" value=\"" . _('Edit groups') . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=402\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home directory') . "*</td>\n";
			echo "<td><input name=\"form_posixAccount_homeDirectory\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"$this->homeDirectory\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=403\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}
		echo "<tr>\n";
		echo "<td>" . _('Gecos') . "</td>\n";
		echo "<td><input name=\"form_posixAccount_gecos\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"$this->gecos\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Description') . "</td>\n";
		echo "<td><input name=\"form_posixAccount_description\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"$this->description\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if ($this->base['type']=='user') {
			if (count($shelllist)!=0) {
				echo "<tr>\n";
				echo "<td>" . _('Login shell') . "*</td>\n";
				echo "<td><select name=\"form_posixAccount_loginShell\">";
				// loop through shells
				foreach ($shelllist as $shell)
					if ($this->loginShell==trim($shell)) echo "<option selected> $shell </option>\n";
							else echo "<option> $shell </option>\n";
				echo "</select></td>\n";
				echo "<td><a href=\"../help.php?HelpNumber=405\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
				echo "</tr>\n";
				}
			echo "<tr>\n";
			echo "<td>" . _('Password') . "</td>\n";
			echo "<td><input name=\"form_posixAccount_userPassword\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"$this->userPassword()\"></td>\n";
			echo "<td><input name=\"form_posixAccount_genpass\" type=\"submit\" value=\"" . _('Generate password') . "\"></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Repeat password') . "</td>\n";
			echo "<td><input name=\"form_posixAccount_userPassword2\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"";
				if (isset($_POST['form_posixAccount_userPassword2'])) echo $_POST['form_posixAccount_userPassword2'];
				else echo $this->userPassword();
			echo "\"></td>\n";
			echo "<td></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Use no password') . "</td>\n";
			echo "<td><input name=\"form_posixAccount_userPassword_no\" type=\"checkbox\"";
				if ($this->userPassword_no) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}
		echo "</table>\n";
		return 0;
		}

	function display_html_group() {
		// load list with all groups
		$dn_groups = getcache('uidNumber', 'posixGroup', 'group');
		foreach ($dn_groups as $group) $groups[] = $group[0];
		// sort groups
		sort($groups, SORT_STRING);
		// remove groups the user is member of from grouplist
		$groups = array_delete($this->groups, $groups);
		// *** fixme primary group mut also be removed if it has changed after setting additional groups
		// Remove primary group from grouplist
		$groups = array_flip($groups);
		if (isset($groups[getgrnam($this->gidNumber)])) unset ($groups[getgrnam($this->gidNumber)]);
		$groups = array_flip($groups);

		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<td><fieldset class=\"useredit-bright\">";
			echo "<legend class=\"useredit-bright\"><b>" . _("Additional groups") . "</b></legend>\n";
			echo "<table border=0 width=\"100%\">\n<tr>\n";
				echo "<td valign=\"top\">";
				echo "<fieldset class=\"useredit-bright\">";
				echo "<legend class=\"useredit-bright\">" . _("Selected groups") . "</legend>\n";
					// Show all groups the user is additional member of
					if (count($this->groups)!=0) {
						echo "<select name=\"form_posixAccount_removegroups[]\" class=\"useredit-bright\" size=15 multiple>\n";
						for ($i=0; $i<count($this->groups); $i++)
							if ($this->groups[$i]!='') echo "<option> $this->groups[$i] </option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"form_posixAccount_addgroups_button\" value=\"<=\">";
				echo " ";
				echo "<input type=\"submit\" name=\"form_posixAccount_removegroups_button\" value=\"=>\"><br><br>";
				echo "<a href=\""."../help.php?HelpNumber=402\" target=\"lamhelp\">"._('Help')."</a></td>\n";
				echo "<td valign=\"top\">\n";
				echo "<fieldset class=\"useredit-bright\">";
				echo "<legend class=\"useredit-bright\">" . _('Available groups') . "</legend>\n";
					// show all groups expect these the user is member of
					if (count($groups)!=0) {
						echo "<select name=\"form_posixAccount_addgroups[]\" size=15 multiple class=\"useredit-bright\">\n";
						for ($i=0; $i<count($groups); $i++)
							if ($groups[$i]!='') echo "<option> $groups[$i] </option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "</tr>\n";
				echo "</table>\n";
			echo "<input name=\"form_posixAccount_toattributes\" type=\"submit\" value=\""; echo _('Back'); echo "\">\n";
			echo "</fieldset>\n";
		echo "</td></tr></table>\n";
		return 0;
		}


	}



?>
