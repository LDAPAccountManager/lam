<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2007  Roland Gruber

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

/**
* Manages the atrributes of object class inetOrgPerson.
*
* @package modules
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* This module manages LDAP attributes of the object class inetOrgPerson (e.g. name and address).
*
* @package modules
*/
class inetOrgPerson extends baseModule {

	/**
	* This function fills the message array.
	**/
	function load_Messages() {
		$this->messages['host'][0] = array('ERROR', _('Unix workstations'), _('Unix workstations are invalid!'));
		$this->messages['host'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_hosts', _('Unix workstations are invalid!'));
		$this->messages['givenName'][0] = array('ERROR', _('First name'), _('First name contains invalid characters!'));
		$this->messages['givenName'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_lastName', _('First name contains invalid characters!'));
		$this->messages['lastname'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters or is empty!'));
		$this->messages['lastname'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_firstName', _('Last name contains invalid characters or is empty!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_telephone', _('Please enter a valid telephone number!'));
		$this->messages['homePhone'][0] = array('ERROR', _('Home telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['homePhone'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_homePhone', _('Please enter a valid telephone number!'));
		$this->messages['mobileTelephone'][0] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		$this->messages['mobileTelephone'][1] = array('ERROR', _('Account %s:') . " inetOrgPerson_mobile", _('Please enter a valid mobile number!'));
		$this->messages['facsimileNumber'][0] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		$this->messages['facsimileNumber'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_fax', _('Please enter a valid fax number!'));
		$this->messages['email'][0] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		$this->messages['email'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_email', _('Please enter a valid eMail address!'));
		$this->messages['street'][0] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		$this->messages['street'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_street', _('Please enter a valid street name!'));
		$this->messages['postalAddress'][0] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		$this->messages['postalAddress'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_address', _('Please enter a valid postal address!'));
		$this->messages['postalCode'][0] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		$this->messages['postalCode'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_postalCode', _('Please enter a valid postal code!'));
		$this->messages['title'][0] = array('ERROR', _('Job title'), _('Please enter a valid job title!'));
		$this->messages['title'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_title', _('Please enter a valid job title!'));
		$this->messages['employeeType'][0] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		$this->messages['employeeType'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_type', _('Please enter a valid employee type!'));
		$this->messages['cn'][0] = array('ERROR', _('Common name'), _('Please enter a valid common name!'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_cn', _('Please enter a valid common name!'));
		$this->messages['uid'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][3] = array('ERROR', _('Account %s:') . ' inetOrgPerson_userName', _('User name already exists!'));
		$this->messages['manager'][0] = array('ERROR', _('Account %s:') . ' inetOrgPerson_manager', _('This is not a valid DN!'));
		$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password fields.'));
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['userPassword'][2] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['photo'][0] = array('ERROR', _('No file selected.'));
		$this->messages['businessCategory'][0] = array('ERROR', _('Business category'), _('Please enter a valid business category!'));
		$this->messages['businessCategory'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_businessCategory', _('Please enter a valid business category!'));
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
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "normal", 'uid' => 'low');
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=inetOrgPerson)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('inetOrgPerson');
		// LDAP aliases
		$return['LDAPaliases'] = array('commonName' => 'cn', 'surname' => 'sn', 'streetAddress' => 'street',
			'fax' => 'facsimileTelephoneNumber', 'gn' => 'givenName', 'userid' => 'uid', 'rfc822Mailbox' => 'mail',
			'mobileTelephoneNumber' => 'mobile');
		// managed attributes
		$return['attributes'] = array('uid', 'cn', 'employeeType', 'givenName', 'jpegPhoto', 'mail', 'manager', 'mobile',
			'host', 'title', 'telephoneNumber', 'facsimileTelephoneNumber', 'street', 'postOfficeBox', 'postalCode', 'postalAddress',
			'sn', 'userPassword', 'description', 'homePhone', 'roomNumber', 'businessCategory');
		// self service search attributes
		$return['selfServiceSearchAttributes'] = array('uid', 'mail', 'cn', 'surname', 'givenName');
		// self service field settings
		$return['selfServiceFieldSettings'] = array('firstName' => _('First name'), 'lastName' => _('Last name'),
			'mail' => _('eMail address'), 'telephoneNumber' => _('Telephone number'), 'mobile' => _('Mobile number'),
			'faxNumber' => _('Fax number'), 'street' => _('Street'), 'postalAddress' => _('Postal address'),
			'postalCode' => _('Postal code'), 'postOfficeBox' => _('Post office box'), 'jpegPhoto' => _('Photo'),
			'homePhone' => _('Home telephone number'), 'roomNumber' => _('Room number'));
		// profile elements
		$return['profile_options'] = array(
			array(
				array('kind' => 'text', 'text' => _('Job title') . ":"),
				array('kind' => 'input', 'name' => 'inetOrgPerson_title', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				array('kind' => 'help', 'value' => 'title')),
			array(
				array('kind' => 'text', 'text' => _('Employee type') . ":"),
				array('kind' => 'input', 'name' => 'inetOrgPerson_employeeType', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				array('kind' => 'help', 'value' => 'employeeType'))
		);
		// profile checks
		$return['profile_checks']['inetOrgPerson_title'] = array(
			'type' => 'ext_preg',
			'regex' => 'title',
			'error_message' => $this->messages['title'][0]);
		$return['profile_checks']['inetOrgPerson_employeeType'] = array(
			'type' => 'ext_preg',
			'regex' => 'employeeType',
			'error_message' => $this->messages['employeeType'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'inetOrgPerson_title' => 'title',
			'inetOrgPerson_employeeType' => 'employeeType'
		);
		// upload fields
		$return['upload_columns'] = array(
		array(
		'name' => 'inetOrgPerson_firstName',
		'description' => _('First name'),
		'help' => 'givenName',
		'example' => _('Steve')
		),
		array(
		'name' => 'inetOrgPerson_lastName',
		'description' => _('Last name'),
		'help' => 'sn',
		'example' => _('Miller'),
		'required' => true
		),
		array(
		'name' => 'inetOrgPerson_description',
		'description' => _('Description'),
		'help' => 'description',
		'example' => _('Temp, contract until december')
		),
		array(
		'name' => 'inetOrgPerson_title',
		'description' => _('Job title'),
		'help' => 'title',
		'example' => _('President')
		),
		array(
		'name' => 'inetOrgPerson_type',
		'description' => _('Employee type'),
		'help' => 'employeeType',
		'example' => _('Temp')
		),
		array(
		'name' => 'inetOrgPerson_manager',
		'description' => _('Manager'),
		'help' => 'manager',
		'example' => _('uid=smiller,ou=People,dc=company,dc=com')
		),
		array(
		'name' => 'inetOrgPerson_businessCategory',
		'description' => _('Business category'),
		'help' => 'businessCategory',
		'example' => _('Administration')
		),
		array(
		'name' => 'inetOrgPerson_street',
		'description' => _('Street'),
		'help' => 'street',
		'example' => _('Mystreetname 42')
		),
		array(
		'name' => 'inetOrgPerson_postalCode',
		'description' => _('Postal code'),
		'help' => 'postalCode',
		'example' => _('GB-12345')
		),
		array(
		'name' => 'inetOrgPerson_address',
		'description' => _('Postal address'),
		'help' => 'postalAddress',
		'example' => _('Mycity')
		),
		array(
		'name' => 'inetOrgPerson_postOfficeBox',
		'description' => _('Post office box'),
		'help' => 'postOfficeBox',
		'example' => _('12345')
		),
		array(
		'name' => 'inetOrgPerson_telephone',
		'description' => _('Telephone number'),
		'help' => 'telephoneNumber',
		'example' => _('123-123-1234')
		),
		array(
		'name' => 'inetOrgPerson_homePhone',
		'description' => _('Home telephone number'),
		'help' => 'homePhone',
		'example' => _('123-124-1234')
		),
		array(
		'name' => 'inetOrgPerson_mobile',
		'description' => _('Mobile number'),
		'help' => 'mobileTelephoneNumber',
		'example' => _('123-123-1235')
		),
		array(
		'name' => 'inetOrgPerson_fax',
		'description' => _('Fax number'),
		'help' => 'facsimileTelephoneNumber',
		'example' => _('123-123-1236')
		),
		array(
		'name' => 'inetOrgPerson_email',
		'description' => _('eMail address'),
		'help' => 'mail',
		'example' => _('user@company.com')
		),
		array(
		'name' => 'inetOrgPerson_roomNumber',
		'description' => _('Room number'),
		'help' => 'roomNumber',
		'example' => 'A 2.24'
		)
		);
		// cn and uid for upload (only if posixAccount is not loaded)
		if ($_SESSION['loggedIn']) {
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				$return['upload_columns'][] = array(
					'name' => 'inetOrgPerson_cn',
					'description' => _('Common name'),
					'help' => 'cn',
					'example' => _('Steve Miller'),
					'default' => '<i>&lt;inetOrgPerson_firstName&gt; &lt;inetOrgPerson_lastName&gt;</i>'
				);
				$return['upload_columns'][] = array(
					'name' => 'inetOrgPerson_userName',
					'description' => _('User name'),
					'help' => 'uid',
					'example' => _('smiller'),
					'unique' => true,
					'required' => true
				);
			}
		}
		// Unix workstations for upload
		if ($_SESSION['loggedIn'] && $_SESSION['ldap']->supports_unix_hosts) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_hosts',
				'description' => _('Unix workstations'),
				'help' => 'workstations',
				'example' => _('pc01,pc02')
				);
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'description',
			'host',
			'givenName',
			'sn',
			'title',
			'employeeType',
			'manager',
			'street',
			'postOfficeBox',
			'postalCode',
			'postalAddress',
			'telephoneNumber',
			'mobileTelephoneNumber',
			'facimilieTelephoneNumber',
			'mail',
			'homePhone',
			'roomNumber',
			'businessCategory'
		);
		if ($_SESSION['loggedIn']) {
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				$return['PDF_fields'][] = 'uid';
				$return['PDF_fields'][] = 'cn';
			}
		}
		// help Entries
		$return['help'] = array (
			'user' => array (
				'description' => array (
					"Headline" => _("Description"),
					"Text" => _("User description. If left empty sur- and give name will be used.")
				),
				'title' => array (
					"Headline" => _("Job title"),
					"Text" => _("Job title of user: President, department manager, ...")
				),
				'givenName' => array (
					"Headline" => _("First name"),
					"Text" => _("First name of user. Only letters, - and spaces are allowed.")
				),
				'sn' => array (
					"Headline" => _("Last name"),
					"Text" => _("Last name of user. Only letters, - and spaces are allowed.")
				),
				'employeeType' => array (
					"Headline" => _("Employee type"),
					"Text" => _("Employee type: Contractor, Employee, Intern, Temp, External, ...")
				),
				'manager' => array (
					"Headline" => _("Manager"),
					"Text" => _("This is the LDAP DN of the user's manager. Use this property to represent hierarchies in your company.")
				),
				'street' => array (
					"Headline" => _("Street"),
					"Text" => _("Street")
				),
				'postOfficeBox' => array (
					"Headline" => _("Post office box"),
					"Text" => _("Post office box")
				),
				'postalCode' => array (
					"Headline" => _("Postal code"),
					"Text" => _("Postal code")
				),
				'postalAddress' => array (
					"Headline" => _("Postal address"),
					"Text" => _("Postal address, city")
				),
				'telephoneNumber' => array (
					"Headline" => _("Telephone number"),
					"Text" => _("Telephone number")
				),
				'mobileTelephoneNumber' => array (
					"Headline" => _("Mobile number"),
					"Text" => _("Mobile number")
				),
				'facsimileTelephoneNumber' => array (
					"Headline" => _("Fax number"),
					"Text" => _("Fax number")
				),
				'mail' => array (
					"Headline" => _("eMail address"),
					"Text" => _("eMail address")
				),
				'cn' => array (
					"Headline" => _("Common name"),
					"Text" => _("This is the natural name of the user. If empty, the first and last name is used.")
				),
				'uid' => array(
					"Headline" => _("User name"),
					"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, .-_. LAM does not allow a number as first character because useradd also does not allow it. If user name is already used user name will be expanded with a number. The next free number will be used. Warning: Older systems have problems with user names longer than 8 characters. You can not log in to Windows if user name is longer than 16 characters.")
				),
				'workstations' => array (
					"Headline" => _("Unix workstations"),
					"Text" => _("Please enter a comma separated list of host names where this user is allowed to log in. If you enable host restrictions for your servers then \"*\" means every host and an empty field means no host.")
				),
				'userPassword' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
				),
				'photoUpload' => array(
					"Headline" => _("Add photo"),
					"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
				),
				'homePhone' => array(
					"Headline" => _("Home telephone number"),
					"Text" => _("Private telephone number")
				),
				'roomNumber' => array(
					"Headline" => _("Room number"),
					"Text" => _("The room number of the employee's office.")
				),
				'businessCategory' => array(
					"Headline" => _("Business category"),
					"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)")
				)
			)
		);

		return $return;
	}

	/**
	* This functions return true if all needed settings are done.
	*
	* @return boolean true, if all is ok
	*/
	function module_complete() {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('inetOrgPerson', $objectClasses)) {
				return true;
			}
		}
		if (!isset($this->attributes['sn'][0]) || ($this->attributes['sn'][0] == '')) return false;
		return true;
	}

	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('inetOrgPerson', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}

	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	*/
	function save_attributes() {
		// skip saving if account is based on another structural object class
		if (!$this->getAccountContainer()->isNewAccount && !in_array('inetOrgPerson', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return array();
		}
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// do not set password if posixAccount is active
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (in_array('posixAccount', $modules)) {
			if (isset($return[$this->getAccountContainer()->dn]['modify']['userPassword'])) {
				unset($return[$this->getAccountContainer()->dn]['modify']['userPassword']);
			}
			if (isset($return[$this->getAccountContainer()->dn]['add']['userPassword'])) {
				unset($return[$this->getAccountContainer()->dn]['add']['userPassword']);
			}
		}
		// postalAddress, facsimileTelephoneNumber and jpegPhoto need special removing
		if (isset($return[$this->getAccountContainer()->dn]['remove']['postalAddress'])) {
			$return[$this->getAccountContainer()->dn]['modify']['postalAddress'] = array();
			unset($return[$this->getAccountContainer()->dn]['remove']['postalAddress']);
		}
		if (isset($return[$this->getAccountContainer()->dn]['remove']['facsimileTelephoneNumber'])) {
			$return[$this->getAccountContainer()->dn]['modify']['facsimileTelephoneNumber'] = array();
			unset($return[$this->getAccountContainer()->dn]['remove']['facsimileTelephoneNumber']);
		}
		if (isset($return[$this->getAccountContainer()->dn]['remove']['jpegPhoto'])) {
			$return[$this->getAccountContainer()->dn]['modify']['jpegPhoto'] = array();
			unset($return[$this->getAccountContainer()->dn]['remove']['jpegPhoto']);
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// Load attributes
		$this->attributes['description'][0] = $_POST['description'];
		$this->attributes['sn'][0] = $_POST['sn'];
		$this->attributes['givenName'][0] = $_POST['givenName'];
		$this->attributes['title'][0] = $_POST['title'];
		$this->attributes['mail'][0] = $_POST['mail'];
		$this->attributes['telephoneNumber'][0] = $_POST['telephoneNumber'];
		$this->attributes['mobile'][0] = $_POST['mobileTelephoneNumber'];
		$this->attributes['facsimileTelephoneNumber'][0] = $_POST['facsimileTelephoneNumber'];
		$this->attributes['street'][0] = $_POST['street'];
		$this->attributes['postOfficeBox'][0] = $_POST['postOfficeBox'];
		$this->attributes['postalCode'][0] = $_POST['postalCode'];
		$this->attributes['postalAddress'][0] = $_POST['postalAddress'];
		$this->attributes['employeeType'][0] = $_POST['employeeType'];
		$this->attributes['homePhone'][0] = $_POST['homePhone'];
		$this->attributes['roomNumber'][0] = $_POST['roomNumber'];
		$this->attributes['businessCategory'][0] = $_POST['businessCategory'];
		if ($_POST['manager'] != '-') {
			$this->attributes['manager'][0] = $_POST['manager'];
		}
		else {
			unset($this->attributes['manager'][0]);
		}

		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			// uid
			if (isset($_POST['userName']) && ($_POST['userName'] != '')) {
				if (!get_preg($_POST['userName'], 'username')) {
					$errors[] = $this->messages['uid'][0];
				}
				else {
					$this->attributes['uid'][0] = $_POST['userName'];
				}
			}
			// cn
			if ($_POST['cn'] == '') {
				if ($_POST['givenName'] != '') {
					$this->attributes['cn'][0] = $_POST['givenName'] . " " . $_POST['sn'];
				}
				else {
					$this->attributes['cn'][0] = $_POST['sn'];
				}
			}
			else {
				$this->attributes['cn'][0] = $_POST['cn'];
			}
			if (!get_preg($this->attributes['cn'][0], 'cn')) {
				$errors[] = $this->messages['cn'][0];
			}
		}

		// handle host-attribute in on epice because it's not set by default
		if ($_SESSION['ldap']->supports_unix_hosts) {
			$host = $_POST['host'];
			if (!get_preg($host,'unixhost')) {
				$errors[] = $this->messages['host'][0];
			}
			$hosts = explode(",", $host);
			$this->attributes['host'] = array();
			for ($i = 0; $i < sizeof($hosts); $i++) {
				if ($hosts[$i] != "") {
					$this->attributes['host'][] = $hosts[$i];
				}
			}
		}
		// Do some regex-checks and return error if attributes are set to wrong values
		if (($this->attributes['givenName'][0] != '') && !get_preg($this->attributes['givenName'][0], 'realname')) $errors[] = $this->messages['givenName'][0];
		if ( !get_preg($this->attributes['sn'][0], 'realname')) $errors[] = $this->messages['lastname'][0];
		if ( !get_preg($this->attributes['telephoneNumber'][0], 'telephone')) $errors[] = $this->messages['telephoneNumber'][0];
		if ( !get_preg($this->attributes['homePhone'][0], 'telephone')) $errors[] = $this->messages['homePhone'][0];
		if ( !get_preg($this->attributes['mobile'][0], 'telephone')) $errors[] = $this->messages['mobileTelephone'][0];
		if ( !get_preg($this->attributes['facsimileTelephoneNumber'][0], 'telephone')) $errors[] = $this->messages['facsimileNumber'][0];
		if (($this->attributes['mail'][0] != '') && !get_preg($this->attributes['mail'][0], 'email')) $errors[] = $this->messages['email'][0];
		if ( !get_preg($this->attributes['street'][0], 'street')) $errors[] = $this->messages['street'][0];
		if ( !get_preg($this->attributes['postalAddress'][0], 'postalAddress')) $errors[] = $this->messages['postalAddress'][0];
		if ( !get_preg($this->attributes['postalCode'][0], 'postalCode')) $errors[] = $this->messages['postalCode'][0];
		if ( !get_preg($this->attributes['title'][0], 'title')) $errors[] = $this->messages['title'][0];
		if ( !get_preg($this->attributes['employeeType'][0], 'employeeType')) $errors[] = $this->messages['employeeType'][0];
		if ( !get_preg($this->attributes['businessCategory'][0], 'businessCategory')) $errors[] = $this->messages['businessCategory'][0];
		if ($_POST['userPassword']) {
			if ($_POST['userPassword'] != $_POST['userPassword2']) {
				$errors[] = $this->messages['userPassword'][0];
			}
			else {
				if (!get_preg($_POST['userPassword'], 'password')) {
					$errors[] = $this->messages['userPassword'][1];
				}
				else {
					$this->attributes['userPassword'][0] = $_POST['userPassword'];
				}
			}
		}
		if ($_POST['delPhoto']) $this->attributes['jpegPhoto'] = array();
		// Return error-messages
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			$uid = '';
			if (isset($this->attributes['uid'][0])) $uid = $this->attributes['uid'][0];
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User name')),
				array('kind' => 'input', 'name' => 'userName', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $uid),
				array('kind' => 'help', 'value' => 'uid'));
		}
		$firstName = '';
		if (isset($this->attributes['givenName'][0])) $firstName = $this->attributes['givenName'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('First name')),
			array('kind' => 'input', 'name' => 'givenName', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $firstName),
			array('kind' => 'help', 'value' => 'givenName'));
		$lastName = '';
		if (isset($this->attributes['sn'][0])) $lastName = $this->attributes['sn'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Last name').'*'),
			array('kind' => 'input', 'name' => 'sn', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $lastName),
			array ('kind' => 'help', 'value' => 'sn'));
		if (!in_array('posixAccount', $modules)) {
			$cn = '';
			if (isset($this->attributes['cn'][0])) $cn = $this->attributes['cn'][0];
			$return[] = array(
				array('kind' => 'text', 'text' => _('Common name') . '*'),
				array('kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $cn),
				array('kind' => 'help', 'value' => 'cn'));
		}
		$description = '';
		if (isset($this->attributes['description'][0])) $description = $this->attributes['description'][0];
		$return[] = array (
			array('kind' => 'text', 'text' => _('Description')),
			array('kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $description),
			array('kind' => 'help', 'value' => 'description'));

		$return[] = array(0 => array('kind' => 'text', 'td' => array('colspan' => 3)));

		// password
		if (!in_array('posixAccount', $modules)) {
			// new account, show input fields
			if ($this->getAccountContainer()->isNewAccount) {
				$return[] = array(
					array('kind' => 'text', 'text' => _('Password') ),
					array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
					array('kind' => 'help', 'value' => 'userPassword'));
				$return[] = array(
					array('kind' => 'text', 'text' => _('Repeat password')),
					array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
					array('kind' => 'text', 'text' => ''));
			}
			// old account, show button for password page
			else {
				$return[] = array(
					array('kind' => 'text', 'text' => _('Password') ),
					array('kind' => 'input', 'name' => 'form_subpage_' . get_class($this) . '_password_open', 'type' => 'submit', 'value' => _('Change password')));
			}

			$return[] = array(array('kind' => 'text', 'td' => array('colspan' => 3)));
		}

		$street = '';
		if (isset($this->attributes['street'][0])) $street = $this->attributes['street'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Street')),
			array('kind' => 'input', 'name' => 'street', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $street),
			array('kind' => 'help', 'value' => 'street'));
		$postOffice = '';
		if (isset($this->attributes['postOfficeBox'][0])) $postOffice = $this->attributes['postOfficeBox'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Post office box')),
			array('kind' => 'input', 'name' => 'postOfficeBox', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $postOffice),
			array('kind' => 'help', 'value' => 'postOfficeBox'));
		$postalCode = '';
		if (isset($this->attributes['postalCode'][0])) $postalCode = $this->attributes['postalCode'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Postal code')),
			array('kind' => 'input', 'name' => 'postalCode', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $postalCode),
			array('kind' => 'help', 'value' => 'postalCode'));
		$postalAddress = '';
		if (isset($this->attributes['postalAddress'][0])) $postalAddress = $this->attributes['postalAddress'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Postal address')),
			array('kind' => 'input', 'name' => 'postalAddress', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $postalAddress),
			array('kind' => 'help', 'value' => 'postalAddress'));
		$roomNumber = '';
		if (isset($this->attributes['roomNumber'][0])) $roomNumber = $this->attributes['roomNumber'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Room number')),
			array('kind' => 'input', 'name' => 'roomNumber', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $roomNumber),
			array('kind' => 'help', 'value' => 'roomNumber'));
		$telephone = '';
		if (isset($this->attributes['telephoneNumber'][0])) $telephone = $this->attributes['telephoneNumber'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Telephone number')),
			array('kind' => 'input', 'name' => 'telephoneNumber', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $telephone),
			array('kind' => 'help', 'value' => 'telephoneNumber'));
		$homePhone = '';
		if (isset($this->attributes['homePhone'][0])) $homePhone = $this->attributes['homePhone'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Home telephone number')),
			array('kind' => 'input', 'name' => 'homePhone', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $homePhone),
			array('kind' => 'help', 'value' => 'homePhone'));
		$mobile = '';
		if (isset($this->attributes['mobile'][0])) $mobile = $this->attributes['mobile'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Mobile number')),
			array('kind' => 'input', 'name' => 'mobileTelephoneNumber', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $mobile),
			array('kind' => 'help', 'value' => 'mobileTelephoneNumber'));
		$fax = '';
		if (isset($this->attributes['facsimileTelephoneNumber'][0])) $fax = $this->attributes['facsimileTelephoneNumber'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Fax number')),
			array('kind' => 'input', 'name' => 'facsimileTelephoneNumber', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $fax),
			array('kind' => 'help', 'value' => 'facsimileTelephoneNumber'));
		$email = '';
		if (isset($this->attributes['mail'][0])) $email = $this->attributes['mail'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('eMail address')),
			array('kind' => 'input', 'name' => 'mail', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $email),
			array('kind' => 'help', 'value' => 'mail'));

		$return[] = array(array('kind' => 'text', 'td' => array('colspan' => 3)));

		$title = '';
		if (isset($this->attributes['title'][0])) $title = $this->attributes['title'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Job title')),
			array('kind' => 'input', 'name' => 'title', 'type' => 'text', 'size' => '30',
				'value' => $title),
			array('kind' => 'help', 'value' => 'title'));
		$employeeType = '';
		if (isset($this->attributes['employeeType'][0])) $employeeType = $this->attributes['employeeType'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Employee type')),
			array('kind' => 'input', 'name' => 'employeeType', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $employeeType),
			array('kind' => 'help', 'value' => 'employeeType'));
		$businessCategory = '';
		if (isset($this->attributes['businessCategory'][0])) $businessCategory = $this->attributes['businessCategory'][0];
		$return[] = array(
			array('kind' => 'text', 'text' => _('Business category')),
			array('kind' => 'input', 'name' => 'businessCategory', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $businessCategory),
			array('kind' => 'help', 'value' => 'businessCategory'));
		if ($_SESSION['ldap']->supports_unix_hosts) {
			$hostvalue = "";
			if (is_array($this->attributes['host'])) {
				$hostvalue .= implode(",", $this->attributes['host']);
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Unix workstations')),
				array('kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $hostvalue ),
				array('kind' => 'help', 'value' => 'workstations'));
		}
		// get list of existing users for manager attribute
		$dnUsers = $_SESSION['cache']->get_cache('uid', 'inetOrgPerson', 'user');
		if (!is_array($dnUsers)) $dnUsers = array();
		$dnUsers = array_keys($dnUsers);
		usort($dnUsers, array($_SESSION['ldap'], 'cmp_array'));
		array_unshift($dnUsers, '-');
		$optionsSelected = array();
		if (isset($this->attributes['manager'][0])) {
			$optionsSelected[] = $this->attributes['manager'][0];
		}
		else {
			$optionsSelected[] = '-';
		}
		$return[] = array(
			array('kind' => 'text', 'text' => _('Manager')),
			array('kind' => 'select', 'name' => 'manager', 'size' => '1',
				'options' => $dnUsers, 'options_selected' => $optionsSelected),
			array('kind' => 'help', 'value' => 'manager'));
		// photo
		$photoFile = '../../graphics/userDefault.png';
		$noPhoto = true;
		if (isset($this->attributes['jpegPhoto'][0])) {
			$jpeg_filename = 'jpg' . $_SESSION['ldap']->new_rand() . '.jpg';
			$outjpeg = @fopen($_SESSION['lampath'] . 'tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $this->attributes['jpegPhoto'][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			$noPhoto = false;
		}
		$photo = array(array(
			array('kind' => 'image', 'alt' => _('Photo'), 'path' => $photoFile, 'td' => array('align' => 'center'))));
		if ($noPhoto) {
			$photo[] = array(array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_' . get_class($this) . '_photo_open', 'value' => _('Add photo')));
		}
		else {
			$photo[] = array(array('kind' => 'input', 'type' => 'submit', 'name' => 'delPhoto', 'value' => _('Delete photo')));
		}
		$return = array(array(
			array('kind' => 'table', 'value' => $return),
			array('kind' => 'table', 'value' => $photo, 'td' => array('valign' => 'top', 'align' => 'right','width' => '100%'))
			));
		return $return;
		}

	/**
	* Sets a new password.
	*/
	function process_password() {
		if ($_POST['form_subpage_' . get_class($this) . '_attributes_back']) return array();
		$messages = array();
		if ($_POST['userPassword'] != $_POST['userPassword2']) {
			$messages[] = $this->messages['userPassword'][0];
		}
		else {
			if (!get_preg($_POST['userPassword'], 'password')) {
				$messages[] = $this->messages['userPassword'][1];
			}
			else {
				$this->attributes['userPassword'][0] = $_POST['userPassword'];
			}
		}
		return $messages;
	}

	/**
	* Displays the password changing dialog.
	*
	* @return array meta HTML code
	*/
	function display_html_password() {
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Password') ),
			1 => array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'userPassword'));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Repeat password')),
			1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ""));
		$return[] = array(
			0 => array('kind' => 'table', 'value' => array(
				0 => array(
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Ok'), 'name' => 'form_subpage_' . get_class($this) . '_attributes_submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Cancel'), 'name' => 'form_subpage_' . get_class($this) . '_attributes_back'),
					2 => array('kind' => 'text')))));
		return $return;
	}

	/**
	* Sets a new photo.
	*/
	function process_photo() {
		if ($_POST['form_subpage_' . get_class($this) . '_attributes_back']) return array();
		$messages = array();
		if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 1000000);
			fclose($handle);
			$this->attributes['jpegPhoto'][0] = $data;
		}
		else {
			$messages[] = $this->messages['photo'][0];
		}
		return $messages;
	}

	/**
	* Displays the photo upload page.
	*
	* @return array meta HTML code
	*/
	function display_html_photo() {
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Photo file (JPG format)') ),
			1 => array('kind' => 'input', 'name' => 'photoFile', 'type' => 'file'),
			2 => array('kind' => 'help', 'value' => 'photoUpload'));
		$return[] = array(
			0 => array('kind' => 'table', 'value' => array(
				0 => array(
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Add photo'), 'name' => 'form_subpage_' . get_class($this) . '_attributes_submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_' . get_class($this) . '_attributes_back'),
					2 => array('kind' => 'text')))));
		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(
			get_class($this) . '_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
			get_class($this) . '_host' => array('<block><key>' . _('Unix workstations') . '</key><value>' . $this->attributes['host'][0] . '</value></block>'),
			get_class($this) . '_title' => array('<block><key>' . _('Job title') . '</key><value>' . $this->attributes['title'][0] . '</value></block>'),
			get_class($this) . '_givenName' => array('<block><key>' . _('First name') . '</key><value>' . $this->attributes['givenName'][0] . '</value></block>'),
			get_class($this) . '_sn' => array('<block><key>' . _('Last name') . '</key><value>' . $this->attributes['sn'][0] . '</value></block>'),
			get_class($this) . '_employeeType' => array('<block><key>' . _('Employee type') . '</key><value>' . $this->attributes['employeeType'][0] . '</value></block>'),
			get_class($this) . '_manager' => array('<block><key>' . _('Manager') . '</key><value>' . $this->attributes['manager'][0] . '</value></block>'),
			get_class($this) . '_street' => array('<block><key>' . _('Street') . '</key><value>' . $this->attributes['street'][0] . '</value></block>'),
			get_class($this) . '_postOfficeBox' => array('<block><key>' . _('Post office box') . '</key><value>' . $this->attributes['postOfficeBox'][0] . '</value></block>'),
			get_class($this) . '_postalCode' => array('<block><key>' . _('Postal code') . '</key><value>' . $this->attributes['postalCode'][0] . '</value></block>'),
			get_class($this) . '_postalAddress' => array('<block><key>' . _('Postal address') . '</key><value>' . $this->attributes['postalAddress'][0] . '</value></block>'),
			get_class($this) . '_telephoneNumber' => array('<block><key>' . _('Telephone number') . '</key><value>' . $this->attributes['telephoneNumber'][0] . '</value></block>'),
			get_class($this) . '_homePhone' => array('<block><key>' . _('Home telephone number') . '</key><value>' . $this->attributes['homePhone'][0] . '</value></block>'),
			get_class($this) . '_mobileTelephoneNumber' => array('<block><key>' . _('Mobile number') . '</key><value>' . $this->attributes['mobile'][0] . '</value></block>'),
			get_class($this) . '_facimileTelephoneNumber' => array('<block><key>' . _('Fax number') . '</key><value>' . $this->attributes['facsimileTelephoneNumber'][0] . '</value></block>'),
			get_class($this) . '_mail' => array('<block><key>' . _('eMail address') . '</key><value>' . $this->attributes['mail'][0] . '</value></block>'),
			get_class($this) . '_cn' => array('<block><key>' . _('Common name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			get_class($this) . '_roomNumber' => array('<block><key>' . _('Room number') . '</key><value>' . $this->attributes['roomNumber'][0] . '</value></block>'),
			get_class($this) . '_businessCategory' => array('<block><key>' . _('Business category') . '</key><value>' . $this->attributes['businessCategory'][0] . '</value></block>'),
			get_class($this) . '_uid' => array('<block><key>' . _('User name') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'));
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("inetOrgPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetOrgPerson";
			// last name
			if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_lastName']], 'realname')) {
				$partialAccounts[$i]['sn'] = $rawAccounts[$i][$ids['inetOrgPerson_lastName']];
			}
			else {
				$errMsg = $this->messages['lastname'][1];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// first name
			if ($rawAccounts[$i][$ids['inetOrgPerson_firstName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_firstName']], 'realname')) {
					$partialAccounts[$i]['givenName'] = $rawAccounts[$i][$ids['inetOrgPerson_firstName']];
				}
				else {
					$errMsg = $this->messages['givenName'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// description
			if ($rawAccounts[$i][$ids['inetOrgPerson_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['inetOrgPerson_description']];
			}
			else {
				if ($partialAccounts[$i]['givenName'] != "") {
					$partialAccounts[$i]['description'] = $partialAccounts[$i]['givenName'] . " " . $partialAccounts[$i]['sn'];
				}
				else {
					$partialAccounts[$i]['description'] = $partialAccounts[$i]['sn'];
				}
			}
			// title
			if ($rawAccounts[$i][$ids['inetOrgPerson_title']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_title']], 'title')) {
					$partialAccounts[$i]['title'] = $rawAccounts[$i][$ids['inetOrgPerson_title']];
				}
				else {
					$errMsg = $this->messages['title'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// employee type
			if ($rawAccounts[$i][$ids['inetOrgPerson_type']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_type']], 'employeeType')) {
					$partialAccounts[$i]['employeeType'] = $rawAccounts[$i][$ids['inetOrgPerson_type']];
				}
				else {
					$errMsg = $this->messages['employeeType'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// business category
			if ($rawAccounts[$i][$ids['inetOrgPerson_businessCategory']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_businessCategory']], 'businessCategory')) {
					$partialAccounts[$i]['businessCategory'] = $rawAccounts[$i][$ids['inetOrgPerson_businessCategory']];
				}
				else {
					$errMsg = $this->messages['businessCategory'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// manager
			if ($rawAccounts[$i][$ids['inetOrgPerson_manager']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_manager']], 'dn')) {
					$partialAccounts[$i]['manager'] = $rawAccounts[$i][$ids['inetOrgPerson_manager']];
				}
				else {
					$errMsg = $this->messages['manager'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// street
			if ($rawAccounts[$i][$ids['inetOrgPerson_street']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_street']], 'street')) {
					$partialAccounts[$i]['street'] = $rawAccounts[$i][$ids['inetOrgPerson_street']];
				}
				else {
					$errMsg = $this->messages['street'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// post office box
			if ($rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']] != "") {
				$partialAccounts[$i]['postOfficeBox'] = $rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']];
			}
			// room number
			if ($rawAccounts[$i][$ids['inetOrgPerson_roomNumber']] != "") {
				$partialAccounts[$i]['roomNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_roomNumber']];
			}
			// postal code
			if ($rawAccounts[$i][$ids['inetOrgPerson_postalCode']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_postalCode']], 'postalCode')) {
					$partialAccounts[$i]['postalCode'] = $rawAccounts[$i][$ids['inetOrgPerson_postalCode']];
				}
				else {
					$errMsg = $this->messages['postalCode'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// postal address
			if ($rawAccounts[$i][$ids['inetOrgPerson_address']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_address']], 'postalAddress')) {
					$partialAccounts[$i]['postalAddress'] = $rawAccounts[$i][$ids['inetOrgPerson_address']];
				}
				else {
					$errMsg = $this->messages['postalAddress'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// telephone
			if ($rawAccounts[$i][$ids['inetOrgPerson_telephone']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_telephone']], 'telephone')) {
					$partialAccounts[$i]['telephoneNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_telephone']];
				}
				else {
					$errMsg = $this->messages['telephoneNumber'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// home telephone
			if ($rawAccounts[$i][$ids['inetOrgPerson_homePhone']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_homePhone']], 'telephone')) {
					$partialAccounts[$i]['homePhone'] = $rawAccounts[$i][$ids['inetOrgPerson_homePhone']];
				}
				else {
					$errMsg = $this->messages['homePhone'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// mobile
			if ($rawAccounts[$i][$ids['inetOrgPerson_mobile']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_mobile']], 'telephone')) {
					$partialAccounts[$i]['mobile'] = $rawAccounts[$i][$ids['inetOrgPerson_mobile']];
				}
				else {
					$errMsg = $this->messages['mobileTelephone'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// facsimile
			if ($rawAccounts[$i][$ids['inetOrgPerson_fax']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_fax']], 'telephone')) {
					$partialAccounts[$i]['facsimileTelephoneNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_fax']];
				}
				else {
					$errMsg = $this->messages['facsimileNumber'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// eMail
			if ($rawAccounts[$i][$ids['inetOrgPerson_email']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_email']], 'email')) {
					$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['inetOrgPerson_email']];
				}
				else {
					$errMsg = $this->messages['email'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// Unix workstations
			if ($rawAccounts[$i][$ids['inetOrgPerson_hosts']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_hosts']], 'unixhost')) {
					$partialAccounts[$i]['host'] = explode(",", $rawAccounts[$i][$ids['inetOrgPerson_hosts']]);
				}
				else {
					$errMsg = $this->messages['host'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				// cn
				if ($rawAccounts[$i][$ids['inetOrgPerson_cn']] != "") {
					if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_cn']], 'cn')) {
						$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['inetOrgPerson_cn']];
					}
					else {
						$errMsg = $this->messages['cn'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					if ($partialAccounts[$i]['givenName'] != "") {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['givenName'] . " " . $partialAccounts[$i]['sn'];
					}
					else {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['sn'];
					}
				}
				// uid
				// get list of existing users
				$dnUsers = $_SESSION['cache']->get_cache('uid', 'inetOrgPerson', 'user');
				$existingUsers = array();
				foreach ($dnUsers as $dn) {
					$existingUsers[] = $dn[0];
				}
				if (in_array($rawAccounts[$i][$ids['inetOrgPerson_userName']], $existingUsers)) {
					$errMsg = $this->messages['uid'][3];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['inetOrgPerson_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['inetOrgPerson_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
		}
		return $errors;
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account (attribute names in lower case)
	 * @return array meta HTML
	 */
	function getSelfServiceOptions($fields, $attributes) {
		$return = array();
		if (in_array('firstName', $fields)) {
			$firstName = '';
			if (isset($attributes['givenName'][0])) $firstName = $attributes['givenName'][0];
			$return['firstName'] = array(
				0 => array('kind' => 'text', 'text' => _('First name')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_firstName', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $firstName));
		}
		if (in_array('lastName', $fields)) {
			$lastName = '';
			if (isset($attributes['sn'][0])) $lastName = $attributes['sn'][0];
			$return['lastName'] = array(
				0 => array('kind' => 'text', 'text' => _('Last name')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_lastName', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $lastName));
		}
		if (in_array('mail', $fields)) {
			$mail = '';
			if (isset($attributes['mail'][0])) $mail = $attributes['mail'][0];
			$return['mail'] = array(
				0 => array('kind' => 'text', 'text' => _('eMail address')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_mail', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $mail));
		}
		if (in_array('telephoneNumber', $fields)) {
			$telephoneNumber = '';
			if (isset($attributes['telephoneNumber'][0])) $telephoneNumber = $attributes['telephoneNumber'][0];
			$return['telephoneNumber'] = array(
				0 => array('kind' => 'text', 'text' => _('Telephone number')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_telephoneNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $telephoneNumber));
		}
		if (in_array('homePhone', $fields)) {
			$homePhone = '';
			if (isset($attributes['homePhone'][0])) $homePhone = $attributes['homePhone'][0];
			$return['homePhone'] = array(
				0 => array('kind' => 'text', 'text' => _('Home telephone number')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_homePhone', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $homePhone));
		}
		if (in_array('mobile', $fields)) {
			$mobile = '';
			if (isset($attributes['mobile'][0])) $mobile = $attributes['mobile'][0];
			$return['mobile'] = array(
				0 => array('kind' => 'text', 'text' => _('Mobile telephone number')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_mobile', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $mobile));
		}
		if (in_array('faxNumber', $fields)) {
			$faxNumber = '';
			if (isset($attributes['facsimileTelephoneNumber'][0])) $faxNumber = $attributes['facsimileTelephoneNumber'][0];
			$return['faxNumber'] = array(
				0 => array('kind' => 'text', 'text' => _('Fax number')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_faxNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $faxNumber));
		}
		if (in_array('street', $fields)) {
			$street = '';
			if (isset($attributes['street'][0])) $street = $attributes['street'][0];
			$return['street'] = array(
				0 => array('kind' => 'text', 'text' => _('Street')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_street', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $street));
		}
		if (in_array('postalAddress', $fields)) {
			$postalAddress = '';
			if (isset($attributes['postalAddress'][0])) $postalAddress = $attributes['postalAddress'][0];
			$return['postalAddress'] = array(
				0 => array('kind' => 'text', 'text' => _('Postal address')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_postalAddress', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $postalAddress));
		}
		if (in_array('postalCode', $fields)) {
			$postalCode = '';
			if (isset($attributes['postalCode'][0])) $postalCode = $attributes['postalCode'][0];
			$return['postalCode'] = array(
				0 => array('kind' => 'text', 'text' => _('Postal code')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_postalCode', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $postalCode));
		}
		if (in_array('postOfficeBox', $fields)) {
			$postOfficeBox = '';
			if (isset($attributes['postOfficeBox'][0])) $postOfficeBox = $attributes['postOfficeBox'][0];
			$return['postOfficeBox'] = array(
				0 => array('kind' => 'text', 'text' => _('Post office box')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_postOfficeBox', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $postOfficeBox));
		}
		if (in_array('roomNumber', $fields)) {
			$roomNumber = '';
			if (isset($attributes['roomNumber'][0])) $roomNumber = $attributes['roomNumber'][0];
			$return['roomNumber'] = array(
				0 => array('kind' => 'text', 'text' => _('Room number')),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_roomNumber', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $roomNumber));
		}
		if (in_array('jpegPhoto', $fields)) {
			if (isset($attributes['jpegPhoto'][0])) {
				$jpeg_filename = 'jpegPhoto' . session_id() . '.jpg';
				$outjpeg = fopen(realpath('../../') . '/tmp/' . $jpeg_filename, "wb");
				fwrite($outjpeg, $attributes['jpegPhoto'][0]);
				fclose ($outjpeg);
				$photoFile = '../../tmp/' . $jpeg_filename;
				$return['jpegPhoto'] = array(
					array('kind' => 'text', 'text' => _('Photo')),
					array('kind' => 'table', 'value' => array(
						array(
							array('kind' => 'image', 'alt' => _('Photo'), 'path' => $photoFile)
						),
						array(
							array('kind' => 'table', 'value' => array(array(
								array('kind' => 'input', 'type' => 'checkbox', 'name' => 'removeReplacePhoto'),
								array('kind' => 'text', 'text' => _('Remove/replace photo')),
								array('kind' => 'input', 'name' => 'replacePhotoFile', 'type' => 'file')
							))),
						)
					))
				);
			}
			else {
				$return['jpegPhoto'] = array(
					array('kind' => 'text', 'text' => _('Photo')),
					array('kind' => 'table', 'value' => array(array(
						array('kind' => 'text', 'text' => _('Add photo')),
						array('kind' => 'input', 'name' => 'photoFile', 'type' => 'file')
					)))
				);	
			}
		}
		return $return;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP commands which should be executed.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and LDAP commands (array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array());
		$attributeNames = array(); // list of attributes which should be checked for modification
		$attributesNew = $attributes;
		// first name
		if (in_array('firstName', $fields)) {
			$attributeNames[] = 'givenName';
			if (isset($_POST['inetOrgPerson_firstName']) && ($_POST['inetOrgPerson_firstName'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_firstName'], 'realname')) $return['messages'][] = $this->messages['givenName'][0];
				else $attributesNew['givenName'][0] = $_POST['inetOrgPerson_firstName'];
			}
			elseif (isset($attributes['givenName'])) unset($attributesNew['givenName']);
		}
		// last name
		if (in_array('lastName', $fields)) {
			$attributeNames[] = 'sn';
			if (isset($_POST['inetOrgPerson_lastName']) && ($_POST['inetOrgPerson_lastName'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_lastName'], 'realname')) $return['messages'][] = $this->messages['lastname'][0];
				else $attributesNew['sn'][0] = $_POST['inetOrgPerson_lastName'];
			}
			else {
				// last name is required attribute
				$return['messages'][] = $this->messages['lastname'][0];
			}
		}
		// email
		if (in_array('mail', $fields)) {
			$attributeNames[] = 'mail';
			if (isset($_POST['inetOrgPerson_mail']) && ($_POST['inetOrgPerson_mail'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_mail'], 'email')) $return['messages'][] = $this->messages['email'][0];
				else $attributesNew['mail'][0] = $_POST['inetOrgPerson_mail'];
			}
			elseif (isset($attributes['mail'])) unset($attributesNew['mail']);
		}
		// telephone number
		if (in_array('telephoneNumber', $fields)) {
			$attributeNames[] = 'telephoneNumber';
			if (isset($_POST['inetOrgPerson_telephoneNumber']) && ($_POST['inetOrgPerson_telephoneNumber'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_telephoneNumber'], 'telephone')) $return['messages'][] = $this->messages['telephoneNumber'][0];
				else $attributesNew['telephoneNumber'][0] = $_POST['inetOrgPerson_telephoneNumber'];
			}
			elseif (isset($attributes['telephoneNumber'])) unset($attributesNew['telephoneNumber']);
		}
		// home telephone number
		if (in_array('homePhone', $fields)) {
			$attributeNames[] = 'homePhone';
			if (isset($_POST['inetOrgPerson_homePhone']) && ($_POST['inetOrgPerson_homePhone'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_homePhone'], 'telephone')) $return['messages'][] = $this->messages['homePhone'][0];
				else $attributesNew['homePhone'][0] = $_POST['inetOrgPerson_homePhone'];
			}
			elseif (isset($attributes['homePhone'])) unset($attributesNew['homePhone']);
		}
		// fax number
		if (in_array('faxNumber', $fields)) {
			$attributeNames[] = 'facsimileTelephoneNumber';
			if (isset($_POST['inetOrgPerson_faxNumber']) && ($_POST['inetOrgPerson_faxNumber'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_faxNumber'], 'telephone')) $return['messages'][] = $this->messages['facsimileNumber'][0];
				else $attributesNew['facsimileTelephoneNumber'][0] = $_POST['inetOrgPerson_faxNumber'];
			}
			elseif (isset($attributes['facsimileTelephoneNumber'])) $attributesNew['facsimileTelephoneNumber'] = array();
		}
		// mobile telephone number
		if (in_array('mobile', $fields)) {
			$attributeNames[] = 'mobile';
			if (isset($_POST['inetOrgPerson_mobile']) && ($_POST['inetOrgPerson_mobile'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_mobile'], 'telephone')) $return['messages'][] = $this->messages['mobileTelephone'][0];
				else $attributesNew['mobile'][0] = $_POST['inetOrgPerson_mobile'];
			}
			elseif (isset($attributes['mobile'])) unset($attributesNew['mobile']);
		}
		// street
		if (in_array('street', $fields)) {
			$attributeNames[] = 'street';
			if (isset($_POST['inetOrgPerson_street']) && ($_POST['inetOrgPerson_street'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_street'], 'street')) $return['messages'][] = $this->messages['street'][0];
				else $attributesNew['street'][0] = $_POST['inetOrgPerson_street'];
			}
			elseif (isset($attributes['street'])) unset($attributesNew['street']);
		}
		// postal address
		if (in_array('postalAddress', $fields)) {
			$attributeNames[] = 'postalAddress';
			if (isset($_POST['inetOrgPerson_postalAddress']) && ($_POST['inetOrgPerson_postalAddress'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_postalAddress'], 'postalAddress')) $return['messages'][] = $this->messages['postalAddress'][0];
				else $attributesNew['postalAddress'][0] = $_POST['inetOrgPerson_postalAddress'];
			}
			elseif (isset($attributes['postalAddress'])) $attributesNew['postalAddress'] = array();
		}
		// postal code
		if (in_array('postalCode', $fields)) {
			$attributeNames[] = 'postalCode';
			if (isset($_POST['inetOrgPerson_postalCode']) && ($_POST['inetOrgPerson_postalCode'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_postalCode'], 'postalCode')) $return['messages'][] = $this->messages['postalCode'][0];
				else $attributesNew['postalCode'][0] = $_POST['inetOrgPerson_postalCode'];
			}
			elseif (isset($attributes['postalCode'])) unset($attributesNew['postalCode']);
		}
		// post office box
		if (in_array('postOfficeBox', $fields)) {
			$attributeNames[] = 'postOfficeBox';
			if (isset($_POST['inetOrgPerson_postOfficeBox']) && ($_POST['inetOrgPerson_postOfficeBox'] != '')) {
				$attributesNew['postOfficeBox'][0] = $_POST['inetOrgPerson_postOfficeBox'];
			}
			elseif (isset($attributes['postOfficeBox'])) unset($attributesNew['postOfficeBox']);
		}
		// room number
		if (in_array('roomNumber', $fields)) {
			$attributeNames[] = 'roomNumber';
			if (isset($_POST['inetOrgPerson_roomNumber']) && ($_POST['inetOrgPerson_roomNumber'] != '')) {
				$attributesNew['roomNumber'][0] = $_POST['inetOrgPerson_roomNumber'];
			}
			elseif (isset($attributes['roomNumber'])) unset($attributesNew['roomNumber']);
		}
		// photo
		if (in_array('jpegPhoto', $fields)) {
			if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
				$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
				$data = fread($handle, 1000000);
				fclose($handle);
				$return['add']['jpegPhoto'][0] = $data;
			}
			if ($_POST['removeReplacePhoto'] == 'on') {
				if ($_FILES['replacePhotoFile'] && ($_FILES['replacePhotoFile']['size'] > 0)) {
					$handle = fopen($_FILES['replacePhotoFile']['tmp_name'], "r");
					$data = fread($handle, 1000000);
					fclose($handle);
					$return['mod']['jpegPhoto'][0] = $data;
				}
				else {
					$return['mod']['jpegPhoto'] = array();
				}
			}
		}
		// find differences
		for ($i = 0; $i < sizeof($attributeNames); $i++) {
			$attrName = $attributeNames[$i];
			if (isset($attributes[$attrName]) && !isset($attributesNew[$attrName])) $return['del'][$attrName] = $attributes[$attrName];
			elseif (!isset($attributes[$attrName]) && isset($attributesNew[$attrName])) $return['add'][$attrName] = $attributesNew[$attrName];
			else {
				for ($a = 0; $a < sizeof($attributes[$attrName]); $a++) {
					if (!in_array($attributes[$attrName][$a], $attributesNew[$attrName])) {
						$return['mod'][$attrName] = $attributesNew[$attrName];
						break;
					}
				}
			}
		}
		return $return;
	}


}

?>
