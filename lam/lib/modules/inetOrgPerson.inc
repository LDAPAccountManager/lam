<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2011  Roland Gruber

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
class inetOrgPerson extends baseModule implements passwordService {
	
	/** caches the list of possible managers */
	private $cachedManagers = null;
	private $clearTextPassword = null;
	
	/**
	* This function fills the message array.
	**/
	function load_Messages() {
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
		$this->messages['email'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['email'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_email', _('Please enter a valid email address!'));
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
		$this->messages['photo'][0] = array('ERROR', _('No file selected.'));
		$this->messages['businessCategory'][0] = array('ERROR', _('Business category'), _('Please enter a valid business category!'));
		$this->messages['businessCategory'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_businessCategory', _('Please enter a valid business category!'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'uid.png';
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
			'mobileTelephoneNumber' => 'mobile', 'organizationName' => 'o');
		// managed attributes
		$return['attributes'] = array('uid', 'cn', 'employeeType', 'givenName', 'jpegPhoto', 'mail', 'manager', 'mobile',
			'title', 'telephoneNumber', 'facsimileTelephoneNumber', 'street', 'postOfficeBox', 'postalCode', 'postalAddress',
			'sn', 'userPassword', 'description', 'homePhone', 'roomNumber', 'businessCategory', 'l', 'st', 'physicalDeliveryOfficeName',
			'carLicense', 'departmentNumber', 'o', 'employeeNumber', 'initials');
		// self service search attributes
		$return['selfServiceSearchAttributes'] = array('uid', 'mail', 'cn', 'surname', 'givenName', 'employeeNumber');
		// self service field settings
		$return['selfServiceFieldSettings'] = array('firstName' => _('First name'), 'lastName' => _('Last name'),
			'mail' => _('Email address'), 'telephoneNumber' => _('Telephone number'), 'mobile' => _('Mobile number'),
			'faxNumber' => _('Fax number'), 'street' => _('Street'), 'postalAddress' => _('Postal address'),
			'postalCode' => _('Postal code'), 'postOfficeBox' => _('Post office box'), 'jpegPhoto' => _('Photo'),
			'homePhone' => _('Home telephone number'), 'roomNumber' => _('Room number'), 'carLicense' => _('Car license'),
			'location' => _('Location'), 'state' => _('State'), 'officeName' => _('Office name'), 'businessCategory' => _('Business category'),
			'departmentNumber' => _('Department(s)'), 'initials' => _('Initials'), 'title' => _('Job title'));
		// profile elements
		$profileElements = array();
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Street'), 'inetOrgPerson_street', null, 'street');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Post office box'), 'inetOrgPerson_postOfficeBox', null, 'postOfficeBox');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Postal code'), 'inetOrgPerson_postalCode', null, 'postalCode');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Location'), 'inetOrgPerson_l', null, 'l');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$profileElements[] = new htmlTableExtendedInputField(_('State'), 'inetOrgPerson_st', null, 'st');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Postal address'), 'inetOrgPerson_postalAddress', null, 'postalAddress');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Telephone number'), 'inetOrgPerson_telephoneNumber', null, 'telephoneNumber');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Fax number'), 'inetOrgPerson_facsimileTelephoneNumber', null, 'facsimileTelephoneNumber');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Email address'), 'inetOrgPerson_mail', null, 'mail');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Department(s)'), 'inetOrgPerson_departmentNumber', null, 'departmentNumber');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Organisation'), 'inetOrgPerson_o', null, 'o');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Job title'), 'inetOrgPerson_title', null, 'title');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Employee type'), 'inetOrgPerson_employeeType', null, 'employeeType');
		}
		if (sizeof($profileElements) > 0) {
			$profileContainer = new htmlTable();
			for ($i = 0; $i < sizeof($profileElements); $i++) {
				$profileContainer->addElement($profileElements[$i]);
				$profileContainer->addNewLine();
			}
			$return['profile_options'] = $profileContainer;
		}
		// profile checks and mappings
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$return['profile_checks']['inetOrgPerson_title'] = array(
				'type' => 'ext_preg',
				'regex' => 'title',
				'error_message' => $this->messages['title'][0]);
			$return['profile_mappings']['inetOrgPerson_title'] = 'title';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$return['profile_checks']['inetOrgPerson_employeeType'] = array(
				'type' => 'ext_preg',
				'regex' => 'employeeType',
				'error_message' => $this->messages['employeeType'][0]);
			$return['profile_mappings']['inetOrgPerson_employeeType'] = 'employeeType';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$return['profile_checks']['inetOrgPerson_street'] = array(
				'type' => 'ext_preg',
				'regex' => 'street',
				'error_message' => $this->messages['street'][0]);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$return['profile_checks']['inetOrgPerson_postalCode'] = array(
				'type' => 'ext_preg',
				'regex' => 'postalCode',
				'error_message' => $this->messages['postalCode'][0]);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$return['profile_checks']['inetOrgPerson_postalAddress'] = array(
				'type' => 'ext_preg',
				'regex' => 'postalAddress',
				'error_message' => $this->messages['postalAddress'][0]);
			$return['profile_mappings']['inetOrgPerson_postalAddress'] = 'postalAddress';
		}
		// configuration settings
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainer->addElement($configContainerHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideDescription', false, _('Description'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideStreet', false, _('Street'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidePostOfficeBox', false, _('Post office box'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidePostalCode', false, _('Postal code'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideLocation', false, _('Location'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideState', false, _('State'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidePostalAddress', false, _('Postal address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideOfficeName', false, _('Office name'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideRoomNumber', false, _('Room number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideTelephoneNumber', false, _('Telephone number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideHomeTelephoneNumber', false, _('Home telephone number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideMobileNumber', false, _('Mobile number'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideFaxNumber', false, _('Fax number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEMailAddress', false, _('Email address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideJobTitle', false, _('Job title'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideCarLicense', false, _('Car license'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeType', false, _('Employee type'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideBusinessCategory', false, _('Business category'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideDepartments', false, _('Department(s)'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideManager', false, _('Manager'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideO', false, _('Organisation'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeNumber', false, _('Employee number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideInitials', false, _('Initials'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		if (isset($_SESSION['conf_config'])) {
			// add password hash type if posixAccount is inactive
			$confActiveUnixModules = array_merge($_SESSION['conf_config']->get_AccountModules('user'), $_SESSION['conf_config']->get_AccountModules('host'), $_SESSION['conf_config']->get_AccountModules('group'));
			if (!in_array('posixAccount', $confActiveUnixModules) && !in_array('posixGroup', $confActiveUnixModules)) {
				$options = array("CRYPT", "SHA", "SSHA", "MD5", "SMD5", "PLAIN");
				$optionsSelected = array('SSHA');
				$hashOption = new htmlTable();
				$hashOption->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', $options, $optionsSelected, _("Password hash type"), 'pwdHash'));
				$configContainer->addElement($hashOption);
			}
		}
		$return['config_options']['all'] = $configContainer;
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
			)
		);
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_initials',
				'description' => _('Initials'),
				'help' => 'initials',
				'example' => 'A.B.'
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Temp, contract until december')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_title',
				'description' => _('Job title'),
				'help' => 'title',
				'example' => _('President')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_employeeNumber',
				'description' => _('Employee number'),
				'help' => 'employeeNumber',
				'example' => '123456'
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_type',
				'description' => _('Employee type'),
				'help' => 'employeeType',
				'example' => _('Temp')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_manager',
				'description' => _('Manager'),
				'help' => 'manager',
				'example' => _('uid=smiller,ou=People,dc=company,dc=com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_businessCategory',
				'description' => _('Business category'),
				'help' => 'businessCategory',
				'example' => _('Administration')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_street',
				'description' => _('Street'),
				'help' => 'street',
				'example' => _('Mystreetname 42')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_postalCode',
				'description' => _('Postal code'),
				'help' => 'postalCode',
				'example' => _('GB-12345')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_address',
				'description' => _('Postal address'),
				'help' => 'postalAddress',
				'example' => _('Mycity')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_postOfficeBox',
				'description' => _('Post office box'),
				'help' => 'postOfficeBox',
				'example' => _('12345')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_telephone',
				'description' => _('Telephone number'),
				'help' => 'telephoneNumber',
				'example' => _('123-123-1234')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_homePhone',
				'description' => _('Home telephone number'),
				'help' => 'homePhone',
				'example' => _('123-124-1234')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_mobile',
				'description' => _('Mobile number'),
				'help' => 'mobileTelephoneNumber',
				'example' => _('123-123-1235')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_fax',
				'description' => _('Fax number'),
				'help' => 'facsimileTelephoneNumber',
				'example' => _('123-123-1236')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_email',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('user@company.com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_roomNumber',
				'description' => _('Room number'),
				'help' => 'roomNumber',
				'example' => 'A 2.24'
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_departmentNumber',
				'description' => _('Department(s)'),
				'help' => 'departmentNumber',
				'example' => _('Administration')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_o',
				'description' => _('Organisation'),
				'help' => 'o',
				'example' => _('YourCompany')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_l',
				'description' => _('Location'),
				'help' => 'l',
				'example' => _('MyCity')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_st',
				'description' => _('State'),
				'help' => 'st',
				'example' => _('New York')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_carLicense',
				'description' => _('Car license'),
				'help' => 'carLicense',
				'example' => _('yes')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_physicalDeliveryOfficeName',
				'description' => _('Office name'),
				'help' => 'physicalDeliveryOfficeName',
				'example' => _('YourCompany, Human Resources')
			);
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'givenName' => _('First name'),
			'sn' => _('Last name')
		);
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$return['PDF_fields']['description'] = _('Description');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$return['PDF_fields']['street'] = _('Street');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$return['PDF_fields']['postOfficeBox'] = _('Post office box');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$return['PDF_fields']['postalCode'] = _('Postal code');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$return['PDF_fields']['location'] = _('Location');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$return['PDF_fields']['state'] = _('State');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$return['PDF_fields']['postalAddress'] = _('Postal address');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$return['PDF_fields']['officeName'] = _('Office name');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$return['PDF_fields']['roomNumber'] = _('Room number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$return['PDF_fields']['telephoneNumber'] = _('Telephone number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$return['PDF_fields']['homePhone'] = _('Home telephone number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$return['PDF_fields']['mobileTelephoneNumber'] = _('Mobile number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$return['PDF_fields']['facsimileTelephoneNumber'] = _('Fax number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$return['PDF_fields']['title'] = _('Job title');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$return['PDF_fields']['carLicense'] = _('Car license');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$return['PDF_fields']['employeeType'] = _('Employee type');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$return['PDF_fields']['businessCategory'] = _('Business category');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$return['PDF_fields']['departmentNumber'] = _('Department(s)');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$return['PDF_fields']['manager'] = _('Manager');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$return['PDF_fields']['o'] = _('Organisation');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$return['PDF_fields']['employeeNumber'] = _('Employee number');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$return['PDF_fields']['initials'] = _('Initials');
		}
		
		if (isset($_SESSION['loggedIn']) && ($_SESSION['loggedIn'] === true)) {
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				$return['PDF_fields']['uid'] = _('User name');
				$return['PDF_fields']['cn'] = _('Common name');
			}
		}
		// help Entries
		$return['help'] = array (
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
				"Text" => _("The street name of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postOfficeBox' => array (
				"Headline" => _("Post office box"),
				"Text" => _("The post office box of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postalCode' => array (
				"Headline" => _("Postal code"),
				"Text" => _("The postal code of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postalAddress' => array (
				"Headline" => _("Postal address"),
				"Text" => _("Postal address, city")
			),
			'telephoneNumber' => array (
				"Headline" => _("Telephone number"),
				"Text" => _("The user's telephone number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mobileTelephoneNumber' => array (
				"Headline" => _("Mobile number"),
				"Text" => _("The user's mobile number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'facsimileTelephoneNumber' => array (
				"Headline" => _("Fax number"),
				"Text" => _("The user's fax number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mail' => array (
				"Headline" => _("Email address"),
				"Text" => _("The user's email address.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'cn' => array (
				"Headline" => _("Common name"),
				"Text" => _("This is the natural name of the user. If empty, the first and last name is used.")
			),
			'uid' => array(
				"Headline" => _("User name"),
				"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, .-_. If user name is already used user name will be expanded with a number. The next free number will be used. Warning: Older systems have problems with user names longer than 8 characters. You can not log in to Windows if user name is longer than 16 characters.")
			),
			'photoUpload' => array(
				"Headline" => _("Add photo"),
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			),
			'homePhone' => array(
				"Headline" => _("Home telephone number"),
				"Text" => _("The user's private telephone number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'roomNumber' => array(
				"Headline" => _("Room number"),
				"Text" => _("The room number of the employee's office.")
			),
			'businessCategory' => array(
				"Headline" => _("Business category"),
				"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)")
			),
			'l' => array(
				"Headline" => _("Location"),
				"Text" => _("This describes the location of the user.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'st' => array(
				"Headline" => _("State"),
				"Text" => _("The state where the user resides or works.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'carLicense' => array(
				"Headline" => _("Car license"),
				"Text" => _("This can be used to specify if the user has a car license.")
			),
			'physicalDeliveryOfficeName' => array(
				"Headline" => _("Office name"),
				"Text" => _("The office name of the user (e.g. YourCompany, Human Resources).")
			),
			'departmentNumber' => array(
				"Headline" => _("Department(s)"),
				"Text" => _("Here you can enter the user's department(s).") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'o' => array(
				"Headline" => _("Organisation"),
				"Text" => _("The user's organisation name.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'employeeNumber' => array(
				"Headline" => _("Employee number"),
				"Text" => _("The user's unique employee number.")
			),
			'initials' => array(
				"Headline" => _("Initials"),
				"Text" => _("The initials of the user's first names.") . ' ' . _('Multiple values are separated by semicolon.')
			),
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
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			if (($this->getAccountContainer()->rdn == 'uid') && !isset($this->attributes['uid'][0])) {
				return false;
			}
		}
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
	* <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	*/
	function save_attributes() {
		// skip saving if account is based on another structural object class
		if (!$this->getAccountContainer()->isNewAccount && !in_array('inetOrgPerson', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return array();
		}
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// postalAddress, facsimileTelephoneNumber and jpegPhoto need special removing
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['postalAddress'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['postalAddress'] = $this->attributes['postalAddress'];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['postalAddress']);
		}
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['facsimileTelephoneNumber'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['facsimileTelephoneNumber'] = $this->attributes['facsimileTelephoneNumber'];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['facsimileTelephoneNumber']);
		}
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['jpegPhoto'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['jpegPhoto'] = array();
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['jpegPhoto']);
		}
		// add information about clear text password
		if ($this->clearTextPassword != null) {
			$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordClearText'][0] = $this->clearTextPassword;
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
		// add parent object classes
		if ($this->getAccountContainer()->isNewAccount) {
			if (!in_array('organizationalPerson', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'organizationalPerson';
			}
			if (!in_array('person', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'person';
			}
		}
		// Load and check attributes
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$this->attributes['description'][0] = $_POST['description'];
		}
		$this->attributes['sn'][0] = $_POST['sn'];
		if (!get_preg($this->attributes['sn'][0], 'realname')) $errors[] = $this->messages['lastname'][0];
		$this->attributes['givenName'][0] = $_POST['givenName'];
		if (($this->attributes['givenName'][0] != '') && !get_preg($this->attributes['givenName'][0], 'realname')) $errors[] = $this->messages['givenName'][0];
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$this->attributes['title'][0] = $_POST['title'];
			if (!get_preg($this->attributes['title'][0], 'title')) $errors[] = $this->messages['title'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$this->attributes['mail'] = preg_split('/;[ ]*/', $_POST['mail']);
			for ($i = 0; $i < sizeof($this->attributes['mail']); $i++) {
				if (($this->attributes['mail'][$i] != '') && !get_preg($this->attributes['mail'][$i], 'email')) {
					$errors[] = $this->messages['email'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$this->attributes['telephoneNumber'] = preg_split('/;[ ]*/', $_POST['telephoneNumber']);
			for ($i = 0; $i < sizeof($this->attributes['telephoneNumber']); $i++) {
				if (!get_preg($this->attributes['telephoneNumber'][$i], 'telephone')) {
					$errors[] = $this->messages['telephoneNumber'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$this->attributes['mobile'] = preg_split('/;[ ]*/', $_POST['mobileTelephoneNumber']);
			for ($i = 0; $i < sizeof($this->attributes['mobile']); $i++) {
				if (!get_preg($this->attributes['mobile'][$i], 'telephone')) {
					$errors[] = $this->messages['mobileTelephone'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$this->attributes['facsimileTelephoneNumber'] = preg_split('/;[ ]*/', $_POST['facsimileTelephoneNumber']);
			for ($i = 0; $i < sizeof($this->attributes['facsimileTelephoneNumber']); $i++) {
				if (!get_preg($this->attributes['facsimileTelephoneNumber'][$i], 'telephone')) {
					$errors[] = $this->messages['facsimileNumber'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$this->attributes['homePhone'] = preg_split('/;[ ]*/', $_POST['homePhone']);
			for ($i = 0; $i < sizeof($this->attributes['homePhone']); $i++) {
				if (!get_preg($this->attributes['homePhone'][$i], 'telephone')) {
					$errors[] = $this->messages['homePhone'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$this->attributes['street'] = preg_split('/;[ ]*/', $_POST['street']);
			for ($i = 0; $i < sizeof($this->attributes['street']); $i++) {
				if (!get_preg($this->attributes['street'][$i], 'street')) {
					$errors[] = $this->messages['street'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$this->attributes['postOfficeBox'] = preg_split('/;[ ]*/', $_POST['postOfficeBox']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$this->attributes['postalCode'] = preg_split('/;[ ]*/', $_POST['postalCode']);
			for ($i = 0; $i < sizeof($this->attributes['postalCode']); $i++) {
				if (!get_preg($this->attributes['postalCode'][$i], 'postalCode')) {
					$errors[] = $this->messages['postalCode'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$addressCounter = 0;
			while (isset($_POST['postalAddress' . $addressCounter])) {
				$this->attributes['postalAddress'][$addressCounter] = implode('$', preg_split('/[\r][\n]/', $_POST['postalAddress' . $addressCounter]));
				if (!get_preg($this->attributes['postalAddress'][$addressCounter], 'postalAddress')) $errors[] = $this->messages['postalAddress'][0];
				if ($this->attributes['postalAddress'][$addressCounter] == '') {
					unset($this->attributes['postalAddress'][$addressCounter]);
				}
				$addressCounter++;
			}
			if (isset($_POST['addPostalAddress'])) {
				$this->attributes['postalAddress'][] = '';
			}
			$this->attributes['postalAddress'] = array_values($this->attributes['postalAddress']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$this->attributes['employeeType'][0] = $_POST['employeeType'];
			if (!get_preg($this->attributes['employeeType'][0], 'employeeType')) $errors[] = $this->messages['employeeType'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$this->attributes['roomNumber'][0] = $_POST['roomNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$this->attributes['businessCategory'][0] = $_POST['businessCategory'];
			if (!get_preg($this->attributes['businessCategory'][0], 'businessCategory')) $errors[] = $this->messages['businessCategory'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$this->attributes['l'] = preg_split('/;[ ]*/', $_POST['l']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$this->attributes['st'] = preg_split('/;[ ]*/', $_POST['st']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$this->attributes['carLicense'][0] = $_POST['carLicense'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$this->attributes['physicalDeliveryOfficeName'][0] = $_POST['physicalDeliveryOfficeName'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			if (isset($_POST['departmentNumber'])) {
				$this->attributes['departmentNumber'] = explode(';', $_POST['departmentNumber']);
				// remove extra spaces
				$this->attributes['departmentNumber'] = array_map('trim', $this->attributes['departmentNumber']);
			}
			else {
				if (isset($this->attributes['departmentNumber'])) {
					unset($this->attributes['departmentNumber']);
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			if ($_POST['manager'] != '-') {
				$this->attributes['manager'][0] = $_POST['manager'];
			}
			else {
				unset($this->attributes['manager'][0]);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$this->attributes['employeeNumber'][0] = $_POST['employeeNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$this->attributes['o'] = preg_split('/;[ ]*/', $_POST['o']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$this->attributes['initials'] = preg_split('/;[ ]*/', $_POST['initials']);
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
			if (isset($_POST['lockPassword'])) {
				$this->attributes['userPassword'][0] = pwd_disable($this->attributes['userPassword'][0]);
			}
			if (isset($_POST['unlockPassword'])) {
				$this->attributes['userPassword'][0] = pwd_enable($this->attributes['userPassword'][0]);
			}
			if (isset($_POST['removePassword'])) {
				unset($this->attributes['userPassword']);
			}
		}
		
		if (isset($_POST['delPhoto'])) {
			$this->attributes['jpegPhoto'] = array();
		}
		// Return error-messages
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		$fieldContainer = new htmlTable();
		$imageContainer = new htmlTable();
		$imageContainer->alignment = htmlElement::ALIGN_TOP;
		$container->addElement($fieldContainer);
		$container->addElement(new htmlSpacer('50px', null));
		$container->addElement($imageContainer, false);
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			$uid = '';
			if (isset($this->attributes['uid'][0])) $uid = $this->attributes['uid'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('User name'), 'userName', $uid, 'uid'), true);
		}
		$firstName = '';
		if (isset($this->attributes['givenName'][0])) $firstName = $this->attributes['givenName'][0];
		$fieldContainer->addElement(new htmlTableExtendedInputField(_('First name'), 'givenName', $firstName, 'givenName'), true);
		$lastName = '';
		if (isset($this->attributes['sn'][0])) $lastName = $this->attributes['sn'][0];
		$lastNameElement = new htmlTableExtendedInputField(_('Last name'), 'sn', $lastName, 'sn');
		$lastNameElement->setRequired(true);
		$fieldContainer->addElement($lastNameElement, true);
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$initials = '';
			if (isset($this->attributes['initials'][0])) $initials = implode('; ', $this->attributes['initials']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Initials'), 'initials', $initials, 'initials'), true);
		}
		if (!in_array('posixAccount', $modules)) {
			$cn = '';
			if (isset($this->attributes['cn'][0])) $cn = $this->attributes['cn'][0];
			$cnElement = new htmlTableExtendedInputField(_('Common name'), 'cn', $cn, 'cn');
			$cnElement->setRequired(true);
			$fieldContainer->addElement($cnElement, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$description = '';
			if (isset($this->attributes['description'][0])) $description = $this->attributes['description'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Description'), 'description', $description, 'description'), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Address')), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$street = '';
			if (isset($this->attributes['street'][0])) $street = implode('; ', $this->attributes['street']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Street'), 'street', $street, 'street'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$postOffice = '';
			if (isset($this->attributes['postOfficeBox'][0])) $postOffice = implode('; ', $this->attributes['postOfficeBox']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Post office box'), 'postOfficeBox', $postOffice, 'postOfficeBox'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$postalCode = '';
			if (isset($this->attributes['postalCode'][0])) $postalCode = implode('; ', $this->attributes['postalCode']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Postal code'), 'postalCode', $postalCode, 'postalCode'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$l = '';
			if (isset($this->attributes['l'][0])) $l = implode('; ', $this->attributes['l']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Location'), 'l', $l, 'l'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$st = '';
			if (isset($this->attributes['st'][0])) $st = implode('; ', $this->attributes['st']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('State'), 'st', $st, 'st'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$postalAddresses = array();
			if (isset($this->attributes['postalAddress'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['postalAddress']); $i++) {
					$postalAddresses[] = implode("\r\n", explode('$', $this->attributes['postalAddress'][$i]));
				}
			}
			if (sizeof($postalAddresses) == 0) {
				$postalAddresses[] = '';
			}
			$addressLabel = new htmlOutputText(_('Postal address'));
			$addressLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($addressLabel);
			$addressContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($postalAddresses); $i++) {
				$addressContainer->addElement(new htmlInputTextarea('postalAddress' . $i, $postalAddresses[$i], 30, 3));
				if ($i < (sizeof($postalAddresses) - 1)) {
					$addressContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$addressContainer->addElement(new htmlButton('addPostalAddress', 'add.png', true));
				}
			}
			$fieldContainer->addElement($addressContainer);
			$addressHelp = new htmlHelpLink('postalAddress');
			$addressHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($addressHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$physicalDeliveryOfficeName = '';
			if (isset($this->attributes['physicalDeliveryOfficeName'][0])) $physicalDeliveryOfficeName = $this->attributes['physicalDeliveryOfficeName'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Office name'), 'physicalDeliveryOfficeName', $physicalDeliveryOfficeName, 'physicalDeliveryOfficeName'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$roomNumber = '';
			if (isset($this->attributes['roomNumber'][0])) $roomNumber = $this->attributes['roomNumber'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Room number'), 'roomNumber', $roomNumber, 'roomNumber'), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Contact data')), true);
		}
		
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$telephone = '';
			if (isset($this->attributes['telephoneNumber'][0])) $telephone = implode('; ', $this->attributes['telephoneNumber']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Telephone number'), 'telephoneNumber', $telephone, 'telephoneNumber'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$homePhone = '';
			if (isset($this->attributes['homePhone'][0])) $homePhone = implode('; ', $this->attributes['homePhone']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Home telephone number'), 'homePhone', $homePhone, 'homePhone'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$mobile = '';
			if (isset($this->attributes['mobile'][0])) $mobile = implode('; ', $this->attributes['mobile']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Mobile number'), 'mobileTelephoneNumber', $mobile, 'mobileTelephoneNumber'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$fax = '';
			if (isset($this->attributes['facsimileTelephoneNumber'][0])) $fax = implode('; ', $this->attributes['facsimileTelephoneNumber']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Fax number'), 'facsimileTelephoneNumber', $fax, 'facsimileTelephoneNumber'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$email = '';
			if (isset($this->attributes['mail'][0])) $email = implode('; ', $this->attributes['mail']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Email address'), 'mail', $email, 'mail'), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Work details')), true);
		}
		
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$title = '';
			if (isset($this->attributes['title'][0])) $title = $this->attributes['title'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Job title'), 'title', $title, 'title'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$carLicense = '';
			if (isset($this->attributes['carLicense'][0])) $carLicense = $this->attributes['carLicense'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Car license'), 'carLicense', $carLicense, 'carLicense'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$employeeNumber = '';
			if (isset($this->attributes['employeeNumber'][0])) $employeeNumber = $this->attributes['employeeNumber'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Employee number'), 'employeeNumber', $employeeNumber, 'employeeNumber'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$employeeType = '';
			if (isset($this->attributes['employeeType'][0])) $employeeType = $this->attributes['employeeType'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Employee type'), 'employeeType', $employeeType, 'employeeType'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$businessCategory = '';
			if (isset($this->attributes['businessCategory'][0])) $businessCategory = $this->attributes['businessCategory'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Business category'), 'businessCategory', $businessCategory, 'businessCategory'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$departmentNumber = '';
			if (isset($this->attributes['departmentNumber'][0])) $departmentNumber = implode('; ', $this->attributes['departmentNumber']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Department(s)'), 'departmentNumber', $departmentNumber, 'departmentNumber'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$o = '';
			if (isset($this->attributes['o'][0])) $o = implode('; ', $this->attributes['o']);
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Organisation'), 'o', $o, 'o'), true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$optionsSelected = array();
			if (isset($this->attributes['manager'][0])) {
				$optionsSelected[] = $this->attributes['manager'][0];
			}
			else {
				$optionsSelected[] = '-';
			}
			$managerElement = new htmlTableExtendedSelect('manager', $this->getManagers(), $optionsSelected, _('Manager'), 'manager');
			$managerElement->setHasDescriptiveElements(true);
			$managerElement->setRightToLeftTextDirection(true);
			$managerElement->setSortElements(false);
			$fieldContainer->addElement($managerElement, true);
		}
		// password buttons
		if (!in_array('posixAccount', $modules) && checkIfWriteAccessIsAllowed() && isset($this->attributes['userPassword'][0])) {
			$fieldContainer->addElement(new htmlSubTitle(_('Password')), true);
			$pwdContainer = new htmlTable();
			if (pwd_is_enabled($this->attributes['userPassword'][0])) {
				$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
			}
			else {
				$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
			}
			$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
			$pwdContainer->colspan = 2;
			$fieldContainer->addElement($pwdContainer);
		}
		// photo
		$photoFile = '../../graphics/userDefault.png';
		$noPhoto = true;
		if (isset($this->attributes['jpegPhoto'][0])) {
			$jpeg_filename = 'jpg' . $_SESSION['ldap']->new_rand() . '.jpg';
			$outjpeg = @fopen(dirname(__FILE__) . '/../../tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $this->attributes['jpegPhoto'][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			$noPhoto = false;
		}
		$imageContainer->addElement(new htmlImage($photoFile), true);
		if ($noPhoto) {
			$imageContainer->addElement(new htmlAccountPageButton(get_class($this), 'photo', 'open', _('Add photo')));
		}
		else {
			$imageContainer->addElement(new htmlButton('delPhoto', _('Delete photo')));
		}
		return $container; 
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
		$container = new htmlTable();
		$container->addElement(new htmlTableExtendedInputFileUpload('photoFile', _('Photo file (JPG format)'), 'photoUpload'), true);
		$buttonContainer = new htmlTable();
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'submit', _('Add photo')));
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')));
		$container->addElement($buttonContainer);
		return $container;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$description = '';
		if (isset($this->attributes['description'][0])) $description = $this->attributes['description'][0];
		$title = '';
		if (isset($this->attributes['title'][0])) $title = $this->attributes['title'][0];
		$givenName = '';
		if (isset($this->attributes['givenName'][0])) $givenName = $this->attributes['givenName'][0];
		$lastName = '';
		if (isset($this->attributes['sn'][0])) {
			$lastName = $this->attributes['sn'][0];
		}
		$manager = '';
		if (isset($this->attributes['manager'][0])) {
			$manager = $this->attributes['manager'][0];
		}
		$street = '';
		if (isset($this->attributes['street'][0])) {
			$street = implode(', ', $this->attributes['street']);
		}
		$postOfficeBox = '';
		if (isset($this->attributes['postOfficeBox'][0])) {
			$postOfficeBox = implode(', ', $this->attributes['postOfficeBox']);
		}
		$postalCode = '';
		if (isset($this->attributes['postalCode'][0])) {
			$postalCode = implode(', ', $this->attributes['postalCode']);
		}
		$postalAddress = '';
		if (isset($this->attributes['postalAddress'][0])) {
			$postalAddress = implode(', ', $this->attributes['postalAddress']);
		}
		$telephoneNumber = '';
		if (isset($this->attributes['telephoneNumber'][0])) {
			$telephoneNumber = implode(', ', $this->attributes['telephoneNumber']);
		}
		$homePhone = '';
		if (isset($this->attributes['homePhone'][0])) {
			$homePhone = implode(', ', $this->attributes['homePhone']);
		}
		$mobile = '';
		if (isset($this->attributes['mobile'][0])) {
			$mobile = implode(', ', $this->attributes['mobile']);
		}
		$mail = '';
		if (isset($this->attributes['mail'][0])) {
			$mail = implode(', ', $this->attributes['mail']);
		}
		$cn = '';
		if (isset($this->attributes['cn'][0])) {
			$cn = $this->attributes['cn'][0];
		}
		$roomNumber = '';
		if (isset($this->attributes['roomNumber'][0])) {
			$roomNumber = $this->attributes['roomNumber'][0];
		}
		$facsimileTelephoneNumber = '';
		if (isset($this->attributes['facsimileTelephoneNumber'][0])) {
			$facsimileTelephoneNumber = implode(', ', $this->attributes['facsimileTelephoneNumber']);
		}
		$businessCategory = '';
		if (isset($this->attributes['businessCategory'][0])) {
			$businessCategory = $this->attributes['businessCategory'][0];
		}
		$uid = '';
		if (isset($this->attributes['uid'][0])) {
			$uid = $this->attributes['uid'][0];
		}
		$carLicense = '';
		if (isset($this->attributes['carLicense'][0])) {
			$carLicense = $this->attributes['carLicense'][0];
		}
		$st = '';
		if (isset($this->attributes['st'][0])) {
			$st = implode(', ', $this->attributes['st']);
		}
		$physicalDeliveryOfficeName = '';
		if (isset($this->attributes['physicalDeliveryOfficeName'][0])) {
			$physicalDeliveryOfficeName = $this->attributes['physicalDeliveryOfficeName'][0];
		}
		$employeeType = '';
		if (isset($this->attributes['employeeType'][0])) $employeeType = $this->attributes['employeeType'][0];
		$l = '';
		if (isset($this->attributes['l'][0])) {
			$l = implode(', ', $this->attributes['l']);
		}
		$employeeNumber = '';
		if (isset($this->attributes['employeeNumber'][0])) {
			$employeeNumber = $this->attributes['employeeNumber'][0];
		}
		$o = '';
		if (isset($this->attributes['o'][0])) {
			$o = implode(', ', $this->attributes['o']);
		}
		$initials = '';
		if (isset($this->attributes['initials'][0])) {
			$initials = implode(', ', $this->attributes['initials']);
		}
		
		$return = array(
			get_class($this) . '_description' => array('<block><key>' . _('Description') . '</key><value>' . $description . '</value></block>'),
			get_class($this) . '_title' => array('<block><key>' . _('Job title') . '</key><value>' . $title . '</value></block>'),
			get_class($this) . '_givenName' => array('<block><key>' . _('First name') . '</key><value>' . $givenName . '</value></block>'),
			get_class($this) . '_sn' => array('<block><key>' . _('Last name') . '</key><value>' . $lastName . '</value></block>'),
			get_class($this) . '_employeeType' => array('<block><key>' . _('Employee type') . '</key><value>' . $employeeType . '</value></block>'),
			get_class($this) . '_manager' => array('<block><key>' . _('Manager') . '</key><value>' . $manager . '</value></block>'),
			get_class($this) . '_street' => array('<block><key>' . _('Street') . '</key><value>' . $street . '</value></block>'),
			get_class($this) . '_postOfficeBox' => array('<block><key>' . _('Post office box') . '</key><value>' . $postOfficeBox . '</value></block>'),
			get_class($this) . '_postalCode' => array('<block><key>' . _('Postal code') . '</key><value>' . $postalCode . '</value></block>'),
			get_class($this) . '_postalAddress' => array('<block><key>' . _('Postal address') . '</key><value>' . $postalAddress . '</value></block>'),
			get_class($this) . '_telephoneNumber' => array('<block><key>' . _('Telephone number') . '</key><value>' . $telephoneNumber . '</value></block>'),
			get_class($this) . '_homePhone' => array('<block><key>' . _('Home telephone number') . '</key><value>' . $homePhone . '</value></block>'),
			get_class($this) . '_mobileTelephoneNumber' => array('<block><key>' . _('Mobile number') . '</key><value>' . $mobile . '</value></block>'),
			get_class($this) . '_facsimileTelephoneNumber' => array('<block><key>' . _('Fax number') . '</key><value>' . $facsimileTelephoneNumber . '</value></block>'),
			get_class($this) . '_mail' => array('<block><key>' . _('Email address') . '</key><value>' . $mail . '</value></block>'),
			get_class($this) . '_cn' => array('<block><key>' . _('Common name') . '</key><value>' . $cn . '</value></block>'),
			get_class($this) . '_roomNumber' => array('<block><key>' . _('Room number') . '</key><value>' . $roomNumber . '</value></block>'),
			get_class($this) . '_businessCategory' => array('<block><key>' . _('Business category') . '</key><value>' . $businessCategory . '</value></block>'),
			get_class($this) . '_uid' => array('<block><key>' . _('User name') . '</key><value>' . $uid . '</value></block>'),
			get_class($this) . '_carLicense' => array('<block><key>' . _('Car license') . '</key><value>' . $carLicense . '</value></block>'),
			get_class($this) . '_location' => array('<block><key>' . _('Location') . '</key><value>' . $l . '</value></block>'),
			get_class($this) . '_state' => array('<block><key>' . _('State') . '</key><value>' . $st . '</value></block>'),
			get_class($this) . '_officeName' => array('<block><key>' . _('Office name') . '</key><value>' . $physicalDeliveryOfficeName . '</value></block>'),
			get_class($this) . '_o' => array('<block><key>' . _('Organisation') . '</key><value>' . $o . '</value></block>'),
			get_class($this) . '_employeeNumber' => array('<block><key>' . _('Employee number') . '</key><value>' . $employeeNumber . '</value></block>'),
			get_class($this) . '_initials' => array('<block><key>' . _('Initials') . '</key><value>' . $initials . '</value></block>'),
		);
		if (isset($this->attributes['departmentNumber'])) {
			$return[get_class($this) . '_departmentNumber'] = array('<block><key>' . _('Department(s)') . '</key><value>' . implode(', ', $this->attributes['departmentNumber']) . '</value></block>');
		}
		return $return;
	}
	
	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			// departments
			if (isset($profile['inetOrgPerson_departmentNumber'][0]) && $profile['inetOrgPerson_departmentNumber'][0] != '') {
				$departments = explode(';', $profile['inetOrgPerson_departmentNumber'][0]);
				// remove extra spaces and set attributes
				$this->attributes['departmentNumber'] = array_map('trim', $departments);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO') && isset($profile['inetOrgPerson_o'][0])) {
			$oList = preg_split('/;[ ]*/', $profile['inetOrgPerson_o'][0]);
			$this->attributes['o'] = $oList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') && isset($profile['inetOrgPerson_mail'][0])) {
			$mailList = preg_split('/;[ ]*/', $profile['inetOrgPerson_mail'][0]);
			$this->attributes['mail'] = $mailList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') && isset($profile['inetOrgPerson_telephoneNumber'][0])) {
			$telephoneNumberList = preg_split('/;[ ]*/', $profile['inetOrgPerson_telephoneNumber'][0]);
			$this->attributes['telephoneNumber'] = $telephoneNumberList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber') && isset($profile['inetOrgPerson_facsimileTelephoneNumber'][0])) {
			$facsimileTelephoneNumberList = preg_split('/;[ ]*/', $profile['inetOrgPerson_facsimileTelephoneNumber'][0]);
			$this->attributes['facsimileTelephoneNumber'] = $facsimileTelephoneNumberList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') && isset($profile['inetOrgPerson_street'][0])) {
			$list = preg_split('/;[ ]*/', $profile['inetOrgPerson_street'][0]);
			$this->attributes['street'] = $list;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox') && isset($profile['inetOrgPerson_postOfficeBox'][0])) {
			$list = preg_split('/;[ ]*/', $profile['inetOrgPerson_postOfficeBox'][0]);
			$this->attributes['postOfficeBox'] = $list;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') && isset($profile['inetOrgPerson_postalCode'][0])) {
			$list = preg_split('/;[ ]*/', $profile['inetOrgPerson_postalCode'][0]);
			$this->attributes['postalCode'] = $list;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation') && isset($profile['inetOrgPerson_l'][0])) {
			$list = preg_split('/;[ ]*/', $profile['inetOrgPerson_l'][0]);
			$this->attributes['l'] = $list;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') && isset($profile['inetOrgPerson_st'][0])) {
			$list = preg_split('/;[ ]*/', $profile['inetOrgPerson_st'][0]);
			$this->attributes['st'] = $list;
		}
	}
	
	/**
	* Checks input values of account profiles.
	*
	* @param array $options a hash array (name => value) containing the options
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	*/
	function check_profileOptions($options) {
		$messages = parent::check_profileOptions($options);
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$telephoneNumberList = preg_split('/;[ ]*/', $options['inetOrgPerson_telephoneNumber'][0]);
			for ($i = 0; $i < sizeof($telephoneNumberList); $i++) {
				if (!get_preg($telephoneNumberList[$i], 'telephone')) {
					$messages[] = $this->messages['telephoneNumber'][0];
					break;
				}
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$facsimileTelephoneNumberList = preg_split('/;[ ]*/', $options['inetOrgPerson_facsimileTelephoneNumber'][0]);
			for ($i = 0; $i < sizeof($facsimileTelephoneNumberList); $i++) {
				if (!get_preg($facsimileTelephoneNumberList[$i], 'telephone')) {
					$messages[] = $this->messages['facsimileNumber'][0];
					break;
				}
			}
		}
		return $messages;
	}

	/**
	* Returns an array containing all input columns for the file upload.
	*
	* Syntax:
	* <br> array(
	* <br>  string: name,  // fixed non-translated name which is used as column name (should be of format: <module name>_<column name>)
	* <br>  string: description,  // short descriptive name
	* <br>  string: help,  // help ID
	* <br>  string: example,  // example value
	* <br>  boolean: required  // true, if user must set a value for this column
	* <br> )
	*
	* @param array $selectedModules list of selected account modules
	* @return array column list
	*/
	function get_uploadColumns($selectedModules) {
		$return = parent::get_uploadColumns($selectedModules);
		// cn and uid for upload (only if posixAccount is not selected)
		if (!in_array('posixAccount', $selectedModules)) {
			$return[] = array(
				'name' => 'inetOrgPerson_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller'),
				'default' => '{inetOrgPerson_firstName} {inetOrgPerson_lastName}'
			);
			$return[] = array(
				'name' => 'inetOrgPerson_userName',
				'description' => _('User name'),
				'help' => 'uid',
				'example' => _('smiller'),
				'unique' => true,
				'required' => true
			);
		}
		return $return;
	}
	
	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$errors = array();
		// get list of existing users
		$existingUsers = searchLDAPByAttribute('uid', '*', 'inetOrgPerson', array('uid'), array('user'));
		for ($e = 0; $e < sizeof($existingUsers); $e++) {
			$existingUsers[$e] = $existingUsers[$e]['uid'][0];
		}
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
			// initials
			if ($rawAccounts[$i][$ids['inetOrgPerson_initials']] != "") {
				$partialAccounts[$i]['initials'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_initials']]);
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
			// employee number
			if ($rawAccounts[$i][$ids['inetOrgPerson_employeeNumber']] != "") {
				$partialAccounts[$i]['employeeNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_employeeNumber']];
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
				$streetList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_street']]);
				$partialAccounts[$i]['street'] = $streetList;
				for ($x = 0; $x < sizeof($streetList); $x++) {
					if (!get_preg($streetList[$x], 'street')) {
						$errMsg = $this->messages['street'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// post office box
			if ($rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']] != "") {
				$partialAccounts[$i]['postOfficeBox'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']]);
			}
			// room number
			if ($rawAccounts[$i][$ids['inetOrgPerson_roomNumber']] != "") {
				$partialAccounts[$i]['roomNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_roomNumber']];
			}
			// departments
			if ($rawAccounts[$i][$ids['inetOrgPerson_departmentNumber']] != "") {
				$partialAccounts[$i]['departmentNumber'] = explode(';', $rawAccounts[$i][$ids['inetOrgPerson_departmentNumber']]);
				// remove extra spaces
				$partialAccounts[$i]['departmentNumber'] = array_map('trim', $partialAccounts[$i]['departmentNumber']);
			}
			// organisation
			if ($rawAccounts[$i][$ids['inetOrgPerson_o']] != "") {
				$partialAccounts[$i]['o'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_o']]);
			}
			// location
			if ($rawAccounts[$i][$ids['inetOrgPerson_l']] != "") {
				$partialAccounts[$i]['l'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_l']]);
			}
			// state
			if ($rawAccounts[$i][$ids['inetOrgPerson_st']] != "") {
				$partialAccounts[$i]['st'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_st']]);
			}
			// physicalDeliveryOfficeName
			if ($rawAccounts[$i][$ids['inetOrgPerson_physicalDeliveryOfficeName']] != "") {
				$partialAccounts[$i]['physicalDeliveryOfficeName'] = $rawAccounts[$i][$ids['inetOrgPerson_physicalDeliveryOfficeName']];
			}
			// carLicense
			if ($rawAccounts[$i][$ids['inetOrgPerson_carLicense']] != "") {
				$partialAccounts[$i]['carLicense'] = $rawAccounts[$i][$ids['inetOrgPerson_carLicense']];
			}
			// postal code
			if ($rawAccounts[$i][$ids['inetOrgPerson_postalCode']] != "") {
				$postalCodeList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_postalCode']]);
				$partialAccounts[$i]['postalCode'] = $postalCodeList;
				for ($x = 0; $x < sizeof($postalCodeList); $x++) {
					if (!get_preg($postalCodeList[$x], 'postalCode')) {
						$errMsg = $this->messages['postalCode'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
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
				$telephoneList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_telephone']]);
				$partialAccounts[$i]['telephoneNumber'] = $telephoneList;
				for ($x = 0; $x < sizeof($telephoneList); $x++) {
					if (!get_preg($telephoneList[$x], 'telephone')) {
						$errMsg = $this->messages['telephoneNumber'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// home telephone
			if ($rawAccounts[$i][$ids['inetOrgPerson_homePhone']] != "") {
				$homePhoneList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_homePhone']]);
				$partialAccounts[$i]['homePhone'] = $homePhoneList;
				for ($x = 0; $x < sizeof($homePhoneList); $x++) {
					if (!get_preg($homePhoneList[$x], 'telephone')) {
						$errMsg = $this->messages['homePhone'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// mobile
			if ($rawAccounts[$i][$ids['inetOrgPerson_mobile']] != "") {
				$mobileList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_mobile']]);
				$partialAccounts[$i]['mobile'] = $mobileList;
				for ($x = 0; $x < sizeof($mobileList); $x++) {
					if (!get_preg($mobileList[$x], 'telephone')) {
						$errMsg = $this->messages['mobileTelephone'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// facsimile
			if ($rawAccounts[$i][$ids['inetOrgPerson_fax']] != "") {
				$facsimileTelephoneNumberList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_fax']]);
				$partialAccounts[$i]['facsimileTelephoneNumber'] = $facsimileTelephoneNumberList;
				for ($x = 0; $x < sizeof($facsimileTelephoneNumberList); $x++) {
					if (!get_preg($facsimileTelephoneNumberList[$x], 'telephone')) {
						$errMsg = $this->messages['facsimileNumber'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// eMail
			if ($rawAccounts[$i][$ids['inetOrgPerson_email']] != "") {
				$mailList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_email']]);
				$partialAccounts[$i]['mail'] = $mailList;
				for ($x = 0; $x < sizeof($mailList); $x++) {
					if (!get_preg($mailList[$x], 'email')) {
						$errMsg = $this->messages['email'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			if (!in_array('posixAccount', $selectedModules)) {
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
	 * @param array $attributes attributes of LDAP account
	 * @return array meta HTML
	 */
	function getSelfServiceOptions($fields, $attributes) {
		$return = array();
		if (in_array('firstName', $fields)) {
			$firstName = '';
			if (isset($attributes['givenName'][0])) $firstName = $attributes['givenName'][0];
			$return['firstName'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('First name'), 'inetOrgPerson_firstName', $firstName)
			));
		}
		if (in_array('lastName', $fields)) {
			$lastName = '';
			if (isset($attributes['sn'][0])) $lastName = $attributes['sn'][0];
			$return['lastName'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Last name'), 'inetOrgPerson_lastName', $lastName)
			));
		}
		if (in_array('mail', $fields)) {
			$mail = '';
			if (isset($attributes['mail'][0])) $mail = $attributes['mail'][0];
			$return['mail'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Email address'), 'inetOrgPerson_mail', $mail)
			));
		}
		if (in_array('telephoneNumber', $fields)) {
			$telephoneNumber = '';
			if (isset($attributes['telephoneNumber'][0])) $telephoneNumber = $attributes['telephoneNumber'][0];
			$return['telephoneNumber'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Telephone number'), 'inetOrgPerson_telephoneNumber', $telephoneNumber)
			));
		}
		if (in_array('homePhone', $fields)) {
			$homePhone = '';
			if (isset($attributes['homePhone'][0])) $homePhone = $attributes['homePhone'][0];
			$return['homePhone'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Home telephone number'), 'inetOrgPerson_homePhone', $homePhone)
			));
		}
		if (in_array('mobile', $fields)) {
			$mobile = '';
			if (isset($attributes['mobile'][0])) $mobile = $attributes['mobile'][0];
			$return['mobile'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Mobile telephone number'), 'inetOrgPerson_mobile', $mobile)
			));
		}
		if (in_array('faxNumber', $fields)) {
			$faxNumber = '';
			if (isset($attributes['facsimileTelephoneNumber'][0])) $faxNumber = $attributes['facsimileTelephoneNumber'][0];
			$return['faxNumber'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Fax number'), 'inetOrgPerson_faxNumber', $faxNumber)
			));
		}
		if (in_array('street', $fields)) {
			$street = '';
			if (isset($attributes['street'][0])) $street = $attributes['street'][0];
			$return['street'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Street'), 'inetOrgPerson_street', $street)
			));
		}
		if (in_array('postalAddress', $fields)) {
			$postalAddress = '';
			if (isset($attributes['postalAddress'][0])) $postalAddress = $attributes['postalAddress'][0];
			$return['postalAddress'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Postal address'), 'inetOrgPerson_postalAddress', $postalAddress)
			));
		}
		if (in_array('postalCode', $fields)) {
			$postalCode = '';
			if (isset($attributes['postalCode'][0])) $postalCode = $attributes['postalCode'][0];
			$return['postalCode'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Postal code'), 'inetOrgPerson_postalCode', $postalCode)
			));
		}
		if (in_array('postOfficeBox', $fields)) {
			$postOfficeBox = '';
			if (isset($attributes['postOfficeBox'][0])) $postOfficeBox = $attributes['postOfficeBox'][0];
			$return['postOfficeBox'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Post office box'), 'inetOrgPerson_postOfficeBox', $postOfficeBox)
			));
		}
		if (in_array('roomNumber', $fields)) {
			$roomNumber = '';
			if (isset($attributes['roomNumber'][0])) $roomNumber = $attributes['roomNumber'][0];
			$return['roomNumber'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Room number'), 'inetOrgPerson_roomNumber', $roomNumber)
			));
		}
		if (in_array('location', $fields)) {
			$l = '';
			if (isset($attributes['l'][0])) $l = $attributes['l'][0];
			$return['location'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Location'), 'inetOrgPerson_location', $l)
			));
		}
		if (in_array('state', $fields)) {
			$st = '';
			if (isset($attributes['st'][0])) $st = $attributes['st'][0];
			$return['state'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('State'), 'inetOrgPerson_state', $st)
			));
		}
		if (in_array('carLicense', $fields)) {
			$carLicense = '';
			if (isset($attributes['carLicense'][0])) $carLicense = $attributes['carLicense'][0];
			$return['carLicense'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Car license'), 'inetOrgPerson_carLicense', $carLicense)
			));
		}
		if (in_array('officeName', $fields)) {
			$physicalDeliveryOfficeName = '';
			if (isset($attributes['physicalDeliveryOfficeName'][0])) $physicalDeliveryOfficeName = $attributes['physicalDeliveryOfficeName'][0];
			$return['officeName'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Office name'), 'inetOrgPerson_officeName', $physicalDeliveryOfficeName)
			));
		}
		if (in_array('businessCategory', $fields)) {
			$businessCategory = '';
			if (isset($attributes['businessCategory'][0])) $businessCategory = $attributes['businessCategory'][0];
			$return['businessCategory'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Business category'), 'inetOrgPerson_businessCategory', $businessCategory)
			));
		}
		if (in_array('jpegPhoto', $fields)) {
			if (isset($attributes['jpegPhoto'][0])) {
				$jpeg_filename = 'jpegPhoto' . session_id() . '.jpg';
				$outjpeg = fopen(realpath('../../') . '/tmp/' . $jpeg_filename, "wb");
				fwrite($outjpeg, $attributes['jpegPhoto'][0]);
				fclose ($outjpeg);
				$photoFile = '../../tmp/' . $jpeg_filename;
				$photoSub = new htmlTable();
				$photoSub->addElement(new htmlImage($photoFile), true);
				$photoSubSub = new htmlTable();
				$photoSubSub->addElement(new htmlTableExtendedInputCheckbox('removeReplacePhoto', false, _('Remove/replace photo'), null, false));
				$photoSubSub->addElement(new htmlInputFileUpload('replacePhotoFile'));
				$photoSub->addElement($photoSubSub);
				$photoRowCells = array(new htmlOutputText(_('Photo')), $photoSub);
				$photoRow = new htmlTableRow($photoRowCells);
				$return['jpegPhoto'] = $photoRow;
			}
			else {
				$photoSub = new htmlTable();
				$photoSub->addElement(new htmlTableExtendedInputFileUpload('photoFile', _('Add photo')));
				$photoRowCells = array(new htmlOutputText(_('Photo')), $photoSub);
				$photoRow = new htmlTableRow($photoRowCells);
				$return['jpegPhoto'] = $photoRow;
			}
		}
		if (in_array('departmentNumber', $fields)) {
			$departmentNumber = '';
			if (isset($attributes['departmentNumber'][0])) $departmentNumber = implode('; ', $attributes['departmentNumber']);
			$return['departmentNumber'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Department(s)'), 'inetOrgPerson_departmentNumber', $departmentNumber)
			));
		}
		if (in_array('initials', $fields)) {
			$initials = '';
			if (isset($attributes['initials'][0])) $initials = implode('; ', $attributes['initials']);
			$return['initials'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Initials'), 'inetOrgPerson_initials', $initials)
			));
		}
		if (in_array('title', $fields)) {
			$title = '';
			if (isset($attributes['title'][0])) $title = $attributes['title'][0];
			$return['title'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Job title'), 'inetOrgPerson_title', $title)
			));
		}
		return $return;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP attributes which should be changed.
	 * <br>Return values:
	 * <br>messages: array of parameters to create status messages
	 * <br>add: array of attributes to add
	 * <br>del: array of attributes to remove
	 * <br>mod: array of attributes to modify
	 * <br>info: array of values with informational value (e.g. to be used later by pre/postModify actions)
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
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
		// l
		if (in_array('location', $fields)) {
			$attributeNames[] = 'l';
			if (isset($_POST['inetOrgPerson_location']) && ($_POST['inetOrgPerson_location'] != '')) {
				$attributesNew['l'][0] = $_POST['inetOrgPerson_location'];
			}
			elseif (isset($attributes['l'])) unset($attributesNew['l']);
		}
		// st
		if (in_array('state', $fields)) {
			$attributeNames[] = 'st';
			if (isset($_POST['inetOrgPerson_state']) && ($_POST['inetOrgPerson_state'] != '')) {
				$attributesNew['st'][0] = $_POST['inetOrgPerson_state'];
			}
			elseif (isset($attributes['st'])) unset($attributesNew['st']);
		}
		// car license
		if (in_array('carLicense', $fields)) {
			$attributeNames[] = 'carLicense';
			if (isset($_POST['inetOrgPerson_carLicense']) && ($_POST['inetOrgPerson_carLicense'] != '')) {
				$attributesNew['carLicense'][0] = $_POST['inetOrgPerson_carLicense'];
			}
			elseif (isset($attributes['carLicense'])) unset($attributesNew['carLicense']);
		}
		// office name
		if (in_array('officeName', $fields)) {
			$attributeNames[] = 'physicalDeliveryOfficeName';
			if (isset($_POST['inetOrgPerson_officeName']) && ($_POST['inetOrgPerson_officeName'] != '')) {
				$attributesNew['physicalDeliveryOfficeName'][0] = $_POST['inetOrgPerson_officeName'];
			}
			elseif (isset($attributes['physicalDeliveryOfficeName'])) unset($attributesNew['physicalDeliveryOfficeName']);
		}
		// business category
		if (in_array('businessCategory', $fields)) {
			$attributeNames[] = 'businessCategory';
			if (isset($_POST['inetOrgPerson_businessCategory']) && ($_POST['inetOrgPerson_businessCategory'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_businessCategory'], 'businessCategory')) {
					$return['messages'][] = $this->messages['businessCategory'][0];
				}
				else {
					$attributesNew['businessCategory'][0] = $_POST['inetOrgPerson_businessCategory'];
				}
			}
			elseif (isset($attributes['businessCategory'])) unset($attributesNew['businessCategory']);
		}
		// photo
		if (in_array('jpegPhoto', $fields)) {
			if (isset($_FILES['photoFile']) && ($_FILES['photoFile']['size'] > 0)) {
				$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
				$data = fread($handle, 1000000);
				fclose($handle);
				$return['add']['jpegPhoto'][0] = $data;
			}
			if (isset($_POST['removeReplacePhoto']) && ($_POST['removeReplacePhoto'] == 'on')) {
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
		// departments
		if (in_array('departmentNumber', $fields)) {
			$attributeNames[] = 'departmentNumber';
			if (isset($_POST['inetOrgPerson_departmentNumber']) && ($_POST['inetOrgPerson_departmentNumber'] != '')) {
				$attributesNew['departmentNumber'] = preg_split('/;[ ]*/', $_POST['inetOrgPerson_departmentNumber']);
			}
			elseif (isset($attributes['departmentNumber'])) unset($attributesNew['departmentNumber']);
		}
		// initials
		if (in_array('initials', $fields)) {
			$attributeNames[] = 'initials';
			if (isset($_POST['inetOrgPerson_initials']) && ($_POST['inetOrgPerson_initials'] != '')) {
				$attributesNew['initials'] = preg_split('/;[ ]*/', $_POST['inetOrgPerson_initials']);
			}
			elseif (isset($attributes['initials'])) unset($attributesNew['initials']);
		}
		// title
		if (in_array('title', $fields)) {
			$attributeNames[] = 'title';
			if (isset($_POST['inetOrgPerson_title']) && ($_POST['inetOrgPerson_title'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_title'], 'title')) $return['messages'][] = $this->messages['title'][0];
				else $attributesNew['title'][0] = $_POST['inetOrgPerson_title'];
			}
			elseif (isset($attributes['title'])) unset($attributesNew['title']);
		}
		// find differences
		for ($i = 0; $i < sizeof($attributeNames); $i++) {
			$attrName = $attributeNames[$i];
			if (isset($attributes[$attrName]) && !isset($attributesNew[$attrName])) $return['del'][$attrName] = $attributes[$attrName];
			elseif (!isset($attributes[$attrName]) && isset($attributesNew[$attrName])) $return['add'][$attrName] = $attributesNew[$attrName];
			else {
				if (isset($attributes[$attrName])) {
					for ($a = 0; $a < sizeof($attributes[$attrName]); $a++) {
						if (!in_array($attributes[$attrName][$a], $attributesNew[$attrName])) {
							$return['mod'][$attrName] = $attributesNew[$attrName];
							break;
						}
					}
				}
				if (isset($attributesNew[$attrName])) {
					for ($a = 0; $a < sizeof($attributesNew[$attrName]); $a++) {
						if (!in_array($attributesNew[$attrName][$a], $attributes[$attrName])) {
							$return['mod'][$attrName] = $attributesNew[$attrName];
							break;
						}
					}
				}
			}
		}
		return $return;
	}
	
	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			return true;
		}
		return false;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$this->clearTextPassword = $password;
		$this->attributes['userPassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		return array();
	}
	
	/**
	 * Returns a list of possible managers.
	 *
	 * @return array list of format array(abstract DN => DN)
	 */
	private function getManagers() {
		if ($this->cachedManagers != null) {
			return $this->cachedManagers;
		}
		$dnUsers = searchLDAPByAttribute(null, null, 'inetOrgPerson', array('dn'), array('user'));
		for ($i = 0; $i < sizeof($dnUsers); $i++) {
			$dnUsers[$i] = $dnUsers[$i]['dn'];
		}
		usort($dnUsers, 'compareDN');
		array_unshift($dnUsers, '-');
		$this->cachedManagers = array();
		for ($i = 0; $i < sizeof($dnUsers); $i++) {
			$this->cachedManagers[getAbstractDN($dnUsers[$i])] = $dnUsers[$i];
		}
		return $this->cachedManagers;		
	}

}

?>
