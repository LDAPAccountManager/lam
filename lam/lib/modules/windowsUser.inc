<?php

use \LAM\TYPES\TypeManager;
use LAM\TYPES\ConfiguredType;
use LAM\ImageUtils\ImageManipulationFactory;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2024  Roland Gruber

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
class windowsUser extends baseModule implements passwordService, AccountStatusProvider {

	/**
	 * ID for locked password status.
	 */
	const STATUS_PASSWORD_LOCKED = "WINDOWS_PASSWORD_LOCKED";

	/**
	 * ID for locked account status.
	 */
	const STATUS_ACCOUNT_LOCKED = "WINDOWS_ACCOUNT_LOCKED";

	/**
	 * ID for expired account status.
	 */
	const STATUS_ACCOUNT_EXPIRED = "WINDOWS_ACCOUNT_EXPIRED";

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['userPrincipalName', 'sAMAccountName', 'displayName',
		'profilePath', 'scriptPath', 'pwdLastSet', 'otherMailbox', 'homeDirectory', 'msSFU30Name',
		'pwdLastSet', 'lastLogonTimestamp', 'accountexpires', 'lockouttime'];

	/** initial account flags */
	const DEFAULT_ACCOUNT_CONTROL = 0x00000200;
	/** password never expires */
	const AC_PWD_NEVER_EXPIRES = 0x00010000;
	/** login requires smartcard */
	const AC_SMARTCARD_REQUIRED = 0x00040000;
	/** account is disabled */
	const AC_ACCOUNT_DISABLED = 0x00000002;

	/** display groups as dn */
	const DISPLAY_GROUPS_DN = 'DN';
	/** display groups as cn */
	const DISPLAY_GROUPS_CN = 'CN';

	/** current group list */
	private $groupList = [];
	/** original group list */
	private $groupList_orig = [];
	/** cache for groups */
	private $groupCache;
	/** host cache to reduce LDAP queries */
	private $cachedHostList;
	/** option for forcing password change, used in postModifyActions */
	private $pwdLastSet;
	/** clear text password */
	private $clearTextPassword;
	/** cache for departments */
	private $departmentCache;
	/** cache for department numbers */
	private $departmentNumberCache;
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

