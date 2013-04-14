<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
	/** currently locked out, read only flag */
	const AC_LOCKED_OUT = 0x00000010;
	
	/** current group of names list */
	private $groupList = array();
	/** original group of names list */
	private $groupList_orig = array();
	/** cache for groups */
	private $groupCache = null;
	
	
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
		// manages user accounts
		$return["account_types"] = array('user');
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
		$return['objectClasses'] = array('user');
		// managed attributes
		$return['attributes'] = array('cn', 'sAMAccountName', 'description', 'displayName', 'givenName', 'initials',
			'l', 'mail', 'otherTelephone', 'physicalDeliveryOfficeName', 'postalCode', 'postOfficeBox', 'sn', 'st',
			'streetAddress', 'telephoneNumber', 'url', 'wWWHomePage', 'userAccountControl', 'profilePath', 'scriptPath',
			'pwdLastSet');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _('User name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the user\'s name.')
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
			"locked" => array(
				"Headline" => _("Account is locked"),
				"Text" => _("If checked then the account is locked. You may only unlock accounts but not lock them.")),
			"noExpire" => array(
				"Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire.")),
			"requireCard" => array(
				"Headline" => _("Require smartcard"),
				"Text" => _("The user must log on using a smart card.")),
			"profilePath" => array(
				"Headline" => _("Profile path"), 'attr' => 'profilePath',
				"Text" => _('Path of the user profile (UNC-path, e.g. \\\\server\\share\\user). $user is replaced with user name.'). ' '. _("Can be left empty.")),
			"scriptPath" => array(
				"Headline" => _("Logon script"), 'attr' => 'scriptPath',
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon. $user is replaced with user name.'). ' '. _("Can be left empty.")),
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
			);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'windowsUser_name',
				'description' => _('User name'),
				'help' => 'cn',
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
				'example' => _('Temp, contract til December'),
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
				'name' => 'windowsUser_groups',
				'description' => _('Groups'),
				'help' => 'groupsUpload',
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('User name'),
			'description' => _('Description'),
			'displayName' => _('Display name'),
			'givenName' => _('First name'),
			'initials' => _('Initials'),
			'l' => _('Location'),
			'mail' => _('Email address'),
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
		);
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
		$this->messages['cn'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_cn', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['displayName'][0] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_displayName', _('Please enter a valid display name!'));
		$this->messages['givenName'][0] = array('ERROR', _('First name'), _('First name contains invalid characters!'));
		$this->messages['givenName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_givenName', _('First name contains invalid characters!'));
		$this->messages['sn'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters or is empty!'));
		$this->messages['sn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_sn', _('Last name contains invalid characters or is empty!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' windowsUser_telephone', _('Please enter a valid telephone number!'));
		$this->messages['otherTelephone'][0] = array('ERROR', _('Other telephone numbers'), _('Please enter a valid telephone number!'));
		$this->messages['otherTelephone'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherTelephone', _('Please enter a valid telephone number!'));
		$this->messages['postalCode'][0] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		$this->messages['postalCode'][1] = array('ERROR', _('Account %s:') . ' windowsUser_postalCode', _('Please enter a valid postal code!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' windowsUser_mail', _('Please enter a valid email address!'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('ERROR', _('Account %s:') . ' windowsUser_profilePath', _('Profile path is invalid!'));
		$this->messages['scriptPath'][0] = array('ERROR', _('Logon script'), _('Logon script is invalid!'));
		$this->messages['scriptPath'][1] = array('ERROR', _('Account %s:') . ' windowsUser_scriptPath', _('Logon script is invalid!'));
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
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$containerLeft = new htmlTable();
		$containerLeft->alignment = htmlElement::ALIGN_TOP;
		if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['userAccountControl'][0])) {
			$this->attributes['userAccountControl'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
		}
		$containerLeft->addElement(new htmlSubTitle(_('General')), true);
		$this->addSimpleInputTextField($containerLeft, 'cn', _('User name'), true);
		$this->addSimpleInputTextField($containerLeft, 'givenName', _('First name'));
		$this->addSimpleInputTextField($containerLeft, 'sn', _('Last name'));
		$this->addSimpleInputTextField($containerLeft, 'displayName', _('Display name'));
		$this->addSimpleInputTextField($containerLeft, 'initials', _('Initials'));
		$this->addSimpleInputTextField($containerLeft, 'description', _('Description'));

		$containerLeft->addElement(new htmlSubTitle(_('Address')), true);
		$this->addSimpleInputTextField($containerLeft, 'streetAddress', _('Street'), false, 20, true);
		$this->addSimpleInputTextField($containerLeft, 'postOfficeBox', _('Post office box'));
		$this->addSimpleInputTextField($containerLeft, 'postalCode', _('Postal code'));
		$this->addSimpleInputTextField($containerLeft, 'l', _('Location'));
		$this->addSimpleInputTextField($containerLeft, 'st', _('State'));
		$this->addSimpleInputTextField($containerLeft, 'physicalDeliveryOfficeName', _('Office name'));

		$containerLeft->addElement(new htmlSubTitle(_('Contact data')), true);
		$this->addSimpleInputTextField($containerLeft, 'mail', _('Email address'));
		$this->addSimpleInputTextField($containerLeft, 'telephoneNumber', _('Telephone number'));
		$this->addMultiValueInputTextField($containerLeft, 'otherTelephone', _('Other telephone numbers'));
		$this->addSimpleInputTextField($containerLeft, 'wWWHomePage', _('Web site'));
		$this->addMultiValueInputTextField($containerLeft, 'url', _('Other web sites'));
		
		$containerLeft->addElement(new htmlSubTitle(_('Options')), true);
		// locked out
		$containerLeft->addElement(new htmlOutputText(_("Account is locked")));
		$lockedOut = windowsUser::isLockedOut($this->attributes);
		$lockedOutCheckbox = new htmlInputCheckbox('lockedOut', $lockedOut);
		$lockedOutCheckbox->setIsEnabled(false);
		$lockedOutGroup = new htmlGroup();
		$lockedOutGroup->addElement($lockedOutCheckbox);
		if ($lockedOut) {
			$lockedOutGroup->addElement(new htmlButton('unlock', _('Unlock')));
		}
		$containerLeft->addElement($lockedOutGroup);
		$containerLeft->addElement(new htmlHelpLink('locked'), true);
		// password change required
		$pwdMustChange = false;
		if (isset($this->attributes['pwdLastSet'][0]) && ($this->attributes['pwdLastSet'][0] === '0')) {
			$pwdMustChange = true;
		}
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('pwdMustChange', $pwdMustChange, _("User must change password"), 'pwdMustChange'), true);
		// deactivated
		$deactivated = windowsUser::isDeactivated($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('deactivated', $deactivated, _("Account is deactivated"), 'deactivated'), true);
		// password does not expire
		$noExpire = windowsUser::isNeverExpiring($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('noExpire', $noExpire, _("Password does not expire"), 'noExpire'), true);
		// require smartcard
		$requireCard = windowsUser::isSmartCardRequired($this->attributes);
		$containerLeft->addElement(new htmlTableExtendedInputCheckbox('requireCard', $requireCard, _("Require smartcard"), 'requireCard'), true);
		
		$containerLeft->addElement(new htmlSubTitle(_('User profile')), true);
		// profile path
		$this->addSimpleInputTextField($containerLeft, 'profilePath', _('Profile path'));
		// logon script
		$this->addSimpleInputTextField($containerLeft, 'scriptPath', _('Logon script'));
		
		$containerLeft->addElement(new htmlEqualWidth(array('streetAddress', 'cn')));
		
		$containerRight = new htmlTable();
		$containerRight->alignment = htmlElement::ALIGN_TOP;
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
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$return = array();
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		$this->attributes['sAMAccountName'][0] = $_POST['cn'];
		if (!get_preg($_POST['cn'], 'username')) {
			$return[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!empty($this->attributes['displayName'][0]) && !get_preg($_POST['displayName'], 'realname')) {
			$return[] = $this->messages['displayName'][0];
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
		if (!empty($this->attributes['mail'][0]) && !get_preg($_POST['mail'], 'email')) {
			$return[] = $this->messages['mail'][0];
		}
		// other telephones
		$this->processMultiValueInputTextField('otherTelephone', $return, 'telephone');
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
		if (isset($_POST['pwdMustChange']) && ($_POST['pwdMustChange'] == 'on')) {
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
		// deactivated
		$deactivated = isset($_POST['deactivated']) && ($_POST['deactivated'] == 'on');
		windowsUser::setIsDeactivated($this->attributes, $deactivated);
		// no expire
		$noExpire = isset($_POST['noExpire']) && ($_POST['noExpire'] == 'on');
		windowsUser::setIsNeverExpiring($this->attributes, $noExpire);
		// smartcard required
		$requireCard = isset($_POST['requireCard']) && ($_POST['requireCard'] == 'on');
		windowsUser::setIsSmartCardRequired($this->attributes, $requireCard);
		// profile path
		$this->attributes['profilePath'][0] = $_POST['profilePath'];
		$this->attributes['profilePath'][0] = str_replace('$user', $this->attributes['cn'][0], $this->attributes['profilePath'][0]);
		if (!($this->attributes['profilePath'][0] == '') && !get_preg($this->attributes['profilePath'][0], 'UNC')) {
			$return[] = $this->messages['profilePath'][0];
		}
		// logon script
		$this->attributes['scriptPath'][0] = $_POST['scriptPath'];
		$this->attributes['scriptPath'][0] = str_replace('$user', $this->attributes['cn'][0], $this->attributes['scriptPath'][0]);
		if (($this->attributes['scriptPath'][0] != '') && (!get_preg($this->attributes['scriptPath'][0], 'logonscript'))) {
			$return[] = $this->messages['scriptPath'][0];
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
		$groupContainer->addElement(new htmlSubTitle(_("Groups of names")), true);
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
		$buttonGroupContainer->addElement(new htmlHelpLink('addgroup'));
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
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toAdd[$i]), ldap_error($_SESSION['ldap']->server()));
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
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), ldap_error($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i]);
				}
			}
		}
		return $messages;
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
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
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
			// cn + sAMAccountName
			if ($rawAccounts[$i][$ids['windowsUser_name']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_name']], 'username')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsUser_name']];
					$partialAccounts[$i]['sAMAccountName'] = $rawAccounts[$i][$ids['windowsUser_name']];
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// password
			if (($rawAccounts[$i][$ids['windowsUser_password']] != "") && (get_preg($rawAccounts[$i][$ids['windowsUser_password']], 'password'))) {
				$partialAccounts[$i]['unicodePwd'] = $this->pwdAttributeValue($rawAccounts[$i][$ids['windowsUser_password']]);
				$partialAccounts[$i]['INFO.userPasswordClearText'] = $rawAccounts[$i][$ids['windowsUser_password']]; // for custom scripts etc.
			}
			elseif ($rawAccounts[$i][$ids['posixAccount_password']] != "") {
				$errMsg = $this->messages['userPassword'][4];
				$errMsg[2] = str_replace('%', '%%', $errMsg[2]); // double "%" because of later sprintf
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
			// display name
			if ($rawAccounts[$i][$ids['windowsUser_displayName']] != "") {
				$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['windowsUser_displayName']];
			}
			// initials
			if ($rawAccounts[$i][$ids['windowsUser_initials']] != "") {
				$partialAccounts[$i]['initials'] = $rawAccounts[$i][$ids['windowsUser_initials']];
			}
			// description
			if ($rawAccounts[$i][$ids['windowsUser_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['windowsUser_description']];
			}
			// street
			if ($rawAccounts[$i][$ids['windowsUser_streetAddress']] != "") {
				$partialAccounts[$i]['streetAddress'] = $rawAccounts[$i][$ids['windowsUser_streetAddress']];
			}
			// post office box
			if ($rawAccounts[$i][$ids['windowsUser_postOfficeBox']] != "") {
				$partialAccounts[$i]['postOfficeBox'] = $rawAccounts[$i][$ids['windowsUser_postOfficeBox']];
			}
			// postal code
			if ($rawAccounts[$i][$ids['windowsUser_postalCode']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_postalCode']], 'postalCode')) {
					$partialAccounts[$i]['postalCode'] = $rawAccounts[$i][$ids['windowsUser_postalCode']];
				}
				else {
					$errMsg = $this->messages['postalCode'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// location
			if ($rawAccounts[$i][$ids['windowsUser_l']] != "") {
				$partialAccounts[$i]['l'] = $rawAccounts[$i][$ids['windowsUser_l']];
			}
			// state
			if ($rawAccounts[$i][$ids['windowsUser_state']] != "") {
				$partialAccounts[$i]['st'] = $rawAccounts[$i][$ids['windowsUser_state']];
			}
			// office name
			if ($rawAccounts[$i][$ids['windowsUser_officeName']] != "") {
				$partialAccounts[$i]['physicalDeliveryOfficeName'] = $rawAccounts[$i][$ids['windowsUser_officeName']];
			}
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
			// telephone
			if ($rawAccounts[$i][$ids['windowsUser_telephoneNumber']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsUser_telephoneNumber']], 'telephone')) {
					$partialAccounts[$i]['telephoneNumber'] = $rawAccounts[$i][$ids['windowsUser_telephoneNumber']];
				}
				else {
					$errMsg = $this->messages['telephoneNumber'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// other telephone
			if (isset($ids['windowsUser_otherTelephone']) && ($rawAccounts[$i][$ids['windowsUser_otherTelephone']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_otherTelephone']]);
				$partialAccounts[$i]['otherTelephone'] = $valueList;
				for ($x = 0; $x < sizeof($valueList); $x++) {
					if (!get_preg($valueList[$x], 'telephone')) {
						$errMsg = $this->messages['otherTelephone'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// website
			if ($rawAccounts[$i][$ids['windowsUser_webSite']] != "") {
				$partialAccounts[$i]['wWWHomePage'] = $rawAccounts[$i][$ids['windowsUser_webSite']];
			}
			// other websites
			if (isset($ids['windowsUser_otherWebSites']) && ($rawAccounts[$i][$ids['windowsUser_otherWebSites']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsUser_otherWebSites']]);
				$partialAccounts[$i]['url'] = $valueList;
			}
			// user account control
			$userAccountControlAttr['userAccountControl'][0] = windowsUser::DEFAULT_ACCOUNT_CONTROL;
			// deactivated
			if ($rawAccounts[$i][$ids['windowsUser_deactivated']] != "") {
				if (!isset($booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]])) {
					$errors[] = array('ERROR', sprintf(_('Account %s:'), $i) . ' windowsUser_deactivated', _('Please enter either yes or no.'));
				}
				else {
					$this->setIsDeactivated($userAccountControlAttr, $booleanOptions[$rawAccounts[$i][$ids['windowsUser_deactivated']]]);
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
		}
		return $errors;
	}
	
	/**
	* This function executes one post upload action.
	*
	* @param array $data array containing one account in each element
	* @param array $ids array(<column_name> => <column number>)
	* @param array $failed list of accounts which were not created successfully
	* @param array $temp variable to store temporary data between two post actions
	* @param array $accounts list of LDAP entries
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress'
	* <br>  'progress' => 0..100
	* <br>  'errors' => array (<array of parameters for StatusMessage>)
	* <br> )
	*/
	function doUploadPostActions(&$data, $ids, $failed, &$temp, &$accounts) {
		if (!checkIfWriteAccessIsAllowed()) {
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
					ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
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
					ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
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
	* Returns a list of PDF entries
	*/
	public function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('User name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		$this->addSimplePDFField($return, 'givenName', _('First name'));
		$this->addSimplePDFField($return, 'initials', _('Initials'));
		$this->addSimplePDFField($return, 'l', _('Location'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'otherTelephone', _('Other telephone numbers'));
		$this->addSimplePDFField($return, 'physicalDeliveryOfficeName', _('Office name'));
		$this->addSimplePDFField($return, 'postalCode', _('Postal code'));
		$this->addSimplePDFField($return, 'postOfficeBox', _('Post office box'));
		$this->addSimplePDFField($return, 'sn', _('Last name'));
		$this->addSimplePDFField($return, 'st', _('State'));
		$this->addSimplePDFField($return, 'streetAddress', _('Street'));
		$this->addSimplePDFField($return, 'telephoneNumber', _('Telephone number'));
		$this->addSimplePDFField($return, 'url', _('Other web sites'));
		$this->addSimplePDFField($return, 'wWWHomePage', _('Web site'));
		$deactivated = _('no');
		if ($this->isDeactivated($this->attributes)) {
			$deactivated = _('yes');
		}
		$return[get_class($this) . '_deactivated'] = array('<block><key>' . _('Account is deactivated') . '</key><value>' . $deactivated . '</value></block>');
		$noExpire = _('no');
		if ($this->isNeverExpiring($this->attributes)) {
			$noExpire = _('yes');
		}
		$return[get_class($this) . '_noExpire'] = array('<block><key>' . _('Password does not expire') . '</key><value>' . $noExpire . '</value></block>');
		$requireCard = _('no');
		if ($this->isSmartCardRequired($this->attributes)) {
			$requireCard = _('yes');
		}
		$return[get_class($this) . '_requireCard'] = array('<block><key>' . _('Require smartcard') . '</key><value>' . $requireCard . '</value></block>');
		$pwdMustChange = _('no');
		if (isset($this->attributes['pwdLastSet'][0]) && ($this->attributes['pwdLastSet'][0] === '0')) {
			$pwdMustChange = _('yes');
		}
		$return[get_class($this) . '_pwdMustChange'] = array('<block><key>' . _('Password change at next login') . '</key><value>' . $pwdMustChange . '</value></block>');
		$this->addSimplePDFField($return, 'profilePath', _('Profile path'));
		$this->addSimplePDFField($return, 'scriptPath', _('Logon script'));
		$groups = array();
		foreach ($this->groupList as $group) {
			$groups[] = extractRDNValue($group);
		}
		$return[get_class($this) . '_groups'] = array('<block><key>' . _('Groups') . '</key><value>' . implode(', ', $groups) . '</value></block>');
		return $return;
	}
	
	/**
	* Returns a list of elements for the account profiles.
	*
	* @return htmlElement profile elements
	*/
	function get_profileOptions() {
		$return = new htmlTable();
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
		// load groups
		if (isset($profile['windowsUser_groups'][0])) {
			$this->groupList = $profile['windowsUser_groups'];
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
		$pwdBin = $this->pwdAttributeValue($password);
		$this->orig['unicodePwd'][0] = 'unknown';
		$this->attributes['unicodePwd'][0] = $pwdBin;
		$this->attributes['pwdLastSet'][0] = '-1';
		return array();
	}
	
	/**
	 * Creates the LDAP password value.
	 * 
	 * @param String $password password
	 */
	public function pwdAttributeValue($password) {
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
	 * Sets if the account is currently deactivated.
	 * 
	 * @param array $attrs LDAP attributes to modify
	 * @param boolean $deactivated is deactivated
	 */
	public static function setIsDeactivated(&$attrs, $deactivated) {
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
	 * Returns if the account is currently locked out.
	 * 
	 * @param array $attrs LDAP attributes 
	 * @return boolean is locked out
	 */
	private static function isLockedOut($attrs) {
		$myAttrs = array_change_key_case($attrs, CASE_LOWER);
		if (!isset($attrs['useraccountcontrol'][0])) {
			return false;
		}
		return intval($attrs['useraccountcontrol'][0]) & windowsUser::AC_LOCKED_OUT;
	}

	/**
	 * Unlocks the account.
	 * 
	 * @param array $attrs LDAP attributes to modify
	 */
	public static function unlock(&$attrs) {
		foreach ($attrs as $key => $value) {
			if (strtolower($key) == 'useraccountcontrol') {
				if (intval($attrs[$key][0]) & windowsUser::AC_LOCKED_OUT) {
					$attrs[$key][0] = intval($attrs[$key][0]) - windowsUser::AC_LOCKED_OUT;
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
	
}
	
?>
