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
* account.inc:
*/


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
	function inetOrgPerson(&$baseobject) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$baseobject) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		// Check if $baseobject is an array
		if (!is_object($baseobject)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'inetOrgPerson\');'), E_USER_ERROR);
		// posixAccount is only a valid objectClass for user and host
		if ($baseobject->get_type() != 'user') trigger_error(_('inetOrgPerson can only be used for users.'), E_USER_WARNING);
		/* Create a reference to basearray so we can read all other modules
		* php will avaois recousrion itself
		*/
		$this->base = &$baseobject;
		// Add attributes which should be cached
		//$_SESSION['cache']->add_cache(array ('user' => array('cn', 'uid'), 'host' => array('cn', 'uid') ));

		// Add account type to object
		$line=-1;
		for ($i=0; $i<count($this->base->ldap->objectClasses) || $i==-1; $i++) {
			if (strpos($this->base->ldap->objectClasses[$i], "NAME 'inetOrgPerson'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in ldap."), 'inetOrgPerson'), E_USER_WARNING);
		// Add Array with all attributes and type
		$baseobject->add_attributes ('inetOrgPerson');
		// create array with must-attributes
		// Get startposition in string
		if (strpos($this->base->ldap->objectClasses[$line],  'MUST (')) {
			$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'MUST (')+6);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Ad must
			foreach (explode(" $ ", $string) as $attribute) {
				$this->attributes[$attribute] = '';
				}
			}
		// create array with may-attributes
		// Get startposition in string
		if (strpos($this->base->ldap->objectClasses[$line],  'MAY (')) {
			$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'MAY (')+5);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Ad may
			foreach (explode(" $ ", $string) as $attribute) {
				$this->attributes[$attribute] = '';
				}
			}
		// Get attributes of subclasses
		while (strpos($this->base->ldap->objectClasses[$line], "SUP ")) {
			$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'SUP ')+4);
			$subclass = substr($string_withtail, 0, strpos($string_withtail, ' '));
			// Add account type to object
			for ($i=0; $i<count($this->base->ldap->objectClasses) || $i==-1; $i++) {
				if (strpos($this->base->ldap->objectClasses[$i], "NAME '$subclass'")) $line = $i;
				}
			// Return error if objectClass isn't found
			// *** fixme, fix error message
			if ($line==-1) trigger_error (_("objectClass objectClass required but not defined in ldap."), E_USER_WARNING);

			// create array with must-attributes
			// Get startposition in string
			if (strpos($this->base->ldap->objectClasses[$line],  'MUST (')) {
				$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Ad must
				foreach (explode(" $ ", $string) as $attribute) {
					$this->attributes[$attribute] = '';
					}
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($this->base->ldap->objectClasses[$line],  'MAY (')) {
				$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Ad may
				foreach (explode(" $ ", $string) as $attribute) {
					$this->attributes[$attribute] = '';
					}
				}
			}
		$this->orig = $this->attributes ;
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
	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;
	/* $attribute['password'] can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	*/
	/* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function userPassword($newpassword=false) {
		// Read existing password if set
		if (!$newpassword) {
			if ($this->attributes['userPassword'][0]!='') {
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->attributes['userPassword'][0]), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				return $password;
				}
			else return '';
			}
		// Write new password
		else {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$this->attributes['userPassword'][0] = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $newpassword, MCRYPT_MODE_ECB, $iv));
			return 0;
			}
		}

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array('main');
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes() {
		// Load attributes
		if (($this->attributes['uid'] != $_POST['form_inetOrgPerson_uid']) &&  ereg('[A-Z]$', $_POST['form_inetOrgPerson_uid']))
			$errors[] = array('WARN', _('Username'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->attributes['uid'] = $_POST['form_inetOrgPerson_uid'];
		$this->attributes['cn'] &= $this->attributes['uid'];
		$this->attributes['description'] = $_POST['form_inetOrgPerson_description'];
		$this->attributes['sn'] = $_POST['form_inetOrgPerson_sn'];
		$this->attributes['givenName'] = $_POST['form_inetOrgPerson_givenName'];
		$this->attributes['title'] = $_POST['form_inetOrgPerson_title'];
		$this->attributes['mail'] = $_POST['form_inetOrgPerson_mail'];
		$this->attributes['telephoneNumber'] = $_POST['form_inetOrgPerson_telephoneNumber'];
		$this->attributes['mobileTelephoneNumber'] = $_POST['form_inetOrgPerson_mobileTelephoneNumber'];
		$this->attributes['facsimileTelephoneNumber'] = $_POST['form_inetOrgPerson_facsimileTelephoneNumber'];
		$this->attributes['street'] = $_POST['form_inetOrgPerson_street'];
		$this->attributes['postalCode'] = $_POST['form_inetOrgPerson_postalCode'];
		$this->attributes['postalAddress'] = $_POST['form_inetOrgPerson_postalAddress'];
		$this->attributes['employeeType'] = $_POST['form_inetOrgPerson_employeeType'];

		if ($_POST['form_inetOrgPerson_userPassword_no']) $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($_POST['form_inetOrgPerson_userPassword_lock']) $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if (isset($_POST['form_inetOrgPerson_userPassword'])) {
			if ($_POST['form_inetOrgPerson_userPassword'] != $_POST['form_inetOrgPerson_userPassword2']) {
				$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
				unset ($_POST['form_inetOrgPerson_userPassword2']);
				}
				else $this->userPassword($_POST['form_inetOrgPerson_userPassword']);
			}
		if ($_POST['form_inetOrgPerson_genpass']) $this->userPassword(genpasswd());

		// Check if givenname is valid
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['givenName'])) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
		// Check if surname is valid
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['sn'])) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
		// Check if Username contains only valid characters
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*$', $this->attributes['uid']))
			$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ($this->base->cache->in_cache($this->attributes['uid'],'uid', '*')!=$this->orig['uid'] && ($this->orig['uid']!='')) $this->attributes['uid'] = $this->orig['uid'];
		// Change uid to a new uid until a free uid is found
		while ($this->base->cache->in_cache($this->attributes['uid'], 'uid', '*')) {
			// get last character of username
			$lastchar = substr($this->attributes['uid'], strlen($this->attributes['uid'])-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				$this->attributes['uid'] = $this->attributes['uid'] . '2';
			 else {
				/* Last character is a number -> we have to increase the number until we've
				* found a groupname with trailing number which is not in use.
				*
				* $i will show us were we have to split groupname so we get a part
				* with the groupname and a part with the trailing number
				*/
			 	$i=strlen($this->attributes['uid'])-1;
				$mark = false;
				// Set $i to the last character which is a number in $account_new->general_username
			 	while (!$mark) {
					if (ereg('^([0-9])+$',substr($this->attributes['uid'], $i, strlen($this->attributes['uid'])-$i))) $i--;
						else $mark=true;
					}
				// increase last number with one
				$firstchars = substr($this->attributes['uid'], 0, $i+1);
				$lastchars = substr($this->attributes['uid'], $i+1, strlen($this->attributes['uid'])-$i);
				// Put username together
				$this->attributes['uid'] = $firstchars . (intval($lastchars)+1);
				}
			}
		// Show warning if lam has changed username
		if ($this->attributes['uid'] != $_POST['form_inetOrgPerson_uid']) $errors[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
		if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $this->userPassword()))
			$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));

		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['telephoneNumber']))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['mobileTelephoneNumber']))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['facsimileTelephoneNumber']))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $this->attributes['mail']))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['street']))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['postalAddress']))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['title']))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['employeeType']))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $this->attributes['personal_postalCode']))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		// Return error-messages
		if (is_array($errors)) return $errors;
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
		$this->orig = $this->attributes;
		$this->userPassword(''); // Remove old password so it won't displayed as hash
		return 0;
		}



	/* This function returns an array with 3 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	*/
	function save_attributes() {
		// Get list off all attributes
		$attributes = $this->orig;
		// Remove attributes which are not as easy to set
		unset ($attributes['userPassword']);
		// Get list of all "easy" attributes
		$attr_names = array_keys($attributes);
		foreach ($attr_names as $attr_name) {
			if (count($this->attributes[$attr_name])!=0 && count($this->orig[$attr_name])==0) $return[$this->base['dn']]['add'][$attr_name] = $this->attributes[$attr_name];
			if (count($this->attributes[$attr_name])!=0 && count($this->orig[$attr_name])!=0) {
				// We have to check every single attribute
				// Get attributes which should be added
				$attributes = array_delete($this->orig[$attr_name], $this->attributes[$attr_name]);
				// Encode as unicode
				for ($i=0; $i<count($attributes); $i++) $attributes[$i] = utf8_encode ($attributes[$i]);
				if (count($attributes)!=0) $return[$this->base['dn']]['add'][$attr_name] = $attributes;
				// Get attributes which should be removed
				$attributes = array_delete($this->attributes[$attr_name], $this->orig[$attr_name]);
				// Encode as unicode
				for ($i=0; $i<count($attributes); $i++) $attributes[$i] = utf8_encode ($attributes[$i]);
				if (count($attributes)!=0) $return[$this->base['dn']]['remove'][$attr_name] = $attributes;
				}
			if (count($this->attributes[$attr_name])==0 && count($this->orig[$attr_name])!=0) $return[$this->base['dn']]['remove'][$attr_name] = $this->orig[$attr_name];
			}

		// Set unix password
		if (count($this->orig['userPassword'])==0) {
			// New user or no old password set
			if ($this->userPassword_no) {
				$return[$this->base['dn']]['add']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
				}
				else $return[$this->base['dn']]['add']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
			}
		else {
			if ($this->userPassword()!='' || $this->userPassword_no) {
				// Write new password
				$return[$this->base['dn']]['remove']['userPassword'] = utf8_encode($this->orig['userPassword']);
				if ($this->userPassword_no) $return[$this->base['dn']]['modify']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
					else $return[$this->base['dn']]['modify']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
				}
			else { // No new password but old password
				// (un)lock password
				if ($this->userPassword_lock == pwd_is_enabled($this->orig['userPassword'])) {
					// Split old password hash in {CRYPT} and password-hash
					$i = 0;
					while ($this->orig['userPassword']{$i} != '}') $i++;
					$passwd = substr($this->orig['userPassword'], $i+1 );
					$crypt = substr($this->orig['userPassword'], 0, $i+1 );
					// remove trailing ! from password hash
					if ($passwd{0} == '!') $passwd = substr($passwd, 1);
					// Write new password
					$return[$this->base['dn']]['remove']['userPassword'] = utf8_encode($this->orig['userPassword']);
					if ($this->userPassword_lock) $return[$this->base['dn']]['modify']['userPassword'][0] = utf8_encode("$crypt!$passwd");
					else $return[$this->base['dn']]['modify']['userPassword'][0] = utf8_encode("$crypt$passwd");
					}
				}
			}
		return $return;
		}

	/* This function returns all ldap attributes
	* which are part of inetOrgPerson and returns
	* also their values.
	*/
	function get_attributes() {
		$return = $this->attributes;
		$return['userPassword'] = $this->userPassword();
		return $return;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes() {

		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo '<td>' . _('Username') . "*</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_uid\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"".$this->attributes['uid'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=400\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Description') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_description\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['description'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Password') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_userPassword\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"".$this->userPassword()."\"></td>\n";
		echo "<td><input name=\"form_inetOrgPerson_genpass\" type=\"submit\" value=\"" . _('Generate password') . "\"></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Repeat password') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_userPassword2\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"";
			if (isset($_POST['form_inetOrgPerson_userPassword2'])) echo $_POST['form_posixAccount_userPassword2'];
			else echo $this->userPassword();
		echo "\"></td>\n";
		echo "<td></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Use no password') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_userPassword_no\" type=\"checkbox\"";
			if ($this->userPassword_no) echo " checked ";
			echo "></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Lock password') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_userPassword_no\" type=\"checkbox\"";
			if ($this->userPassword_no) echo " checked ";
			echo "></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Title') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_title\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"".$this->attributes['title'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=448\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('First name') . "*</td>\n" ;
		echo "<td><input name=\"form_inetOrgPerson_givenName\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"".$this->attributes['givenName'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=425\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Last name') . "*</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_sn\" type=\"text\" size=\"20\" maxlength=\"50\" value=\"".$this->attributes['sn'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=424\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Employee type') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_employeeType\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['employeeType'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=449\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Street') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_street\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$this->attributes['street'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=450\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Postal code') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_postalCode\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"".$this->attributes['postalCode'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=451\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Postal address') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_postalAddress\" type=\"text\" size=\"30\" maxlength=\"80\" value=\"".$this->attributes['postalAddress'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=452\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Telephone number') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_telephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['telephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=453\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Mobile number') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_mobileTelephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['mobileTelephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=454\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Fax number') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_facsimileTelephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['facsimileTelephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=455\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('eMail address') . "</td>\n";
		echo "<td><input name=\"form_inetOrgPerson_mail\" type=\"text\" size=\"30\" maxlength=\"80\" value=\"".$this->attributes['mail'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=456\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}


	}



?>