	/** cache for user name */
	private $cachedUserNameList;
	/** cache for cn */
	private $cachedCnList;


	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return in_array($this->get_scope(), ['user']);
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	public function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'samba.svg';
		// this is a base module
		$return["is_base"] = true;
		// PHP extensions
		$return['extensions'] = ['iconv'];
		// RDN attribute
		$return["RDN"] = ["cn" => "high"];
		// LDAP filter
		$return["ldap_filter"] = ['and' => '(!(objectClass=computer))', 'or' => "(objectClass=user)"];
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['user', 'securityPrincipal'];
		// managed attributes
		$return['attributes'] = ['userPrincipalName', 'cn', 'sAMAccountName', 'description', 'displayName', 'givenName', 'initials',
			'l', 'mail', 'otherTelephone', 'physicalDeliveryOfficeName', 'postalCode', 'postOfficeBox', 'sn', 'st',
			'streetAddress', 'telephoneNumber', 'url', 'wWWHomePage', 'useraccountcontrol', 'profilePath', 'scriptPath',
			'pwdLastSet', 'otherMailbox', 'homeDirectory', 'homeDrive', 'msSFU30Name', 'msSFU30NisDomain', 'pwdLastSet',
			'lastLogonTimestamp', 'accountexpires', 'jpegPhoto', 'title', 'carLicense', 'employeeNumber', 'employeeType',
			'businessCategory', 'department', 'departmentNumber', 'ou', 'o', 'manager', 'facsimileTelephoneNumber', 'company',
			'pager', 'otherPager', 'mobile', 'otherMobile', 'proxyAddresses', 'lockouttime', 'userWorkstations'
		];
		// help Entries
		$return['help'] = [
			'cn' => [
				"Headline" => _('Common name'), 'attr' => 'cn',
				"Text" => _('This is the natural name of the user. If empty, the first and last name or user name is used.')
			],
			'userPrincipalName' => [
				"Headline" => _('User name'), 'attr' => 'userPrincipalName',
				"Text" => _('Please enter the user\'s name.')
			],
			'userPrincipalNameDomain' => [
				"Headline" => _('Domain'), 'attr' => 'userPrincipalName',
				"Text" => _('Windows domain name of account.')
			],
			'sAMAccountName' => [
				"Headline" => _('User name (pre W2K)'), 'attr' => 'sAMAccountName',
				"Text" => _('Please enter the user\'s name.') . ' ' . _('This user name is only used for old Windows versions (e.g. NT4, W98).')
			],
			'description' => [
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Please enter a descriptive text for this user.')
			],
			'displayName' => [
				"Headline" => _('Display name'), 'attr' => 'displayName',
				"Text" => _('This is the account\'s full name on Windows systems.')
			],
			'givenName' => [
				"Headline" => _('First name'), 'attr' => 'givenName',
				"Text" => _('First name of user. Only letters, - and spaces are allowed.')
			],
			'initials' => [
				"Headline" => _('Initials'), 'attr' => 'initials',
				"Text" => _('The initials of the user\'s first names.')
			],
			'l' => [
				"Headline" => _('Location'), 'attr' => 'l',
				"Text" => _('This describes the location of the user.')
			],
			'mail' => [
				"Headline" => _('Email address'), 'attr' => 'mail',
				"Text" => _('The user\'s email address.')
			],
			'otherTelephone' => [
				"Headline" => _('Other telephone numbers'), 'attr' => 'otherTelephone',
				"Text" => _('If the user has multiple telephone numbers then please enter it here.')
			],
			'otherTelephoneList' => [
				"Headline" => _('Other telephone numbers'), 'attr' => 'otherTelephone',
				"Text" => _('If the user has multiple telephone numbers then please enter it here.') . ' ' . _("Multiple values are separated by semicolon.")
			],
			'physicalDeliveryOfficeName' => [
				"Headline" => _('Office name'), 'attr' => 'physicalDeliveryOfficeName',
				"Text" => _('The office name of the user (e.g. YourCompany, Human Resources).')
			],
			'postalCode' => [
				"Headline" => _('Postal code'), 'attr' => 'postalCode',
				"Text" => _('The postal code of the user\'s address.')
			],
			'postOfficeBox' => [
				"Headline" => _('Post office box'), 'attr' => 'postOfficeBox',
				"Text" => _('The post office box of the user\'s address.')
			],
			'sn' => [
				"Headline" => _('Last name'), 'attr' => 'sn',
				"Text" => _('Last name of user. Only letters, - and spaces are allowed.')
			],
			'st' => [
				"Headline" => _('State'), 'attr' => 'st',
				"Text" => _('The state where the user resides or works.')
			],
			'streetAddress' => [
				"Headline" => _('Street'), 'attr' => 'streetAddress',
				"Text" => _('The street name of the user\'s address.')
			],
			'telephoneNumber' => [
				"Headline" => _('Telephone number'), 'attr' => 'telephoneNumber',
				"Text" => _('The user\'s telephone number.')
			],
			'facsimileTelephoneNumber' => [
				"Headline" => _("Fax number"), 'attr' => 'facsimileTelephoneNumber',
				"Text" => _("The user's fax number.")
			],
			'url' => [
				"Headline" => _('Other web sites'), 'attr' => 'url',
				"Text" => _('Here you can enter additional web sites for the user.')
			],
			'urlList' => [
				"Headline" => _('Other web sites'), 'attr' => 'url',
				"Text" => _('Here you can enter additional web sites for the user.') . ' ' . _("Multiple values are separated by semicolon.")
			],
			'wWWHomePage' => [
				"Headline" => _('Web site'), 'attr' => 'wWWHomePage',
				"Text" => _('The user\'s web site (e.g. http://www.company.com).')
			],
			"deactivated" => [
				"Headline" => _("Account is deactivated"),
				"Text" => _("If checked then the account will be deactivated.")],
			"noExpire" => [
				"Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire.")],
			"requireCard" => [
				"Headline" => _("Require smartcard"),
				"Text" => _("The user must log on using a smart card.")],
			"profilePath" => [
				"Headline" => _("Profile path"), 'attr' => 'profilePath',
				"Text" => _('Path of the user profile (UNC-path, e.g. \\\\server\\share\\user).') . ' ' . _("Can be left empty.")
			],
			"scriptPath" => [
				"Headline" => _("Logon script"), 'attr' => 'scriptPath',
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon.') . ' ' . _("Can be left empty.")
			],
			"pwdMustChange" => [
				"Headline" => _("Password change at next login"),
				"Text" => _("If you set this option then the user has to change his password at the next login.")],
			"groups" => [
				"Headline" => _("Groups"),
				"Text" => _("Hold the CTRL-key to (de)select multiple groups.")],
			"groupsUpload" => [
				"Headline" => _("Groups"),
				"Text" => _("The groups for this account. You can insert a group name or DN.") . ' ' . _("Multiple values are separated by semicolon.")],
			'password' => [
				"Headline" => _("Password"),
				"Text" => _("Please enter the password which you want to set for this account.")
			],
			'otherMailbox' => [
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.")
			],
			'otherMailboxList' => [
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.") . ' ' . _("Multiple values are separated by semicolon.")
			],
			'hiddenOptions' => [
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			],
			'domains' => [
				"Headline" => _('Domains'),
				"Text" => _('Please enter a list of Windows domains that can be selected for your user accounts.')
			],
			"homeDrive" => [
				"Headline" => _("Home drive"), 'attr' => 'homeDrive',
				"Text" => _("The home directory will be connected under this drive letter.")],
			"homeDirectory" => [
				"Headline" => _("Home directory"), 'attr' => 'homeDirectory',
				"Text" => _('UNC-path (\\\\server\\share\) of home directory. If no home drive is set then this directory must start with a drive letter (e.g. "c:\dir\user").')
			],
			'msSFU30Name' => [
				"Headline" => _('NIS name'), 'attr' => 'msSFU30Name',
				"Text" => _('User name for NIS.')
			],
			'msSFU30NisDomain' => [
				"Headline" => _('NIS domain'), 'attr' => 'msSFU30NisDomain',
				"Text" => _('NIS domain name.')
			],
			'pwdLastSet' => [
				"Headline" => _('Last password change'), 'attr' => 'pwdLastSet',
				"Text" => _('Time of user\'s last password change.')
			],
			'lastLogonTimestamp' => [
				"Headline" => _('Last login'), 'attr' => 'lastLogonTimestamp',
				"Text" => _('Time of user\'s last login.')
			],
			'accountexpires' => [
				"Headline" => _('Account expiration date'), 'attr' => 'accountExpires',
				"Text" => _('This is the date when the account will expire.')
			],
			'accountexpiresUpload' => [
				"Headline" => _('Account expiration date'), 'attr' => 'accountExpires',
				"Text" => _('This is the date when the account will expire. Format: DD-MM-YYYY')
			],
			'accountexpiresProfile' => [
				"Headline" => _('Account expiration'), 'attr' => 'accountExpires',
				"Text" => _('Number of days after which the account will expire.')
			],
			'jpegPhoto' => [
				"Headline" => _("Add photo"), 'attr' => 'jpegPhoto',
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			],
			'title' => [
				"Headline" => _("Job title"), 'attr' => 'title',
				"Text" => _("Job title of user: President, department manager, ...")
			],
			'carLicense' => [
				"Headline" => _("Car license"), 'attr' => 'carLicense',
				"Text" => _("This can be used to specify if the user has a car license.")
			],
			'employeeNumber' => [
				"Headline" => _("Employee number"), 'attr' => 'employeeNumber',
				"Text" => _("The user's unique employee number.")
			],
			'employeeType' => [
				"Headline" => _("Employee type"), 'attr' => 'employeeType',
				"Text" => _("Employee type: Contractor, Employee, Intern, Temp, External, ...")
			],
			'businessCategory' => [
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Management, ...)")
			],
			'businessCategoryList' => [
				"Headline" => _("Business category"), 'attr' => 'businessCategory',
				"Text" => _("Business category (e.g. Administration, IT-Services, Management, ...)") . '. ' . _("Multiple values are separated by semicolon.")
			],
			'department' => [
				"Headline" => _("Department"), 'attr' => 'department',
				"Text" => _("Here you can enter the user's department.")
			],
			'departmentNumber' => [
				"Headline" => _("Department number"), 'attr' => 'departmentNumber',
				"Text" => _("Here you can enter the user's department number.")
			],
			'departmentNumberList' => [
				"Headline" => _("Department number"), 'attr' => 'departmentNumber',
				"Text" => _("Here you can enter the user's department number.") . ' ' . _("Multiple values are separated by semicolon.")
			],
			'ou' => [
				"Headline" => _("Organisational unit"), 'attr' => 'ou',
				"Text" => _("The user's organisational unit.")
			],
			'ouList' => [
				"Headline" => _("Organisational unit"), 'attr' => 'ou',
				"Text" => _("The user's organisational unit.") . ' ' . _('Multiple values are separated by semicolon.')
			],
			'o' => [
				"Headline" => _("Organisation"), 'attr' => 'o',
				"Text" => _("The user's organisation name.")
			],
			'oList' => [
				"Headline" => _("Organisation"), 'attr' => 'o',
				"Text" => _("The user's organisation name.") . ' ' . _('Multiple values are separated by semicolon.')
			],
			'manager' => [
				"Headline" => _("Manager"), 'attr' => 'manager',
				"Text" => _("This is the LDAP DN of the user's manager. Use this property to represent hierarchies in your company.")
			],
			'company' => [
				"Headline" => _('Company'), 'attr' => 'company',
				"Text" => _('Please enter the company name.')
			],
			'pager' => [
				"Headline" => _('Pager'), 'attr' => 'pager',
				"Text" => _('The user\'s pager number.')
			],
			'otherPager' => [
				"Headline" => _('Other pagers'), 'attr' => 'otherPager',
				"Text" => _('Any secondary pager numbers.')
			],
			'otherPagerUpload' => [
				"Headline" => _('Other pagers'), 'attr' => 'otherPager',
				"Text" => _('Any secondary pager numbers.') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'mobile' => [
				"Headline" => _('Mobile'), 'attr' => 'mobile',
				"Text" => _('The user\'s mobile number.')
			],
			'otherMobile' => [
				"Headline" => _('Other mobiles'), 'attr' => 'otherMobile',
				"Text" => _('Any secondary mobile numbers.')
			],
			'otherMobileUpload' => [
				"Headline" => _('Other mobiles'), 'attr' => 'otherMobile',
				"Text" => _('Any secondary mobile numbers.') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'proxyAddresses' => [
				"Headline" => _('Proxy-Addresses'), 'attr' => 'proxyAddresses',
				"Text" => _('Use this to enter additional email addresses in format "smtp:user@domain.com".')
			],
			'proxyAddressesUpload' => [
				"Headline" => _('Proxy-Addresses'), 'attr' => 'proxyAddresses',
				"Text" => _('Use this to enter additional email addresses in format "smtp:user@domain.com".') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'crop' => [
				"Headline" => _('Image cropping'),
				"Text" => _('Uploaded images will be cropped to these maximum values.')
			],
			'photoUpload' => [
				"Headline" => _("Add photo"), 'attr' => 'jpegPhoto',
				"Text" => _("Please select an image file to upload. It must be in JPG format (.jpg/.jpeg).")
			],
			'excludeFromGroupSync' => [
				"Headline" => _('Exclude from group sync'),
				"Text" => _('Enter one group per line that should be ignored when syncing groups.')
			],
			"userWorkstations" => [
				"Headline" => _("Workstations"), 'attr' => 'userWorkstations',
				"Text" => _("List of workstations the user is allowed to login. Empty means every workstation.")
			],
			"workstations" => [
				"Headline" => _("Workstations"), 'attr' => 'userWorkstations',
				"Text" => _("Comma separated list of workstations the user is allowed to login. Empty means every workstation.") . ' ' . _("Can be left empty.")
			],
			'displayGroups' => [
				"Headline" => _('Display format'),
				"Text" => _('Specifies how groups are displayed.')
			],
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'windowsUser_userPrincipalName',
				'description' => _('User name'),
				'help' => 'userPrincipalName',
				'example' => _('smiller'),
				'required' => true,
				'unique' => true,
			],
			[
				'name' => 'windowsUser_password',
				'description' => _('Password'),
				'help' => 'password',
				'example' => _('secret'),
			],
			[
				'name' => 'windowsUser_firstName',
				'description' => _('First name'),
				'help' => 'givenName',
				'example' => _('Steve'),
			],
			[
				'name' => 'windowsUser_lastName',
				'description' => _('Last name'),
				'help' => 'sn',
				'example' => _('Miller'),
			],
			[
				'name' => 'windowsUser_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller'),
			],
			[
				'name' => 'windowsUser_deactivated',
				'description' => _('Account is deactivated'),
				'help' => 'deactivated',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			],
			[
				'name' => 'windowsUser_noExpire',
				'description' => _('Password does not expire'),
				'help' => 'noExpire',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			],
			[
				'name' => 'windowsUser_accountExpires',
				'description' => _('Account expiration date'),
				'help' => 'accountexpiresUpload',
				'example' => '21-11-2030',
			],
			[
				'name' => 'windowsUser_pwdMustChange',
				'description' => _('Password change at next login'),
				'help' => 'pwdMustChange',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			],
			[
				'name' => 'windowsUser_groups',
				'description' => _('Groups'),
				'help' => 'groupsUpload',
			],
		];
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_displayName',
				'description' => _('Display name'),
				'help' => 'displayName',
				'example' => _('Steve Miller'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideinitials')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_initials',
				'description' => _('Initials'),
				'help' => 'initials',
				'example' => _('S.M.'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedescription')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Temp, contract till December'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_streetAddress',
				'description' => _('Street'),
				'help' => 'streetAddress',
				'example' => _('Mystreetname 42'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_postOfficeBox',
				'description' => _('Post office box'),
				'help' => 'postOfficeBox',
				'example' => _('12345'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_postalCode',
				'description' => _('Postal code'),
				'help' => 'postalCode',
				'example' => _('GB-12345'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_l',
				'description' => _('Location'),
				'help' => 'l',
				'example' => _('MyCity'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_state',
				'description' => _('State'),
				'help' => 'st',
				'example' => _('New York'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_officeName',
				'description' => _('Office name'),
				'help' => 'physicalDeliveryOfficeName',
				'example' => _('YourCompany'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('user@company.com'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_otherMailbox',
				'description' => _('Email alias'),
				'help' => 'otherMailboxList',
				'example' => _('user@company.com'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetelephoneNumber')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_telephoneNumber',
				'description' => _('Telephone number'),
				'help' => 'telephoneNumber',
				'example' => _('123-124-1234'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherTelephone')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_otherTelephone',
				'description' => _('Other telephone numbers'),
				'help' => 'otherTelephoneList',
				'example' => _('123-124-1234'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidewWWHomePage')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_webSite',
				'description' => _('Web site'),
				'help' => 'wWWHomePage',
				'example' => _('http://www.company.com'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideurl')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_otherWebSites',
				'description' => _('Other web sites'),
				'help' => 'urlList',
				'example' => _('http://www.company.com'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_proxyAddresses',
				'description' => _('Proxy-Addresses'),
				'help' => 'proxyAddressesUpload',
				'example' => _('smtp:user@example.com')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_facsimileTelephoneNumber',
				'description' => _('Fax number'),
				'help' => 'facsimileTelephoneNumber',
				'example' => _('123-123-1236')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepager', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_pager',
				'description' => _('Pager'),
				'help' => 'pager',
				'example' => _('123-123-1236')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_otherPager',
				'description' => _('Other pagers'),
				'help' => 'otherPagerUpload',
				'example' => _('123-123-1236')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemobile', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_mobile',
				'description' => _('Mobile'),
				'help' => 'mobile',
				'example' => _('123-123-1235')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_otherMobile',
				'description' => _('Other mobiles'),
				'help' => 'otherMobileUpload',
				'example' => _('123-123-1235')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_company',
				'description' => _('Company'),
				'help' => 'company',
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_sAMAccountName',
				'description' => _('User name (pre W2K)'),
				'help' => 'sAMAccountName',
				'example' => _('smiller'),
				'unique' => true,
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_msSFU30Name',
				'description' => _('NIS name'),
				'help' => 'msSFU30Name',
				'example' => _('smiller'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_msSFU30NisDomain',
				'description' => _('NIS domain'),
				'help' => 'msSFU30NisDomain',
				'example' => _('domain'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_title',
				'description' => _('Job title'),
				'help' => 'title',
				'example' => _('President'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecarLicense', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_carLicense',
				'description' => _('Car license'),
				'help' => 'carLicense',
				'example' => _('yes'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeNumber', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_employeeNumber',
				'description' => _('Employee number'),
				'help' => 'employeeNumber',
				'example' => '123456',
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_employeeType',
				'description' => _('Employee type'),
				'help' => 'employeeType',
				'example' => _('Temp'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_businessCategory',
				'description' => _('Business category'),
				'help' => 'businessCategoryList',
				'example' => _('Administration'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_department',
				'description' => _('Department'),
				'help' => 'department',
				'example' => _('Administration'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_departmentNumber',
				'description' => _('Department number'),
				'help' => 'departmentNumberList',
				'example' => 'A123',
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_ou',
				'description' => _('Organisational unit'),
				'help' => 'ouList',
				'example' => _('Administration'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_o',
				'description' => _('Organisation'),
				'help' => 'oList',
				'example' => _('YourCompany'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_manager',
				'description' => _('Manager'),
				'help' => 'manager',
				'example' => _('uid=smiller,ou=People,dc=company,dc=com'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideWorkstations')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_workstations',
				'description' => _('Workstations'),
				'help' => 'workstations',
				'example' => 'PC01,PC02,PC03'
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_profilePath',
				'description' => _('Profile path'),
				'help' => 'profilePath',
				'example' => _('\\\\server\\profiles\\smiller'),
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_scriptPath',
				'description' => _('Logon script'),
				'help' => 'scriptPath',
				'example' => 'logon.bat',
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_homeDrive',
				'description' => _('Home drive'),
				'help' => 'homeDrive',
				'example' => 'K:'
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_homeDirectory',
				'description' => _('Home directory'),
				'help' => 'homeDirectory',
				'example' => _('\\\\server\\homes\\smiller')
			];
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideRequireSmartcard')) {
			$return['upload_columns'][] = [
				'name' => 'windowsUser_requireCard',
				'description' => _('Require smartcard'),
				'help' => 'requireCard',
				'example' => _('no'),
				'default' => _('no'),
				'values' => _('yes') . ', ' . _('no')
			];
		}
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Common name'), 'windowsUser_cn', null, 'cn'), 12);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Display name'), 'windowsUser_displayName', null, 'displayName'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Street'), 'windowsUser_streetAddress', null, 'streetAddress'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Post office box'), 'windowsUser_postOfficeBox', null, 'postOfficeBox'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Postal code'), 'windowsUser_postalCode', null, 'postalCode'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Location'), 'windowsUser_l', null, 'l'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
			$profileContainer->add(new htmlResponsiveInputField(_('State'), 'windowsUser_st', null, 'st'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Office name'), 'windowsUser_physicalDeliveryOfficeName', null, 'physicalDeliveryOfficeName'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Email address'), 'windowsUser_mail', null, 'mail'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Email alias'), 'windowsUser_otherMailbox', null, 'otherMailboxList'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Logon script'), 'windowsUser_scriptPath', null, 'scriptPath'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Profile path'), 'windowsUser_profilePath', null, 'profilePath'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory')) {
			$profileContainer->add(new htmlResponsiveInputField(_('Home directory'), 'windowsUser_homeDirectory', null, 'homeDirectory'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('NIS domain'), 'windowsUser_msSFU30NisDomain', null, 'msSFU30NisDomain'), 12);
		}
		$profileContainer->add(new htmlResponsiveInputField(_('Account expiration'), 'windowsUser_accountExpires', null, 'accountexpiresProfile'), 12);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidecompany', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Company'), 'windowsUser_company', null, 'company'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Department'), 'windowsUser_department', null, 'department'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Department number'), 'windowsUser_departmentNumber', null, 'departmentNumberList'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Organisational unit'), 'windowsUser_ou', null, 'ouList'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Organisation'), 'windowsUser_o', null, 'oList'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Job title'), 'windowsUser_title', null, 'title'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Employee type'), 'windowsUser_employeeType', null, 'employeeType'), 12);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$profileContainer->add(new htmlResponsiveInputField(_('Business category'), 'windowsUser_businessCategory', null, 'businessCategory'), 12);
		}
		$return['profile_options'] = $profileContainer;
		// profile mappings
		$return['profile_mappings'] = [
			'windowsUser_cn' => 'cn',
			'windowsUser_streetAddress' => 'streetAddress',
			'windowsUser_company' => 'company',
		];
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$return['profile_mappings']['windowsUser_displayName'] = 'displayName';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideWorkstations')) {
			$return['profile_mappings']['windowsUser_userWorkstations'] = 'userWorkstations';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$return['profile_mappings']['windowsUser_msSFU30NisDomain'] = 'msSFU30NisDomain';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
			$return['profile_mappings']['windowsUser_postOfficeBox'] = 'postOfficeBox';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
			$return['profile_mappings']['windowsUser_postalCode'] = 'postalCode';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
			$return['profile_mappings']['windowsUser_l'] = 'l';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
			$return['profile_mappings']['windowsUser_st'] = 'st';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
			$return['profile_mappings']['windowsUser_physicalDeliveryOfficeName'] = 'physicalDeliveryOfficeName';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail')) {
			$return['profile_mappings']['windowsUser_mail'] = 'mail';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath')) {
			$return['profile_mappings']['windowsUser_scriptPath'] = 'scriptPath';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath')) {
			$return['profile_mappings']['windowsUser_profilePath'] = 'profilePath';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory')) {
			$return['profile_mappings']['windowsUser_homeDirectory'] = 'homeDirectory';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive')) {
			$return['profile_mappings']['windowsUser_homeDrive'] = 'homeDrive';
		}
		// profile checks
		$return['profile_checks']['windowsUser_accountExpires'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['accountexpires'][0]];
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetitle', true)) {
			$return['profile_checks']['windowsUser_title'] = [
				'type' => 'ext_preg',
				'regex' => 'title',
				'error_message' => $this->messages['title'][0]];
			$return['profile_mappings']['windowsUser_title'] = 'title';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideemployeeType', true)) {
			$return['profile_checks']['windowsUser_employeeType'] = [
				'type' => 'ext_preg',
				'regex' => 'employeeType',
				'error_message' => $this->messages['employeeType'][0]];
			$return['profile_mappings']['windowsUser_employeeType'] = 'employeeType';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidebusinessCategory', true)) {
			$return['profile_checks']['windowsUser_businessCategory'] = [
				'type' => 'ext_preg',
				'regex' => 'businessCategory',
				'error_message' => $this->messages['businessCategory'][0]];
			$return['profile_mappings']['windowsUser_businessCategory'] = 'businessCategory';
		}
		// available PDF fields
		$return['PDF_fields'] = [
			'userPrincipalName' => _('User name'),
			'cn' => _('Common name'),
			'givenName' => _('First name'),
			'sn' => _('Last name'),
			'deactivated' => _('Account is deactivated'),
			'noExpire' => _('Password does not expire'),
			'pwdMustChange' => _('Password change at next login'),
			'groups' => _('Groups'),
			'password' => _('Password'),
			'accountExpires' => _('Account expiration date'),
		];
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$return['PDF_fields']['displayName'] = _('Display name');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideRequireSmartcard')) {
			$return['PDF_fields']['requireCard'] = _('Require smartcard');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath')) {
			$return['PDF_fields']['profilePath'] = _('Profile path');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath')) {
			$return['PDF_fields']['scriptPath'] = _('Logon script');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive')) {
			$return['PDF_fields']['homeDrive'] = _('Home drive');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory')) {
			$return['PDF_fields']['homeDirectory'] = _('Home directory');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedescription')) {
			$return['PDF_fields']['description'] = _('Description');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideinitials')) {
			$return['PDF_fields']['initials'] = _('Initials');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
			$return['PDF_fields']['l'] = _('Location');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail')) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox')) {
			$return['PDF_fields']['otherMailbox'] = _('Email alias');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherTelephone')) {
			$return['PDF_fields']['otherTelephone'] = _('Other telephone numbers');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
			$return['PDF_fields']['physicalDeliveryOfficeName'] = _('Office name');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
			$return['PDF_fields']['postalCode'] = _('Postal code');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
			$return['PDF_fields']['postOfficeBox'] = _('Post office box');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
			$return['PDF_fields']['st'] = _('State');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress')) {
			$return['PDF_fields']['streetAddress'] = _('Street');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetelephoneNumber')) {
			$return['PDF_fields']['telephoneNumber'] = _('Telephone number');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideurl')) {
			$return['PDF_fields']['url'] = _('Other web sites');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidewWWHomePage')) {
			$return['PDF_fields']['wWWHomePage'] = _('Web site');
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideWorkstations')) {
			$return['PDF_fields']['userWorkstations'] = _('Workstations');
		}
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
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
			$return['PDF_fields']['departmentNumber'] = _('Department number');
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
		$return['selfServiceSearchAttributes'] = ['sAMAccountName', 'userPrincipalName',
			'employeeNumber'
		];
		// self service field settings
		$return['selfServiceFieldSettings'] = [
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
			'departmentNumber' => _('Department number'),
			'proxyAddresses' => _('Proxy-Addresses (read-only)'),
			'otherMailbox' => _('Email alias (read-only)'),
		];
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = ['physicalDeliveryOfficeName', 'telephoneNumber',
			'facsimileTelephoneNumber', 'wWWHomePage', 'streetAddress', 'st', 'l', 'postOfficeBox',
			'departmentNumber', 'postalCode'];
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
		$this->groupList = [];
		$this->groupList_orig = [];
	}

	/**
	 * This function fills the $messages variable with output messages from this module.
	 */
	public function load_Messages() {
		$this->messages['userPrincipalName'][0] = ['ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['userPrincipalName'][1] = ['ERROR', _('Account %s:') . ' windowsUser_userPrincipalName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['userPrincipalName'][2] = ['ERROR', _('User name already exists!')];
		$this->messages['userPrincipalName'][3] = ['WARN', _('Account %s:') . ' windowsUser_userPrincipalName', _('User name already exists!')];
		$this->messages['cn'][0] = ['ERROR', _('Common name'), _('Please enter a valid common name!')];
		$this->messages['cn'][1] = ['ERROR', _('Account %s:') . ' windowsUser_cn', _('Please enter a valid common name!')];
		$this->messages['cn'][2] = ['WARN', _('Common name already exists.')];
		$this->messages['cn'][3] = ['WARN', _('Account %s:') . ' windowsUser_cn', _('Common name already exists.')];
		$this->messages['sAMAccountName'][0] = ['ERROR', _('User name (pre W2K)'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['sAMAccountName'][1] = ['ERROR', _('Account %s:') . ' windowsUser_sAMAccountName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['displayName'][0] = ['ERROR', _('Display name'), _('Please enter a valid display name!')];
		$this->messages['displayName'][1] = ['ERROR', _('Account %s:') . ' windowsUser_displayName', _('Please enter a valid display name!')];
		$this->messages['givenName'][0] = ['ERROR', _('First name'), _('First name contains invalid characters!')];
		$this->messages['givenName'][1] = ['ERROR', _('Account %s:') . ' windowsUser_givenName', _('First name contains invalid characters!')];
		$this->messages['sn'][0] = ['ERROR', _('Last name'), _('Last name contains invalid characters or is empty!')];
		$this->messages['sn'][1] = ['ERROR', _('Account %s:') . ' windowsUser_sn', _('Last name contains invalid characters or is empty!')];
		$this->messages['telephoneNumber'][0] = ['ERROR', _('Telephone number'), _('Please enter a valid telephone number!')];
		$this->messages['telephoneNumber'][1] = ['ERROR', _('Account %s:') . ' windowsUser_telephone', _('Please enter a valid telephone number!')];
		$this->messages['facsimileTelephoneNumber'][0] = ['ERROR', _('Fax number'), _('Please enter a valid fax number!')];
		$this->messages['facsimileTelephoneNumber'][1] = ['ERROR', _('Account %s:') . ' windowsUser_facsimileTelephoneNumber', _('Please enter a valid fax number!')];
		$this->messages['otherTelephone'][0] = ['ERROR', _('Other telephone numbers'), _('Please enter a valid telephone number!')];
		$this->messages['otherTelephone'][1] = ['ERROR', _('Account %s:') . ' windowsUser_otherTelephone', _('Please enter a valid telephone number!')];
		$this->messages['pager'][0] = ['ERROR', _('Pager'), _('Please enter a valid telephone number!')];
		$this->messages['pager'][1] = ['ERROR', _('Account %s:') . ' windowsUser_pager', _('Please enter a valid telephone number!')];
		$this->messages['otherPager'][0] = ['ERROR', _('Other pagers'), _('Please enter a valid telephone number!')];
		$this->messages['otherPager'][1] = ['ERROR', _('Account %s:') . ' windowsUser_otherPager', _('Please enter a valid telephone number!')];
		$this->messages['mobile'][0] = ['ERROR', _('Mobile'), _('Please enter a valid telephone number!')];
		$this->messages['mobile'][1] = ['ERROR', _('Account %s:') . ' windowsUser_mobile', _('Please enter a valid telephone number!')];
		$this->messages['otherMobile'][0] = ['ERROR', _('Other mobiles'), _('Please enter a valid telephone number!')];
		$this->messages['otherMobile'][1] = ['ERROR', _('Account %s:') . ' windowsUser_otherMobile', _('Please enter a valid telephone number!')];
		$this->messages['postalCode'][0] = ['ERROR', _('Postal code'), _('Please enter a valid postal code!')];
		$this->messages['postalCode'][1] = ['ERROR', _('Account %s:') . ' windowsUser_postalCode', _('Please enter a valid postal code!')];
		$this->messages['mail'][0] = ['ERROR', _('Email address'), _('Please enter a valid email address!')];
		$this->messages['mail'][1] = ['ERROR', _('Account %s:') . ' windowsUser_mail', _('Please enter a valid email address!')];
		$this->messages['mail'][2] = ['WARN', _('Email address'), _('Email "%s" already in use.')];
		$this->messages['mail'][3] = ['WARN', _('Account %s:') . ' windowsUser_mail', _('Email "%s" already in use.')];
		$this->messages['otherMailbox'][0] = ['ERROR', _('Email alias'), _('Email alias is invalid!')];
		$this->messages['otherMailbox'][1] = ['ERROR', _('Account %s:') . ' windowsUser_otherMailbox', _('Email alias is invalid!')];
		$this->messages['otherMailbox'][2] = ['WARN', _('Email alias'), _('Email alias "%s" already in use.')];
		$this->messages['otherMailbox'][3] = ['WARN', _('Account %s:') . ' windowsUser_otherMailbox', _('Email alias "%s" already in use.')];
		$this->messages['profilePath'][0] = ['ERROR', _('Profile path'), _('Profile path is invalid!')];
		$this->messages['profilePath'][1] = ['ERROR', _('Account %s:') . ' windowsUser_profilePath', _('Profile path is invalid!')];
		$this->messages['scriptPath'][0] = ['ERROR', _('Logon script'), _('Logon script is invalid!')];
		$this->messages['scriptPath'][1] = ['ERROR', _('Account %s:') . ' windowsUser_scriptPath', _('Logon script is invalid!')];
		$this->messages['unicodePwd'][0] = ['ERROR', _('Password'), _('Please enter the same password in both password fields.')];
		$this->messages['unicodePwd'][1] = ['ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!'];
		$this->messages['homeDrive'][0] = ['ERROR', _('Account %s:') . ' windowsUser_homeDrive', _('Please enter a valid drive letter.')];
		$this->messages['homeDirectory'][0] = ['ERROR', _('Home directory'), _('Homedirectory contains invalid characters.')];
		$this->messages['homeDirectory'][1] = ['ERROR', _('Account %s:') . ' windowsUser_homeDirectory', _('Homedirectory contains invalid characters.')];
		$this->messages['msSFU30Name'][0] = ['ERROR', _('NIS name'), _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['msSFU30Name'][1] = ['ERROR', _('Account %s:') . ' windowsUser_msSFU30Name', _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['accountexpires'][0] = ['ERROR', _('Account expiration'), _('Please enter a number.')];
		$this->messages['accountexpires'][1] = ['ERROR', _('Account %s:') . ' windowsUser_accountExpires', _('Please enter a valid date in format DD-MM-YYYY.')];
		$this->messages['title'][0] = ['ERROR', _('Job title'), _('Please enter a valid job title!')];
		$this->messages['title'][1] = ['ERROR', _('Account %s:') . ' windowsUser_title', _('Please enter a valid job title!')];
		$this->messages['employeeType'][0] = ['ERROR', _('Employee type'), _('Please enter a valid employee type!')];
		$this->messages['employeeType'][1] = ['ERROR', _('Account %s:') . ' windowsUser_employeeType', _('Please enter a valid employee type!')];
		$this->messages['businessCategory'][0] = ['ERROR', _('Business category'), _('Please enter a valid business category!')];
		$this->messages['businessCategory'][1] = ['ERROR', _('Account %s:') . ' windowsUser_businessCategory', _('Please enter a valid business category!')];
		$this->messages['manager'][0] = ['ERROR', _('Account %s:') . ' windowsUser_manager', _('This is not a valid DN!')];
		$this->messages['file'][0] = ['ERROR', _('No file selected.')];
		$this->messages['file'][1] = ['ERROR', _('Please upload a .jpg/.jpeg file.')];
		$this->messages['file'][2] = ['ERROR', _('Unable to process this file.')];
		$this->messages['file'][3] = ['ERROR', _('File is too large. Maximum allowed size is %s kB.')];
		$this->messages['workstations'][0] = ['ERROR', _('Workstations'), _('Please enter a comma separated list of host names!')];
		$this->messages['workstations'][1] = ['ERROR', _('Account %s:') . ' windowsUser_workstations', _('Please enter a comma separated list of host names!')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// get group memberships
		$groupList = searchLDAPByAttribute('member', $this->getAccountContainer()->dn_orig, 'group', ['dn'], ['group']);
		$this->groupList_orig = [];
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
		$keysToReplace = ['cn', 'displayName', 'mail', 'otherMailbox',
			'profilePath', 'scriptPath', 'homeDirectory', 'userPrincipalName',
			'sAMAccountName'];
		$this->getAccountContainer()->replaceWildcardsInArray($keysToReplace, $this->attributes);
		$containerLeft = new htmlResponsiveRow();
		if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['useraccountcontrol'][0])) {
			$this->attributes['useraccountcontrol'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
		}
		$containerLeft->add(new htmlSubTitle(_('General')), 12);
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
		$containerLeft->addLabel($userPrincipalNameLabel);
		$userPrincipalNameGroup = new htmlGroup();
		$userPrincipalNameGroup->addElement(new htmlInputField('userPrincipalName', $userPrincipalName, '15'));
		$userPrincipalNameGroup->addElement(new htmlSelect('userPrincipalNameDomain', $domains, [$userPrincipalNameDomain]));
		$userPrincipalNameGroup->addElement(new htmlHelpLink('userPrincipalName'));
		$containerLeft->addField($userPrincipalNameGroup);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidesAMAccountName', true)) {
			$this->addSimpleInputTextField($containerLeft, 'sAMAccountName', _('User name (pre W2K)'));
		}
		$this->addSimpleInputTextField($containerLeft, 'givenName', _('First name'));
		$this->addSimpleInputTextField($containerLeft, 'sn', _('Last name'));
		$this->addSimpleInputTextField($containerLeft, 'cn', _('Common name'), true);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$this->addSimpleInputTextField($containerLeft, 'displayName', _('Display name'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideinitials')) {
			$this->addSimpleInputTextField($containerLeft, 'initials', _('Initials'));
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedescription')) {
			$this->addSimpleInputTextField($containerLeft, 'description', _('Description'));
		}
		// address area
		$showStreetAddress = !$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress');
		$showPostOfficeBox = !$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox');
		$showPostalCode = !$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode');
		$showL = !$this->isBooleanConfigOptionSet('windowsUser_hidel');
		$showSt = !$this->isBooleanConfigOptionSet('windowsUser_hidest');
		$showPhysicalDeliveryOfficeName = !$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName');
		$showAddress = $showStreetAddress || $showPostOfficeBox || $showPostalCode || $showL || $showSt || $showPhysicalDeliveryOfficeName;
		if ($showAddress) {
			$containerLeft->add(new htmlSubTitle(_('Address')), 12);
		}
		if ($showStreetAddress) {
			$this->addSimpleInputTextField($containerLeft, 'streetAddress', _('Street'), false, 20, true);
		}
		if ($showPostOfficeBox) {
			$this->addSimpleInputTextField($containerLeft, 'postOfficeBox', _('Post office box'));
		}
		if ($showPostalCode) {
			$this->addSimpleInputTextField($containerLeft, 'postalCode', _('Postal code'));
		}
		if ($showL) {
			$this->addSimpleInputTextField($containerLeft, 'l', _('Location'));
		}
		if ($showSt) {
			$this->addSimpleInputTextField($containerLeft, 'st', _('State'));
		}
		if ($showPhysicalDeliveryOfficeName) {
			$this->addSimpleInputTextField($containerLeft, 'physicalDeliveryOfficeName', _('Office name'));
		}
		// contact data area
		$showMail = !$this->isBooleanConfigOptionSet('windowsUser_hidemail');
		$showOtherMailbox = !$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox');
		$showProxyAddresses = !$this->isBooleanConfigOptionSet('windowsUser_hideproxyAddresses', true);
		$showTelephoneNumber = !$this->isBooleanConfigOptionSet('windowsUser_hidetelephoneNumber');
		$showOtherTelephone = !$this->isBooleanConfigOptionSet('windowsUser_hideotherTelephone');
		$showMobile = !$this->isBooleanConfigOptionSet('windowsUser_hidemobile');
		$showOtherMobile = !$this->isBooleanConfigOptionSet('windowsUser_hideotherMobile', true);
		$showPager = !$this->isBooleanConfigOptionSet('windowsUser_hidepager', true);
		$showOtherPager = !$this->isBooleanConfigOptionSet('windowsUser_hideotherPager', true);
		$showFacsimileTelephoneNumber = !$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber');
		$showWWWHomePage = !$this->isBooleanConfigOptionSet('windowsUser_hidewWWHomePage');
		$showUrl = !$this->isBooleanConfigOptionSet('windowsUser_hideurl');
		$showContactData = $showMail || $showOtherMailbox || $showProxyAddresses || $showTelephoneNumber || $showOtherTelephone || $showMobile ||
			$showOtherMobile || $showPager || $showOtherPager || $showFacsimileTelephoneNumber || $showWWWHomePage || $showUrl;
		if ($showContactData) {
			$containerLeft->add(new htmlSubTitle(_('Contact data')), 12);
		}
		if ($showMail) {
			$this->addSimpleInputTextField($containerLeft, 'mail', _('Email address'));
		}
		if ($showOtherMailbox) {
			$this->addMultiValueInputTextField($containerLeft, 'otherMailbox', _('Email alias'));
		}
		if ($showProxyAddresses) {
			$this->addMultiValueInputTextField($containerLeft, 'proxyAddresses', _('Proxy-Addresses'));
		}
		if ($showTelephoneNumber) {
			$this->addSimpleInputTextField($containerLeft, 'telephoneNumber', _('Telephone number'));
		}
		if ($showOtherTelephone) {
			$this->addMultiValueInputTextField($containerLeft, 'otherTelephone', _('Other telephone numbers'));
		}
		if ($showMobile) {
			$this->addSimpleInputTextField($containerLeft, 'mobile', _('Mobile'));
		}
		if ($showOtherMobile) {
			$this->addMultiValueInputTextField($containerLeft, 'otherMobile', _('Other mobiles'));
		}
		if ($showPager) {
			$this->addSimpleInputTextField($containerLeft, 'pager', _('Pager'));
		}
		if ($showOtherPager) {
			$this->addMultiValueInputTextField($containerLeft, 'otherPager', _('Other pagers'));
		}
		if ($showFacsimileTelephoneNumber) {
			$this->addSimpleInputTextField($containerLeft, 'facsimileTelephoneNumber', _('Fax number'));
		}
		if ($showWWWHomePage) {
			$this->addSimpleInputTextField($containerLeft, 'wWWHomePage', _('Web site'));
		}
		if ($showUrl) {
			$this->addMultiValueInputTextField($containerLeft, 'url', _('Other web sites'));
		}
		// work details area
		if ($this->manageWorkDetails()) {
			$containerLeft->add(new htmlSubTitle(_('Work details')), 12);
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
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'departmentNumber', _('Department number'), false, null, false, array_slice($this->departmentNumberCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideou', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'ou', _('Organisational unit'), false, null, false, array_slice($this->ouCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideo', true)) {
				$this->addMultiValueInputTextField($containerLeft, 'o', _('Organisation'), false, null, false, array_slice($this->oCache, 0, 300));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemanager', true)) {
				$containerLeft->addLabel(new htmlOutputText(_('Manager')));
				$managerGroup = new htmlGroup();
				$managerVal = (empty($this->attributes['manager'][0])) ? '-' : getAbstractDN($this->attributes['manager'][0]);
				$managerGroup->addElement(new htmlOutputText($managerVal));
				$managerGroup->addElement(new htmlSpacer('0.5rem', null));
				$managerGroup->addElement(new htmlAccountPageButton(get_class($this), 'manager', 'change', _("Change")));
				$managerGroup->addElement(new htmlSpacer('0.5rem', null));
				$managerGroup->addElement(new htmlHelpLink('manager'), true);
				$containerLeft->addField($managerGroup);
			}
		}
		// account area
		$containerLeft->add(new htmlSubTitle(_('Account')), 12);
		// deactivated
		$deactivated = windowsUser::isDeactivated($this->attributes);
		$containerLeft->add(new htmlResponsiveInputCheckbox('deactivated', $deactivated, _("Account is deactivated"), 'deactivated'), 12);
		// password does not expire
		$noExpire = windowsUser::isNeverExpiring($this->attributes);
		$containerLeft->add(new htmlResponsiveInputCheckbox('noExpire', $noExpire, _("Password does not expire"), 'noExpire'), 12);
		// require smartcard
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideRequireSmartcard')) {
			$requireCard = windowsUser::isSmartCardRequired($this->attributes);
			$containerLeft->add(new htmlResponsiveInputCheckbox('requireCard', $requireCard, _("Require smartcard"), 'requireCard'), 12);
		}
		// account expiration
		$containerLeft->addLabel(new htmlOutputText(_('Account expiration date')));
		$accountExpiresGroup = new htmlGroup();
		$accountExpiresGroup->addElement(new htmlOutputText($this->formatAccountExpires()));
		$accountExpiresGroup->addElement(new htmlSpacer('0.5rem', null));
		$accountExpiresGroup->addElement(new htmlAccountPageButton(get_class($this), 'accountexpires', 'edit', _('Change')));
		$accountExpiresGroup->addElement(new htmlSpacer('0.5rem', null));
		$accountExpiresGroup->addElement(new htmlHelpLink('accountexpires'), true);
		$containerLeft->addField($accountExpiresGroup);
		// last password change
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepwdLastSet')) {
			$containerLeft->addLabel(new htmlOutputText(_('Last password change')));
			$pwdLastSetGroup = new htmlGroup();
			$pwdLastSetGroup->addElement(new htmlOutputText($this->formatPwdLastSet()));
			$pwdLastSetGroup->addElement(new htmlSpacer('0.5rem', null));
			if (isset($this->attributes['pwdLastSet'][0]) && ($this->attributes['pwdLastSet'][0] === '0')) {
				$pwdLastSetGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'releasePasswordChange', _('Clear password expiration')));
			}
			else {
				$pwdLastSetGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'forcePasswordChange', _('Force password change')));
			}
			$pwdLastSetGroup->addElement(new htmlSpacer('0.5rem', null));
			$pwdLastSetGroup->addElement(new htmlHelpLink('pwdLastSet'));
			$containerLeft->addField($pwdLastSetGroup);
		}
		// last login
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidelastLogonTimestamp')) {
			$containerLeft->addLabel(new htmlOutputText(_('Last login')));
			$lastLogonTimestampGroup = new htmlGroup();
			$lastLogonTimestampGroup->addElement(new htmlOutputText($this->formatLastLogonTimestamp()));
			$lastLogonTimestampGroup->addElement(new htmlSpacer('0.5rem', null));
			$lastLogonTimestampGroup->addElement(new htmlHelpLink('lastLogonTimestamp'));
			$containerLeft->addField($lastLogonTimestampGroup);
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideWorkstations')) {
			$containerLeft->addLabel(new htmlOutputText(_('Workstations')));
			$userWorkstationsGroup = new htmlGroup();
			$userWorkstationsGroup->addElement(new htmlAccountPageButton(get_class($this), 'userWorkstations', 'open', _('Edit workstations')));
			$userWorkstationsGroup->addElement(new htmlSpacer('0.5rem', null));
			$userWorkstationsGroup->addElement(new htmlHelpLink('userWorkstations'));
			$containerLeft->addField($userWorkstationsGroup);
		}
		// user profile area
		$showProfilePath = !$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath');
		$showScriptPath = !$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath');
		$showHomeDrive = !$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive');
		$showHomeDirectory = !$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory');
		$showUserProfile = $showProfilePath || $showScriptPath || $showHomeDrive || $showHomeDirectory;
		if ($showUserProfile) {
			$containerLeft->add(new htmlSubTitle(_('User profile')), 12);
		}
		// profile path
		if ($showProfilePath) {
			$this->addSimpleInputTextField($containerLeft, 'profilePath', _('Profile path'));
		}
		// logon script
		if ($showScriptPath) {
			$this->addSimpleInputTextField($containerLeft, 'scriptPath', _('Logon script'));
		}
		// home drive
		if ($showHomeDrive) {
			$drives = ['-'];
			for ($i = 90; $i > 67; $i--) {
				$drives[] = chr($i) . ':';
			}
			if (!empty($this->attributes['homeDrive'][0])) {
				$selected = [strtoupper($this->attributes['homeDrive'][0])];
			}
			else {
				$selected = ['-'];
			}
			$containerLeft->add(new htmlResponsiveSelect('homeDrive', $drives, $selected, _('Home drive'), 'homeDrive'), 12);
		}
		// home directory
		if ($showHomeDirectory) {
			$this->addSimpleInputTextField($containerLeft, 'homeDirectory', _('Home directory'));
		}

		// NIS attributes
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true) || !$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
			$containerLeft->add(new htmlSubTitle(_('NIS')), 12);
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30Name', true)) {
				$this->addSimpleInputTextField($containerLeft, 'msSFU30Name', _('NIS name'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemsSFU30NisDomain', true)) {
				$this->addSimpleInputTextField($containerLeft, 'msSFU30NisDomain', _('NIS domain'));
			}
		}

		$containerRight = new htmlResponsiveRow();
		// photo
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidejpegPhoto', true)) {
			$photoFile = '../../graphics/user.svg';
			$noPhoto = true;
			if (isset($this->attributes['jpegPhoto'][0])) {
				$tempFilesManager = new LamTemporaryFilesManager();
				$jpegFileName = $tempFilesManager->registerTemporaryFile('.jpg');
				$outjpeg = $tempFilesManager->openTemporaryFileForWrite($jpegFileName);
				fwrite($outjpeg, $this->attributes['jpegPhoto'][0]);
				fclose($outjpeg);
				$photoFile = $tempFilesManager->getResourceLink($jpegFileName);
				$noPhoto = false;
			}
			$img = new htmlImage($photoFile);
			$img->setCSSClasses(['photo']);
			$img->enableLightbox();
			$containerRight->add($img, 12);
			$containerRight->addVerticalSpacer('0.5rem');
			if ($noPhoto) {
				$containerRight->add(new htmlAccountPageButton(get_class($this), 'photo', 'open', _('Add photo')), 12);
			}
			else {
				$containerRight->add(new htmlButton('delPhoto', _('Delete photo')), 12);
			}
			$containerRight->addVerticalSpacer('2rem');
		}
		// groups
		$containerRight->add(new htmlSubTitle(_('Groups')), 12);
		$containerRight->add(new htmlAccountPageButton(get_class($this), 'group', 'edit', _('Edit groups')), 12);
		$containerRight->addVerticalSpacer('1rem');
		$groupsList = new htmlGroup();
		$groupNames = [];
		if ($this->groupDisplayContainsDn()) {
			usort($this->groupList, 'compareDN');
		}
		foreach ($this->groupList as $groupDn) {
			$groupCn = extractRDNValue($groupDn);
			$groupNames[] = $this->formatGroupName($groupCn, $groupDn);
		}
		if (!$this->groupDisplayContainsDn()) {
			natcasesort($groupNames);
		}
		foreach ($groupNames as $cn) {
			$groupsList->addElement(new htmlOutputText($cn));
			$groupsList->addElement(new htmlOutputText('<br>', false));
		}
		$groupsListClass = $this->groupDisplayContainsDn() ? 'rightToLeftText' : '';
		$groupsListDiv = new htmlDiv(null, $groupsList, [$groupsListClass]);
		$containerRight->add($groupsListDiv, 12);

		$container = new htmlResponsiveRow();
		$container->add($containerLeft, 12, 12, 7);
		$container->add(new htmlSpacer('1rem', null), 0, 0, 1);
		$container->add($containerRight, 12, 12, 4);
		return $container;
	}

	/**
	 * Formats a group name for the display.
	 *
	 * @param string $cn common name
	 * @param string $dn DN
	 * @return string formatted name
	 */
	private function formatGroupName($cn, $dn) {
		$mode = empty($this->moduleSettings['windowsUser_displayGroups'][0]) ? 'dn' : $this->moduleSettings['windowsUser_displayGroups'][0];
		switch ($mode) {
			case self::DISPLAY_GROUPS_CN:
				return $cn;
			case self::DISPLAY_GROUPS_DN:
			default:
				return getAbstractDN($dn);
		}
	}

	/**
	 * Returns if the group display name contains the DN.
	 *
	 * @return bool contains DN.
	 */
	private function groupDisplayContainsDn() {
		$mode = empty($this->moduleSettings['windowsUser_displayGroups'][0]) ? 'dn' : $this->moduleSettings['windowsUser_displayGroups'][0];
		return ($mode == self::DISPLAY_GROUPS_DN);
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
			&& $this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)
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
		$return = [];
		$keysToReplace = ['cn', 'displayName', 'mail', 'otherMailbox',
			'profilePath', 'scriptPath', 'homeDirectory', 'userPrincipalName',
			'sAMAccountName'];
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
		if ($this->getAccountContainer()->isNewAccount) {
			$existingUsers = $this->getUserNames();
			if (array_key_exists($userPrincipalName, $existingUsers)) {
				$return[] = $this->messages['userPrincipalName'][2];
			}
		}
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
		if ($this->getAccountContainer()->isNewAccount && !empty($this->attributes['cn'][0])) {
			$existingCns = $this->getCns();
			if (array_key_exists($this->attributes['cn'][0], $existingCns)) {
				$return[] = $this->messages['cn'][2];
			}
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
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedescription')) {
			$this->attributes['description'][0] = $_POST['description'];
		}
		// display name
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
			$this->attributes['displayName'][0] = $_POST['displayName'];
			if (!empty($this->attributes['displayName'][0]) && !get_preg($this->attributes['displayName'][0], 'realname')) {
				$return[] = $this->messages['displayName'][0];
			}
			if (empty($this->attributes['displayName'][0]) && !empty($this->attributes['cn'][0])) {
				$this->attributes['displayName'][0] = $this->attributes['cn'][0];
			}
		}
		// first name
		$this->attributes['givenName'][0] = $_POST['givenName'];
		if (!empty($this->attributes['givenName'][0]) && !get_preg($_POST['givenName'], 'realname')) {
			$return[] = $this->messages['givenName'][0];
		}
		// initials
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideinitials')) {
			$this->attributes['initials'][0] = $_POST['initials'];
		}
		// location
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
			$this->attributes['l'][0] = $_POST['l'];
		}
		// email
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail')) {
			$this->attributes['mail'][0] = $_POST['mail'];
			if (!empty($this->attributes['mail'][0]) && !get_preg($this->attributes['mail'][0], 'email')) {
				$return[] = $this->messages['mail'][0];
			}
			elseif (!empty($this->attributes['mail'][0]) && $this->getAccountContainer()->isNewAccount) {
				$existingMailAccounts = searchLDAPByAttribute('mail', $this->attributes['mail'][0], 'user', ['mail'], ['user']);
				if (!empty($existingMailAccounts)) {
					$msg = $this->messages['mail'][2];
					$msg[] = [$this->attributes['mail'][0]];
					$return[] = $msg;
				}
			}
		}
		// email aliases
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox')) {
			$this->processMultiValueInputTextField('otherMailbox', $return, 'email');
			if (!empty($this->attributes['otherMailbox'][0]) && $this->getAccountContainer()->isNewAccount) {
				foreach ($this->attributes['otherMailbox'] as $otherMailbox) {
					$existingMailAccounts = searchLDAPByAttribute('otherMailbox', $otherMailbox, 'user', ['otherMailbox'], ['user']);
					if (!empty($existingMailAccounts)) {
						$msg = $this->messages['otherMailbox'][2];
						$msg[] = [$otherMailbox];
						$return[] = $msg;
					}
				}
			}
		}
		// other telephones
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherTelephone')) {
			$this->processMultiValueInputTextField('otherTelephone', $return, 'telephone');
		}
		// fax number
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
			$this->attributes['facsimileTelephoneNumber'][0] = $_POST['facsimileTelephoneNumber'];
			if (!get_preg($_POST['facsimileTelephoneNumber'], 'telephone')) {
				$return[] = $this->messages['facsimileTelephoneNumber'][0];
			}
		}
		// office name
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
			$this->attributes['physicalDeliveryOfficeName'][0] = $_POST['physicalDeliveryOfficeName'];
		}
		// postal code
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
			$this->attributes['postalCode'][0] = $_POST['postalCode'];
			if (!get_preg($_POST['postalCode'], 'postalCode')) {
				$return[] = $this->messages['postalCode'][0];
			}
		}
		// post office box
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
			$this->attributes['postOfficeBox'][0] = $_POST['postOfficeBox'];
		}
		// last name
		$this->attributes['sn'][0] = $_POST['sn'];
		if (!empty($this->attributes['sn'][0]) && !get_preg($_POST['sn'], 'realname')) {
			$return[] = $this->messages['sn'][0];
		}
		// state
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
			$this->attributes['st'][0] = $_POST['st'];
		}
		// street
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress')) {
			$this->attributes['streetAddress'][0] = $_POST['streetAddress'];
		}
		// telephone
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidetelephoneNumber')) {
			$this->attributes['telephoneNumber'][0] = $_POST['telephoneNumber'];
			if (!get_preg($_POST['telephoneNumber'], 'telephone')) {
				$return[] = $this->messages['telephoneNumber'][0];
			}
		}
		// other web sites
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideurl')) {
			$this->processMultiValueInputTextField('url', $return);
		}
		// web site
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidewWWHomePage')) {
			$this->attributes['wWWHomePage'][0] = $_POST['wWWHomePage'];
		}
		// password must be changed
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_forcePasswordChange'])) {
			$this->attributes['pwdLastSet'][0] = '0';
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_attributes_releasePasswordChange'])) {
			if (isset($this->orig['pwdLastSet'][0]) && ($this->orig['pwdLastSet'][0] !== '0')) {
				$this->attributes['pwdLastSet'][0] = $this->orig['pwdLastSet'][0];
			}
			else {
				$this->attributes['pwdLastSet'][0] = '-1';
			}
		}
		// save pwdLastSet for postModifyActions
		$this->pwdLastSet = null;
		if (isset($this->attributes['pwdLastSet'][0])
			&& (!isset($this->orig['pwdLastSet'][0]) || ($this->orig['pwdLastSet'][0] != $this->attributes['pwdLastSet'][0]))) {
			$this->pwdLastSet = $this->attributes['pwdLastSet'][0];
		}
		// deactivated
		$deactivated = isset($_POST['deactivated']) && ($_POST['deactivated'] == 'on');
		windowsUser::setIsDeactivated($deactivated);
		// no expire
		$noExpire = isset($_POST['noExpire']) && ($_POST['noExpire'] == 'on');
		windowsUser::setIsNeverExpiring($this->attributes, $noExpire);
		// smartcard required
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideRequireSmartcard')) {
			$requireCard = isset($_POST['requireCard']) && ($_POST['requireCard'] == 'on');
			windowsUser::setIsSmartCardRequired($this->attributes, $requireCard);
		}
		// profile path
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath')) {
			$this->attributes['profilePath'][0] = $_POST['profilePath'];
			if (!($this->attributes['profilePath'][0] == '') && !get_preg($this->attributes['profilePath'][0], 'UNC')) {
				$return[] = $this->messages['profilePath'][0];
			}
		}
		// logon script
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath')) {
			$this->attributes['scriptPath'][0] = $_POST['scriptPath'];
			if (($this->attributes['scriptPath'][0] != '') && (!get_preg($this->attributes['scriptPath'][0], 'logonscript'))) {
				$return[] = $this->messages['scriptPath'][0];
			}
		}
		// home drive
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive')) {
			if ($_POST['homeDrive'] == "-") {
				$this->attributes['homeDrive'][0] = '';
			}
			else {
				$this->attributes['homeDrive'][0] = $_POST['homeDrive'];
			}
		}
		// home directory
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory')) {
			$this->attributes['homeDirectory'][0] = $_POST['homeDirectory'];
			if (!empty($this->attributes['homeDrive'][0]) && !get_preg($this->attributes['homeDirectory'][0], 'UNC')) {
				$return[] = $this->messages['homeDirectory'][0];
			}
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
		// department number
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
			$this->processMultiValueInputTextField('departmentNumber', $return);
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
		// photo
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidejpegPhoto', true)
			&& isset($_POST['delPhoto'])) {
			unset($this->attributes['jpegPhoto']);
		}
		return $return;
	}

	/**
	 * This function will create the meta HTML code to show a page to change account expiration.
	 *
	 * @return htmlElement meta HTML code
	 */
	function display_html_accountexpires() {
		$return = new htmlResponsiveRow();
		$attr = 'accountexpires';
		$datetime = new DateTime('now', getTimeZone());
		if (!empty($this->attributes[$attr][0]) && !($this->attributes[$attr][0] == '0')) {
			$datetime = windowsUser::getFileTime($this->attributes[$attr][0]);
		}
		$dateInput = new htmlResponsiveInputField(_('Account expiration date'), $attr, $datetime->format('Y-m-d'), 'accountexpires');
		$dateInput->showCalendar('Y-m-d');
		$return->add($dateInput);

		if ($this->getAccountContainer()->getAccountModule('shadowAccount') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncShadow', false, _('Set also for Shadow')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('heimdalKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncHeimdal', false, _('Set also for Kerberos')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMIT', false, _('Set also for Kerberos')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberosStructural') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMITStructural', false, _('Set also for Kerberos')), 12);
		}
		$return->addVerticalSpacer('2rem');
		$buttons = new htmlGroup();
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'change' . $attr, _('Change')));
		if (isset($this->attributes[$attr][0])) {
			$buttons->addElement(new htmlSpacer('0.5rem', null));
			$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'del' . $attr, _('Remove')));
		}
		$buttons->addElement(new htmlSpacer('0.5rem', null));
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back' . $attr, _('Cancel')));
		$return->add($buttons, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * Processes user input of the account expiration page.
	 *
	 * @return array list of info/error messages
	 */
	function process_accountexpires() {
		$return = [];
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (strpos($postKeys[$i], 'form_subpage_windowsUser_attributes_') !== false) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (strpos($buttonName, '_back') !== false)) {
			return [];
		}
		$attr = 'accountexpires';
		// determine action
		if (strpos($buttonName, '_change') !== false) {
			$date = DateTime::createFromFormat('Y-m-d', $_POST['accountexpires'], new DateTimeZone('UTC'));
			$year = $date->format('Y');
			$month = $date->format('m');
			$day = $date->format('d');
			// set new time
			$this->setExpirationDate($year, $month, $day);
			// sync other modules
			if (isset($_POST['syncShadow']) && ($_POST['syncShadow'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('shadowAccount')->setExpirationDate(
					$year, $month, $day);
			}
			if (isset($_POST['syncHeimdal']) && ($_POST['syncHeimdal'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('heimdalKerberos')->setExpirationDate(
					$year, $month, $day);
			}
			if (isset($_POST['syncMIT']) && ($_POST['syncMIT'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberos')->setExpirationDate(
					$year, $month, $day);
			}
			if (isset($_POST['syncMITStructural']) && ($_POST['syncMITStructural'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberosStructural')->setExpirationDate(
					$year, $month, $day);
			}
		}
		elseif (strpos($buttonName, '_del') !== false) {
			// remove attribute value
			if (!isset($this->orig[$attr][0])) {
				unset($this->attributes[$attr]);
			}
			else {
				$this->attributes[$attr][0] = '0';
			}
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
	public function display_html_group() {
		$return = new htmlResponsiveRow();
		$return->setCSSClasses(['maxrow']);
		$return->add(new htmlSubTitle(_("Groups")), 12);
		$groups = $this->findGroups();
		$groupDisplayContainsDn = $this->groupDisplayContainsDn();
		// sort by DN
		if ($groupDisplayContainsDn) {
			usort($groups, 'compareDN');
		}

		$selectedGroups = [];
		// sort by DN
		if ($groupDisplayContainsDn) {
			usort($this->groupList, 'compareDN');
		}
		for ($i = 0; $i < sizeof($this->groupList); $i++) {
			if (in_array($this->groupList[$i], $groups)) {
				$groupDn = $this->groupList[$i];
				$groupCn = extractRDNValue($groupDn);
				$displayName = $this->formatGroupName($groupCn, $groupDn);
				$selectedGroups[$displayName] = $groupDn;
			}
		}
		$availableGroups = [];
		foreach ($groups as $dn) {
			if (!in_array($dn, $this->groupList)) {
				$groupCn = extractRDNValue($dn);
				$displayName = $this->formatGroupName($groupCn, $dn);
				$availableGroups[$displayName] = $dn;
			}
		}

		if (!$groupDisplayContainsDn) {
			$selectedGroups = array_flip($selectedGroups);
			natcasesort($selectedGroups);
			$selectedGroups = array_flip($selectedGroups);
			$availableGroups = array_flip($availableGroups);
			natcasesort($availableGroups);
			$availableGroups = array_flip($availableGroups);
		}

		$this->addDoubleSelectionArea($return, _("Selected groups"), _("Available groups"),
			$selectedGroups, null, $availableGroups, null, 'groups', $groupDisplayContainsDn, true);

		// sync options
		$typeManager = new TypeManager();
		$syncTypes = $typeManager->getConfiguredTypesForScopes(['group', 'gon', 'user']);
		$syncActive = false;
		$syncGonActive = false;
		$possibleGonSyncModules = ['groupOfNames', 'groupOfMembers', 'groupOfUniqueNames'];
		foreach ($syncTypes as $syncType) {
			$modules = $syncType->getModules();
			foreach ($possibleGonSyncModules as $possibleModule) {
				if (in_array($possibleModule, $modules)) {
					$syncGonActive = true;
					break;
				}
			}
		}
		if (in_array('posixAccount', $this->getAccountContainer()->get_type()->getModules())) {
			$syncActive = true;
		}
		$syncActive = $syncActive && !$this->isBooleanConfigOptionSet('windowsUser_syncGroups');
		if ($syncActive) {
			$return->add(new htmlSubTitle(_('Sync groups')), 12);
			$return->add(new htmlResponsiveInputCheckbox('syncDeleteGroups', true, _('Delete non-matching entries')), 12);
			$return->addVerticalSpacer('2rem');
			$syncButtons = new htmlGroup();
			$u2wButton = new htmlAccountPageButton(get_class($this), 'group', 'syncU2W', _('Sync Unix to Windows'));
			$syncButtons->addElement($u2wButton);
			$syncButtons->addElement(new htmlSpacer('2rem', null));
			if ($syncGonActive) {
				$g2wButton = new htmlAccountPageButton(get_class($this), 'group', 'syncG2W', _('Sync group of names to Windows'));
				$syncButtons->addElement($g2wButton);
				$syncButtons->addElement(new htmlSpacer('2rem', null));
			}
			$return->add($syncButtons, 12, 12, 12, 'text-center');
			$return->addVerticalSpacer('2rem');
		}

		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$return->add($backButton, 12);
		return $return;
	}

	/**
	 * Processes user input of the group selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	public function process_group() {
		if (isset($_POST['groups_2']) && isset($_POST['groups_left'])) { // Add groups to list
			// add new group
			$this->groupList = @array_merge($this->groupList, $_POST['groups_2']);
		}
		elseif (isset($_POST['groups_1']) && isset($_POST['groups_right'])) { // remove groups from list
			$this->groupList = array_delete($_POST['groups_1'], $this->groupList);
		}
		// sync Unix to Windows
		if (isset($_POST['form_subpage_windowsUser_group_syncU2W'])) {
			$this->manualSyncUnixToWindows();
		}
		// sync group of names to Windows
		if (isset($_POST['form_subpage_windowsUser_group_syncG2W'])) {
			$this->manualSyncGonToWindows();
		}
		return [];
	}

	/**
	 * Syncs the Unix groups to Windows.
	 */
	private function manualSyncUnixToWindows() {
		$windowsGroups = $this->getGroupList();
		$unixGroups = $this->getAccountContainer()->getAccountModule('posixAccount')->getGroups();
		$allWindowsGroups = searchLDAPByAttribute('cn', '*', 'group', ['cn'], ['group']);
		$dnToCn = [];
		foreach ($allWindowsGroups as $windowsGroup) {
			$dnToCn[$windowsGroup['dn']] = $windowsGroup['cn'][0];
		}
		$cnToDn = array_flip($dnToCn);
		$currentGroupNames = [];
		foreach ($windowsGroups as $windowsGroup) {
			$currentGroupNames[] = $dnToCn[$windowsGroup];
		}
		$deleteNonMatching = isset($_POST['syncDeleteGroups']) && ($_POST['syncDeleteGroups'] == 'on');
		$namesToIgnore = [];
		if (!empty($this->moduleSettings['windowsUser_syncGroupsExclusions'])) {
			$namesToIgnore = $this->moduleSettings['windowsUser_syncGroupsExclusions'];
			array_map('trim', $namesToIgnore);
		}
		foreach ($unixGroups as $unixGroup) {
			if (in_array($unixGroup, $namesToIgnore)) {
				continue;
			}
			if (!in_array($unixGroup, $currentGroupNames) && isset($cnToDn[$unixGroup])) {
				$windowsGroups[] = $cnToDn[$unixGroup];
			}
		}
		if ($deleteNonMatching) {
			foreach ($currentGroupNames as $currentGroupName) {
				if (in_array($currentGroupName, $namesToIgnore)) {
					continue;
				}
				if (!in_array($currentGroupName, $unixGroups)) {
					foreach ($windowsGroups as $windowsGroup) {
						if ($dnToCn[$windowsGroup] == $currentGroupName) {
							$windowsGroups = array_delete([$windowsGroup], $windowsGroups);
							break;
						}
					}
				}
			}
		}
		$this->groupList = $windowsGroups;
	}

	/**
	 * Syncs the group of names to Windows.
	 */
	private function manualSyncGonToWindows() {
		$windowsGroups = $this->getGroupList();
		$gonGroupDns = $this->getAccountContainer()->getAccountModule('posixAccount')->getGroupOfNames();
		$allGons = $this->getAccountContainer()->getAccountModule('posixAccount')->findGroupOfNames();
		$gonGroups = [];
		foreach ($gonGroupDns as $gonGroupDn) {
			$gonGroups[] = $this->getAccountContainer()->getAccountModule('posixAccount')->getGonName($gonGroupDn, $allGons);
		}
		$allWindowsGroups = searchLDAPByAttribute('cn', '*', 'group', ['cn'], ['group']);
		$dnToCn = [];
		foreach ($allWindowsGroups as $windowsGroup) {
			$dnToCn[$windowsGroup['dn']] = $windowsGroup['cn'][0];
		}
		$cnToDn = array_flip($dnToCn);
		$currentGroupNames = [];
		foreach ($windowsGroups as $windowsGroup) {
			$currentGroupNames[] = $dnToCn[$windowsGroup];
		}
		$deleteNonMatching = isset($_POST['syncDeleteGroups']) && ($_POST['syncDeleteGroups'] == 'on');
		$namesToIgnore = [];
		if (!empty($this->moduleSettings['windowsUser_syncGroupsExclusions'])) {
			$namesToIgnore = $this->moduleSettings['windowsUser_syncGroupsExclusions'];
			array_map('trim', $namesToIgnore);
		}
		foreach ($gonGroups as $gonGroup) {
			if (in_array($gonGroup, $namesToIgnore)) {
				continue;
			}
			if (!in_array($gonGroup, $currentGroupNames) && isset($cnToDn[$gonGroup])) {
				$windowsGroups[] = $cnToDn[$gonGroup];
			}
		}
		if ($deleteNonMatching) {
			foreach ($currentGroupNames as $currentGroupName) {
				if (in_array($currentGroupName, $namesToIgnore)) {
					continue;
				}
				if (!in_array($currentGroupName, $gonGroups)) {
					foreach ($windowsGroups as $windowsGroup) {
						if ($dnToCn[$windowsGroup] == $currentGroupName) {
							$windowsGroups = array_delete([$windowsGroup], $windowsGroups);
							break;
						}
					}
				}
			}
		}
		$this->groupList = $windowsGroups;
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
			$errorMessage->setCSSClasses(['hidden', 'lam-webcam-message']);
			$webcamContent->add($errorMessage, 12);
			$captureButton = new htmlButton('lam-webcam-capture', _('Start capture'));
			$captureButton->setOnClick('window.lam.tools.webcam.capture(event);');
			$webcamContent->add($captureButton, 12, 12, 12, 'text-center');
			$video = new htmlVideo('lam-webcam-video');
			$video->setCSSClasses(['hidden']);
			$webcamContent->add($video, 12, 12, 12, 'text-center');
			$webcamContent->addVerticalSpacer('0.5rem');
			$webcamUploadButton = new htmlButton('uploadWebcam', _('Upload'));
			$webcamUploadButton->setCSSClasses(['btn-lam-webcam-upload', 'hidden']);
			$webcamUploadButton->setOnClick('window.lam.tools.webcam.upload();');
			$webcamContent->add($webcamUploadButton, 12, 12, 12, 'text-center');
			$canvas = new htmlCanvas('lam-webcam-canvas');
			$canvas->setCSSClasses(['hidden']);
			$webcamContent->add($canvas, 12);
			$webcamDiv = new htmlDiv('lam_webcam_div', $webcamContent, ['hidden']);
			$container->add($webcamDiv, 12);
			$container->addVerticalSpacer('1rem');
			$container->add(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')), 12);
		}
		else {
			$container->add(new htmlSubTitle(_('Crop image')));
			$tempFilesManager = new LamTemporaryFilesManager();
			$jpegFileName = $tempFilesManager->registerTemporaryFile('.jpg');
			$handle = $tempFilesManager->openTemporaryFileForWrite($jpegFileName);
			fwrite($handle, $this->attributes['jpegPhoto'][0]);
			fclose($handle);
			$photoFile = $tempFilesManager->getResourceLink($jpegFileName);
			$img = new htmlImage($photoFile);
			$img->setCSSClasses(['photo']);
			$img->enableCropping();
			$container->add($img);
			$container->addVerticalSpacer('2rem');
			$container->add(new htmlAccountPageButton(get_class($this), 'attributes', 'crop', _('Done')), 12);
		}
		return $container;
	}

	/**
	 * Sets a new photo.
	 */
	public function process_photo() {
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_back'])) {
			return [];
		}
		if (isset($_POST['form_subpage_' . get_class($this) . '_photo_upload']) || isset($_POST['webcamData'])) {
			return $this->uploadPhoto();
		}
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_crop'])) {
			$messages = [];
			try {
				include_once dirname(__FILE__) . '/../imageutils.inc';
				$imageManipulator = ImageManipulationFactory::getImageManipulator($this->attributes['jpegPhoto'][0]);
				$imageManipulator->crop($_POST['croppingDataX'], $_POST['croppingDataY'], $_POST['croppingDataWidth'], $_POST['croppingDataHeight']);
				$this->attributes['jpegPhoto'][0] = $imageManipulator->getImageData();
			}
			catch (Exception $e) {
				$msg = $this->messages['file'][2];
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
			if (!empty($this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]) && (strlen($data) > (1024 * $this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]))) {
				$errMsg = $this->messages['file'][3];
				$errMsg[] = null;
				$errMsg[] = [$this->moduleSettings['windowsUser_jpegPhoto_maxSize'][0]];
				return [$errMsg];
			}
		}
		elseif (isset($_POST['webcamData'])) {
			$data = $_POST['webcamData'];
			$data = str_replace('data:image/png;base64,', '', $data);
			$data = base64_decode($data);
		}
		// convert to JPG
		try {
			include_once dirname(__FILE__) . '/../imageutils.inc';
			$imageManipulator = ImageManipulationFactory::getImageManipulator($data);
			// resize if maximum values specified
			if (!empty($this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0]) || !empty($this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0])) {
				$maxWidth = empty($this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0]) ? $imageManipulator->getWidth() : $this->moduleSettings['windowsUser_jpegPhoto_maxWidth'][0];
				$maxHeight = empty($this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0]) ? $imageManipulator->getHeight() : $this->moduleSettings['windowsUser_jpegPhoto_maxHeight'][0];
				$imageManipulator->thumbnail($maxWidth, $maxHeight);
			}
			$imageManipulator->convertToJpeg();
			$data = $imageManipulator->getImageData();
		}
		catch (Exception $e) {
			$msg = $this->messages['file'][2];
			$msg[] = htmlspecialchars($e->getMessage());
			$messages[] = $msg;
			return $messages;
		}
		$this->attributes['jpegPhoto'][0] = $data;
		return $messages;
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
		$options = [];
		$filter = get_ldap_filter('user');
		$entries = searchLDAPByFilter('(|' . $filter . '(objectclass=organizationalRole))', ['dn'], ['user']);
		for ($i = 0; $i < sizeof($entries); $i++) {
			$entries[$i] = $entries[$i]['dn'];
		}
		// sort by DN
		usort($entries, 'compareDN');
		for ($i = 0; $i < sizeof($entries); $i++) {
			$options[getAbstractDN($entries[$i])] = $entries[$i];
		}
		$selectedManager = [];
		if (!empty($this->attributes['manager'][0])) {
			$selectedManager[] = $this->attributes['manager'][0];
		}
		$managerSelect = new htmlSelect('manager', $options, $selectedManager);
		$managerSelect->setHasDescriptiveElements(true);
		$managerSelect->setRightToLeftTextDirection(true);
		$managerSelect->setSortElements(false);
		$managerSelect->setTransformSingleSelect(false);
		$return->add($managerSelect, 12);
		$return->addVerticalSpacer('2rem');
		$buttonTable = new htmlGroup();
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'setManager', _('Change')));
		$buttonTable->addElement(new htmlSpacer('0.5rem', null));
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'removeManager', _('Remove')));
		$buttonTable->addElement(new htmlSpacer('2rem', null));
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancelManager', _('Cancel')));
		$return->add($buttonTable, 12);
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
	 * This function will create the HTML page to edit the allowed workstations.
	 *
	 * @return htmlElement meta HTML code
	 */
	function display_html_userWorkstations() {
		$return = new htmlResponsiveRow();
		if ($this->get_scope() == 'user') {
			// Get list of all hosts.
			$userWorkstations = [];
			$availableUserWorkstations = [];

			$result = $this->getHostList();
			foreach ($result as $host) {
				$availableUserWorkstations[] = str_replace("$", '', $host);
			}
			sort($availableUserWorkstations, SORT_STRING);
			if (isset($this->attributes['userWorkstations'][0])) {
				$wsAttr = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$userWorkstations = explode(',', $wsAttr);
			}
			$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);

			$return->add(new htmlSubTitle(_("Allowed workstations")), 12);

			$userWorkstationsOptions = [];
			foreach ($userWorkstations as $userWorkstation) {
				$userWorkstationsOptions[$userWorkstation] = $userWorkstation;
			}
			$availableUserWorkstationsOptions = [];
			foreach ($availableUserWorkstations as $availableUserWorkstation) {
				$availableUserWorkstationsOptions[$availableUserWorkstation] = $availableUserWorkstation;
			}
			$this->addDoubleSelectionArea($return, _("Allowed workstations"), _("Available workstations"), $userWorkstationsOptions, [], $availableUserWorkstationsOptions, [], 'workstations', false, true);

			$return->addVerticalSpacer('2rem');

			$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
			$return->add($backButton, 12);
		}
		return $return;
	}

	/**
	 * Processes user input of the workstation page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_userWorkstations() {
		// Load attributes
		if (isset($_POST['workstations_2']) && isset($_POST['workstations_left'])) { // Add workstations to list
			$workstations = [];
			if (isset($this->attributes['userWorkstations'][0])) {
				$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$workstations = explode(',', $temp);
				for ($i = 0; $i < count($workstations); $i++) {
					if ($workstations[$i] == '') {
						unset($workstations[$i]);
					}
				}
				$workstations = array_values($workstations);
			}
			// Add new // Add workstations
			$workstations = array_merge($workstations, $_POST['workstations_2']);
			// remove doubles
			$workstations = array_flip($workstations);
			$workstations = array_unique($workstations);
			$workstations = array_flip($workstations);
			// sort workstations
			sort($workstations);
			// Recreate workstation string
			$this->attributes['userWorkstations'][0] = $workstations[0];
			for ($i = 1; $i < count($workstations); $i++) {
				$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
			}
		}
		elseif (isset($_POST['workstations_1']) && isset($_POST['workstations_right'])) { // remove // Add workstations from list
			// Put all workstations in array
			$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
			$workstations = explode(',', $temp);
			for ($i = 0; $i < count($workstations); $i++) {
				if ($workstations[$i] == '') {
					unset($workstations[$i]);
				}
			}
			$workstations = array_values($workstations);
			// Remove unwanted workstations from array
			$workstations = array_delete($_POST['workstations_1'], $workstations);
			// Recreate workstation string
			unset($this->attributes['userWorkstations'][0]);
			if (sizeof($workstations) > 0) {
				$this->attributes['userWorkstations'][0] = $workstations[0];
				for ($i = 1; $i < count($workstations); $i++) {
					$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
				}
			}
		}
		return [];
	}

	/**
	 * Returns a list of existing hosts.
	 *
	 * @return array host names
	 */
	private function getHostList() {
		if ($this->cachedHostList != null) {
			return $this->cachedHostList;
		}
		$this->cachedHostList = searchLDAPByAttribute('cn', '*', 'computer', ['cn'], ['host']);
		for ($i = 0; $i < sizeof($this->cachedHostList); $i++) {
			$this->cachedHostList[$i] = $this->cachedHostList[$i]['cn'][0];
		}
		return $this->cachedHostList;
	}

	/**
	 * Runs the postmodify actions.
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 * @see baseModule::postModifyActions()
	 *
	 */
	public function postModifyActions($newAccount, $attributes) {
		$messages = [];
		// set groups
		$groups = $this->findGroups();
		$toAdd = array_values(array_diff($this->groupList, $this->groupList_orig));
		$toRem = array_values(array_diff($this->groupList_orig, $this->groupList));
		// add groups
		for ($i = 0; $i < sizeof($toAdd); $i++) {
			if (in_array($toAdd[$i], $groups)) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $toAdd[$i], ['member' => [$this->getAccountContainer()->finalDN]]);
				if (!$success) {
					logNewMessage(LOG_ERR, 'Unable to add user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = ['ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toAdd[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
				}
				else {
					logNewMessage(LOG_NOTICE, 'Added user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i]);
				}
			}
		}
		// remove groups
		for ($i = 0; $i < sizeof($toRem); $i++) {
			if (in_array($toRem[$i], $groups)) {
				// membership is removed with potentially new DN as Windows updates group automatically on user move
				$success = @ldap_mod_del($_SESSION['ldap']->server(), $toRem[$i], ['member' => [$this->getAccountContainer()->finalDN]]);
				if (!$success) {
					logNewMessage(LOG_ERR, 'Unable to delete user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = ['ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
				}
				else {
					logNewMessage(LOG_NOTICE, 'Removed user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i]);
				}
			}
		}
		// force password change if needed
		if ($this->pwdLastSet != null) {
			$attrs = ['pwdLastSet' => [$this->pwdLastSet]];
			$success = @ldap_modify($_SESSION['ldap']->server(), $this->getAccountContainer()->finalDN, $attrs);
			if (!$success) {
				logNewMessage(LOG_ERR, 'Unable to change pwdLastSet for ' . $this->getAccountContainer()->finalDN . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$messages[] = ['ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $this->getAccountContainer()->finalDN), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = [];
		// get list of existing groups
		$groupList = $this->findGroups();
		$groupMap = [];
		foreach ($groupList as $dn) {
			$groupMap[extractRDNValue($dn)] = $dn;
		}
		$existingUsers = $this->getUserNames();
		$existingCns = $this->getCns();
		$existingMailAccounts = searchLDAPByAttribute(null, null, 'user', ['mail', 'otherMailbox'], ['user']);
		$existingMails = [];
		foreach ($existingMailAccounts as $existingMailAccount) {
			if (!empty($existingMailAccount['mail'][0])) {
				$existingMails[] = $existingMailAccount['mail'][0];
			}
			if (!empty($existingMailAccount['othermailbox'][0])) {
				$existingMails = array_merge($existingMails, $existingMailAccount['othermailbox']);
			}
		}
		$booleanOptions = [_('yes') => true, _('no') => false];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('user', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'user';
			}
			// userPrincipalName
			if (get_preg($rawAccounts[$i][$ids['windowsUser_userPrincipalName']], 'username')) {
				$partialAccounts[$i]['userPrincipalName'] = $rawAccounts[$i][$ids['windowsUser_userPrincipalName']];
				if (array_key_exists($partialAccounts[$i]['userPrincipalName'], $existingUsers)) {
					$errMsg = $this->messages['userPrincipalName'][3];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			else {
				$errMsg = $this->messages['userPrincipalName'][1];
				array_push($errMsg, [$i]);
				$errors[] = $errMsg;
			}
			// first name
			if ($rawAccounts[$i][$ids['windowsUser_firstName']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_firstName']], 'realname')) {
					$partialAccounts[$i]['givenName'] = $rawAccounts[$i][$ids['windowsUser_firstName']];
				}
				else {
					$errMsg = $this->messages['givenName'][1];
					array_push($errMsg, [$i]);
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
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// cn
			if ($rawAccounts[$i][$ids['windowsUser_cn']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_cn']], 'cn')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsUser_cn']];
					if (array_key_exists($partialAccounts[$i]['cn'], $existingCns)) {
						$errMsg = $this->messages['cn'][3];
						array_push($errMsg, [$i]);
						$errors[] = $errMsg;
					}
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, [$i]);
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
						array_push($errMsg, [$i]);
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
				array_push($errMsg, [$i]);
				$errors[] = $errMsg;
			}
			// display name
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedisplayName')) {
				if ($rawAccounts[$i][$ids['windowsUser_displayName']] != "") {
					$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['windowsUser_displayName']];
				}
				elseif (!empty($partialAccounts[$i]['cn'])) {
					$partialAccounts[$i]['displayName'] = $partialAccounts[$i]['cn'];
				}
			}
			// initials
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideinitials')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_initials', 'initials');
			}
			// description
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedescription')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_description', 'description');
			}
			// street
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidestreetAddress')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_streetAddress', 'streetAddress');
			}
			// post office box
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostOfficeBox')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_postOfficeBox', 'postOfficeBox');
			}
			// postal code
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidepostalCode')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_postalCode', 'postalCode',
					'postalCode', $this->messages['postalCode'][1], $errors);
			}
			// location
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidel')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_l', 'l');
			}
			// state
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidest')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_state', 'st');
			}
			// office name
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidephysicalDeliveryOfficeName')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_officeName', 'physicalDeliveryOfficeName');
			}
			// mail
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidemail') && ($rawAccounts[$i][$ids['windowsUser_mail']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_mail']], 'email')) {
					$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['windowsUser_mail']];
					if (in_array_ignore_case($partialAccounts[$i]['mail'], $existingMails)) {
						$errMsg = $this->messages['mail'][3];
						array_push($errMsg, [$i, $partialAccounts[$i]['mail']]);
						$errors[] = $errMsg;
					}
				}
				else {
					$errMsg = $this->messages['mail'][1];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// mail aliases
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherMailbox') && isset($ids['windowsUser_otherMailbox']) && ($rawAccounts[$i][$ids['windowsUser_otherMailbox']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_otherMailbox']]);
				$partialAccounts[$i]['otherMailbox'] = $valueList;
				for ($x = 0; $x < sizeof($valueList); $x++) {
					if (!get_preg($valueList[$x], 'email')) {
						$errMsg = $this->messages['otherMailbox'][1];
						array_push($errMsg, [$i]);
						$errors[] = $errMsg;
						break;
					}
					if (in_array_ignore_case($valueList[$x], $existingMails)) {
						$errMsg = $this->messages['otherMailbox'][3];
						array_push($errMsg, [$i, $valueList[$x]]);
						$errors[] = $errMsg;
					}
				}
			}
			// telephone
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidetelephoneNumber')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_telephoneNumber', 'telephoneNumber',
					'telephone', $this->messages['telephoneNumber'][1], $errors);
			}
			// other telephone
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideotherTelephone')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherTelephone', 'otherTelephone', 'telephone', $this->messages['otherTelephone'][1], $errors, '/;[ ]*/');
			}
			// fax number
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidefacsimileTelephoneNumber')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_facsimileTelephoneNumber', 'facsimileTelephoneNumber',
					'telephone', $this->messages['facsimileTelephoneNumber'][1], $errors);
			}
			// website
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidewWWHomePage')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_webSite', 'wWWHomePage');
			}
			// other websites
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideurl')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_otherWebSites', 'url', null, null, $errors, '/;[ ]*/');
			}
			// user account control
			$userAccountControlAttr['useraccountcontrol'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
			// deactivated
			if ($rawAccounts[$i][$ids['windowsUser_deactivated']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]])) {
					$errors[] = ['ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_deactivated', _('Please enter either yes or no.')];
				}
				else {
					$this->setIsDeactivated($booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]], $userAccountControlAttr);
				}
			}
			// password does not expire
			if ($rawAccounts[$i][$ids['windowsUser_noExpire']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_noExpire']]])) {
					$errors[] = ['ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_noExpire', _('Please enter either yes or no.')];
				}
				else {
					$this->setIsNeverExpiring($userAccountControlAttr, $booleanOptions[$rawAccounts[$i][$ids['windowsUser_noExpire']]]);
				}
			}
			// account expiration
			if (!empty($rawAccounts[$i][$ids['windowsUser_accountExpires']])) {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_accountExpires']], 'date')) {
					$dateParts = explode('-', $rawAccounts[$i][$ids['windowsUser_accountExpires']]);
					$partialAccounts[$i]['accountexpires'] = $this->buildExpirationDate($dateParts[2], $dateParts[1], $dateParts[0]);
				}
				else {
					$errMsg = $this->messages['accountexpires'][1];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// require smartcard
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideRequireSmartcard') && ($rawAccounts[$i][$ids['windowsUser_requireCard']] != "")) {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_requireCard']]])) {
					$errors[] = ['ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_requireCard', _('Please enter either yes or no.')];
				}
				else {
					$this->setIsSmartCardRequired($userAccountControlAttr, $booleanOptions[$rawAccounts[$i][$ids['windowsUser_requireCard']]]);
				}
			}
			$partialAccounts[$i]['useraccountcontrol'][0] = $userAccountControlAttr['useraccountcontrol'][0];
			// end user account control
			// password change at next login
			if (($rawAccounts[$i][$ids['windowsUser_pwdMustChange']] != "")
				&& !isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_pwdMustChange']]])) {
				$errors[] = ['ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_pwdMustChange', _('Please enter either yes or no.')];
				// attribute must be set in postModify
			}
			// profile path
			if (!$this->isBooleanConfigOptionSet('windowsUser_hideprofilePath') && ($rawAccounts[$i][$ids['windowsUser_profilePath']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_profilePath']], 'UNC')) {
					$partialAccounts[$i]['profilePath'] = $rawAccounts[$i][$ids['windowsUser_profilePath']];
				}
				else {
					$errMsg = $this->messages['profilePath'][1];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// logon script
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidescriptPath') && ($rawAccounts[$i][$ids['windowsUser_scriptPath']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_scriptPath']], 'logonscript')) {
					$partialAccounts[$i]['scriptPath'] = $rawAccounts[$i][$ids['windowsUser_scriptPath']];
				}
				else {
					$errMsg = $this->messages['scriptPath'][1];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// home drive
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive') && (!empty($rawAccounts[$i][$ids['windowsUser_homeDrive']]))) {
				if (preg_match("/[d-z]:/i", $rawAccounts[$i][$ids['windowsUser_homeDrive']])) {
					$partialAccounts[$i]['homeDrive'] = strtoupper($rawAccounts[$i][$ids['windowsUser_homeDrive']]);
				}
				else {
					$errMsg = $this->messages['homeDrive'][0];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// home directory
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDirectory') && ($rawAccounts[$i][$ids['windowsUser_homeDirectory']] != "")) {
				if (empty($partialAccounts[$i]['homeDrive']) || get_preg($rawAccounts[$i][$ids['windowsUser_homeDirectory']], 'UNC')) {
					$partialAccounts[$i]['homeDirectory'] = $rawAccounts[$i][$ids['windowsUser_homeDirectory']];
				}
				else {
					$errMsg = $this->messages['homeDirectory'][1];
					array_push($errMsg, [$i]);
					$errors[] = $errMsg;
				}
			}
			// workstations
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_workstations', 'userWorkstations',
				'workstations', $this->messages['workstations'][1], $errors);
			// groups
			if (isset($ids['windowsUser_groups']) && ($rawAccounts[$i][$ids['windowsUser_groups']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_groups']]);
				$invalidGroups = [];
				foreach ($valueList as $group) {
					if (!in_array($group, $groupList) && !isset($groupMap[$group])) {
						$invalidGroups[] = $group;
					}
				}
				if (sizeof($invalidGroups) > 0) {
					$errors[] = ['ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_groups',
						_('LAM was unable to find a group with this name!') . '<br>' . htmlspecialchars(implode('; ', $invalidGroups))];
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
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_department', 'department', null, null, $errors);
			}
			// department numbers
			if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsUser_departmentNumber', 'departmentNumber', null, null, $errors, '/;[ ]*/');
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
	function doUploadPostActions(&$data, $ids, $failed, &$temp, &$accounts, $selectedModules, $type) {
		if (!checkIfWriteAccessIsAllowed($this->get_scope())) {
			die();
		}
		// on first call generate list of ldap operations
		if (!isset($temp['counter'])) {
			$groupList = $this->findGroups();
			$groupMap = [];
			foreach ($groupList as $dn) {
				$groupMap[extractRDNValue($dn)] = $dn;
			}
			$temp['groups'] = &$groupList;
			$temp['groupMap'] = &$groupMap;
			$temp['members'] = [];
			$temp['memberCount'] = 0;
			$temp['pwdChange'] = [];
			$temp['pwdChangeCount'] = 0;
			$groupCol = $ids['windowsUser_groups'];
			$passwordChangeRequiredCol = $ids['windowsUser_pwdMustChange'];
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) {
					continue;
				} // ignore failed accounts
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
			return [
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => []
			];
		}
		// add users to groups
		elseif (sizeof($temp['members']) > 0) {
			$keys = array_keys($temp['members']);
			$group = $keys[0];
			$member = array_pop($temp['members'][$group]);
			$success = @ldap_mod_add($_SESSION['ldap']->server(), $group, ['member' => $member]);
			$errors = [];
			if (!$success) {
				$errors[] = [
					"ERROR",
					_("LAM was unable to modify group memberships for group: %s"),
					getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					[$group]
				];
			}
			if (sizeof($temp['members'][$group]) == 0) {
				unset($temp['members'][$group]);
			}
			$memberPercentage = (100 * ($temp['memberCount'] - sizeof($temp['members']))) / $temp['counter'];
			return [
				'status' => 'inProgress',
				'progress' => $memberPercentage,
				'errors' => $errors
			];
		}
		// force password change
		elseif (sizeof($temp['pwdChange']) > 0) {
			$dn = array_pop($temp['pwdChange']);
			$success = @ldap_mod_replace($_SESSION['ldap']->server(), $dn, ['pwdLastSet' => '0']);
			$errors = [];
			if (!$success) {
				$errors[] = [
					"ERROR",
					_("Was unable to modify attributes of DN: %s."),
					getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					[$dn]
				];
			}
			$pwdPercentage = (100 * ($temp['memberCount'] + ($temp['pwdChangeCount'] - sizeof($temp['pwdChange'])))) / $temp['counter'];
			return [
				'status' => 'inProgress',
				'progress' => $pwdPercentage,
				'errors' => $errors
			];
		}
		// all modifications are done
		else {
			return [
				'status' => 'finished',
				'progress' => 100,
				'errors' => []
			];
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
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
		$this->addSimplePDFField($return, 'departmentNumber', _('Department number'));
		$this->addSimplePDFField($return, 'ou', _('Organisational unit'));
		$this->addSimplePDFField($return, 'o', _('Organisation'));
		$this->addSimplePDFField($return, 'manager', _('Manager'));
		$this->addSimplePDFField($return, 'userWorkstations', _('Workstations'));
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
		$groups = [];
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
		$return->add(new htmlResponsiveSelect('windowsUser_userPrincipalNameDomain', $domains, [], _('Domain'), 'userPrincipalNameDomain'));
		// group memberships
		$groups = $this->findGroups();
		$groupOptions = [];
		foreach ($groups as $dn) {
			$groupOptions[extractRDNValue($dn)] = $dn;
		}
		$groupSelect = new htmlResponsiveSelect('windowsUser_groups', $groupOptions, [], _('Groups'), 'groups', 10);
		$groupSelect->setHasDescriptiveElements(true);
		$groupSelect->setMultiSelect(true);
		$return->add($groupSelect);
		// home drive
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidehomeDrive')) {
			$drives = ['-'];
			for ($i = 90; $i > 67; $i--) {
				$drives[] = chr($i) . ':';
			}
			$driveSelect = new htmlResponsiveSelect('windowsUser_homeDrive', $drives, ['-'], _('Home drive'), 'homeDrive');
			$return->add($driveSelect);
		}
		// force password change
		$passwordChangeCheckbox = new htmlResponsiveInputCheckbox('windowsUser_pwdMustChange', true, _('Password change at next login'), 'pwdMustChange');
		$return->add($passwordChangeCheckbox);
		// password does not expire
		$passwordNoExpireCheckbox = new htmlResponsiveInputCheckbox('windowsUser_pwdNoExpire', false, _('Password does not expire'), 'noExpire');
		$return->add($passwordNoExpireCheckbox);
		if (!$this->isBooleanConfigOptionSet('windowsUser_hideWorkstations')) {
			// allowed workstations
			$return->add(new htmlResponsiveInputField(_('Workstations'), 'windowsUser_userWorkstations', '', 'workstations'));
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
		// load domain
		if (isset($profile['windowsUser_userPrincipalNameDomain'][0])) {
			$user = empty($this->attributes['userPrincipalName'][0]) ? '' : $this->attributes['userPrincipalName'][0];
			$userParts = explode('@', $user);
			if (!empty($profile['windowsUser_userPrincipalNameDomain'][0])) {
				$user = $userParts[0] . '@' . $profile['windowsUser_userPrincipalNameDomain'][0];
			}
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
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment')
			&& isset($profile['windowsUser_department'][0])
			&& ($profile['windowsUser_department'][0] != '')) {
			$departments = explode(';', $profile['windowsUser_department'][0]);
			// remove extra spaces and set attributes
			$this->attributes['department'] = array_map('trim', $departments);
		}
		// department numbers
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber')
			&& !empty($profile['windowsUser_departmentNumber'][0])) {
			$departmentNumbers = explode(';', $profile['windowsUser_departmentNumber'][0]);
			// remove extra spaces and set attributes
			$this->attributes['departmentNumber'] = array_map('trim', $departmentNumbers);
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
		// no password expiration
		if (isset($profile['windowsUser_pwdNoExpire'][0]) && ($profile['windowsUser_pwdNoExpire'][0] === "true")) {
			if (!isset($this->attributes['useraccountcontrol'][0])) {
				$this->attributes['useraccountcontrol'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
			}
			windowsUser::setIsNeverExpiring($this->attributes, true);
		}
		// force password change
		if (isset($profile['windowsUser_pwdMustChange'][0]) && ($profile['windowsUser_pwdMustChange'][0] === "true")) {
			$this->attributes['pwdLastSet'][0] = '0';
		}
		elseif (isset($profile['windowsUser_pwdMustChange'][0]) && ($profile['windowsUser_pwdMustChange'][0] == "false")) {
			if (isset($this->orig['pwdLastSet'][0]) && ($this->orig['pwdLastSet'][0] !== '0')) {
				$this->attributes['pwdLastSet'][0] = $this->orig['pwdLastSet'][0];
			}
			else {
				$this->attributes['pwdLastSet'][0] = '-1';
			}
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
		$return = [];
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
		$this->addMultiValueSelfServiceTextField($return, 'departmentNumber', _('Department number'), $fields, $attributes, $readOnlyFields);
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
		if (in_array('proxyAddresses', $fields)) {
			$proxyAddresses = [];
			if (isset($attributes['proxyAddresses'])) {
				$proxyAddresses = $attributes['proxyAddresses'];
			}
			array_map('htmlspecialchars', $proxyAddresses);
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('proxyAddresses', _('Proxy-Addresses'))));
			$row->addField(new htmlOutputText(implode('<br>', $proxyAddresses), false));
			$return['proxyAddresses'] = $row;
		}
		if (in_array('otherMailbox', $fields)) {
			$otherMailbox = [];
			if (isset($attributes['otherMailbox'])) {
				$otherMailbox = $attributes['otherMailbox'];
			}
			array_map('htmlspecialchars', $otherMailbox);
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('otherMailbox', _('Email alias'))));
			$row->addField(new htmlOutputText(implode('<br>', $otherMailbox), false));
			$return['otherMailbox'] = $row;
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
	 * @return array messages and attributes (array('messages' => [], 'add' => array('mail' => array('test@test.com')), 'del' => [], 'mod' => [], 'info' => []))
	 */
	function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if (in_array('unicodePwd', $fields)
			&& isset($_POST['windowsUser_unicodePwd'])
			&& ($_POST['windowsUser_unicodePwd'] != '')) {
			if ($_POST['windowsUser_unicodePwd'] != $_POST['windowsUser_unicodePwd2']) {
				$return['messages'][] = $this->messages['unicodePwd'][0];
			}
			else {
				if (!get_preg($_POST['windowsUser_unicodePwd'], 'password')) {
					$return['messages'][] = $this->messages['unicodePwd'][1];
				}
				else {
					$userName = empty($attributes['userPrincipalName'][0]) ? null : $attributes['userPrincipalName'][0];
					$additionalAttrs = [];
					if (!empty($attributes['sn'][0])) {
						$additionalAttrs[] = $attributes['sn'][0];
					}
					if (!empty($attributes['givenName'][0])) {
						$additionalAttrs[] = $attributes['givenName'][0];
					}
					$pwdPolicyResult = checkPasswordStrength($_POST['windowsUser_unicodePwd'], $userName, $additionalAttrs);
					if ($pwdPolicyResult === true) {
						$this->setSelfServicePassword($return);
						$return['info']['userPasswordClearText'][0] = $_POST['windowsUser_unicodePwd'];
					}
					else {
						$return['messages'][] = ['ERROR', $pwdPolicyResult];
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
		$this->checkMultiValueSelfServiceTextField($return, 'departmentNumber', $attributes, $fields, $readOnlyFields);
		return $return;
	}

	/**
	 * Sets the user password in self service.
	 * Since the change requires the old password we need to run ldapmodify for this task.
	 *
	 * Enter description here ...
	 * @param array $return return value for checkSelfServiceOptions() (used to add message if any)
	 */
	private function setSelfServicePassword(&$return) {
		$newPasswordVal = self::pwdAttributeValue($_POST['windowsUser_unicodePwd']);
		$oldPassword = lamDecrypt($_SESSION['selfService_clientPassword'], 'SelfService');
		$oldPasswordVal = self::pwdAttributeValue($oldPassword);
		$dn = lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
		$operation = [
			[
				'attrib' => 'unicodePwd',
				'modtype' => LDAP_MODIFY_BATCH_REMOVE,
				'values' => [$oldPasswordVal]
			],
			[
				'attrib' => 'unicodePwd',
				'modtype' => LDAP_MODIFY_BATCH_ADD,
				'values' => [$newPasswordVal]
			]
		];
		@ldap_modify_batch($_SESSION['ldapHandle']->getServer(), $dn, $operation);
		$returnCode = ldap_errno($_SESSION['ldapHandle']->getServer());
		if ($returnCode != 0) {
			$outputMessages = htmlspecialchars(getExtendedLDAPErrorMessage($_SESSION['ldapHandle']->getServer()));
			// Active Directory message translations
			if ((strpos($outputMessages, 'DSID-03190F80') !== false) && (strpos($outputMessages, 'unicodePwd') !== false)) {
				$outputMessages = _('Your password does not meet the password strength qualifications. Please retry with another one.') . '<br><br>' . $outputMessages;
			}
			logNewMessage(LOG_ERR, 'Changing user password failed: ' . $outputMessages);
			$return['messages'][] = ['ERROR', _('Unable to change password.'), $outputMessages];
		}
		else {
			// update session password for next page load
			$_SESSION['selfService_clientPasswordNew'] = $_POST['windowsUser_unicodePwd'];
			if (isset($_SESSION['selfService_passwordExpired'])) {
				unset($_SESSION['selfService_passwordExpired']);
			}
			$_SESSION['selfService_resetPasswordChangeOnly'] = true;
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
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return [];
		}
		// check password strength
		$user = empty($this->attributes['userPrincipalName'][0]) ? null : $this->attributes['userPrincipalName'][0];
		$additionalAttrs = [];
		if (!empty($this->attributes['sn'][0])) {
			$additionalAttrs[] = $this->attributes['sn'][0];
		}
		if (!empty($this->attributes['givenName'][0])) {
			$additionalAttrs[] = $this->attributes['givenName'][0];
		}
		$checkResult = checkPasswordStrength($password, $user, $additionalAttrs);
		if ($checkResult !== true) {
			return [['ERROR', $checkResult]];
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
		return [];
	}

	/**
	 * Creates the LDAP password value.
	 *
	 * @param String $password password
	 */
	public static function pwdAttributeValue($password) {
		return convertUtf8ToUtf16Le('"' . $password . '"');
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
	 * @return DateTime|false false or locked till
	 */
	public static function getPasswordLocked($attrs) {
		if (empty($attrs['lockouttime'][0])) {
			return false;
		}
		$lockoutDuration = self::getDomainLockoutDuration();
		if (empty($lockoutDuration) || ($lockoutDuration > -1)) {
			return false;
		}
		$lockoutDurationSeconds = substr($lockoutDuration, 0, -7);
		$lockoutTime = self::getFileTime($attrs['lockouttime'][0]);
		$unlockTime = clone $lockoutTime;
		$unlockTime->add(new DateInterval('PT' . abs($lockoutDurationSeconds) . 'S'));
		$now = new DateTime('now', getTimeZone());
		if ($unlockTime > $now) {
			$unlockTime->setTimezone(getTimeZone());
			return $unlockTime;
		}
		return false;
	}

	/**
	 * Returns the domain lockout duration for this DN.
	 */
	private static function getDomainLockoutDuration() {
		if (!isset($_SESSION['windowsUser_cacheDomainLockoutDuration'])) {
			$lockoutValue = self::getDomainPolicyAttribute('lockoutduration');
			if (isset($lockoutValue[0])) {
				$lockoutValue = $lockoutValue[0];
			}
			else {
				$lockoutValue = null;
			}
			$_SESSION['windowsUser_cacheDomainLockoutDuration'] = $lockoutValue;
		}
		return $_SESSION['windowsUser_cacheDomainLockoutDuration'];
	}

	/**
	 * Returns the value of a domain policy attribute.
	 *
	 * @param string $attributeName attribute name
	 * @return array value
	 */
	public static function getDomainPolicyAttribute(string $attributeName): array {
		$attributeName = strtolower($attributeName);
		$rootDSE = ldapGetDN("", ['defaultnamingcontext']);
		if (empty($rootDSE['defaultnamingcontext'][0])) {
			return [];
		}
		$namingContext = $rootDSE['defaultnamingcontext'][0];
		$policyOptions = ldapGetDN($namingContext, [$attributeName]);
		if (!isset($policyOptions[$attributeName])) {
			return [];
		}
		return $policyOptions[$attributeName];
	}

	/**
	 * Unlocks the user password. This resets 'lockoutTime' to 0.
	 *
	 * @param array|null $attributes LDAP attributes
	 */
	public function unlockPassword(?array &$attributes = null) {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (!empty($attributes['lockouttime'][0])) {
			$attributes['lockouttime'][0] = '0';
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
		if (!isset($attrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($attrs['useraccountcontrol'][0]) & windowsUser::AC_SMARTCARD_REQUIRED;
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
		if (!isset($attrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($attrs['useraccountcontrol'][0]) & windowsUser::AC_PWD_NEVER_EXPIRES;
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
		$return = [];
		$types = ['group'];
		$results = searchLDAPByFilter('(objectClass=group)', ['dn'], $types);
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
		$domains = [];
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
	 * Returns a list of all user names in LDAP.
	 *
	 * @return array user names
	 */
	private function getUserNames() {
		if ($this->cachedUserNameList != null) {
			return $this->cachedUserNameList;
		}
		$this->cachedUserNameList = [];
		$attrs = ['userPrincipalName'];
		$filter = '(&(objectClass=user)(userPrincipalName=*))';
		$typeManager = new TypeManager();
		$typesUser = $typeManager->getConfiguredTypesForScope('user');
		$suffixes = [];
		foreach ($typesUser as $type) {
			$suffixes[] = $type->getSuffix();
		}
		$suffixes = array_unique($suffixes);
		foreach ($suffixes as $suffix) {
			$result = searchLDAP($suffix, $filter, $attrs);
			foreach ($result as $resultEntry) {
				$this->cachedUserNameList[$resultEntry['userprincipalname'][0]] = $resultEntry['dn'];
			}
		}
		return $this->cachedUserNameList;
	}

	/**
	 * Returns a list of all CNs in LDAP.
	 *
	 * @return array CN list
	 */
	private function getCns() {
		if ($this->cachedCnList != null) {
			return $this->cachedCnList;
		}
		$this->cachedCnList = [];
		$attrs = ['cn'];
		$filter = '(&(objectClass=user)(cn=*))';
		$typeManager = new TypeManager();
		$typesUser = $typeManager->getConfiguredTypesForScope('user');
		$suffixes = [];
		foreach ($typesUser as $type) {
			$suffixes[] = $type->getSuffix();
		}
		$suffixes = array_unique($suffixes);
		foreach ($suffixes as $suffix) {
			$result = searchLDAP($suffix, $filter, $attrs);
			foreach ($result as $resultEntry) {
				$this->cachedCnList[$resultEntry['cn'][0]] = $resultEntry['dn'];
			}
		}
		return $this->cachedCnList;
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
		if (empty($attributes['accountexpires'][0]) || ($attributes['accountexpires'][0] == '0')
			|| ($attributes['accountexpires'][0] == '9223372036854775807')) {
			return ' - ';
		}
		return $this->formatFileTime($attributes['accountexpires'][0]);
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
		if (empty($value) || !get_preg($value, 'digit')) {
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
			unset($this->attributes['accountexpires']);
			return;
		}
		$this->attributes['accountexpires'][0] = $this->buildExpirationDate($year, $month, $day);
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
	 * {@inheritDoc}
	 * @see baseModule::get_configOptions()
	 */
	public function get_configOptions($scopes, $allScopes) {
		$typeManager = new TypeManager($_SESSION['conf_config']);
		// configuration options
		$configContainer = new htmlResponsiveRow();
		$configContainer->add(new htmlResponsiveInputTextarea('windowsUser_domains', '', 30, 3, _('Domains'), 'domains'), 12);
		$displayOptions = [
			'dn' => self::DISPLAY_GROUPS_DN,
			'cn' => self::DISPLAY_GROUPS_CN,
		];
		$groupDisplaySelect = new htmlResponsiveSelect('windowsUser_displayGroups', $displayOptions, [], _('Display format'), 'displayGroups');
		$groupDisplaySelect->setHasDescriptiveElements(true);
		$configContainer->add($groupDisplaySelect, 12);
		$configHiddenGroup = new htmlGroup();
		$configHiddenGroup->addElement(new htmlOutputText(_('Hidden options')));
		$configHiddenGroup->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainer->add($configHiddenGroup, 12);
		$hiddenOptions = [];
		$hiddenOptions[_('Display name')] = ['windowsUser_hidedisplayName', false];
		$hiddenOptions[_('Proxy-Addresses')] = ['windowsUser_hideproxyAddresses', true];
		$hiddenOptions[_('Fax number')] = ['windowsUser_hidefacsimileTelephoneNumber', false];
		$hiddenOptions[_('Mobile')] = ['windowsUser_hidemobile', false];
		$hiddenOptions[_('Other mobiles')] = ['windowsUser_hideotherMobile', true];
		$hiddenOptions[_('Pager')] = ['windowsUser_hidepager', true];
		$hiddenOptions[_('Other pagers')] = ['windowsUser_hideotherPager', true];
		$hiddenOptions[_('User name (pre W2K)')] = ['windowsUser_hidesAMAccountName', true];
		$hiddenOptions[_('NIS name')] = ['windowsUser_hidemsSFU30Name', true];
		$hiddenOptions[_('NIS domain')] = ['windowsUser_hidemsSFU30NisDomain', true];
		$hiddenOptions[_('Last password change')] = ['windowsUser_hidepwdLastSet', false];
		$hiddenOptions[_('Last login')] = ['windowsUser_hidelastLogonTimestamp', false];
		$hiddenOptions[_('Workstations')] = ['windowsUser_hideWorkstations', false];
		$hiddenOptions[_('Photo')] = ['windowsUser_hidejpegPhoto', true];
		$hiddenOptions[_('Job title')] = ['windowsUser_hidetitle', true];
		$hiddenOptions[_('Car license')] = ['windowsUser_hidecarLicense', true];
		$hiddenOptions[_('Employee number')] = ['windowsUser_hideemployeeNumber', true];
		$hiddenOptions[_('Employee type')] = ['windowsUser_hideemployeeType', true];
		$hiddenOptions[_('Business category')] = ['windowsUser_hidebusinessCategory', true];
		$hiddenOptions[_('Company')] = ['windowsUser_hidecompany', true];
		$hiddenOptions[_('Department')] = ['windowsUser_hidedepartment', true];
		$hiddenOptions[_('Department number')] = ['windowsUser_hidedepartmentNumber', true];
		$hiddenOptions[_('Organisational unit')] = ['windowsUser_hideou', true];
		$hiddenOptions[_('Organisation')] = ['windowsUser_hideo', true];
		$hiddenOptions[_('Manager')] = ['windowsUser_hidemanager', true];
		$hiddenOptions[_('Profile path')] = ['windowsUser_hideprofilePath', false];
		$hiddenOptions[_('Logon script')] = ['windowsUser_hidescriptPath', false];
		$hiddenOptions[_('Home drive')] = ['windowsUser_hidehomeDrive', false];
		$hiddenOptions[_('Home directory')] = ['windowsUser_hidehomeDirectory', false];
		$hiddenOptions[_('Initials')] = ['windowsUser_hideinitials', false];
		$hiddenOptions[_('Description')] = ['windowsUser_hidedescription', false];
		$hiddenOptions[_('Street')] = ['windowsUser_hidestreetAddress', false];
		$hiddenOptions[_('Post office box')] = ['windowsUser_hidepostOfficeBox', false];
		$hiddenOptions[_('Postal code')] = ['windowsUser_hidepostalCode', false];
		$hiddenOptions[_('Location')] = ['windowsUser_hidel', false];
		$hiddenOptions[_('State')] = ['windowsUser_hidest', false];
		$hiddenOptions[_('Office name')] = ['windowsUser_hidephysicalDeliveryOfficeName', false];
		$hiddenOptions[_('Email address')] = ['windowsUser_hidemail', false];
		$hiddenOptions[_('Email alias')] = ['windowsUser_hideotherMailbox', false];
		$hiddenOptions[_('Telephone number')] = ['windowsUser_hidetelephoneNumber', false];
		$hiddenOptions[_('Other telephone numbers')] = ['windowsUser_hideotherTelephone', false];
		$hiddenOptions[_('Web site')] = ['windowsUser_hidewWWHomePage', false];
		$hiddenOptions[_('Other web sites')] = ['windowsUser_hideurl', false];
		$hiddenOptions[_('Require smartcard')] = ['windowsUser_hideRequireSmartcard', false];
		ksort($hiddenOptions);
		foreach ($hiddenOptions as $hiddenOptionLabel => $hiddenOption) {
			$configContainer->add(new htmlResponsiveInputCheckbox($hiddenOption[0], $hiddenOption[1], $hiddenOptionLabel, null, true), 12, 4);
		}
		$syncTypes = $typeManager->getConfiguredTypesForScopes(['group', 'gon', 'user']);
		$syncActive = false;
		$possibleSyncModules = ['groupOfNames', 'groupOfMembers', 'groupOfUniqueNames', 'posixAccount'];
		foreach ($syncTypes as $syncType) {
			$modules = $syncType->getModules();
			foreach ($possibleSyncModules as $possibleModule) {
				if (in_array($possibleModule, $modules)) {
					$syncActive = true;
					break;
				}
			}
		}
		if ($syncActive) {
			$syncGroupsCheckbox = new htmlResponsiveInputCheckbox('windowsUser_syncGroups', false, _('Sync groups'), null, true);
			$syncGroupsCheckbox->setTableRowsToHide(['windowsUser_syncGroupsExclusions']);
			$configContainer->add($syncGroupsCheckbox, 12, 4);
			$configContainer->addVerticalSpacer('2rem');
			$configContainer->add(new htmlResponsiveInputTextarea('windowsUser_syncGroupsExclusions', '', 20, 4, _('Exclude from group sync'), 'excludeFromGroupSync'), 12);
		}
		$advancedOptions = new htmlResponsiveRow();
		$advancedOptions->add(new htmlSubTitle(_('Photo')), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum width (px)'), 'windowsUser_jpegPhoto_maxWidth', null, 'crop'), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum height (px)'), 'windowsUser_jpegPhoto_maxHeight', null, 'crop'), 12);
		$advancedOptions->add(new htmlResponsiveInputField(_('Maximum file size (kB)'), 'windowsUser_jpegPhoto_maxSize'), 12);
		$advancedOptionsAccordion = new htmlAccordion('windowsUserAdvancedOptions', [_('Advanced options') => $advancedOptions], false);
		$configContainer->add($advancedOptionsAccordion);
		return $configContainer;
	}

	/**
	 * Loads cached data from LDAP such as departmets etc.
	 */
	private function initCache() {
		if ($this->departmentCache != null) {
			return;
		}
		$attrs = [];
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartment', true)) {
			$attrs[] = 'department';
		}
		if (!$this->isBooleanConfigOptionSet('windowsUser_hidedepartmentNumber', true)) {
			$attrs[] = 'departmentNumber';
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
		$departments = [];
		$departmentNumbers = [];
		$ous = [];
		$os = [];
		$titles = [];
		$employeeTypes = [];
		$businessCategories = [];
		if (sizeof($attrs) > 0) {
			$result = searchLDAPByFilter('(objectClass=user)', $attrs, [$this->get_scope()]);
			foreach ($result as $attributes) {
				if (isset($attributes['department'])) {
					foreach ($attributes['department'] as $val) {
						$departments[] = $val;
					}
				}
				if (isset($attributes['departmentnumber'])) {
					foreach ($attributes['departmentnumber'] as $val) {
						$departmentNumbers[] = $val;
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
		$this->departmentNumberCache = array_values(array_unique($departmentNumbers));
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
		return [
			new WindowsPasswordNotifyJob(),
			new WindowsAccountExpirationCleanupJob(),
			new WindowsAccountExpirationNotifyJob(),
			new WindowsManagedGroupsNotifyJob()
		];
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
		if (!empty($_POST['userPrincipalName'])) {
			$replacements['user'] = $_POST['userPrincipalName'];
		}
		elseif (!empty($this->attributes['userPrincipalName'][0])) {
			$userParts = explode('@', $this->attributes['userPrincipalName'][0]);
			if ((sizeof($userParts) === 2) && !empty($userParts[0])) {
				$replacements['user'] = $userParts[0];
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
	 * Returns if the given account is expired.
	 *
	 * @param array $attrs LDAP attributes
	 * @return bool expired
	 */
	public static function isAccountExpired($attrs) {
		$attrs = array_change_key_case($attrs, CASE_LOWER);
		if (empty($attrs['accountexpires'][0])) {
			return false;
		}
		$value = $attrs['accountexpires'][0];
		if ($value < 1) {
			return false;
		}
		$seconds = substr($value, 0, -7);
		$time = new DateTime('1601-01-01', new DateTimeZone('UTC'));
		$time->add(new DateInterval('PT' . $seconds . 'S'));
		$now = new DateTime('now', getTimeZone());
		return ($time < $now);
	}

	/**
	 * Returns the list of groups.
	 *
	 * @return array DNs of Windows groups
	 */
	public function getGroupList() {
		return $this->groupList;
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
		if (!empty($this->attributes['userPrincipalName'][0])) {
			$row->addLabel(new htmlOutputText(_('User name')));
			$row->addField(new htmlOutputText($this->attributes['userPrincipalName'][0]));
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
		if (!in_array_ignore_case('user', $this->attributes['objectClass'])) {
			return [];
		}
		return [
			new PasswordQuickChangeOption('syncWindows', _('Change Windows password'))
		];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		$attrs = [];
		if (isset($_POST['syncWindows'])) {
			$attrs['unicodePwd'][0] = self::pwdAttributeValue($password);
		}
		return $attrs;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string {
		if (isset($this->attributes['userPrincipalName'][0])) {
			return $this->attributes['userPrincipalName'][0];
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
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$details = [];
		if (self::isDeactivated($attributes)) {
			$details[] = AccountStatusDetails::newLocked(_('Deactivated'), self::STATUS_ACCOUNT_LOCKED);
		}
		$passwordLockedTime = self::getPasswordLocked($attributes);
		if ($passwordLockedTime !== false) {
			$details[] = AccountStatusDetails::newLocked(_('Locked till') . ' ' . $passwordLockedTime->format('Y-m-d H:i:s'), self::STATUS_PASSWORD_LOCKED);
		}
		if (self::isAccountExpired($attributes)) {
			$details[] = AccountStatusDetails::newExpired(_('Account is expired'), self::STATUS_ACCOUNT_EXPIRED);
		}
		return $details;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusRequiredAttributes(ConfiguredType $type): array {
		return ['useraccountcontrol', 'lockouttime', 'accountexpires'];
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusPossibleLockOptions(ConfiguredType $type, ?array &$attributes): array {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$options = [];
		if (!self::isDeactivated($attributes)) {
			$options[] = AccountStatusDetails::newLocked(_('Deactivated'), self::STATUS_ACCOUNT_LOCKED);
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformLock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_ACCOUNT_LOCKED, $lockIds)) {
			self::setIsDeactivated(true, $attributes);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformUnlock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_ACCOUNT_LOCKED, $lockIds)) {
			self::setIsDeactivated(false, $attributes);
		}
		if (in_array(self::STATUS_ACCOUNT_EXPIRED, $lockIds)) {
			$attributes['accountexpires'][0] = '0';
		}
		if (in_array(self::STATUS_PASSWORD_LOCKED, $lockIds)) {
			self::unlockPassword($attributes);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'userprincipalname' => _('User name'),
			'cn' => _('Common name'),
			'displayname' => _('Display name'),
			'givenname' => _('First name'),
			'sn' => _('Last name'),
			'mail' => _('Email address'),
			'accountexpires' => _('Account expiration date'),
			'whencreated' => _('Creation time'),
			'whenchanged' => _('Change date'),
		];
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName): ?callable {
		if ($attributeName === 'mail') {
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
						$result = @ldap_read($_SESSION['ldap']->server(), $entry['dn'], $attribute . "=*", [$attribute], 0, 0, 0, LDAP_DEREF_NEVER);
						if ($result) {
							$tempEntry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
							if ($tempEntry) {
								$binData = ldap_get_values_len($_SESSION['ldap']->server(), $tempEntry, $attribute);
								$entry[$attribute] = $binData;
							}
						}
					}
					$tempFilesManager = new LamTemporaryFilesManager();
					$jpegFileName = $tempFilesManager->registerTemporaryFile('.jpg');
					$handle = $tempFilesManager->openTemporaryFileForWrite($jpegFileName);
					fwrite($handle, $entry[$attribute][0]);
					fclose($handle);
					$photoFile = $tempFilesManager->getResourceLink($jpegFileName);
					$image = new htmlImage($photoFile);
					$image->enableLightbox();
					$image->setCSSClasses(['thumbnail']);
					return $image;
				}
				return null;
			};
		}
		elseif ($attributeName === 'accountexpires') {
			return function(array $entry, string $attribute): ?htmlElement {
				return new htmlOutputText($this->formatAccountExpires($entry));
			};
		}
		elseif (($attributeName === 'whencreated') || ($attributeName === 'whenchanged')) {
			return function(array $entry, string $attribute): ?htmlElement {
				$value = ' - ';
				if (!empty($entry[$attribute][0])) {
					$value = formatLDAPTimestamp($entry[$attribute][0]);
				}
				return new htmlOutputText($value);
			};
		}
		elseif ($attributeName === 'manager') {
			return function(array $entry, string $attribute): ?htmlElement {
				if (!empty($entry[$attribute][0])) {
					$value = $entry[$attribute][0];
					$typeManager = new TypeManager();
					$replaced = false;
					foreach ($typeManager->getConfiguredTypes() as $type) {
						if ((stripos($value, $type->getSuffix()) > 0) && !isAccountTypeHidden($type->getId())) {
							$value = '<a href="../account/edit.php?type=' . $type->getId() . '&amp;DN=\'' . $value . '\'">' . getAbstractDN($value) . '</a>';
							$replaced = true;
							break;
						}
					}
					if (!$replaced) {
						$value = getAbstractDN($value);
					}
					return new htmlDiv(null, new htmlOutputText($value, false), ['rightToLeftText']);
				}
				return null;
			};
		}
		return null;
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
			$maxPwdAge = [];
			foreach ($typeManager->getConfiguredTypesForScope('user') as $type) {
				$userSuffix = $type->getSuffix();
				if (empty($userSuffix)) {
					$this->jobResultLog->logError('No user suffix set in server profile for ' . $type->getAlias() . '.');
					continue;
				}
				$policyAttrs = windowsUser::getDomainPolicyAttribute('maxPwdAge');
				if (empty($policyAttrs[0])) {
					$this->jobResultLog->logError('No maxPwdAge found for this domain in ' . $type->getAlias() . '.');
					continue;
				}
				$maxPwdAge = $policyAttrs[0];
				$this->jobResultLog->logDebug("Using maxPwdAge = " . $maxPwdAge . ".");
			}
			return ['maxpwdage' => $maxPwdAge];
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
			$sysattrs = [$_SESSION['cfgMain']->getMailAttribute(), 'pwdLastSet', 'accountexpires', 'useraccountcontrol'];
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			return searchLDAPByFilter('(&(pwdLastSet=*)(' . $_SESSION['cfgMain']->getMailAttribute() . '=*))', $attrs, ['user']);
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
			// skip if password does not expire at all
			if (windowsUser::isNeverExpiring($user)) {
				$this->jobResultLog->logDebug($dn . ' does not expire.');
				return;
			}
			// skip if no information about last password change
			if (empty($user['pwdlastset'][0]) || ($user['pwdlastset'][0] < 1)) {
				$this->jobResultLog->logDebug($dn . ' has no valid "pwdLastSet".');
				return;
			}
			// skip if account itself is expired
			if (!empty($user['accountexpires'][0])) {
				$accountExpiration = windowsUser::getFileTime($user['accountexpires'][0]);
				if ($accountExpiration <= $now) {
					$this->jobResultLog->logDebug($dn . ' already expired');
					return;
				}
			}
			// skip if account is deactivated
			if (windowsUser::isDeactivated($user)) {
				$this->jobResultLog->logDebug($dn . ' is deactivated.');
				return;
			}
			if (empty($policyOptions['maxpwdage'])) {
				$this->jobResultLog->logDebug('Skipping ' . $dn . ' as maxpwdage is empty or 0.');
				return;
			}
			$maxPwdAge = $policyOptions['maxpwdage'];
			// calculate time when password expires
			$lastPwdTime = windowsUser::getFileTime($user['pwdlastset'][0]);
			$this->jobResultLog->logDebug("Last password change on " . $lastPwdTime->format('Y-m-d'));
			$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			$this->jobResultLog->logDebug("Number of days before warning " . $numDaysToWarn);
			// expiration date = pwdLastSet - maxpwdage
			$expirationValue = $user['pwdlastset'][0] - $maxPwdAge;
			$this->jobResultLog->logDebug("PwdLastSet: " . $user['pwdlastset'][0]);
			$this->jobResultLog->logDebug("Expiration raw value: " . $expirationValue);
			$expireTime = windowsUser::getFileTime($expirationValue);
			if ($expireTime === null) {
				$this->jobResultLog->logWarning('Invalid expiration values ' . $user['pwdlastset'][0] . ' - ' . $expirationValue);
				return;
			}
			$this->jobResultLog->logDebug("Password expires on " . $expireTime->format('Y-m-d'));
			// skip already expired accounts
			if ($expireTime <= $now) {
				$this->jobResultLog->logDebug($dn . ' already expired');
				return;
			}
			// calculate time of notification
			$notifyTime = clone $expireTime;
			$notifyTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			$notifyTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Password notification on " . $notifyTime->format('Y-m-d H:i'));
			// skip if notification is in the future
			if ($notifyTime > $now) {
				$this->jobResultLog->logDebug($dn . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $dn);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['pwdlastset'][0]) {
				$this->jobResultLog->logDebug($dn . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				$this->jobResultLog->logInfo('Not sending email to ' . $dn . ' because of dry run.');
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
	 * Job to notify users about their managed groups.
	 *
	 * @package jobs
	 */
	class WindowsManagedGroupsNotifyJob extends \LAM\JOB\PasswordExpirationJob {

		const MANAGED_GROUPS = 'LAM_MANAGED_GROUPS';
		const PERIOD_MONTHLY = 'MONTHLY';
		const PERIOD_QUARTERLY = 'QUARTERLY';
		const PERIOD_HALF_YEARLY = 'HALF_YEARLY';
		const PERIOD_YEARLY = 'YEARLY';

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Windows') . ': ' . _('Notify users about their managed groups');
		}

		/**
		 * @inheritDoc
		 */
		public function getDescription() {
			return _('This will send each user a summary of the managed groups and their members.');
		}

		/**
		 * @inheritDoc
		 */
		public function getConfigOptions($jobID) {
			$prefix = $this->getConfigPrefix();
			$container = new htmlResponsiveRow();
			$container->add(new htmlResponsiveInputField(_('From address'), $prefix . '_mailFrom' . $jobID, null, '800', true), 12);
			$container->add(new htmlResponsiveInputField(_('Reply-to address'), $prefix . '_mailReplyTo' . $jobID, null, '801'), 12);
			$container->add(new htmlResponsiveInputField(_('CC address'), $prefix . '_mailCC' . $jobID, null, '805'), 12);
			$container->add(new htmlResponsiveInputField(_('BCC address'), $prefix . '_mailBCC' . $jobID, null, '806'), 12);
			$container->add(new htmlResponsiveInputField(_('Subject'), $prefix . '_mailSubject' . $jobID, null, '802'), 12);
			$container->add(new htmlResponsiveInputCheckbox($prefix . '_mailIsHTML' . $jobID, false, _('HTML format'), '553'), 12);
			$container->add(new htmlResponsiveInputTextarea($prefix . '_mailtext' . $jobID, '', 50, 4, _('Text'), '810'), 12);
			$periodOptions = [
				_('Monthly') => self::PERIOD_MONTHLY,
				_('Quarterly') => self::PERIOD_QUARTERLY,
				_('Half-yearly') => self::PERIOD_HALF_YEARLY,
				_('Yearly') => self::PERIOD_YEARLY,
			];
			$periodSelect = new htmlResponsiveSelect($prefix . '_period' . $jobID, $periodOptions, [], _('Period'), '811');
			$periodSelect->setHasDescriptiveElements(true);
			$periodSelect->setSortElements(false);
			$container->add($periodSelect, 12);
			return $container;
		}

		/**
		 * @inheritDoc
		 */
		public function checkConfigOptions($jobID, $options) {
			$prefix = $this->getConfigPrefix();
			$errors = [];
			// from address
			if (empty($options[$prefix . '_mailFrom' . $jobID][0])
				|| !(get_preg($options[$prefix . '_mailFrom' . $jobID][0], 'email')
					|| get_preg($options[$prefix . '_mailFrom' . $jobID][0], 'emailWithName'))) {
				$errors[] = ['ERROR', _('Please enter a valid email address!'), _('From address')];
			}
			// reply-to
			if (!empty($options[$prefix . '_mailReplyTo' . $jobID][0])
				&& !get_preg($options[$prefix . '_mailReplyTo' . $jobID][0], 'email')
				&& !get_preg($options[$prefix . '_mailReplyTo' . $jobID][0], 'emailWithName')) {
				$errors[] = ['ERROR', _('Please enter a valid email address!'), _('Reply-to address')];
			}
			// CC address
			if (!empty($options[$prefix . '_mailCC' . $jobID][0])
				&& !get_preg($options[$prefix . '_mailCC' . $jobID][0], 'email')
				&& !get_preg($options[$prefix . '_mailCC' . $jobID][0], 'emailWithName')) {
				$errors[] = ['ERROR', _('Please enter a valid email address!'), _('CC address')];
			}
			// BCC address
			if (!empty($options[$prefix . '_mailBCC' . $jobID][0])
				&& !get_preg($options[$prefix . '_mailBCC' . $jobID][0], 'email')
				&& !get_preg($options[$prefix . '_mailBCC' . $jobID][0], 'emailWithName')) {
				$errors[] = ['ERROR', _('Please enter a valid email address!'), _('BCC address')];
			}
			// text
			$mailText = implode('', $options[$prefix . '_mailtext' . $jobID]);
			if (empty($mailText)) {
				$errors[] = ['ERROR', _('Please set a email text.')];
			}
			if (strpos($mailText, '@@' . self::MANAGED_GROUPS . '@@') === false) {
				$errors[] = ['ERROR', _('Please add the wildcard for the list of managed groups.'), '@@' . self::MANAGED_GROUPS . '@@'];
			}
			return $errors;
		}

		/**
		 * @inheritDoc
		 */
		protected function getPolicyOptions() {
			return [];
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
			$sysAttrs = ['managedObjects', $_SESSION['cfgMain']->getMailAttribute()];
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysAttrs)));
			$users = searchLDAPByFilter('(&(' . $_SESSION['cfgMain']->getMailAttribute() . '=*)(managedObjects=*))', $attrs, ['user']);
			$groups = searchLDAPByFilter('(managedBy=*)', ['cn', 'member'], ['group']);
			$groupByDn = [];
			foreach ($groups as $group) {
				$groupByDn[$group['dn']] = $group;
			}
			unset($groups);
			foreach ($users as $index => $user) {
				$managedObjectDns = $user['managedobjects'];
				$managedGroups = [];
				foreach ($managedObjectDns as $managedObjectDn) {
					if (array_key_exists($managedObjectDn, $groupByDn)) {
						$managedGroups[] = $groupByDn[$managedObjectDn];
					}
				}
				$users[$index][strtolower(self::MANAGED_GROUPS)] = $managedGroups;
			}
			return $users;
		}

		/**
		 * @inheritDoc
		 */
		public function execute($jobID, $options, &$pdo, $isDryRun, &$resultLog) {
			$this->jobResultLog = &$resultLog;
			$this->jobResultLog->logDebug("Configuration options:");
			foreach ($options as $key => $value) {
				if (strpos($key, $jobID) === false) {
					continue;
				}
				$this->jobResultLog->logDebug($key . ': ' . implode(', ', $value));
			}
			$now = new DateTime('now', getTimeZone());
			$baseDate = $this->getBaseDate($now);
			$monthInterval = $this->getMonthInterval($options, $jobID);
			if (!$this->shouldRun($pdo, $jobID, $baseDate, $monthInterval)) {
				$this->jobResultLog->logDebug('No run needed yet');
				return;
			}
			$userResults = $this->findUsers($jobID, $options);
			$this->jobResultLog->logDebug("Found " . sizeof($userResults) . " users to send an email.");
			$isHTML = (!empty($options[$this->getConfigPrefix() . '_mailIsHTML' . $jobID][0]) && ($options[$this->getConfigPrefix() . '_mailIsHTML' . $jobID][0] == 'true'));
			foreach ($userResults as $user) {
				if (empty($user[strtolower(self::MANAGED_GROUPS)])) {
					continue;
				}
				$user[strtolower(self::MANAGED_GROUPS)][0] = $this->formatGroups($user[strtolower(self::MANAGED_GROUPS)], $isHTML);
				if ($isDryRun) {
					// no action for dry run
					$this->jobResultLog->logInfo("Managed groups text for " . $user['dn'] . ":\n" . $user[strtolower(self::MANAGED_GROUPS)][0]);
					$this->jobResultLog->logInfo('Not sending email to ' . $user['dn'] . ' because of dry run.');
					continue;
				}
				// send email
				$this->sendMail($options, $jobID, $user, null);
			}
			if (!$isDryRun) {
				$this->setDBLastPwdChangeTime($jobID, $pdo, $jobID, self::getLastEffectiveExecutionDate($baseDate, $monthInterval, $this->jobResultLog)->format('Y-m-d'));
			}
		}

		/**
		 * Returns if the job should run.
		 *
		 * @param $pdo PDO
		 * @param $jobId job id
		 * @param DateTime $baseDate base date
		 * @param int $monthInterval month interval
		 * @return bool should run
		 */
		private function shouldRun(&$pdo, $jobId, $baseDate, $monthInterval) {
			$dbLastChange = $this->getDBLastPwdChangeTime($jobId, $pdo, $jobId);
			if (empty($dbLastChange)) {
				return true;
			}
			$this->jobResultLog->logDebug('Base date: ' . $baseDate->format('Y-m-d'));
			$effectiveDate = self::getLastEffectiveExecutionDate($baseDate, $monthInterval, $this->jobResultLog);
			$dbLastChangeDate = DateTime::createFromFormat('Y-m-d', $dbLastChange, getTimeZone());
			$this->jobResultLog->logDebug('Last run date: ' . $dbLastChangeDate->format('Y-m-d'));
			return $effectiveDate > $dbLastChangeDate;
		}

		/**
		 * Returns the month interval.
		 *
		 * @param array $options config options
		 * @param $jobId job id
		 * @return int interval
		 */
		private function getMonthInterval($options, $jobId) {
			$monthInterval = 12;
			switch ($options[$this->getConfigPrefix() . '_period' . $jobId][0]) {
				case self::PERIOD_HALF_YEARLY:
					$monthInterval = 6;
					break;
				case self::PERIOD_QUARTERLY:
					$monthInterval = 3;
					break;
				case self::PERIOD_MONTHLY:
					$monthInterval = 1;
					break;
			}
			return $monthInterval;
		}

		/**
		 * Returns the base date (first of month) for the current date.
		 *
		 * @param DateTime $currentDate current date
		 * @return DateTime base date
		 */
		private function getBaseDate($currentDate) {
			$baseDateText = $currentDate->format('Y-m-') . '1';
			return DateTime::createFromFormat('Y-m-d', $baseDateText, getTimeZone());
		}

		/**
		 * Returns the last effective execution date.
		 *
		 * @param DateTime $baseDate base date
		 * @param int $monthInterval number of months in interval
		 * @param \LAM\JOB\JobResultLog $resultLog result log
		 */
		public static function getLastEffectiveExecutionDate($baseDate, $monthInterval, $resultLog) {
			$month = $baseDate->format('m');
			$monthIndex = $month - 1;
			while (($monthIndex % $monthInterval) !== 0) {
				$monthIndex--;
			}
			$month = $monthIndex + 1;
			$effectiveDateString = $baseDate->format('Y-') . $month . '-1';
			$effectiveDate = DateTime::createFromFormat('Y-m-d', $effectiveDateString, getTimeZone());
			$resultLog->logDebug("Effective date: " . $effectiveDate->format('Y-m-d'));
			return $effectiveDate;
		}

		/**
		 * @inheritDoc
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			// not used
		}

		/**
		 * Formats the managed groups.
		 *
		 * @param $managedGroups managed groups
		 * @param bool $isHTML HTML email
		 * @return string formatted text
		 */
		private function formatGroups($managedGroups, bool $isHTML) {
			$text = '';
			foreach ($managedGroups as $managedGroup) {
				if ($isHTML) {
					$text .= '<br><b>' . $managedGroup['cn'][0] . '</b><br>';
				}
				else {
					$text .= "\r\n" . $managedGroup['cn'][0] . "\r\n\r\n";
				}
				if (empty($managedGroup['member'])) {
					continue;
				}
				foreach ($managedGroup['member'] as $member) {
					$member = getAbstractDN($member);
					if ($isHTML) {
						$text .= '&nbsp;&nbsp;' . $member . '<br>';
					}
					else {
						$text .= "  " . $member . "\r\n";
					}
				}
			}
			return $text;
		}

	}

	/**
	 * Job to notify users about account expiration.
	 *
	 * @package jobs
	 */
	class WindowsAccountExpirationNotifyJob extends \LAM\JOB\PasswordExpirationJob {

		/**
		 * {@inheritDoc}
		 * @see \LAM\JOB\Job::getAlias()
		 */
		public function getAlias() {
			return _('Windows') . ': ' . _('Notify users about account expiration');
		}

		/**
		 * {@inheritDoc}
		 * @see \LAM\JOB\PasswordExpirationJob::getDescription()
		 */
		public function getDescription() {
			return _('This job sends out emails to inform your users that their account will expire soon.');
		}

		/**
		 * {@inheritDoc}
		 * @see \LAM\JOB\PasswordExpirationJob::findUsers()
		 */
		protected function findUsers($jobID, $options) {
			// read users
			$sysattrs = [$_SESSION['cfgMain']->getMailAttribute(), 'accountexpires', 'useraccountcontrol'];
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			return searchLDAPByFilter('(&(accountexpires=*)(!(accountexpires=0))(' . $_SESSION['cfgMain']->getMailAttribute() . '=*))', $attrs, ['user']);
		}

		/**
		 * {@inheritDoc}
		 * @see \LAM\JOB\PasswordExpirationJob::checkSingleUser()
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			$dn = $user['dn'];
			// skip if account is deactivated
			if (windowsUser::isDeactivated($user)) {
				$this->jobResultLog->logDebug($dn . ' is deactivated.');
				return;
			}
			// skip if account itself is expired
			if (!empty($user['accountexpires'][0])) {
				$accountExpiration = windowsUser::getFileTime($user['accountexpires'][0]);
				if ($accountExpiration <= $now) {
					$this->jobResultLog->logDebug($dn . ' already expired');
					return;
				}
			}
			// get time when account expires
			$expirationTime = windowsUser::getFileTime($user['accountexpires'][0]);
			$this->jobResultLog->logDebug("Account expiration on " . $expirationTime->format('Y-m-d'));
			$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			$this->jobResultLog->logDebug("Number of days before warning " . $numDaysToWarn);
			// calculate time of notification
			$notifyTime = clone $expirationTime;
			$notifyTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			$notifyTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Account expiration notification on " . $notifyTime->format('Y-m-d H:i'));
			// skip if notification is in the future
			if ($notifyTime > $now) {
				$this->jobResultLog->logDebug($dn . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $dn);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['accountexpires'][0]) {
				$this->jobResultLog->logDebug($dn . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				$this->jobResultLog->logInfo('Not sending email to ' . $dn . ' because of dry run.');
				return;
			}
			// send email
			$success = $this->sendMail($options, $jobID, $user, $expirationTime);
			// update DB if mail was sent successfully
			if ($success) {
				$this->setDBLastPwdChangeTime($jobID, $pdo, $dn, $user['accountexpires'][0]);
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
			return _('Windows') . ': ' . _('Cleanup expired user accounts');
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
			$attrs = ['accountexpires'];
			return searchLDAPByFilter('(accountexpires=*)', $attrs, ['user']);
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
			$this->jobResultLog->logDebug("Expiration on " . $expireTime->format('Y-m-d'));
			$delay = 0;
			if (!empty($options[$this->getConfigPrefix() . '_delay' . $jobID][0])) {
				$delay = $options[$this->getConfigPrefix() . '_delay' . $jobID][0];
			}
			$actionTime = clone $expireTime;
			if ($delay != 0) {
				$actionTime->add(new DateInterval('P' . $delay . 'D'));
			}
			$actionTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Action time on " . $actionTime->format('Y-m-d'));
			if ($actionTime <= $now) {
				$this->performAction($jobID, $options, $user, $isDryRun);
			}
		}

	}

}
