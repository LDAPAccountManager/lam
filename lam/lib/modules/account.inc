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


/* This class contains all account LDAP attributes
* and funtioncs required to deal with account
* account can only be created when it should be added
* to an array.
* basearray is the same array account should be added
* to. If basearray is not given the constructor tries to
* create an array with account and all other required
* objects.
* Example: $user[] = new account($user);
*
*/

class account {
	// Constructor
	function account($base) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'account\');'), E_USER_ERROR);
		$this->base = $base;
		// account is only a valid objectClass for users
		if ($_SESSION[$this->base]->get_type() != 'host') trigger_error(_('account can only be used for hosts.'), E_USER_WARNING);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('account');
		$_SESSION[$this->base]->add_attributes ('account');
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'account';
		// unset userPassword because we handle it separat.
		$this->alias = _('account');
		}

	// Variables
	// Alias Name. This name is shown in the menu instead of posixAccount
	var $alias;
	// reference to base-array so we can read other classes in basearray
	var $base;
	// This variable contains all account attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;

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
		$this->attributes['description'][0] = $post['form_account_description'];

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
		$this->orig['objectClass'][0] = 'account';
		$this->attributes = $this->orig;
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

		return $return;
		}

	/* This function returns all ldap attributes
	* which are part of account and returns
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
		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Description') . "</td>\n";
		echo "<td><input name=\"form_account_description\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['description'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}


	}



?>
