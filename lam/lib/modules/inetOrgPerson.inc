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
	* Creates a new inetOrgPerson object.
	*/
	function inetOrgPerson($scope) {
		// call parent constructor
		parent::baseModule($scope);
	}

	/** this functin fills the error message array with messages
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
		$this->messages['uid'][1] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][3] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name already exists!'));
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
		if ($_SESSION['ldap']->supports_unix_hosts) {
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
			'title',
			'givenName',
			'sn',
			'employeeType',
			'street',
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
				'street' => array (
					"Headline" => _("Street"),
					"Text" => _("Street")
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
				)
			)
		);
		
		return $return;
	}

	/**
	* Initializes the module in its accountContainer
	*
	* @param string $base the name of account_container in session
	*/
	function init($base) {
		// call parent init
		parent::init($base);
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
		if (isset($return[$_SESSION[$this->base]->dn]['add']['userPassword']))
			unset($return[$_SESSION[$this->base]->dn]['add']['userPassword']);
		// Return attributes
		return $return;
		}
	/* Write variables into object and do some regexp checks
	*/

	function delete_attributes($post) {
		return 0;
		}

	function process_attributes(&$post) {
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
		if ( !get_preg($this->attributes['mobileTelephoneNumber'][0], 'telephone')) $triggered_messages['mobileTelephoneNumber'][] = $this->messages['mobileTelephone'][0];
		if ( !get_preg($this->attributes['facsimileTelephoneNumber'][0], 'telephone')) $triggered_messages['facsimileTelephoneNumber'][] = $this->messages['facsimileNumber'][0];
		if (($this->attributes['mail'][0] != '') && !get_preg($this->attributes['mail'][0], 'email')) $triggered_messages['mail'][] = $this->messages['email'][0];
		if ( !get_preg($this->attributes['street'][0], 'street')) $triggered_messages['street'][] = $this->messages['street'][0];
		if ( !get_preg($this->attributes['postalAddress'][0], 'postalAddress')) $triggered_messages['postalAdress'][] = $this->messages['postalAddress'][0];
		if ( !get_preg($this->attributes['personal_postalCode'][0], 'postalCode')) $triggered_messages['personal_postalCode'][] = $this->messages['postalCode'][0];
		if ( !get_preg($this->attributes['title'][0], 'title')) $triggered_messages['title'][] = $this->messages['title'][0];
		if ( !get_preg($this->attributes['employeeType'][0], 'employeeType')) $triggered_messages['employeeType'][] = $this->messages['employeeType'][0];
		// Return error-messages
		if (is_array($triggered_messages)) return $triggered_messages;
		return 0;
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
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Job title') ),
			1 => array ( 'kind' => 'input', 'name' => 'title', 'type' => 'text', 'size' => '30',
				'value' => $this->attributes['title'][0] ),
			2 => array ('kind' => 'help', 'value' => 'title'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Employee type') ),
			1 => array ( 'kind' => 'input', 'name' => 'employeeType', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['employeeType'][0] ),
			2 => array ('kind' => 'help', 'value' => 'employeeType'));
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
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));
		if (isset($this->attributes['host'])) {
			if (is_array($this->attributes['host'])) {
				$hostvalue .= implode(",", $this->attributes['host']);
			}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Unix workstations') ),
				1 => array ( 'kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '30',
					'maxlength' => '255', 'value' => $hostvalue ),
				2 => array ('kind' => 'help', 'value' => 'workstations'));
			}
		return $return;
		}

	function display_html_delete(&$post) {
		return 0;
		}

	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array(
			'inetOrgPerson_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
			'inetOrgPerson_host' => array('<block><key>' . _('Unix workstations') . '</key><value>' . $this->attributes['host'][0] . '</value></block>'),
			'inetOrgPerson_title' => array('<block><key>' . _('Job title') . '</key><value>' . $this->attributes['title'][0] . '</value></block>'),
			'inetOrgPerson_givenName' => array('<block><key>' . _('First name') . '</key><value>' . $this->attributes['givenName'][0] . '</value></block>'),
			'inetOrgPerson_sn' => array('<block><key>' . _('Last name') . '</key><value>' . $this->attributes['sn'][0] . '</value></block>'),
			'inetOrgPerson_employeeType' => array('<block><key>' . _('Employee type') . '</key><value>' . $this->attributes['employeeType'][0] . '</value></block>'),
			'inetOrgPerson_street' => array('<block><key>' . _('Street') . '</key><value>' . $this->attributes['street'][0] . '</value></block>'),
			'inetOrgPerson_postalCode' => array('<block><key>' . _('Postal code') . '</key><value>' . $this->attributes['postalCode'][0] . '</value></block>'),
			'inetOrgPerson_postalAddress' => array('<block><key>' . _('Postal address') . '</key><value>' . $this->attributes['postalAddress'][0] . '</value></block>'),
			'inetOrgPerson_telephoneNumber' => array('<block><key>' . _('Telephone number') . '</key><value>' . $this->attributes['telephoneNumber'][0] . '</value></block>'),
			'inetOrgPerson_mobileTelephoneNumber' => array('<block><key>' . _('Mobile number') . '</key><value>' . $this->attributes['mobileTelephoneNumber'][0] . '</value></block>'),
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
					$partialAccounts[$i]['mobiletelephoneNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_mobile']];
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
				$dnUsers = $_SESSION['cache']->get_cache('uid', 'inetOrgPerson', '*');
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
