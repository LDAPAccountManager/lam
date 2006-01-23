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

/**
* Manages the atrributes of object class inetOrgPerson.
*
* @package modules
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* This class contains all account LDAP attributes
* and funtioncs required to deal with inetOrgPerson.
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
		$this->messages['lastname'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters!'));
		$this->messages['lastname'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_firstName', _('Last name contains invalid characters!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_telephone', _('Please enter a valid telephone number!'));
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
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['userPassword'][2] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['photo'][0] = array('ERROR', _('No file selected.'));
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
		$return["RDN"] = array("cn" => "low");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=inetOrgPerson)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// profile elements
		$return['profile_options'] = array(
			array(
				0 => array('kind' => 'text', 'text' => _('Job title') . ":"),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_title', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'title')),
			array(
				0 => array('kind' => 'text', 'text' => _('Employee type') . ":"),
				1 => array('kind' => 'input', 'name' => 'inetOrgPerson_employeeType', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'employeeType'))
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
		'example' => 'uid=smiller,ou=People,dc=company,dc=com'
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
		'example' => '12345'
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
		'example' => '12345'
		),
		array(
		'name' => 'inetOrgPerson_telephone',
		'description' => _('Telephone number'),
		'help' => 'telephoneNumber',
		'example' => '123-123-1234'
		),
		array(
		'name' => 'inetOrgPerson_mobile',
		'description' => _('Mobile number'),
		'help' => 'mobileTelephoneNumber',
		'example' => '123-123-1234'
		),
		array(
		'name' => 'inetOrgPerson_fax',
		'description' => _('Fax number'),
		'help' => 'facsimileTelephoneNumber',
		'example' => '123-123-1234'
		),
		array(
		'name' => 'inetOrgPerson_email',
		'description' => _('eMail address'),
		'help' => 'mail',
		'example' => _('user@company.com')
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
			'mail'
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
					"Text" => _("User name of the user who should be created. Valid characters are: a-z,0-9, .-_. LAM does not allow a number as first character because useradd also does not allow it. LAM does not allow capital letters A-Z because it can cause several problems. If user name is already used user name will be expanded with a number. The next free number will be used. Warning: Older systems have problems with user names longer than 8 characters. You can not log in to Windows if user name is longer than 16 characters.")
				),
				'workstations' => array (
					"Headline" => _("Unix workstations"),
					"Text" => _("Please enter a comma separated list of host names where this user is allowed to log in. Can be left empty.")
				),
				'userPassword' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
				),
				'photoUpload' => array(
					"Headline" => _("Add photo"),
					"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
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
		if (!$_SESSION[$this->base]->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $_SESSION[$this->base]->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('inetOrgPerson', $objectClasses)) {
				return true;
			}
		}
		if ($this->attributes['sn'][0] == '') return false;
		return true;
	}	
		
	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		if (!$_SESSION[$this->base]->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $_SESSION[$this->base]->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('inetOrgPerson', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
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
		// skip saving if account is based on another structural object class
		if (!$_SESSION[$this->base]->isNewAccount && !in_array('inetOrgPerson', $_SESSION[$this->base]->attributes_orig['objectClass'])) {
			return array();
		}
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// do not set password if posixAccount is active
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (in_array('posixAccount', $modules)) {
			if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword'])) {
				unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']); 	 
			}
			if (isset($return[$_SESSION[$this->base]->dn]['add']['userPassword'])) {
				unset($return[$_SESSION[$this->base]->dn]['add']['userPassword']);
			}
		}
		// postalAddress, facsimileTelephoneNumber and jpegPhoto need special removing
		if (isset($return[$_SESSION[$this->base]->dn]['remove']['postalAddress'])) {
			$return[$_SESSION[$this->base]->dn]['modify']['postalAddress'] = array();
			unset($return[$_SESSION[$this->base]->dn]['remove']['postalAddress']);
		}
		if (isset($return[$_SESSION[$this->base]->dn]['remove']['facsimileTelephoneNumber'])) {
			$return[$_SESSION[$this->base]->dn]['modify']['facsimileTelephoneNumber'] = array();
			unset($return[$_SESSION[$this->base]->dn]['remove']['facsimileTelephoneNumber']);
		}
		if (isset($return[$_SESSION[$this->base]->dn]['remove']['jpegPhoto'])) {
			$return[$_SESSION[$this->base]->dn]['modify']['jpegPhoto'] = array();
			unset($return[$_SESSION[$this->base]->dn]['remove']['jpegPhoto']);
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		$triggered_messages = array();
		// Load attributes
		$this->attributes['description'][0] = $post['description'];
		$this->attributes['sn'][0] = $post['sn'];
		$this->attributes['givenName'][0] = $post['givenName'];
		$this->attributes['title'][0] = $post['title'];
		$this->attributes['mail'][0] = $post['mail'];
		$this->attributes['telephoneNumber'][0] = $post['telephoneNumber'];
		$this->attributes['mobile'][0] = $post['mobileTelephoneNumber'];
		$this->attributes['facsimileTelephoneNumber'][0] = $post['facsimileTelephoneNumber'];
		$this->attributes['street'][0] = $post['street'];
		$this->attributes['postOfficeBox'][0] = $post['postOfficeBox'];
		$this->attributes['postalCode'][0] = $post['postalCode'];
		$this->attributes['postalAddress'][0] = $post['postalAddress'];
		$this->attributes['employeeType'][0] = $post['employeeType'];
		if ($post['manager'] != '-') {
			$this->attributes['manager'][0] = $post['manager'];
		}
		else {
			unset($this->attributes['manager'][0]);
		}

		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			// uid
			if (!get_preg($post['userName'], 'username')) {
				$triggered_messages['uid'][] = $this->messages['uid'][0];
			}
			else {
				$this->attributes['uid'][0] = $post['userName'];
			}
			// cn
			if ($post['cn'] == '') {
				if ($post['givenName'] != '') {
					$this->attributes['cn'][0] = $post['givenName'] . " " . $post['sn'];
				}
				else {
					$this->attributes['cn'][0] = $post['sn'];
				}
			}
			else {
				$this->attributes['cn'][0] = $post['cn'];
			}
			if (!get_preg($this->attributes['cn'][0], 'cn')) {
				$triggered_messages['cn'][] = $this->messages['cn'][0];
			}
		}

		// handle host-attribute in on epice because it's not set by default
		if (isset($this->attributes['host'])) {
			$host = $post['host'];
			if (!get_preg($host,'unixhost'))
				$triggered_messages['host'][] = $this->messages['host'][0];
			$hosts = explode(",", $host);
			$this->attributes['host'] = array();
			foreach ($hosts as $host)
				if ($host!="") $this->attributes['host'][] = $host;
			}
		// Do some regex-checks and return error if attributes are set to wrong values
		if (($this->attributes['givenName'][0] != '') && !get_preg($this->attributes['givenName'][0], 'realname')) $triggered_messages['givenName'][] = $this->messages['givenName'][0];
		if ( !get_preg($this->attributes['sn'][0], 'realname')) $triggered_messages['sn'][] = $this->messages['lastname'][0];
		if ( !get_preg($this->attributes['telephoneNumber'][0], 'telephone')) $triggered_messages['telephoneNumber'][] = $this->messages['telephoneNumber'][0];
		if ( !get_preg($this->attributes['mobile'][0], 'telephone')) $triggered_messages['mobileTelephoneNumber'][] = $this->messages['mobileTelephone'][0];
		if ( !get_preg($this->attributes['facsimileTelephoneNumber'][0], 'telephone')) $triggered_messages['facsimileTelephoneNumber'][] = $this->messages['facsimileNumber'][0];
		if (($this->attributes['mail'][0] != '') && !get_preg($this->attributes['mail'][0], 'email')) $triggered_messages['mail'][] = $this->messages['email'][0];
		if ( !get_preg($this->attributes['street'][0], 'street')) $triggered_messages['street'][] = $this->messages['street'][0];
		if ( !get_preg($this->attributes['postalAddress'][0], 'postalAddress')) $triggered_messages['postalAdress'][] = $this->messages['postalAddress'][0];
		if ( !get_preg($this->attributes['personal_postalCode'][0], 'postalCode')) $triggered_messages['personal_postalCode'][] = $this->messages['postalCode'][0];
		if ( !get_preg($this->attributes['title'][0], 'title')) $triggered_messages['title'][] = $this->messages['title'][0];
		if ( !get_preg($this->attributes['employeeType'][0], 'employeeType')) $triggered_messages['employeeType'][] = $this->messages['employeeType'][0];
		if ($post['userPassword']) {
			if ($post['userPassword'] != $post['userPassword2']) {
				$triggered_messages['userPassword'][] = $this->messages['userPassword'][0];
			}
			else {
				if (!get_preg($post['userPassword'], 'password')) {
					$triggered_messages['userPassword'][] = $this->messages['userPassword'][1];
				}
				else {
					$this->attributes['userPassword'][0] = $post['userPassword'];
				}
			}
		}
		if ($post['delPhoto']) $this->attributes['jpegPhoto'] = array();
		// Return error-messages
		return $triggered_messages;
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User name') . '*' ),
				1 => array ( 'kind' => 'input', 'name' => 'userName', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['uid'][0] ),
				2 => array ('kind' => 'help', 'value' => 'uid'));
		}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('First name') ),
			1 => array ( 'kind' => 'input', 'name' => 'givenName', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['givenName'][0] ),
			2 => array ('kind' => 'help', 'value' => 'givenName'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Last name').'*' ),
			1 => array ( 'kind' => 'input', 'name' => 'sn', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['sn'][0] ),
			2 => array ('kind' => 'help', 'value' => 'sn'));
		if (!in_array('posixAccount', $modules)) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Common name') . '*' ),
				1 => array ( 'kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $this->attributes['cn'][0] ),
				2 => array ('kind' => 'help', 'value' => 'cn'));
		}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));

		$return[] = array(0 => array('kind' => 'text', 'td' => array('colspan' => 3)));

		// password
		if (!in_array('posixAccount', $modules)) {
			// new account, show input fields
			if ($_SESSION[$this->base]->isNewAccount) {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Password') ),
					1 => array('kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
					2 => array('kind' => 'help', 'value' => 'userPassword'));
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Repeat password')),
					1 => array('kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['userPassword'][0]),
					2 => array('kind' => 'text', 'text' => ''));
			}
			// old account, show button for password page
			else {
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Password') ),
					1 => array('kind' => 'input', 'name' => 'form_subpage_inetOrgPerson_password_open', 'type' => 'submit', 'value' => _('Change password')));
			}

			$return[] = array(0 => array('kind' => 'text', 'td' => array('colspan' => 3)));
		}

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Street') ),
			1 => array ( 'kind' => 'input', 'name' => 'street', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['street'][0] ),
			2 => array ('kind' => 'help', 'value' => 'street'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Post office box') ),
			1 => array ( 'kind' => 'input', 'name' => 'postOfficeBox', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['postOfficeBox'][0] ),
			2 => array ('kind' => 'help', 'value' => 'postOfficeBox'));
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
				'maxlength' => '255', 'value' => $this->attributes['mobile'][0] ),
			2 => array ('kind' => 'help', 'value' => 'mobileTelephoneNumber'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Fax number') ),
			1 => array ( 'kind' => 'input', 'name' => 'facsimileTelephoneNumber', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['facsimileTelephoneNumber'][0] ),
			2 => array ('kind' => 'help', 'value' => 'facsimileTelephoneNumber'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('eMail address') ),
			1 => array ( 'kind' => 'input', 'name' => 'mail', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['mail'][0] ),
			2 => array ('kind' => 'help', 'value' => 'mail'));

		$return[] = array(0 => array('kind' => 'text', 'td' => array('colspan' => 3)));

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Job title') ),
			1 => array ( 'kind' => 'input', 'name' => 'title', 'type' => 'text', 'size' => '30',
				'value' => $this->attributes['title'][0] ),
			2 => array ('kind' => 'help', 'value' => 'title'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Employee type') ),
			1 => array ( 'kind' => 'input', 'name' => 'employeeType', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['employeeType'][0] ),
			2 => array ('kind' => 'help', 'value' => 'employeeType'));
		if (isset($this->attributes['host'])) {
			if (is_array($this->attributes['host'])) {
				$hostvalue .= implode(",", $this->attributes['host']);
			}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Unix workstations') ),
				1 => array ( 'kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $hostvalue ),
				2 => array ('kind' => 'help', 'value' => 'workstations'));
		}
		// get list of existing users for manager attribute
		$dnUsers = $_SESSION['cache']->get_cache('uid', 'inetOrgPerson', 'user');
		if (!is_array($dnUsers)) $dnUsers = array();
		$dnUsers = array_keys($dnUsers);
		usort($dnUsers, array($_SESSION['ldap'], 'cmp_array'));
		array_unshift($dnUsers, '-');
		$optionsSelected = array();
		if ($this->attributes['manager'][0]) {
			$optionsSelected[] = $this->attributes['manager'][0];
		}
		else {
			$optionsSelected[] = '-';
		}
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Manager')),
			1 => array('kind' => 'select', 'name' => 'manager', 'size' => '1',
				'options' => $dnUsers, 'options_selected' => $optionsSelected),
			2 => array('kind' => 'help', 'value' => 'manager'));
		// photo
		$photoFile = '../../graphics/userDefault.png';
		$noPhoto = true;
		if ($this->attributes['jpegPhoto'][0]) {
			$jpeg_filename = 'jpg' . $_SESSION['ldap']->new_rand() . '.jpg';
			$outjpeg = @fopen($_SESSION['lampath'] . 'tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $this->attributes['jpegPhoto'][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			$noPhoto = false;
		}
		$photo = array(array(
			0 => array('kind' => 'image', 'alt' => _('Photo'), 'path' => $photoFile, 'td' => array('align' => 'center'))));
		if ($noPhoto) {
			$photo[] = array(array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_inetOrgPerson_photo_open', 'value' => _('Add photo')));
		}
		else {
			$photo[] = array(array('kind' => 'input', 'type' => 'submit', 'name' => 'delPhoto', 'value' => _('Delete photo')));
		}
		$return = array(array(
			0 => array('kind' => 'table', 'value' => $return),
			1 => array('kind' => 'table', 'value' => $photo, 'td' => array('valign' => 'top', 'align' => 'right','width' => '100%'))
			));
		return $return;
		}

	/**
	* Sets a new password.
	*
	* @param $post HTTP POST
	*/
	function process_password(&$post) {
		if ($post['form_subpage_inetOrgPerson_attributes_back']) return;
		$messages = array();
		if ($post['userPassword'] != $post['userPassword2']) {
			$messages['userPassword'][] = $this->messages['userPassword'][0];
		}
		else {
			if (!get_preg($post['userPassword'], 'password')) {
				$messages['userPassword'][] = $this->messages['userPassword'][1];
			}
			else {
				$this->attributes['userPassword'][0] = $post['userPassword'];
			}
		}
		return $messages;
	}

	/**
	* Displays the password changing dialog.
	*
	* @param array $post HTTP-POST
	* @return array meta HTML code
	*/
	function display_html_password(&$post) {
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
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Submit'), 'name' => 'form_subpage_inetOrgPerson_attributes_submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_inetOrgPerson_attributes_back'),
					2 => array('kind' => 'text')))));
		return $return;
	}

	/**
	* Sets a new photo.
	*
	* @param $post HTTP POST
	*/
	function process_photo(&$post) {
		if ($post['form_subpage_inetOrgPerson_attributes_back']) return;
		$messages = array();
		if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 1000000);
			fclose($handle);
			$this->attributes['jpegPhoto'][0] = $data;
		}
		else {
			$messages['photo'][] = $this->messages['photo'][0];
		}
		return $messages;
	}

	/**
	* Displays the photo upload page.
	*
	* @param array $post HTTP-POST
	* @return array meta HTML code
	*/
	function display_html_photo(&$post) {
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Photo file (JPG format)') ),
			1 => array('kind' => 'input', 'name' => 'photoFile', 'type' => 'file'),
			2 => array('kind' => 'help', 'value' => 'photoUpload'));
		$return[] = array(
			0 => array('kind' => 'table', 'value' => array(
				0 => array(
					0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Add photo'), 'name' => 'form_subpage_inetOrgPerson_attributes_submit'),
					1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_inetOrgPerson_attributes_back'),
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
			'inetOrgPerson_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
			'inetOrgPerson_host' => array('<block><key>' . _('Unix workstations') . '</key><value>' . $this->attributes['host'][0] . '</value></block>'),
			'inetOrgPerson_title' => array('<block><key>' . _('Job title') . '</key><value>' . $this->attributes['title'][0] . '</value></block>'),
			'inetOrgPerson_givenName' => array('<block><key>' . _('First name') . '</key><value>' . $this->attributes['givenName'][0] . '</value></block>'),
			'inetOrgPerson_sn' => array('<block><key>' . _('Last name') . '</key><value>' . $this->attributes['sn'][0] . '</value></block>'),
			'inetOrgPerson_employeeType' => array('<block><key>' . _('Employee type') . '</key><value>' . $this->attributes['employeeType'][0] . '</value></block>'),
			'inetOrgPerson_manager' => array('<block><key>' . _('Manager') . '</key><value>' . $this->attributes['manager'][0] . '</value></block>'),
			'inetOrgPerson_street' => array('<block><key>' . _('Street') . '</key><value>' . $this->attributes['street'][0] . '</value></block>'),
			'inetOrgPerson_postOfficeBox' => array('<block><key>' . _('Post office box') . '</key><value>' . $this->attributes['postOfficeBox'][0] . '</value></block>'),
			'inetOrgPerson_postalCode' => array('<block><key>' . _('Postal code') . '</key><value>' . $this->attributes['postalCode'][0] . '</value></block>'),
			'inetOrgPerson_postalAddress' => array('<block><key>' . _('Postal address') . '</key><value>' . $this->attributes['postalAddress'][0] . '</value></block>'),
			'inetOrgPerson_telephoneNumber' => array('<block><key>' . _('Telephone number') . '</key><value>' . $this->attributes['telephoneNumber'][0] . '</value></block>'),
			'inetOrgPerson_mobileTelephoneNumber' => array('<block><key>' . _('Mobile number') . '</key><value>' . $this->attributes['mobile'][0] . '</value></block>'),
			'inetOrgPerson_facimileTelephoneNumber' => array('<block><key>' . _('Fax number') . '</key><value>' . $this->attributes['facsimileTelephoneNumber'][0] . '</value></block>'),
			'inetOrgPerson_mail' => array('<block><key>' . _('eMail address') . '</key><value>' . $this->attributes['mail'][0] . '</value></block>'),
			'inetOrgPerson_cn' => array('<block><key>' . _('Common name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			'inetOrgPerson_uid' => array('<block><key>' . _('User name') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'));
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
		$triggered_messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("inetOrgPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetOrgPerson";
			// last name
			if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_lastName']], 'realname')) {
				$partialAccounts[$i]['sn'] = $rawAccounts[$i][$ids['inetOrgPerson_lastName']];
			}
			else {
				$errMsg = $this->messages['lastname'][1];
				array_push($errMsg, array($i));
				$triggered_messages[] = $errMsg;
			}
			// first name
			if ($rawAccounts[$i][$ids['inetOrgPerson_firstName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_firstName']], 'realname')) {
					$partialAccounts[$i]['givenName'] = $rawAccounts[$i][$ids['inetOrgPerson_firstName']];
				}
				else {
					$errMsg = $this->messages['givenName'][1];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
				}
			}
			// post office box
			if ($rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']] != "") {
				$partialAccounts[$i]['postOfficeBox'] = $rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']];
			}
			// postal code
			if ($rawAccounts[$i][$ids['inetOrgPerson_postalCode']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_postalCode']], 'postalCode')) {
					$partialAccounts[$i]['postalCode'] = $rawAccounts[$i][$ids['inetOrgPerson_postalCode']];
				}
				else {
					$errMsg = $this->messages['postalCode'][1];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['inetOrgPerson_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['inetOrgPerson_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][1];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
				}
			}
		}
		return $triggered_messages;
	}

	
}

?>
