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
* type: Type of account. Can be user, group, host, domain
* attributes: List of all attributes, how to get them and are theiy required or optional
* dn: current DN without uid= or cn=
* dn_orig: old DN if account was loaded with uid= or cn=
*/


/* This class contains all account LDAP attributes
* and funtioncs required to deal with inetOrgPerson
* inetOrgPerson can only be created when it should be added
* to an array.
* $base is the name of account_container in session
*/

class inetOrgPerson {
	// Constructor
	function inetOrgPerson($base) {
		// Get local copy of name of account_container in session
		$this->base = $base;
		// Do some error checks
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'inetOrgPerson\');'), E_USER_ERROR);
		if ($_SESSION[$this->base]->type != 'user') trigger_error(_('inetOrgPerson can only be used for users.'), E_USER_WARNING);
		// load attribtues which are used in inetOrgPerson objectClass
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('inetOrgPerson');
		// Create copy of attributes
		$this->orig = $this->attributes ;
		// Add objectClass to attributes
		$this->attributes['objectClass'][0] = 'inetOrgPerson';
		}

	// Variables
	// name of account_container in session so we can read other classes in account_container
	var $base;
	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;

	function get_alias() {
		return _('inetOrgPerson');
		}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		if ($scope=='user') return array('require' => array('main'), 'conflict' => array('account', 'posixGroup', 'sambaDomain') );
		return -1;
		}

	/* This function returns true if all required attributes from other
	* modules are set. This is required to prevent undefined states
	*/
	function module_ready() {
		return true;
		}

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes');
		}

	/* This function returns all ldap attributes
	* which are part of inetOrgPerson and returns
	* also their values.
	*/
	function get_attributes() {
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
		// Add objectClass to orig because we don't want to add objectClass if it's already set
		$this->orig['objectClass'][0] = 'inetOrgPerson';
		return 0;
		}



	/* This function returns an array with 4 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr), 'lamdaemon' => array(cmds)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* lamdaemon are lamdaemon commands to modify homedir, quotas, ...
	*/
	function save_attributes() {
		// Get easy attributes
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// unset password. First we hanlde userPassword with posixAccount, second we hanlde it completly separat
		// because it en/decrypted in session
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		// Return attributes
		return $return;
		}
	/* Write variables into object and do some regexp checks
	*/

	function delete_attributes($post) {
		$return = array();
		return $return;
		}

	function proccess_attributes($post) {
		// Load attributes
		$this->attributes['description'][0] = $post['description'];
		$this->attributes['sn'][0] = $post['sn'];
		$this->attributes['givenName'][0] = $post['givenName'];
		$this->attributes['title'][0] = $post['title'];
		$this->attributes['mail'][0] = $post['mail'];
		$this->attributes['telephoneNumber'][0] = $post['telephoneNumber'];
		$this->attributes['mobileTelephoneNumber'][0] = $post['mobileTelephoneNumber'];
		$this->attributes['facsimileTelephoneNumber'][0] = $post['facsimileTelephoneNumber'];
		$this->attributes['street'][0] = $post['street'];
		$this->attributes['postalCode'][0] = $post['postalCode'];
		$this->attributes['postalAddress'][0] = $post['postalAddress'];
		$this->attributes['employeeType'][0] = $post['employeeType'];

		// handle host-attribute in on epice because it's not set by default
		if (isset($this->attributes['host'])) {
			$host = $post['host'];
			if ((!$host=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $host))
				$errors[] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'), 'host');
			$hosts = explode(" ", $host);
			$this->attributes['host'] = array();
			foreach ($hosts as $host)
				if ($host!="") $this->attributes['host'][] = $host;
			}

		// Do some regex-checks and return error if attributes are set to wrong values
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['givenName'][0])) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'), 'givenName');
		if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['sn'][0])) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'), 'sn');
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['telephoneNumber'][0]))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'), 'telephoneNumber');
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['mobileTelephoneNumber'][0]))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'), 'mobileTelephoneNumber');
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['facsimileTelephoneNumber'][0]))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'), 'facsimileTelephoneNumber');
		if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $this->attributes['mail'][0]))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'), 'mail');
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['street'][0]))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'), 'street');
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['postalAddress'][0]))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'), 'postalAdress');
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['title'][0]))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'), 'title');
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['employeeType'][0]))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'), 'employeeType');
		if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $this->attributes['personal_postalCode'][0]))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'), 'personal_postalCode');
		// Return error-messages
		if (is_array($errors)) return $errors;
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo "<td>" . _('Description') . "</td>\n";
		echo "<td><input name=\"description\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['description'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if (isset($this->attributes['host'])) {
			echo "<tr>\n";
			echo "<td>" . _('Unix workstations') . "</td>\n";
			echo "<td><input name=\"host\" type=\"text\" size=\"20\" maxlength=\"80\" value=\"";
			if (is_array($this->attributes['host']))
				foreach ($this->attributes['host'] as $host) echo $host." ";
			echo "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=466\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}
		echo "<tr>\n";
		echo "<td>" . _('Title') . "</td>\n";
		echo "<td><input name=\"title\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"".$this->attributes['title'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=448\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('First name') . "*</td>\n" ;
		echo "<td><input name=\"givenName\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"".$this->attributes['givenName'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=425\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Last name') . "*</td>\n";
		echo "<td><input name=\"sn\" type=\"text\" size=\"20\" maxlength=\"50\" value=\"".$this->attributes['sn'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=424\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Employee type') . "</td>\n";
		echo "<td><input name=\"employeeType\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['employeeType'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=449\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Street') . "</td>\n";
		echo "<td><input name=\"street\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$this->attributes['street'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=450\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Postal code') . "</td>\n";
		echo "<td><input name=\"postalCode\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"".$this->attributes['postalCode'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=451\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Postal address') . "</td>\n";
		echo "<td><input name=\"postalAddress\" type=\"text\" size=\"30\" maxlength=\"80\" value=\"".$this->attributes['postalAddress'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=452\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Telephone number') . "</td>\n";
		echo "<td><input name=\"telephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['telephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=453\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Mobile number') . "</td>\n";
		echo "<td><input name=\"mobileTelephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['mobileTelephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=454\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Fax number') . "</td>\n";
		echo "<td><input name=\"facsimileTelephoneNumber\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"".$this->attributes['facsimileTelephoneNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=455\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('eMail address') . "</td>\n";
		echo "<td><input name=\"mail\" type=\"text\" size=\"30\" maxlength=\"80\" value=\"".$this->attributes['mail'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=456\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}

	function display_html_delete($post) {
		return 0;
		}
	}

?>
