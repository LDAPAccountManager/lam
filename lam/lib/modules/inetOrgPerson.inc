<?php

use \LAM\TYPES\TypeManager;
use LAM\ImageUtils\ImageManipulationFactory;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2024  Roland Gruber

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
* Manages the attributes of object class inetOrgPerson.
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
class inetOrgPerson extends baseModule implements passwordService,AccountStatusProvider {

	/**
	 * ID for locked password status.
	 */
	const STATUS_PASSWORD_LOCKED = "INETORG_PERSON_PASSWORD_LOCKED";

	/** clear text password */
	private $clearTextPassword;
	/** cache for departments */
	private $departmentCache;
	/** organization cache */
	private $oCache;
	/** organizational unit cache */
	private $ouCache;
	/** title cache */
	private $titleCache;
	/** employee type cache */
	private $employeeTypeCache;
	/** business category cache */
	private $businessCategoryCache;

	/** session variable for existing user certificates in self service */
	const SESS_CERTIFICATES_LIST = 'inetOrgPerson_certificatesList';
	/** session variable for existing user certificates in self service */
	const SESS_PHOTO = 'inetOrgPerson_jpegPhoto';

	/**
	* This function fills the message array.
	**/
	function load_Messages() {
		$this->messages['givenName'][0] = array('ERROR', _('First name'), _('First name contains invalid characters!'));
		$this->messages['givenName'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_firstName', _('First name contains invalid characters!'));
		$this->messages['lastname'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters or is empty!'));
		$this->messages['lastname'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_lastName', _('Last name contains invalid characters or is empty!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_telephone', _('Please enter a valid telephone number!'));
		$this->messages['homePhone'][0] = array('ERROR', _('Home telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['homePhone'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_homePhone', _('Please enter a valid telephone number!'));
		$this->messages['mobile'][0] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		$this->messages['mobileTelephone'][1] = array('ERROR', _('Account %s:') . " inetOrgPerson_mobile", _('Please enter a valid mobile number!'));
		$this->messages['facsimileTelephoneNumber'][0] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		$this->messages['facsimileNumber'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_fax', _('Please enter a valid fax number!'));
		$this->messages['pager'][0] = array('ERROR', _('Pager'), _('Please enter a valid telephone number!'));
		$this->messages['pager'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_pager', _('Please enter a valid telephone number!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('WARN', _('Email address'), _('Email "%s" already in use.'));
		$this->messages['mail'][2] = array('WARN', _('Account %s:') . ' inetOrgPerson_email', _('Email "%s" already in use.'));
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
		$this->messages['uid'][3] = array('WARN', _('Account %s:') . ' inetOrgPerson_userName', _('User name already exists!'));
		$this->messages['manager'][0] = array('ERROR', _('Account %s:') . ' inetOrgPerson_manager', _('This is not a valid DN!'));
		$this->messages['file'][0] = array('ERROR', _('No file selected.'));
		$this->messages['file'][2] = array('ERROR', _('Unable to process this file.'));
		$this->messages['file'][3] = array('ERROR', _('File is too large. Maximum allowed size is %s kB.'));
		$this->messages['businessCategory'][0] = array('ERROR', _('Business category'), _('Please enter a valid business category!'));
		$this->messages['businessCategory'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_businessCategory', _('Please enter a valid business category!'));
		$this->messages['userPassword'][0] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['sendPasswordMail'][0] = array('ERROR', _('Account %s:') . ' inetOrgPerson_sendPasswordMail', _('This value can only be "true" or "false".'));
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'user.svg';
		// alias name
		$return["alias"] = _('Personal');
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "normal", 'uid' => 'low');
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=inetOrgPerson)");
		// module dependencies
		$return['dependencies'] = array('depends' => [], 'conflicts' => []);
		// managed object classes
		$return['objectClasses'] = array('inetOrgPerson');
		// LDAP aliases
		$return['LDAPaliases'] = array('commonName' => 'cn', 'surname' => 'sn', 'streetAddress' => 'street',
			'fax' => 'facsimileTelephoneNumber', 'gn' => 'givenName', 'userid' => 'uid', 'rfc822mailbox' => 'mail',
			'mobileTelephoneNumber' => 'mobile', 'organizationName' => 'o', 'organizationalUnitName' => 'ou');
		// managed attributes
		$return['attributes'] = array('uid', 'employeeType', 'givenName', 'jpegPhoto', 'mail', 'manager', 'mobile',
			'title', 'telephoneNumber', 'facsimileTelephoneNumber', 'street', 'postOfficeBox', 'postalCode', 'postalAddress',
			'sn', 'userpassword', 'description', 'homePhone', 'pager', 'roomNumber', 'businessCategory', 'l', 'st',
			'physicalDeliveryOfficeName', 'carLicense', 'departmentNumber', 'o', 'employeeNumber', 'initials',
			'registeredAddress', 'labeledURI', 'ou', 'userCertificate;binary', 'INFO.userPasswordClearText');
		// self service search attributes
		$return['selfServiceSearchAttributes'] = array('uid', 'mail', 'cn', 'surname', 'givenName', 'employeeNumber');
		// self service field settings
		$return['selfServiceFieldSettings'] = array('firstName' => _('First name'), 'lastName' => _('Last name'),
			'mail' => _('Email address'), 'telephoneNumber' => _('Telephone number'), 'mobile' => _('Mobile number'),
			'faxNumber' => _('Fax number'), 'street' => _('Street'), 'postalAddress' => _('Postal address'), 'registeredAddress' => _('Registered address'),
			'postalCode' => _('Postal code'), 'postOfficeBox' => _('Post office box'), 'jpegPhoto' => _('Photo'),
			'homePhone' => _('Home telephone number'), 'pager' => _('Pager'), 'roomNumber' => _('Room number'), 'carLicense' => _('Car license'),
			'location' => _('Location'), 'state' => _('State'), 'officeName' => _('Office name'), 'businessCategory' => _('Business category'),
			'departmentNumber' => _('Department'), 'initials' => _('Initials'), 'title' => _('Job title'), 'labeledURI' => _('Web site'),
			'userCertificate' => _('User certificates'), 'o' => _('Organisation'), 'ou' => _('Organisational unit'), 'description' => _('Description'),
			'uid' => _('User name'), 'displayName' => _('Display name'));
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = array('firstName', 'lastName', 'mail', 'telephoneNumber', 'mobile', 'faxNumber', 'pager', 'street',
			'postalAddress', 'registeredAddress', 'postalCode', 'postOfficeBox', 'jpegPhoto', 'homePhone', 'roomNumber', 'carLicense',
			'location', 'state', 'officeName', 'businessCategory', 'departmentNumber', 'initials', 'title', 'labeledURI', 'userCertificate',
			'o', 'ou', 'description', 'uid', 'displayName');
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
				'example' => _('Temp, contract till December')
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
				'example' => _('MyCity')
			);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_registeredAddress',
				'description' => _('Registered address'),
				'help' => 'registeredAddress',
				'example' => _('MyCity')
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true)) {
			$return['upload_columns'][] = array(
				'name' => 'inetOrgPerson_pager',
				'description' => _('Pager'),
				'help' => 'pagerList',
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
			if (isLAMProVersion()) {
				$return['upload_columns'][] = array(
					'name' => 'inetOrgPerson_sendPasswordMail',
					'description' => _('Send password via mail'),
					'help' => 'mailPassword',
					'values' => 'true, false',
					'default' => 'false',
					'example' => 'false'
				);
			}
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true)) {
			$return['PDF_fields']['pager'] = _('Pager');
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto')) {
			$return['PDF_fields']['jpegPhoto'] = _('Photo');
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
			'mobile' => array (
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
			'pager' => array (
				"Headline" => _('Pager'), 'attr' => 'pager',
				"Text" => _("The user's pager number.")
			),
			'pagerList' => array (
				"Headline" => _('Pager'), 'attr' => 'pager',
				"Text" => _("The user's pager number.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mail' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The user's email address.")
			),
			'mailList' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The user's email address.") . ' ' . _('Multiple values are separated by semicolon.')
			),
			"mailPassword" => array (
				"Headline" => _("Send password via mail"),
				"Text" => _("Sends the password to the user via mail. Please edit your LAM server profile to setup the mail settings.")),
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
			'displayName' => array (
				"Headline" => _("Display name"), 'attr' => 'displayName',
				"Text" => _("This is the user's preferred name to be used when displaying entries.")
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
				"Text" => _("Business category (e.g. Administration, IT-Services, Management, ...)")
			),
			'businessCategoryList' => array(
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Management, ...)") . '. ' . _("Multiple values are separated by semicolon.")
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
				"Text" => _("LAM supports a large number of possibilities to generate the hash value of passwords. CRYPT-SHA512 and SSHA are the most common. We do not recommend to use plain text passwords unless passwords are hashed server-side.")
						. ' ' . _('K5KEY is only needed if you use Kerberos with smbk5pwd.')
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
			'crop' => array(
				"Headline" => _('Image cropping'),
				"Text" => _('Uploaded images will be cropped to these maximum values.')
			),
			'addAddressbook' => array(
				"Headline" => _('Add addressbook (ou=addressbook)'),
				"Text" => _('Adds an "ou=addressbook" subentry to each user.')
			),
		);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	public function getManagedAttributes($typeId) {
		$attrs = parent::getManagedAttributes($typeId);
		if (!$this->isUnixActive()) {
			$attrs[] = 'cn';
		}
		if (!$this->isSamba3Active()) {
			$attrs[] = 'displayName';
		}
		return $attrs;
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
		if (!$this->isUnixActive()) {
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
			return [];
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
			$return[$this->getAccountContainer()->dn_orig]['modify']['jpegPhoto'] = [];
			unset($return[$this->getAccountContainer()->dn_orig]['remove']['jpegPhoto']);
		}
		// add information about clear text password
		if ($this->clearTextPassword != null) {
			$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordClearText'][0] = $this->clearTextPassword;
		}
		// password status change
		if (!$this->isUnixActive()) {
			$pwdOrig = empty($this->orig['userpassword'][0]) ? '' : $this->orig['userpassword'][0];
			$pwdNew = empty($this->attributes['userpassword'][0]) ? '' : $this->attributes['userpassword'][0];
			if ((pwd_is_enabled($pwdOrig) && pwd_is_enabled($pwdNew)) || (!pwd_is_enabled($pwdOrig) && !pwd_is_enabled($pwdNew))) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unchanged';
			}
			elseif (pwd_is_enabled($pwdOrig)) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'locked';
			}
			else {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unlocked';
			}
		}
		return $return;
	}

	/**
	 * Runs the postmodify actions.
	 *
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
		$messages = [];
		// set exop password
		$messages = array_merge($messages, $this->setExopPassword($this->moduleSettings));
		// add address book
		$accountContainer = $this->getAccountContainer();
		if ($this->isBooleanConfigOptionSet('inetOrgPerson_addAddressbook')
			&& !empty($accountContainer)
			&& !empty($accountContainer->finalDN)) {
			$dn = 'ou=addressbook,' . $accountContainer->finalDN;
			$result = ldapGetDN($dn);
			if (empty($result)) {
				$attrs = array(
					'objectClass' => array('organizationalUnit'),
					'ou' => 'addressbook'
				);
				$success = @ldap_add($_SESSION['ldap']->server(), $dn, $attrs);
				if (!$success) {
					logNewMessage(LOG_ERR, 'Unable to add addressbook for user ' . $accountContainer->finalDN . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_("Was unable to create DN: %s."), htmlspecialchars($dn)), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, 'Added addressbook for user ' . $accountContainer->finalDN);
				}
			}
		}
		return $messages;
	}

	/**
	 * Sets the password via ldap_exop if configured.
	 *
	 * @param array $settings settings
	 * @return array error message parameters if any
	 */
	private function setExopPassword($settings) {
		if (!empty($this->clearTextPassword) && !empty($settings['posixAccount_pwdHash'][0])
				&& ($settings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
			$success = ldap_exop_passwd($_SESSION['ldap']->server(), $this->getAccountContainer()->finalDN, null, $this->clearTextPassword);
			if (!$success) {
				return array(array('ERROR', _('Unable to set password'), getExtendedLDAPErrorMessage($_SESSION['ldap']->server())));
			}
		}
		return [];
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = [];
		$keysToReplace = array('mail', 'description', 'postalAddress', 'cn',
			'registeredAddress', 'labeledURI');
		if ($this->isUnixActive()) {
			$keysToReplace[] = 'uid';
		}
		$this->getAccountContainer()->replaceWildcardsInPOST($keysToReplace);
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription') && !$this->isAdminReadOnly('description')) {
			$this->processMultiValueInputTextField('description', $errors);
		}
		// last name
		if (!$this->isAdminReadOnly('sn')) {
			$this->attributes['sn'][0] = trim($_POST['sn']);
			if (!get_preg($this->attributes['sn'][0], 'realname')) $errors[] = $this->messages['lastname'][0];
		}
		// first name
		if (!$this->isAdminReadOnly('givenName')) {
			$this->attributes['givenName'][0] = trim($_POST['givenName']);
			if (($this->attributes['givenName'][0] != '') && !get_preg($this->attributes['givenName'][0], 'realname')) $errors[] = $this->messages['givenName'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle') && !$this->isAdminReadOnly('title')) {
			$this->processMultiValueInputTextField('title', $errors, 'title');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') && !$this->isAdminReadOnly('mail')) {
			$this->processMultiValueInputTextField('mail', $errors, 'email');
			if (!empty($this->attributes['mail'])) {
				foreach ($this->attributes['mail'] as &$mail) {
					if (empty($this->orig['mail']) || !in_array($mail, $this->orig['mail'])) {
						if ($this->emailExists($mail)) {
							$msg = $this->messages['mail'][1];
							$msg[] = array(htmlspecialchars($mail));
							$errors[] = $msg;
						}
					}
				}
			}
		}
		if (!$this->isSamba3Active() && !$this->isBooleanConfigOptionSet('inetOrgPerson_hidedisplayName', true)) {
			$this->attributes['displayName'][0] = $_POST['displayName'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') && !$this->isAdminReadOnly('telephoneNumber')) {
			$this->processMultiValueInputTextField('telephoneNumber', $errors, 'telephone');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber') && !$this->isAdminReadOnly('mobile')) {
			$this->processMultiValueInputTextField('mobile', $errors, 'telephone');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber') && !$this->isAdminReadOnly('facsimileTelephoneNumber')) {
			$this->processMultiValueInputTextField('facsimileTelephoneNumber', $errors, 'telephone');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true) && !$this->isAdminReadOnly('pager')) {
			$this->processMultiValueInputTextField('pager', $errors, 'telephone');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber') && !$this->isAdminReadOnly('homePhone')) {
			$this->processMultiValueInputTextField('homePhone', $errors, 'telephone');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') && !$this->isAdminReadOnly('street')) {
			$this->processMultiValueInputTextField('street', $errors, 'street');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox') && !$this->isAdminReadOnly('postOfficeBox')) {
			$this->processMultiValueInputTextField('postOfficeBox', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') && !$this->isAdminReadOnly('postalCode')) {
			$this->processMultiValueInputTextField('postalCode', $errors, 'postalCode');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress') && !$this->isAdminReadOnly('postalAddress')) {
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI') && !$this->isAdminReadOnly('labeledURI')) {
			$this->processMultiValueInputTextField('labeledURI', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress') && !$this->isAdminReadOnly('registeredAddress')) {
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType') && !$this->isAdminReadOnly('employeeType')) {
			$this->attributes['employeeType'][0] = $_POST['employeeType'];
			if (!get_preg($this->attributes['employeeType'][0], 'employeeType')) $errors[] = $this->messages['employeeType'][0];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber') && !$this->isAdminReadOnly('roomNumber')) {
			$this->attributes['roomNumber'][0] = $_POST['roomNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation') && !$this->isAdminReadOnly('l')) {
			$this->processMultiValueInputTextField('l', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') && !$this->isAdminReadOnly('st')) {
			$this->processMultiValueInputTextField('st', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense') && !$this->isAdminReadOnly('carLicense')) {
			$this->attributes['carLicense'][0] = $_POST['carLicense'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName') && !$this->isAdminReadOnly('physicalDeliveryOfficeName')) {
			$this->processMultiValueInputTextField('physicalDeliveryOfficeName', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory') && !$this->isAdminReadOnly('businessCategory')) {
			$this->processMultiValueInputTextField('businessCategory', $errors, 'businessCategory');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments') && !$this->isAdminReadOnly('departmentNumber')) {
			$this->processMultiValueInputTextField('departmentNumber', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber') && !$this->isAdminReadOnly('employeeNumber')) {
			$this->attributes['employeeNumber'][0] = $_POST['employeeNumber'];
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu') && !$this->isAdminReadOnly('ou')) {
			$this->processMultiValueInputTextField('ou', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO') && !$this->isAdminReadOnly('o')) {
			$this->processMultiValueInputTextField('o', $errors);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials') && !$this->isAdminReadOnly('initials')) {
			$this->attributes['initials'] = preg_split('/;[ ]*/', $_POST['initials']);
		}

		if (!$this->isUnixActive()) {
			// uid
			if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID') && !$this->isAdminReadOnly('uid')) {
				if (isset($_POST['uid']) && ($_POST['uid'] != '')) {
					if (!get_preg($_POST['uid'], 'username')) {
						$errors[] = $this->messages['uid'][0];
					}
					else {
						$this->attributes['uid'][0] = $_POST['uid'];
					}
				}
				elseif (isset($this->attributes['uid'][0])) {
					unset($this->attributes['uid'][0]);
				}
			}
			// cn
			if (!$this->isAdminReadOnly('cn')) {
				$this->processMultiValueInputTextField('cn', $errors, 'cn');
				if (empty($this->attributes['cn'][0])) {
					if ($_POST['givenName'] != '') {
						$this->attributes['cn'][0] = $_POST['givenName'] . " " . $_POST['sn'];
					}
					else {
						$this->attributes['cn'][0] = $_POST['sn'];
					}
				}
			}
			if (!$this->isAdminReadOnly('userPassword')) {
				if (isset($_POST['lockPassword'])) {
					$this->attributes['userpassword'][0] = pwd_disable($this->attributes['userpassword'][0]);
				}
				if (isset($_POST['unlockPassword'])) {
					$this->attributes['userpassword'][0] = pwd_enable($this->attributes['userpassword'][0]);
				}
				if (isset($_POST['removePassword'])) {
					unset($this->attributes['userpassword']);
				}
				// set SASL password for new and renamed users
				if (!empty($this->attributes['uid'][0]) && !empty($this->moduleSettings['posixAccount_pwdHash'][0])
						&& ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')
						&& ($this->getAccountContainer()->isNewAccount || ($this->attributes['uid'][0] != $this->orig['uid'][0]))) {
					$this->attributes['userpassword'][0] = '{SASL}' . $this->attributes['uid'][0];
				}
				// set K5KEY password for new users
				if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
					$this->attributes[$this->getPasswordAttrName()][0] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto') && isset($_POST['delPhoto']) && !$this->isAdminReadOnly('jpegPhoto')) {
			$this->attributes['jpegPhoto'] = [];
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
		$this->initCache();
        $keysToReplace = array('mail', 'description', 'postalAddress', 'uid', 'cn',
            'registeredAddress', 'labeledURI');
        if ($this->isUnixActive()) {
            $keysToReplace[] = 'uid';
        }
        $this->getAccountContainer()->replaceWildcardsInArray($keysToReplace, $this->attributes);
		$container = new htmlResponsiveRow();
		$fieldContainer = new htmlResponsiveRow();
		$fieldTabletColumns = $this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto') ? 12 : 8;
		$container->add($fieldContainer, 12, 12, $fieldTabletColumns);
		// uid
		if (!$this->isUnixActive() && !$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
			if ($this->isAdminReadOnly('uid')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'uid', _('User name'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'uid', _('User name'));
			}
		}
		// first name
		if ($this->isAdminReadOnly('givenName')) {
			$this->addSimpleReadOnlyField($fieldContainer, 'givenName', _('First name'));
		}
		else {
			$this->addSimpleInputTextField($fieldContainer, 'givenName', _('First name'));
		}
		// last name
		if ($this->isAdminReadOnly('sn')) {
			$this->addSimpleReadOnlyField($fieldContainer, 'sn', _('Last name'));
		}
		else {
			$this->addSimpleInputTextField($fieldContainer, 'sn', _('Last name'), true);
		}
		// initials
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			if ($this->isAdminReadOnly('initials')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'initials', _('Initials'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'initials', _('Initials'));
			}
		}
		// common name
		if (!$this->isUnixActive()) {
			if ($this->isAdminReadOnly('cn')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'cn', _('Common name'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'cn', _('Common name'), true);
			}
		}
		// display name
		if (!$this->isSamba3Active() && !$this->isBooleanConfigOptionSet('inetOrgPerson_hidedisplayName', true)) {
			$this->addSimpleInputTextField($fieldContainer, 'displayName', _('Display name'));
		}
		// description
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			if ($this->isAdminReadOnly('description')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'description', _('Description'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'description', _('Description'));
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$fieldContainer->add(new htmlSubTitle(_('Address')), 12);
		}

		// street
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			if ($this->isAdminReadOnly('street')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'street', _('Street'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'street', _('Street'));
			}
		}
		// post office box
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			if ($this->isAdminReadOnly('postOfficeBox')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'postOfficeBox', _('Post office box'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'postOfficeBox', _('Post office box'));
			}
		}
		// postal code
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			if ($this->isAdminReadOnly('postalCode')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'postalCode', _('Postal code'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'postalCode', _('Postal code'));
			}
		}
		// location
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			if ($this->isAdminReadOnly('l')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'l', _('Location'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'l', _('Location'));
			}
		}
		// state
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			if ($this->isAdminReadOnly('st')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'st', _('State'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'st', _('State'));
			}
		}
		// postal address
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$postalAddresses = [];
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
			$fieldContainer->addLabel($addressLabel);
			$addressContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($postalAddresses); $i++) {
				if ($this->isAdminReadOnly('postalAddress')) {
					$val = str_replace("\r\n", '<br>', htmlspecialchars($postalAddresses[$i]));
					$addressContainer->addElement(new htmlOutputText($val, false));
					if ($i < (sizeof($postalAddresses) - 1)) {
						$addressContainer->addElement(new htmlOutputText('<br>', false));
					}
				}
				else {
					$postalAddressTextarea = new htmlInputTextarea('postalAddress' . $i, $postalAddresses[$i], 30, 3);
					$postalAddressTextarea->setAccessibilityLabel(_('Postal address'));
					$addressContainer->addElement($postalAddressTextarea);
					if ($i < (sizeof($postalAddresses) - 1)) {
						$addressContainer->addElement(new htmlOutputText('<br>', false));
					}
					else {
						$addButton = new htmlButton('addPostalAddress', 'add.svg', true);
						$addButton->setTitle(_('Add'));
						$addressContainer->addElement($addButton);
						$addressHelp = new htmlHelpLink('postalAddress');
						$addressContainer->addElement($addressHelp);
					}
				}
			}
			$fieldContainer->addField($addressContainer);
		}
		// registered address
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$registeredAddresses = [];
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
			$fieldContainer->addLabel($registeredAddressLabel);
			$registeredAddressContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($registeredAddresses); $i++) {
				if ($this->isAdminReadOnly('registeredAddress')) {
					$val = str_replace("\r\n", '<br>', htmlspecialchars($registeredAddresses[$i]));
					$registeredAddressContainer->addElement(new htmlOutputText($val, false));
					if ($i < (sizeof($registeredAddresses) - 1)) {
						$registeredAddressContainer->addElement(new htmlOutputText('<br>', false));
					}
				}
				else {
					$registeredAddressTextarea = new htmlInputTextarea('registeredAddress' . $i, $registeredAddresses[$i], 30, 3);
					$registeredAddressTextarea->setAccessibilityLabel(_('Registered address'));
					$registeredAddressContainer->addElement($registeredAddressTextarea);
					if ($i < (sizeof($registeredAddresses) - 1)) {
						$registeredAddressContainer->addElement(new htmlOutputText('<br>', false));
					}
					else {
						$addButton = new htmlButton('addRegisteredAddress', 'add.svg', true);
						$addButton->setTitle(_('Add'));
						$registeredAddressContainer->addElement($addButton);
						$registeredAddressHelp = new htmlHelpLink('registeredAddress');
						$registeredAddressContainer->addElement($registeredAddressHelp);
					}
				}
			}
			$fieldContainer->addField($registeredAddressContainer);
		}
		// office name
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			if ($this->isAdminReadOnly('physicalDeliveryOfficeName')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'physicalDeliveryOfficeName', _('Office name'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'physicalDeliveryOfficeName', _('Office name'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			if ($this->isAdminReadOnly('roomNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'roomNumber', _('Room number'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'roomNumber', _('Room number'));
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$fieldContainer->add(new htmlSubTitle(_('Contact data')), 12);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			if ($this->isAdminReadOnly('telephoneNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'telephoneNumber', _('Telephone number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'telephoneNumber', _('Telephone number'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			if ($this->isAdminReadOnly('homePhone')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'homePhone', _('Home telephone number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'homePhone', _('Home telephone number'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			if ($this->isAdminReadOnly('mobile')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'mobile', _('Mobile number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'mobile', _('Mobile number'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			if ($this->isAdminReadOnly('facsimileTelephoneNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'facsimileTelephoneNumber', _('Fax number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'facsimileTelephoneNumber', _('Fax number'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true)) {
			if ($this->isAdminReadOnly('pager')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'pager', _('Pager'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'pager', _('Pager'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			if ($this->isAdminReadOnly('mail')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'mail', _('Email address'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'mail', _('Email address'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			if ($this->isAdminReadOnly('labeledURI')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'labeledURI', _('Web site'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'labeledURI', _('Web site'));
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideuserCertificate')) {
			$fieldContainer->add(new htmlSubTitle(_('Work details')), 12);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			if ($this->isAdminReadOnly('title')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'title', _('Job title'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'title', _('Job title'), false, null, false, array_slice($this->titleCache, 0, 300));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			if ($this->isAdminReadOnly('carLicense')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'carLicense', _('Car license'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'carLicense', _('Car license'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			if ($this->isAdminReadOnly('employeeNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'employeeNumber', _('Employee number'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'employeeNumber', _('Employee number'));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			if ($this->isAdminReadOnly('employeeType')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'employeeType', _('Employee type'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'employeeType', _('Employee type'), false, null, false, array_slice($this->employeeTypeCache, 0, 300));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			if ($this->isAdminReadOnly('businessCategory')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'businessCategory', _('Business category'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'businessCategory', _('Business category'), false, null, false, array_slice($this->businessCategoryCache, 0, 300));
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			if ($this->isAdminReadOnly('departmentNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'departmentNumber', _('Department'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'departmentNumber', _('Department'), false, null, false, array_slice($this->departmentCache, 0, 300));
			}
		}
		// organisational unit
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			if ($this->isAdminReadOnly('ou')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'ou', _('Organisational unit'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'ou', _('Organisational unit'), false, null, false, array_slice($this->ouCache, 0, 300));
			}
		}
		// organisation
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			if ($this->isAdminReadOnly('o')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'o', _('Organisation'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'o', _('Organisation'), false, null, false, array_slice($this->oCache, 0, 300));
			}
		}
		// user certificates
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideuserCertificate')) {
			$fieldContainer->addVerticalSpacer('0.5rem');
			$fieldContainer->addLabel(new htmlOutputText(_('User certificates')));
			$userCertificateGroup = new htmlGroup();
			$userCertificateCount = 0;
			if (isset($this->attributes['userCertificate;binary'])) {
				$userCertificateCount = sizeof($this->attributes['userCertificate;binary']);
			}
			$userCertificateGroup->addElement(new htmlOutputText($userCertificateCount));
			$userCertificateGroup->addElement(new htmlSpacer('10px', null));
			if (!$this->isAdminReadOnly('manager')) {
				$userCertificateGroup->addElement(new htmlAccountPageButton(get_class($this), 'userCertificate', 'manage', _('Manage')));
				$userCertificateGroup->addElement(new htmlHelpLink('userCertificate'));
			}
			$fieldContainer->addField($userCertificateGroup);
		}
		// manager
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$fieldContainer->addVerticalSpacer('0.5rem');
			$fieldContainer->addLabel(new htmlOutputText(_('Manager')));
			if (!$this->isAdminReadOnly('manager')) {
				$managerButtonGroup = new htmlGroup();
				$managerButtonGroup->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'change', _("Change")));
				$managerButtonGroup->addElement(new htmlHelpLink('manager'));
				$fieldContainer->addField($managerButtonGroup);
			}
			if (isset($this->attributes['manager'][0])) {
				$managerList = [];
				for ($i = 0; $i < sizeof($this->attributes['manager']); $i++) {
					$managerList[] = $this->attributes['manager'][$i];
				}
				usort($managerList, 'compareDN');
				$managers = new htmlTable();
				$managers->alignment = htmlElement::ALIGN_RIGHT;
				$managers->colspan = 3;
				for ($i = 0; $i < sizeof($managerList); $i++) {
					$manager = new htmlOutputText(getAbstractDN($managerList[$i]));
					$manager->alignment = htmlElement::ALIGN_RIGHT;
					$managers->addElement($manager, true);
				}
				$fieldContainer->addLabel(new htmlOutputText('&nbsp;', false));
				$fieldContainer->addField($managers);
			}
		}
		// password buttons
		if (!$this->isUnixActive() && checkIfWriteAccessIsAllowed($this->get_scope()) && isset($this->attributes['userpassword'][0]) && !$this->isAdminReadOnly('userPassword')) {
			$fieldContainer->add(new htmlSubTitle(_('Password')), 12);
			$pwdContainer = new htmlGroup();
			if (pwd_is_enabled($this->attributes['userpassword'][0])) {
				$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
			}
			else {
				$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
			}
			$pwdContainer->addElement(new htmlSpacer('0.5rem', null));
			$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
			$fieldContainer->add($pwdContainer, 12, 12, 12, 'text-center');
		}
		// photo
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto')) {
			$imageContainer = new htmlTable();
			$imageContainer->setCSSClasses(array('div-center'));
			$imageContainer->alignment = htmlElement::ALIGN_TOP;
			$photoFile = '../../graphics/user.svg';
			$noPhoto = true;
			if (isset($this->attributes['jpegPhoto'][0])) {
				try {
					$temporaryFilesManager = new LamTemporaryFilesManager();
					$jpeg_filename = $temporaryFilesManager->registerTemporaryFile('.jpg');
					$handle = $temporaryFilesManager->openTemporaryFileForWrite($jpeg_filename);
					fwrite($handle, $this->attributes['jpegPhoto'][0]);
					fclose($handle);
					$photoFile = $temporaryFilesManager->getResourceLink($jpeg_filename);
				}
				catch (LAMException $e) {
					logNewMessage(LOG_ERR, $e->getTitle());
				}
				$noPhoto = false;
			}
			$img = new htmlImage($photoFile);
			$img->setCSSClasses(array('photo'));
			$img->enableLightbox();
			$imageContainer->addElement($img, true);
			if (!$this->isAdminReadOnly('jpegPhoto')) {
				if ($noPhoto) {
					$imageContainer->addElement(new htmlAccountPageButton(get_class($this), 'photo', 'open', _('Add photo')));
				}
				else {
					$imageContainer->addElement(new htmlButton('delPhoto', _('Delete photo')));
				}
			}
			$container->add($imageContainer, 12, 12, 4);
		}
		return $container;
	}

	/**
	* Sets a new photo.
	*
	* @return array list of error messages if any
	*/
	public function process_photo() {
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_back'])) {
			return [];
		}
		if ($this->isAdminReadOnly('jpegPhoto')) {
			return [];
		}
		if (isset($_POST['form_subpage_' . get_class($this) . '_photo_upload']) || isset($_POST['webcamData'])) {
			return $this->uploadPhoto();
		}
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_crop'])) {
			$messages = [];
			try {
				include_once __DIR__ . '/../imageutils.inc';
				$imageManipulator = ImageManipulationFactory::getImageManipulator($this->attributes['jpegPhoto'][0]);
				$imageManipulator->crop($_POST['croppingDataX'], $_POST['croppingDataY'], $_POST['croppingDataWidth'], $_POST['croppingDataHeight']);
				$this->attributes['jpegPhoto'][0] = $imageManipulator->getImageData();
			}
			catch (Exception $e) {
				$msg =  $this->messages['file'][2];
				$msg[] = htmlspecialchars($e->getMessage());
				$messages[] = $msg;
			}
			return $messages;
		}
	}

	/**
	 * Uploads the photo file.
	 *
	 * @return array error messages if any
	 */
	private function uploadPhoto() {
		$messages = [];
		if ((empty($_FILES['photoFile']) || ($_FILES['photoFile']['size'] <= 0)) && empty($_POST['webcamData'])) {
			$messages[] = $this->messages['file'][0];
			return $messages;
		}
		if (!empty($_FILES['photoFile']['tmp_name'])) {
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 100000000);
			fclose($handle);
			if (!empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]) && (strlen($data) > (1024 * $this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]))) {
				$errMsg = $this->messages['file'][3];
				$errMsg[] = null;
				$errMsg[] = array($this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]);
				return array($errMsg);
			}
		}
		elseif (isset($_POST['webcamData'])) {
			$data = $_POST['webcamData'];
			$data = str_replace('data:image/png;base64,', '', $data);
			$data = base64_decode($data);
		}
		// convert to JPG
		try {
			include_once __DIR__ . '/../imageutils.inc';
			$imageManipulator = ImageManipulationFactory::getImageManipulator($data);
			// resize if maximum values specified
			if (!empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0]) || !empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0])) {
				$maxWidth = empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0]) ? $imageManipulator->getWidth() : $this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0];
				$maxHeight = empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0]) ? $imageManipulator->getHeight() : $this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0];
				$imageManipulator->thumbnail($maxWidth, $maxHeight);
			}
			$imageManipulator->convertToJpeg();
			$data = $imageManipulator->getImageData();
		}
		catch (Exception $e) {
			$msg =  $this->messages['file'][2];
			$msg[] = htmlspecialchars($e->getMessage());
			$messages[] = $msg;
			return $messages;
		}
		$this->attributes['jpegPhoto'][0] = $data;
		return $messages;
	}

	/**
	* Displays the photo upload page.
	*
	* @return array meta HTML code
	*/
	public function display_html_photo() {
		$container = new htmlResponsiveRow();
		if (empty($this->attributes['jpegPhoto'][0])) {
			$container->add(new htmlSubTitle(_('Upload image')), 12);
			$label = _('Photo file');
			$container->add(new htmlResponsiveInputFileUpload('photoFile', $label, 'photoUpload'), 12);
			$container->addVerticalSpacer('0.5rem');
			$container->addLabel(new htmlOutputText('&nbsp;', false));
			$container->addField(new htmlAccountPageButton(get_class($this), 'photo', 'upload', _('Upload')));
			$container->addVerticalSpacer('1rem');
			$webcamContent = new htmlResponsiveRow();
			$webcamContent->add(new htmlSubTitle(_('Use webcam')), 12);
			$errorMessage = new htmlStatusMessage('ERROR', '');
			$errorMessage->setCSSClasses(array('hidden', 'lam-webcam-message'));
			$webcamContent->add($errorMessage, 12);
			$captureButton = new htmlButton('lam-webcam-capture', _('Start capture'));
			$captureButton->setOnClick('window.lam.tools.webcam.capture(event);');
			$webcamContent->add($captureButton, 12, 12, 12, 'text-center');
			$video = new htmlVideo('lam-webcam-video');
			$video->setCSSClasses(array('hidden'));
			$webcamContent->add($video, 12, 12, 12, 'text-center');
			$webcamContent->addVerticalSpacer('0.5rem');
			$webcamUploadButton = new htmlButton('uploadWebcam', _('Upload'));
			$webcamUploadButton->setCSSClasses(array('btn-lam-webcam-upload', 'hidden'));
			$webcamUploadButton->setOnClick('window.lam.tools.webcam.upload();');
			$webcamContent->add($webcamUploadButton, 12, 12, 12, 'text-center');
			$canvas = new htmlCanvas('lam-webcam-canvas');
			$canvas->setCSSClasses(array('hidden'));
			$webcamContent->add($canvas, 12);
			$webcamDiv = new htmlDiv('lam_webcam_div', $webcamContent, array('hidden'));
			$container->add($webcamDiv, 12);
			$container->addVerticalSpacer('1rem');
			$container->add(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')), 12);
		}
		else {
			$container->add(new htmlSubTitle(_('Crop image')));
			try {
				$tempFilesManager = new LamTemporaryFilesManager();
				$jpeg_filename = $tempFilesManager->registerTemporaryFile('.jpg');
				$handle = $tempFilesManager->openTemporaryFileForWrite($jpeg_filename);
				fwrite($handle, $this->attributes['jpegPhoto'][0]);
				fclose($handle);
				$photoFile = $tempFilesManager->getResourceLink($jpeg_filename);
				$img = new htmlImage($photoFile);
				$img->setCSSClasses(array('photo'));
				$img->enableCropping();
				$container->add($img);
			}
			catch (LAMException $e) {
				logNewMessage(LOG_ERR, $e->getTitle());
			}
			$container->addVerticalSpacer('1rem');
			$doneButton = new htmlAccountPageButton(get_class($this), 'attributes', 'crop', _('Done'));
			$container->add($doneButton);
		}
		return $container;
	}

	/**
	* This function will create the meta HTML code to show a page to change the manager attribute.
	*
	* @return htmlElement HTML meta data
	*/
	function display_html_manager() {
		$return = new htmlResponsiveRow();
		if (!isset($this->attributes['manager'])) {
			$this->attributes['manager'] = [];
		}
		// show list of possible new managers
		if (isset($_POST['form_subpage_' . get_class($this) . '_manager_select'])) {
			$options = [];
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
			if (sizeof($options) < 20) {
				$size = sizeof($options);
			}
			$managerSelect = new htmlSelect('manager', $options, [], $size);
			$managerSelect->setHasDescriptiveElements(true);
			$managerSelect->setMultiSelect(true);
			$managerSelect->setRightToLeftTextDirection(true);
			$managerSelect->setSortElements(false);
			$managerSelect->setTransformSingleSelect(false);
			$return->add($managerSelect, 12);
			$return->addVerticalSpacer('1rem');
			$return->addLabel(new htmlAccountPageButton(get_class($this), 'manager', 'addManagers', _('Add')));
			$return->addField(new htmlAccountPageButton(get_class($this), 'manager', 'cancel', _('Cancel')));
			return $return;
		}
		// show existing managers
		$managerTemp = [];
		if (isset($this->attributes['manager'])) {
			$managerTemp = $this->attributes['manager'];
		}
		// sort by DN
		usort($managerTemp, 'compareDN');
		$managers = [];
		for ($i = 0; $i < sizeof($managerTemp); $i++) {
			$managers[getAbstractDN($managerTemp[$i])] = $managerTemp[$i];
		}
		$size = 20;
		if (sizeof($this->attributes['manager']) < 20) {
			$size = sizeof($this->attributes['manager']);
		}
		if (sizeof($managers) > 0) {
			$managerSelect = new htmlSelect('manager', $managers, [], $size);
			$managerSelect->setHasDescriptiveElements(true);
			$managerSelect->setMultiSelect(true);
			$managerSelect->setRightToLeftTextDirection(true);
			$managerSelect->setSortElements(false);
			$managerSelect->setTransformSingleSelect(false);
			$return->add($managerSelect, 12);
			$return->addVerticalSpacer('0.5rem');
			$removeButton = new htmlAccountPageButton(get_class($this), 'manager', 'remove', _('Remove selected entries'));
			$return->add($removeButton, 12, 12, 12, 'text-center');
			$return->addVerticalSpacer('1rem');
			$return->add(new htmlHorizontalLine(), 12);
		}
		$return->addVerticalSpacer('1rem');
		$return->addLabel(new htmlAccountPageButton(get_class($this), 'manager', 'select', _('Add entries')));
		$return->addField(new htmlAccountPageButton(get_class($this), 'attributes', 'managerBack', _('Back')));
		return $return;
	}

	/**
	* Processes user input of the manager page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_manager() {
		$return = [];
		if ($this->isAdminReadOnly('manager')) {
			return $return;
		}
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
		$container = new htmlResponsiveRow();
		if (isset($this->attributes['userCertificate;binary'])) {
			$table = new htmlTable();
			$table->colspan = 10;
			$temFilesManager = new LamTemporaryFilesManager();
			for ($i = 0; $i < sizeof($this->attributes['userCertificate;binary']); $i++) {
				$filename = $temFilesManager->registerTemporaryFile('.der');
				$out = $temFilesManager->openTemporaryFileForWrite($filename);
				fwrite($out, $this->attributes['userCertificate;binary'][$i]);
				fclose ($out);
				$path = $temFilesManager->getDownloadLink($filename);
				$link = new htmlLink('', $path, '../../graphics/save.svg');
				$link->setTargetWindow('_blank');
				$link->setCSSClasses(array('icon'));
				$table->addElement($link);
				$deleteButton = new htmlAccountPageButton(get_class($this), 'userCertificate', 'delete_' . $i, 'del.svg', true);
				$table->addElement($deleteButton);
				$pem = @chunk_split(@base64_encode($this->attributes['userCertificate;binary'][$i]), 64, "\n");
				if (!empty($pem)) {
					$pem = "-----BEGIN CERTIFICATE-----\n" . $pem . "-----END CERTIFICATE-----\n";
					$pemData = @openssl_x509_parse($pem);
					$data = [];
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
				$table->addNewLine();
			}
			$container->add($table, 12);
			$container->addVerticalSpacer('2rem');
		}
		$container->add(new htmlResponsiveInputFileUpload('userCertificateUpload', _('New user certificate')), 12, 6);

		$uploadButton = new htmlAccountPageButton(get_class($this), 'userCertificate', 'submit', _('Upload'));
		$container->add($uploadButton, 12, 6);
		$container->addVerticalSpacer('2rem');
		$container->add(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')), 12);
		return $container;
	}

	/**
	* Sets a new certificate or deletes old ones.
	*
	* @return array list of info/error messages
	*/
	function process_userCertificate() {
		$messages = [];
		if ($this->isAdminReadOnly('userCertificate')) {
			return $messages;
		}
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
	 * {@inheritDoc}
	 * @see baseModule::get_pdfFields()
	 */
	public function get_pdfFields($typeId) {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if (!$this->isUnixActive($modules)) {
			if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
				$fields['uid'] = _('User name');
			}
			$fields['cn'] = _('Common name');
			$fields['userPassword'] = _('Password');
		}
		if (!$this->isSamba3Active($modules)) {
			if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidedisplayName', true)) {
				$fields['displayName'] = _('Display name');
			}
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$this->addSimplePDFField($return, 'description', _('Description'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$this->addSimplePDFField($return, 'title', _('Job title'));
		}
		$this->addSimplePDFField($return, 'givenName', _('First name'));
		$this->addSimplePDFField($return, 'sn', _('Last name'));
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$this->addSimplePDFField($return, 'manager', _('Manager'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$this->addSimplePDFField($return, 'street', _('Street'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$this->addSimplePDFField($return, 'postOfficeBox', _('Post office box'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$this->addSimplePDFField($return, 'postalCode', _('Postal code'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')
			&& !empty($this->attributes['postalAddress'])) {
			$this->addPDFKeyValue($return, 'postalAddress', _('Postal address'), str_replace('$', "\n", implode("\n\n", $this->attributes['postalAddress'])));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$this->addSimplePDFField($return, 'registeredAddress', _('Registered address'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$this->addSimplePDFField($return, 'telephoneNumber', _('Telephone number'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			$this->addSimplePDFField($return, 'homePhone', _('Home telephone number'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			$this->addSimplePDFField($return, 'mobileTelephoneNumber', _('Mobile number'), 'mobile');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true)) {
			$this->addSimplePDFField($return, 'pager', _('Pager'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$this->addSimplePDFField($return, 'mail', _('Email address'));
		}
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$this->addSimplePDFField($return, 'facsimileTelephoneNumber', _('Fax number'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$this->addSimplePDFField($return, 'roomNumber', _('Room number'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$this->addSimplePDFField($return, 'businessCategory', _('Business category'));
		}
		$this->addSimplePDFField($return, 'uid', _('User name'));
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			$this->addSimplePDFField($return, 'carLicense', _('Car license'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$this->addSimplePDFField($return, 'state', _('State'), 'st');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$this->addSimplePDFField($return, 'officeName', _('Office name'), 'physicalDeliveryOfficeName');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$this->addSimplePDFField($return, 'employeeType', _('Employee type'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$this->addSimplePDFField($return, 'location', _('Location'), 'l');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			$this->addSimplePDFField($return, 'employeeNumber', _('Employee number'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$this->addSimplePDFField($return, 'ou', _('Organisational unit'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$this->addSimplePDFField($return, 'o', _('Organisation'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$this->addSimplePDFField($return, 'initials', _('Initials'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$this->addSimplePDFField($return, 'labeledURI', _('Web site'));
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$this->addSimplePDFField($return, 'departmentNumber', _('Department'));
		}
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		if (isset($this->clearTextPassword)) {
			$this->addPDFKeyValue($return, 'userPassword', _('Password'), $this->clearTextPassword);
		}
		else if (isset($this->attributes['INFO.userPasswordClearText'])) {
			$this->addPDFKeyValue($return, 'userPassword', _('Password'), $this->attributes['INFO.userPasswordClearText']);
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto')) {
			$this->addPDFImage($return, 'jpegPhoto');
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_profileOptions()
	 */
	public function get_profileOptions($typeId) {
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		$profileElements = [];
		if (!$this->isUnixActive($modules)) {
			$profileElements[] = new htmlResponsiveInputField(_('User name'), 'inetOrgPerson_uid', null, 'uid');
		}
		if (!$this->isUnixActive($modules)) {
			$profileElements[] = new htmlResponsiveInputField(_('Common name'), 'inetOrgPerson_cn', null, 'cn');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			$profileElements[] = new htmlResponsiveInputField(_('Initials'), 'inetOrgPerson_initials', null, 'initials');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			$profileElements[] = new htmlResponsiveInputField(_('Description'), 'inetOrgPerson_description', null, 'description');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			$profileElements[] = new htmlResponsiveInputField(_('Street'), 'inetOrgPerson_street', null, 'streetList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			$profileElements[] = new htmlResponsiveInputField(_('Post office box'), 'inetOrgPerson_postOfficeBox', null, 'postOfficeBoxList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			$profileElements[] = new htmlResponsiveInputField(_('Postal code'), 'inetOrgPerson_postalCode', null, 'postalCodeList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			$profileElements[] = new htmlResponsiveInputField(_('Location'), 'inetOrgPerson_l', null, 'lList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			$profileElements[] = new htmlResponsiveInputField(_('State'), 'inetOrgPerson_st', null, 'stList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')) {
			$profileElements[] = new htmlResponsiveInputField(_('Postal address'), 'inetOrgPerson_postalAddress', null, 'postalAddress');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$profileElements[] = new htmlResponsiveInputField(_('Registered address'), 'inetOrgPerson_registeredAddress', null, 'registeredAddress');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			$profileElements[] = new htmlResponsiveInputField(_('Office name'), 'inetOrgPerson_physicalDeliveryOfficeName', null, 'physicalDeliveryOfficeName');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			$profileElements[] = new htmlResponsiveInputField(_('Room number'), 'inetOrgPerson_roomNumber', null, 'roomNumber');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			$profileElements[] = new htmlResponsiveInputField(_('Telephone number'), 'inetOrgPerson_telephoneNumber', null, 'telephoneNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			$profileElements[] = new htmlResponsiveInputField(_('Fax number'), 'inetOrgPerson_facsimileTelephoneNumber', null, 'facsimileTelephoneNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			$profileElements[] = new htmlResponsiveInputField(_('Email address'), 'inetOrgPerson_mail', null, 'mailList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$profileElements[] = new htmlResponsiveInputField(_('Web site'), 'inetOrgPerson_labeledURI', null, 'labeledURIList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$profileElements[] = new htmlResponsiveInputField(_('Department'), 'inetOrgPerson_departmentNumber', null, 'departmentNumberList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$profileElements[] = new htmlResponsiveInputField(_('Organisational unit'), 'inetOrgPerson_ou', null, 'ouList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$profileElements[] = new htmlResponsiveInputField(_('Organisation'), 'inetOrgPerson_o', null, 'oList');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$profileElements[] = new htmlResponsiveInputField(_('Job title'), 'inetOrgPerson_title', null, 'title');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$profileElements[] = new htmlResponsiveInputField(_('Employee type'), 'inetOrgPerson_employeeType', null, 'employeeType');
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
		    $profileElements[] = new htmlResponsiveInputField(_('Business category'), 'inetOrgPerson_businessCategory', null, 'businessCategory');
		}
		if (sizeof($profileElements) > 0) {
			$profileContainer = new htmlResponsiveRow();
			for ($i = 0; $i < sizeof($profileElements); $i++) {
				$profileContainer->add($profileElements[$i], 12);
			}
			return $profileContainer;
		}
		return null;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		if (!$this->isUnixActive() && !empty($profile['inetOrgPerson_uid'][0])) {
			$this->attributes['uid'][0] = $profile['inetOrgPerson_uid'][0];
		}
		if (!$this->isUnixActive() && !empty($profile['inetOrgPerson_cn'][0])) {
			$this->attributes['cn'][0] = $profile['inetOrgPerson_cn'][0];
		}
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
	* {@inheritDoc}
	*/
	function check_profileOptions($options, $typeId) {
		$messages = parent::check_profileOptions($options, $typeId);
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
					$messages[] = $this->messages['facsimileTelephoneNumber'][0];
					break;
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	function get_uploadColumns($selectedModules, &$type) {
		$return = parent::get_uploadColumns($selectedModules, $type);
		// cn and uid for upload (only if posixAccount is not selected)
		if (!$this->isUnixActive($selectedModules)) {
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
		if (!$this->isSamba3Active() && !$this->isBooleanConfigOptionSet('inetOrgPerson_hidedisplayName', true)) {
			$return[] = array(
				'name' => 'inetOrgPerson_displayName',
				'description' => _('Display name'),
				'help' => 'displayName',
				'example' => _('Steve Miller'),
			);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = [];
		// get list of existing users
		$existingUsers = searchLDAPByAttribute('uid', '*', 'inetOrgPerson', array('uid'), array('user'));
		for ($e = 0; $e < sizeof($existingUsers); $e++) {
			$existingUsers[$e] = $existingUsers[$e]['uid'][0];
		}
		$existingMails = searchLDAPByAttribute('mail', '*', 'inetOrgPerson', array('mail'), array('user'));
		for ($e = 0; $e < sizeof($existingMails); $e++) {
			$existingMails[$e] = $existingMails[$e]['mail'][0];
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("inetOrgPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetOrgPerson";
			// last name
			if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_lastName']], 'realname')) {
				$partialAccounts[$i]['sn'] = trim($rawAccounts[$i][$ids['inetOrgPerson_lastName']]);
			}
			else {
				$errMsg = $this->messages['lastname'][1];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// first name
			if ($rawAccounts[$i][$ids['inetOrgPerson_firstName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_firstName']], 'realname')) {
					$partialAccounts[$i]['givenName'] = trim($rawAccounts[$i][$ids['inetOrgPerson_firstName']]);
				}
				else {
					$errMsg = $this->messages['givenName'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			if (!$this->isUnixActive($selectedModules)) {
				// uid
				if (isset($ids['inetOrgPerson_userName']) && !empty($rawAccounts[$i][$ids['inetOrgPerson_userName']])) {
					if (in_array($rawAccounts[$i][$ids['inetOrgPerson_userName']], $existingUsers)) {
						$errMsg = $this->messages['uid'][3];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
					if (get_preg($rawAccounts[$i][$ids['inetOrgPerson_userName']], 'username')) {
						$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['inetOrgPerson_userName']];
					}
					else {
						$errMsg = $this->messages['uid'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
			}
			// initials
			if (isset($ids['inetOrgPerson_initials']) && ($rawAccounts[$i][$ids['inetOrgPerson_initials']] != "")) {
				$partialAccounts[$i]['initials'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_initials']]);
			}
			// display name
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_displayName', 'displayName');
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_description', 'description');
			// title
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_title', 'title', 'title', $this->messages['title'][1], $errors, '/;[ ]*/');
			// employee number
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_employeeNumber', 'employeeNumber');
			// employee type
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_type', 'employeeType',
				'employeeType', $this->messages['employeeType'][1], $errors);
			// business category
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_businessCategory', 'businessCategory', 'businessCategory', $this->messages['businessCategory'][1], $errors, '/;[ ]*/');
			// manager
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_manager', 'manager', 'dn', $this->messages['manager'][0], $errors, '/;[ ]*/');
			// street
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_street', 'street', 'street', $this->messages['street'][1], $errors, '/;[ ]*/');
			// post office box
			if (isset($ids['inetOrgPerson_postOfficeBox']) && ($rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']] != "")) {
				$partialAccounts[$i]['postOfficeBox'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_postOfficeBox']]);
			}
			// room number
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_roomNumber', 'roomNumber');
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
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_carLicense', 'carLicense');
			// postal code
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_postalCode', 'postalCode', 'postalCode', $this->messages['postalCode'][1], $errors, '/;[ ]*/');
			// postal address
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_address', 'postalAddress',
				'postalAddress',$this->messages['postalAddress'][1] , $errors);
			// registered address
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_registeredAddress', 'registeredAddress',
				'postalAddress',$this->messages['registeredAddress'][1] , $errors);
			// telephone
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_telephone', 'telephoneNumber', 'telephone', $this->messages['telephoneNumber'][1], $errors, '/;[ ]*/');
			// home telephone
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_homePhone', 'homePhone', 'telephone', $this->messages['homePhone'][1], $errors, '/;[ ]*/');
			// mobile
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_mobile', 'mobile', 'telephone', $this->messages['mobileTelephone'][1], $errors, '/;[ ]*/');
			// facsimile
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_fax', 'facsimileTelephoneNumber', 'telephone', $this->messages['facsimileNumber'][1], $errors, '/;[ ]*/');
			// pager
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetOrgPerson_pager', 'pager', 'telephone', $this->messages['pager'][1], $errors, '/;[ ]*/');
			// eMail
			if (isset($ids['inetOrgPerson_email']) && ($rawAccounts[$i][$ids['inetOrgPerson_email']] != "")) {
				$mailList = preg_split('/;[ ]*/', trim($rawAccounts[$i][$ids['inetOrgPerson_email']]));
				$partialAccounts[$i]['mail'] = $mailList;
				for ($x = 0; $x < sizeof($mailList); $x++) {
					if (!get_preg($mailList[$x], 'email')) {
						$errMsg = $this->messages['email'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
					elseif (in_array($mailList[$x], $existingMails)) {
						$errMsg = $this->messages['mail'][2];
						array_push($errMsg, array($i, $mailList[$x]));
						$errors[] = $errMsg;
					}
				}
			}
			if (isLAMProVersion() && isset($ids['inetOrgPerson_sendPasswordMail']) && ($rawAccounts[$i][$ids['inetOrgPerson_sendPasswordMail']] != "")) {
				if (!in_array($rawAccounts[$i][$ids['inetOrgPerson_sendPasswordMail']], array('true', 'false'))) {
					$errMsg = $this->messages['sendPasswordMail'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// labeledURI
			if (isset($ids['inetOrgPerson_labeledURI']) && ($rawAccounts[$i][$ids['inetOrgPerson_labeledURI']] != "")) {
				$partialAccounts[$i]['labeledURI'] = preg_split('/;[ ]*/', trim($rawAccounts[$i][$ids['inetOrgPerson_labeledURI']]));
			}
			if (!$this->isUnixActive($selectedModules)) {
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
				// password
				// delay exop passwords
				if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
					// changed in post action
				}
				// set SASL password
				elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')) {
					$partialAccounts[$i]['userpassword'] = '{SASL}' . $partialAccounts[$i]['uid'];
				}
				// set K5KEY password
				elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
					$partialAccounts[$i]['userpassword'] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				// set normal password
				else {
					if (($rawAccounts[$i][$ids['inetOrgPerson_userPassword']] != "") && (get_preg($rawAccounts[$i][$ids['inetOrgPerson_userPassword']], 'password'))) {
						$partialAccounts[$i]['userpassword'] = pwd_hash($rawAccounts[$i][$ids['inetOrgPerson_userPassword']], true, $this->moduleSettings['posixAccount_pwdHash'][0]);
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
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::doUploadPostActions()
	 */
	function doUploadPostActions(&$data, $ids, $failed, &$temp, &$accounts, $selectedModules, $type) {
		if (!checkIfWriteAccessIsAllowed($this->get_scope())) {
			die();
		}
		if (!isset($temp['counter'])) {
			$temp['counter'] = 0;
		}
		$errors = [];
		$dataSize = sizeof($data);
		if (($temp['counter'] < $dataSize) && !in_array($temp['counter'], $failed)) {
			// mail sending is LAM Pro only
			if (isLAMProVersion()
				&& isset($ids['inetOrgPerson_email'])
				&& ($data[$temp['counter']][$ids['inetOrgPerson_email']] != "")) {
				if (isset($ids['inetOrgPerson_sendPasswordMail']) && ($data[$temp['counter']][$ids['inetOrgPerson_sendPasswordMail']] == "true")
						&& isset($accounts[$temp['counter']]['INFO.userPasswordClearText'])) {
					$mailMessages = sendPasswordMail($accounts[$temp['counter']]['INFO.userPasswordClearText'], $accounts[$temp['counter']]);
					for ($i = 0; $i < sizeof($mailMessages); $i++) {
						if ($mailMessages[$i][0] == 'ERROR') {
							$errors[] = $mailMessages[$i];
						}
					}
				}
			}
			// add addressbook entry
			if ($this->isBooleanConfigOptionSet('inetOrgPerson_addAddressbook')) {
				$attrs = array(
					'objectClass' => array('organizationalUnit'),
					'ou' => 'addressbook'
				);
				$dn = 'ou=addressbook,' . $accounts[$temp['counter']]['dn'];
				$success = @ldap_add($_SESSION['ldap']->server(), $dn, $attrs);
				if (!$success) {
					logNewMessage(LOG_ERR, 'Unable to add addressbook for user ' . $accounts[$temp['counter']]['dn'] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_("Was unable to create DN: %s."), htmlspecialchars($dn)), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, 'Added addressbook for user ' . $accounts[$temp['counter']]['dn']);
				}
			}
			// set password via exop
			if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
				if (isset($ids['inetOrgPerson_userPassword']) && !empty($data[$temp['counter']][$ids['inetOrgPerson_userPassword']])) {
					$dn = $accounts[$temp['counter']]['dn'];
					$password = $data[$temp['counter']][$ids['inetOrgPerson_userPassword']];
					$success = ldap_exop_passwd($_SESSION['ldap']->server(), $dn, null, $password);
					if (!$success) {
						$errors[] = array(
							"ERROR",
							_('Unable to set password'),
							$dn . '<br>' . getDefaultLDAPErrorString($_SESSION['ldap']->server()),
							array($temp['groups'][$temp['counter']])
						);
					}
				}
			}
		}
		$temp['counter']++;
		if ($temp['counter'] < $dataSize) {
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / $dataSize,
				'errors' => $errors
			);
		}
		else {
			return array (
				'status' => 'finished',
				'progress' => 100,
				'errors' => $errors
			);
		}
	}



	/**
	 * Returns a list of self service configuration settings.
	 *
	 * @param selfServiceProfile $profile currently edited profile
	 * @return htmlElement meta HTML object
	 */
	public function getSelfServiceSettings($profile) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlSubTitle(_('Photo')), 12);
		$container->add(new htmlResponsiveInputField(_('Maximum width (px)'), 'inetOrgPerson_jpegPhoto_maxWidth', null, array('crop', get_class($this))), 12);
		$container->add(new htmlResponsiveInputField(_('Maximum height (px)'), 'inetOrgPerson_jpegPhoto_maxHeight', null, array('crop', get_class($this))), 12);
		$container->add(new htmlResponsiveInputField(_('Maximum file size (kB)'), 'inetOrgPerson_jpegPhoto_maxSize'), 12);
		return $container;
	}

	/**
	 * Checks if the self service settings are valid.
	 *
	 * If the input data is invalid the return value is an array that contains arrays
	 * to build StatusMessages (message type, message head, message text). If no errors
	 * occurred the function returns an empty array.
	 *
	 * @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @param selfServiceProfile $profile self service profile
	 * @return array error messages
	 */
	public function checkSelfServiceSettings(&$options, &$profile) {
		$errors = [];
		if (!empty($options['inetOrgPerson_jpegPhoto_maxWidth'][0]) && !is_numeric($options['inetOrgPerson_jpegPhoto_maxWidth'][0])) {
			$errors[] = array('ERROR', _('Please enter a number.'), _('Maximum width (px)'));
		}
		if (!empty($options['inetOrgPerson_jpegPhoto_maxHeight'][0]) && !is_numeric($options['inetOrgPerson_jpegPhoto_maxHeight'][0])) {
			$errors[] = array('ERROR', _('Please enter a number.'), _('Maximum height (px)'));
		}
		if (!empty($options['inetOrgPerson_jpegPhoto_maxSize'][0]) && !is_numeric($options['inetOrgPerson_jpegPhoto_maxSize'][0])) {
			$errors[] = array('ERROR', _('Please enter a number.'), _('Maximum file size (kB)'));
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
	 * @return array list of meta HTML elements (field name => htmlResponsiveRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		$this->addSimpleSelfServiceTextField($return, 'firstName', _('First name'), $fields,
			$attributes, $readOnlyFields, false, false, 'givenName');
		$this->addSimpleSelfServiceTextField($return, 'lastName', _('Last name'), $fields,
			$attributes, $readOnlyFields, true, false, 'sn');
		$this->addSimpleSelfServiceTextField($return, 'mail', _('Email address'), $fields,
			$attributes, $readOnlyFields);
		$this->addMultiValueSelfServiceTextField($return, 'labeledURI', _('Web site'), $fields,
			$attributes, $readOnlyFields, false, false, 'labeledURI');
		$this->addSimpleSelfServiceTextField($return, 'telephoneNumber', _('Telephone number'), $fields,
			$attributes, $readOnlyFields, false, false, 'telephoneNumber');
		$this->addSimpleSelfServiceTextField($return, 'homePhone', _('Home telephone number'), $fields,
			$attributes, $readOnlyFields, false, false, 'homePhone');
		$this->addSimpleSelfServiceTextField($return, 'mobile', _('Mobile telephone number'), $fields,
			$attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'faxNumber', _('Fax number'), $fields,
			$attributes, $readOnlyFields, false, false, 'facsimileTelephoneNumber');
		$this->addSimpleSelfServiceTextField($return, 'pager', _('Pager'), $fields,
			$attributes, $readOnlyFields);
		$this->addMultiValueSelfServiceTextField($return, 'street', _('Street'), $fields,
			$attributes, $readOnlyFields);
		$this->addMultiValueSelfServiceTextField($return, 'postalAddress', _('Postal address'), $fields,
			$attributes, $readOnlyFields, false, true, 'postalAddress');
		$this->addMultiValueSelfServiceTextField($return, 'registeredAddress', _('Registered address'), $fields,
			$attributes, $readOnlyFields, false, true, 'registeredAddress');
		$this->addMultiValueSelfServiceTextField($return, 'postalCode', _('Postal code'), $fields,
			$attributes, $readOnlyFields, false, false, 'postalCode');
		$this->addMultiValueSelfServiceTextField($return, 'postOfficeBox', _('Post office box'), $fields,
			$attributes, $readOnlyFields, false, false, 'postOfficeBox');
		$this->addSimpleSelfServiceTextField($return, 'roomNumber', _('Room number'), $fields,
			$attributes, $readOnlyFields, false, false, 'roomNumber');
		$this->addMultiValueSelfServiceTextField($return, 'location', _('Location'), $fields,
			$attributes, $readOnlyFields, false, false, 'l');
		$this->addMultiValueSelfServiceTextField($return, 'state', _('State'), $fields,
			$attributes, $readOnlyFields, false, false, 'st');
		$this->addSimpleSelfServiceTextField($return, 'carLicense', _('Car license'), $fields,
			$attributes, $readOnlyFields, false, false, 'carLicense');
		$this->addMultiValueSelfServiceTextField($return, 'officeName', _('Office name'), $fields,
			$attributes, $readOnlyFields, false, false, 'physicalDeliveryOfficeName');
		$this->addMultiValueSelfServiceTextField($return, 'businessCategory', _('Business category'), $fields,
			$attributes, $readOnlyFields, false, false, 'businessCategory');

		if (in_array('jpegPhoto', $fields)) {
			$_SESSION[self::SESS_PHOTO] = null;
			if (isset($attributes['jpegPhoto'][0])) {
				$_SESSION[self::SESS_PHOTO] = $attributes['jpegPhoto'][0];
			}
			$readOnlyPhoto = in_array('jpegPhoto', $readOnlyFields);
			if (!empty($attributes['jpegPhoto'][0]) || !$readOnlyPhoto) {
				$photoSub = new htmlDiv('inetOrgPersonPhotoUploadContent', $this->getSelfServicePhoto($readOnlyPhoto, false));
				$photoRow = new htmlResponsiveRow();
				$photoRow->add($this->getSelfServicePhotoJS($readOnlyPhoto), 0);
				$photoRow->addLabel(new htmlOutputText($this->getSelfServiceLabel('jpegPhoto', _('Photo'))));
				$photoRow->addField(new htmlDiv('jpegPhotoDiv', $photoSub));
				$return['jpegPhoto'] = $photoRow;
			}
		}

		$this->addMultiValueSelfServiceTextField($return, 'departmentNumber', _('Department'), $fields,
			$attributes, $readOnlyFields, false, false, 'departmentNumber');
		$this->addSimpleSelfServiceTextField($return, 'initials', _('Initials'), $fields,
			$attributes, $readOnlyFields);
		$this->addMultiValueSelfServiceTextField($return, 'title', _('Job title'), $fields,
			$attributes, $readOnlyFields);

		if (in_array('userCertificate', $fields)) {
			$userCertificates = [];
			if (isset($attributes['userCertificate'][0])) {
				$userCertificates = $attributes['userCertificate'];
			}
			elseif (isset($attributes['userCertificate;binary'][0])) {
				$userCertificates = $attributes['userCertificate;binary'];
			}
			$_SESSION[self::SESS_CERTIFICATES_LIST] = $userCertificates;
			$certTable = new htmlResponsiveRow();
			$certTable->add(new htmlDiv('userCertificateDiv', $this->getSelfServiceUserCertificates()));
			// JavaScript functions
			$certTable->add($this->getSelfServiceUserCertificatesJSBlock(), 12);
			// upload button
			$uploadButtonGroup = new htmlGroup();
			$uploadLabel = new htmlLabel('inetOrgPersonCertificate_file', _('Upload a file'));
			$uploadButtonGroup->addElement($uploadLabel);
			$uploadInput = new htmlInputFileUpload('inetOrgPersonCertificate_file');
			$uploadInput->setOnChange('inetOrgPersonUploadCert();');
			$uploadButtonGroup->addElement($uploadInput);
			$certLabel = new htmlOutputText($this->getSelfServiceLabel('userCertificate', _('User certificates')));
			$certTable->add(new htmlDiv('inetOrgPersonCertUploadId', $uploadButtonGroup, array('lam-upload-button')));
			$return['userCertificate'] = new htmlResponsiveRow($certLabel, $certTable);
		}
		// o
		if (in_array('o', $fields)) {
			$o = '';
			if (isset($attributes['o'][0])) {
				$o = $attributes['o'][0];
			}
			if (in_array('o', $readOnlyFields)) {
				$oField = new htmlOutputText(getAbstractDN($o));
			}
			else {
				$filter = '(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container))';
				$suffix = $_SESSION['selfServiceProfile']->LDAPSuffix;
				$foundOs = searchLDAPPaged($_SESSION['ldapHandle']->getServer(), $suffix, $filter, array('dn'), false, 0);
				$oList = [];
				foreach ($foundOs as $foundO) {
					$oList[] = $foundO['dn'];
				}
				if (!empty($attributes['o'][0]) && !in_array($attributes['o'][0], $oList)) {
					$oList[] = $attributes['o'][0];
					usort($oList, 'compareDN');
				}
				$oSelectionList = array('' => '');
				foreach ($oList as $singleOU) {
					$oSelectionList[getAbstractDN($singleOU)] = $singleOU;
				}
				$oSelectionListSelected = [];
				if (!empty($attributes['o'][0])) {
					$oSelectionListSelected[] = $attributes['o'][0];
				}
				$oField = new htmlSelect('inetOrgPerson_o', $oSelectionList, $oSelectionListSelected);
				$oField->setHasDescriptiveElements(true);
				$oField->setRightToLeftTextDirection(true);
				$oField->setSortElements(false);
			}
			$return['o'] = new htmlResponsiveRow(
				new htmlLabel('inetOrgPerson_o', $this->getSelfServiceLabel('o', _('Organisation'))), $oField
			);
		}
		// ou
		if (in_array('ou', $fields)) {
			$ou = '';
			if (isset($attributes['ou'][0])) {
				$ou = $attributes['ou'][0];
			}
			if (in_array('ou', $readOnlyFields)) {
				$ouField = new htmlOutputText(getAbstractDN($ou));
			}
			else {
				$filter = '(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container))';
				$suffix = $_SESSION['selfServiceProfile']->LDAPSuffix;
				$foundOus = searchLDAPPaged($_SESSION['ldapHandle']->getServer(), $suffix, $filter, array('dn'), false, 0);
				$ouList = [];
				foreach ($foundOus as $foundOu) {
					$ouList[] = $foundOu['dn'];
				}
				if (!empty($attributes['ou'][0]) && !in_array($attributes['ou'][0], $ouList)) {
					$ouList[] = $attributes['ou'][0];
					usort($ouList, 'compareDN');
				}
				$ouSelectionList = array('' => '');
				foreach ($ouList as $singleOU) {
					$ouSelectionList[getAbstractDN($singleOU)] = $singleOU;
				}
				$ouSelectionListSelected = [];
				if (!empty($attributes['ou'][0])) {
					$ouSelectionListSelected[] = $attributes['ou'][0];
				}
				$ouField = new htmlSelect('inetOrgPerson_ou', $ouSelectionList, $ouSelectionListSelected);
				$ouField->setHasDescriptiveElements(true);
				$ouField->setRightToLeftTextDirection(true);
				$ouField->setSortElements(false);
			}
			$return['ou'] = new htmlResponsiveRow(
				new htmlLabel('inetOrgPerson_ou', $this->getSelfServiceLabel('ou', _('Organisational unit'))), $ouField
			);
		}

		$this->addMultiValueSelfServiceTextField($return, 'description', _('Description'), $fields,
			$attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'uid', _('User name'), $fields,
			$attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'displayName', _('Display name'), $fields,
			$attributes, $readOnlyFields, false, false, 'displayName');
		return $return;
	}

	/**
	 * Renders the photo area for self service.
	 *
	 * @param boolean $readOnly content is read-only
	 * @param boolean $crop enable cropping
	 * @return htmlResponsiveRow content
	 * @throws LAMException error displaying page
	 */
	private function getSelfServicePhoto($readOnly, $crop) {
		$photo = $_SESSION[self::SESS_PHOTO];
		$row = new htmlResponsiveRow();
		if (!empty($photo)) {
			$tempFilesManager = new LamTemporaryFilesManager();
			$fileName = $tempFilesManager->registerTemporaryFile('.jpg');
			$handle = $tempFilesManager->openTemporaryFileForWrite($fileName);
			fwrite($handle, $photo);
			fclose ($handle);
			$photoFile = $tempFilesManager->getResourceLink($fileName);
			$img = new htmlImage($photoFile, null, null, $this->getSelfServiceLabel('jpegPhoto', _('Photo')));
			$img->setCSSClasses(array('photo'));
			if ($crop) {
				$img->enableCropping();
			}
			$row->add($img);
			if (!$readOnly) {
				$row->addVerticalSpacer('0.5rem');
				$deleteButton = new htmlLink(_('Delete'), '#', '../../graphics/del.svg', true);
				$deleteButton->setOnClick('inetOrgPersonDeletePhoto(); return false;');
				$row->add($deleteButton);
			}
			$row->addVerticalSpacer('0.5rem');
		}
		// upload button
		$uploadButtonGroup = new htmlGroup();
		$uploadLabel = new htmlLabel('inetOrgPersonPhoto_file', _('Upload a file'));
		$uploadButtonGroup->addElement($uploadLabel);
		$uploadInput = new htmlInputFileUpload('inetOrgPersonPhoto_file');
		$uploadInput->setOnChange('inetOrgPersonUploadPhoto();');
		$uploadButtonGroup->addElement($uploadInput);
		$row->add(new htmlDiv('inetOrgPersonPhotoUploadId', $uploadButtonGroup, array('lam-upload-button')));
		// webcam button
		$webcamContent = new htmlResponsiveRow();
		$webcamContent->addVerticalSpacer('0.5rem');
		$errorMessage = new htmlStatusMessage('ERROR', '');
		$errorMessage->setCSSClasses(array('hidden', 'lam-webcam-message'));
		$webcamContent->add($errorMessage);
		$webcamContent->addVerticalSpacer('0.5rem');
		$captureButton = new htmlLink(_('Use webcam'), '#', '../../graphics/webcam.png', true);
		$captureButton->setId('btn_lam-webcam-capture');
		$captureButton->setOnClick('window.lam.tools.webcam.capture(event);');
		$webcamContent->add($captureButton, 12, 12, 12);
		$video = new htmlVideo('lam-webcam-video');
		$video->setCSSClasses(array('hidden'));
		$webcamContent->add($video, 12, 12, 12, 'text-center');
		$webcamContent->addVerticalSpacer('1rem');
		$webcamUploadButton = new htmlLink(_('Upload'), '#', '../../graphics/upload.svg', true);
		$webcamUploadButton->setId('btn-lam-webcam-upload');
		$webcamUploadButton->setCSSClasses(array('btn-lam-webcam-upload', 'hidden'));
		$webcamUploadButton->setOnClick('window.lam.tools.webcam.uploadSelfService(event, "' . getSecurityTokenName()
			.  '", "' . getSecurityTokenValue() . '", "inetOrgPerson", "user", "' . _('File upload failed!') . '", "inetOrgPersonPhotoUploadContent");');
		$webcamContent->add($webcamUploadButton, 12, 12, 12);
		$canvas = new htmlCanvas('lam-webcam-canvas');
		$canvas->setCSSClasses(array('hidden'));
		$webcamContent->add($canvas, 12);
		$webcamDiv = new htmlDiv('lam_webcam_div', $webcamContent, array('hidden'));
		$webcamContent->addVerticalSpacer('1rem');
		$row->add($webcamDiv);
		return $row;
	}

	/**
	 * Returns the Java Script functions to manage the photo.
	 *
	 * @param boolean $readOnly content is read-only
	 * @return htmlJavaScript JS block
	 */
	private static function getSelfServicePhotoJS($readOnly) {
		if ($readOnly) {
			return new htmlGroup();
		}
		$content = '
				function inetOrgPersonUploadPhoto() {
				let params = new FormData();
				params.append("action", "ajaxPhotoUpload");
				params.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				let reader = new FileReader();
				reader.onload = function () {
					const content = reader.result;
					params.append("file", btoa(content));
					fetch("../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user", {
						method: "POST",
						body: params
					})
					.then(async response => {
						const data = await response.json();
						if (data.success) {
							if (data.html) {
								document.getElementById(\'inetOrgPersonPhotoUploadContent\').innerHTML = data.html;
								window.lam.tools.webcam.init();
								window.lam.html.initCropping();
							}
						}
						else if (data.error) {
							window.lam.dialog.showInfo(data.error, "' . _('Ok') . '");
						}
						else if (data.errormessage) {
							window.lam.dialog.showInfo(data.errormessage, "' . _('Ok') . '");
						}
					});
				};
				const fileInput = document.getElementById("inetOrgPersonPhoto_file");
				reader.readAsBinaryString(fileInput.files[0]);
			}

			function inetOrgPersonDeletePhoto(id) {
				var actionJSON = {
					"action": "deletePhoto",
					"id": id
				};
				let data = new FormData();
				data.append("jsonInput", JSON.stringify(actionJSON));
				data.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				fetch("../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user", {
					method: "POST",
					body: data
				})
				.then(async response => {
					const data = await response.json();
					inetOrgPersonDeletePhotoHandleReply(data);
				});
			}

			function inetOrgPersonDeletePhotoHandleReply(data) {
				if (data.errorsOccurred == "false") {
					document.getElementById(\'inetOrgPersonPhotoUploadContent\').innerHTML = data.html;
					window.lam.tools.webcam.init();
				}
				else {
					window.lam.dialog.showInfo(data.errormessage, "' . _('Ok') . '");
				}
			}

				';
		return new htmlJavaScript($content);
	}


	/**
	 * Returns the meta HTML code to display the certificate area.
	 * This also includes the file upload.
	 *
	 * @return htmlTable certificate content
	 * @throws LAMException
	 */
	private function getSelfServiceUserCertificates() {
		$userCertificates = $_SESSION[self::SESS_CERTIFICATES_LIST];
		$content = new htmlGroup();
		if (sizeof($userCertificates) > 0) {
			$certTable = new htmlResponsiveRow();
			$tempFilesManager = new LamTemporaryFilesManager();
			for ($i = 0; $i < sizeof($userCertificates); $i++) {
				$group = new htmlGroup();
				$filename = $tempFilesManager->registerTemporaryFile('.der', 'userCertificate_');
				$out = $tempFilesManager->openTemporaryFileForWrite($filename);
				fwrite($out, $userCertificates[$i]);
				fclose ($out);
				$pem = @chunk_split(@base64_encode($userCertificates[$i]), 64, "\n");
				if (!empty($pem)) {
					$pem = "-----BEGIN CERTIFICATE-----\n" . $pem . "-----END CERTIFICATE-----\n";
					$pemData = @openssl_x509_parse($pem);
					$data = [];
					if (isset($pemData['serialNumber'])) {
						$data[] = $pemData['serialNumber'];
					}
					if (isset($pemData['name'])) {
						$data[] = $pemData['name'];
					}
					if (sizeof($data) > 0) {
						$group->addElement(new htmlOutputText(implode(': ', $data)));
						$group->addElement(new htmlSpacer('5px', null));
					}
				}
				$saveLink = new htmlLink('', $tempFilesManager->getDownloadLink($filename), '../../graphics/save.svg');
				$saveLink->setTitle(_('Save'));
				$saveLink->setTargetWindow('_blank');
				$saveLink->setCSSClasses(array('icon'));
				$group->addElement($saveLink);
				$delLink = new htmlLink('', '#', '../../graphics/del.svg');
				$delLink->setTitle(_('Delete'));
				$delLink->setOnClick('inetOrgPersonDeleteCertificate(' . $i . '); return false;');
				$delLink->setCSSClasses(array('icon'));
				$group->addElement($delLink);
				$certTable->add($group, 12);
			}
			$content->addElement($certTable);
		}
		return $content;
	}

	/**
	 * Returns the Java Script functions to manage the certificates.
	 *
	 * @return htmlJavaScript JS block
	 */
	private static function getSelfServiceUserCertificatesJSBlock() {
		$content = '
			function inetOrgPersonDeleteCertificate(id) {
				var actionJSON = {
					"action": "deleteCert",
					"id": id
				};
				let data = new FormData();
				data.append("jsonInput", JSON.stringify(actionJSON));
				data.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				fetch(\'../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user\', {
					method: "POST",
					body: data
				})
				.then(async response => {
					const jsonData = await response.json();
					inetOrgPersonDeleteCertificateHandleReply(jsonData);
				});
			}

			function inetOrgPersonDeleteCertificateHandleReply(data) {
				if (data.errorsOccurred == "false") {
					document.getElementById(\'userCertificateDiv\').innerHTML = data.html;
				}
				else {
					alert(data.errormessage);
				}
			}

			function inetOrgPersonUploadCert() {
				let data = new FormData();
				data.append("action", \'ajaxCertUpload\');
				data.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				let reader = new FileReader();
				reader.onload = function () {
					const content = reader.result;
					data.append("file", btoa(content));
					fetch(\'../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user\', {
						method: "POST",
						body: data
					})
					.then(async response => {
						const jsonData = await response.json();
						if (jsonData.success) {
							if (jsonData.html) {
								document.getElementById(\'userCertificateDiv\').innerHTML = jsonData.html;
							}
						}
						else if (jsonData.error) {
							window.lam.dialog.showInfo(jsonData.error, "' . _('Ok') . '");
						}
						else if (jsonData.errormessage) {
							window.lam.dialog.showInfo(jsonData.errormessage, "' . _('Ok') . '");
						}
					});
				};
				const fileInput = document.getElementById("inetOrgPersonCertificate_file");
				reader.readAsBinaryString(fileInput.files[0]);
			}

			';
		return new htmlJavaScript($content);
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
	 * @return array messages and attributes (array('messages' => [], 'add' => array('mail' => array('test@test.com')), 'del' => [], 'mod' => [], 'info' => []))
	 */
	function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array('messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []);
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		$attributeNames = []; // list of attributes which should be checked for modification
		$attributesNew = $attributes;
		$this->checkSimpleSelfServiceTextField($return, 'firstName', $attributes, $fields,
			$readOnlyFields, 'realname', $this->messages['givenName'][0], null,
			'givenName');
		$this->checkSimpleSelfServiceTextField($return, 'lastName', $attributes, $fields,
			$readOnlyFields, 'realname', $this->messages['lastname'][0], $this->messages['lastname'][0],
			'sn');
		$this->checkSimpleSelfServiceTextField($return, 'mail', $attributes, $fields,
			$readOnlyFields, 'email', $this->messages['mail'][0]);
		$this->checkMultiValueSelfServiceTextField($return, 'labeledURI', $attributes, $fields,
			$readOnlyFields, null, null, null, 'labeledURI');
		$this->checkSimpleSelfServiceTextField($return, 'telephoneNumber', $attributes, $fields,
			$readOnlyFields, 'telephone', $this->messages['telephoneNumber'][0], null,
			'telephoneNumber');
		$this->checkSimpleSelfServiceTextField($return, 'homePhone', $attributes, $fields,
			$readOnlyFields, 'telephone', $this->messages['homePhone'][0], null,
			'homePhone');
		$this->checkSimpleSelfServiceTextField($return, 'faxNumber', $attributes, $fields,
			$readOnlyFields, 'telephone', $this->messages['facsimileTelephoneNumber'][0], null,
			'facsimileTelephoneNumber');
		$this->checkSimpleSelfServiceTextField($return, 'mobile', $attributes, $fields,
			$readOnlyFields, 'telephone', $this->messages['mobile'][0]);
		$this->checkSimpleSelfServiceTextField($return, 'pager', $attributes, $fields,
			$readOnlyFields, 'telephone', $this->messages['pager'][0]);
		$this->checkMultiValueSelfServiceTextField($return, 'street', $attributes, $fields,
			$readOnlyFields, 'street', $this->messages['street'][0]);
		$this->checkMultiValueSelfServiceTextField($return, 'postalAddress', $attributes, $fields,
			$readOnlyFields, 'postalAddress', $this->messages['postalAddress'][0], null,
			'postalAddress');
		$this->checkMultiValueSelfServiceTextField($return, 'registeredAddress', $attributes, $fields,
			$readOnlyFields, 'postalAddress', $this->messages['registeredAddress'][0], null,
			'registeredAddress');
		$this->checkMultiValueSelfServiceTextField($return, 'postalCode', $attributes, $fields,
			$readOnlyFields, 'postalCode', $this->messages['postalCode'][0], null,
			'postalCode');
		$this->checkMultiValueSelfServiceTextField($return, 'postOfficeBox', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'postOfficeBox');
		$this->checkSimpleSelfServiceTextField($return, 'roomNumber', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'roomNumber');
		$this->checkMultiValueSelfServiceTextField($return, 'location', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'l');
		$this->checkMultiValueSelfServiceTextField($return, 'state', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'st');
		$this->checkSimpleSelfServiceTextField($return, 'carLicense', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'carLicense');
		$this->checkMultiValueSelfServiceTextField($return, 'officeName', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'physicalDeliveryOfficeName');
		$this->checkMultiValueSelfServiceTextField($return, 'businessCategory', $attributes, $fields,
			$readOnlyFields, 'businessCategory', $this->messages['businessCategory'][0], null,
			'businessCategory');

		// photo
		if (in_array('jpegPhoto', $fields) && !in_array('jpegPhoto', $readOnlyFields)) {
			$data = $_SESSION[self::SESS_PHOTO];
			// remove photo
			if (!empty($attributes['jpegPhoto'][0]) && empty($data)) {
				$return['mod']['jpegPhoto'] = [];
			}
			// set/replace photo
			elseif (!empty($data) && (empty($attributes['jpegPhoto'][0]) || ($data != $attributes['jpegPhoto'][0]))) {
				$moduleSettings = $this->selfServiceSettings->moduleSettings;
				try {
					include_once __DIR__ . '/../imageutils.inc';
					$imageManipulator = ImageManipulationFactory::getImageManipulator($data);
					$imageManipulator->crop($_POST['croppingDataX'], $_POST['croppingDataY'], $_POST['croppingDataWidth'], $_POST['croppingDataHeight']);
					$data = $imageManipulator->getImageData();
					$data = inetOrgPerson::resizeAndConvertImage($data, $moduleSettings);
					if (!empty($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]) && ($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0] < (strlen($data) / 1024))) {
						$msg =  $this->messages['file'][3];
						$msg[] = null;
						$msg[] = htmlspecialchars($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]);
						$return['messages'][] = $msg;
					}
					else {
						if (!empty($attributes['jpegPhoto'][0])) {
							$return['mod']['jpegPhoto'][0] = $data;
						}
						else {
							$return['add']['jpegPhoto'][0] = $data;
						}
					}
				}
				catch (Exception $e) {
					$msg =  $this->messages['file'][2];
					$msg[] = htmlspecialchars($e->getMessage());
					$return['messages'][] = $msg;
				}
			}
		}

		$this->checkMultiValueSelfServiceTextField($return, 'departmentNumber', $attributes, $fields,
			$readOnlyFields, null, null, null,
			'departmentNumber');
		$this->checkSimpleSelfServiceTextField($return, 'initials', $attributes, $fields,
			$readOnlyFields);
		$this->checkMultiValueSelfServiceTextField($return, 'title', $attributes, $fields,
			$readOnlyFields, 'title', $this->messages['title'][0]);

		// user certificates
		if (in_array('userCertificate', $fields)) {
			$userCertificates = $_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST];
			$userCertificatesAttrName = 'userCertificate;binary';
			if (isset($attributes['userCertificate'])) {
				$userCertificatesAttrName = 'userCertificate';
			}
			$attributeNames[] = $userCertificatesAttrName;
			if (sizeof($userCertificates) > 0) {
				$attributesNew[$userCertificatesAttrName] = $userCertificates;
			}
			elseif (isset($attributesNew[$userCertificatesAttrName])) {
				unset($attributesNew[$userCertificatesAttrName]);
			}
		}
		// ou
		if (in_array('ou', $fields) && !in_array('ou', $readOnlyFields)) {
			$attributeNames[] = 'ou';
			if (!empty($_POST['inetOrgPerson_ou'])) {
				$attributesNew['ou'][0] = $_POST['inetOrgPerson_ou'];
			}
			elseif (isset($attributes['ou'])) {
				unset($attributesNew['ou']);
			}
		}
		// o
		if (in_array('o', $fields) && !in_array('o', $readOnlyFields)) {
			$attributeNames[] = 'o';
			if (!empty($_POST['inetOrgPerson_o'])) {
				$attributesNew['o'][0] = $_POST['inetOrgPerson_o'];
			}
			elseif (isset($attributes['o'])) {
				unset($attributesNew['o']);
			}
		}

		$this->checkMultiValueSelfServiceTextField($return, 'description', $attributes, $fields,
			$readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'uid', $attributes, $fields,
			$readOnlyFields, 'username', $this->messages['uid'][0]);
		$this->checkSimpleSelfServiceTextField($return, 'displayName', $attributes, $fields,
			$readOnlyFields, null, null, null, 'displayName');

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
	 * Resizes the given image data to the settings provided.
	 *
	 * @param array $data binary image data
	 * @param array $settings settings
	 * @return array binary image data
	 */
	private static function resizeAndConvertImage($data, $settings) {
		include_once __DIR__ . '/../imageutils.inc';
		$imageManipulator = ImageManipulationFactory::getImageManipulator($data);
		// resize if maximum values specified
		if (!empty($settings['inetOrgPerson_jpegPhoto_maxWidth'][0]) || !empty($settings['inetOrgPerson_jpegPhoto_maxHeight'][0])) {
			$maxWidth = empty($settings['inetOrgPerson_jpegPhoto_maxWidth'][0]) ? $imageManipulator->getWidth() : $settings['inetOrgPerson_jpegPhoto_maxWidth'][0];
			$maxHeight = empty($settings['inetOrgPerson_jpegPhoto_maxHeight'][0]) ? $imageManipulator->getHeight() : $settings['inetOrgPerson_jpegPhoto_maxHeight'][0];
			$imageManipulator->thumbnail($maxWidth, $maxHeight);
		}
		$imageManipulator->convertToJpeg();
		return $imageManipulator->getImageData();
	}

	/**
	 * Manages AJAX requests.
	 * This function may be called with or without an account container.
	 */
	public function handleAjaxRequest() {
		// AJAX uploads are non-JSON
		if (isset($_POST['action']) && ($_POST['action'] == 'ajaxCertUpload')) {
			$this->ajaxUploadCert();
			return;
		}
		if ((isset($_POST['action']) && ($_POST['action'] == 'ajaxPhotoUpload'))
			|| (isset($_GET['action']) && ($_GET['action'] == 'ajaxPhotoUpload'))) {
			$this->ajaxUploadPhoto();
			return;
		}
		$jsonInput = json_decode($_POST['jsonInput'], true);
		$jsonReturn = self::invalidAjaxRequest();
		if (isset($jsonInput['action'])) {
			if ($jsonInput['action'] == 'deleteCert') {
				$jsonReturn = $this->ajaxDeleteSelfServiceUserCertificate($jsonInput);
			}
			elseif ($jsonInput['action'] == 'deletePhoto') {
				$jsonReturn = $this->ajaxDeleteSelfServicePhoto();
			}
		}
		echo json_encode($jsonReturn);
	}

	/**
	 * Handles an AJAX certificate file upload and prints the JSON result.
	 */
	private function ajaxUploadCert() {
		$result = array('success' => true);
		if (!isset($_POST['file']) || (strlen($_POST['file']) < 100)) {
			$result = array('error' => _('No file received.'));
		}
		else {
			$data = base64_decode($_POST['file']);
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
			$_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST][] = $data;
			$_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST] = array_unique($_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST]);
			ob_start();
			$contentElement = $this->getSelfServiceUserCertificates();
			ob_end_clean();
			ob_start();
			parseHtml(null, $contentElement, [], true, $this->get_scope());
			$content = ob_get_contents();
			ob_end_clean();
			$result['html'] = $content;
		}
		echo json_encode($result);
	}

	/**
	 * Handles an AJAX photo file upload and prints the JSON result.
	 */
	private function ajaxUploadPhoto() {
		$result = array('success' => true);
		if ((!isset($_POST['file']) || (strlen($_POST['file']) < 100)) && empty($_POST['webcamData'])) {
			$result = array('error' => _('No file received.'));
		}
		else {
			if (empty($_POST['webcamData'])) {
				$data = base64_decode($_POST['file']);
			}
			else {
				$data = $_POST['webcamData'];
				$data = str_replace('data:image/png;base64,', '', $data);
				$data = base64_decode($data);
			}
			try {
				include_once __DIR__ . '/../imageutils.inc';
				$imageManipulator = ImageManipulationFactory::getImageManipulator($data);
				$imageManipulator->convertToJpeg();
				$data = $imageManipulator->getImageData();
			}
			catch (Exception $e) {
				$result = array('success' => false, 'error' => htmlspecialchars($e->getMessage()));
				echo json_encode($result);
				return;
			}
			$_SESSION[inetOrgPerson::SESS_PHOTO] = $data;
			ob_start();
			$contentElement = $this->getSelfServicePhoto(false, true);
			ob_end_clean();
			ob_start();
			parseHtml(null, $contentElement, [], true, $this->get_scope());
			$content = ob_get_contents();
			ob_end_clean();
			$result['html'] = $content;
		}
		echo json_encode($result);
	}

	/**
	 * Manages the deletion of a photo.
	 */
	private function ajaxDeleteSelfServicePhoto() {
		$_SESSION[self::SESS_PHOTO] = null;
		ob_start();
		$contentElement = $this->getSelfServicePhoto(false, false);
		ob_end_clean();
		ob_start();
		parseHtml(null, $contentElement, [], true, $this->get_scope());
		$content = ob_get_contents();
		ob_end_clean();
		return array(
			'errorsOccurred' => 'false',
			'html' => $content,
		);
	}

	/**
	 * Manages the deletion of a certificate.
	 *
	 * @param array $data JSON data
	 */
	private function ajaxDeleteSelfServiceUserCertificate($data) {
		if (!isset($data['id'])) {
			return self::invalidAjaxRequest();
		}
		$index = $data['id'];
		if (array_key_exists($index, $_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST])) {
			unset($_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST][$index]);
			$_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST] = array_values($_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST]);
		}
		ob_start();
		$contentElement = $this->getSelfServiceUserCertificates();
		ob_end_clean();
		ob_start();
		parseHtml(null, $contentElement, [], true, $this->get_scope());
		$content = ob_get_contents();
		ob_end_clean();
		return array(
			'errorsOccurred' => 'false',
			'html' => $content,
		);
	}

	/**
	 * Invalid AJAX request received.
	 *
	 * @param String $message error message
	 */
	public static function invalidAjaxRequest($message = null) {
		if ($message == null) {
			$message = _('Invalid request');
		}
		return array('errorsOccurred' => 'true', 'errormessage' => $message);
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		if (!$this->isUnixActive()) {
			return !$this->isAdminReadOnly('userPassword');
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
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return [];
		}
		// check password strength
		$user = empty($this->attributes['uid'][0]) ? null : $this->attributes['uid'][0];
		$additionalAttrs = [];
		if (!empty($this->attributes['sn'][0])) {
			$additionalAttrs[] = $this->attributes['sn'][0];
		}
		if (!empty($this->attributes['givenName'][0])) {
			$additionalAttrs[] = $this->attributes['givenName'][0];
		}
		$checkResult = checkPasswordStrength($password, $user, $additionalAttrs);
		if ($checkResult !== true) {
			return array(array('ERROR', $checkResult));
		}
		// set new password
		$this->clearTextPassword = $password;
		// set SASL password
		if (!empty($this->attributes['uid'][0]) && !empty($this->moduleSettings['posixAccount_pwdHash'][0])
				&& ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')) {
			$this->attributes['userpassword'][0] = '{SASL}' . $this->attributes['uid'][0];
		}
		// delay on ldap_exop
		elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
			logNewMessage(LOG_DEBUG, 'Setting password in post action, exop');
		}
		// set normal password
		else {
			$this->attributes['userpassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		}
		return [];
	}

	/**
	 * Loads cached data from LDAP such as departmets etc.
	 */
	private function initCache() {
		if ($this->departmentCache != null) {
			return;
		}
		$attrs = [];
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			$attrs[] = 'departmentNumber';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			$attrs[] = 'ou';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			$attrs[] = 'o';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			$attrs[] = 'title';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			$attrs[] = 'employeeType';
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			$attrs[] = 'businessCategory';
		}
		$departments = [];
		$ous = [];
		$os = [];
		$titles = [];
		$employeeTypes = [];
		$businessCategories = [];
		if (sizeof($attrs) > 0) {
			$result = searchLDAPByFilter('(objectClass=inetOrgPerson)', $attrs, array($this->get_scope()));
			foreach ($result as $attributes) {
				if (isset($attributes['departmentnumber'])) {
					foreach ($attributes['departmentnumber'] as $val) {
						$departments[] = $val;
					}
				}
				if (isset($attributes['ou'])) {
					foreach ($attributes['ou'] as $val) {
						$ous[] = $val;
					}
				}
				if (isset($attributes['o'])) {
					foreach ($attributes['o'] as $val) {
						$os[] = $val;
					}
				}
				if (isset($attributes['title'])) {
					foreach ($attributes['title'] as $val) {
						$titles[] = $val;
					}
				}
				if (isset($attributes['employeetype'])) {
					foreach ($attributes['employeetype'] as $val) {
						$employeeTypes[] = $val;
					}
				}
				if (isset($attributes['businesscategory'])) {
					foreach ($attributes['businesscategory'] as $val) {
						$businessCategories[] = $val;
					}
				}
			}
		}
		$this->departmentCache = array_values(array_unique($departments));
		$this->oCache = array_values(array_unique($os));
		$this->ouCache = array_values(array_unique($ous));
		$this->titleCache = array_values(array_unique($titles));
		$this->employeeTypeCache = array_values(array_unique($employeeTypes));
		$this->businessCategoryCache = array_values(array_unique($businessCategories));
	}

	/**
	 * Returns if the attribute is read-only in admin interface.
	 *
	 * @param String $attrName attribute name
	 * @return boolean attribute is read-only
	 */
	private function isAdminReadOnly($attrName) {
		// for new accounts all fields can be edited
		if ($this->getAccountContainer()->isNewAccount) {
			return false;
		}
		return $this->isBooleanConfigOptionSet('inetOrgPerson_readOnly_' . $attrName);
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_configOptions()
	 */
	public function get_configOptions($scopes, $allScopes) {
		$configContainer = new htmlResponsiveRow();
		if (isset($_SESSION['conf_config'])) {
			// add password hash type if posixAccount is inactive
			$unixModuleFound = false;
			$typeManager = new TypeManager($_SESSION['conf_config']);
			$types = $typeManager->getConfiguredTypesForScopes(array('user', 'group'));
			foreach ($types as $type) {
				$modules = $type->getModules();
				if (in_array('posixAccount', $modules) || in_array('posixGroup', $modules)) {
					$unixModuleFound = true;
					break;
				}
			}
			if (!$unixModuleFound) {
				$optionsSelected = array('CRYPT-SHA512');
				$hashOption = new htmlResponsiveSelect('posixAccount_pwdHash', getSupportedHashTypes(), $optionsSelected, _("Password hash type"), 'pwdHash');
				$configContainer->add($hashOption, 12);
			}
		}
		$configContainerHead = new htmlGroup();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainer->add($configContainerHead, 12);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideDescription', false, _('Description'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideStreet', false, _('Street'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidePostOfficeBox', false, _('Post office box'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidePostalCode', false, _('Postal code'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideLocation', false, _('Location'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideState', false, _('State'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidePostalAddress', false, _('Postal address'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideRegisteredAddress', false, _('Registered address'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideOfficeName', false, _('Office name'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideRoomNumber', false, _('Room number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideTelephoneNumber', false, _('Telephone number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideHomeTelephoneNumber', false, _('Home telephone number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideMobileNumber', false, _('Mobile number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideFaxNumber', false, _('Fax number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidePager', true, _('Pager'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideEMailAddress', false, _('Email address'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideJobTitle', false, _('Job title'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideCarLicense', false, _('Car license'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideEmployeeType', false, _('Employee type'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideBusinessCategory', false, _('Business category'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideDepartments', false, _('Department'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideManager', false, _('Manager'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideOu', false, _('Organisational unit'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideO', false, _('Organisation'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideEmployeeNumber', false, _('Employee number'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideInitials', false, _('Initials'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideLabeledURI', false, _('Web site'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideuserCertificate', false, _('User certificates'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidejpegPhoto', false, _('Photo'), null, true), 12, 4);
		if (isset($_SESSION['conf_config'])) {
			$confActiveUserModules = $_SESSION['conf_config']->get_AccountModules('user');
			if (!in_array('sambaSamAccount', $confActiveUserModules)) {
				$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hidedisplayName', true, _('Display name'), null, true), 12, 4);
			}
			// option to hide uid
			if (!in_array('posixAccount', $confActiveUserModules)) {
				$configContainer->add(new htmlResponsiveInputCheckbox('inetOrgPerson_hideUID', false, _('User name'), null, true), 12, 4);
			}
			else {
				$configContainer->add(new htmlOutputText(''), 0, 4);
			}
		}
		$configContainer->addVerticalSpacer('1rem');
		$advancedOptions = new htmlResponsiveRow();
		$advancedOptions->add(new htmlResponsiveInputCheckbox('inetOrgPerson_addAddressbook', false, _('Add addressbook (ou=addressbook)'), 'addAddressbook'), 12);
		$advancedOptions->add(new htmlSubTitle(_('Read-only fields')), 12);
		$readOnlyOptions = array(
			_('Description') => 'inetOrgPerson_readOnly_description', _('Street') => 'inetOrgPerson_readOnly_street',
			_('First name') => 'inetOrgPerson_readOnly_givenName', _('Last name') => 'inetOrgPerson_readOnly_sn',
			_('Post office box') => 'inetOrgPerson_readOnly_postOfficeBox', _('Postal code') => 'inetOrgPerson_readOnly_postalCode',
			_('Location') => 'inetOrgPerson_readOnly_l', _('State') => 'inetOrgPerson_readOnly_st',
			_('Postal address') => 'inetOrgPerson_readOnly_postalAddress', _('Registered address') => 'inetOrgPerson_readOnly_registeredAddress',
			_('Office name') => 'inetOrgPerson_readOnly_physicalDeliveryOfficeName', _('Room number') => 'inetOrgPerson_readOnly_roomNumber',
			_('Telephone number') => 'inetOrgPerson_readOnly_telephoneNumber', _('Home telephone number') => 'inetOrgPerson_readOnly_homePhone',
			_('Mobile number') => 'inetOrgPerson_readOnly_mobile', _('Fax number') => 'inetOrgPerson_readOnly_facsimileTelephoneNumber',
			_('Pager') => 'inetOrgPerson_readOnly_pager', _('Email address') => 'inetOrgPerson_readOnly_mail',
			_('Web site') => 'inetOrgPerson_readOnly_labeledURI', _('Job title') => 'inetOrgPerson_readOnly_title',
			_('Car license') => 'inetOrgPerson_readOnly_carLicense', _('Employee type') => 'inetOrgPerson_readOnly_employeeType',
			_('Business category') => 'inetOrgPerson_readOnly_businessCategory',
			_('Department') => 'inetOrgPerson_readOnly_departmentNumber', _('Manager') => 'inetOrgPerson_readOnly_manager',
			_('Organisation') => 'inetOrgPerson_readOnly_o', _('Organisational unit') => 'inetOrgPerson_readOnly_ou',
			_('Employee number') => 'inetOrgPerson_readOnly_employeeNumber', _('Initials') => 'inetOrgPerson_readOnly_initials',
			_('Photo') => 'inetOrgPerson_readOnly_jpegPhoto', _('Password') => 'inetOrgPerson_readOnly_userPassword'
		);
		if (isset($_SESSION['conf_config'])) {
			$readOnlyOptions[_('User name')] = 'inetOrgPerson_readOnly_uid';
			$readOnlyOptions[_('Common name')] = 'inetOrgPerson_readOnly_cn';
		}
		ksort($readOnlyOptions);
		foreach ($readOnlyOptions as $label => $id) {
			$advancedOptions->add(new htmlResponsiveInputCheckbox($id, false, $label, null, true), 12, 4);
		}
		$advancedOptions->add(new htmlSubTitle(_('Photo')), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum width (px)'), 'inetOrgPerson_jpegPhoto_maxWidth', null, 'crop'), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum height (px)'), 'inetOrgPerson_jpegPhoto_maxHeight', null, 'crop'), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum file size (kB)'), 'inetOrgPerson_jpegPhoto_maxSize'), 12);
		$advancedOptionsAccordion = new htmlAccordion('inetOrgPersonAdvancedOptions', array(_('Advanced options') => $advancedOptions), false);
		$configContainer->add($advancedOptionsAccordion);
		return $configContainer;
	}

	/**
	 * Checks if the given email address already exists in LDAP.
	 *
	 * @param String $mail email address
	 * @return boolean true if already exists
	 */
	private function emailExists($mail) {
		if (empty($mail)) {
			return false;
		}
		if (isset($this->emailCheckCache[$mail])) {
			return $this->emailCheckCache[$mail];
		}
		$result = searchLDAPByAttribute('mail', $mail, 'inetOrgPerson', array('dn'), array('user'));
		$this->emailCheckCache[$mail] = (sizeof($result) > 0);
		return $this->emailCheckCache[$mail];
	}

	/**
	 * Returns if the Unix module is also active.
	 *
	 * @param string[] $modules active account modules
	 * @return boolean Unix is active
	 */
	private function isUnixActive($modules = null) {
		if (!empty($modules)) {
			return in_array('posixAccount', $modules);
		}
		if ($this->getAccountContainer() == null) {
			return false;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		return in_array('posixAccount', $modules);
	}

	/**
	 * Returns if the Samba 3 module is also active.
	 *
	 * @param string[] $modules active account modules
	 * @return boolean Samba 3 is active
	 */
	private function isSamba3Active($modules = null) {
		if (!empty($modules)) {
			return in_array('sambaSamAccount', $modules);
		}
		if ($this->getAccountContainer() == null) {
			return false;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		return in_array('sambaSamAccount', $modules);
	}

	/**
	 * {@inheritdoc}
	 */
	public function getWildCardReplacements() {
		$replacements = [];
		// first name
		if (!empty($_POST['givenName'])) {
			$replacements['firstname'] = $_POST['givenName'];
		}
		elseif (!empty($this->attributes['givenName'][0])) {
			$replacements['firstname'] = $this->attributes['givenName'][0];
		}
		// last name
		if (!empty($_POST['sn'])) {
			$replacements['lastname'] = $_POST['sn'];
		}
		elseif (!empty($this->attributes['sn'][0])) {
			$replacements['lastname'] = $this->attributes['sn'][0];
		}
		// user name
		if (!$this->isUnixActive()) {
			if (!empty($_POST['uid'])) {
				$replacements['user'] = $_POST['uid'];
			}
			elseif (!empty($this->attributes['uid'][0])) {
				$replacements['user'] = $this->attributes['uid'][0];
			}
		}
		// cn
		if (!empty($_POST['cn_0'])) {
			$replacements['commonname'] = $_POST['cn_0'];
		}
		elseif (!empty($this->attributes['cn'][0])) {
			$replacements['commonname'] = $this->attributes['cn'][0];
		}
		// mail
		if (!empty($_POST['mail_0'])) {
			$replacements['email'] = $_POST['mail_0'];
		}
		elseif (!empty($this->attributes['mail'][0])) {
			$replacements['email'] = $this->attributes['mail'][0];
		}
		return $replacements;
	}

	/**
	 * Returns if the password of the current account is locked.
	 *
	 * @param ?array $attributes LDAP attribute data
	 * @return bool password is locked
	 */
	public function isLocked(?array $attributes = null): bool {
		if ($attributes === null) {
			$attributes = array_change_key_case($this->attributes, CASE_LOWER);
		}
		return isset($attributes['userpassword'][0])
			&& !pwd_is_enabled($attributes['userpassword'][0]);
	}

	/**
	 * @inheritDoc
	 */
	public function supportsPasswordQuickChangePage(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function addPasswordQuickChangeAccountDetails(htmlResponsiveRow $row): void {
		if (!$this->isUnixActive() && !empty($this->attributes['uid'][0])) {
			$row->addLabel(new htmlOutputText(_('User name')));
			$row->addField(new htmlOutputText($this->attributes['uid'][0]));
		}
		$nameParts = [];
		if (isset($this->attributes['givenName'][0])) {
			$nameParts[] = $this->attributes['givenName'][0];
		}
		if (isset($this->attributes['sn'][0])) {
			$nameParts[] = $this->attributes['sn'][0];
		}
		if (!empty($nameParts)) {
			$row->addLabel(new htmlOutputText(_('Full name')));
			$row->addField(new htmlOutputText(implode(' ', $nameParts)));
		}
		if (!empty($this->attributes['mail'][0])) {
			$row->addLabel(new htmlOutputText(_('Email address')));
			$row->addField(new htmlOutputText($this->attributes['mail'][0]));
		}
		if (!empty($this->attributes['telephoneNumber'][0])) {
			$row->addLabel(new htmlOutputText(_('Telephone number')));
			$row->addField(new htmlOutputText($this->attributes['telephoneNumber'][0]));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeOptions(): array {
		if ($this->isUnixActive() && in_array_ignore_case('posixAccount', $this->attributes['objectClass'])) {
			return [];
		}
		if (!in_array_ignore_case('inetOrgPerson', $this->attributes['objectClass'])) {
			return [];
		}
		return array(
			new PasswordQuickChangeOption('updateUnixPwd', _('Change password'))
		);
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		if ($this->isUnixActive() && in_array_ignore_case('posixAccount', $this->attributes['objectClass'])) {
			return [];
		}
		if (!in_array_ignore_case('inetOrgPerson', $this->attributes['objectClass'])) {
			return [];
		}
		$attrs = [];
		if (isset($_POST['updateUnixPwd'])) {
			$hashType = $this->moduleSettings['posixAccount_pwdHash'][0];
			if ($hashType === 'LDAP_EXOP') {
				$this->clearTextPassword = $password;
			}
			else {
				$attrs['userpassword'][0] = pwd_hash($password, true, $hashType);
			}
		}
		return $attrs;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string {
		if (isset($this->attributes['uid'][0])) {
			return $this->attributes['uid'][0];
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array {
		$values = [];
		if (isset($this->attributes['sn'][0])) {
			$values[] = $this->attributes['sn'][0];
		}
		if (isset($this->attributes['givenName'][0])) {
			$values[] = $this->attributes['givenName'][0];
		}
		return $values;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusDetails(ConfiguredType $type, ?array &$attributes): array {
		if ($this->isUnixActive($type->getModules())) {
			return [];
		}
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$details = [];
		if (self::isLocked($attributes)) {
			$details[] = AccountStatusDetails::newLocked(_('Personal'), self::STATUS_PASSWORD_LOCKED);
		}
		return $details;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusRequiredAttributes(ConfiguredType $type): array {
		if ($this->isUnixActive($type->getModules())) {
			return [];
		}
		return array('userpassword');
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusPossibleLockOptions(ConfiguredType $type, ?array &$attributes): array {
		if ($this->isUnixActive($type->getModules())) {
			return [];
		}
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$options = [];
		if (!self::isLocked($attributes) && isset($attributes['userpassword'][0]) && pwd_is_lockable($attributes['userpassword'][0])) {
			$options[] = AccountStatusDetails::newLocked(_('Personal'), self::STATUS_PASSWORD_LOCKED);
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformLock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($this->isUnixActive($type->getModules())) {
			return;
		}
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_PASSWORD_LOCKED, $lockIds)) {
			$attributes['userpassword'][0] = pwd_disable($attributes['userpassword'][0]);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformUnlock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($this->isUnixActive($type->getModules())) {
			return;
		}
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_PASSWORD_LOCKED, $lockIds)) {
			$attributes['userpassword'][0] = pwd_enable($attributes['userpassword'][0]);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return array(
			'givenname' => _('First name'),
			'sn' => _('Last name'),
			'description' => _('Description'),
			'street' => _('Street'),
			'postofficebox' => _('Post office box'),
			'postalcode' => _('Postal code'),
			'l' => _('Location'),
			'st' => _('State'),
			'postaladdress' => _('Postal address'),
			'registeredaddress' => _('Registered address'),
			'officename' => _('Office name'),
			'roomnumber' => _('Room number'),
			'telephonenumber' => _('Telephone number'),
			'homephone' => _('Home telephone number'),
			'mobiletelephonenumber' => _('Mobile number'),
			'pager' => _('Pager'),
			'mail' => _('Email address'),
			'labeleduri' => _('Web site'),
			'title' => _('Job title'),
			'carlicense' => _('Car license'),
			'facsimiletelephonenumber' => _('Fax number'),
			'employeetype' => _('Employee type'),
			'businesscategory' => _('Business category'),
			'departmentnumber' => _('Department'),
			'manager' => _('Manager'),
			'o' => _('Organisation'),
			'ou' => _('Organisational unit'),
			'employeenumber' => _('Employee number'),
			'initials' => _('Initials'),
			'jpegphoto' => _('Photo'),
		);
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName): ?callable {
		if (($attributeName === 'mail') || ($attributeName === 'rfc822mailbox')) {
			return function(array $entry, string $attribute): ?htmlElement {
				$group = new htmlGroup();
				if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
					for ($i = 0; $i < sizeof($entry[$attribute]); $i++) {
						if ($i > 0) {
							$group->addElement(new htmlOutputText(", "));
						}
						$group->addElement(new htmlLink($entry[$attribute][$i], "mailto:" . $entry[$attribute][$i]));
					}
				}
				return $group;
			};
		}
		elseif ($attributeName === 'jpegphoto') {
			return function(array $entry, string $attribute): ?htmlElement {
				if (isset($entry[$attribute][0]) && ($entry[$attribute][0] !== '')) {
					if (strlen($entry[$attribute][0]) < 100) {
						// looks like we have read broken binary data, reread photo
						$result = @ldap_read($_SESSION['ldap']->server(), $entry['dn'], $attribute . "=*", array($attribute), 0, 0, 0, LDAP_DEREF_NEVER);
						if ($result) {
							$tempEntry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
							if ($tempEntry) {
								$binData = ldap_get_values_len($_SESSION['ldap']->server(), $tempEntry, $attribute);
								$entry[$attribute] = $binData;
							}
						}
					}
					$tempFilesManager = new LamTemporaryFilesManager();
					$fileName = $tempFilesManager->registerTemporaryFile('.jpg');
					$handle = $tempFilesManager->openTemporaryFileForWrite($fileName);
					fwrite($handle, $entry[$attribute][0]);
					fclose ($handle);
					$photoFile = $tempFilesManager->getResourceLink($fileName);
					$image = new htmlImage($photoFile);
					$image->enableLightbox();
					$image->setCSSClasses(array('thumbnail'));
					return $image;
				}
				return null;
			};
		}
		return null;
	}

}
