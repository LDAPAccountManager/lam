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

class inetOrgPerson extends baseModule {

	/**
	* Creates a new inetOrgPerson object.
	*/
	function inetOrgPerson($scope) {
	// error messages for input checks
	$this->messages['host'] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
	$this->messages['givenName'] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
	$this->messages['surname'] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
	$this->messages['telephoneNumber'] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
	$this->messages['mobileTelephone'] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
	$this->messages['facsimileNumber'] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
	$this->messages['email'] = $errors['mail'] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
	$this->messages['street'] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
	$this->messages['postalAddress'] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
	$this->messages['postalCode'] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
	$this->messages['title'] = array('ERROR', _('Title'), _('Please enter a valid title!'));
	$this->messages['employeeType'] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
	// call parent constructor
	parent::baseModule($scope);
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _('Personal');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array('account'));
		// profile elements
		$return['profile_options'] = array(
			array(
				0 => array('kind' => 'text', 'text' => _('Job title') . ":"),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_title', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'TODO')),
			array(
				0 => array('kind' => 'text', 'text' => _('Employee type') . ":"),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_employeeType', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'TODO'))
		);
		// profile checks
		$return['profile_checks']['inetOrgPerson_title'] = array('type' => 'regex_i', 'regex' => $this->regex_title,
			'error_message' => $this->messages['title']);
		$return['profile_checks']['inetOrgPerson_employeeType'] = array('type' => 'regex_i',
			'regex' => $this->regex_employeeType, 'error_message' => $this->messages['employeeType']);
		return $return;
	}

	function init($base) {
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
	
	var $regex_host = '^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$';
	var $regex_givenName = '^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$';
	var $regex_surname = '^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$';
	var $regex_telephoneNumber = '^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/]|[-])*$';
	var $regex_email = '^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$';
	var $regex_street = '^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$';
	var $regex_postalAddress = '^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$';
	var $regex_postalCode = '^([0-9]|[A-Z]|[a-z])*$';
	var $regex_title = '^([0-9]|[A-Z]|[a-z]|[-]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$';
	var $regex_employeeType = '^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$';
	
	var $messages = array();

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
			if ((!$host=='') && !ereg($this->regex_host, $host))
				$errors['host'][] = $this->messages['host'];
			$hosts = explode(" ", $host);
			$this->attributes['host'] = array();
			foreach ($hosts as $host)
				if ($host!="") $this->attributes['host'][] = $host;
			}

		// Do some regex-checks and return error if attributes are set to wrong values
		if (!$profile) {
			if ( !ereg($this->regex_givenName, $this->attributes['givenName'][0])) $errors['givenName'][] = $this->messages['givenName'];
			if ( !ereg($this->regex_surname, $this->attributes['sn'][0])) $errors['sn'][] = $this->messages['surname'];
			if ( !ereg($this->regex_telephoneNumber, $this->attributes['telephoneNumber'][0]))  $errors['telephoneNumber'][] = $this->messages['telephoneNumber'];
			if ( !ereg($this->regex_telephoneNumber, $this->attributes['mobileTelephoneNumber'][0]))  $errors['mobileTelephoneNumber'][] = $this->messages['mobileTelephone'];
			if ( !ereg($this->regex_telephoneNumber, $this->attributes['facsimileTelephoneNumber'][0]))  $errors['facsimileTelephoneNumber'][] = $this->messages['facsimileNumber'];
			if ( !ereg($this->regex_email, $this->attributes['mail'][0])) $this->messages['email'];
			if ( !ereg($this->regex_street, $this->attributes['street'][0]))  $errors['street'][] = $this->messages['street'];
			if ( !ereg($this->regex_postalAddress, $this->attributes['postalAddress'][0]))  $errors['postalAdress'][] = $this->messages['postalAddress'];
			if ( !ereg($this->regex_postalCode, $this->attributes['personal_postalCode'][0]))  $errors['personal_postalCode'][] = $this->messages['postalCode'];
			}
		if ( !ereg($this->regex_title, $this->attributes['title'][0]))  $errors['title'][] = $this->messages['title'];
		if ( !ereg($this->regex_employeeType, $this->attributes['employeeType'][0]))  $errors['employeeType'][] = $this->messages['employeeType'];
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
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Job title') ),
			1 => array ( 'kind' => 'input', 'name' => 'title', 'type' => 'text', 'size' => '10',
				'value' => $this->attributes['title'][0] ),
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

	function get_pdfFields($account_type = "user") {
		return array(	'description',
							'host',
							'title',
							'givenName',
							'sn',
							'employeeType',
							'street',
							'postalCode',
							'postalAddress',
							'telephoneNumber',
							'mobileTelephoneNumber',
							'facimilieTelefonNumber',
							'mail');
	}
	
	function get_pdfEntries($account_type = "user") {
		return array(	'inetOrgPerson_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
							'inetOrgPerson_host' => array('<block><key>' . _('Unix workstations') . '</key><value>' . $this->attributes['host'][0] . '</value></block>'),
							'inetOrgPerson_title' => array('<block><key>' . _('Title') . '</key><value>' . $this->attributes['title'][0] . '</value></block>'),
							'inetOrgPerson_givenName' => array('<block><key>' . _('First name') . '</key><value>' . $this->attributes['givenName'][0] . '</value></block>'),
							'inetOrgPerson_sn' => array('<block><key>' . _('Last name') . '</key><value>' . $this->attributes['sn'][0] . '</value></block>'),
							'inetOrgPerson_employeeType' => array('<block><key>' . _('Employee type') . '</key><value>' . $this->attributes['employeeType'][0]),
							'inetOrgPerson_street' => array('<block><key>' . _('Street') . '</key><value>' . $this->attributes['street'] . '</value></block>'),
							'inetOrgPerson_postalCode' => array('<block><key>' . _('Postal code') . '</key><value>' . $this->attributes['postalCode'][0] . '</value></block>'),
							'inetOrgPerson_postalAddress' => array('<block><key>' . _('Postal address') . '</key><value>' . $this->attributes['postalAddress'][0] . '</value></block>'),
							'inetOrgPerson_telephoneNumber' => array('<block><key>' . _('Telephone number') . '</key><value>' . $this->attributes['telephoneNumber'][0] . '</value></block>'),
							'inetOrgPerson_mobileTelephoneNumber' => array('<block><key>' . _('Mobile number') . '</key><value>' . $this->attributes['mobileTelephoneNumber'][0] . '</value></block>'),
							'inetOrgPerson_facimileTelefonNumber' => array('<block><key>' . _('Fax number') . '</key><value>' . $this->attributes['facsimileTelephoneNumber'][0] . '</value></block>'),
							'inetOrgPerson_mail' => array('<block><key>' . _('eMail address') . '</key><value>' . $this->attributes['mail'][0] . '</value></block>'));
	}

}

?>
