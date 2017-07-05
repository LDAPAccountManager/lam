<?php
use \LAM\TYPES\TypeManager;
use LAM\TYPES\ConfiguredType;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2017  Roland Gruber

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
* Manages Windows AD (e.g. Samba 4) users.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Windows AD (e.g. Samba 4) users.
*
* @package modules
*/
class windowsUser extends baseModule implements passwordService {

	/** initial account flags */
	const DEFAULT_ACCOUNT_CONTROL = 0x00000200;
	/** password never expires */
	const AC_PWD_NEVER_EXPIRES = 0x00010000;
	/** login requires smartcard */
	const AC_SMARTCARD_REQUIRED = 0x00040000;
	/** account is disabled */
	const AC_ACCOUNT_DISABLED = 0x00000002;

	/** current group of names list */
	private $groupList = array();
	/** original group of names list */
	private $groupList_orig = array();
	/** cache for groups */
	private $groupCache = null;
	/** option for forcing password change, used in postModifyActions */
	private $pwdLastSet = null;
	/** clear text password */
	private $clearTextPassword;
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

	/** cache for lockout duration */
	private static $lockoutDurationCache = array();


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
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// this is a base module
		$return["is_base"] = true;
		// PHP extensions
		$return['extensions'] = array('iconv');
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('and' => '(!(objectClass=computer))', 'or' => "(objectClass=user)");
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('user', 'securityPrincipal');
		// managed attributes
		$return['attributes'] = array('userPrincipalName', 'cn', 'sAMAccountName', 'description', 'displayName', 'givenName', 'initials',
			'l', 'mail', 'otherTelephone', 'physicalDeliveryOfficeName', 'postalCode', 'postOfficeBox', 'sn', 'st',
			'streetAddress', 'telephoneNumber', 'url', 'wWWHomePage', 'userAccountControl', 'profilePath', 'scriptPath',
			'pwdLastSet', 'otherMailbox', 'homeDirectory', 'homeDrive', 'msSFU30Name', 'msSFU30NisDomain', 'pwdLastSet',
			'lastLogonTimestamp', 'accountExpires', 'jpegPhoto', 'title', 'carLicense', 'employeeNumber', 'employeeType',
			'businessCategory', 'department', 'ou', 'o', 'manager', 'facsimileTelephoneNumber', 'company',
			'pager', 'otherPager', 'mobile', 'otherMobile', 'proxyAddresses', 'lockoutTime'
		);
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _('Common name'), 'attr' => 'cn',
				"Text" => _('This is the natural name of the user. If empty, the first and last name or user name is used.')
			),
			'userPrincipalName' => array(
				"Headline" => _('User name'), 'attr' => 'userPrincipalName',
				"Text" => _('Please enter the user\'s name.')
			),
			'userPrincipalNameDomain' => array(
				"Headline" => _('Domain'), 'attr' => 'userPrincipalName',
				"Text" => _('Windows domain name of account.')
			),
			'sAMAccountName' => array(
				"Headline" => _('User name (pre W2K)'), 'attr' => 'sAMAccountName',
				"Text" => _('Please enter the user\'s name.') . ' ' . _('This user name is only used for old Windows versions (e.g. NT4, W98).')
			),
			'description' => array(
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Please enter a descriptive text for this user.')
			),
			'displayName' => array(
				"Headline" => _('Display name'), 'attr' => 'displayName',
				"Text" => _('This is the account\'s full name on Windows systems.')
			),
			'givenName' => array(
				"Headline" => _('First name'), 'attr' => 'givenName',
				"Text" => _('First name of user. Only letters, - and spaces are allowed.')
			),
			'initials' => array(
				"Headline" => _('Initials'), 'attr' => 'initials',
				"Text" => _('The initials of the user\'s first names.')
			),
			'l' => array(
				"Headline" => _('Location'), 'attr' => 'l',
				"Text" => _('This describes the location of the user.')
			),
			'mail' => array(
				"Headline" => _('Email address'), 'attr' => 'mail',
				"Text" => _('The user\'s email address.')
			),
			'otherTelephone' => array(
				"Headline" => _('Other telephone numbers'), 'attr' => 'otherTelephone',
				"Text" => _('If the user has multiple telephone numbers then please enter it here.')
			),
			'otherTelephoneList' => array(
				"Headline" => _('Other telephone numbers'), 'attr' => 'otherTelephone',
				"Text" => _('If the user has multiple telephone numbers then please enter it here.') . ' ' . _("Multiple values are separated by semicolon.")
			),
			'physicalDeliveryOfficeName' => array(
				"Headline" => _('Office name'), 'attr' => 'physicalDeliveryOfficeName',
				"Text" => _('The office name of the user (e.g. YourCompany, Human Resources).')
			),
			'postalCode' => array(
				"Headline" => _('Postal code'), 'attr' => 'postalCode',
				"Text" => _('The postal code of the user\'s address.')
			),
			'postOfficeBox' => array(
				"Headline" => _('Post office box'), 'attr' => 'postOfficeBox',
				"Text" => _('The post office box of the user\'s address.')
			),
			'sn' => array(
				"Headline" => _('Last name'), 'attr' => 'sn',
				"Text" => _('Last name of user. Only letters, - and spaces are allowed.')
			),
			'st' => array(
				"Headline" => _('State'), 'attr' => 'st',
				"Text" => _('The state where the user resides or works.')
			),
			'streetAddress' => array(
				"Headline" => _('Street'), 'attr' => 'streetAddress',
				"Text" => _('The street name of the user\'s address.')
			),
			'telephoneNumber' => array(
				"Headline" => _('Telephone number'), 'attr' => 'telephoneNumber',
				"Text" => _('The user\'s telephone number.')
			),
			'facsimileTelephoneNumber' => array (
				"Headline" => _("Fax number"), 'attr' => 'facsimileTelephoneNumber',
				"Text" => _("The user's fax number.")
			),
			'url' => array(
				"Headline" => _('Other web sites'), 'attr' => 'url',
				"Text" => _('Here you can enter additional web sites for the user.')
			),
			'urlList' => array(
				"Headline" => _('Other web sites'), 'attr' => 'url',
				"Text" => _('Here you can enter additional web sites for the user.') . ' ' . _("Multiple values are separated by semicolon.")
			),
			'wWWHomePage' => array(
				"Headline" => _('Web site'), 'attr' => 'wWWHomePage',
				"Text" => _('The user\'s web site (e.g. http://www.company.com).')
			),
			"deactivated" => array(
				"Headline" => _("Account is deactivated"),
				"Text" => _("If checked then the account will be deactivated.")),
			"noExpire" => array(
				"Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire.")),
			"requireCard" => array(
				"Headline" => _("Require smartcard"),
				"Text" => _("The user must log on using a smart card.")),
			"profilePath" => array(
				"Headline" => _("Profile path"), 'attr' => 'profilePath',
				"Text" => _('Path of the user profile (UNC-path, e.g. \\\\server\\share\\user).'). ' '. _("Can be left empty.")
			),
			"scriptPath" => array(
				"Headline" => _("Logon script"), 'attr' => 'scriptPath',
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon.'). ' '. _("Can be left empty.")
			),
			"pwdMustChange" => array (
				"Headline" => _("Password change at next login"),
				"Text" => _("If you set this option then the user has to change his password at the next login.")),
			"groups" => array (
				"Headline" => _("Groups"),
				"Text" => _("Hold the CTRL-key to (de)select multiple groups.")),
			"groupsUpload" => array (
				"Headline" => _("Groups"),
				"Text" => _("The groups for this account. You can insert a group name or DN.") . ' ' . _("Multiple values are separated by semicolon.")),
			'password' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
			),
			'otherMailbox' => array(
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.")
			),
			'otherMailboxList' => array(
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'domains' => array(
				"Headline" => _('Domains'),
				"Text" => _('Please enter a list of Windows domains that can be selected for your user accounts.')
			),
			"homeDrive" => array(
				"Headline" => _("Home drive"), 'attr' => 'homeDrive',
				"Text" => _("The home directory will be connected under this drive letter.")),
			"homeDirectory" => array(
				"Headline" => _("Home directory"), 'attr' => 'homeDirectory',
				"Text" => _('UNC-path (\\\\server\\share\) of home directory. If no home drive is set then this directory must start with a drive letter (e.g. "c:\dir\user").')
			),
			'msSFU30Name' => array(
				"Headline" => _('NIS name'), 'attr' => 'msSFU30Name',
				"Text" => _('User name for NIS.')
			),
			'msSFU30NisDomain' => array(
				"Headline" => _('NIS domain'), 'attr' => 'msSFU30NisDomain',
				"Text" => _('NIS domain name.')
			),
			'pwdLastSet' => array(
				"Headline" => _('Last password change'), 'attr' => 'pwdLastSet',
				"Text" => _('Time of user\'s last password change.')
			),
			'lastLogonTimestamp' => array(
				"Headline" => _('Last login'), 'attr' => 'lastLogonTimestamp',
				"Text" => _('Time of user\'s last login.')
			),
			'accountExpires' => array(
				"Headline" => _('Account expiration date'), 'attr' => 'accountExpires',
				"Text" => _('This is the date when the account will expire.')
			),
			'accountExpiresUpload' => array(
				"Headline" => _('Account expiration date'), 'attr' => 'accountExpires',
				"Text" => _('This is the date when the account will expire. Format: DD-MM-YYYY')
			),
			'accountExpiresProfile' => array(
				"Headline" => _('Account expiration'), 'attr' => 'accountExpires',
				"Text" => _('Number of days after which the account will expire.')
			),
			'jpegPhoto' => array(
				"Headline" => _("Add photo"), 'attr' => 'jpegPhoto',
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			),
			'title' => array (
				"Headline" => _("Job title"), 'attr' => 'title',
				"Text" => _("Job title of user: President, department manager, ...")
			),
			'carLicense' => array(
				"Headline" => _("Car license"), 'attr' => 'carLicense',
				"Text" => _("This can be used to specify if the user has a car license.")
			),
			'employeeNumber' => array(
				"Headline" => _("Employee number"), 'attr' => 'employeeNumber',
				"Text" => _("The user's unique employee number.")
			),
			'employeeType' => array (
				"Headline" => _("Employee type"), 'attr' => 'employeeType',
				"Text" => _("Employee type: Contractor, Employee, Intern, Temp, External, ...")
			),
			'businessCategory' => array(
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)")
			),
			'businessCategoryList' => array(
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Manangement, ...)") . '. ' . _("Multiple values are separated by semicolon.")
			),
			'department' => array(
				"Headline" => _("Department"), 'attr' => 'department',
				"Text" => _("Here you can enter the user's department.")
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
			'manager' => array (
				"Headline" => _("Manager"), 'attr' => 'manager',
				"Text" => _("This is the LDAP DN of the user's manager. Use this property to represent hierarchies in your company.")
			),
			'company' => array (
				"Headline" => _('Company'), 'attr' => 'company',
				"Text" => _('Please enter the company name.')
			),
			'pager' => array (
				"Headline" => _('Pager'), 'attr' => 'pager',
				"Text" => _('The user\'s pager number.')
			),
			'otherPager' => array (
				"Headline" => _('Other pagers'), 'attr' => 'otherPager',
				"Text" => _('Any secondary pager numbers.')
			),
			'otherPagerUpload' => array (
				"Headline" => _('Other pagers'), 'attr' => 'otherPager',
				"Text" => _('Any secondary pager numbers.') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'mobile' => array (
				"Headline" => _('Mobile'), 'attr' => 'mobile',
				"Text" => _('The user\'s mobile number.')
			),
			'otherMobile' => array (
				"Headline" => _('Other mobiles'), 'attr' => 'otherMobile',
				"Text" => _('Any secondary mobile numbers.')
			),
			'otherMobileUpload' => array (
				"Headline" => _('Other mobiles'), 'attr' => 'otherMobile',
				"Text" => _('Any secondary mobile numbers.') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'proxyAddresses' => array (
				"Headline" => _('Proxy-Addresses'), 'attr' => 'proxyAddresses',
				"Text" => _('Use this to enter additional email addresses in format "smtp:user@domain.com".')
			),
			'proxyAddressesUpload' => array (
				"Headline" => _('Proxy-Addresses'), 'attr' => 'proxyAddresses',
				"Text" => _('Use this to enter additional email addresses in format "smtp:user@domain.com".') . ' ' . _('Multiple values are separated by semicolon.')
			),
			'crop' => array(
				"Headline" => _('Image cropping'),
				"Text" => _('Uploaded images will be cropped to these maximum values.')
			),
			'photoUpload' => array(
				"Headline" => _("Add photo"), 'attr' => 'jpegPhoto',
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'windowsUser_userPrincipalName',
				'description' => _('User name'),
				'help' => 'userPrincipalName',
				'example' => _('smiller'),
				'required' => true,
				'unique' => true,
			),
			array(
				'name' => 'windowsUser_password',
				'description' => _('Password'),
				'help' => 'password',
				'example' => _('secret'),
			),
			array(
				'name' => 'windowsUser_firstName',
				'description' => _('First name'),
				'help' => 'givenName',
				'example' => _('Steve'),
			),
			array(
				'name' => 'windowsUser_lastName',
				'description' => _('Last name'),
				'help' => 'sn',
				'example' => _('Miller'),
			),
			array(
				'name' => 'windowsUser_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller'),
			),
			array(
				'name' => 'windowsUser_displayName',
				'description' => _('Display name'),
				'help' => 'displayName',
				'example' => _('Steve Miller'),
			),
			array(
				'name' => 'windowsUser_initials',
				'description' => _('Initials'),
				'help' => 'initials',
				'example' => _('S.M.'),
			),
			array(
				'name' => 'windowsUser_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Temp, contract till December'),
			),
			array(
				'name' => 'windowsUser_streetAddress',
				'description' => _('Street'),
				'help' => 'streetAddress',
				'example' => _('Mystreetname 42'),
			),
			array(
				'name' => 'windowsUser_postOfficeBox',
				'description' => _('Post office box'),
				'help' => 'postOfficeBox',
				'example' => _('12345'),
			),
			array(
				'name' => 'windowsUser_postalCode',
				'description' => _('Postal code'),
				'help' => 'postalCode',
				'example' => _('GB-12345'),
			),
			array(
				'name' => 'windowsUser_l',
				'description' => _('Location'),
				'help' => 'l',
				'example' => _('MyCity'),
			),
			array(
				'name' => 'windowsUser_state',
				'description' => _('State'),
				'help' => 'st',
				'example' => _('New York'),
			),
			array(
				'name' => 'windowsUser_officeName',
				'description' => _('Office name'),
				'help' => 'physicalDeliveryOfficeName',
				'example' => _('YourCompany'),
			),
			array(
				'name' => 'windowsUser_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('user@company.com'),
			),
			array(
				'name' => 'windowsUser_otherMailbox',
				'description' => _('Email alias'),
				'help' => 'otherMailboxList',
				'example' => _('user@company.com'),
			),
			array(
				'name' => 'windowsUser_telephoneNumber',
				'description' => _('Telephone number'),
				'help' => 'telephoneNumber',
				'example' => _('123-124-1234'),
			),
			array(
				'name' => 'windowsUser_otherTelephone',
				'description' => _('Other telephone numbers'),
				'help' => 'otherTelephoneList',
				'example' => _('123-124-1234'),
			),
			array(
				'name' => 'windowsUser_webSite',
				'description' => _('Web site'),
				'help' => 'wWWHomePage',
				'example' => _('http://www.company.com'),
			),
			array(
				'name' => 'windowsUser_otherWebSites',
				'description' => _('Other web sites'),
				'help' => 'urlList',
				'example' => _('http://www.company.com'),
			),
			array(
				'name' => 'windowsUser_deactivated',
				'description' => _('Account is deactivated'),
				'help' => 'deactivated',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			),
			array(
				'name' => 'windowsUser_noExpire',
				'description' => _('Password does not expire'),
				'help' => 'noExpire',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			),
			array(
				'name' => 'windowsUser_accountExpires',
				'description' => _('Account expiration date'),
				'help' => 'accountExpiresUpload',
				'example' => '21-11-2030',
			),
			array(
				'name' => 'windowsUser_requireCard',
				'description' => _('Require smartcard'),
				'help' => 'requireCard',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			),
			array(
				'name' => 'windowsUser_pwdMustChange',
				'description' => _('Password change at next login'),
				'help' => 'pwdMustChange',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			),
			array(
				'name' => 'windowsUser_profilePath',
				'description' => _('Profile path'),
				'help' => 'profilePath',
				'example' => _('\\\\server\\profiles\\smiller'),
			),
			array(
				'name' => 'windowsUser_scriptPath',
				'description' => _('Logon script'),
				'help' => 'scriptPath',
				'example' => 'logon.bat',
			),
			array(
				'name' => 'windowsUser_homeDrive',
				'description' => _('Home drive'),
				'help' => 'homeDrive',
				'example' => 'K:'
			),
			array(
				'name' => 'windowsUser_homeDirectory',
				'description' => _('Home directory'),
				'help' => 'homeDirectory',
				'example' => _('\\\\server\\homes\\smiller')
			),
			array(
				'name' => 'windowsUser_groups',
				'description' => _('Groups'),
				'help' => 'groupsUpload',
			),
		);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_proxyAddresses',
				'description' => _('Proxy-Addresses'),
				'help' => 'proxyAddressesUpload',
				'example' => _('smtp:user@example.com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_facsimileTelephoneNumber',
				'description' => _('Fax number'),
				'help' => 'facsimileTelephoneNumber',
				'example' => _('123-123-1236')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
			$return['upload_columns'][] = array(
					'name' => 'windowsUser_pager',
					'description' => _('Pager'),
					'help' => 'pager',
					'example' => _('123-123-1236')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
			$return['upload_columns'][] = array(
					'name' => 'windowsUser_otherPager',
					'description' => _('Other pagers'),
					'help' => 'otherPagerUpload',
					'example' => _('123-123-1236')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
			$return['upload_columns'][] = array(
					'name' => 'windowsUser_mobile',
					'description' => _('Mobile'),
					'help' => 'mobile',
					'example' => _('123-123-1235')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
			$return['upload_columns'][] = array(
					'name' => 'windowsUser_otherMobile',
					'description' => _('Other mobiles'),
					'help' => 'otherMobileUpload',
					'example' => _('123-123-1235')
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$return['upload_columns'][] = array(
					'name' => 'windowsUser_company',
					'description' => _('Company'),
					'help' => 'company',
			);
		}

		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_sAMAccountName',
				'description' => _('User name (pre W2K)'),
				'help' => 'sAMAccountName',
				'example' => _('smiller'),
				'unique' => true,
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_msSFU30Name',
				'description' => _('NIS name'),
				'help' => 'msSFU30Name',
				'example' => _('smiller'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_msSFU30NisDomain',
				'description' => _('NIS domain'),
				'help' => 'msSFU30NisDomain',
				'example' => _('domain'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_title',
				'description' => _('Job title'),
				'help' => 'title',
				'example' => _('President'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_carLicense',
				'description' => _('Car license'),
				'help' => 'carLicense',
				'example' => _('yes'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_employeeNumber',
				'description' => _('Employee number'),
				'help' => 'employeeNumber',
				'example' => '123456',
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_employeeType',
				'description' => _('Employee type'),
				'help' => 'employeeType',
				'example' => _('Temp'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_businessCategory',
				'description' => _('Business category'),
				'help' => 'businessCategoryList',
				'example' => _('Administration'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_department',
				'description' => _('Department'),
				'help' => 'department',
				'example' => _('Administration'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_ou',
				'description' => _('Organisational unit'),
				'help' => 'ouList',
				'example' => _('Administration'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_o',
				'description' => _('Organisation'),
				'help' => 'oList',
				'example' => _('YourCompany'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsUser_manager',
				'description' => _('Manager'),
				'help' => 'manager',
				'example' => _('uid=smiller,ou=People,dc=company,dc=com'),
			);
		}
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Common name'), 'windowsUser_cn', null, 'cn'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Display name'), 'windowsUser_displayName', null, 'displayName'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Email address'), 'windowsUser_mail', null, 'mail'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Email alias'), 'windowsUser_otherMailbox', null, 'otherMailboxList'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Logon script'), 'windowsUser_scriptPath', null, 'scriptPath'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Profile path'), 'windowsUser_profilePath', null, 'profilePath'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Home directory'), 'windowsUser_homeDirectory', null, 'homeDirectory'), true);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('NIS domain'), 'windowsUser_msSFU30NisDomain', null, 'msSFU30NisDomain'), true);
		}
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Account expiration'), 'windowsUser_accountExpires', null, 'accountExpiresProfile'), true);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Company'), 'windowsUser_company', null, 'company'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Department'), 'windowsUser_department', null, 'department'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Organisational unit'), 'windowsUser_ou', null, 'ouList'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Organisation'), 'windowsUser_o', null, 'oList'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Job title'), 'windowsUser_title', null, 'title'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$profileContainer->addElement(new htmlTableExtendedInputField(_('Employee type'), 'windowsUser_employeeType', null, 'employeeType'), true);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
		    $profileContainer->addElement(new htmlTableExtendedInputField(_('Business category'), 'windowsUser_businessCategory', null, 'businessCategory'), true);
		}
		$return['profile_options'] = $profileContainer;
		// profile mappings
		$return['profile_mappings'] = array(
			'windowsUser_displayName' => 'displayName',
			'windowsUser_cn' => 'cn',
			'windowsUser_mail' => 'mail',
			'windowsUser_scriptPath' => 'scriptPath',
			'windowsUser_profilePath' => 'profilePath',
			'windowsUser_homeDirectory' => 'homeDirectory',
			'windowsUser_company' => 'company',
		);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$return['profile_mappings']['windowsUser_msSFU30NisDomain'] = 'msSFU30NisDomain';
		}
		// profile checks
		$return['profile_checks']['windowsUser_accountExpires'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['accountExpires'][0]);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$return['profile_checks']['windowsUser_title'] = array(
				'type' => 'ext_preg',
				'regex' => 'title',
				'error_message' => $this->messages['title'][0]);
			$return['profile_mappings']['windowsUser_title'] = 'title';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$return['profile_checks']['windowsUser_employeeType'] = array(
				'type' => 'ext_preg',
				'regex' => 'employeeType',
				'error_message' => $this->messages['employeeType'][0]);
			$return['profile_mappings']['windowsUser_employeeType'] = 'employeeType';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
		    $return['profile_checks']['windowsUser_businessCategory'] = array(
		    	'type' => 'ext_preg',
		    	'regex' => 'businessCategory',
		    	'error_message' => $this->messages['businessCategory'][0]);
		    $return['profile_mappings']['windowsUser_businessCategory'] = 'businessCategory';
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'userPrincipalName' => _('User name'),
			'cn' => _('Common name'),
			'description' => _('Description'),
			'displayName' => _('Display name'),
			'givenName' => _('First name'),
			'initials' => _('Initials'),
			'l' => _('Location'),
			'mail' => _('Email address'),
			'otherMailbox' => _('Email alias'),
			'otherTelephone' => _('Other telephone numbers'),
			'physicalDeliveryOfficeName' => _('Office name'),
			'postalCode' => _('Postal code'),
			'postOfficeBox' => _('Post office box'),
			'sn' => _('Last name'),
			'st' => _('State'),
			'streetAddress' => _('Street'),
			'telephoneNumber' => _('Telephone number'),
			'url' => _('Other web sites'),
			'wWWHomePage' => _('Web site'),
			'deactivated' => _('Account is deactivated'),
			'noExpire' => _('Password does not expire'),
			'requireCard' => _('Require smartcard'),
			'profilePath' => _('Profile path'),
			'scriptPath' => _('Logon script'),
			'pwdMustChange' => _('Password change at next login'),
			'groups' => _('Groups'),
			'password' => _('Password'),
			'homeDrive' => _('Home drive'),
			'homeDirectory' => _('Home directory'),
			'accountExpires' => _('Account expiration date'),
		);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
			$return['PDF_fields']['proxyAddresses'] = _('Proxy-Addresses');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$return['PDF_fields']['facsimileTelephoneNumber'] = _('Fax number');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			$return['PDF_fields']['sAMAccountName'] = _('User name (pre W2K)');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
			$return['PDF_fields']['msSFU30Name'] = _('NIS name');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$return['PDF_fields']['msSFU30NisDomain'] = _('NIS domain');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepwdLastSet')) {
			$return['PDF_fields']['pwdLastSet'] = _('Last password change');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidelastLogonTimestamp')) {
			$return['PDF_fields']['lastLogonTimestamp'] = _('Last login');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$return['PDF_fields']['title'] = _('Job title');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
			$return['PDF_fields']['carLicense'] = _('Car license');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
			$return['PDF_fields']['employeeNumber'] = _('Employee number');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$return['PDF_fields']['employeeType'] = _('Employee type');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$return['PDF_fields']['businessCategory'] = _('Business category');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$return['PDF_fields']['department'] = _('Department');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$return['PDF_fields']['ou'] = _('Organisational unit');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$return['PDF_fields']['o'] = _('Organisation');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
			$return['PDF_fields']['manager'] = _('Manager');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$return['PDF_fields']['company'] = _('Company');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
			$return['PDF_fields']['pager'] = _('Pager');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
			$return['PDF_fields']['otherPager'] = _('Other pagers');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
			$return['PDF_fields']['mobile'] = _('Mobile');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
			$return['PDF_fields']['otherMobile'] = _('Other mobiles');
		}
		// self service search attributes
		$return['selfServiceSearchAttributes'] = array('sAMAccountName', 'userPrincipalName',
				'employeeNumber'
		);
		// self service field settings
		$return['selfServiceFieldSettings'] = array(
			'physicalDeliveryOfficeName' => _('Office name'),
			'telephoneNumber' => _('Telephone number'),
			'facsimileTelephoneNumber' => _('Fax number'),
			'wWWHomePage' => _('Web site'),
			'streetAddress' => _('Street'),
			'st' => _('State'),
			'l' => _('Location'),
			'postOfficeBox' => _('Post office box'),
			'postalCode' => _('Postal code'),
			'unicodePwd' => _('Password'),
			'pwdLastSet' => _('Last password change (read-only)'),
			'accountExpires' => _('Account expiration date (read-only)'),
			'department' => _('Department'),
		);
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = array('physicalDeliveryOfficeName', 'telephoneNumber',
			'facsimileTelephoneNumber', 'wWWHomePage', 'streetAddress', 'st', 'l', 'postOfficeBox', 'postalCode');
		return $return;
	}

	/**
	* Initializes the module after it became part of an accountContainer
	*
	* @param string $base the name of the accountContainer object ($_SESSION[$base])
	*/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->groupList = array();
		$this->groupList_orig = array();
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['userPrincipalName'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['userPrincipalName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_userPrincipalName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][0] = array('ERROR', _('Common name'), _('Please enter a valid common name!'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_cn', _('Please enter a valid common name!'));
		$this->messages['sAMAccountName'][0] = array('ERROR', _('User name (pre W2K)'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['sAMAccountName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_sAMAccountName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['displayName'][0] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_displayName', _('Please enter a valid display name!'));
		$this->messages['givenName'][0] = array('ERROR', _('First name'), _('First name contains invalid characters!'));
		$this->messages['givenName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_givenName', _('First name contains invalid characters!'));
		$this->messages['sn'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters or is empty!'));
		$this->messages['sn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_sn', _('Last name contains invalid characters or is empty!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' windowsUser_telephone', _('Please enter a valid telephone number!'));
		$this->messages['facsimileTelephoneNumber'][0] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		$this->messages['facsimileTelephoneNumber'][1] = array('ERROR', _('Account %s:') . ' windowsUser_facsimileTelephoneNumber', _('Please enter a valid fax number!'));
		$this->messages['otherTelephone'][0] = array('ERROR', _('Other telephone numbers'), _('Please enter a valid telephone number!'));
		$this->messages['otherTelephone'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherTelephone', _('Please enter a valid telephone number!'));
		$this->messages['pager'][0] = array('ERROR', _('Pager'), _('Please enter a valid telephone number!'));
		$this->messages['pager'][1] = array('ERROR', _('Account %s:') . ' windowsUser_pager', _('Please enter a valid telephone number!'));
		$this->messages['otherPager'][0] = array('ERROR', _('Other pagers'), _('Please enter a valid telephone number!'));
		$this->messages['otherPager'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherPager', _('Please enter a valid telephone number!'));
		$this->messages['mobile'][0] = array('ERROR', _('Mobile'), _('Please enter a valid telephone number!'));
		$this->messages['mobile'][1] = array('ERROR', _('Account %s:') . ' windowsUser_mobile', _('Please enter a valid telephone number!'));
		$this->messages['otherMobile'][0] = array('ERROR', _('Other mobiles'), _('Please enter a valid telephone number!'));
		$this->messages['otherMobile'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherMobile', _('Please enter a valid telephone number!'));
		$this->messages['postalCode'][0] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		$this->messages['postalCode'][1] = array('ERROR', _('Account %s:') . ' windowsUser_postalCode', _('Please enter a valid postal code!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' windowsUser_mail', _('Please enter a valid email address!'));
		$this->messages['otherMailbox'][0] = array('ERROR', _('Email alias'), _('Email alias is invalid!'));
		$this->messages['otherMailbox'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherMailbox', _('Email alias is invalid!'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('ERROR', _('Account %s:') . ' windowsUser_profilePath', _('Profile path is invalid!'));
		$this->messages['scriptPath'][0] = array('ERROR', _('Logon script'), _('Logon script is invalid!'));
		$this->messages['scriptPath'][1] = array('ERROR', _('Account %s:') . ' windowsUser_scriptPath', _('Logon script is invalid!'));
		$this->messages['unicodePwd'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password fields.'));
		$this->messages['unicodePwd'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['homeDrive'][0] = array('ERROR', _('Account %s:') . ' windowsUser_homeDrive', _('Please enter a valid drive letter.'));
		$this->messages['homeDirectory'][0] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
		$this->messages['homeDirectory'][1] = array('ERROR', _('Account %s:') . ' windowsUser_homeDirectory', _('Homedirectory contains invalid characters.'));
		$this->messages['msSFU30Name'][0] = array('ERROR', _('NIS name'), _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['msSFU30Name'][1] = array('ERROR', _('Account %s:') . ' windowsUser_msSFU30Name', _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['accountExpires'][0] = array('ERROR', _('Account expiration'), _('Please enter a number.'));
		$this->messages['accountExpires'][1] = array('ERROR', _('Account %s:') . ' windowsUser_accountExpires', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['title'][0] = array('ERROR', _('Job title'), _('Please enter a valid job title!'));
		$this->messages['title'][1] = array('ERROR', _('Account %s:') . ' windowsUser_title', _('Please enter a valid job title!'));
		$this->messages['employeeType'][0] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		$this->messages['employeeType'][1] = array('ERROR', _('Account %s:') . ' windowsUser_employeeType', _('Please enter a valid employee type!'));
		$this->messages['businessCategory'][0] = array('ERROR', _('Business category'), _('Please enter a valid business category!'));
		$this->messages['businessCategory'][1] = array('ERROR', _('Account %s:') . ' windowsUser_businessCategory', _('Please enter a valid business category!'));
		$this->messages['manager'][0] = array('ERROR', _('Account %s:') . ' windowsUser_manager', _('This is not a valid DN!'));
		$this->messages['file'][0] = array('ERROR', _('No file selected.'));
		$this->messages['file'][1] = array('ERROR', _('Please upload a .jpg/.jpeg file.'));
		$this->messages['file'][2] = array('ERROR', _('Unable to process this file.'));
		$this->messages['file'][3] = array('ERROR', _('File is too large. Maximum allowed size is %s kB.'));
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// get group memberships
		$groupList = searchLDAPByAttribute('member', $this->getAccountContainer()->dn_orig, 'group', array('dn'), array('group'));
		$this->groupList_orig = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$this->groupList_orig[] = $groupList[$i]['dn'];
		}
		$this->groupList_orig = array_values(array_unique($this->groupList_orig));
		$this->groupList = $this->groupList_orig;
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
	public function save_attributes() {
		$return = parent::save_attributes();
		// add information about clear text password and password status change
		$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordClearText'][0] = $this->clearTextPassword;
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$this->initCache();
		$containerLeft = new htmlTable();
		$containerLeft->alignment = htmlElement::ALIGN_TOP;
		if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['userAccountControl'][0])) {
			$this->attributes['userAccountControl'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
		}
		$containerLeft->addElement(new htmlSubTitle(_('General')), true);
		// user name
		$userPrincipalName = '';
		$userPrincipalNameDomain = '';
		$domains = $this->getDomains();
		$domains[] = '';
		if (!empty($this->attributes['userPrincipalName'][0])) {
			$parts = explode('@', $this->attributes['userPrincipalName'][0]);
			$userPrincipalName = $parts[0];
			if (!empty($parts[1])) {
				$userPrincipalNameDomain = $parts[1];
				if (!in_array($userPrincipalNameDomain, $domains)) {
					$domains[] = $userPrincipalNameDomain;
				}
			}
		}
		$userPrincipalNameLabel = new htmlOutputText(_('User name'));
		$userPrincipalNameLabel->setMarkAsRequired(true);
		$containerLeft->addElement($userPrincipalNameLabel);
		$userPrincipalNameGroup = new htmlGroup();
		$userPrincipalNameGroup->addElement(new htmlInputField('userPrincipalName', $userPrincipalName, '15'));
		$userPrincipalNameGroup->addElement(new htmlSelect('userPrincipalNameDomain', $domains, array($userPrincipalNameDomain)));
		$containerLeft->addElement($userPrincipalNameGroup);
		$containerLeft->addElement(new htmlHelpLink('userPrincipalName'), true);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			$this->addSimpleInputTextField($containerLeft, 'sAMAccountName', _('User name (pre W2K)'));
		}
		$this->addSimpleInputTextField($containerLeft, 'givenName', _('First name'));
		$this->addSimpleInputTextField($containerLeft, 'sn', _('Last name'));
		$this->addSimpleInputTextField($containerLeft, 'cn', _('Common name'), true);
		$this->addSimpleInputTextField($containerLeft, 'displayName', _('Display name'));
		$this->addSimpleInputTextField($containerLeft, 'initials', _('Initials'));
		$this->addSimpleInputTextField($containerLeft, 'description', _('Description'));
		// address area
		$containerLeft->addElement(new htmlSubTitle(_('Address')), true);
		$this->addSimpleInputTextField($containerLeft, 'streetAddress', _('Street'), false, 20, true);
		$this->addSimpleInputTextField($containerLeft, 'postOfficeBox', _('Post office box'));
		$this->addSimpleInputTextField($containerLeft, 'postalCode', _('Postal code'));
		$this->addSimpleInputTextField($containerLeft, 'l', _('Location'));
		$this->addSimpleInputTextField($containerLeft, 'st', _('State'));
		$this->addSimpleInputTextField($containerLeft, 'physicalDeliveryOfficeName', _('Office name'));
		// contact data area
		$containerLeft->addElement(new htmlSubTitle(_('Contact data')), true);
		$this->addSimpleInputTextField($containerLeft, 'mail', _('Email address'));
		$this->addMultiValueInputTextField($containerLeft, 'otherMailbox', _('Email alias'));
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
			$this->addMultiValueInputTextField($containerLeft, 'proxyAddresses', _('Proxy-Addresses'));
		}
		$this->addSimpleInputTextField($containerLeft, 'telephoneNumber', _('Telephone number'));
		$this->addMultiValueInputTextField($containerLeft, 'otherTelephone', _('Other telephone numbers'));
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
			$this->addSimpleInputTextField($containerLeft, 'mobile', _('Mobile'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
			$this->addMultiValueInputTextField($containerLeft, 'otherMobile', _('Other mobiles'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
			$this->addSimpleInputTextField($containerLeft, 'pager', _('Pager'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
			$this->addMultiValueInputTextField($containerLeft, 'otherPager', _('Other pagers'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$this->addSimpleInputTextField($containerLeft, 'facsimileTelephoneNumber', _('Fax number'));
		}
		$this->addSimpleInputTextField($containerLeft, 'wWWHomePage', _('Web site'));
		$this->addMultiValueInputTextField($containerLeft, 'url', _('Other web sites'));
		// work details area
		if ($this->manageWorkDetails()) {
			$containerLeft->addElement(new htmlSubTitle(_('Work details')), true);
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
				$this->addSimpleInputTextField($containerLeft, 'title', _('Job title'), false, null, false, array_slice($this->titleCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
				$this->addSimpleInputTextField($containerLeft, 'carLicense', _('Car license'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
				$this->addSimpleInputTextField($containerLeft, 'employeeNumber', _('Employee number'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
				$this->addSimpleInputTextField($containerLeft, 'employeeType', _('Employee type'), false, null, false, array_slice($this->employeeTypeCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'businessCategory', _('Business category'), false, null, false, array_slice($this->businessCategoryCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
				$this->addSimpleInputTextField($containerLeft, 'company', _('Company'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
				$this->addSimpleInputTextField($containerLeft, 'department', _('Department'), false, null, false, array_slice($this->departmentCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'ou', _('Organisational unit'), false, null, false, array_slice($this->ouCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'o', _('Organisation'), false, null, false, array_slice($this->oCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
				$containerLeft->addElement(new htmlOutputText(_('Manager')));
				$managerGroup = new htmlGroup();
				$managerVal = (empty($this->attributes['manager'][0])) ? '-' : getAbstractDN($this->attributes['manager'][0]);
				$managerGroup->addElement(new htmlOutputText($managerVal));
				$managerGroup->addElement(new htmlSpacer('5px', null));
				$managerGroup->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'change', _("Change")));
				$containerLeft->addElement($managerGroup);
				$containerLeft->addElement(new htmlHelpLink('manager'), true);
			}
		}
		// account area
		$containerLeft->addElement(new htmlSubTitle(_('Account')), true);
		// password change required
		$pwdMustChange = false;
		if (isset($this->attributes['pwdLastSet'][0]) && ($this->attributes['pwdLastSet'][0] === '0')) {
			$pwdMustChange = true;
		}
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('forcePasswordChangeOption', $pwdMustChange, _("User must change password"), 'pwdMustChange'), true);
		// deactivated
		$deactivated = windowsUser::isDeactivated($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('deactivated', $deactivated, _("Account is deactivated"), 'deactivated'), true);
		// password does not expire
		$noExpire = windowsUser::isNeverExpiring($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('noExpire', $noExpire, _("Password does not expire"), 'noExpire'), true);
		// require smartcard
		$requireCard = windowsUser::isSmartCardRequired($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('requireCard', $requireCard, _("Require smartcard"), 'requireCard'), true);
		// account expiration
		$containerLeft->addElement(new htmlOutputText(_('Account expiration date')));
		$accountExpiresGroup = new htmlGroup();
		$accountExpiresGroup->addElement(new htmlOutputText($this->formatAccountExpires()));
		$accountExpiresGroup->addElement(new htmlSpacer('5px', null));
		$accountExpiresGroup->addElement(new htmlAccountPageButton(get_class($this), 'accountExpires', 'edit', _('Change')));
		$containerLeft->addElement($accountExpiresGroup);
		$containerLeft->addElement(new htmlHelpLink('accountExpires'), true);
		// last password change
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepwdLastSet')) {
			$containerLeft->addElement(new htmlOutputText(_('Last password change')));
			$containerLeft->addElement(new htmlOutputText($this->formatPwdLastSet()));
			$containerLeft->addElement(new htmlHelpLink('pwdLastSet'), true);
		}
		// last login
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidelastLogonTimestamp')) {
			$containerLeft->addElement(new htmlOutputText(_('Last login')));
			$containerLeft->addElement(new htmlOutputText($this->formatLastLogonTimestamp()));
			$containerLeft->addElement(new htmlHelpLink('lastLogonTimestamp'), true);
		}
		// user profile area
		$containerLeft->addElement(new htmlSubTitle(_('User profile')), true);
		// profile path
		$this->addSimpleInputTextField($containerLeft, 'profilePath', _('Profile path'));
		// logon script
		$this->addSimpleInputTextField($containerLeft, 'scriptPath', _('Logon script'));
		// home drive
		$drives = array('-');
		for ($i=90; $i>67; $i--) $drives[] = chr($i) . ':';
		if (!empty($this->attributes['homeDrive'][0])) {
			$selected = array(strtoupper($this->attributes['homeDrive'][0]));
		}
		else {
			$selected = array('-');
		}
		$containerLeft->addElement(new htmlTableExtendedSelect('homeDrive', $drives, $selected, _('Home drive'), 'homeDrive'), true);
		// home directory
		$this->addSimpleInputTextField($containerLeft, 'homeDirectory', _('Home directory'));

		// NIS attributes
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true) || !$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$containerLeft->addElement(new htmlSubTitle(_('NIS')), true);
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
				$this->addSimpleInputTextField($containerLeft, 'msSFU30Name', _('NIS name'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
				$this->addSimpleInputTextField($containerLeft, 'msSFU30NisDomain', _('NIS domain'));
			}
		}

		$containerLeft->addElement(new htmlEqualWidth(array('streetAddress', 'cn')), true);

		$containerRight = new htmlTable();
		$containerRight->alignment = htmlElement::ALIGN_TOP;
		// photo
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidejpegPhoto', true)) {
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
			if ($noPhoto) {
				$imageContainer->addElement(new htmlAccountPageButton(get_class($this), 'photo', 'open', _('Add photo')));
			}
			else {
				$imageContainer->addElement(new htmlButton('delPhoto', _('Delete photo')));
			}
			$containerRight->addElement($imageContainer, true);
			$containerRight->addElement(new htmlSpacer(null, '20px'), true);
		}
		// groups
		$containerRight->addElement(new htmlSubTitle(_('Groups')), true);
		$containerRight->addElement(new htmlAccountPageButton(get_class($this), 'group', 'edit', _('Edit groups')), true);
		$containerRight->addElement(new htmlSpacer(null, '10px'), true);
		$groupsList = new htmlGroup();
		$groupCNs = array();
		for ($i = 0; $i < sizeof($this->groupList); $i++) {
			$groupCNs[] = extractRDNValue($this->groupList[$i]);
		}
		natcasesort($groupCNs);
		foreach ($groupCNs as $cn) {
			$groupsList->addElement(new htmlOutputText($cn));
			$groupsList->addElement(new htmlOutputText('<br>', false));
		}
		$containerRight->addElement($groupsList);

		$container = new htmlTable();
		$container->addElement($containerLeft);
		$container->addElement(new htmlSpacer('40px', null));
		$container->addElement($containerRight);
		return $container;
	}

	/**
	 * Returns if any of the work details attributes should be managed.
	 *
	 * @return boolean has any work attributes to manage
	 */
	private function manageWorkDetails() {
		$allHidden = $this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hideou', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hideo', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)
				&& $this->isBooleanConfigOptionSet('windowsUser_hidemanager', true);
		return !$allHidden;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$return = array();
		$keysToReplace = array('cn', 'displayName', 'mail', 'otherMailbox',
				'profilePath', 'scriptPath', 'homeDirectory');
		$this->getAccountContainer()->replaceWildcardsInPOST($keysToReplace);
		// user name
		$userPrincipalName = $_POST['userPrincipalName'];
		if (!get_preg($userPrincipalName, 'username')) {
			$return[] = $this->messages['userPrincipalName'][0];
		}
		if (!empty($_POST['userPrincipalNameDomain'])) {
			$userPrincipalName .= '@' . $_POST['userPrincipalNameDomain'];
		}
		$this->attributes['userPrincipalName'][0] = $userPrincipalName;
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		if (empty($this->attributes['cn'][0])) {
			$cn = '';
			if (!empty($_POST['givenName'])) {
				$cn = $_POST['givenName'];
			}
			if (!empty($_POST['sn'])) {
				$cn .= ' ' . $_POST['sn'];
			}
			$this->attributes['cn'][0] = trim($cn);
		}
		if (!get_preg($this->attributes['cn'][0], 'cn')) {
			$return[] = $this->messages['cn'][0];
		}
		// sAMAccountName
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['sAMAccountName']) && empty($_POST['sAMAccountName'])) {
				$this->attributes['sAMAccountName'][0] = $_POST['userPrincipalName'];
			}
			else {
				$this->attributes['sAMAccountName'][0] = $_POST['sAMAccountName'];
			}
			if (!empty($this->attributes['sAMAccountName'][0]) && !get_preg($this->attributes['sAMAccountName'][0], 'username')) {
				$return[] = $this->messages['sAMAccountName'][0];
			}
		}
		elseif ($this->getAccountContainer()->isNewAccount && !empty($_POST['userPrincipalName'])) {
			// set sAMAccountName to user name if attribute input field is not visible (W2012 requires this attribute)
			$this->attributes['sAMAccountName'][0] = $_POST['userPrincipalName'];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!empty($this->attributes['displayName'][0]) && !get_preg($this->attributes['displayName'][0], 'realname')) {
			$return[] = $this->messages['displayName'][0];
		}
		if (empty($this->attributes['displayName'][0]) && !empty($this->attributes['cn'][0])) {
			$this->attributes['displayName'][0] = $this->attributes['cn'][0];
		}
		// first name
		$this->attributes['givenName'][0] = $_POST['givenName'];
		if (!empty($this->attributes['givenName'][0]) && !get_preg($_POST['givenName'], 'realname')) {
			$return[] = $this->messages['givenName'][0];
		}
		// initials
		$this->attributes['initials'][0] = $_POST['initials'];
		// location
		$this->attributes['l'][0] = $_POST['l'];
		// email
		$this->attributes['mail'][0] = $_POST['mail'];
		if (!empty($this->attributes['mail'][0]) && !get_preg($this->attributes['mail'][0], 'email')) {
			$return[] = $this->messages['mail'][0];
		}
		// email aliases
		$this->processMultiValueInputTextField('otherMailbox', $return, 'email');
		// other telephones
		$this->processMultiValueInputTextField('otherTelephone', $return, 'telephone');
		// fax number
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$this->attributes['facsimileTelephoneNumber'][0] = $_POST['facsimileTelephoneNumber'];
			if (!get_preg($_POST['facsimileTelephoneNumber'], 'telephone')) {
				$return[] = $this->messages['facsimileTelephoneNumber'][0];
			}
		}
		// office name
		$this->attributes['physicalDeliveryOfficeName'][0] = $_POST['physicalDeliveryOfficeName'];
		// postal code
		$this->attributes['postalCode'][0] = $_POST['postalCode'];
		if (!get_preg($_POST['postalCode'], 'postalCode')) {
			$return[] = $this->messages['postalCode'][0];
		}
		// post office box
		$this->attributes['postOfficeBox'][0] = $_POST['postOfficeBox'];
		// last name
		$this->attributes['sn'][0] = $_POST['sn'];
		if (!empty($this->attributes['sn'][0]) && !get_preg($_POST['sn'], 'realname')) {
			$return[] = $this->messages['sn'][0];
		}
		// state
		$this->attributes['st'][0] = $_POST['st'];
		// street
		$this->attributes['streetAddress'][0] = $_POST['streetAddress'];
		// telephone
		$this->attributes['telephoneNumber'][0] = $_POST['telephoneNumber'];
		if (!get_preg($_POST['telephoneNumber'], 'telephone')) {
			$return[] = $this->messages['telephoneNumber'][0];
		}
		// other web sites
		$this->processMultiValueInputTextField('url', $return);
		// web site
		$this->attributes['wWWHomePage'][0] = $_POST['wWWHomePage'];
		// password must be changed
		if (isset($_POST['forcePasswordChangeOption']) && ($_POST['forcePasswordChangeOption'] == 'on')) {
			$this->attributes['pwdLastSet'][0] = '0';
		}
		else {
			if (isset($this->orig['pwdLastSet'][0]) && ($this->orig['pwdLastSet'][0] !== '0')) {
				$this->attributes['pwdLastSet'][0] = $this->orig['pwdLastSet'][0];
			}
			else {
				$this->attributes['pwdLastSet'][0] = '-1';
			}
		}
		// save pwdLastSet for postModifyActions
		$this->pwdLastSet = null;
		if (isset($this->attributes['pwdLastSet'][0])) {
			if (!isset($this->orig['pwdLastSet'][0]) || ($this->orig['pwdLastSet'][0] != $this->attributes['pwdLastSet'][0])) {
				$this->pwdLastSet = $this->attributes['pwdLastSet'][0];
			}
		}
		// deactivated
		$deactivated = isset($_POST['deactivated']) && ($_POST['deactivated'] == 'on');
		windowsUser::setIsDeactivated($deactivated);
		// no expire
		$noExpire = isset($_POST['noExpire']) && ($_POST['noExpire'] == 'on');
		windowsUser::setIsNeverExpiring($this->attributes, $noExpire);
		// smartcard required
		$requireCard = isset($_POST['requireCard']) && ($_POST['requireCard'] == 'on');
		windowsUser::setIsSmartCardRequired($this->attributes, $requireCard);
		// profile path
		$this->attributes['profilePath'][0] = $_POST['profilePath'];
		if (!($this->attributes['profilePath'][0] == '') && !get_preg($this->attributes['profilePath'][0], 'UNC')) {
			$return[] = $this->messages['profilePath'][0];
		}
		// logon script
		$this->attributes['scriptPath'][0] = $_POST['scriptPath'];
		if (($this->attributes['scriptPath'][0] != '') && (!get_preg($this->attributes['scriptPath'][0], 'logonscript'))) {
			$return[] = $this->messages['scriptPath'][0];
		}
		// home drive
		if ($_POST['homeDrive'] == "-") {
			$this->attributes['homeDrive'][0] = '';
		}
		else {
			$this->attributes['homeDrive'][0] = $_POST['homeDrive'];
		}
		// home directory
		$this->attributes['homeDirectory'][0] = $_POST['homeDirectory'];
		if (!empty($this->attributes['homeDrive'][0]) && !get_preg($this->attributes['homeDirectory'][0], 'UNC')) {
			$return[] = $this->messages['homeDirectory'][0];
		}
		// NIS name
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
			if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['msSFU30Name']) && empty($_POST['msSFU30Name'])) {
				$this->attributes['msSFU30Name'][0] = $_POST['userPrincipalName'];
			}
			else {
				$this->attributes['msSFU30Name'][0] = $_POST['msSFU30Name'];
			}
			if (!empty($this->attributes['msSFU30Name'][0]) && !get_preg($this->attributes['msSFU30Name'][0], 'username')) {
				$return[] = $this->messages['msSFU30Name'][0];
			}
		}
		// NIS domain
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
			$this->attributes['msSFU30NisDomain'][0] = $_POST['msSFU30NisDomain'];
		}
		// title
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$this->attributes['title'][0] = $_POST['title'];
		}
		// car license
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
			$this->attributes['carLicense'][0] = $_POST['carLicense'];
		}
		// employee number
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
			$this->attributes['employeeNumber'][0] = $_POST['employeeNumber'];
		}
		// employee type
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$this->attributes['employeeType'][0] = $_POST['employeeType'];
		}
		// business category
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$this->processMultiValueInputTextField('businessCategory', $return, 'businessCategory');
		}
		// department
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$this->attributes['department'][0] = $_POST['department'];
		}
		// organizational unit
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$this->processMultiValueInputTextField('ou', $return);
		}
		// organisation
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$this->processMultiValueInputTextField('o', $return);
		}
		// Company
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$this->attributes['company'][0] = $_POST['company'];
		}
		// Mobile
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
			$this->attributes['mobile'][0] = $_POST['mobile'];
			if (!get_preg($_POST['mobile'], 'telephone')) {
				$return[] = $this->messages['mobile'][0];
			}
		}
		// Other mobiles
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
			$this->processMultiValueInputTextField('otherMobile', $return, 'telephone');
		}
		// Pager
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
			$this->attributes['pager'][0] = $_POST['pager'];
			if (!get_preg($_POST['pager'], 'telephone')) {
				$return[] = $this->messages['pager'][0];
			}
		}
		// Other pagers
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
			$this->processMultiValueInputTextField('otherPager', $return, 'telephone');
		}
		// Proxy-Addresses
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
			$this->processMultiValueInputTextField('proxyAddresses', $return);
		}
		return $return;
	}

	/**
	* This function will create the meta HTML code to show a page to change account expiration.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_accountExpires() {
		$return = new htmlTable();
		$attr = 'accountExpires';
		$text = _('Account expiration date');
		$help = "accountExpires";
		$datetime = new DateTime('now', getTimeZone());
		if (!empty($this->attributes[$attr][0]) && !($this->attributes[$attr][0] == '0')) {
			$datetime = windowsUser::getFileTime($this->attributes[$attr][0]);
		}
		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2050; $i++ ) $year[] = $i;
		$return->addElement(new htmlOutputText($text));
		$return->addElement(new htmlSelect('expire_day', $mday, array($datetime->format('d'))));
		$return->addElement(new htmlSelect('expire_mon', $mon, array($datetime->format('m'))));
		$return->addElement(new htmlSelect('expire_yea', $year, array($datetime->format('Y'))));
		$return->addElement(new htmlHelpLink($help), true);
		if ($this->getAccountContainer()->getAccountModule('shadowAccount') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncShadow', false, _('Set also for Shadow')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('heimdalKerberos') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncHeimdal', false, _('Set also for Kerberos')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberos') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncMIT', false, _('Set also for Kerberos')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberosStructural') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncMITStructural', false, _('Set also for Kerberos')), true);
		}
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttons = new htmlTable();
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'change' . $attr, _('Change')));
		if (isset($this->attributes[$attr][0])) {
			$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'del' . $attr, _('Remove')));
		}
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back' . $attr, _('Cancel')));
		$buttons->colspan = 6;
		$return->addElement($buttons);
		return $return;
	}

	/**
	* Processes user input of the account expiration page.
	*
	* @return array list of info/error messages
	*/
	function process_accountExpires() {
		$return = array();
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (strpos($postKeys[$i], 'form_subpage_windowsUser_attributes_') !== false) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (strpos($buttonName, '_back') !== false)) return array();
		$attr = 'accountExpires';
		// determine action
		if (strpos($buttonName, '_change') !== false) {
			// set new time
			$this->setExpirationDate($_POST['expire_yea'], $_POST['expire_mon'], $_POST['expire_day']);
			// sync other modules
			if (isset($_POST['syncShadow']) && ($_POST['syncShadow'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('shadowAccount')->setExpirationDate(
					$_POST['expire_yea'], $_POST['expire_mon'], $_POST['expire_day']);
			}
			if (isset($_POST['syncHeimdal']) && ($_POST['syncHeimdal'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('heimdalKerberos')->setExpirationDate(
					$_POST['expire_yea'], $_POST['expire_mon'], $_POST['expire_day']);
			}
			if (isset($_POST['syncMIT']) && ($_POST['syncMIT'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberos')->setExpirationDate(
					$_POST['expire_yea'], $_POST['expire_mon'], $_POST['expire_day']);
			}
			if (isset($_POST['syncMITStructural']) && ($_POST['syncMITStructural'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberosStructural')->setExpirationDate(
					$_POST['expire_yea'], $_POST['expire_mon'], $_POST['expire_day']);
			}
		}
		elseif (strpos($buttonName, '_del') !== false) {
			// remove attribute value
			unset($this->attributes[$attr]);
			// sync other modules
			if (isset($_POST['syncShadow']) && ($_POST['syncShadow'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('shadowAccount')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncHeimdal']) && ($_POST['syncHeimdal'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('heimdalKerberos')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncMIT']) && ($_POST['syncMIT'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberos')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncMITStructural']) && ($_POST['syncMITStructural'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberosStructural')->setExpirationDate(
					null, null, null);
			}
		}
		return $return;
	}

	/**
	* Displays the group selection.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_group() {
		$return = new htmlTable();
		$groups = $this->findGroups();
		// sort by DN
		usort($groups, 'compareDN');

		$groupContainer = new htmlTable();
		$groupContainer->alignment = htmlElement::ALIGN_TOP;
		$groupContainer->addElement(new htmlSubTitle(_("Groups")), true);
		$groupContainer->addElement(new htmlOutputText(_("Selected groups")));
		$groupContainer->addElement(new htmlOutputText(''));
		$groupContainer->addElement(new htmlOutputText(_("Available groups")));
		$groupContainer->addNewLine();

		$selectedGroups = array();
		// sort by DN
		usort($this->groupList, 'compareDN');
		for ($i = 0; $i < sizeof($this->groupList); $i++) {
			if (in_array($this->groupList[$i], $groups)) {
				$selectedGroups[getAbstractDN($this->groupList[$i])] = $this->groupList[$i];
			}
		}
		$availableGroups = array();
		foreach ($groups as $dn) {
			if (!in_array($dn, $this->groupList)) {
				$availableGroups[getAbstractDN($dn)] = $dn;
			}
		}

		$remGroupSelect = new htmlSelect('removegroups', $selectedGroups, null, 15);
		$remGroupSelect->setMultiSelect(true);
		$remGroupSelect->setTransformSingleSelect(false);
		$remGroupSelect->setHasDescriptiveElements(true);
		$remGroupSelect->setRightToLeftTextDirection(true);
		$remGroupSelect->setSortElements(false);
		$groupContainer->addElement($remGroupSelect);
		$buttonGroupContainer = new htmlTable();
		$buttonGroupContainer->addElement(new htmlButton('addgroups_button', 'back.gif', true), true);
		$buttonGroupContainer->addElement(new htmlButton('removegroups_button', 'forward.gif', true), true);
		$groupContainer->addElement($buttonGroupContainer);
		$addGroupSelect = new htmlSelect('addgroups', $availableGroups, null, 15);
		$addGroupSelect->setMultiSelect(true);
		$addGroupSelect->setHasDescriptiveElements(true);
		$addGroupSelect->setTransformSingleSelect(false);
		$addGroupSelect->setRightToLeftTextDirection(true);
		$addGroupSelect->setSortElements(false);
		$groupContainer->addElement($addGroupSelect);
		$groupContainer->addNewLine();
		$return->addElement($groupContainer);
		$return->addNewLine();

		$backGroup = new htmlGroup();
		$backGroup->colspan = 10;
		$backGroup->addElement(new htmlSpacer(null, '10px'), true);
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$backGroup->addElement($backButton);
		$return->addElement($backGroup);
		return $return;
	}

	/**
	* Processes user input of the group selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_group() {
		if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
			// add new group
			$this->groupList = @array_merge($this->groupList, $_POST['addgroups']);
		}
		elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
			$this->groupList = array_delete($_POST['removegroups'], $this->groupList);
		}
		return array();
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
	* Sets a new photo.
	*/
	function process_photo() {
		if (!isset($_POST['form_subpage_' . get_class($this) . '_attributes_submit'])) {
			return array();
		}
		$messages = array();
		if ($_FILES['photoFile'] && ($_FILES['photoFile']['size'] > 0)) {
			$name = $_FILES['photoFile']['name'];
			$extension = strtolower(substr($name, strpos($name, '.') + 1));
			if (!extension_loaded('imagick') && !($extension == 'jpg') && !($extension == 'jpeg')) {
				$messages[] = $this->messages['file'][1];
				return $messages;
			}
			$handle = fopen($_FILES['photoFile']['tmp_name'], "r");
			$data = fread($handle, 10000000);
			if (!empty($this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]) && (strlen($data) > (1024 * $this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]))) {
				$errMsg = $this->messages['file'][3];
				$errMsg[] = null;
				$errMsg[] = array($this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]);
				return array($errMsg);
			}
			fclose($handle);
			if (extension_loaded('imagick')) {
				// convert to JPG if imagick extension is available
				$image = new Imagick();
				try {
				$image->readImageBlob($data);
				// resize if maximum values specified
				if (!empty($this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0]) || !empty($this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0])) {
					$maxWidth = empty($this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0]) ? $image->getimagewidth() : $this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0];
					$maxHeight = empty($this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0]) ? $image->getimageheight() : $this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0];
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
		$options = array();
		$filter = get_ldap_filter('user');
		$entries = searchLDAPByFilter('(|' . $filter . '(objectclass=organizationalRole))', array('dn'), array('user'));
		for ($i = 0; $i < sizeof($entries); $i++) {
			$entries[$i] = $entries[$i]['dn'];
		}
		// sort by DN
		usort($entries, 'compareDN');
		for ($i = 0; $i < sizeof($entries); $i++) {
			$options[getAbstractDN($entries[$i])] = $entries[$i];
		}
		$selectedManager = array();
		if (!empty($this->attributes['manager'][0])) {
			$selectedManager[] = $this->attributes['manager'][0];
		}
		$size = 20;
		if (sizeof($options) < 20) $size = sizeof($options);
		$managerSelect = new htmlSelect('manager', $options, $selectedManager, $size);
		$managerSelect->setHasDescriptiveElements(true);
		$managerSelect->setRightToLeftTextDirection(true);
		$managerSelect->setSortElements(false);
		$managerSelect->setTransformSingleSelect(false);
		$return->addElement($managerSelect, true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'setManager', _('Change')));
		$buttonTable->addSpace('5px');
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'removeManager', _('Remove')));
		$buttonTable->addSpace('20px');
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancelManager', _('Cancel')));
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
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_removeManager'])) {
			if (!empty($this->attributes['manager'])) {
				unset($this->attributes['manager']);
			}
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_attributes_setManager']) && isset($_POST['manager'])) {
			$this->attributes['manager'][0] = $_POST['manager'];
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
		// set groups
		$groups = $this->findGroups();
		$toAdd = array_values(array_diff($this->groupList, $this->groupList_orig));
		$toRem = array_values(array_diff($this->groupList_orig, $this->groupList));
		$toUpdate = array_values(array_intersect($this->groupList, $this->groupList_orig));
		$ldapUser = $_SESSION['ldap']->decrypt_login();
		$ldapUser = $ldapUser[0];
		// add groups
		for ($i = 0; $i < sizeof($toAdd); $i++) {
			if (in_array($toAdd[$i], $groups)) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $toAdd[$i], array('member' => array($this->getAccountContainer()->finalDN)));
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toAdd[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i]);
				}
			}
		}
		// remove groups
		for ($i = 0; $i < sizeof($toRem); $i++) {
			if (in_array($toRem[$i], $groups)) {
				$success = @ldap_mod_del($_SESSION['ldap']->server(), $toRem[$i], array('member' => array($this->getAccountContainer()->dn_orig)));
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i]);
				}
			}
		}
		// force password change if needed
		if ($this->pwdLastSet != null) {
			$attrs = array('pwdLastSet' => array($this->pwdLastSet));
			$success = @ldap_modify($_SESSION['ldap']->server(), $this->getAccountContainer()->finalDN, $attrs);
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to change pwdLastSet for ' . $this->getAccountContainer()->finalDN . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$messages[] = array('ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $this->getAccountContainer()->finalDN), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = array();
		// get list of existing groups
		$groupList = $this->findGroups();
		$groupMap = array();
		foreach ($groupList as $dn) {
			$groupMap[extractRDNValue($dn)] = $dn;
		}
		$booleanOptions = array(_('yes') => true, _('no') => false);
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('user', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'user';
			// userPrincipalName
			if (get_preg($rawAccounts[$i][$ids['windowsUser_userPrincipalName']], 'username')) {
				$partialAccounts[$i]['userPrincipalName'] = $rawAccounts[$i][$ids['windowsUser_userPrincipalName']];
			}
			else {
				$errMsg = $this->messages['userPrincipalName'][1];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// first name
			if ($rawAccounts[$i][$ids['windowsUser_firstName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_firstName']], 'realname')) {
					$partialAccounts[$i]['givenName'] = $rawAccounts[$i][$ids['windowsUser_firstName']];
				}
				else {
					$errMsg = $this->messages['givenName'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// last name
			if ($rawAccounts[$i][$ids['windowsUser_lastName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_lastName']], 'realname')) {
					$partialAccounts[$i]['sn'] = $rawAccounts[$i][$ids['windowsUser_lastName']];
				}
				else {
					$errMsg = $this->messages['sn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// cn
			if ($rawAccounts[$i][$ids['windowsUser_cn']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_cn']], 'cn')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsUser_cn']];
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$cn = '';
				if (!empty($rawAccounts[$i][$ids['windowsUser_firstName']])) {
					$cn = $rawAccounts[$i][$ids['windowsUser_firstName']];
				}
				if (!empty($rawAccounts[$i][$ids['windowsUser_lastName']])) {
					$cn .= ' ' . $rawAccounts[$i][$ids['windowsUser_lastName']];
				}
				$cn = trim($cn);
				if (!empty($cn)) {
					$partialAccounts[$i]['cn'] = $cn;
				}
			}
			// sAMAccountName
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
				if (!empty($rawAccounts[$i][$ids['windowsUser_sAMAccountName']])) {
					if (get_preg($rawAccounts[$i][$ids['windowsUser_sAMAccountName']], 'username')) {
						$partialAccounts[$i]['sAMAccountName'] = $rawAccounts[$i][$ids['windowsUser_sAMAccountName']];
					}
					else {
						$errMsg = $this->messages['sAMAccountName'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					$samUser = explode('@', $partialAccounts[$i]['userPrincipalName']);
					$partialAccounts[$i]['sAMAccountName'] = $samUser[0];
				}
			}
			else {
				// set sAMAccountName to user name if not managed (W2012 requires it)
				$parts = explode('@', $partialAccounts[$i]['userPrincipalName']);
				$partialAccounts[$i]['sAMAccountName'] = $parts[0];
			}
			// password
			if (($rawAccounts[$i][$ids['windowsUser_password']] != "") && (get_preg($rawAccounts[$i][$ids['windowsUser_password']], 'password'))) {
				$partialAccounts[$i]['unicodePwd'] = self::pwdAttributeValue($rawAccounts[$i][$ids['windowsUser_password']]);
				$partialAccounts[$i]['INFO.userPasswordClearText'] = $rawAccounts[$i][$ids['windowsUser_password']]; // for custom scripts etc.
			}
			elseif ($rawAccounts[$i][$ids['windowsUser_password']] != "") {
				$errMsg = $this->messages['userPassword'][4];
				$errMsg[2] = str_replace('%', '%%', $errMsg[2]); // double "%" because of later sprintf
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// display name
			if ($rawAccounts[$i][$ids['windowsUser_displayName']] != "") {
				$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['windowsUser_displayName']];
			}
			elseif (!empty($partialAccounts[$i]['cn'])) {
				$partialAccounts[$i]['displayName'] = $partialAccounts[$i]['cn'];
			}
			// initials
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_initials', 'initials');
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_description', 'description');
			// street
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_streetAddress', 'streetAddress');
			// post office box
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_postOfficeBox', 'postOfficeBox');
			// postal code
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_postalCode', 'postalCode',
				'postalCode', $this->messages['postalCode'][1], $errors);
			// location
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_l', 'l');
			// state
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_state', 'st');
			// office name
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_officeName', 'physicalDeliveryOfficeName');
			// mail
			if ($rawAccounts[$i][$ids['windowsUser_mail']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_mail']], 'email')) {
					$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['windowsUser_mail']];
				}
				else {
					$errMsg = $this->messages['mail'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// mail aliases
			if (isset($ids['windowsUser_otherMailbox']) && ($rawAccounts[$i][$ids['windowsUser_otherMailbox']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_otherMailbox']]);
				$partialAccounts[$i]['otherMailbox'] = $valueList;
				for ($x = 0; $x < sizeof($valueList); $x++) {
					if (!get_preg($valueList[$x], 'email')) {
						$errMsg = $this->messages['otherMailbox'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// telephone
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_telephoneNumber', 'telephoneNumber',
				'telephone', $this->messages['telephoneNumber'][1], $errors);
			// other telephone
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherTelephone', 'otherTelephone', 'telephone', $this->messages['otherTelephone'][1], $errors, '/;[ ]*/');
			// fax number
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_facsimileTelephoneNumber', 'facsimileTelephoneNumber',
						'telephone', $this->messages['facsimileTelephoneNumber'][1], $errors);
			}
			// website
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_webSite', 'wWWHomePage');
			// other websites
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherWebSites', 'url', null, null, $errors, '/;[ ]*/');
			// user account control
			$userAccountControlAttr['userAccountControl'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
			// deactivated
			if ($rawAccounts[$i][$ids['windowsUser_deactivated']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]])) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_deactivated', _('Please enter either yes or no.'));
				}
				else {
					$this->setIsDeactivated($booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]], $userAccountControlAttr);
				}
			}
			// password does not expire
			if ($rawAccounts[$i][$ids['windowsUser_noExpire']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_noExpire']]])) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_noExpire', _('Please enter either yes or no.'));
				}
				else {
					$this->setIsNeverExpiring($userAccountControlAttr, $booleanOptions[$rawAccounts[$i][$ids['windowsUser_noExpire']]]);
				}
			}
			// account expiration
			if (!empty($rawAccounts[$i][$ids['windowsUser_accountExpires']])) {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_accountExpires']], 'date')) {
					$dateParts = explode('-', $rawAccounts[$i][$ids['windowsUser_accountExpires']]);
					$partialAccounts[$i]['accountExpires'] = $this->buildExpirationDate($dateParts[2], $dateParts[1], $dateParts[0]);
				}
				else {
					$errMsg = $this->messages['accountExpires'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// require smartcard
			if ($rawAccounts[$i][$ids['windowsUser_requireCard']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_requireCard']]])) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_requireCard', _('Please enter either yes or no.'));
				}
				else {
					$this->setIsSmartCardRequired($userAccountControlAttr, $booleanOptions[$rawAccounts[$i][$ids['windowsUser_requireCard']]]);
				}
			}
			$partialAccounts[$i]['userAccountControl'][0] = $userAccountControlAttr['userAccountControl'][0];
			// end user account control
			// password change at next login
			if ($rawAccounts[$i][$ids['windowsUser_pwdMustChange']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_pwdMustChange']]])) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_pwdMustChange', _('Please enter either yes or no.'));
				}
				// attribute must be set in postModify
			}
			// profile path
			if ($rawAccounts[$i][$ids['windowsUser_profilePath']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_profilePath']], 'UNC')) {
					$partialAccounts[$i]['profilePath'] = $rawAccounts[$i][$ids['windowsUser_profilePath']];
				}
				else {
					$errMsg = $this->messages['profilePath'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// logon script
			if ($rawAccounts[$i][$ids['windowsUser_scriptPath']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_scriptPath']], 'logonscript')) {
					$partialAccounts[$i]['scriptPath'] = $rawAccounts[$i][$ids['windowsUser_scriptPath']];
				}
				else {
					$errMsg = $this->messages['scriptPath'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// home drive
			if (!empty($rawAccounts[$i][$ids['windowsUser_homeDrive']])) {
				if (preg_match("/[d-z]:/i", $rawAccounts[$i][$ids['windowsUser_homeDrive']])) {
					$partialAccounts[$i]['homeDrive'] = strtoupper($rawAccounts[$i][$ids['windowsUser_homeDrive']]);
				}
				else {
					$errMsg = $this->messages['homeDrive'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// home directory
			if ($rawAccounts[$i][$ids['windowsUser_homeDirectory']] != "") {
				if (empty($partialAccounts[$i]['homeDrive']) || get_preg($rawAccounts[$i][$ids['windowsUser_homeDirectory']], 'UNC')) {
					$partialAccounts[$i]['homeDirectory'] = $rawAccounts[$i][$ids['windowsUser_homeDirectory']];
				}
				else {
					$errMsg = $this->messages['homeDirectory'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// groups
			if (isset($ids['windowsUser_groups']) && ($rawAccounts[$i][$ids['windowsUser_groups']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_groups']]);
				$invalidGroups = array();
				foreach ($valueList as $group) {
					if (!in_array($group, $groupList) && !isset($groupMap[$group])) {
						$invalidGroups[] = $group;
					}
				}
				if (sizeof($invalidGroups) > 0) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_groups',
						_('LAM was unable to find a group with this name!') . '<br>' . htmlspecialchars(implode('; ', $invalidGroups)));
				}
			}
			// NIS name
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_msSFU30Name', 'msSFU30Name',
					'username', $this->messages['msSFU30Name'][1], $errors);
			}
			// NIS domain
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_msSFU30NisDomain', 'msSFU30NisDomain');
			}
			// title
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_title', 'title', 'title', $this->messages['title'][1], $errors);
			}
			// carLicense
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_carLicense', 'carLicense');
			}
			// employee number
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_employeeNumber', 'employeeNumber');
			}
			// employee type
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_employeeType', 'employeeType',
					'employeeType', $this->messages['employeeType'][1], $errors);
			}
			// business category
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_businessCategory', 'businessCategory', 'businessCategory', $this->messages['businessCategory'][1], $errors, '/;[ ]*/');
			}
			// departments
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_department', 'department', null, null, $errors, '/;[ ]*/');
			}
			// organisational unit
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_ou', 'ou', null, null, $errors, '/;[ ]*/');
			}
			// organisation
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_o', 'o', null, null, $errors, '/;[ ]*/');
			}
			// manager
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_manager', 'manager', 'dn', $this->messages['manager'][0], $errors);
			}
			// company
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_company', 'company');
			}
			// mobile
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_mobile', 'mobile',
						'telephone', $this->messages['mobile'][1], $errors);
			}
			// other mobile
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherMobile', 'otherMobile',
						'telephone', $this->messages['otherMobile'][1], $errors, '/;[ ]*/');
			}
			// pager
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_pager', 'pager',
						'telephone', $this->messages['pager'][1], $errors);
			}
			// other pager
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherPager', 'otherPager',
						'telephone', $this->messages['otherPager'][1], $errors, '/;[ ]*/');
			}
			// Proxy-Addresses
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_proxyAddresses', 'proxyAddresses',
						null, null, $errors, '/;[ ]*/');
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
		// on first call generate list of ldap operations
		if (!isset($temp['counter'])) {
			$groupList = $this->findGroups();
			$groupMap = array();
			foreach ($groupList as $dn) {
				$groupMap[extractRDNValue($dn)] = $dn;
			}
			$temp['groups'] = &$groupList;
			$temp['groupMap'] = &$groupMap;
			$temp['members'] = array();
			$temp['memberCount'] = 0;
			$temp['pwdChange'] = array();
			$temp['pwdChangeCount'] = 0;
			$groupCol = $ids['windowsUser_groups'];
			$passwordChangeRequiredCol = $ids['windowsUser_pwdMustChange'];
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				if ($data[$i][$groupCol] != "") {
					$groups = preg_split('/;[ ]*/', $data[$i][$groupCol]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (in_array($groups[$g], $temp['groups'])) {
							$temp['members'][$groups[$g]][] = $accounts[$i]['dn'];
						}
						elseif (isset($temp['groupMap'][$groups[$g]])) {
							$temp['members'][$temp['groupMap'][$groups[$g]]][] = $accounts[$i]['dn'];
						}
					}
				}
				if (isset($data[$i][$passwordChangeRequiredCol]) && ($data[$i][$passwordChangeRequiredCol] == _('yes'))) {
					$temp['pwdChange'][] = $accounts[$i]['dn'];
				}
			}
			$temp['memberCount'] = sizeof($temp['members']);
			$temp['pwdChangeCount'] = sizeof($temp['pwdChange']);
			$temp['counter'] = $temp['memberCount'] + $temp['pwdChangeCount'];
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => array()
			);
		}
		// add users to groups
		elseif (sizeof($temp['members']) > 0) {
			$keys = array_keys($temp['members']);
			$group = $keys[0];
			$member = array_pop($temp['members'][$group]);
			$success = @ldap_mod_add($_SESSION['ldap']->server(), $group, array('member' => $member));
			$errors = array();
			if (!$success) {
				$errors[] = array(
					"ERROR",
					_("LAM was unable to modify group memberships for group: %s"),
					getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					array($group)
				);
			}
			if (sizeof($temp['members'][$group]) == 0) {
				unset($temp['members'][$group]);
			}
			$memberPercentage = (100 * ($temp['memberCount'] - sizeof($temp['members']))) / $temp['counter'];
			return array (
				'status' => 'inProgress',
				'progress' => $memberPercentage,
				'errors' => $errors
			);
		}
		// force password change
		elseif (sizeof($temp['pwdChange']) > 0) {
			$dn = array_pop($temp['pwdChange']);
			$success = @ldap_mod_replace($_SESSION['ldap']->server(), $dn, array('pwdLastSet' => '0'));
			$errors = array();
			if (!$success) {
				$errors[] = array(
					"ERROR",
					_("Was unable to modify attributes of DN: %s."),
					getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					array($dn)
				);
			}
			$pwdPercentage = (100 * ($temp['memberCount'] + ($temp['pwdChangeCount'] - sizeof($temp['pwdChange'])))) / $temp['counter'];
			return array (
				'status' => 'inProgress',
				'progress' => $pwdPercentage,
				'errors' => $errors
			);
		}
		// all modifications are done
		else {
			return array (
				'status' => 'finished',
				'progress' => 100,
				'errors' => array()
			);
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'userPrincipalName', _('User name'));
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'sAMAccountName', _('User name (pre W2K)'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		$this->addSimplePDFField($return, 'givenName', _('First name'));
		$this->addSimplePDFField($return, 'initials', _('Initials'));
		$this->addSimplePDFField($return, 'l', _('Location'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'otherMailbox', _('Email alias'));
		$this->addSimplePDFField($return, 'otherTelephone', _('Other telephone numbers'));
		$this->addSimplePDFField($return, 'physicalDeliveryOfficeName', _('Office name'));
		$this->addSimplePDFField($return, 'postalCode', _('Postal code'));
		$this->addSimplePDFField($return, 'postOfficeBox', _('Post office box'));
		$this->addSimplePDFField($return, 'sn', _('Last name'));
		$this->addSimplePDFField($return, 'st', _('State'));
		$this->addSimplePDFField($return, 'streetAddress', _('Street'));
		$this->addSimplePDFField($return, 'telephoneNumber', _('Telephone number'));
		$this->addSimplePDFField($return, 'facsimileTelephoneNumber', _('Fax number'));
		$this->addSimplePDFField($return, 'company', _('Company'));
		$this->addSimplePDFField($return, 'pager', _('Pager'));
		$this->addSimplePDFField($return, 'otherPager', _('Other pagers'));
		$this->addSimplePDFField($return, 'mobile', _('Mobile'));
		$this->addSimplePDFField($return, 'otherMobile', _('Other mobiles'));
		$this->addSimplePDFField($return, 'proxyAddresses', _('Proxy-Addresses'));
		$this->addSimplePDFField($return, 'url', _('Other web sites'));
		$this->addSimplePDFField($return, 'wWWHomePage', _('Web site'));
		$this->addSimplePDFField($return, 'msSFU30Name', _('NIS name'));
		$this->addSimplePDFField($return, 'msSFU30NisDomain', _('NIS domain'));
		$this->addSimplePDFField($return, 'title', _('Job title'));
		$this->addSimplePDFField($return, 'carLicense', _('Car license'));
		$this->addSimplePDFField($return, 'employeeNumber', _('Employee number'));
		$this->addSimplePDFField($return, 'employeeType', _('Employee type'));
		$this->addSimplePDFField($return, 'businessCategory', _('Business category'));
		$this->addSimplePDFField($return, 'department', _('Department'));
		$this->addSimplePDFField($return, 'ou', _('Organisational unit'));
		$this->addSimplePDFField($return, 'o', _('Organisation'));
		$this->addSimplePDFField($return, 'manager', _('Manager'));
		$deactivated = _('no');
		if ($this->isDeactivated($this->attributes)) {
			$deactivated = _('yes');
		}
		$this->addPDFKeyValue($return, 'deactivated', _('Account is deactivated'), $deactivated);
		$noExpire = _('no');
		if ($this->isNeverExpiring($this->attributes)) {
			$noExpire = _('yes');
		}
		$this->addPDFKeyValue($return, 'noExpire', _('Password does not expire'), $noExpire);
		$this->addPDFKeyValue($return, 'accountExpires', _('Account expiration date'), $this->formatAccountExpires());
		$requireCard = _('no');
		if ($this->isSmartCardRequired($this->attributes)) {
			$requireCard = _('yes');
		}
		$this->addPDFKeyValue($return, 'requireCard', _('Require smartcard'), $requireCard);
		$pwdMustChange = _('no');
		if (isset($this->attributes['pwdLastSet'][0]) && ($this->attributes['pwdLastSet'][0] === '0')) {
			$pwdMustChange = _('yes');
		}
		$this->addPDFKeyValue($return, 'pwdMustChange', _('Password change at next login'), $pwdMustChange);
		$this->addSimplePDFField($return, 'profilePath', _('Profile path'));
		$this->addSimplePDFField($return, 'scriptPath', _('Logon script'));
		$this->addSimplePDFField($return, 'homeDirectory', _('Home directory'));
		$this->addSimplePDFField($return, 'homeDrive', _('Home drive'));
		$groups = array();
		foreach ($this->groupList as $group) {
			$groups[] = extractRDNValue($group);
		}
		$this->addPDFKeyValue($return, 'groups', _('Groups'), $groups);
		// password
		if (isset($this->clearTextPassword)) {
			$this->addPDFKeyValue($return, 'password', _('Password'), $this->clearTextPassword);
		}
		else if (isset($this->attributes['INFO.userPasswordClearText'])) {
			$this->addPDFKeyValue($return, 'password', _('Password'), $this->attributes['INFO.userPasswordClearText']);
		}
		// last password change
		$this->addPDFKeyValue($return, 'pwdLastSet', _('Last password change'), $this->formatPwdLastSet());
		// last login
		$this->addPDFKeyValue($return, 'lastLogonTimestamp', _('Last login'), $this->formatLastLogonTimestamp());
		return $return;
	}

	/**
	* {@inheritDoc}
	*/
	function get_profileOptions($typeId) {
		$return = parent::get_profileOptions($typeId);
		// domain
		$domains = $this->getDomains();
		$domains[] = '';
		$return->addElement(new htmlTableExtendedSelect('windowsUser_userPrincipalNameDomain', $domains, array(), _('Domain'), 'userPrincipalNameDomain'), true);
		// group memberships
		$groups = $this->findGroups();
		$groupList = array();
		foreach ($groups as $dn) {
			$groupList[extractRDNValue($dn)] = $dn;
		}
		$groupSelect = new htmlTableExtendedSelect('windowsUser_groups', $groupList, array(), _('Groups'), 'groups', 10);
		$groupSelect->setHasDescriptiveElements(true);
		$groupSelect->setMultiSelect(true);
		$return->addElement($groupSelect, true);
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
		// load domain
		if (isset($profile['windowsUser_userPrincipalNameDomain'][0])) {
			$user = empty($this->attributes['userPrincipalName'][0]) ? '' : $this->attributes['userPrincipalName'][0];
			$user = explode('@', $user);
			$user = $user[0] . '@' . $profile['windowsUser_userPrincipalNameDomain'][0];
			$this->attributes['userPrincipalName'][0] = $user;
		}
		// load groups
		if (isset($profile['windowsUser_groups'][0])) {
			$this->groupList = $profile['windowsUser_groups'];
		}
		// other mailboxes
		if (!empty($profile['windowsUser_otherMailbox'][0])) {
			$this->attributes['otherMailbox'] = preg_split('/;[ ]*/', $profile['windowsUser_otherMailbox'][0]);
		}
		// account expiration date
		if (!empty($profile['windowsUser_accountExpires'][0]) && is_numeric($profile['windowsUser_accountExpires'][0])) {
			$numDays = $profile['windowsUser_accountExpires'][0];
			$date = new DateTime('now', getTimeZone());
			$toAdd = new DateInterval('P' . $numDays . 'D');
			$dateTarget = $date->add($toAdd);
			$this->setExpirationDate($dateTarget->format('Y'), $dateTarget->format('m'), $dateTarget->format('d'));
		}
		// departments
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment')) {
			if (isset($profile['windowsUser_department'][0]) && $profile['windowsUser_department'][0] != '') {
				$departments = explode(';', $profile['windowsUser_department'][0]);
				// remove extra spaces and set attributes
				$this->attributes['department'] = array_map('trim', $departments);
			}
		}
		// organizational unit
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou') && isset($profile['windowsUser_ou'][0])) {
			$oList = preg_split('/;[ ]*/', $profile['windowsUser_ou'][0]);
			$this->attributes['ou'] = $oList;
		}
		// organisation
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo') && isset($profile['windowsUser_o'][0])) {
			$oList = preg_split('/;[ ]*/', $profile['windowsUser_o'][0]);
			$this->attributes['o'] = $oList;
		}
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
		if (in_array('unicodePwd', $fields)) {
			$row = new htmlResponsiveRow();
			$pwd1 = new htmlResponsiveInputField($this->getSelfServiceLabel('unicodePwd', _('New password')), 'windowsUser_unicodePwd');
			$pwd1->setIsPassword(true, true);
			$pwd1->setFieldSize(null);
			$row->add($pwd1, 12);
			$pwd2 = new htmlResponsiveInputField(_('Reenter password'), 'windowsUser_unicodePwd2');
			$pwd2->setIsPassword(true);
			$pwd2->setFieldSize(null);
			$pwd2->setSameValueFieldID('windowsUser_unicodePwd');
			$row->add($pwd2, 12);
			$return['unicodePwd'] = $row;
		}
		if ($passwordChangeOnly) {
			return $return; // only password fields as long no LDAP content can be read
		}
		$this->addSimpleSelfServiceTextField($return, 'physicalDeliveryOfficeName', _('Office name'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'telephoneNumber', _('Telephone number'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'facsimileTelephoneNumber', _('Fax number'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'wWWHomePage', _('Web site'), $fields, $attributes, $readOnlyFields);
		if (isset($attributes['street'])) {
			$attributes['streetAddress'] = $attributes['street'];
		}
		$this->addSimpleSelfServiceTextField($return, 'streetAddress', _('Street'), $fields, $attributes, $readOnlyFields, false, true);
		$this->addSimpleSelfServiceTextField($return, 'st', _('State'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'l', _('Location'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'postOfficeBox', _('Post office box'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'postalCode', _('Postal code'), $fields, $attributes, $readOnlyFields);
		$this->addSimpleSelfServiceTextField($return, 'department', _('Department'), $fields, $attributes, $readOnlyFields);
		// last password change
		$row = new htmlResponsiveRow();
		$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('pwdLastSet', _('Last password change'))));
		$row->addField(new htmlOutputText($this->formatPwdLastSet($attributes)));
		$return['pwdLastSet'] = $row;
		// account expiration
		$row = new htmlResponsiveRow();
		$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('accountExpires', _('Account expiration date'))));
		$row->addField(new htmlOutputText($this->formatAccountExpires($attributes)));
		$return['accountExpires'] = $row;
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
		if (in_array('unicodePwd', $fields)) {
			if (isset($_POST['windowsUser_unicodePwd']) && ($_POST['windowsUser_unicodePwd'] != '')) {
				if ($_POST['windowsUser_unicodePwd'] != $_POST['windowsUser_unicodePwd2']) {
					$return['messages'][] = $this->messages['unicodePwd'][0];
				}
				else {
					if (!get_preg($_POST['windowsUser_unicodePwd'], 'password')) {
						$return['messages'][] = $this->messages['unicodePwd'][1];
					}
					else {
						$userName = empty($attributes['userPrincipalName'][0]) ? null : $attributes['userPrincipalName'][0];
						$additionalAttrs = array();
						if (!empty($attributes['sn'][0])) {
							$additionalAttrs[] = $attributes['sn'][0];
						}
						if (!empty($attributes['givenName'][0])) {
							$additionalAttrs[] = $attributes['givenName'][0];
						}
						$pwdPolicyResult = checkPasswordStrength($_POST['windowsUser_unicodePwd'], $userName, $additionalAttrs);
						if ($pwdPolicyResult === true) {
							$this->setSelfServicePassword($return, $attributes);
							$return['info']['userPasswordClearText'][0] = $_POST['windowsUser_unicodePwd'];
						}
						else {
							$return['messages'][] = array('ERROR', $pwdPolicyResult);
						}
					}
				}
			}
		}
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		if (isset($attributes['street'])) {
			$attributes['streetAddress'] = $attributes['street'];
		}
		$this->checkSimpleSelfServiceTextField($return, 'physicalDeliveryOfficeName', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'telephoneNumber', $attributes, $fields, $readOnlyFields, 'telephone');
		$this->checkSimpleSelfServiceTextField($return, 'facsimileTelephoneNumber', $attributes, $fields, $readOnlyFields, 'telephone');
		$this->checkSimpleSelfServiceTextField($return, 'wWWHomePage', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'streetAddress', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'st', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'l', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'postOfficeBox', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'postalCode', $attributes, $fields, $readOnlyFields, 'postalCode');
		$this->checkSimpleSelfServiceTextField($return, 'department', $attributes, $fields, $readOnlyFields);
		return $return;
	}

	/**
	 * Sets the user password in self service.
	 * Since the change requires the old password we need to run ldapmodify for this task.
	 *
	 * Enter description here ...
	 * @param array $return return value for checkSelfServiceOptions() (used to add message if any)
	 * @param array $attributes LDAP attributes
	 */
	private function setSelfServicePassword(&$return, $attributes) {
		if (!function_exists('ldap_modify_batch')) {
			return $this->setSelfServicePasswordCMD($return, $attributes);
		}
		$newPasswordVal = self::pwdAttributeValue($_POST['windowsUser_unicodePwd']);
		$oldPassword = lamDecrypt($_SESSION['selfService_clientPassword'], 'SelfService');
		$oldPasswordVal = self::pwdAttributeValue($oldPassword);
		$dn = $attributes['dn'];
		$operation = array(
			array(
				'attrib' => 'unicodePwd',
				'modtype' => LDAP_MODIFY_BATCH_REMOVE,
				'values' => array($oldPasswordVal)
			),
			array(
				'attrib' => 'unicodePwd',
				'modtype' => LDAP_MODIFY_BATCH_ADD,
				'values' => array($newPasswordVal)
			)
		);
		$success = @ldap_modify_batch($_SESSION['ldapHandle'], $dn, $operation);
		$returnCode = ldap_errno($_SESSION['ldapHandle']);
		if ($returnCode != 0) {
			$outputMessages = htmlspecialchars(getExtendedLDAPErrorMessage($_SESSION['ldapHandle']));
			// Active Directory message translations
			if ((strpos($outputMessages, 'DSID-03190F80') !== false) && (strpos($outputMessages, 'unicodePwd') !== false)) {
				$outputMessages = _('Your password does not meet the password strength qualifications. Please retry with another one.') . '<br><br>' . $outputMessages;
			}
			logNewMessage(LOG_ERR, 'Changing user password failed: ' . $outputMessages);
			$return['messages'][] = array('ERROR', _('Unable to change password.'), $outputMessages);
			return;
		}
		else {
			// update session password for next page load
			$_SESSION['selfService_clientPasswordNew'] = $_POST['windowsUser_unicodePwd'];
		}
	}

	/**
	 * Sets the user password in self service.
	 * Since the change requires the old password we need to run ldapmodify for this task.
	 *
	 * Enter description here ...
	 * @param array $return return value for checkSelfServiceOptions() (used to add message if any)
	 * @param array $attributes LDAP attributes
	 */
	private function setSelfServicePasswordCMD(&$return, $attributes) {
		$newPasswordVal = self::pwdAttributeValue($_POST['windowsUser_unicodePwd']);
		$oldPassword = lamDecrypt($_SESSION['selfService_clientPassword'], 'SelfService');
		$oldPasswordVal = self::pwdAttributeValue($oldPassword);
		$dn = $attributes['dn'];
		$ldif = "dn: " . $dn . "\n";
		$ldif .= "changetype: modify\n";
		$ldif .= "delete: unicodePwd\n";
		$ldif .= "unicodePwd:: " . base64_encode($oldPasswordVal) . "\n";
		$ldif .= "-\n";
		$ldif .= "add: unicodePwd\n";
		$ldif .= "unicodePwd:: " . base64_encode($newPasswordVal) . "\n";
		$ldif .= "-\n";

		$serverURL = $_SESSION['selfServiceProfile']->serverURL;
		$tls = '';
		if ($_SESSION['selfServiceProfile']->useTLS) {
			$tls = ' -ZZ ';
		}
		$cmd = "/usr/bin/ldapmodify -H " . $serverURL . $tls . " -D " . escapeshellarg($dn) . " -x -w " . escapeshellarg($oldPassword);

		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("pipe", "w") // sterr
		);
		$process = proc_open($cmd, $descriptorspec, $pipes);
		if (is_resource($process)) {
			fwrite($pipes[0], $ldif);
		}
		else {
			logNewMessage(LOG_ERR, 'Unable to change password of ' . $dn . '. Calling /usr/bin/ldapmodify failed.');
			$return['messages'][] = array('ERROR', _('Unable to change password.'));
			return;
		}
		fclose($pipes[0]);
		$outputMessages = '';
		while (!feof($pipes[1])) {
			$output = fgets($pipes[1], 1024);
			if ($output != '') {
				$outputMessages .= $output;
			}
		}
		while (!feof($pipes[2])) {
			$output = fgets($pipes[2], 1024);
			if ($output != '') {
				$outputMessages .= $output;
			}
		}
		fclose($pipes[1]);
		$returnCode = proc_close($process);
		if ($returnCode != 0) {
			$outputMessages = htmlspecialchars($outputMessages);
			// Active Directory message translations
			if ((strpos($outputMessages, 'DSID-03190F80') !== false) && (strpos($outputMessages, 'unicodePwd') !== false)) {
				$outputMessages = _('Your password does not meet the password strength qualifications. Please retry with another one.') . '<br><br>' . $outputMessages;
			}
			logNewMessage(LOG_ERR, 'Changing user password failed: ' . $outputMessages);
			$return['messages'][] = array('ERROR', _('Unable to change password.'), $outputMessages);
			return;
		}
		else {
			// update session password for next page load
			$_SESSION['selfService_clientPasswordNew'] = $_POST['windowsUser_unicodePwd'];
		}
	}

	/**
	 * This method specifies if a module manages password attributes. The module alias will
	 * then appear as option in the GUI.
	 * <br>If the module only wants to get notified about password changes then return false.
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		return true;
	}

	/**
	 * Specifies if this module supports to force that a user must change his password on next login.
	 *
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange() {
		return true;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param array $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		// check password strength
		$user = empty($this->attributes['userPrincipalName'][0]) ? null : $this->attributes['userPrincipalName'][0];
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
		$pwdBin = self::pwdAttributeValue($password);
		$this->orig['unicodePwd'][0] = 'unknown';
		$this->attributes['unicodePwd'][0] = $pwdBin;
		$this->attributes['pwdLastSet'][0] = '-1';
		if ($forcePasswordChange) {
			$this->attributes['pwdLastSet'][0] = '0';
		}
		$this->clearTextPassword = $password;
		return array();
	}

	/**
	 * Creates the LDAP password value.
	 *
	 * @param String $password password
	 */
	public static function pwdAttributeValue($password) {
		return iconv('UTF-8', 'UTF-16LE', '"' . $password . '"');
	}

	/**
	 * Returns if the account is currently deactivated.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean is deactivated
	 */
	public static function isDeactivated($attrs) {
		$myAttrs = array_change_key_case($attrs, CASE_LOWER);
		if (!isset($myAttrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($myAttrs['useraccountcontrol'][0]) & windowsUser::AC_ACCOUNT_DISABLED;
	}

	/**
	 * Returns if the account is currently deactivated.
	 *
	 * @param array $attrs LDAP attributes
	 * @param ConfiguredType type
	 * @return DateTime locked till or null
	 */
	public static function getPasswordLocked($attrs, $type) {
		$myAttrs = array_change_key_case($attrs, CASE_LOWER);
		if (empty($myAttrs['lockouttime'][0])) {
			return false;
		}
		$lockoutDuration = self::getDomainLockoutDuration($attrs['dn']);
		if (empty($lockoutDuration) || ($lockoutDuration > -1)) {
			return false;
		}
		$lockoutDurationSeconds = substr($lockoutDuration, 0, -7);
		$lockoutTime = self::getFileTime($myAttrs['lockouttime'][0]);
		$unlockTime = clone $lockoutTime;
		$unlockTime->add(new DateInterval('PT' . abs($lockoutDurationSeconds) . 'S'));
		$now = new DateTime(null, getTimeZone());
		if ($unlockTime > $now) {
			$unlockTime->setTimezone(getTimeZone());
			return $unlockTime;
		}
		return null;
	}

	/**
	 * Returns the domain lockout duration for this DN.
	 *
	 * @param string $dn user DN
	 */
	private static function getDomainLockoutDuration($dn) {
		$lowerDn = strtolower($dn);
		$domainRoot = substr($lowerDn, strpos($lowerDn, 'dc='));
		if (isset(windowsUser::$lockoutDurationCache[$domainRoot])) {
			return windowsUser::$lockoutDurationCache[$domainRoot];
		}
		$policyDN = 'cn=builtin,' . $domainRoot;
		$policyAttrs = ldapGetDN($policyDN, array('lockoutduration'));
		if (!empty($policyAttrs['lockoutduration'][0])) {
			windowsUser::$lockoutDurationCache[$domainRoot] = $policyAttrs['lockoutduration'][0];
		}
		else {
			windowsUser::$lockoutDurationCache[$domainRoot] = null;
		}
		return windowsUser::$lockoutDurationCache[$domainRoot];
	}

	/**
	 * Unlocks the user password. This resets 'lockoutTime' to 0.
	 */
	public function unlockPassword() {
		if (!empty($this->attributes['lockoutTime'][0])) {
			$this->attributes['lockoutTime'][0] = '0';
		}
	}

	/**
	 * Sets if the account is currently deactivated.
	 *
	 * @param boolean $deactivated is deactivated
	 * @param array $attrs LDAP attributes to modify (default $this->attributes)
	 */
	public function setIsDeactivated($deactivated, &$attrs = null) {
		if ($attrs == null) {
			$attrs = &$this->attributes;
		}
		foreach ($attrs as $key => $value) {
			if (strtolower($key) == 'useraccountcontrol') {
				if ($deactivated) {
					$attrs[$key][0] = intval($attrs[$key][0]) | windowsUser::AC_ACCOUNT_DISABLED;
				}
				else {
					if (intval($attrs[$key][0]) & windowsUser::AC_ACCOUNT_DISABLED) {
						$attrs[$key][0] = intval($attrs[$key][0]) - windowsUser::AC_ACCOUNT_DISABLED;
					}
				}
			}
		}
	}

	/**
	 * Returns if the account requires a smartcard to login.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean requires a smartcard
	 */
	public static function isSmartCardRequired($attrs) {
		$myAttrs = array_change_key_case($attrs, CASE_LOWER);
		if (!isset($myAttrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($myAttrs['useraccountcontrol'][0]) & windowsUser::AC_SMARTCARD_REQUIRED;
	}

	/**
	 * Sets if the account requires a smartcard to login.
	 *
	 * @param array $attrs LDAP attributes to modify
	 * @param boolean $requireCard requires a smartcard
	 */
	public static function setIsSmartCardRequired(&$attrs, $requireCard) {
		foreach ($attrs as $key => $value) {
			if (strtolower($key) == 'useraccountcontrol') {
				if ($requireCard) {
					$attrs[$key][0] = intval($attrs[$key][0]) | windowsUser::AC_SMARTCARD_REQUIRED;
				}
				else {
					if (intval($attrs[$key][0]) & windowsUser::AC_SMARTCARD_REQUIRED) {
						$attrs[$key][0] = intval($attrs[$key][0]) - windowsUser::AC_SMARTCARD_REQUIRED;
					}
				}
			}
		}
	}

	/**
	 * Returns if the account never expires.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean never expires
	 */
	public static function isNeverExpiring($attrs) {
		$myAttrs = array_change_key_case($attrs, CASE_LOWER);
		if (!isset($myAttrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($myAttrs['useraccountcontrol'][0]) & windowsUser::AC_PWD_NEVER_EXPIRES;
	}

	/**
	 * Sets if the account never expires.
	 *
	 * @param array $attrs LDAP attributes to modify
	 * @param boolean $neverExpires never expires
	 */
	public static function setIsNeverExpiring(&$attrs, $neverExpires) {
		foreach ($attrs as $key => $value) {
			if (strtolower($key) == 'useraccountcontrol') {
				if ($neverExpires) {
					$attrs[$key][0] = intval($attrs[$key][0]) | windowsUser::AC_PWD_NEVER_EXPIRES;
				}
				else {
					if (intval($attrs[$key][0]) & windowsUser::AC_PWD_NEVER_EXPIRES) {
						$attrs[$key][0] = intval($attrs[$key][0]) - windowsUser::AC_PWD_NEVER_EXPIRES;
					}
				}
			}
		}
	}

	/**
	 * Finds all existing groups.
	 *
	 * @return array group DNs
	 */
	private function findGroups() {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$return = array();
		$types = array('group');
		$results = searchLDAPByFilter('(objectClass=group)', array('dn'), $types);
		$count = sizeof($results);
		for ($i = 0; $i < $count; $i++) {
			if (isset($results[$i]['dn'])) {
				$return[] = $results[$i]['dn'];
			}
		}
		$this->groupCache = $return;
		return $return;
	}

	/**
	 * Gets the list of possible domains from the config setting.
	 *
	 * @return array domain list
	 */
	private function getDomains() {
		$domains = array();
		if (!empty($this->moduleSettings['windowsUser_domains'])) {
			foreach ($this->moduleSettings['windowsUser_domains'] as $domain) {
				$domain = trim(str_replace('@', '', $domain));
				if (!empty($domain)) {
					$domains[] = $domain;
				}
			}
		}
		return array_values(array_unique($domains));
	}

	/**
	 * Returns the formatted value for last password change.
	 *
	 * @param array $attributes user attributes ($this->attributes if null)
	 * @return String last password change or " - "
	 */
	private function formatPwdLastSet($attributes = null) {
		if ($attributes == null) {
			$attributes = &$this->attributes;
		}
		if (!empty($attributes['pwdlastset'][0])) {
			return $this->formatFileTime($attributes['pwdlastset'][0]);
		}
		elseif (empty($attributes['pwdLastSet'][0])) {
			return ' - ';
		}
		return $this->formatFileTime($attributes['pwdLastSet'][0]);
	}

	/**
	 * Returns the formatted value for last login.
	 *
	 * @return String last login or " - "
	 */
	private function formatLastLogonTimestamp() {
		if (empty($this->attributes['lastLogonTimestamp'][0])) {
			return ' - ';
		}
		return $this->formatFileTime($this->attributes['lastLogonTimestamp'][0]);
	}

	/**
	 * Returns the formatted value for the account expiration date.
	 *
	 * @param array $attributes user attributes ($this->attributes if null)
	 * @return String date or -
	 */
	private function formatAccountExpires($attributes = null) {
		if ($attributes == null) {
			$attributes = &$this->attributes;
		}
		if (empty($attributes['accountExpires'][0]) || ($attributes['accountExpires'][0] == '0')
				|| ($attributes['accountExpires'][0] == '9223372036854775807')) {
			return ' - ';
		}
		return $this->formatFileTime($attributes['accountExpires'][0]);
	}

	/**
	 * Formats a value in file time (100 ns since 1601-01-01).
	 *
	 * @param integer $value time value
	 * @return String formatted value
	 */
	private function formatFileTime($value) {
		if (empty($value) || ($value == '-1')) {
			return '';
		}
		$seconds = substr($value, 0, -7);
		$time = new DateTime('1601-01-01', new DateTimeZone('UTC'));
		$time->add(new DateInterval('PT' . $seconds . 'S'));
		$time->setTimezone(getTimeZone());
		return $time->format('Y-m-d H:i:s');
	}

	/**
	 * Returns a value in file time (100 ns since 1601-01-01).
	 *
	 * @param integer $value time value as int
	 * @return DateTime time value
	 */
	public static function getFileTime($value) {
		if (empty($value)) {
			return null;
		}
		$seconds = substr($value, 0, -7);
		$time = new DateTime('1601-01-01', new DateTimeZone('UTC'));
		$time->add(new DateInterval('PT' . $seconds . 'S'));
		$time->setTimezone(getTimeZone());
		return $time;
	}

	/**
	 * Sets the expiration date of this account.
	 * If all parameters are null the expiration date will be removed.
	 *
	 * @param String $year year (e.g. 2040)
	 * @param String $month month (e.g. 8)
	 * @param String $day day (e.g. 27)
	 */
	public function setExpirationDate($year, $month, $day) {
		if (($year == null) && ($month == null) && ($day == null)) {
			unset($this->attributes['accountExpires']);
			return;
		}
		$this->attributes['accountExpires'][0] = $this->buildExpirationDate($year, $month, $day);
	}

	/**
	 * Builds the value for the expiration date.
	 *
	 * @param int $year year
	 * @param int $month month
	 * @param int $day day
	 */
	private function buildExpirationDate($year, $month, $day) {
		$timeBase = new DateTime('1601-01-01', getTimeZone());
		$time = new DateTime("$year-$month-$day", getTimeZone());
		$timeDiff = $time->diff($timeBase);
		$days = $timeDiff->format('%a');
		$seconds = $days * 24 * 3600 - ($time->getOffset());
		return $seconds . '0000000';
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
		// configuration options
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlTableExtendedInputTextarea('windowsUser_domains', '', 30, 3, _('Domains'), 'domains'));
		$configContainer->addElement($configContainerHead, true);
		$configContainer->addVerticalSpace('10px');
		$configHiddenGroup = new htmlGroup();
		$configHiddenGroup->addElement(new htmlOutputText(_('Hidden options')));
		$configHiddenGroup->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainer->addElement($configHiddenGroup, true);
		$configContainerOptions = new htmlTable();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideproxyAddresses', true, _('Proxy-Addresses'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidefacsimileTelephoneNumber', false, _('Fax number'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidemobile', true, _('Mobile'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideotherMobile', true, _('Other mobiles'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidepager', true, _('Pager'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideotherPager', true, _('Other pagers'), null, false), true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidesAMAccountName', true, _('User name (pre W2K)'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidemsSFU30Name', true, _('NIS name'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidemsSFU30NisDomain', true, _('NIS domain'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidepwdLastSet', false, _('Last password change'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidelastLogonTimestamp', false, _('Last login'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidejpegPhoto', true, _('Photo'), null, false), true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidetitle', true, _('Job title'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidecarLicense', true, _('Car license'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideemployeeNumber', true, _('Employee number'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideemployeeType', true, _('Employee type'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidebusinessCategory', true, _('Business category'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidecompany', true, _('Company'), null, false), true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidedepartment', true, _('Department'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideou', true, _('Organisational unit'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hideo', true, _('Organisation'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsUser_hidemanager', true, _('Manager'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		$advancedOptions = new htmlTable();
		$advancedOptions->addElement(new htmlSubTitle(_('Photo')), true);
		$photoTable = new htmlTable();
		$photoTable->colspan = 2;
		if (extension_loaded('imagick')) {
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum width (px)'), 'windowsUser_jpegPhoto_maxWidth', null, 'crop'), true);
			$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum height (px)'), 'windowsUser_jpegPhoto_maxHeight', null, 'crop'), true);
		}
		$photoTable->addElement(new htmlTableExtendedInputField(_('Maximum file size (kB)'), 'windowsUser_jpegPhoto_maxSize'), true);
		$advancedOptions->addElement($photoTable, true);
		$advancedOptionsAccordion = new htmlAccordion('inetOrgPersonAdvancedOptions', array(_('Advanced options') => $advancedOptions), false);
		$advancedOptionsAccordion->colspan = 5;
		$configContainer->addElement($advancedOptionsAccordion);
		return $configContainer;
	}

	/**
	 * Loads cached data from LDAP such as departmets etc.
	 */
	private function initCache() {
		if ($this->departmentCache != null) {
			return;
		}
		$attrs = array();
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$attrs[] = 'department';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$attrs[] = 'ou';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$attrs[] = 'o';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$attrs[] = 'title';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$attrs[] = 'employeeType';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$attrs[] = 'businessCategory';
		}
		$departments = array();
		$ous = array();
		$os = array();
		$titles = array();
		$employeeTypes = array();
		$businessCategories = array();
		if (sizeof($attrs) > 0) {
			$result = searchLDAPByFilter('(objectClass=user)', $attrs, array($this->get_scope()));
			foreach ($result as $attributes) {
				if (isset($attributes['department'])) {
					foreach ($attributes['department'] as $val) {
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
	 * Returns a list of jobs that can be run.
	 *
	 * @param LAMConfig $config configuration
	 * @return array list of jobs
	 */
	public function getSupportedJobs(&$config) {
		return array(
			new WindowsPasswordNotifyJob(),
			new WindowsAccountExpirationCleanupJob()
		);
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
		if (!empty($_POST['userPrincipalName'])) {
			$replacements['user'] = $_POST['userPrincipalName'];
		}
		elseif (!empty($this->attributes['userPrincipalName'][0])) {
			$replacements['user'] = $this->attributes['userPrincipalName'][0];
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

if (interface_exists('\LAM\JOB\Job', false)) {

	include_once dirname(__FILE__) . '/../passwordExpirationJob.inc';

	/**
	 * Job to notify users about password expiration.
	 *
	 * @package jobs
	 */
	class WindowsPasswordNotifyJob extends \LAM\JOB\PasswordExpirationJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Windows') . ': ' . _('Notify users about password expiration');
		}

		/**
		 * Returns a list of password policy options. The options are later passed to function checkSingleUser().
		 * This reads the password policy to calculate expiration times later. Maxpwdage is the maximum amount of
		 * time, in 100-nanosecond intervals, a password is valid.
		 *
		 * @return array options ('maxpwdage' => max age in ns)
		 */
		protected function getPolicyOptions() {
			$typeManager = new TypeManager();
			$maxPwdAge = array();
			foreach ($typeManager->getConfiguredTypesForScope('user') as $type) {
				$userSuffix = $type->getSuffix();
				if (empty($userSuffix)) {
					logNewMessage(LOG_ERR, 'No user suffix set in server profile for ' . $type->getAlias() . '.');
					continue;
				}
				// extract base DN from user suffix
				$domainRoot = strtolower(substr($userSuffix, stripos($userSuffix, 'dc=')));
				if (empty($domainRoot)) {
					logNewMessage(LOG_ERR, "No domain root found in $userSuffix.");
					continue;
				}
				logNewMessage(LOG_DEBUG, "Using $domainRoot as domain root");
				$policyDN = 'cn=builtin,' . $domainRoot;
				$policyAttrs = ldapGetDN($policyDN, array('maxPwdAge'));
				if (empty($policyAttrs['maxpwdage'][0])) {
					logNewMessage(LOG_ERR, 'No maxPwdAge found for this domain in ' . $type->getAlias() . '.');
					continue;
				}
				$maxPwdAge[$domainRoot] = $policyAttrs['maxpwdage'][0];
				logNewMessage(LOG_DEBUG, "Using maxPwdAge = " . $maxPwdAge[$domainRoot] . ".");
			}
			return array('maxpwdage' => $maxPwdAge);
		}

		/**
		 * Searches for users in LDAP.
		 *
		 * @param String $jobID unique job identifier
		 * @param array $options config options (name => value)
		 * @return array list of user attributes
		 */
		protected function findUsers($jobID, $options) {
			// read users
			$sysattrs = array('mail', 'pwdLastSet', 'accountExpires', 'useraccountcontrol');
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			$userResults = searchLDAPByFilter('(&(pwdLastSet=*)(mail=*))', $attrs, array('user'));
			return $userResults;
		}

		/**
		 * Checks if a user needs to change his password.
		 *
		 * @param integer $jobID job ID
		 * @param array $options job settings
		 * @param PDO $pdo PDO
		 * @param DateTime $now current time
		 * @param array $policyOptions list of max age values (policy DN => maxAge)
		 * @param array $user user attributes
		 * @param boolean $isDryRun just do a dry run, nothing is modified
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			$dn = $user['dn'];
			$domainRoot = strtolower(substr($dn, stripos($dn, 'dc=')));
			// skip if password does not expire at all
			if (windowsUser::isNeverExpiring($user)) {
				logNewMessage(LOG_DEBUG, $dn . ' does not expire.');
				return;
			}
			// skip if no information about last password change
			if (empty($user['pwdlastset'][0]) || ($user['pwdlastset'][0] < 1)) {
				logNewMessage(LOG_DEBUG, $dn . ' has no valid "pwdLastSet".');
				return;
			}
			// skip if account itself is expired
			if (!empty($user['accountexpires'][0])) {
				$accountExpiration = windowsUser::getFileTime($user['accountexpires'][0]);
				if ($accountExpiration <= $now) {
					logNewMessage(LOG_DEBUG, $dn . ' already expired');
					return;
				}
			}
			// skip if account is deactivated
			if (windowsUser::isDeactivated($user)) {
				logNewMessage(LOG_DEBUG, $dn . ' is deactivated.');
				return;
			}
			$maxPwdAge = $policyOptions['maxpwdage'][$domainRoot];
			// calculate time when password expires
			$lastPwdTime = windowsUser::getFileTime($user['pwdlastset'][0]);
			logNewMessage(LOG_DEBUG, "Last password change on " . $lastPwdTime->format('Y-m-d'));
			$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			logNewMessage(LOG_DEBUG, "Number of days before warning " . $numDaysToWarn);
			// expiration date = pwdLastSet - maxpwdage
			$expireTime = windowsUser::getFileTime($user['pwdlastset'][0] - $maxPwdAge);
			logNewMessage(LOG_DEBUG, "Password expires on " . $expireTime->format('Y-m-d'));
			// skip already expired accounts
			if ($expireTime <= $now) {
				logNewMessage(LOG_DEBUG, $dn . ' already expired');
				return;
			}
			// calculate time of notification
			$notifyTime = clone $expireTime;
			$notifyTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			$notifyTime->setTimeZone(getTimeZone());
			logNewMessage(LOG_DEBUG, "Password notification on " . $notifyTime->format('Y-m-d H:i'));
			// skip if notification is in the future
			if ($notifyTime > $now) {
				logNewMessage(LOG_DEBUG, $dn . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $dn);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['pwdlastset'][0]) {
				logNewMessage(LOG_DEBUG, $dn . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				logNewMessage(LOG_NOTICE, 'Not sending email to ' . $dn . ' because of dry run.');
				return;
			}
			// send email
			$success = $this->sendMail($options, $jobID, $user, $expireTime);
			// update DB if mail was sent successfully
			if ($success) {
				$this->setDBLastPwdChangeTime($jobID, $pdo, $dn, $user['pwdlastset'][0]);
			}
		}

	}

	/**
	 * Job to delete or move users on account expiration.
	 *
	 * @package jobs
	 */
	class WindowsAccountExpirationCleanupJob extends \LAM\JOB\AccountExpirationCleanupJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Windows') .  ': ' . _('Cleanup expired user accounts');
		}

		/**
		 * Returns the description of the job.
		 *
		 * @return String description
		 */
		public function getDescription() {
			return _('This job deletes or moves user accounts when they expire.');
		}

		/**
		 * Searches for users in LDAP.
		 *
		 * @param String $jobID unique job identifier
		 * @param array $options config options (name => value)
		 * @return array list of user attributes
		 */
		protected function findUsers($jobID, $options) {
			// read users
			$attrs = array('accountExpires');
			$userResults = searchLDAPByFilter('(accountExpires=*)', $attrs, array('user'));
			return $userResults;
		}

		/**
		 * Checks if a user is expired.
		 *
		 * @param integer $jobID job ID
		 * @param array $options job settings
		 * @param PDO $pdo PDO
		 * @param DateTime $now current time
		 * @param array $policyOptions list of policy options by getPolicyOptions()
		 * @param array $user user attributes
		 * @param boolean $isDryRun just do a dry run, nothing is modified
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			$seconds = substr($user['accountexpires'][0], 0, -7);
			$expireTime = new DateTime('1601-01-01', new DateTimeZone('UTC'));
			$expireTime->add(new DateInterval('PT' . $seconds . 'S'));
			$expireTime->setTimezone(getTimeZone());
			logNewMessage(LOG_DEBUG, "Expiration on " . $expireTime->format('Y-m-d'));
			$delay = 0;
			if (!empty($options[$this->getConfigPrefix() . '_delay' . $jobID][0])) {
				$delay = $options[$this->getConfigPrefix() . '_delay' . $jobID][0];
			}
			$actionTime = clone $expireTime;
			if ($delay != 0) {
				$actionTime->add(new DateInterval('P' . $delay . 'D'));
			}
			$actionTime->setTimeZone(getTimeZone());
			logNewMessage(LOG_DEBUG, "Action time on " . $actionTime->format('Y-m-d'));
			if ($actionTime <= $now) {
				$this->performAction($jobID, $options, $user, $isDryRun);
			}
		}

	}

}

?>
