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
		if (!is_string($base)) trigger_error(_('Please create a new module in an accountContainer object first.'), E_USER_ERROR);
		if ($_SESSION[$this->base]->type != 'user') trigger_error(_('inetOrgPerson can only be used for users.'), E_USER_WARNING);
		// load attribtues which are used in inetOrgPerson objectClass
		// unset attributes which are "must" but not in this module
		// cn will be set to uid in module posixAccount
		// Therefore module posixAccount is required an cn will be removed from main index in account_container
		// Create copy of attributes
		$this->orig = $_SESSION[$this->base]->get_module_attributes('inetOrgPerson');
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('inetOrgPerson');
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

	function get_alias($scope) {
		return _('inetOrgPerson');
	}

	function can_manage($scope) {
		if ($scope == "user") return true;
		else return false;
	}

	function is_base_module($scope) {
		return false;
	}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		if ($scope=='user') return array('require' => array('main', 'posixAccount'), 'conflict' => array('account', 'posixGroup', 'sambaDomain') );
		return -1;
		}

	/* This function returns true if all required attributes from other
	* modules are set. This is required to prevent undefined states
	*/
	function module_ready() {
		return true;
		}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['sn'][0] == '') return false;
		if ($this->attributes['givenName'][0] == '') return false;
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
		return 0;
		}

	function proccess_attributes($post, $profile=false) {
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
				$errors['host'][] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
			$hosts = explode(" ", $host);
			$this->attributes['host'] = array();
			foreach ($hosts as $host)
				if ($host!="") $this->attributes['host'][] = $host;
			}

		// Do some regex-checks and return error if attributes are set to wrong values
		if (!$profile) {
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['givenName'][0])) $errors['givenName'][] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $this->attributes['sn'][0])) $errors['sn'][] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['telephoneNumber'][0]))  $errors['telephoneNumber'][] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['mobileTelephoneNumber'][0]))  $errors['mobileTelephoneNumber'][] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$', $this->attributes['facsimileTelephoneNumber'][0]))  $errors['facsimileTelephoneNumber'][] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
			if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $this->attributes['mail'][0]))  $errors['mail'] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['street'][0]))  $errors['street'][] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['postalAddress'][0]))  $errors['postalAdress'][] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $this->attributes['personal_postalCode'][0]))  $errors['personal_postalCode'][] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
			}
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['title'][0]))  $errors['title'][] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $this->attributes['employeeType'][0]))  $errors['employeeType'][] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		// Return error-messages
		if (is_array($errors)) return $errors;
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));
		if (isset($this->attributes['host'])) {
			if (is_array($this->attributes['host']))
				foreach ($this->attributes['host'] as $host) $hostvalue .= $host." ";
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Unix workstations') ),
				1 => array ( 'kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '20',
					'maxlength' => '255', 'value' => $hostvalues ),
				2 => array ('kind' => 'help', 'value' => 'host'));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Title') ),
			1 => array ( 'kind' => 'input', 'name' => 'title', 'type' => 'text', 'size' => '10',
				'maxlength' => '10', 'value' => $this->attributes['title'][0] ),
			2 => array ('kind' => 'help', 'value' => 'title'));
		if (!$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('First name').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'givenName', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['givenName'][0] ),
				2 => array ('kind' => 'help', 'value' => 'givenName'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Last name').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'sn', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['sn'][0] ),
				2 => array ('kind' => 'help', 'value' => 'sn'));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Employee type') ),
			1 => array ( 'kind' => 'input', 'name' => 'employeeType', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['employeeType'][0] ),
			2 => array ('kind' => 'help', 'value' => 'employeeType'));
		if (!$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Street') ),
				1 => array ( 'kind' => 'input', 'name' => 'street', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['street'][0] ),
				2 => array ('kind' => 'help', 'value' => 'street'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Postal code') ),
				1 => array ( 'kind' => 'input', 'name' => 'postalCode', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['postalCode'][0] ),
				2 => array ('kind' => 'help', 'value' => 'postalCode'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Postal address') ),
				1 => array ( 'kind' => 'input', 'name' => 'postalAddress', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['postalAddress'][0] ),
				2 => array ('kind' => 'help', 'value' => 'postalAddress'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Telephone number') ),
				1 => array ( 'kind' => 'input', 'name' => 'telephoneNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['telephoneNumber'][0] ),
				2 => array ('kind' => 'help', 'value' => 'telephoneNumber'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Mobile number') ),
				1 => array ( 'kind' => 'input', 'name' => 'mobileTelephoneNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['mobileTelephoneNumber'][0] ),
				2 => array ('kind' => 'help', 'value' => 'mobileTelephoneNumber'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Fax number') ),
				1 => array ( 'kind' => 'input', 'name' => 'facsimileTelephoneNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['facsimileTelephoneNumber'][0] ),
				2 => array ('kind' => 'help', 'value' => 'facsimileTelephoneNumber'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('eMail address') ),
				1 => array ( 'kind' => 'input', 'name' => 'mail', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['mail'][0] ),
				2 => array ('kind' => 'help', 'value' => 'mail'));
			}
		return $return;
		}

	function display_html_delete($post, $profile=false) {
		return 0;
		}

	function get_profileOptions($scope) {
		if ($scope == "user") return array();
	}

	// checks if the values of a new or modified profile are valid
	// $scope: the account type (user, group, host, ...)
	// $options: a hash array (name => value) containing the options
	function check_profileOptions($scope, $options) {
		return array();
	}

}

?>
