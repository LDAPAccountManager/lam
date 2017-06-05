<?php
use \LAM\TYPES\TypeManager;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2017  Roland Gruber

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

	/** clear text password */
	private $clearTextPassword = null;
	/** cache for departments */
	private $departmentCache = null;
	/** organization cache */
	private $oCache = null;
	/** organizational unit cache */
	private $ouCache = null;
	/** title cache */
	private $titleCache = null;
	/** employee type cache */
	private $employeeTypeCache = null;
	/** business category cache */
	private $businessCategoryCache = null;
	/** cache for email duplication checks */
	private $emailCheckCache = array();

	/** session variable for existing user certificates in self service */
	const SESS_CERTIFICATES_LIST = 'inetOrgPerson_certificatesList';

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
		$this->messages['file'][1] = array('ERROR', _('Please upload a .jpg/.jpeg file.'));
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
		$return = array();
		// icon
		$return['icon'] = 'uid.png';
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
		$return['attributes'] = array('uid', 'employeeType', 'givenName', 'jpegPhoto', 'mail', 'manager', 'mobile',
			'title', 'telephoneNumber', 'facsimileTelephoneNumber', 'street', 'postOfficeBox', 'postalCode', 'postalAddress',
			'sn', 'userPassword', 'description', 'homePhone', 'pager', 'roomNumber', 'businessCategory', 'l', 'st',
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
			'uid' => _('User name'));
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = array('firstName', 'lastName', 'mail', 'telephoneNumber', 'mobile', 'faxNumber', 'pager', 'street',
			'postalAddress', 'registeredAddress', 'postalCode', 'postOfficeBox', 'jpegPhoto', 'homePhone', 'roomNumber', 'carLicense',
			'location', 'state', 'officeName', 'businessCategory', 'departmentNumber', 'initials', 'title', 'labeledURI', 'userCertificate',
			'o', 'ou', 'description', 'uid');
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
		// password status change
		if (!$this->isUnixActive()) {
			$pwdOrig = empty($this->orig['userPassword'][0]) ? '' : $this->orig['userPassword'][0];
			$pwdNew = empty($this->attributes['userPassword'][0]) ? '' : $this->attributes['userPassword'][0];
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
		$messages = array();
		// add address book
		$accountContainer = $this->getAccountContainer();
		if ($this->isBooleanConfigOptionSet('inetOrgPerson_addAddressbook') && !empty($accountContainer)) {
			$dn = 'ou=addressbook,' . $accountContainer->finalDN;
			$result = ldapGetDN($dn);
			if (empty($result)) {
				$ldapUser = $_SESSION['ldap']->decrypt_login();
				$ldapUser = $ldapUser[0];
				$attrs = array(
					'objectClass' => array('organizationalUnit'),
					'ou' => 'addressbook'
				);
				$success = @ldap_add($_SESSION['ldap']->server(), $dn, $attrs);
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add addressbook for user ' . $accountContainer->finalDN . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_("Was unable to create DN: %s."), htmlspecialchars($dn)), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added addressbook for user ' . $accountContainer->finalDN);
				}
			}
		}
		return $messages;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
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
					$this->attributes['userPassword'][0] = pwd_disable($this->attributes['userPassword'][0]);
				}
				if (isset($_POST['unlockPassword'])) {
					$this->attributes['userPassword'][0] = pwd_enable($this->attributes['userPassword'][0]);
				}
				if (isset($_POST['removePassword'])) {
					unset($this->attributes['userPassword']);
				}
				// set SASL password for new and renamed users
				if (!empty($this->attributes['uid'][0]) && !empty($this->moduleSettings['posixAccount_pwdHash'][0])
						&& ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')
						&& ($this->getAccountContainer()->isNewAccount || ($this->attributes['uid'][0] != $this->orig['uid'][0]))) {
					$this->attributes['userPassword'][0] = '{SASL}' . $this->attributes['uid'][0];
				}
				// set K5KEY password for new users
				if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
					$this->attributes[$this->getPasswordAttrName()][0] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto') && isset($_POST['delPhoto']) && !$this->isAdminReadOnly('jpegPhoto')) {
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
		$this->initCache();
		$equalWidthElements = array();
		$container = new htmlTable();
		$fieldContainer = new htmlTable();
		$container->addElement($fieldContainer);
		// uid
		if (!$this->isUnixActive() && !$this->isBooleanConfigOptionSet('inetOrgPerson_hideUID')) {
			if ($this->isAdminReadOnly('uid')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'uid', _('User name'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'uid', _('User name'));
				$equalWidthElements[] = 'uid';
			}
		}
		// first name
		if ($this->isAdminReadOnly('givenName')) {
			$this->addSimpleReadOnlyField($fieldContainer, 'givenName', _('First name'));
		}
		else {
			$this->addSimpleInputTextField($fieldContainer, 'givenName', _('First name'));
			$equalWidthElements[] = 'givenName';
		}
		// last name
		if ($this->isAdminReadOnly('sn')) {
			$this->addSimpleReadOnlyField($fieldContainer, 'sn', _('Last name'));
		}
		else {
			$this->addSimpleInputTextField($fieldContainer, 'sn', _('Last name'), true);
			$equalWidthElements[] = 'sn';
		}
		// initials
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideInitials')) {
			if ($this->isAdminReadOnly('initials')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'initials', _('Initials'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'initials', _('Initials'));
				$equalWidthElements[] = 'initials';
			}
		}
		// common name
		if (!$this->isUnixActive()) {
			if ($this->isAdminReadOnly('cn')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'cn', _('Common name'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'cn', _('Common name'), true);
				$equalWidthElements[] = 'cn';
			}
		}
		// description
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDescription')) {
			if ($this->isAdminReadOnly('description')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'description', _('Description'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'description', _('Description'), false, null, false, null, null, $equalWidthElements);
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideState') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalAddress')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideRegisteredAddress')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Address')), true);
		}

		// street
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideStreet')) {
			if ($this->isAdminReadOnly('street')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'street', _('Street'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'street', _('Street'), false, null, false, null, null, $equalWidthElements);
			}
		}
		// post office box
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostOfficeBox')) {
			if ($this->isAdminReadOnly('postOfficeBox')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'postOfficeBox', _('Post office box'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'postOfficeBox', _('Post office box'), false, null, false, null, null, $equalWidthElements);
			}
		}
		// postal code
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePostalCode')) {
			if ($this->isAdminReadOnly('postalCode')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'postalCode', _('Postal code'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'postalCode', _('Postal code'), false, null, false, null, null, $equalWidthElements);
			}
		}
		// location
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLocation')) {
			if ($this->isAdminReadOnly('l')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'l', _('Location'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'l', _('Location'), false, null, false, null, null, $equalWidthElements);
			}
		}
		// state
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideState')) {
			if ($this->isAdminReadOnly('st')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'st', _('State'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'st', _('State'), false, null, false, null, null, $equalWidthElements);
			}
		}
		// postal address
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
				if ($this->isAdminReadOnly('postalAddress')) {
					$val = str_replace("\r\n", '<br>', htmlspecialchars($postalAddresses[$i]));
					$addressContainer->addElement(new htmlOutputText($val, false));
					if ($i < (sizeof($postalAddresses) - 1)) {
						$addressContainer->addElement(new htmlOutputText('<br>', false));
					}
				}
				else {
					$addressContainer->addElement(new htmlInputTextarea('postalAddress' . $i, $postalAddresses[$i], 30, 3));
					$equalWidthElements[] = 'postalAddress' . $i;
					if ($i < (sizeof($postalAddresses) - 1)) {
						$addressContainer->addElement(new htmlOutputText('<br>', false));
					}
					else {
						$addressContainer->addElement(new htmlButton('addPostalAddress', 'add.png', true));
					}
				}
			}
			$fieldContainer->addElement($addressContainer);
			if ($this->isAdminReadOnly('postalAddress')) {
				$fieldContainer->addNewLine();
			}
			else {
				$addressHelp = new htmlHelpLink('postalAddress');
				$addressHelp->alignment = htmlElement::ALIGN_TOP;
				$fieldContainer->addElement($addressHelp, true);
			}
		}
		// registered address
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
				if ($this->isAdminReadOnly('registeredAddress')) {
					$val = str_replace("\r\n", '<br>', htmlspecialchars($registeredAddresses[$i]));
					$registeredAddressContainer->addElement(new htmlOutputText($val, false));
					if ($i < (sizeof($registeredAddresses) - 1)) {
						$registeredAddressContainer->addElement(new htmlOutputText('<br>', false));
					}
				}
				else {
					$registeredAddressContainer->addElement(new htmlInputTextarea('registeredAddress' . $i, $registeredAddresses[$i], 30, 3));
					$equalWidthElements[] = 'registeredAddress' . $i;
					if ($i < (sizeof($registeredAddresses) - 1)) {
						$registeredAddressContainer->addElement(new htmlOutputText('<br>', false));
					}
					else {
						$registeredAddressContainer->addElement(new htmlButton('addRegisteredAddress', 'add.png', true));
					}
				}
			}
			$fieldContainer->addElement($registeredAddressContainer);
			if ($this->isAdminReadOnly('registeredAddress')) {
				$fieldContainer->addNewLine();
			}
			else {
				$registeredAddressHelp = new htmlHelpLink('registeredAddress');
				$registeredAddressHelp->alignment = htmlElement::ALIGN_TOP;
				$fieldContainer->addElement($registeredAddressHelp, true);
			}
		}
		// office name
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOfficeName')) {
			if ($this->isAdminReadOnly('physicalDeliveryOfficeName')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'physicalDeliveryOfficeName', _('Office name'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'physicalDeliveryOfficeName', _('Office name'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideRoomNumber')) {
			if ($this->isAdminReadOnly('roomNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'roomNumber', _('Room number'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'roomNumber', _('Room number'));
				$equalWidthElements[] = 'roomNumber';
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Contact data')), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideTelephoneNumber')) {
			if ($this->isAdminReadOnly('telephoneNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'telephoneNumber', _('Telephone number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'telephoneNumber', _('Telephone number'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideHomeTelephoneNumber')) {
			if ($this->isAdminReadOnly('homePhone')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'homePhone', _('Home telephone number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'homePhone', _('Home telephone number'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideMobileNumber')) {
			if ($this->isAdminReadOnly('mobile')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'mobile', _('Mobile number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'mobile', _('Mobile number'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideFaxNumber')) {
			if ($this->isAdminReadOnly('facsimileTelephoneNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'facsimileTelephoneNumber', _('Fax number'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'facsimileTelephoneNumber', _('Fax number'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidePager', true)) {
			if ($this->isAdminReadOnly('pager')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'pager', _('Pager'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'pager', _('Pager'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEMailAddress')) {
			if ($this->isAdminReadOnly('mail')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'mail', _('Email address'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'mail', _('Email address'), false, null, false, null, null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideLabeledURI')) {
			if ($this->isAdminReadOnly('labeledURI')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'labeledURI', _('Web site'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'labeledURI', _('Web site'), false, null, false, null, null, $equalWidthElements);
			}
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments') || !$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')
				|| !$this->isBooleanConfigOptionSet('inetOrgPerson_hideuserCertificate')) {
			$fieldContainer->addElement(new htmlSubTitle(_('Work details')), true);
		}

		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideJobTitle')) {
			if ($this->isAdminReadOnly('title')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'title', _('Job title'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'title', _('Job title'), false, null, false, array_slice($this->titleCache, 0, 300), null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideCarLicense')) {
			if ($this->isAdminReadOnly('carLicense')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'carLicense', _('Car license'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'carLicense', _('Car license'));
				$equalWidthElements[] = 'carLicense';
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeNumber')) {
			if ($this->isAdminReadOnly('employeeNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'employeeNumber', _('Employee number'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'employeeNumber', _('Employee number'));
				$equalWidthElements[] = 'employeeNumber';
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideEmployeeType')) {
			if ($this->isAdminReadOnly('employeeType')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'employeeType', _('Employee type'));
			}
			else {
				$this->addSimpleInputTextField($fieldContainer, 'employeeType', _('Employee type'), false, null, false, array_slice($this->employeeTypeCache, 0, 300));
				$equalWidthElements[] = 'employeeType';
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideBusinessCategory')) {
			if ($this->isAdminReadOnly('businessCategory')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'businessCategory', _('Business category'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'businessCategory', _('Business category'), false, null, false, array_slice($this->businessCategoryCache, 0, 300), null, $equalWidthElements);
			}
		}
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideDepartments')) {
			if ($this->isAdminReadOnly('departmentNumber')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'departmentNumber', _('Department'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'departmentNumber', _('Department'), false, null, false, array_slice($this->departmentCache, 0, 300), null, $equalWidthElements);
			}
		}
		// organisational unit
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideOu')) {
			if ($this->isAdminReadOnly('ou')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'ou', _('Organisational unit'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'ou', _('Organisational unit'), false, null, false, array_slice($this->ouCache, 0, 300), null, $equalWidthElements);
			}
		}
		// organisation
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideO')) {
			if ($this->isAdminReadOnly('o')) {
				$this->addSimpleReadOnlyField($fieldContainer, 'o', _('Organisation'));
			}
			else {
				$this->addMultiValueInputTextField($fieldContainer, 'o', _('Organisation'), false, null, false, array_slice($this->oCache, 0, 300), null, $equalWidthElements);
			}
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
			if (!$this->isAdminReadOnly('manager')) {
				$userCertificateGroup->addElement(new htmlAccountPageButton(get_class($this), 'userCertificate', 'manage', _('Manage')));
			}
			$fieldContainer->addElement($userCertificateGroup);
			if ($this->isAdminReadOnly('manager')) {
				$fieldContainer->addNewLine();
			}
			else {
				$fieldContainer->addElement(new htmlHelpLink('userCertificate'), true);
			}
		}
		// manager
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hideManager')) {
			$fieldContainer->addElement(new htmlOutputText(_('Manager')));
			if ($this->isAdminReadOnly('manager')) {
				$fieldContainer->addNewLine();
			}
			else {
				$fieldContainer->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'change', _("Change")));
				$fieldContainer->addElement(new htmlHelpLink('manager'), true);
			}
			if (isset($this->attributes['manager'][0])) {
				$managerList = array();
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
				$fieldContainer->addElement(new htmlOutputText(''));
				$fieldContainer->addElement($managers, true);
			}
		}
		// password buttons
		if (!$this->isUnixActive() && checkIfWriteAccessIsAllowed($this->get_scope()) && isset($this->attributes['userPassword'][0]) && !$this->isAdminReadOnly('userPassword')) {
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
		if (!$this->isBooleanConfigOptionSet('inetOrgPerson_hidejpegPhoto')) {
			$imageContainer = new htmlTable();
			$imageContainer->alignment = htmlElement::ALIGN_TOP;
			$photoFile = '../../graphics/userDefault.png';
			$noPhoto = true;
			if (isset($this->attributes['jpegPhoto'][0])) {
				$jpeg_filename = 'jpg' . getRandomNumber() . '.jpg';
				$outjpeg = @fopen(dirname(__FILE__) . '/../../tmp/' . $jpeg_filename, "wb");
				fwrite($outjpeg, $this->attributes['jpegPhoto'][0]);
				fclose ($outjpeg);
				$photoFile = '../../tmp/' . $jpeg_filename;
				$noPhoto = false;
			}
			$img = new htmlImage($photoFile);
			$img->setCSSClasses(array('photo'));
			$imageContainer->addElement($img, true);
			if (!$this->isAdminReadOnly('jpegPhoto')) {
				if ($noPhoto) {
					$imageContainer->addElement(new htmlAccountPageButton(get_class($this), 'photo', 'open', _('Add photo')));
				}
				else {
					$imageContainer->addElement(new htmlButton('delPhoto', _('Delete photo')));
				}
			}
			$container->addElement(new htmlSpacer('50px', null));
			$container->addElement($imageContainer, false);
		}
		$container->addElement(new htmlEqualWidth($equalWidthElements));
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
		if ($this->isAdminReadOnly('jpegPhoto')) {
			return $messages;
		}
		if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
			$name = $_FILES['photoFile']['name'];
			$extension = strtolower(substr($name, strpos($name, '.') + 1));
			if (!extension_loaded('imagick') && !($extension == 'jpg') && !($extension == 'jpeg')) {
				$messages[] = $this->messages['file'][1];
				return $messages;
			}
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 10000000);
			if (!empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]) && (strlen($data) > (1024 * $this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]))) {
				$errMsg = $this->messages['file'][3];
				$errMsg[] = null;
				$errMsg[] = array($this->moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]);
				return array($errMsg);
			}
			fclose($handle);
			if (extension_loaded('imagick')) {
				// convert to JPG if imagick extension is available
				$image = new Imagick();
				try {
				$image->readImageBlob($data);
				// resize if maximum values specified
				if (!empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0]) || !empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0])) {
					$maxWidth = empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0]) ? $image->getimagewidth() : $this->moduleSettings['inetOrgPerson_jpegPhoto_maxWidth'][0];
					$maxHeight = empty($this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0]) ? $image->getimageheight() : $this->moduleSettings['inetOrgPerson_jpegPhoto_maxHeight'][0];
					$image->thumbnailimage($maxWidth, $maxHeight, true);
				}
				$image->setImageCompression(Imagick::COMPRESSION_JPEG);
				$image->setImageFormat('jpeg');
				$data = $image->getimageblob();
				}
				catch (Exception $e) {
					$msg =  $this->messages['file'][2];
					$msg[] = htmlspecialchars($e->getMessage());
					$messages[] = $msg;
					return $messages;
				}
			}
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
		$label = _('Photo file (JPG format)');
		if (extension_loaded('imagick')) {
			$label = _('Photo file');
		}
		$container->addElement(new htmlTableExtendedInputFileUpload('photoFile', $label, 'photoUpload'), true);
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
		$container = new htmlTable();
		if (isset($this->attributes['userCertificate;binary'])) {
			$table = new htmlTable();
			$table->colspan = 10;
			for ($i = 0; $i < sizeof($this->attributes['userCertificate;binary']); $i++) {
				$filename = 'userCertificate' . getRandomNumber() . '.der';
				$pathOut = dirname(__FILE__) . '/../../tmp/' . $filename;
				$out = @fopen($pathOut, "wb");
				@chmod($pathOut, 0640);
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
	*
	* @return array list of info/error messages
	*/
	function process_userCertificate() {
		$messages = array();
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
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'title', _('Job title'));
		$this->addSimplePDFField($return, 'givenName', _('First name'));
		$this->addSimplePDFField($return, 'sn', _('Last name'));
		$this->addSimplePDFField($return, 'manager', _('Manager'));
		$this->addSimplePDFField($return, 'street', _('Street'));
		$this->addSimplePDFField($return, 'postOfficeBox', _('Post office box'));
		$this->addSimplePDFField($return, 'postalCode', _('Postal code'));
		$this->addSimplePDFField($return, 'postalAddress', _('Postal address'));
		$this->addSimplePDFField($return, 'registeredAddress', _('Registered address'));
		$this->addSimplePDFField($return, 'telephoneNumber', _('Telephone number'));
		$this->addSimplePDFField($return, 'homePhone', _('Home telephone number'));
		$this->addSimplePDFField($return, 'mobileTelephoneNumber', _('Mobile number'), 'mobile');
		$this->addSimplePDFField($return, 'pager', _('Pager'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'facsimileTelephoneNumber', _('Fax number'));
		$this->addSimplePDFField($return, 'roomNumber', _('Room number'));
		$this->addSimplePDFField($return, 'businessCategory', _('Business category'));
		$this->addSimplePDFField($return, 'uid', _('User name'));
		$this->addSimplePDFField($return, 'carLicense', _('Car license'));
		$this->addSimplePDFField($return, 'state', _('State'), 'st');
		$this->addSimplePDFField($return, 'officeName', _('Office name'), 'physicalDeliveryOfficeName');
		$this->addSimplePDFField($return, 'employeeType', _('Employee type'));
		$this->addSimplePDFField($return, 'location', _('Location'), 'l');
		$this->addSimplePDFField($return, 'employeeNumber', _('Employee number'));
		$this->addSimplePDFField($return, 'ou', _('Organisational unit'));
		$this->addSimplePDFField($return, 'o', _('Organisation'));
		$this->addSimplePDFField($return, 'initials', _('Initials'));
		$this->addSimplePDFField($return, 'labeledURI', _('Web site'));
		$this->addSimplePDFField($return, 'departmentNumber', _('Department'));
		if (isset($this->clearTextPassword)) {
			$this->addPDFKeyValue($return, 'userPassword', _('Password'), $this->clearTextPassword);
		}
		else if (isset($this->attributes['INFO.userPasswordClearText'])) {
			$this->addPDFKeyValue($return, 'userPassword', _('Password'), $this->attributes['INFO.userPasswordClearText']);
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
		$profileElements = array();
		if (!$this->isUnixActive($modules)) {
			$profileElements[] = new htmlTableExtendedInputField(_('Common name'), 'inetOrgPerson_cn', null, 'cn');
		}
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
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
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
			// initials
			if (isset($ids['inetOrgPerson_initials']) && ($rawAccounts[$i][$ids['inetOrgPerson_initials']] != "")) {
				$partialAccounts[$i]['initials'] = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['inetOrgPerson_initials']]);
			}
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
				// set SASL password
				if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')) {
					$partialAccounts[$i]['userPassword'] = '{SASL}' . $partialAccounts[$i]['uid'];
				}
				// set K5KEY password
				elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
					$partialAccounts[$i]['userPassword'] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				// set normal password
				else {
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
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::doUploadPostActions()
	 */
	function doUploadPostActions(&$data, $ids, $failed, &$temp, &$accounts, $selectedModules) {
		if (!checkIfWriteAccessIsAllowed($this->get_scope())) {
			die();
		}
		if (!isset($temp['counter'])) {
			$temp['counter'] = 0;
		}
		$errors = array();
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
		$container = new htmlTable();
		$container->addElement(new htmlSubTitle(_('Photo')), true);
		$photoTable = new htmlTable();
		$photoTable->colspan = 2;
		if (extension_loaded('imagick')) {
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum width (px)'), 'inetOrgPerson_jpegPhoto_maxWidth'));
			$photoTable->addElement(new htmlHelpLink('crop', get_class($this)), true);
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum height (px)'), 'inetOrgPerson_jpegPhoto_maxHeight'));
			$photoTable->addElement(new htmlHelpLink('crop', get_class($this)), true);
		}
		$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum file size (kB)'), 'inetOrgPerson_jpegPhoto_maxSize'), true);
		$container->addElement($photoTable, true);
		return $container;
	}

	/**
	 * Checks if the self service settings are valid.
	 *
	 * If the input data is invalid the return value is an array that contains arrays
	 * to build StatusMessages (message type, message head, message text). If no errors
	 * occured the function returns an empty array.
	 *
	 * @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @param selfServiceProfile $profile self service profile
	 * @return array error messages
	 */
	public function checkSelfServiceSettings(&$options, &$profile) {
		$errors = array();
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
			$return['firstName'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('firstName', _('First name'))), $firstNameField
			);
		}
		if (in_array('lastName', $fields)) {
			$lastName = '';
			if (isset($attributes['sn'][0])) $lastName = $attributes['sn'][0];
			$lastNameField = new htmlInputField('inetOrgPerson_lastName', $lastName);
			if (in_array('lastName', $readOnlyFields)) {
				$lastNameField = new htmlOutputText($lastName);
			}
			$return['lastName'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('lastName', _('Last name'))), $lastNameField
			);
		}
		if (in_array('mail', $fields)) {
			$mail = '';
			if (isset($attributes['mail'][0])) $mail = $attributes['mail'][0];
			$mailField = new htmlInputField('inetOrgPerson_mail', $mail);
			if (in_array('mail', $readOnlyFields)) {
				$mailField = new htmlOutputText($mail);
			}
			$return['mail'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('mail', _('Email address'))), $mailField
			);
		}
		if (in_array('labeledURI', $fields)) {
			$labeledURI = '';
			if (isset($attributes['labeledURI'][0])) $labeledURI = implode('; ', $attributes['labeledURI']);
			$labeledURIField = new htmlInputField('inetOrgPerson_labeledURI', $labeledURI);
			if (in_array('labeledURI', $readOnlyFields)) {
				$labeledURIField = new htmlOutputText($labeledURI);
			}
			$return['labeledURI'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('labeledURI', _('Web site'))), $labeledURIField
			);
		}
		if (in_array('telephoneNumber', $fields)) {
			$telephoneNumber = '';
			if (isset($attributes['telephoneNumber'][0])) $telephoneNumber = $attributes['telephoneNumber'][0];
			$telephoneNumberField = new htmlInputField('inetOrgPerson_telephoneNumber', $telephoneNumber);
			if (in_array('telephoneNumber', $readOnlyFields)) {
				$telephoneNumberField = new htmlOutputText($telephoneNumber);
			}
			$return['telephoneNumber'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('telephoneNumber', _('Telephone number'))), $telephoneNumberField
			);
		}
		if (in_array('homePhone', $fields)) {
			$homePhone = '';
			if (isset($attributes['homePhone'][0])) $homePhone = $attributes['homePhone'][0];
			$homePhoneField = new htmlInputField('inetOrgPerson_homePhone', $homePhone);
			if (in_array('homePhone', $readOnlyFields)) {
				$homePhoneField = new htmlOutputText($homePhone);
			}
			$return['homePhone'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('homePhone', _('Home telephone number'))), $homePhoneField
			);
		}
		if (in_array('mobile', $fields)) {
			$mobile = '';
			if (isset($attributes['mobile'][0])) $mobile = $attributes['mobile'][0];
			$mobileField = new htmlInputField('inetOrgPerson_mobile', $mobile);
			if (in_array('mobile', $readOnlyFields)) {
				$mobileField = new htmlOutputText($mobile);
			}
			$return['mobile'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('mobile', _('Mobile telephone number'))), $mobileField
			);
		}
		if (in_array('faxNumber', $fields)) {
			$faxNumber = '';
			if (isset($attributes['facsimileTelephoneNumber'][0])) $faxNumber = $attributes['facsimileTelephoneNumber'][0];
			$faxNumberField = new htmlInputField('inetOrgPerson_faxNumber', $faxNumber);
			if (in_array('faxNumber', $readOnlyFields)) {
				$faxNumberField = new htmlOutputText($faxNumber);
			}
			$return['faxNumber'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('faxNumber', _('Fax number'))), $faxNumberField
			);
		}
		if (in_array('pager', $fields)) {
			$pager = '';
			if (isset($attributes['pager'][0])) $pager = $attributes['pager'][0];
			$pagerField = new htmlInputField('inetOrgPerson_pager', $pager);
			if (in_array('pager', $readOnlyFields)) {
				$pagerField = new htmlOutputText($pager);
			}
			$return['pager'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('pager', _('Pager'))), $pagerField
			);
		}
		if (in_array('street', $fields)) {
			$street = '';
			if (isset($attributes['street'][0])) $street = $attributes['street'][0];
			$streetField = new htmlInputField('inetOrgPerson_street', $street);
			if (in_array('street', $readOnlyFields)) {
				$streetField = new htmlOutputText($street);
			}
			$return['street'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('street', _('Street'))), $streetField
			);
		}
		if (in_array('postalAddress', $fields)) {
			$postalAddress = '';
			if (isset($attributes['postalAddress'][0])) $postalAddress = $attributes['postalAddress'][0];
			$postalAddressField = new htmlInputField('inetOrgPerson_postalAddress', $postalAddress);
			if (in_array('postalAddress', $readOnlyFields)) {
				$postalAddressField = new htmlOutputText($postalAddress);
			}
			$return['postalAddress'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('postalAddress', _('Postal address'))), $postalAddressField
			);
		}
		if (in_array('registeredAddress', $fields)) {
			$registeredAddress = '';
			if (isset($attributes['registeredAddress'][0])) $registeredAddress = $attributes['registeredAddress'][0];
			$registeredAddressField = new htmlInputField('inetOrgPerson_registeredAddress', $registeredAddress);
			if (in_array('registeredAddress', $readOnlyFields)) {
				$registeredAddressField = new htmlOutputText($registeredAddress);
			}
			$return['registeredAddress'] = new htmlResponsiveRow(
				new htmlOutputText(_('Registered address')), $registeredAddressField
			);
		}
		if (in_array('postalCode', $fields)) {
			$postalCode = '';
			if (isset($attributes['postalCode'][0])) $postalCode = $attributes['postalCode'][0];
			$postalCodeField = new htmlInputField('inetOrgPerson_postalCode', $postalCode);
			if (in_array('postalCode', $readOnlyFields)) {
				$postalCodeField = new htmlOutputText($postalCode);
			}
			$return['postalCode'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('postalCode', _('Postal code'))), $postalCodeField
			);
		}
		if (in_array('postOfficeBox', $fields)) {
			$postOfficeBox = '';
			if (isset($attributes['postOfficeBox'][0])) $postOfficeBox = $attributes['postOfficeBox'][0];
			$postOfficeBoxField = new htmlInputField('inetOrgPerson_postOfficeBox', $postOfficeBox);
			if (in_array('postOfficeBox', $readOnlyFields)) {
				$postOfficeBoxField = new htmlOutputText($postOfficeBox);
			}
			$return['postOfficeBox'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('postOfficeBox', _('Post office box'))), $postOfficeBoxField
			);
		}
		if (in_array('roomNumber', $fields)) {
			$roomNumber = '';
			if (isset($attributes['roomNumber'][0])) $roomNumber = $attributes['roomNumber'][0];
			$roomNumberField = new htmlInputField('inetOrgPerson_roomNumber', $roomNumber);
			if (in_array('roomNumber', $readOnlyFields)) {
				$roomNumberField = new htmlOutputText($roomNumber);
			}
			$return['roomNumber'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('roomNumber', _('Room number'))), $roomNumberField
			);
		}
		if (in_array('location', $fields)) {
			$l = '';
			if (isset($attributes['l'][0])) $l = $attributes['l'][0];
			$lField = new htmlInputField('inetOrgPerson_location', $l);
			if (in_array('location', $readOnlyFields)) {
				$lField = new htmlOutputText($l);
			}
			$return['location'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('location', _('Location'))), $lField
			);
		}
		if (in_array('state', $fields)) {
			$st = '';
			if (isset($attributes['st'][0])) $st = $attributes['st'][0];
			$stField = new htmlInputField('inetOrgPerson_state', $st);
			if (in_array('state', $readOnlyFields)) {
				$stField = new htmlOutputText($st);
			}
			$return['state'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('state', _('State'))), $stField
			);
		}
		if (in_array('carLicense', $fields)) {
			$carLicense = '';
			if (isset($attributes['carLicense'][0])) $carLicense = $attributes['carLicense'][0];
			$carLicenseField = new htmlInputField('inetOrgPerson_carLicense', $carLicense);
			if (in_array('carLicense', $readOnlyFields)) {
				$carLicenseField = new htmlOutputText($carLicense);
			}
			$return['carLicense'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('carLicense', _('Car license'))), $carLicenseField
			);
		}
		if (in_array('officeName', $fields)) {
			$physicalDeliveryOfficeName = '';
			if (isset($attributes['physicalDeliveryOfficeName'][0])) $physicalDeliveryOfficeName = $attributes['physicalDeliveryOfficeName'][0];
			$physicalDeliveryOfficeNameField = new htmlInputField('inetOrgPerson_officeName', $physicalDeliveryOfficeName);
			if (in_array('officeName', $readOnlyFields)) {
				$physicalDeliveryOfficeNameField = new htmlOutputText($physicalDeliveryOfficeName);
			}
			$return['officeName'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('officeName', _('Office name'))), $physicalDeliveryOfficeNameField
			);
		}
		if (in_array('businessCategory', $fields)) {
			$businessCategory = '';
			if (isset($attributes['businessCategory'][0])) $businessCategory = $attributes['businessCategory'][0];
			$businessCategoryField = new htmlInputField('inetOrgPerson_businessCategory', $businessCategory);
			if (in_array('businessCategory', $readOnlyFields)) {
				$businessCategoryField = new htmlOutputText($businessCategory);
			}
			$return['businessCategory'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('businessCategory', _('Business category'))), $businessCategoryField
			);
		}
		if (in_array('jpegPhoto', $fields)) {
			if (isset($attributes['jpegPhoto'][0])) {
				$jpeg_filename = 'jpegPhoto' . session_id() . '.jpg';
				$outjpeg = fopen(realpath('../../') . '/tmp/' . $jpeg_filename, "wb");
				fwrite($outjpeg, $attributes['jpegPhoto'][0]);
				fclose ($outjpeg);
				$photoFile = '../../tmp/' . $jpeg_filename;
				$photoSub = new htmlTable();
				$img = new htmlImage($photoFile);
				$img->setCSSClasses(array('photo'));
				$photoSub->addElement($img, true);
				if (!in_array('jpegPhoto', $readOnlyFields)) {
					$photoSubSub = new htmlTable();
					$upload = new htmlInputFileUpload('photoFile');
					$upload->colspan = 2;
					$photoSubSub->addElement($upload, true);
					$photoSubSub->addElement(new htmlTableExtendedInputCheckbox('removeReplacePhoto', false, _('Remove/replace photo'), null, false));
					$photoSub->addElement($photoSubSub);
				}
				$return['jpegPhoto'] = new htmlResponsiveRow(new htmlOutputText($this->getSelfServiceLabel('jpegPhoto', _('Photo'))), $photoSub);
			}
			elseif (!in_array('jpegPhoto', $readOnlyFields)) {
				$photoSub = new htmlTable();
				$photoSub->addElement(new htmlTableExtendedInputFileUpload('photoFile', _('Add photo')));
				$photoSub->addElement(new htmlHiddenInput('addPhoto', 'true'));
				$return['jpegPhoto'] = new htmlResponsiveRow(new htmlOutputText($this->getSelfServiceLabel('jpegPhoto', _('Photo'))), $photoSub);
			}
		}
		if (in_array('departmentNumber', $fields)) {
			$departmentNumber = '';
			if (isset($attributes['departmentNumber'][0])) $departmentNumber = implode('; ', $attributes['departmentNumber']);
			$departmentNumberField = new htmlInputField('inetOrgPerson_departmentNumber', $departmentNumber);
			if (in_array('departmentNumber', $readOnlyFields)) {
				$departmentNumberField = new htmlOutputText($departmentNumber);
			}
			$return['departmentNumber'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('departmentNumber', _('Department'))), $departmentNumberField
			);
		}
		if (in_array('initials', $fields)) {
			$initials = '';
			if (isset($attributes['initials'][0])) $initials = implode('; ', $attributes['initials']);
			$initialsField = new htmlInputField('inetOrgPerson_initials', $initials);
			if (in_array('initials', $readOnlyFields)) {
				$initialsField = new htmlOutputText($initials);
			}
			$return['initials'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('initials', _('Initials'))), $initialsField
			);
		}
		if (in_array('title', $fields)) {
			$title = '';
			if (isset($attributes['title'][0])) $title = $attributes['title'][0];
			$titleField = new htmlInputField('inetOrgPerson_title', $title);
			if (in_array('title', $readOnlyFields)) {
				$titleField = new htmlOutputText($title);
			}
			$return['title'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('title', _('Job title'))), $titleField
			);
		}
		if (in_array('userCertificate', $fields)) {
			$userCertificates = array();
			if (isset($attributes['userCertificate'][0])) {
				$userCertificates = $attributes['userCertificate'];
			}
			elseif (isset($attributes['userCertificate;binary'][0])) {
				$userCertificates = $attributes['userCertificate;binary'];
			}
			$_SESSION[self::SESS_CERTIFICATES_LIST] = $userCertificates;
			$certTable = new htmlResponsiveRow();
			$certTable->add(new htmlDiv('userCertificateDiv', $this->getSelfServiceUserCertificates()), 12);
			// JavaScript functions
			$certTable->add($this->getSelfServiceUserCertificatesJSBlock(), 12);
			// upload button
			$uploadButtons = new htmlGroup();
			$uploadButtons->addElement(new htmlDiv('inetOrgPersonCertUploadId', new htmlOutputText('')), true);
			$certUpload = new htmlJavaScript('inetOrgPersonUploadCert(\'inetOrgPersonCertUploadId\');');
			$uploadButtons->addElement($certUpload);
			$certTable->add($uploadButtons, 12);
			// upload status
			$uploadStatus = new htmlDiv('inetOrgPerson_upload_status_cert', new htmlOutputText(''));
			$uploadStatus->setCSSClasses(array('qq-upload-list'));
			$uploadStatus->colspan = 7;
			$certTable->add($uploadStatus, 12);
			$certLabel = new htmlOutputText($this->getSelfServiceLabel('userCertificate', _('User certificates')));
			$return['userCertificate'] = new htmlResponsiveRow($certLabel, $certTable);
		}
		// o
		if (in_array('o', $fields)) {
			$o = '';
			if (isset($attributes['o'][0])) $o = $attributes['o'][0];
			if (in_array('o', $readOnlyFields)) {
				$oField = new htmlOutputText(getAbstractDN($o));
			}
			else {
				$filter = '(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container))';
				$suffix = $_SESSION['selfServiceProfile']->LDAPSuffix;
				$foundOs = searchLDAPPaged($_SESSION['ldapHandle'], $suffix, $filter, array('dn'), false, 0);
				$oList = array();
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
				$oSelectionListSelected = array();
				if (!empty($attributes['o'][0])) {
					$oSelectionListSelected[] = $attributes['o'][0];
				}
				$oField = new htmlSelect('inetOrgPerson_o', $oSelectionList, $oSelectionListSelected);
				$oField->setHasDescriptiveElements(true);
				$oField->setRightToLeftTextDirection(true);
				$oField->setSortElements(false);
			}
			$return['o'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('o', _('Organisation'))), $oField
			);
		}
		// ou
		if (in_array('ou', $fields)) {
			$ou = '';
			if (isset($attributes['ou'][0])) $ou = $attributes['ou'][0];
			if (in_array('ou', $readOnlyFields)) {
				$ouField = new htmlOutputText(getAbstractDN($ou));
			}
			else {
				$filter = '(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container))';
				$suffix = $_SESSION['selfServiceProfile']->LDAPSuffix;
				$foundOus = searchLDAPPaged($_SESSION['ldapHandle'], $suffix, $filter, array('dn'), false, 0);
				$ouList = array();
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
				$ouSelectionListSelected = array();
				if (!empty($attributes['ou'][0])) {
					$ouSelectionListSelected[] = $attributes['ou'][0];
				}
				$ouField = new htmlSelect('inetOrgPerson_ou', $ouSelectionList, $ouSelectionListSelected);
				$ouField->setHasDescriptiveElements(true);
				$ouField->setRightToLeftTextDirection(true);
				$ouField->setSortElements(false);
			}
			$return['ou'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('ou', _('Organisational unit'))), $ouField
			);
		}
		// description
		if (in_array('description', $fields)) {
			$description = '';
			if (isset($attributes['description'][0])) $description = $attributes['description'][0];
			$descriptionField = new htmlInputField('inetOrgPerson_description', $description);
			if (in_array('description', $readOnlyFields)) {
				$descriptionField = new htmlOutputText($description);
			}
			$return['description'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('description', _('Description'))), $descriptionField
			);
		}
		// uid
		if (in_array('uid', $fields)) {
			$uid = '';
			if (isset($attributes['uid'][0])) $uid = $attributes['uid'][0];
			$uidField = new htmlInputField('inetOrgPerson_uid', $uid);
			if (in_array('uid', $readOnlyFields)) {
				$uidField = new htmlOutputText($uid);
			}
			$return['uid'] = new htmlResponsiveRow(
				new htmlOutputText($this->getSelfServiceLabel('uid', _('User name'))), $uidField
			);
		}
		return $return;
	}

	/**
	 * Returns the meta HTML code to display the certificate area.
	 * This also includes the file upload.
	 *
	 * @return htmlTable certificate content
	 */
	private function getSelfServiceUserCertificates() {
		$userCertificates = $_SESSION[self::SESS_CERTIFICATES_LIST];
		$content = new htmlGroup();
		if (sizeof($userCertificates) > 0) {
			$certTable = new htmlResponsiveRow();
			for ($i = 0; $i < sizeof($userCertificates); $i++) {
				$group = new htmlGroup();
				$filename = 'userCertificate' . getRandomNumber() . '.der';
				$out = @fopen(dirname(__FILE__) . '/../../tmp/' . $filename, "wb");
				fwrite($out, $userCertificates[$i]);
				fclose ($out);
				$path = '../../tmp/' . $filename;
				if (function_exists('openssl_x509_parse')) {
					$pem = @chunk_split(@base64_encode($userCertificates[$i]), 64, "\n");
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
							$group->addElement(new htmlOutputText(implode(': ', $data)));
							$group->addElement(new htmlSpacer('5px', null));
						}
					}
				}
				$saveLink = new htmlLink('', $path, '../../graphics/save.png');
				$saveLink->setTitle(_('Save'));
				$saveLink->setTargetWindow('_blank');
				$group->addElement($saveLink);
				$delLink = new htmlLink('', '#', '../../graphics/del.png');
				$delLink->setTitle(_('Delete'));
				$delLink->setOnClick('inetOrgPersonDeleteCertificate(' . $i . '); return false;');
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
					"action": "delete",
					"id": id
				};
				jQuery.post(\'../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user'
				 . '&' . getSecurityTokenName() . '=' . getSecurityTokenValue()
				 . '\', {jsonInput: actionJSON}, function(data) {inetOrgPersonDeleteCertificateHandleReply(data);}, \'json\');
			}

			function inetOrgPersonDeleteCertificateHandleReply(data) {
				if (data.errorsOccured == "false") {
					jQuery(\'#userCertificateDiv\').html(data.html);
				}
				else {
					alert(data.errormessage);
				}
			}

			function inetOrgPersonUploadCert(elementID) {
				var uploadStatus = document.getElementById(\'inetOrgPerson_upload_status_cert\');
				var uploader = new qq.FineUploader({
					element: document.getElementById(elementID),
					listElement: uploadStatus,
					request: {
						endpoint: \'../misc/ajax.php?selfservice=1&module=inetOrgPerson&scope=user'
						 . '&' . getSecurityTokenName() . '=' . getSecurityTokenValue() . '\',
						forceMultipart: true,
						params: {
							action: \'ajaxCertUpload\'
						}
					},
					multiple: false,
					callbacks: {
						onComplete: function(id, fileName, data) {
							if (data.success) {
								if (data.html) {
									jQuery(\'#userCertificateDiv\').html(data.html);
								}
							}
							else {
								alert(data.error);
							}
						}
					}
				});
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
		// pager
		if (in_array('pager', $fields) && !in_array('pager', $readOnlyFields)) {
			$attributeNames[] = 'pager';
			if (isset($_POST['inetOrgPerson_pager']) && ($_POST['inetOrgPerson_pager'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_pager'], 'telephone')) $return['messages'][] = $this->messages['pager'][0];
				else $attributesNew['pager'][0] = $_POST['inetOrgPerson_pager'];
			}
			elseif (isset($attributes['pager'])) unset($attributesNew['pager']);
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
			// remove photo
			if (isset($_POST['removeReplacePhoto']) && ($_POST['removeReplacePhoto'] == 'on')
					&& (empty($_FILES['replacePhotoFile']) || ($_FILES['replacePhotoFile']['size'] == 0))) {
				$return['mod']['jpegPhoto'] = array();
			}
			// set/replace photo
			if (isset($_FILES['photoFile']) && ($_FILES['photoFile']['size'] > 0)) {
				$moduleSettings = $this->selfServiceSettings->moduleSettings;
				if (!empty($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]) && ($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0] < ($_FILES['photoFile']['size'] / 1024))) {
					$msg =  $this->messages['file'][3];
					$msg[] = null;
					$msg[] = htmlspecialchars($moduleSettings['inetOrgPerson_jpegPhoto_maxSize'][0]);
					$return['messages'][] = $msg;
				}
				else {
					$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
					$data = fread($handle, 100000000);
					fclose($handle);
					if (extension_loaded('imagick')) {
						try {
							$data = inetOrgPerson::resizeAndConvertImage($data, $moduleSettings);
						}
						catch (Exception $e) {
							$msg =  $this->messages['file'][2];
							$msg[] = htmlspecialchars($e->getMessage());
							$return['messages'][] = $msg;
						}
					}
					if (isset($_POST['removeReplacePhoto']) && ($_POST['removeReplacePhoto'] == 'on')) {
						$return['mod']['jpegPhoto'][0] = $data;
					}
					elseif (isset($_POST['addPhoto'])) {
						$return['add']['jpegPhoto'][0] = $data;
					}
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
		// description
		if (in_array('description', $fields) && !in_array('description', $readOnlyFields)) {
			$attributeNames[] = 'description';
			if (!empty($_POST['inetOrgPerson_description'])) {
				$attributesNew['description'][0] = $_POST['inetOrgPerson_description'];
			}
			elseif (isset($attributes['description'])) unset($attributesNew['description']);
		}
		// ou
		if (in_array('ou', $fields) && !in_array('ou', $readOnlyFields)) {
			$attributeNames[] = 'ou';
			if (!empty($_POST['inetOrgPerson_ou'])) {
				$attributesNew['ou'][0] = $_POST['inetOrgPerson_ou'];
			}
			elseif (isset($attributes['ou'])) unset($attributesNew['ou']);
		}
		// o
		if (in_array('o', $fields) && !in_array('o', $readOnlyFields)) {
			$attributeNames[] = 'o';
			if (!empty($_POST['inetOrgPerson_o'])) {
				$attributesNew['o'][0] = $_POST['inetOrgPerson_o'];
			}
			elseif (isset($attributes['o'])) unset($attributesNew['o']);
		}
		// uid
		if (in_array('uid', $fields) && !in_array('uid', $readOnlyFields)) {
			$attributeNames[] = 'uid';
			if (isset($_POST['inetOrgPerson_uid']) && ($_POST['inetOrgPerson_uid'] != '')) {
				if (!get_preg($_POST['inetOrgPerson_uid'], 'username')) $return['messages'][] = $this->messages['uid'][0];
				else $attributesNew['uid'][0] = $_POST['inetOrgPerson_uid'];
			}
			elseif (isset($attributes['uid'])) unset($attributesNew['uid']);
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
	 * Resizes the given image data to the settings provided.
	 *
	 * @param array $data binary image data
	 * @param array $settings settings
	 * @return array binary image data
	 */
	private static function resizeAndConvertImage($data, $settings) {
		if (extension_loaded('imagick')) {
			// convert to JPG if imagick extension is available
			$image = new Imagick();
			$image->readImageBlob($data);
			// resize if maximum values specified
			if (!empty($settings['inetOrgPerson_jpegPhoto_maxWidth'][0]) || !empty($settings['inetOrgPerson_jpegPhoto_maxHeight'][0])) {
				$maxWidth = empty($settings['inetOrgPerson_jpegPhoto_maxWidth'][0]) ? $image->getimagewidth() : $settings['inetOrgPerson_jpegPhoto_maxWidth'][0];
				$maxHeight = empty($settings['inetOrgPerson_jpegPhoto_maxHeight'][0]) ? $image->getimageheight() : $settings['inetOrgPerson_jpegPhoto_maxHeight'][0];
				$image->thumbnailimage($maxWidth, $maxHeight, true);
			}
			$image->setImageCompression(Imagick::COMPRESSION_JPEG);
			$image->setImageFormat('jpeg');
			$data = $image->getimageblob();
		}
		return $data;
	}

	/**
	 * Manages AJAX requests.
	 * This function may be called with or without an account container.
	 */
	public function handleAjaxRequest() {
		// AJAX uploads are non-JSON
		if (isset($_GET['action']) && ($_GET['action'] == 'ajaxCertUpload')) {
			$this->ajaxUpload();
			return;
		}
		$jsonInput = $_POST['jsonInput'];
		$jsonReturn = self::invalidAjaxRequest();
		if (isset($jsonInput['action'])) {
			if ($jsonInput['action'] == 'delete') {
				$jsonReturn = $this->ajaxDeleteSelfServiceUserCertificate($jsonInput);
			}
		}
		echo json_encode($jsonReturn);
	}

	/**
	 * Handles an AJAX file upload and prints the JSON result.
	 */
	private function ajaxUpload() {
		$result = array('success' => true);
		if (!isset($_FILES['qqfile']) || ($_FILES['qqfile']['size'] < 100)) {
			$result = array('error' => _('No file received.'));
		}
		else {
			$handle = fopen($_FILES['qqfile']['tmp_name'], "r");
			$data = fread($handle, 100000000);
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
			$_SESSION[inetOrgPerson::SESS_CERTIFICATES_LIST][] = $data;
			ob_start();
			$contentElement = $this->getSelfServiceUserCertificates();
			ob_end_clean();
			ob_start();
			$tabindex = 999;
			parseHtml(null, $contentElement, array(), true, $tabindex, $this->get_scope());
			$content = ob_get_contents();
			ob_end_clean();
			$result['html'] = $content;
		}
		echo json_encode($result);
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
		$tabindex = 999;
		parseHtml(null, $contentElement, array(), true, $tabindex, $this->get_scope());
		$content = ob_get_contents();
		ob_end_clean();
		return array(
			'errorsOccured' => 'false',
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
		return array('errorsOccured' => 'true', 'errormessage' => $message);
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
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		// check password strength
		$user = empty($this->attributes['uid'][0]) ? null : $this->attributes['uid'][0];
		$additionalAttrs = array();
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
			$this->attributes['userPassword'][0] = '{SASL}' . $this->attributes['uid'][0];
		}
		// set normal password
		else {
			$this->attributes['userPassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		}
		return array();
	}

	/**
	 * Loads cached data from LDAP such as departmets etc.
	 */
	private function initCache() {
		if ($this->departmentCache != null) {
			return;
		}
		$attrs = array();
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
		$departments = array();
		$ous = array();
		$os = array();
		$titles = array();
		$employeeTypes = array();
		$businessCategories = array();
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
	* Returns a list of configuration options.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The field names are used as keywords to load and save settings.
	* We recommend to use the module name as prefix for them (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all active account modules and their scopes (module => array(scopes))
	* @return mixed htmlElement or array of htmlElement
	*
	* @see htmlElement
	*/
	public function get_configOptions($scopes, $allScopes) {
		$return = parent::get_configOptions($scopes, $allScopes);
		// configuration settings
		$configContainer = new htmlTable();
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
				$optionsSelected = array('SSHA');
				$hashOption = new htmlTable();
				$hashOption->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', getSupportedHashTypes(), $optionsSelected, _("Password hash type"), 'pwdHash'));
				$configContainer->addElement($hashOption, true);
			}
		}
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
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidePager', true, _('Pager'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEMailAddress', false, _('Email address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideJobTitle', false, _('Job title'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideCarLicense', false, _('Car license'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeType', false, _('Employee type'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideBusinessCategory', false, _('Business category'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideDepartments', false, _('Department'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideManager', false, _('Manager'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideOu', false, _('Organisational unit'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideO', false, _('Organisation'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideEmployeeNumber', false, _('Employee number'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideInitials', false, _('Initials'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideLabeledURI', false, _('Web site'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideuserCertificate', false, _('User certificates'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hidejpegPhoto', false, _('Photo'), null, false));
		if (isset($_SESSION['conf_config'])) {
			$confActiveUnixUserModules = $_SESSION['conf_config']->get_AccountModules('user');
			// option to hide uid
			if (!in_array('posixAccount', $confActiveUnixUserModules)) {
				$configContainerOptions->addElement(new htmlOutputText(' '));
				$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_hideUID', false, _('User name'), null, false));
			}
		}
		$configContainer->addElement($configContainerOptions, true);
		$advancedOptions = new htmlTable();
		$addressbookTable = new htmlTable();
		$addressbookTable->addElement(new htmlTableExtendedInputCheckbox('inetOrgPerson_addAddressbook', false, _('Add addressbook (ou=addressbook)'), 'addAddressbook'));
		$advancedOptions->addElement($addressbookTable, true);
		$advancedOptions->addElement(new htmlSubTitle(_('Read-only fields')), true);
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
			$confActiveUnixUserModules = $_SESSION['conf_config']->get_AccountModules('user');
			$readOnlyOptions[_('User name')] = 'inetOrgPerson_readOnly_uid';
			$readOnlyOptions[_('Common name')] = 'inetOrgPerson_readOnly_cn';
		}
		ksort($readOnlyOptions);
		$readOnlyCounter = 0;
		$readOnlyOptionsTable = new htmlTable();
		$readOnlyOptionsTable->colspan = 5;
		foreach ($readOnlyOptions as $label => $id) {
			$readOnlyOptionsTable->addElement(new htmlTableExtendedInputCheckbox($id, false, $label, null, false));
			$readOnlyCounter++;
			if (($readOnlyCounter % 5) == 0) {
				$readOnlyOptionsTable->addNewLine();
			}
		}
		$advancedOptions->addElement($readOnlyOptionsTable, true);
		$advancedOptions->addElement(new htmlSubTitle(_('Photo')), true);
		$photoTable = new htmlTable();
		$photoTable->colspan = 2;
		if (extension_loaded('imagick')) {
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum width (px)'), 'inetOrgPerson_jpegPhoto_maxWidth', null, 'crop'), true);
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum height (px)'), 'inetOrgPerson_jpegPhoto_maxHeight', null, 'crop'), true);
		}
		$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum file size (kB)'), 'inetOrgPerson_jpegPhoto_maxSize'), true);
		$advancedOptions->addElement($photoTable, true);
		$advancedOptionsAccordion = new htmlAccordion('inetOrgPersonAdvancedOptions', array(_('Advanced options') => $advancedOptions), false);
		$advancedOptionsAccordion->colspan = 5;
		$configContainer->addElement($advancedOptionsAccordion);
		$return[] = $configContainer;
		return $return;
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
	 * @param string[] $modules active account mdules
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
	 * {@inheritdoc}
	 */
	public function getWildCardReplacements() {
		$replacements = array();
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

}

?>
