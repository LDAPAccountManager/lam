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
	function inetOrgPerson(&$basearray) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$basearray) trigger_error(_('Please create a new object with $array[] = new posixAccount($array);'), E_USER_ERROR);
		// Check if $basearray is an array
		if (!is_object($basearray)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'inetOrgPerson\');'), E_USER_ERROR);
		// posixAccount is only a valid objectClass for user and host
		if ($basearray->get_type() != 'user') trigger_error(_('inetOrgPerson can only be used for users.'), E_USER_WARNING);
		/* Create a reference to basearray so we can read all other modules
		* php will avaois recousrion itself
		*/
		$this->base = &$basearray;
		// Add attributes which should be cached
		$_SESSION['cache']->add_cache(array ('user' => array('cn', 'uid'), 'host' => array('cn', 'uid') ));
		// Add Array with all attributes and type
		$basearray->add_attributes ('inetOrgPerson');

		// Add account type to object
		$line=-1;
		for ($i=0; $i<count($this->base->ldap->objectClasses) || $i==-1; $i++) {
			if (strpos($this->base->ldap->objectClasses[$i], "NAME 'inetOrgPerson'")) $line = $i;
			}
		// Return error if objectClass isn't found
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
		// Get attributes of subclasses
		while (strpos($this->base->ldap->objectClasses[$line], "SUP ")) {
			$string_withtail = substr($this->base->ldap->objectClasses[$line], strpos($this->base->ldap->objectClasses[$line],  'SUP ')+4);
			$subclass = substr($string_withtail, 0, strpos($string_withtail, ' '));
			// Add account type to object
			for ($i=0; $i<count($this->base->ldap->objectClasses) || $i==-1; $i++) {
				if (strpos($this->base->ldap->objectClasses[$i], "NAME '$subclass'")) $line = $i;
				}
			// Return error if objectClass isn't found
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
		$this->attributes = $this->orig;
		$this->alias = _('inetOrgPerson');
		}

	// Variables
	// Alias Name. This name is shown in the menu instead of posixAccount
	var $alias;
	// reference to base-array so we can read other classes in basearray
	var $base;
	// This variable contains all inetOrgPerson Attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;
	/* $attribute['password'] can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	*/


	/* This function returns a list with all required modules
	*/
	function dependencies() {
		// return error if unsupported type is used
		return array();
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes() {
		// Load attributes
		$this->attributes['uid'] = $_POST['form_inetOrgPerson_uid'];
		$this->attributes['cn'] &= $this->attributes['cn'];

		// Check if Username contains only valid characters
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*$', $this->attributes['uid']))
			$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));

		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// *** fixme make incache modularized. Incache will return the found attribute
		// Set username back to original name if new username is in use
		if (incache($this->attributes['uid'],'uid', '*')!=$this->orig['uid'] && ($this->orig['uid']!='')) $this->attributes['uid'] = $this->orig['uid'];
		// Change uid to a new uid until a free uid is found
		while (incache($this->attributes['uid'], 'uid', '*')) {
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


		// Return error-messages
		if (is_array($errors)) return $errors;
		return 0;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load attributes which are displayed
		// Values are kept as copy so we can compare old attributes with new attributes
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
