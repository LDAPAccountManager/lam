<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2013  Roland Gruber

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
	/** clear text password */
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
		$this->messages['registeredAddress'][0] = array('ERROR', _('Registered address'), _('Please enter a valid registered address.'));
		$this->messages['registeredAddress'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_registeredAddress', _('Please enter a valid registered address.'));
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
		$this->messages['file'][0] = array('ERROR', _('No file selected.'));
		$this->messages['businessCategory'][0] = array('ERROR', _('Business category'), _('Please enter a valid business category!'));
		$this->messages['businessCategory'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_businessCategory', _('Please enter a valid business category!'));
		$this->messages['userPassword'][0] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
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
			'mobileTelephoneNumber' => 'mobile', 'organizationName' => 'o', 'organizationalUnitName' => 'ou');
		// managed attributes
		$return['attributes'] = array('uid', 'cn', 'employeeType', 'givenName', 'jpegPhoto', 'mail', 'manager', 'mobile',
			'title', 'telephoneNumber', 'facsimileTelephoneNumber', 'street', 'postOfficeBox', 'postalCode', 'postalAddress',
			'sn', 'userPassword', 'description', 'homePhone', 'roomNumber', 'businessCategory', 'l', 'st', 'physicalDeliveryOfficeName',
			'carLicense', 'departmentNumber', 'o', 'employeeNumber', 'initials', 'registeredAddress', 'labeledURI', 'ou', 'userCertificate;binary');
		// self service search attributes
		$return['selfServiceSearchAttributes'] = array('uid', 'mail', 'cn', 'surname', 'givenName', 'employeeNumber');
		// self service field settings
		$return['selfServiceFieldSettings'] = array('firstName' => _('First name'), 'lastName' => _('Last name'),
			'mail' => _('Email address'), 'telephoneNumber' => _('Telephone number'), 'mobile' => _('Mobile number'),
			'faxNumber' => _('Fax number'), 'street' => _('Street'), 'postalAddress' => _('Postal address'), 'registeredAddress' => _('Registered address'),
			'postalCode' => _('Postal code'), 'postOfficeBox' => _('Post office box'), 'jpegPhoto' => _('Photo'),
			'homePhone' => _('Home telephone number'), 'roomNumber' => _('Room number'), 'carLicense' => _('Car license'),
			'location' => _('Location'), 'state' => _('State'), 'officeName' => _('Office name'), 'businessCategory' => _('Business category'),
			'departmentNumber' => _('Department'), 'initials' => _('Initials'), 'title' => _('Job title'), 'labeledURI' => _('Web site'),
			'userCertificate' => _('User certificates'));
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = array('firstName', 'lastName', 'mail', 'telephoneNumber', 'mobile', 'faxNumber', 'street',
			'postalAddress', 'registeredAddress', 'postalCode', 'postOfficeBox', 'jpegPhoto', 'homePhone', 'roomNumber', 'carLicense',
			'location', 'state', 'officeName', 'businessCategory', 'departmentNumber', 'initials', 'title', 'labeledURI', 'userCertificate');
		// profile elements
		$profileElements = array();
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Initials'), 'inetOrgPerson_initials', null, 'initials');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Description'), 'inetOrgPerson_description', null, 'description');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Street'), 'inetOrgPerson_street', null, 'streetList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Post office box'), 'inetOrgPerson_postOfficeBox', null, 'postOfficeBoxList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Postal code'), 'inetOrgPerson_postalCode', null, 'postalCodeList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Location'), 'inetOrgPerson_l', null, 'lList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$profileElements[] = new htmlTableExtendedInputField(_('State'), 'inetOrgPerson_st', null, 'stList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Postal address'), 'inetOrgPerson_postalAddress', null, 'postalAddress');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Registered address'), 'inetOrgPerson_registeredAddress', null, 'registeredAddress');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Office name'), 'inetOrgPerson_physicalDeliveryOfficeName', null, 'physicalDeliveryOfficeName');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Room number'), 'inetOrgPerson_roomNumber', null, 'roomNumber');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Telephone number'), 'inetOrgPerson_telephoneNumber', null, 'telephoneNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Fax number'), 'inetOrgPerson_facsimileTelephoneNumber', null, 'facsimileTelephoneNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Email address'), 'inetOrgPerson_mail', null, 'mailList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Web site'), 'inetOrgPerson_labeledURI', null, 'labeledURIList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Department'), 'inetOrgPerson_departmentNumber', null, 'departmentNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Organisational unit'), 'inetOrgPerson_ou', null, 'ouList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Organisation'), 'inetOrgPerson_o', null, 'oList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Job title'), 'inetOrgPerson_title', null, 'title');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Employee type'), 'inetOrgPerson_employeeType', null, 'employeeType');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
		    $profileElements[] = new htmlTableExtendedInputField(_('Business category'), 'inetOrgPerson_businessCategory', null, 'businessCategory');
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$return['profile_mappings']['inetOrgPerson_initials'] = 'initials';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$return['profile_mappings']['inetOrgPerson_description'] = 'description';
		}
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
		    $return['profile_checks']['inetOrgPerson_businessCategory'] = array(
		    	'type' => 'ext_preg',
		    	'regex' => 'businessCategory',
		    	'error_message' => $this->messages['businessCategory'][0]);
		    $return['profile_mappings']['inetOrgPerson_businessCategory'] = 'businessCategory';
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$return['profile_checks']['inetOrgPerson_registeredAddress'] = array(
				'type' => 'ext_preg',
				'regex' => 'postalAddress',
				'error_message' => $this->messages['registeredAddress'][0]);
			$return['profile_mappings']['inetOrgPerson_registeredAddress'] = 'registeredAddress';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$return['profile_mappings']['inetOrgPerson_physicalDeliveryOfficeName'] = 'physicalDeliveryOfficeName';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$return['profile_mappings']['inetOrgPerson_roomNumber'] = 'roomNumber';
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
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideLocation', false, _('Location'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideState', false, _('State'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidePostalAddress', false, _('Postal address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideRegisteredAddress', false, _('Registered address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideOfficeName', false, _('Office name'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideRoomNumber', false, _('Room number'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideTelephoneNumber', false, _('Telephone number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideHomeTelephoneNumber', false, _('Home telephone number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideMobileNumber', false, _('Mobile number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideFaxNumber', false, _('Fax number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEMailAddress', false, _('Email address'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideJobTitle', false, _('Job title'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideCarLicense', false, _('Car license'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeType', false, _('Employee type'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideBusinessCategory', false, _('Business category'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideDepartments', false, _('Department'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideManager', false, _('Manager'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideOu', false, _('Organisational unit'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideO', false, _('Organisation'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeNumber', false, _('Employee number'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideInitials', false, _('Initials'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideLabeledURI', false, _('Web site'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideuserCertificate', false, _('User certificates'), null, false));
		if (isset($_SESSION['conf_config'])) {
			// add password hash type if posixAccount is inactive
			$confActiveUnixModules = array_merge($_SESSION['conf_config']->get_AccountModules('user'), $_SESSION['conf_config']->get_AccountModules('host'), $_SESSION['conf_config']->get_AccountModules('group'));
			if (!in_array('posixAccount', $confActiveUnixModules) && !in_array('posixGroup', $confActiveUnixModules)) {
				$optionsSelected = array('SSHA');
				$hashOption = new htmlTable();
				$hashOption->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', getSupportedHashTypes(), $optionsSelected, _("Password hash type"), 'pwdHash'));
				$configContainer->addElement($hashOption);
			}
			$confActiveUnixUserModules = $_SESSION['conf_config']->get_AccountModules('user');
			// option to hide uid
			if (!in_array('posixAccount', $confActiveUnixUserModules)) {
				$configContainerOptions->addElement(new htmlOutputText(' '));
				$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideUID', false, _('User name'), null, false));
			}
		}
		$configContainer->addElement($configContainerOptions, true);
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
				'help' => 'titleList',
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
				'help' => 'managerList',
				'example' => _('uid=smiller,ou=People,dc=company,dc=com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_businessCategory',
				'description' => _('Business category'),
				'help' => 'businessCategoryList',
				'example' => _('Administration')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_street',
				'description' => _('Street'),
				'help' => 'streetList',
				'example' => _('Mystreetname 42')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_postalCode',
				'description' => _('Postal code'),
				'help' => 'postalCodeList',
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_registeredAddress',
				'description' => _('Registered address'),
				'help' => 'registeredAddress',
				'example' => _('Mycity')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_postOfficeBox',
				'description' => _('Post office box'),
				'help' => 'postOfficeBoxList',
				'example' => _('12345')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_telephone',
				'description' => _('Telephone number'),
				'help' => 'telephoneNumberList',
				'example' => _('123-123-1234')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_homePhone',
				'description' => _('Home telephone number'),
				'help' => 'homePhoneList',
				'example' => _('123-124-1234')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_mobile',
				'description' => _('Mobile number'),
				'help' => 'mobileTelephoneNumberList',
				'example' => _('123-123-1235')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_fax',
				'description' => _('Fax number'),
				'help' => 'facsimileTelephoneNumberList',
				'example' => _('123-123-1236')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_email',
				'description' => _('Email address'),
				'help' => 'mailList',
				'example' => _('user@company.com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_labeledURI',
				'description' => _('Web site'),
				'help' => 'labeledURIList',
				'example' => _('http://www.company.com')
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
				'description' => _('Department'),
				'help' => 'departmentNumberList',
				'example' => _('Administration')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_ou',
				'description' => _('Organisational unit'),
				'help' => 'ouList',
				'example' => _('Administration')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_o',
				'description' => _('Organisation'),
				'help' => 'oList',
				'example' => _('YourCompany')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_l',
				'description' => _('Location'),
				'help' => 'lList',
				'example' => _('MyCity')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_st',
				'description' => _('State'),
				'help' => 'stList',
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
				'help' => 'physicalDeliveryOfficeNameList',
				'example' => _('YourCompany')
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$return['PDF_fields']['registeredAddress'] = _('Registered address');
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$return['PDF_fields']['labeledURI'] = _('Web site');
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
			$return['PDF_fields']['departmentNumber'] = _('Department');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$return['PDF_fields']['manager'] = _('Manager');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$return['PDF_fields']['o'] = _('Organisation');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$return['PDF_fields']['ou'] = _('Organisational unit');
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
				if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
					$return['PDF_fields']['uid'] = _('User name');
				}
				$return['PDF_fields']['cn'] = _('Common name');
			}
		}
		// help Entries
		$return['help'] = array (
			'description' => array (
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("User description. If left empty sur- and give name will be used.")
			),
			'title' => array (
				"Headline" => _("Job title"), 'attr' => 'title',
				"Text" => _("Job title of user: President, department manager, ...")
			),
			'titleList' => array (
				"Headline" => _("Job title"), 'attr' => 'title',
				"Text" => _("Job title of user: President, department manager, ...") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'givenName' => array (
				"Headline" => _("First name"), 'attr' => 'givenName',
				"Text" => _("First name of user. Only letters, - and spaces are allowed.")
			),
			'sn' => array (
				"Headline" => _("Last name"), 'attr' => 'sn',
				"Text" => _("Last name of user. Only letters, - and spaces are allowed.")
			),
			'employeeType' => array (
				"Headline" => _("Employee type"), 'attr' => 'employeeType',
				"Text" => _("Employee type: Contractor, Employee, Intern, Temp, External, ...")
			),
			'manager' => array (
				"Headline" => _("Manager"), 'attr' => 'manager',
				"Text" => _("This is the LDAP DN of the user's manager. Use this property to represent hierarchies in your company.")
			),
			'managerList' => array (
				"Headline" => _("Manager"), 'attr' => 'manager',
				"Text" => _("This is the LDAP DN of the user's manager. Use this property to represent hierarchies in your company.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'street' => array (
				"Headline" => _("Street"), 'attr' => 'street',
				"Text" => _("The street name of the user's address.")
			),
			'streetList' => array (
				"Headline" => _("Street"), 'attr' => 'street',
				"Text" => _("The street name of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postOfficeBox' => array (
				"Headline" => _("Post office box"), 'attr' => 'postOfficeBox',
				"Text" => _("The post office box of the user's address.")
			),
			'postOfficeBoxList' => array (
				"Headline" => _("Post office box"), 'attr' => 'postOfficeBox',
				"Text" => _("The post office box of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postalCode' => array (
				"Headline" => _("Postal code"), 'attr' => 'postalCode',
				"Text" => _("The postal code of the user's address.")
			),
			'postalCodeList' => array (
				"Headline" => _("Postal code"), 'attr' => 'postalCode',
				"Text" => _("The postal code of the user's address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'postalAddress' => array (
				"Headline" => _("Postal address"), 'attr' => 'postalAddress',
				"Text" => _("Postal address, city")
			),
			'registeredAddress' => array (
				"Headline" => _("Registered address"), 'attr' => 'registeredAddress',
				"Text" => _("Registered address, city")
			),
			'telephoneNumber' => array (
				"Headline" => _("Telephone number"), 'attr' => 'telephoneNumber',
				"Text" => _("The user's telephone number.")
			),
			'telephoneNumberList' => array (
				"Headline" => _("Telephone number"), 'attr' => 'telephoneNumber',
				"Text" => _("The user's telephone number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mobileTelephoneNumber' => array (
				"Headline" => _("Mobile number"), 'attr' => 'mobile',
				"Text" => _("The user's mobile number.")
			),
			'mobileTelephoneNumberList' => array (
				"Headline" => _("Mobile number"), 'attr' => 'mobile',
				"Text" => _("The user's mobile number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'facsimileTelephoneNumber' => array (
				"Headline" => _("Fax number"), 'attr' => 'facsimileTelephoneNumber',
				"Text" => _("The user's fax number.")
			),
			'facsimileTelephoneNumberList' => array (
				"Headline" => _("Fax number"), 'attr' => 'facsimileTelephoneNumber',
				"Text" => _("The user's fax number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mail' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The user's email address.")
			),
			'mailList' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The user's email address.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'labeledURI' => array(
				"Headline" => _("Web site"), 'attr' => 'labeledURI',
				"Text" => _("The user's web site (e.g. http://www.company.com).")
			),
			'labeledURIList' => array(
				"Headline" => _("Web site"), 'attr' => 'labeledURI',
				"Text" => _("The user's web site (e.g. http://www.company.com).") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'cn' => array (
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("This is the natural name of the user. If empty, the first and last name is used.")
			),
			'uid' => array(
				"Headline" => _("User name"), 'attr' => 'uid',
				"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, @.-_.")
			),
			'photoUpload' => array(
				"Headline" => _("Add photo"), 'attr' => 'jpegPhoto',
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			),
			'homePhone' => array(
				"Headline" => _("Home telephone number"), 'attr' => 'homePhone',
				"Text" => _("The user's private telephone number.")
			),
			'homePhoneList' => array(
				"Headline" => _("Home telephone number"), 'attr' => 'homePhone',
				"Text" => _("The user's private telephone number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'roomNumber' => array(
				"Headline" => _("Room number"), 'attr' => 'roomNumber',
				"Text" => _("The room number of the employee's office.")
			),
			'businessCategory' => array(
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)")
			),
			'businessCategoryList' => array(
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)") . '. ' . _("Multiple values are separated by semicolon.")
			),
			'l' => array(
				"Headline" => _("Location"), 'attr' => 'l',
				"Text" => _("This describes the location of the user.")
			),
			'lList' => array(
				"Headline" => _("Location"), 'attr' => 'l',
				"Text" => _("This describes the location of the user.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'st' => array(
				"Headline" => _("State"), 'attr' => 'st',
				"Text" => _("The state where the user resides or works.")
			),
			'stList' => array(
				"Headline" => _("State"), 'attr' => 'st',
				"Text" => _("The state where the user resides or works.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'carLicense' => array(
				"Headline" => _("Car license"), 'attr' => 'carLicense',
				"Text" => _("This can be used to specify if the user has a car license.")
			),
			'physicalDeliveryOfficeName' => array(
				"Headline" => _("Office name"), 'attr' => 'physicalDeliveryOfficeName',
				"Text" => _("The office name of the user (e.g. YourCompany, Human Resources).")
			),
			'physicalDeliveryOfficeNameList' => array(
				"Headline" => _("Office name"), 'attr' => 'physicalDeliveryOfficeName',
				"Text" => _("The office name of the user (e.g. YourCompany, Human Resources).") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'departmentNumber' => array(
				"Headline" => _("Department"), 'attr' => 'departmentNumber',
				"Text" => _("Here you can enter the user's department.")
			),
			'departmentNumberList' => array(
				"Headline" => _("Department"), 'attr' => 'departmentNumber',
				"Text" => _("Here you can enter the user's department.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, CRYPT-SHA512, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'ou' => array(
				"Headline" => _("Organisational unit"), 'attr' => 'ou',
				"Text" => _("The user's organisational unit.")
			),
			'ouList' => array(
				"Headline" => _("Organisational unit"), 'attr' => 'ou',
				"Text" => _("The user's organisational unit.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'o' => array(
				"Headline" => _("Organisation"), 'attr' => 'o',
				"Text" => _("The user's organisation name.")
			),
			'oList' => array(
				"Headline" => _("Organisation"), 'attr' => 'o',
				"Text" => _("The user's organisation name.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'employeeNumber' => array(
				"Headline" => _("Employee number"), 'attr' => 'employeeNumber',
				"Text" => _("The user's unique employee number.")
			),
			'initials' => array(
				"Headline" => _("Initials"), 'attr' => 'initials',
				"Text" => _("The initials of the user's first names.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'userPassword' => array(
				"Headline" => _("Password"),
				"Text" => _("Please enter the password which you want to set for this account.")
			),
			'userCertificate' => array(
				"Headline" => _('User certificates'),
				"Text" => _('These are the user\'s certificates.')
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
		// postalAddress, registeredAddress, facsimileTelephoneNumber and jpegPhoto need special removing
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['postalAddress'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['postalAddress'] = $this->attributes['postalAddress'];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['postalAddress']);
		}
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['registeredAddress'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['registeredAddress'] = $this->attributes['registeredAddress'];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['registeredAddress']);
		}
		if (isset($return[$this->getAccountContainer()->dn_orig]['remove']['facsimileTelephoneNumber'])) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['facsimileTelephoneNumber'] = $this->attributes['facsimileTelephoneNumber'];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['facsimileTelephoneNumber']);
		}
		if (isset($return[$this->getAccountContainer()->dn_orig]['add']['facsimileTelephoneNumber'])
			&& isset($this->orig['facsimileTelephoneNumber']) && (sizeof($this->orig['facsimileTelephoneNumber']) > 0)) {
			$return[$this->getAccountContainer()->dn_orig]['modify']['facsimileTelephoneNumber'] = $this->attributes['facsimileTelephoneNumber'];
			unset($return[$this->getAccountContainer()->dn_orig]['add']['facsimileTelephoneNumber']);
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
		// load and check attributes
		// description
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$descriptionCounter = 0;
			while (isset($_POST['description' . $descriptionCounter])) {
				$this->attributes['description'][$descriptionCounter] = $_POST['description' . $descriptionCounter];
				if ($this->attributes['description'][$descriptionCounter] == '') {
					unset($this->attributes['description'][$descriptionCounter]);
				}
				$descriptionCounter++;
			}
			if (isset($_POST['addDescription'])) {
				$this->attributes['description'][] = '';
			}
			$this->attributes['description'] = array_values($this->attributes['description']);
		}
		$this->attributes['sn'][0] = $_POST['sn'];
		if (!get_preg($this->attributes['sn'][0], 'realname')) $errors[] = $this->messages['lastname'][0];
		$this->attributes['givenName'][0] = $_POST['givenName'];
		if (($this->attributes['givenName'][0] != '') && !get_preg($this->attributes['givenName'][0], 'realname')) $errors[] = $this->messages['givenName'][0];
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$titleCounter = 0;
			while (isset($_POST['title' . $titleCounter])) {
				$this->attributes['title'][$titleCounter] = $_POST['title' . $titleCounter];
				if ($this->attributes['title'][$titleCounter] == '') {
					unset($this->attributes['title'][$titleCounter]);
				}
				if (!get_preg($this->attributes['title'][$titleCounter], 'title')) {
					$errors[] = $this->messages['title'][0];
				}
				$titleCounter++;
			}
			if (isset($_POST['addtitle'])) {
				$this->attributes['title'][] = '';
			}
			$this->attributes['title'] = array_values($this->attributes['title']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$mailCounter = 0;
			while (isset($_POST['mail' . $mailCounter])) {
				$this->attributes['mail'][$mailCounter] = $_POST['mail' . $mailCounter];
				if (($_POST['mail' . $mailCounter] != '') && !get_preg($this->attributes['mail'][$mailCounter], 'email')) $errors[] = $this->messages['email'][0];
				if ($this->attributes['mail'][$mailCounter] == '') {
					unset($this->attributes['mail'][$mailCounter]);
				}
				$mailCounter++;
			}
			if (isset($_POST['addMail'])) {
				$this->attributes['mail'][] = '';
			}
			$this->attributes['mail'] = array_values($this->attributes['mail']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$telephoneNumberCounter = 0;
			while (isset($_POST['telephoneNumber' . $telephoneNumberCounter])) {
				$this->attributes['telephoneNumber'][$telephoneNumberCounter] = $_POST['telephoneNumber' . $telephoneNumberCounter];
				if (!get_preg($this->attributes['telephoneNumber'][$telephoneNumberCounter], 'telephone')) $errors[] = $this->messages['telephoneNumber'][0];
				if ($this->attributes['telephoneNumber'][$telephoneNumberCounter] == '') {
					unset($this->attributes['telephoneNumber'][$telephoneNumberCounter]);
				}
				$telephoneNumberCounter++;
			}
			if (isset($_POST['addTelephoneNumber'])) {
				$this->attributes['telephoneNumber'][] = '';
			}
			$this->attributes['telephoneNumber'] = array_values($this->attributes['telephoneNumber']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$mobileCounter = 0;
			while (isset($_POST['mobile' . $mobileCounter])) {
				$this->attributes['mobile'][$mobileCounter] = $_POST['mobile' . $mobileCounter];
				if (!get_preg($this->attributes['mobile'][$mobileCounter], 'telephone')) $errors[] = $this->messages['mobileTelephone'][0];
				if ($this->attributes['mobile'][$mobileCounter] == '') {
					unset($this->attributes['mobile'][$mobileCounter]);
				}
				$mobileCounter++;
			}
			if (isset($_POST['addMobile'])) {
				$this->attributes['mobile'][] = '';
			}
			$this->attributes['mobile'] = array_values($this->attributes['mobile']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$facsimileTelephoneNumberCounter = 0;
			while (isset($_POST['facsimileTelephoneNumber' . $facsimileTelephoneNumberCounter])) {
				$this->attributes['facsimileTelephoneNumber'][$facsimileTelephoneNumberCounter] = $_POST['facsimileTelephoneNumber' . $facsimileTelephoneNumberCounter];
				if (!get_preg($this->attributes['facsimileTelephoneNumber'][$facsimileTelephoneNumberCounter], 'telephone')) $errors[] = $this->messages['facsimileNumber'][0];
				if ($this->attributes['facsimileTelephoneNumber'][$facsimileTelephoneNumberCounter] == '') {
					unset($this->attributes['facsimileTelephoneNumber'][$facsimileTelephoneNumberCounter]);
				}
				$facsimileTelephoneNumberCounter++;
			}
			if (isset($_POST['addFacsimileTelephoneNumber'])) {
				$this->attributes['facsimileTelephoneNumber'][] = '';
			}
			$this->attributes['facsimileTelephoneNumber'] = array_values($this->attributes['facsimileTelephoneNumber']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$homePhoneCounter = 0;
			while (isset($_POST['homePhone' . $homePhoneCounter])) {
				$this->attributes['homePhone'][$homePhoneCounter] = $_POST['homePhone' . $homePhoneCounter];
				if (!get_preg($this->attributes['homePhone'][$homePhoneCounter], 'telephone')) $errors[] = $this->messages['homePhone'][0];
				if ($this->attributes['homePhone'][$homePhoneCounter] == '') {
					unset($this->attributes['homePhone'][$homePhoneCounter]);
				}
				$homePhoneCounter++;
			}
			if (isset($_POST['addHomePhone'])) {
				$this->attributes['homePhone'][] = '';
			}
			$this->attributes['homePhone'] = array_values($this->attributes['homePhone']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$streetCounter = 0;
			while (isset($_POST['street' . $streetCounter])) {
				$this->attributes['street'][$streetCounter] = $_POST['street' . $streetCounter];
				if (!get_preg($this->attributes['street'][$streetCounter], 'street')) $errors[] = $this->messages['street'][0];
				if ($this->attributes['street'][$streetCounter] == '') {
					unset($this->attributes['street'][$streetCounter]);
				}
				$streetCounter++;
			}
			if (isset($_POST['addStreet'])) {
				$this->attributes['street'][] = '';
			}
			$this->attributes['street'] = array_values($this->attributes['street']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$postOfficeBoxCounter = 0;
			while (isset($_POST['postOfficeBox' . $postOfficeBoxCounter])) {
				$this->attributes['postOfficeBox'][$postOfficeBoxCounter] = $_POST['postOfficeBox' . $postOfficeBoxCounter];
				if ($this->attributes['postOfficeBox'][$postOfficeBoxCounter] == '') {
					unset($this->attributes['postOfficeBox'][$postOfficeBoxCounter]);
				}
				$postOfficeBoxCounter++;
			}
			if (isset($_POST['addPostOfficeBox'])) {
				$this->attributes['postOfficeBox'][] = '';
			}
			$this->attributes['postOfficeBox'] = array_values($this->attributes['postOfficeBox']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$postalCodeCounter = 0;
			while (isset($_POST['postalCode' . $postalCodeCounter])) {
				$this->attributes['postalCode'][$postalCodeCounter] = $_POST['postalCode' . $postalCodeCounter];
				if (!get_preg($this->attributes['postalCode'][$postalCodeCounter], 'postalCode')) $errors[] = $this->messages['postalCode'][0];
				if ($this->attributes['postalCode'][$postalCodeCounter] == '') {
					unset($this->attributes['postalCode'][$postalCodeCounter]);
				}
				$postalCodeCounter++;
			}
			if (isset($_POST['addPostalCode'])) {
				$this->attributes['postalCode'][] = '';
			}
			$this->attributes['postalCode'] = array_values($this->attributes['postalCode']);
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$labeledURICounter = 0;
			while (isset($_POST['labeledURI' . $labeledURICounter])) {
				$this->attributes['labeledURI'][$labeledURICounter] = $_POST['labeledURI' . $labeledURICounter];
				if ($this->attributes['labeledURI'][$labeledURICounter] == '') {
					unset($this->attributes['labeledURI'][$labeledURICounter]);
				}
				$labeledURICounter++;
			}
			if (isset($_POST['addLabeledURI'])) {
				$this->attributes['labeledURI'][] = '';
			}
			$this->attributes['labeledURI'] = array_values($this->attributes['labeledURI']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$addressCounter = 0;
			while (isset($_POST['registeredAddress' . $addressCounter])) {
				$this->attributes['registeredAddress'][$addressCounter] = implode('$', preg_split('/[\r][\n]/', $_POST['registeredAddress' . $addressCounter]));
				if (!get_preg($this->attributes['registeredAddress'][$addressCounter], 'postalAddress')) $errors[] = $this->messages['registeredAddress'][0];
				if ($this->attributes['registeredAddress'][$addressCounter] == '') {
					unset($this->attributes['registeredAddress'][$addressCounter]);
				}
				$addressCounter++;
			}
			if (isset($_POST['addRegisteredAddress'])) {
				$this->attributes['registeredAddress'][] = '';
			}
			$this->attributes['registeredAddress'] = array_values($this->attributes['registeredAddress']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$this->attributes['employeeType'][0] = $_POST['employeeType'];
			if (!get_preg($this->attributes['employeeType'][0], 'employeeType')) $errors[] = $this->messages['employeeType'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$this->attributes['roomNumber'][0] = $_POST['roomNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$lCounter = 0;
			while (isset($_POST['l' . $lCounter])) {
				$this->attributes['l'][$lCounter] = $_POST['l' . $lCounter];
				if ($this->attributes['l'][$lCounter] == '') {
					unset($this->attributes['l'][$lCounter]);
				}
				$lCounter++;
			}
			if (isset($_POST['addL'])) {
				$this->attributes['l'][] = '';
			}
			$this->attributes['l'] = array_values($this->attributes['l']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$stCounter = 0;
			while (isset($_POST['st' . $stCounter])) {
				$this->attributes['st'][$stCounter] = $_POST['st' . $stCounter];
				if ($this->attributes['st'][$stCounter] == '') {
					unset($this->attributes['st'][$stCounter]);
				}
				$stCounter++;
			}
			if (isset($_POST['addSt'])) {
				$this->attributes['st'][] = '';
			}
			$this->attributes['st'] = array_values($this->attributes['st']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$this->attributes['carLicense'][0] = $_POST['carLicense'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$physicalDeliveryOfficeNameCounter = 0;
			while (isset($_POST['physicalDeliveryOfficeName' . $physicalDeliveryOfficeNameCounter])) {
				$this->attributes['physicalDeliveryOfficeName'][$physicalDeliveryOfficeNameCounter] = $_POST['physicalDeliveryOfficeName' . $physicalDeliveryOfficeNameCounter];
				if ($this->attributes['physicalDeliveryOfficeName'][$physicalDeliveryOfficeNameCounter] == '') {
					unset($this->attributes['physicalDeliveryOfficeName'][$physicalDeliveryOfficeNameCounter]);
				}
				$physicalDeliveryOfficeNameCounter++;
			}
			if (isset($_POST['addPhysicalDeliveryOfficeName'])) {
				$this->attributes['physicalDeliveryOfficeName'][] = '';
			}
			$this->attributes['physicalDeliveryOfficeName'] = array_values($this->attributes['physicalDeliveryOfficeName']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$businessCategoryCounter = 0;
			while (isset($_POST['businessCategory' . $businessCategoryCounter])) {
				$this->attributes['businessCategory'][$businessCategoryCounter] = $_POST['businessCategory' . $businessCategoryCounter];
				if ($this->attributes['businessCategory'][$businessCategoryCounter] == '') {
					unset($this->attributes['businessCategory'][$businessCategoryCounter]);
				}
				if (!get_preg($this->attributes['businessCategory'][$businessCategoryCounter], 'businessCategory')) {
					$errors[] = $this->messages['businessCategory'][0];
				}
				$businessCategoryCounter++;
			}
			if (isset($_POST['addbusinessCategory'])) {
				$this->attributes['businessCategory'][] = '';
			}
			$this->attributes['businessCategory'] = array_values($this->attributes['businessCategory']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$departmentNumberCounter = 0;
			while (isset($_POST['departmentNumber' . $departmentNumberCounter])) {
				$this->attributes['departmentNumber'][$departmentNumberCounter] = $_POST['departmentNumber' . $departmentNumberCounter];
				if ($this->attributes['departmentNumber'][$departmentNumberCounter] == '') {
					unset($this->attributes['departmentNumber'][$departmentNumberCounter]);
				}
				$departmentNumberCounter++;
			}
			if (isset($_POST['addDepartmentNumber'])) {
				$this->attributes['departmentNumber'][] = '';
			}
			$this->attributes['departmentNumber'] = array_values($this->attributes['departmentNumber']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$this->attributes['employeeNumber'][0] = $_POST['employeeNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$ouCounter = 0;
			while (isset($_POST['ou' . $ouCounter])) {
				$this->attributes['ou'][$ouCounter] = $_POST['ou' . $ouCounter];
				if ($this->attributes['ou'][$ouCounter] == '') {
					unset($this->attributes['ou'][$ouCounter]);
				}
				$ouCounter++;
			}
			if (isset($_POST['addOu'])) {
				$this->attributes['ou'][] = '';
			}
			$this->attributes['ou'] = array_values($this->attributes['ou']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$oCounter = 0;
			while (isset($_POST['o' . $oCounter])) {
				$this->attributes['o'][$oCounter] = $_POST['o' . $oCounter];
				if ($this->attributes['o'][$oCounter] == '') {
					unset($this->attributes['o'][$oCounter]);
				}
				$oCounter++;
			}
			if (isset($_POST['addO'])) {
				$this->attributes['o'][] = '';
			}
			$this->attributes['o'] = array_values($this->attributes['o']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$this->attributes['initials'] = preg_split('/;[ ]*/', $_POST['initials']);
		}
		
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			// uid
			if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
				if (isset($_POST['userName']) && ($_POST['userName'] != '')) {
					if (!get_preg($_POST['userName'], 'username')) {
						$errors[] = $this->messages['uid'][0];
					}
					else {
						$this->attributes['uid'][0] = $_POST['userName'];
					}
				}
				elseif (isset($this->attributes['uid'][0])) {
					unset($this->attributes['uid'][0]);
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
		if (!in_array('posixAccount', $modules) && !$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
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
		// description
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$descriptions = array();
			if (isset($this->attributes['description'][0])) {
				$descriptions = $this->attributes['description'];
			}
			if (sizeof($descriptions) == 0) {
				$descriptions[] = '';
			}
			$descriptionLabel = new htmlOutputText(_('Description'));
			$descriptionLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($descriptionLabel);
			$descriptionContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($descriptions); $i++) {
				$descriptionContainer->addElement(new htmlInputField('description' . $i, $descriptions[$i]));
				if ($i < (sizeof($descriptions) - 1)) {
					$descriptionContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$descriptionContainer->addElement(new htmlButton('addDescription', 'add.png', true));
				}
			}
			$fieldContainer->addElement($descriptionContainer);
			$descriptionHelp = new htmlHelpLink('description');
			$descriptionHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($descriptionHelp, true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Address')), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$streets = array();
			if (isset($this->attributes['street'][0])) {
				$streets = $this->attributes['street'];
			}
			if (sizeof($streets) == 0) {
				$streets[] = '';
			}
			$streetLabel = new htmlOutputText(_('Street'));
			$streetLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($streetLabel);
			$streetContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($streets); $i++) {
				$streetContainer->addElement(new htmlInputField('street' . $i, $streets[$i]));
				if ($i < (sizeof($streets) - 1)) {
					$streetContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$streetContainer->addElement(new htmlButton('addStreet', 'add.png', true));
				}
			}
			$fieldContainer->addElement($streetContainer);
			$streetHelp = new htmlHelpLink('street');
			$streetHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($streetHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$postOfficeBoxes = array();
			if (isset($this->attributes['postOfficeBox'][0])) {
				$postOfficeBoxes = $this->attributes['postOfficeBox'];
			}
			if (sizeof($postOfficeBoxes) == 0) {
				$postOfficeBoxes[] = '';
			}
			$postOfficeBoxLabel = new htmlOutputText(_('Post office box'));
			$postOfficeBoxLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($postOfficeBoxLabel);
			$postOfficeBoxContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($postOfficeBoxes); $i++) {
				$postOfficeBoxContainer->addElement(new htmlInputField('postOfficeBox' . $i, $postOfficeBoxes[$i]));
				if ($i < (sizeof($postOfficeBoxes) - 1)) {
					$postOfficeBoxContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$postOfficeBoxContainer->addElement(new htmlButton('addPostOfficeBox', 'add.png', true));
				}
			}
			$fieldContainer->addElement($postOfficeBoxContainer);
			$postOfficeBoxHelp = new htmlHelpLink('postOfficeBox');
			$postOfficeBoxHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($postOfficeBoxHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$postalCodes = array();
			if (isset($this->attributes['postalCode'][0])) {
				$postalCodes = $this->attributes['postalCode'];
			}
			if (sizeof($postalCodes) == 0) {
				$postalCodes[] = '';
			}
			$postalCodeLabel = new htmlOutputText(_('Postal code'));
			$postalCodeLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($postalCodeLabel);
			$postalCodeContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($postalCodes); $i++) {
				$postalCodeContainer->addElement(new htmlInputField('postalCode' . $i, $postalCodes[$i]));
				if ($i < (sizeof($postalCodes) - 1)) {
					$postalCodeContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$postalCodeContainer->addElement(new htmlButton('addPostalCode', 'add.png', true));
				}
			}
			$fieldContainer->addElement($postalCodeContainer);
			$postalCodeHelp = new htmlHelpLink('postalCode');
			$postalCodeHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($postalCodeHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$locations = array();
			if (isset($this->attributes['l'][0])) {
				$locations = $this->attributes['l'];
			}
			if (sizeof($locations) == 0) {
				$locations[] = '';
			}
			$locationLabel = new htmlOutputText(_('Location'));
			$locationLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($locationLabel);
			$locationContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($locations); $i++) {
				$locationContainer->addElement(new htmlInputField('l' . $i, $locations[$i]));
				if ($i < (sizeof($locations) - 1)) {
					$locationContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$locationContainer->addElement(new htmlButton('addL', 'add.png', true));
				}
			}
			$fieldContainer->addElement($locationContainer);
			$locationHelp = new htmlHelpLink('l');
			$locationHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($locationHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$states = array();
			if (isset($this->attributes['st'][0])) {
				$states = $this->attributes['st'];
			}
			if (sizeof($states) == 0) {
				$states[] = '';
			}
			$stateLabel = new htmlOutputText(_('State'));
			$stateLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($stateLabel);
			$stateContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($states); $i++) {
				$stateContainer->addElement(new htmlInputField('st' . $i, $states[$i]));
				if ($i < (sizeof($states) - 1)) {
					$stateContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$stateContainer->addElement(new htmlButton('addSt', 'add.png', true));
				}
			}
			$fieldContainer->addElement($stateContainer);
			$stateHelp = new htmlHelpLink('st');
			$stateHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($stateHelp, true);
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$registeredAddresses = array();
			if (isset($this->attributes['registeredAddress'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['registeredAddress']); $i++) {
					$registeredAddresses[] = implode("\r\n", explode('$', $this->attributes['registeredAddress'][$i]));
				}
			}
			if (sizeof($registeredAddresses) == 0) {
				$registeredAddresses[] = '';
			}
			$registeredAddressLabel = new htmlOutputText(_('Registered address'));
			$registeredAddressLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($registeredAddressLabel);
			$registeredAddressContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($registeredAddresses); $i++) {
				$registeredAddressContainer->addElement(new htmlInputTextarea('registeredAddress' . $i, $registeredAddresses[$i], 30, 3));
				if ($i < (sizeof($registeredAddresses) - 1)) {
					$registeredAddressContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$registeredAddressContainer->addElement(new htmlButton('addRegisteredAddress', 'add.png', true));
				}
			}
			$fieldContainer->addElement($registeredAddressContainer);
			$registeredAddressHelp = new htmlHelpLink('registeredAddress');
			$registeredAddressHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($registeredAddressHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$physicalDeliveryOfficeNames = array();
			if (isset($this->attributes['physicalDeliveryOfficeName'][0])) {
				$physicalDeliveryOfficeNames = $this->attributes['physicalDeliveryOfficeName'];
			}
			if (sizeof($physicalDeliveryOfficeNames) == 0) {
				$physicalDeliveryOfficeNames[] = '';
			}
			$physicalDeliveryOfficeNameLabel = new htmlOutputText(_('Office name'));
			$physicalDeliveryOfficeNameLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($physicalDeliveryOfficeNameLabel);
			$physicalDeliveryOfficeNameContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($physicalDeliveryOfficeNames); $i++) {
				$physicalDeliveryOfficeNameContainer->addElement(new htmlInputField('physicalDeliveryOfficeName' . $i, $physicalDeliveryOfficeNames[$i]));
				if ($i < (sizeof($physicalDeliveryOfficeNames) - 1)) {
					$physicalDeliveryOfficeNameContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$physicalDeliveryOfficeNameContainer->addElement(new htmlButton('addPhysicalDeliveryOfficeName', 'add.png', true));
				}
			}
			$fieldContainer->addElement($physicalDeliveryOfficeNameContainer);
			$physicalDeliveryOfficeNameHelp = new htmlHelpLink('physicalDeliveryOfficeName');
			$physicalDeliveryOfficeNameHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($physicalDeliveryOfficeNameHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$roomNumber = '';
			if (isset($this->attributes['roomNumber'][0])) $roomNumber = $this->attributes['roomNumber'][0];
			$fieldContainer->addElement(new htmlTableExtendedInputField(_('Room number'), 'roomNumber', $roomNumber, 'roomNumber'), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Contact data')), true);
		}
		
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$telephoneNumbers = array();
			if (isset($this->attributes['telephoneNumber'][0])) {
				$telephoneNumbers = $this->attributes['telephoneNumber'];
			}
			if (sizeof($telephoneNumbers) == 0) {
				$telephoneNumbers[] = '';
			}
			$telephoneNumberLabel = new htmlOutputText(_('Telephone number'));
			$telephoneNumberLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($telephoneNumberLabel);
			$telephoneNumberContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($telephoneNumbers); $i++) {
				$telephoneNumberContainer->addElement(new htmlInputField('telephoneNumber' . $i, $telephoneNumbers[$i]));
				if ($i < (sizeof($telephoneNumbers) - 1)) {
					$telephoneNumberContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$telephoneNumberContainer->addElement(new htmlButton('addTelephoneNumber', 'add.png', true));
				}
			}
			$fieldContainer->addElement($telephoneNumberContainer);
			$telephoneNumberHelp = new htmlHelpLink('telephoneNumber');
			$telephoneNumberHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($telephoneNumberHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$homePhones = array();
			if (isset($this->attributes['homePhone'][0])) {
				$homePhones = $this->attributes['homePhone'];
			}
			if (sizeof($homePhones) == 0) {
				$homePhones[] = '';
			}
			$homePhoneLabel = new htmlOutputText(_('Home telephone number'));
			$homePhoneLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($homePhoneLabel);
			$homePhoneContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($homePhones); $i++) {
				$homePhoneContainer->addElement(new htmlInputField('homePhone' . $i, $homePhones[$i]));
				if ($i < (sizeof($homePhones) - 1)) {
					$homePhoneContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$homePhoneContainer->addElement(new htmlButton('addHomePhone', 'add.png', true));
				}
			}
			$fieldContainer->addElement($homePhoneContainer);
			$homePhoneHelp = new htmlHelpLink('homePhone');
			$homePhoneHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($homePhoneHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$mobiles = array();
			if (isset($this->attributes['mobile'][0])) {
				$mobiles = $this->attributes['mobile'];
			}
			if (sizeof($mobiles) == 0) {
				$mobiles[] = '';
			}
			$mobileLabel = new htmlOutputText(_('Mobile number'));
			$mobileLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($mobileLabel);
			$mobileContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($mobiles); $i++) {
				$mobileContainer->addElement(new htmlInputField('mobile' . $i, $mobiles[$i]));
				if ($i < (sizeof($mobiles) - 1)) {
					$mobileContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$mobileContainer->addElement(new htmlButton('addMobile', 'add.png', true));
				}
			}
			$fieldContainer->addElement($mobileContainer);
			$mobileHelp = new htmlHelpLink('mobileTelephoneNumber');
			$mobileHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($mobileHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$facsimileTelephoneNumbers = array();
			if (isset($this->attributes['facsimileTelephoneNumber'][0])) {
				$facsimileTelephoneNumbers = $this->attributes['facsimileTelephoneNumber'];
			}
			if (sizeof($facsimileTelephoneNumbers) == 0) {
				$facsimileTelephoneNumbers[] = '';
			}
			$facsimileTelephoneNumberLabel = new htmlOutputText(_('Fax number'));
			$facsimileTelephoneNumberLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($facsimileTelephoneNumberLabel);
			$facsimileTelephoneNumberContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($facsimileTelephoneNumbers); $i++) {
				$facsimileTelephoneNumberContainer->addElement(new htmlInputField('facsimileTelephoneNumber' . $i, $facsimileTelephoneNumbers[$i]));
				if ($i < (sizeof($facsimileTelephoneNumbers) - 1)) {
					$facsimileTelephoneNumberContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$facsimileTelephoneNumberContainer->addElement(new htmlButton('addFacsimileTelephoneNumber', 'add.png', true));
				}
			}
			$fieldContainer->addElement($facsimileTelephoneNumberContainer);
			$facsimileTelephoneNumberHelp = new htmlHelpLink('facsimileTelephoneNumber');
			$facsimileTelephoneNumberHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($facsimileTelephoneNumberHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$mails = array();
			if (isset($this->attributes['mail'][0])) {
				$mails = $this->attributes['mail'];
			}
			if (sizeof($mails) == 0) {
				$mails[] = '';
			}
			$mailLabel = new htmlOutputText(_('Email address'));
			$mailLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($mailLabel);
			$mailContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($mails); $i++) {
				$mailContainer->addElement(new htmlInputField('mail' . $i, $mails[$i]));
				if ($i < (sizeof($mails) - 1)) {
					$mailContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$mailContainer->addElement(new htmlButton('addMail', 'add.png', true));
				}
			}
			$fieldContainer->addElement($mailContainer);
			$mailHelp = new htmlHelpLink('mail');
			$mailHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($mailHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$labeledURI = array();
			if (isset($this->attributes['labeledURI'][0])) {
				$labeledURI = $this->attributes['labeledURI'];
			}
			if (sizeof($labeledURI) == 0) {
				$labeledURI[] = '';
			}
			$labeledURILabel = new htmlOutputText(_('Web site'));
			$labeledURILabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($labeledURILabel);
			$labeledURIContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($labeledURI); $i++) {
				$labeledURIContainer->addElement(new htmlInputField('labeledURI' . $i, $labeledURI[$i]));
				if ($i < (sizeof($labeledURI) - 1)) {
					$labeledURIContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$labeledURIContainer->addElement(new htmlButton('addLabeledURI', 'add.png', true));
				}
			}
			$fieldContainer->addElement($labeledURIContainer);
			$labeledURIHelp = new htmlHelpLink('labeledURI');
			$labeledURIHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($labeledURIHelp, true);
		}
		
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideuserCertificate')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Work details')), true);
		}
		
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$titles = array();
			if (isset($this->attributes['title'][0])) {
				$titles = $this->attributes['title'];
			}
			if (sizeof($titles) == 0) {
				$titles[] = '';
			}
			$titleLabel = new htmlOutputText(_('Job title'));
			$titleLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($titleLabel);
			$titleContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($titles); $i++) {
				$titleContainer->addElement(new htmlInputField('title' . $i, $titles[$i]));
				if ($i < (sizeof($titles) - 1)) {
					$titleContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$titleContainer->addElement(new htmlButton('addtitle', 'add.png', true));
				}
			}
			$fieldContainer->addElement($titleContainer);
			$titleHelp = new htmlHelpLink('title');
			$titleHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($titleHelp, true);
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
			$businessCategories = array();
			if (isset($this->attributes['businessCategory'][0])) {
				$businessCategories = $this->attributes['businessCategory'];
			}
			if (sizeof($businessCategories) == 0) {
				$businessCategories[] = '';
			}
			$businessCategoryLabel = new htmlOutputText(_('Business category'));
			$businessCategoryLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($businessCategoryLabel);
			$businessCategoryContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($businessCategories); $i++) {
				$businessCategoryContainer->addElement(new htmlInputField('businessCategory' . $i, $businessCategories[$i]));
				if ($i < (sizeof($businessCategories) - 1)) {
					$businessCategoryContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$businessCategoryContainer->addElement(new htmlButton('addbusinessCategory', 'add.png', true));
				}
			}
			$fieldContainer->addElement($businessCategoryContainer);
			$businessCategoryHelp = new htmlHelpLink('businessCategory');
			$businessCategoryHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($businessCategoryHelp, true);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$departmentNumbers = array();
			if (isset($this->attributes['departmentNumber'][0])) {
				$departmentNumbers = $this->attributes['departmentNumber'];
			}
			if (sizeof($departmentNumbers) == 0) {
				$departmentNumbers[] = '';
			}
			$departmentNumberLabel = new htmlOutputText(_('Department'));
			$departmentNumberLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($departmentNumberLabel);
			$departmentNumberContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($departmentNumbers); $i++) {
				$departmentNumberContainer->addElement(new htmlInputField('departmentNumber' . $i, $departmentNumbers[$i]));
				if ($i < (sizeof($departmentNumbers) - 1)) {
					$departmentNumberContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$departmentNumberContainer->addElement(new htmlButton('addDepartmentNumber', 'add.png', true));
				}
			}
			$fieldContainer->addElement($departmentNumberContainer);
			$departmentNumberHelp = new htmlHelpLink('departmentNumber');
			$departmentNumberHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($departmentNumberHelp, true);
		}
		// organisational unit
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$ous = array();
			if (isset($this->attributes['ou'][0])) {
				$ous = $this->attributes['ou'];
			}
			if (sizeof($ous) == 0) {
				$ous[] = '';
			}
			$ouLabel = new htmlOutputText(_('Organisational unit'));
			$ouLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($ouLabel);
			$ouContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($ous); $i++) {
				$ouContainer->addElement(new htmlInputField('ou' . $i, $ous[$i]));
				if ($i < (sizeof($ous) - 1)) {
					$ouContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$ouContainer->addElement(new htmlButton('addOu', 'add.png', true));
				}
			}
			$fieldContainer->addElement($ouContainer);
			$ouHelp = new htmlHelpLink('ou');
			$ouHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($ouHelp, true);
		}
		// organisation
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$os = array();
			if (isset($this->attributes['o'][0])) {
				$os = $this->attributes['o'];
			}
			if (sizeof($os) == 0) {
				$os[] = '';
			}
			$oLabel = new htmlOutputText(_('Organisation'));
			$oLabel->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($oLabel);
			$oContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($os); $i++) {
				$oContainer->addElement(new htmlInputField('o' . $i, $os[$i]));
				if ($i < (sizeof($os) - 1)) {
					$oContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$oContainer->addElement(new htmlButton('addO', 'add.png', true));
				}
			}
			$fieldContainer->addElement($oContainer);
			$oHelp = new htmlHelpLink('o');
			$oHelp->alignment = htmlElement::ALIGN_TOP;
			$fieldContainer->addElement($oHelp, true);
		}
		// user certificates
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideuserCertificate')) {
			$fieldContainer->addElement(new htmlOutputText(_('User certificates')));
			$userCertificateGroup = new htmlGroup();
			$userCertificateCount = 0;
			if (isset($this->attributes['userCertificate;binary'])) {
				$userCertificateCount = sizeof($this->attributes['userCertificate;binary']);
			}
			$userCertificateGroup->addElement(new htmlOutputText($userCertificateCount));
			$userCertificateGroup->addElement(new htmlSpacer('10px', null));
			$userCertificateGroup->addElement(new htmlAccountPageButton(get_class($this), 'userCertificate', 'manage', _('Manage')));
			$fieldContainer->addElement($userCertificateGroup);
			$fieldContainer->addElement(new htmlHelpLink('userCertificate'), true);
		}
		// manager
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$fieldContainer->addElement(new htmlOutputText(_('Manager')));
			$fieldContainer->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'change', _("Change")));
			$fieldContainer->addElement(new htmlHelpLink('manager'), true);
			if (isset($this->attributes['manager'][0])) {
				$managerList = array();
				for ($i = 0; $i < sizeof($this->attributes['manager']); $i++) {
					$managerList[] = $this->attributes['manager'][$i];
				}
				usort($managerList, 'compareDN');
				$managers = new htmlTable();
				$managers->colspan = 2;
				for ($i = 0; $i < sizeof($managerList); $i++) {
					$managers->addElement(new htmlOutputText(getAbstractDN($managerList[$i])), true);
				}
				$fieldContainer->addElement(new htmlOutputText(''));
				$fieldContainer->addElement($managers, true);
			}
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
		if (!isset($_POST['form_subpage_' . get_class($this) . '_attributes_submit'])) {
			return array();
		}
		$messages = array();
		if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 1000000);
			fclose($handle);
			$this->attributes['jpegPhoto'][0] = $data;
		}
		else {
			$messages[] = $this->messages['file'][0];
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
	* This function will create the meta HTML code to show a page to change the manager attribute.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_manager() {
		$return = new htmlTable();
		if (!isset($this->attributes['manager'])) {
			$this->attributes['manager'] = array();
		}
		// show list of possible new managers
		if (isset($_POST['form_subpage_' . get_class($this) . '_manager_select'])) {
			$options = array();
			$filter = get_ldap_filter('user');
			$entries = searchLDAPByFilter('(|' . $filter . '(objectclass=organizationalRole))', array('dn'), array('user'));
			for ($i = 0; $i < sizeof($entries); $i++) {
				$entries[$i] = $entries[$i]['dn'];
			}
			// sort by DN
			usort($entries, 'compareDN');
			for ($i = 0; $i < sizeof($entries); $i++) {
				if (!isset($this->attributes['manager']) || !in_array($entries[$i], $this->attributes['manager'])) {
					$options[getAbstractDN($entries[$i])] = $entries[$i];
				}
			}
			$size = 20;
			if (sizeof($options) < 20) $size = sizeof($options);
			$managerSelect = new htmlSelect('manager', $options, array(), $size);
			$managerSelect->setHasDescriptiveElements(true);
			$managerSelect->setMultiSelect(true);
			$managerSelect->setRightToLeftTextDirection(true);
			$managerSelect->setSortElements(false);
			$managerSelect->setTransformSingleSelect(false);
			$return->addElement($managerSelect, true);
			$buttonTable = new htmlTable();
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'addManagers', _('Add')));
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'cancel', _('Cancel')));
			$return->addElement($buttonTable);
			return $return;
		}
		// show existing managers
		$managerTemp = array();
		if (isset($this->attributes['manager'])) {
			$managerTemp = $this->attributes['manager'];
		}
		// sort by DN
		usort($managerTemp, 'compareDN');
		$managers = array();
		for ($i = 0; $i < sizeof($managerTemp); $i++) {
			$managers[getAbstractDN($managerTemp[$i])] = $managerTemp[$i];
		}
		$size = 20;
		if (sizeof($this->attributes['manager']) < 20) {
			$size = sizeof($this->attributes['manager']);
		}
		if (sizeof($managers) > 0) {
			$managerSelect = new htmlSelect('manager', $managers, array(), $size);
			$managerSelect->setHasDescriptiveElements(true);
			$managerSelect->setMultiSelect(true);
			$managerSelect->setRightToLeftTextDirection(true);
			$managerSelect->setSortElements(false);
			$managerSelect->setTransformSingleSelect(false);
			$return->addElement($managerSelect, true);
			$removeButton = new htmlAccountPageButton(get_class($this), 'manager', 'remove', _('Remove selected entries'));
			$removeButton->colspan = 3;
			$return->addElement($removeButton, true);
			$return->addElement(new htmlOutputText('&nbsp;', false), true);
		}
		$return->addElement(new htmlOutputText('&nbsp;', false), true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'select', _('Add entries')));
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'managerBack', _('Back')));
		$return->addElement($buttonTable);
		return $return;
	}

	/**
	* Processes user input of the manager page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_manager() {
		$return = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_manager_remove']) && isset($_POST['manager'])) {
			$managers = array_flip($this->attributes['manager']);
			for ($i = 0; $i < sizeof($_POST['manager']); $i++) {
				if (isset($managers[$_POST['manager'][$i]])) {
					unset($managers[$_POST['manager'][$i]]);
				}
			}
			$this->attributes['manager'] = array_values(array_flip($managers));
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_manager_addManagers']) && isset($_POST['manager'])) {
			for ($i = 0; $i < sizeof($_POST['manager']); $i++) {
				$this->attributes['manager'][] = $_POST['manager'][$i];
				$this->attributes['manager'] = array_unique($this->attributes['manager']);
			}
		}
		return $return;
	}

	/**
	* Displays the certificate upload page.
	*
	* @return array meta HTML code
	*/
	function display_html_userCertificate() {
		$container = new htmlTable();
		if (isset($this->attributes['userCertificate;binary'])) {
			$table = new htmlTable();
			$table->colspan = 10;
			for ($i = 0; $i < sizeof($this->attributes['userCertificate;binary']); $i++) {
				$filename = 'userCertificate' . $_SESSION['ldap']->new_rand() . '.der';
				$out = @fopen(dirname(__FILE__) . '/../../tmp/' . $filename, "wb");
				fwrite($out, $this->attributes['userCertificate;binary'][$i]);
				fclose ($out);
				$path = '../../tmp/' . $filename;
				$link = new htmlLink('', $path, '../../graphics/save.png');
				$link->setTargetWindow('_blank');
				$table->addElement($link);
				$deleteButton = new htmlAccountPageButton(get_class($this), 'userCertificate', 'delete_' . $i, 'delete.png', true);
				$deleteButton->setIconClass('deleteButton');
				$table->addElement($deleteButton);
				if (function_exists('openssl_x509_parse')) {
					$pem = @chunk_split(@base64_encode($this->attributes['userCertificate;binary'][$i]), 64, "\n");
					if (!empty($pem)) {
						$pem = "-----BEGIN CERTIFICATE-----\n" . $pem . "-----END CERTIFICATE-----\n";
						$pemData = @openssl_x509_parse($pem);
						$data = array();
						if (isset($pemData['serialNumber'])) {
							$data[] = $pemData['serialNumber'];
						}
						if (isset($pemData['name'])) {
							$data[] = $pemData['name'];
						}
						if (sizeof($data) > 0) {
							$table->addElement(new htmlOutputText(implode(': ', $data)));
						}
					}
				}
				$table->addNewLine();
			}
			$container->addElement($table, true);
			$container->addElement(new htmlSpacer(null, '20px'), true);
		}
		$newGroup = new htmlGroup();
		$newGroup->addElement(new htmlOutputText(_('New user certificate')));
		$newGroup->addElement(new htmlSpacer('1px', null));
		$newGroup->addElement(new htmlInputFileUpload('userCertificateUpload'));
		$newGroup->addElement(new htmlSpacer('1px', null));
		$uploadButton = new htmlAccountPageButton(get_class($this), 'userCertificate', 'submit', _('Upload'));
		$uploadButton->setIconClass('upButton');
		$newGroup->addElement($uploadButton);
		$container->addElement($newGroup, true);
		$container->addElement(new htmlSpacer(null, '10px'), true);
		$container->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')));
		return $container;
	}
	
	/**
	* Sets a new certificate or deletes old ones.
	*/
	function process_userCertificate() {
		$messages = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_userCertificate_submit'])) {
			if ($_FILES['userCertificateUpload'] && ($_FILES['userCertificateUpload']['size'] > 0)) {
				$handle = fopen($_FILES['userCertificateUpload']['tmp_name'], "r");
				$data = fread($handle, 10000000);
				fclose($handle);
				if (strpos($data, '-----BEGIN CERTIFICATE-----') === 0) {
					$pemData = str_replace("\r", '', $data);
					$pemData = explode("\n", $pemData);
					array_shift($pemData);
					$last = array_pop($pemData);
					while (($last != '-----END CERTIFICATE-----') && sizeof($pemData) > 2) {
						$last = array_pop($pemData);
					}
					$pemData = implode('', $pemData);
					$data = base64_decode($pemData);
				}
				$this->attributes['userCertificate;binary'][] = $data;
			}
			else {
				$messages[] = $this->messages['file'][0];
			}
		}
		elseif (isset($this->attributes['userCertificate;binary'])) {
			for ($i = 0; $i < sizeof($this->attributes['userCertificate;binary']); $i++) {
				if (isset($_POST['form_subpage_' . get_class($this) . '_userCertificate_delete_' . $i])) {
					unset($this->attributes['userCertificate;binary'][$i]);
					$this->attributes['userCertificate;binary'] = array_values($this->attributes['userCertificate;binary']);
					break;
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$description = '';
		if (isset($this->attributes['description'][0])) $description = implode(', ', $this->attributes['description']);
		$title = '';
		if (isset($this->attributes['title'][0])) {
			$title = implode(', ', $this->attributes['title']);
		}
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
		$registeredAddress = '';
		if (isset($this->attributes['registeredAddress'][0])) {
			$registeredAddress = implode(', ', $this->attributes['registeredAddress']);
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
			$businessCategory = implode(', ', $this->attributes['businessCategory']);
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
			$physicalDeliveryOfficeName = implode(', ', $this->attributes['physicalDeliveryOfficeName']);
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
		$ou = '';
		if (isset($this->attributes['ou'][0])) {
			$ou = implode(', ', $this->attributes['ou']);
		}
		$o = '';
		if (isset($this->attributes['o'][0])) {
			$o = implode(', ', $this->attributes['o']);
		}
		$initials = '';
		if (isset($this->attributes['initials'][0])) {
			$initials = implode(', ', $this->attributes['initials']);
		}
		$labeledURI = '';
		if (isset($this->attributes['labeledURI'][0])) {
			$labeledURI = implode(', ', $this->attributes['labeledURI']);
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
			get_class($this) . '_registeredAddress' => array('<block><key>' . _('Registered address') . '</key><value>' . $registeredAddress . '</value></block>'),
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
			get_class($this) . '_ou' => array('<block><key>' . _('Organisational unit') . '</key><value>' . $ou . '</value></block>'),
			get_class($this) . '_o' => array('<block><key>' . _('Organisation') . '</key><value>' . $o . '</value></block>'),
			get_class($this) . '_employeeNumber' => array('<block><key>' . _('Employee number') . '</key><value>' . $employeeNumber . '</value></block>'),
			get_class($this) . '_initials' => array('<block><key>' . _('Initials') . '</key><value>' . $initials . '</value></block>'),
			get_class($this) . '_labeledURI' => array('<block><key>' . _('Web site') . '</key><value>' . $labeledURI . '</value></block>'),
		);
		if (isset($this->attributes['departmentNumber'])) {
			$return[get_class($this) . '_departmentNumber'] = array('<block><key>' . _('Department') . '</key><value>' . implode(', ', $this->attributes['departmentNumber']) . '</value></block>');
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu') && isset($profile['inetOrgPerson_ou'][0])) {
			$oList = preg_split('/;[ ]*/', $profile['inetOrgPerson_ou'][0]);
			$this->attributes['ou'] = $oList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO') && isset($profile['inetOrgPerson_o'][0])) {
			$oList = preg_split('/;[ ]*/', $profile['inetOrgPerson_o'][0]);
			$this->attributes['o'] = $oList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') && isset($profile['inetOrgPerson_mail'][0])) {
			$mailList = preg_split('/;[ ]*/', $profile['inetOrgPerson_mail'][0]);
			$this->attributes['mail'] = $mailList;
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI') && isset($profile['inetOrgPerson_labeledURI'][0])) {
			$labeledURIList = preg_split('/;[ ]*/', $profile['inetOrgPerson_labeledURI'][0]);
			$this->attributes['labeledURI'] = $labeledURIList;
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
			if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
				$return[] = array(
					'name' => 'inetOrgPerson_userName',
					'description' => _('User name'),
					'help' => 'uid',
					'example' => _('smiller'),
					'unique' => true,
					'required' => false
				);
			}
			$return[] = array(
				'name' => 'inetOrgPerson_userPassword',
				'description' => _('Password'),
				'help' => 'userPassword',
				'example' => _('secret'),
			);
		}
		return $return;
	}
	
	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
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
			if (isset($ids['inetOrgPerson_initials']) && ($rawAccounts[$i][$ids['inetOrgPerson_initials']] != "")) {
				$partialAccounts[$i]['initials'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_initials']]);
			}
			// description
			if (isset($ids['inetOrgPerson_description']) && ($rawAccounts[$i][$ids['inetOrgPerson_description']] != "")) {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['inetOrgPerson_description']];
			}
			// title
			if (isset($ids['inetOrgPerson_title']) && ($rawAccounts[$i][$ids['inetOrgPerson_title']] != "")) {
				$titleList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_title']]);
				$partialAccounts[$i]['title'] = $titleList;
				for ($x = 0; $x < sizeof($titleList); $x++) {
					if (!get_preg($titleList[$x], 'title')) {
						$errMsg = $this->messages['title'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// employee number
			if (isset($ids['inetOrgPerson_employeeNumber']) && ($rawAccounts[$i][$ids['inetOrgPerson_employeeNumber']] != "")) {
				$partialAccounts[$i]['employeeNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_employeeNumber']];
			}
			// employee type
			if (isset($ids['inetOrgPerson_type']) && ($rawAccounts[$i][$ids['inetOrgPerson_type']] != "")) {
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
			if (isset($ids['inetOrgPerson_businessCategory']) && ($rawAccounts[$i][$ids['inetOrgPerson_businessCategory']] != "")) {
				$businessCategoryList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_businessCategory']]);
				$partialAccounts[$i]['businessCategory'] = $businessCategoryList;
				for ($x = 0; $x < sizeof($businessCategoryList); $x++) {
					if (!get_preg($businessCategoryList[$x], 'businessCategory')) {
						$errMsg = $this->messages['businessCategory'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// manager
			if (isset($ids['inetOrgPerson_manager']) && ($rawAccounts[$i][$ids['inetOrgPerson_manager']] != "")) {
				$managerList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_manager']]);
				$partialAccounts[$i]['manager'] = $managerList;
				for ($x = 0; $x < sizeof($managerList); $x++) {
					if (!get_preg($managerList[$x], 'dn')) {
						$errMsg = $this->messages['manager'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// street
			if (isset($ids['inetOrgPerson_street']) && ($rawAccounts[$i][$ids['inetOrgPerson_street']] != "")) {
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
			if (isset($ids['inetOrgPerson_postOfficeBox']) && ($rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']] != "")) {
				$partialAccounts[$i]['postOfficeBox'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']]);
			}
			// room number
			if (isset($ids['inetOrgPerson_roomNumber']) && ($rawAccounts[$i][$ids['inetOrgPerson_roomNumber']] != "")) {
				$partialAccounts[$i]['roomNumber'] = $rawAccounts[$i][$ids['inetOrgPerson_roomNumber']];
			}
			// departments
			if (isset($ids['inetOrgPerson_departmentNumber']) && ($rawAccounts[$i][$ids['inetOrgPerson_departmentNumber']] != "")) {
				$partialAccounts[$i]['departmentNumber'] = explode(';', $rawAccounts[$i][$ids['inetOrgPerson_departmentNumber']]);
				// remove extra spaces
				$partialAccounts[$i]['departmentNumber'] = array_map('trim', $partialAccounts[$i]['departmentNumber']);
			}
			// organisational unit
			if (isset($ids['inetOrgPerson_ou']) && ($rawAccounts[$i][$ids['inetOrgPerson_ou']] != "")) {
				$partialAccounts[$i]['ou'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_ou']]);
			}
			// organisation
			if (isset($ids['inetOrgPerson_o']) && ($rawAccounts[$i][$ids['inetOrgPerson_o']] != "")) {
				$partialAccounts[$i]['o'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_o']]);
			}
			// location
			if (isset($ids['inetOrgPerson_l']) && ($rawAccounts[$i][$ids['inetOrgPerson_l']] != "")) {
				$partialAccounts[$i]['l'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_l']]);
			}
			// state
			if (isset($ids['inetOrgPerson_st']) && ($rawAccounts[$i][$ids['inetOrgPerson_st']] != "")) {
				$partialAccounts[$i]['st'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_st']]);
			}
			// physicalDeliveryOfficeName
			if (isset($ids['inetOrgPerson_physicalDeliveryOfficeName']) && ($rawAccounts[$i][$ids['inetOrgPerson_physicalDeliveryOfficeName']] != "")) {
				$partialAccounts[$i]['physicalDeliveryOfficeName'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_physicalDeliveryOfficeName']]);
			}
			// carLicense
			if (isset($ids['inetOrgPerson_carLicense']) && ($rawAccounts[$i][$ids['inetOrgPerson_carLicense']] != "")) {
				$partialAccounts[$i]['carLicense'] = $rawAccounts[$i][$ids['inetOrgPerson_carLicense']];
			}
			// postal code
			if (isset($ids['inetOrgPerson_postalCode']) && ($rawAccounts[$i][$ids['inetOrgPerson_postalCode']] != "")) {
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
			if (isset($ids['inetOrgPerson_address']) && ($rawAccounts[$i][$ids['inetOrgPerson_address']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_address']], 'postalAddress')) {
					$partialAccounts[$i]['postalAddress'] = $rawAccounts[$i][$ids['inetOrgPerson_address']];
				}
				else {
					$errMsg = $this->messages['postalAddress'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// registered address
			if (isset($ids['inetOrgPerson_registeredAddress']) && ($rawAccounts[$i][$ids['inetOrgPerson_registeredAddress']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_registeredAddress']], 'postalAddress')) {
					$partialAccounts[$i]['registeredAddress'] = $rawAccounts[$i][$ids['inetOrgPerson_registeredAddress']];
				}
				else {
					$errMsg = $this->messages['registeredAddress'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// telephone
			if (isset($ids['inetOrgPerson_telephone']) && ($rawAccounts[$i][$ids['inetOrgPerson_telephone']] != "")) {
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
			if (isset($ids['inetOrgPerson_homePhone']) && ($rawAccounts[$i][$ids['inetOrgPerson_homePhone']] != "")) {
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
			if (isset($ids['inetOrgPerson_mobile']) && ($rawAccounts[$i][$ids['inetOrgPerson_mobile']] != "")) {
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
			if (isset($ids['inetOrgPerson_fax']) && ($rawAccounts[$i][$ids['inetOrgPerson_fax']] != "")) {
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
			if (isset($ids['inetOrgPerson_email']) && ($rawAccounts[$i][$ids['inetOrgPerson_email']] != "")) {
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
			// labeledURI
			if (isset($ids['inetOrgPerson_labeledURI']) && ($rawAccounts[$i][$ids['inetOrgPerson_labeledURI']] != "")) {
				$partialAccounts[$i]['labeledURI'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_labeledURI']]);
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
				if (isset($ids['inetOrgPerson_userName']) && !empty($rawAccounts[$i][$ids['inetOrgPerson_userName']])) {
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
				// password
				if (($rawAccounts[$i][$ids['inetOrgPerson_userPassword']] != "") && (get_preg($rawAccounts[$i][$ids['inetOrgPerson_userPassword']], 'password'))) {
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['inetOrgPerson_userPassword']], true, $this->moduleSettings['posixAccount_pwdHash'][0]);
					$partialAccounts[$i]['INFO.userPasswordClearText'] = $rawAccounts[$i][$ids['inetOrgPerson_userPassword']]; // for custom scripts etc.
				}
				elseif ($rawAccounts[$i][$ids['inetOrgPerson_userPassword']] != "") {
					$errMsg = $this->messages['userPassword'][0];
					$errMsg[2] = str_replace('%', '%%', $errMsg[2]); // double "%" because of later sprintf
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
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array list of meta HTML elements (field name => htmlTableRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array();
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('firstName', $fields)) {
			$firstName = '';
			if (isset($attributes['givenName'][0])) $firstName = $attributes['givenName'][0];
			$firstNameField = new htmlInputField('inetOrgPerson_firstName', $firstName);
			if (in_array('firstName', $readOnlyFields)) {
				$firstNameField = new htmlOutputText($firstName);
			}
			$return['firstName'] = new htmlTableRow(array(
				new htmlOutputText(_('First name')), $firstNameField
			));
		}
		if (in_array('lastName', $fields)) {
			$lastName = '';
			if (isset($attributes['sn'][0])) $lastName = $attributes['sn'][0];
			$lastNameField = new htmlInputField('inetOrgPerson_lastName', $lastName);
			if (in_array('lastName', $readOnlyFields)) {
				$lastNameField = new htmlOutputText($lastName);
			}
			$return['lastName'] = new htmlTableRow(array(
				new htmlOutputText(_('Last name')), $lastNameField
			));
		}
		if (in_array('mail', $fields)) {
			$mail = '';
			if (isset($attributes['mail'][0])) $mail = $attributes['mail'][0];
			$mailField = new htmlInputField('inetOrgPerson_mail', $mail);
			if (in_array('mail', $readOnlyFields)) {
				$mailField = new htmlOutputText($mail);
			}
			$return['mail'] = new htmlTableRow(array(
				new htmlOutputText(_('Email address')), $mailField
			));
		}
		if (in_array('labeledURI', $fields)) {
			$labeledURI = '';
			if (isset($attributes['labeledURI'][0])) $labeledURI = implode('; ', $attributes['labeledURI']);
			$labeledURIField = new htmlInputField('inetOrgPerson_labeledURI', $labeledURI);
			if (in_array('labeledURI', $readOnlyFields)) {
				$labeledURIField = new htmlOutputText($labeledURI);
			}
			$return['labeledURI'] = new htmlTableRow(array(
				new htmlOutputText(_('Web site')), $labeledURIField
			));
		}
		if (in_array('telephoneNumber', $fields)) {
			$telephoneNumber = '';
			if (isset($attributes['telephoneNumber'][0])) $telephoneNumber = $attributes['telephoneNumber'][0];
			$telephoneNumberField = new htmlInputField('inetOrgPerson_telephoneNumber', $telephoneNumber);
			if (in_array('telephoneNumber', $readOnlyFields)) {
				$telephoneNumberField = new htmlOutputText($telephoneNumber);
			}
			$return['telephoneNumber'] = new htmlTableRow(array(
				new htmlOutputText(_('Telephone number')), $telephoneNumberField
			));
		}
		if (in_array('homePhone', $fields)) {
			$homePhone = '';
			if (isset($attributes['homePhone'][0])) $homePhone = $attributes['homePhone'][0];
			$homePhoneField = new htmlInputField('inetOrgPerson_homePhone', $homePhone);
			if (in_array('homePhone', $readOnlyFields)) {
				$homePhoneField = new htmlOutputText($homePhone);
			}
			$return['homePhone'] = new htmlTableRow(array(
				new htmlOutputText(_('Home telephone number')), $homePhoneField
			));
		}
		if (in_array('mobile', $fields)) {
			$mobile = '';
			if (isset($attributes['mobile'][0])) $mobile = $attributes['mobile'][0];
			$mobileField = new htmlInputField('inetOrgPerson_mobile', $mobile);
			if (in_array('mobile', $readOnlyFields)) {
				$mobileField = new htmlOutputText($mobile);
			}
			$return['mobile'] = new htmlTableRow(array(
				new htmlOutputText(_('Mobile telephone number')), $mobileField
			));
		}
		if (in_array('faxNumber', $fields)) {
			$faxNumber = '';
			if (isset($attributes['facsimileTelephoneNumber'][0])) $faxNumber = $attributes['facsimileTelephoneNumber'][0];
			$faxNumberField = new htmlInputField('inetOrgPerson_faxNumber', $faxNumber);
			if (in_array('faxNumber', $readOnlyFields)) {
				$faxNumberField = new htmlOutputText($faxNumber);
			}
			$return['faxNumber'] = new htmlTableRow(array(
				new htmlOutputText(_('Fax number')), $faxNumberField
			));
		}
		if (in_array('street', $fields)) {
			$street = '';
			if (isset($attributes['street'][0])) $street = $attributes['street'][0];
			$streetField = new htmlInputField('inetOrgPerson_street', $street);
			if (in_array('street', $readOnlyFields)) {
				$streetField = new htmlOutputText($street);
			}
			$return['street'] = new htmlTableRow(array(
				new htmlOutputText(_('Street')), $streetField
			));
		}
		if (in_array('postalAddress', $fields)) {
			$postalAddress = '';
			if (isset($attributes['postalAddress'][0])) $postalAddress = $attributes['postalAddress'][0];
			$postalAddressField = new htmlInputField('inetOrgPerson_postalAddress', $postalAddress);
			if (in_array('postalAddress', $readOnlyFields)) {
				$postalAddressField = new htmlOutputText($postalAddress);
			}
			$return['postalAddress'] = new htmlTableRow(array(
				new htmlOutputText(_('Postal address')), $postalAddressField
			));
		}
		if (in_array('registeredAddress', $fields)) {
			$registeredAddress = '';
			if (isset($attributes['registeredAddress'][0])) $registeredAddress = $attributes['registeredAddress'][0];
			$registeredAddressField = new htmlInputField('inetOrgPerson_registeredAddress', $registeredAddress);
			if (in_array('registeredAddress', $readOnlyFields)) {
				$registeredAddressField = new htmlOutputText($registeredAddress);
			}
			$return['registeredAddress'] = new htmlTableRow(array(
				new htmlOutputText(_('Registered address')), $registeredAddressField
			));
		}
		if (in_array('postalCode', $fields)) {
			$postalCode = '';
			if (isset($attributes['postalCode'][0])) $postalCode = $attributes['postalCode'][0];
			$postalCodeField = new htmlInputField('inetOrgPerson_postalCode', $postalCode);
			if (in_array('postalCode', $readOnlyFields)) {
				$postalCodeField = new htmlOutputText($postalCode);
			}
			$return['postalCode'] = new htmlTableRow(array(
				new htmlOutputText(_('Postal code')), $postalCodeField
			));
		}
		if (in_array('postOfficeBox', $fields)) {
			$postOfficeBox = '';
			if (isset($attributes['postOfficeBox'][0])) $postOfficeBox = $attributes['postOfficeBox'][0];
			$postOfficeBoxField = new htmlInputField('inetOrgPerson_postOfficeBox', $postOfficeBox);
			if (in_array('postOfficeBox', $readOnlyFields)) {
				$postOfficeBoxField = new htmlOutputText($postOfficeBox);
			}
			$return['postOfficeBox'] = new htmlTableRow(array(
				new htmlOutputText(_('Post office box')), $postOfficeBoxField
			));
		}
		if (in_array('roomNumber', $fields)) {
			$roomNumber = '';
			if (isset($attributes['roomNumber'][0])) $roomNumber = $attributes['roomNumber'][0];
			$roomNumberField = new htmlInputField('inetOrgPerson_roomNumber', $roomNumber);
			if (in_array('roomNumber', $readOnlyFields)) {
				$roomNumberField = new htmlOutputText($roomNumber);
			}
			$return['roomNumber'] = new htmlTableRow(array(
				new htmlOutputText(_('Room number')), $roomNumberField
			));
		}
		if (in_array('location', $fields)) {
			$l = '';
			if (isset($attributes['l'][0])) $l = $attributes['l'][0];
			$lField = new htmlInputField('inetOrgPerson_location', $l);
			if (in_array('location', $readOnlyFields)) {
				$lField = new htmlOutputText($l);
			}
			$return['location'] = new htmlTableRow(array(
				new htmlOutputText(_('Location')), $lField
			));
		}
		if (in_array('state', $fields)) {
			$st = '';
			if (isset($attributes['st'][0])) $st = $attributes['st'][0];
			$stField = new htmlInputField('inetOrgPerson_state', $st);
			if (in_array('state', $readOnlyFields)) {
				$stField = new htmlOutputText($st);
			}
			$return['state'] = new htmlTableRow(array(
				new htmlOutputText(_('State')), $stField
			));
		}
		if (in_array('carLicense', $fields)) {
			$carLicense = '';
			if (isset($attributes['carLicense'][0])) $carLicense = $attributes['carLicense'][0];
			$carLicenseField = new htmlInputField('inetOrgPerson_carLicense', $carLicense);
			if (in_array('carLicense', $readOnlyFields)) {
				$carLicenseField = new htmlOutputText($carLicense);
			}
			$return['carLicense'] = new htmlTableRow(array(
				new htmlOutputText(_('Car license')), $carLicenseField
			));
		}
		if (in_array('officeName', $fields)) {
			$physicalDeliveryOfficeName = '';
			if (isset($attributes['physicalDeliveryOfficeName'][0])) $physicalDeliveryOfficeName = $attributes['physicalDeliveryOfficeName'][0];
			$physicalDeliveryOfficeNameField = new htmlInputField('inetOrgPerson_officeName', $physicalDeliveryOfficeName);
			if (in_array('officeName', $readOnlyFields)) {
				$physicalDeliveryOfficeNameField = new htmlOutputText($physicalDeliveryOfficeName);
			}
			$return['officeName'] = new htmlTableRow(array(
				new htmlOutputText(_('Office name')), $physicalDeliveryOfficeNameField
			));
		}
		if (in_array('businessCategory', $fields)) {
			$businessCategory = '';
			if (isset($attributes['businessCategory'][0])) $businessCategory = $attributes['businessCategory'][0];
			$businessCategoryField = new htmlInputField('inetOrgPerson_businessCategory', $businessCategory);
			if (in_array('businessCategory', $readOnlyFields)) {
				$businessCategoryField = new htmlOutputText($businessCategory);
			}
			$return['businessCategory'] = new htmlTableRow(array(
				new htmlOutputText(_('Business category')), $businessCategoryField
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
				if (!in_array('jpegPhoto', $readOnlyFields)) {
					$photoSubSub = new htmlTable();
					$photoSubSub->addElement(new htmlTableExtendedInputCheckbox('removeReplacePhoto', false, _('Remove/replace photo'), null, false));
					$photoSubSub->addElement(new htmlInputFileUpload('replacePhotoFile'));
					$photoSub->addElement($photoSubSub);
				}
				$photoRowCells = array(new htmlOutputText(_('Photo')), $photoSub);
				$photoRow = new htmlTableRow($photoRowCells);
				$return['jpegPhoto'] = $photoRow;
			}
			elseif (!in_array('jpegPhoto', $readOnlyFields)) {
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
			$departmentNumberField = new htmlInputField('inetOrgPerson_departmentNumber', $departmentNumber);
			if (in_array('departmentNumber', $readOnlyFields)) {
				$departmentNumberField = new htmlOutputText($departmentNumber);
			}
			$return['departmentNumber'] = new htmlTableRow(array(
				new htmlOutputText(_('Department')), $departmentNumberField
			));
		}
		if (in_array('initials', $fields)) {
			$initials = '';
			if (isset($attributes['initials'][0])) $initials = implode('; ', $attributes['initials']);
			$initialsField = new htmlInputField('inetOrgPerson_initials', $initials);
			if (in_array('initials', $readOnlyFields)) {
				$initialsField = new htmlOutputText($initials);
			}
			$return['initials'] = new htmlTableRow(array(
				new htmlOutputText(_('Initials')), $initialsField
			));
		}
		if (in_array('title', $fields)) {
			$title = '';
			if (isset($attributes['title'][0])) $title = $attributes['title'][0];
			$titleField = new htmlInputField('inetOrgPerson_title', $title);
			if (in_array('title', $readOnlyFields)) {
				$titleField = new htmlOutputText($title);
			}
			$return['title'] = new htmlTableRow(array(
				new htmlOutputText(_('Job title')), $titleField
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
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		$attributeNames = array(); // list of attributes which should be checked for modification
		$attributesNew = $attributes;
		// first name
		if (in_array('firstName', $fields) && !in_array('firstName', $readOnlyFields)) {
			$attributeNames[] = 'givenName';
			if (isset($_POST['inetOrgPerson_firstName']) && ($_POST['inetOrgPerson_firstName'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_firstName'], 'realname')) $return['messages'][] = $this->messages['givenName'][0];
				else $attributesNew['givenName'][0] = $_POST['inetOrgPerson_firstName'];
			}
			elseif (isset($attributes['givenName'])) unset($attributesNew['givenName']);
		}
		// last name
		if (in_array('lastName', $fields) && !in_array('lastName', $readOnlyFields)) {
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
		if (in_array('mail', $fields) && !in_array('mail', $readOnlyFields)) {
			$attributeNames[] = 'mail';
			if (isset($_POST['inetOrgPerson_mail']) && ($_POST['inetOrgPerson_mail'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_mail'], 'email')) $return['messages'][] = $this->messages['email'][0];
				else $attributesNew['mail'][0] = $_POST['inetOrgPerson_mail'];
			}
			elseif (isset($attributes['mail'])) unset($attributesNew['mail']);
		}
		// labeledURI
		if (in_array('labeledURI', $fields) && !in_array('labeledURI', $readOnlyFields)) {
			$attributeNames[] = 'labeledURI';
			if (isset($_POST['inetOrgPerson_labeledURI']) && ($_POST['inetOrgPerson_labeledURI'] != '')) {
				$attributesNew['labeledURI'] = preg_split('/;[ ]*/', $_POST['inetOrgPerson_labeledURI']);
			}
			elseif (isset($attributes['labeledURI'])) unset($attributesNew['labeledURI']);
		}
		// telephone number
		if (in_array('telephoneNumber', $fields) && !in_array('telephoneNumber', $readOnlyFields)) {
			$attributeNames[] = 'telephoneNumber';
			if (isset($_POST['inetOrgPerson_telephoneNumber']) && ($_POST['inetOrgPerson_telephoneNumber'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_telephoneNumber'], 'telephone')) $return['messages'][] = $this->messages['telephoneNumber'][0];
				else $attributesNew['telephoneNumber'][0] = $_POST['inetOrgPerson_telephoneNumber'];
			}
			elseif (isset($attributes['telephoneNumber'])) unset($attributesNew['telephoneNumber']);
		}
		// home telephone number
		if (in_array('homePhone', $fields) && !in_array('homePhone', $readOnlyFields)) {
			$attributeNames[] = 'homePhone';
			if (isset($_POST['inetOrgPerson_homePhone']) && ($_POST['inetOrgPerson_homePhone'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_homePhone'], 'telephone')) $return['messages'][] = $this->messages['homePhone'][0];
				else $attributesNew['homePhone'][0] = $_POST['inetOrgPerson_homePhone'];
			}
			elseif (isset($attributes['homePhone'])) unset($attributesNew['homePhone']);
		}
		// fax number
		if (in_array('faxNumber', $fields) && !in_array('faxNumber', $readOnlyFields)) {
			$attributeNames[] = 'facsimileTelephoneNumber';
			if (isset($_POST['inetOrgPerson_faxNumber']) && ($_POST['inetOrgPerson_faxNumber'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_faxNumber'], 'telephone')) $return['messages'][] = $this->messages['facsimileNumber'][0];
				else $attributesNew['facsimileTelephoneNumber'][0] = $_POST['inetOrgPerson_faxNumber'];
			}
			elseif (isset($attributes['facsimileTelephoneNumber'])) $attributesNew['facsimileTelephoneNumber'] = array();
		}
		// mobile telephone number
		if (in_array('mobile', $fields) && !in_array('mobile', $readOnlyFields)) {
			$attributeNames[] = 'mobile';
			if (isset($_POST['inetOrgPerson_mobile']) && ($_POST['inetOrgPerson_mobile'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_mobile'], 'telephone')) $return['messages'][] = $this->messages['mobileTelephone'][0];
				else $attributesNew['mobile'][0] = $_POST['inetOrgPerson_mobile'];
			}
			elseif (isset($attributes['mobile'])) unset($attributesNew['mobile']);
		}
		// street
		if (in_array('street', $fields) && !in_array('street', $readOnlyFields)) {
			$attributeNames[] = 'street';
			if (isset($_POST['inetOrgPerson_street']) && ($_POST['inetOrgPerson_street'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_street'], 'street')) $return['messages'][] = $this->messages['street'][0];
				else $attributesNew['street'][0] = $_POST['inetOrgPerson_street'];
			}
			elseif (isset($attributes['street'])) unset($attributesNew['street']);
		}
		// postal address
		if (in_array('postalAddress', $fields) && !in_array('postalAddress', $readOnlyFields)) {
			$attributeNames[] = 'postalAddress';
			if (isset($_POST['inetOrgPerson_postalAddress']) && ($_POST['inetOrgPerson_postalAddress'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_postalAddress'], 'postalAddress')) $return['messages'][] = $this->messages['postalAddress'][0];
				else $attributesNew['postalAddress'][0] = $_POST['inetOrgPerson_postalAddress'];
			}
			elseif (isset($attributes['postalAddress'])) $attributesNew['postalAddress'] = array();
		}
		// registered address
		if (in_array('registeredAddress', $fields) && !in_array('registeredAddress', $readOnlyFields)) {
			$attributeNames[] = 'registeredAddress';
			if (isset($_POST['inetOrgPerson_registeredAddress']) && ($_POST['inetOrgPerson_registeredAddress'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_registeredAddress'], 'postalAddress')) $return['messages'][] = $this->messages['registeredAddress'][0];
				else $attributesNew['registeredAddress'][0] = $_POST['inetOrgPerson_registeredAddress'];
			}
			elseif (isset($attributes['registeredAddress'])) $attributesNew['registeredAddress'] = array();
		}
		// postal code
		if (in_array('postalCode', $fields) && !in_array('postalCode', $readOnlyFields)) {
			$attributeNames[] = 'postalCode';
			if (isset($_POST['inetOrgPerson_postalCode']) && ($_POST['inetOrgPerson_postalCode'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_postalCode'], 'postalCode')) $return['messages'][] = $this->messages['postalCode'][0];
				else $attributesNew['postalCode'][0] = $_POST['inetOrgPerson_postalCode'];
			}
			elseif (isset($attributes['postalCode'])) unset($attributesNew['postalCode']);
		}
		// post office box
		if (in_array('postOfficeBox', $fields) && !in_array('postOfficeBox', $readOnlyFields)) {
			$attributeNames[] = 'postOfficeBox';
			if (isset($_POST['inetOrgPerson_postOfficeBox']) && ($_POST['inetOrgPerson_postOfficeBox'] != '')) {
				$attributesNew['postOfficeBox'][0] = $_POST['inetOrgPerson_postOfficeBox'];
			}
			elseif (isset($attributes['postOfficeBox'])) unset($attributesNew['postOfficeBox']);
		}
		// room number
		if (in_array('roomNumber', $fields) && !in_array('roomNumber', $readOnlyFields)) {
			$attributeNames[] = 'roomNumber';
			if (isset($_POST['inetOrgPerson_roomNumber']) && ($_POST['inetOrgPerson_roomNumber'] != '')) {
				$attributesNew['roomNumber'][0] = $_POST['inetOrgPerson_roomNumber'];
			}
			elseif (isset($attributes['roomNumber'])) unset($attributesNew['roomNumber']);
		}
		// l
		if (in_array('location', $fields) && !in_array('location', $readOnlyFields)) {
			$attributeNames[] = 'l';
			if (isset($_POST['inetOrgPerson_location']) && ($_POST['inetOrgPerson_location'] != '')) {
				$attributesNew['l'][0] = $_POST['inetOrgPerson_location'];
			}
			elseif (isset($attributes['l'])) unset($attributesNew['l']);
		}
		// st
		if (in_array('state', $fields) && !in_array('state', $readOnlyFields)) {
			$attributeNames[] = 'st';
			if (isset($_POST['inetOrgPerson_state']) && ($_POST['inetOrgPerson_state'] != '')) {
				$attributesNew['st'][0] = $_POST['inetOrgPerson_state'];
			}
			elseif (isset($attributes['st'])) unset($attributesNew['st']);
		}
		// car license
		if (in_array('carLicense', $fields) && !in_array('carLicense', $readOnlyFields)) {
			$attributeNames[] = 'carLicense';
			if (isset($_POST['inetOrgPerson_carLicense']) && ($_POST['inetOrgPerson_carLicense'] != '')) {
				$attributesNew['carLicense'][0] = $_POST['inetOrgPerson_carLicense'];
			}
			elseif (isset($attributes['carLicense'])) unset($attributesNew['carLicense']);
		}
		// office name
		if (in_array('officeName', $fields) && !in_array('officeName', $readOnlyFields)) {
			$attributeNames[] = 'physicalDeliveryOfficeName';
			if (isset($_POST['inetOrgPerson_officeName']) && ($_POST['inetOrgPerson_officeName'] != '')) {
				$attributesNew['physicalDeliveryOfficeName'][0] = $_POST['inetOrgPerson_officeName'];
			}
			elseif (isset($attributes['physicalDeliveryOfficeName'])) unset($attributesNew['physicalDeliveryOfficeName']);
		}
		// business category
		if (in_array('businessCategory', $fields) && !in_array('businessCategory', $readOnlyFields)) {
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
		if (in_array('jpegPhoto', $fields) && !in_array('jpegPhoto', $readOnlyFields)) {
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
		if (in_array('departmentNumber', $fields) && !in_array('departmentNumber', $readOnlyFields)) {
			$attributeNames[] = 'departmentNumber';
			if (isset($_POST['inetOrgPerson_departmentNumber']) && ($_POST['inetOrgPerson_departmentNumber'] != '')) {
				$attributesNew['departmentNumber'] = preg_split('/;[ ]*/', $_POST['inetOrgPerson_departmentNumber']);
			}
			elseif (isset($attributes['departmentNumber'])) unset($attributesNew['departmentNumber']);
		}
		// initials
		if (in_array('initials', $fields) && !in_array('initials', $readOnlyFields)) {
			$attributeNames[] = 'initials';
			if (isset($_POST['inetOrgPerson_initials']) && ($_POST['inetOrgPerson_initials'] != '')) {
				$attributesNew['initials'] = preg_split('/;[ ]*/', $_POST['inetOrgPerson_initials']);
			}
			elseif (isset($attributes['initials'])) unset($attributesNew['initials']);
		}
		// title
		if (in_array('title', $fields) && !in_array('title', $readOnlyFields)) {
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
	 * Specifies if this module supports to force that a user must change his password on next login.
	 * 
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange() {
		return false;
	}
	
	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
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
