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
	function inetOrgPerson($base) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'inetOrgPerson\');'), E_USER_ERROR);
		$this->base = $base;
		// inetOrgPerson is only a valid objectClass for users
		if ($_SESSION[$this->base]->get_type() != 'user') trigger_error(_('inetOrgPerson can only be used for users.'), E_USER_WARNING);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('inetOrgPerson');
		$_SESSION[$this->base]->add_attributes ('inetOrgPerson');
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'inetOrgPerson';
		// unset userPassword because we handle it separat.
		$this->alias = _('inetOrgPerson');
		// Add attributes which should be cached
		$_SESSION[$_SESSION[$this->base]->cache]->add_cache(array ('user' => array('uid'), 'host' => array('uid')));
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
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$this->attributes['userPassword'][0] = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $newpassword, MCRYPT_MODE_ECB, $iv));
			return 0;
			}
		else {
			if ($this->attributes['userPassword'][0]!='') {
				// Read existing password if set
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->attributes['userPassword'][0]), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				return $password;
				}
			else return '';
			}
		}

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array('main');
		}

	function module_ready() {
		return true;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Load attributes
		if (($this->attributes['uid'][0] != $post['form_inetOrgPerson_uid']) &&  ereg('[A-Z]$', $post['form_inetOrgPerson_uid']))
			$errors[] = array('WARN', _('Username'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->attributes['uid'][0] = $post['form_inetOrgPerson_uid'];
		$this->attributes['cn'][0] = $this->attributes['uid'][0];
		$this->attributes['description'][0] = $post['form_inetOrgPerson_description'];
		$this->attributes['sn'][0] = $post['form_inetOrgPerson_sn'];
		$this->attributes['givenName'][0] = $post['form_inetOrgPerson_givenName'];
		$this->attributes['title'][0] = $post['form_inetOrgPerson_title'];
		$this->attributes['mail'][0] = $post['form_inetOrgPerson_mail'];
		$this->attributes['telephoneNumber'][0] = $post['form_inetOrgPerson_telephoneNumber'];
		$this->attributes['mobileTelephoneNumber'][0] = $post['form_inetOrgPerson_mobileTelephoneNumber'];
		$this->attributes['facsimileTelephoneNumber'][0] = $post['form_inetOrgPerson_facsimileTelephoneNumber'];
		$this->attributes['street'][0] = $post['form_inetOrgPerson_street'];
		$this->attributes['postalCode'][0] = $post['form_inetOrgPerson_postalCode'];
		$this->attributes['postalAddress'][0] = $post['form_inetOrgPerson_postalAddress'];
		$this->attributes['employeeType'][0] = $post['form_inetOrgPerson_employeeType'];

		if (isset($this->attributes['host'])) {
			$host = $post['form_inetOrgPerson_host'];
			if ((!$host=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $host))
				$errors[] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
			$hosts = explode(" ", $host);
			$this->attributes['host'] = array();
			foreach ($hosts as $host)
				if ($host!="") $this->attributes['host'][] = $host;
			}

		if ($post['form_inetOrgPerson_userPassword_no']) $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($post['form_inetOrgPerson_userPassword_lock']) $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if (isset($post['form_inetOrgPerson_userPassword'])) {
			if ($post['form_inetOrgPerson_userPassword'] != $post['form_inetOrgPerson_userPassword2']) {
				$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
				unset ($post['form_inetOrgPerson_userPassword2']);
				}
				else $this->userPassword($post['form_inetOrgPerson_userPassword']);
			}
		if ($post['form_inetOrgPerson_genpass']) $this->userPassword(genpasswd());

		// Check if givenname is valid
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['givenName'][0])) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
		// Check if surname is valid
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['sn'][0])) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
		// Check if Username contains only valid characters
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])+$', $this->attributes['uid'][0]))
			$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['uid'][0],'uid', '*')!=false && ($this->orig['uid'][0]!='')) {
			$this->attributes['uid'][0] = $this->orig['uid'][0];
			}
		// Change uid to a new uid until a free uid is found
		else while ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['uid'][0], 'uid', '*')) {
			// get last character of username
			$lastchar = substr($this->attributes['uid'][0], strlen($this->attributes['uid'][0])-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				$this->attributes['uid'][0] = $this->attributes['uid'][0] . '2';
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
				$this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1);
				}
			}
		// Show warning if lam has changed username
		if ($this->attributes['uid'][0] != $post['form_inetOrgPerson_uid']) $errors[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
		if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $this->userPassword()))
			$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));

		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['telephoneNumber'][0]))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['mobileTelephoneNumber'][0]))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['facsimileTelephoneNumber'][0]))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $this->attributes['mail'][0]))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['street'][0]))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['postalAddress'][0]))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['title'][0]))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['employeeType'][0]))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $this->attributes['personal_postalCode'][0]))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
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
			if (isset($this->orig[$attribute])) {
				$this->orig[$attribute] = $attr[$attribute];
				// decode as unicode
				for ($i=0; $i<count($this->orig[$attribute]); $i++) $this->orig[$attribute][$i] = utf8_decode ($this->orig[$attribute][$i]);
				}
			}
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->orig['objectClass'][0] = 'inetOrgPerson';
		$this->attributes = $this->orig;
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
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		// Set unix password
		if (count($this->orig['userPassword'])==0) {
			// New user or no old password set
			if ($this->userPassword_no) {
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
				}
				else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
			}
		else {
			if ($this->userPassword()!='' || $this->userPassword_no) {
				// Write new password
				if ($this->userPassword_no) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
					else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
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
	function display_html_attributes($post) {

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
			if ($post['form_inetOrgPerson_userPassword2']!='') echo $post['form_posixAccount_userPassword2'];
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
		echo "<td><input name=\"form_inetOrgPerson_userPassword_lock\" type=\"checkbox\"";
			if ($this->userPassword_lock) echo " checked ";
			echo "></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if (isset($this->attributes['host'])) {
			echo "<tr>\n";
			echo "<td>" . _('Unix workstations') . "</td>\n";
			echo "<td><input name=\"form_inetOrgPerson_host\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"";
			if (is_array($this->attributes['host']))
				foreach ($this->attributes['host'] as $host) echo $host." ";
			echo "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=466\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}
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
