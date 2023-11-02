<?php

use LAM\TYPES\ConfiguredType;
use function LAM\ACCOUNTLIST\isPasswordChangeByDefault;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2023  Roland Gruber

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
* Manages Samba 3 accounts for users and hosts.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/** terminals server options */

include_once('sambaSamAccount/sambaMungedDial.inc');

/**
* Manages the object class "sambaSamAccount" for users and hosts.
*
* @package modules
*/
class sambaSamAccount extends baseModule implements passwordService,AccountStatusProvider {

	/**
	 * ID for locked account status.
	 */
	const STATUS_ACCOUNT_LOCKED = "SAMBA_ACCOUNT_LOCKED";

	/**
	 * ID for locked account status.
	 */
	const STATUS_ACCOUNT_DEACTIVATED = "SAMBA_ACCOUNT_DEACTIVATED";

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	const ATTRIBUTES_TO_IGNORE_ON_COPY = array('sambaSID', 'sambaLMPassword', 'sambaNTPassword',
		'sambaPwdLastSet', 'displayName', 'sambaHomePath', 'sambaProfilePath',
		'sambaPasswordHistory', 'sambaPwdCanChange', 'sambaPwdMustChange', 'sambaKickoffTime',
		'sambaLogonTime', 'sambaLogoffTime');

	// Variables
	/** use no password? */
	private $nopwd;
	/** password does not expire? */
	private $noexpire;
	/** array of well known group rids */
	private $groupRids;
	/** array of well known user rids */
	private $userRids;
	/** HEX to binary conversion table */
	private $hex2bitstring = array('0' => '0000', '1' => '0001', '2' => '0010', '3' => '0011', '4' => '0100',
		'5' => '0101', '6' => '0110', '7' => '0111', '8' => '1000', '9' => '1001', 'A' => '1010',
		'B' => '1011', 'C' => '1100', 'D' => '1101', 'E' => '1110', 'F' => '1111');
	/** specifies if the password should be expired */
	private $expirePassword = false;
	/** host cache to reduce LDAP queries */
	private $cachedHostList;
	/** group cache to reduce LDAP queries */
	private $cachedGroupSIDList;
	/** cache for domain list */
	private $cachedDomainList;
	/** delimiter for remote commands */
	private static $SPLIT_DELIMITER = "###x##y##x###";


	/**
	* Creates a new sambaSamAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// List of well known group rids
		$this->groupRids = array(
			_('Domain admins') => 512, _('Domain users') => 513, _('Domain guests') => 514,
			_('Domain computers') => 515, _('Domain controllers') => 516, _('Domain certificate admins') => 517,
			_('Domain schema admins') => 518, _('Domain enterprise admins') => 519, _('Domain policy admins') => 520);
		// List of well known user rids
		$this->userRids = array(
			_('Domain admins') => 500, _('Domain guests') => 501, _('Domain KRBTGT') => 502);
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/** this function fills the error message array with messages
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['homePath'][0] = array('ERROR', _('Home path'), _('Home path is invalid.'));
		$this->messages['homePath'][1] = array('INFO', _('Home path'), _('Inserted user or group name in home path.'));
		$this->messages['homePath'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_homePath', _('Home path is invalid.'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('INFO', _('Profile path'), _('Inserted user or group name in profile path.'));
		$this->messages['profilePath'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_profilePath', _('Profile path is invalid!'));
		$this->messages['logonScript'][0] = array('ERROR', _('Logon script'), _('Logon script is invalid!'));
		$this->messages['logonScript'][1] = array('INFO', _('Logon script'), _('Inserted user or group name in logon script.'));
		$this->messages['logonScript'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_logonScript', _('Logon script is invalid!'));
		$this->messages['workstations'][0] = array('ERROR', _('Samba workstations'), _('Please enter a comma separated list of host names!'));
		$this->messages['workstations'][1] = array('ERROR', _('Account %s:') . ' sambaSamAccount_workstations', _('Please enter a comma separated list of host names!'));
		$this->messages['sambaLMPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password fields.'));
		$this->messages['sambaLMPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['sambaLMPassword'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_password', _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['rid'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('Please enter a RID number or the name of a special account!'));
		$this->messages['rid'][3] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('This is not a valid RID number!'));
		$this->messages['displayName'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_displayName', _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['pwdUnix'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdUnix', _('This value can only be "true" or "false".'));
		$this->messages['noPassword'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noPassword', _('This value can only be "true" or "false".'));
		$this->messages['noExpire'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noExpire', _('This value can only be "true" or "false".'));
		$this->messages['deactivated'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_deactivated', _('This value can only be "true" or "false".'));
		$this->messages['expireDate'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_expireDate', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['homeDrive'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_homeDrive', _('Please enter a valid drive letter.'));
		$this->messages['domain'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_domain', _('LAM was unable to find a domain with this name!'));
		$this->messages['logonHours'][0] = array('ERROR', _('Logon hours'), _('The format of the logon hours field is invalid!'));
		$this->messages['logonHours'][1] = array('ERROR', _('Account %s:') . ' sambaSamAccount_logonHours', _('The format of the logon hours field is invalid!'));
		$this->messages['group'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_group', _('Please enter a valid group name!'));
		$this->messages['profileCanMustChange'][0] = array('ERROR', _('The value for the Samba 3 field "User can/must change password" needs to be a number.'));
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user', 'host'));
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
		$return['icon'] = 'samba.svg';
		// alias name
		$return["alias"] = _('Samba 3');
		// RDN attribute
		$return["RDN"] = array("sambaSID" => "low");
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=sambaSamAccount)");
		// managed object classes
		$return['objectClasses'] = array('sambaSamAccount');
		// managed attributes
		$return['attributes'] = array('uid', 'sambaSID', 'sambaLMPassword', 'sambaNTPassword', 'sambaPwdLastSet',
			'sambaLogonTime', 'sambaLogoffTime', 'sambaKickoffTime', 'sambaacctflags',
			'sambaPwdLastSet', 'displayName', 'sambaHomePath', 'sambaHomeDrive', 'sambaLogonScript', 'sambaProfilePath',
			'sambaUserWorkstations', 'sambaPrimaryGroupSID', 'sambaDomainName', 'sambaLogonHours', 'sambaMungedDial',
			'sambaPasswordHistory', 'sambaPwdCanChange', 'sambaPwdMustChange'); // sambaPwdCanChange/sambaPwdMustChange only for extension removal
		// PHP extensions
		$return['extensions'] = array('hash', 'iconv');
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'), 12);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['sambaSamAccount_smbhome'] = array(
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['homePath'][0]);
		$return['profile_checks']['sambaSamAccount_profilePath'] = array(
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['profilePath'][0]);
		$return['profile_checks']['sambaSamAccount_logonScript'] = array(
			'type' => 'ext_preg',
			'regex' => 'logonscript',
			'error_message' => $this->messages['logonScript'][0]);
		$return['profile_checks']['sambaSamAccount_userWorkstations'] = array(
			'type' => 'ext_preg',
			'regex' => 'unixhost',
			'error_message' => $this->messages['workstations'][0]);
		$return['profile_checks']['sambaSamAccount_logonHours'] = array(
			'type' => 'ext_preg',
			'regex' => 'sambaLogonHours',
			'error_message' => $this->messages['logonHours'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'sambaSamAccount_sambaDomainName' => 'sambaDomainName',
			'sambaSamAccount_displayName' => 'displayName',
		);
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
			$return['profile_mappings']['sambaSamAccount_smbhome'] = 'sambaHomePath';
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
			$return['profile_mappings']['sambaSamAccount_profilePath'] = 'sambaProfilePath';
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
			$return['profile_mappings']['sambaSamAccount_logonScript'] = 'sambaLogonScript';
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
			$return['profile_mappings']['sambaSamAccount_userWorkstations'] = 'sambaUserWorkstations';
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
			$return['profile_mappings']['sambaSamAccount_logonHours'] = 'sambaLogonHours';
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'displayName' => _('Display name'),
			'sambaKickoffTime' => _('Account expiration date'),
			'sambaDomainName' => _('Domain'),
			'sambaPrimaryGroupSID' => _('Windows group')
		);
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
			$return['PDF_fields']['sambaHomeDrive'] = _('Home drive');
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
			$return['PDF_fields']['sambaHomePath'] = _('Home path');
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
			$return['PDF_fields']['sambaProfilePath'] = _('Profile path');
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
			$return['PDF_fields']['sambaLogonScript'] = _('Logon script');
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
			$return['PDF_fields']['sambaUserWorkstations'] = _('Samba workstations');
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideTerminalServer')) {
			$return['PDF_fields']['tsAllowLogin'] = _('Allow terminal server login');
			$return['PDF_fields']['tsHomeDir'] = _('Home directory') . ' (TS)';
			$return['PDF_fields']['tsHomeDrive'] = _('Home drive') . ' (TS)';
			$return['PDF_fields']['tsProfilePath'] = _('Profile path') . ' (TS)';
			$return['PDF_fields']['tsInherit'] = _('Inherit client startup configuration') . ' (TS)';
			$return['PDF_fields']['tsInitialProgram'] = _('Initial program') . ' (TS)';
			$return['PDF_fields']['tsWorkDirectory'] = _('Working directory') . ' (TS)';
			$return['PDF_fields']['tsConnectionLimit'] = _('Connection time limit') . ' (TS)';
			$return['PDF_fields']['tsDisconnectionLimit'] = _('Disconnection time limit') . ' (TS)';
			$return['PDF_fields']['tsIdleLimit'] = _('Idle time limit') . ' (TS)';
			$return['PDF_fields']['tsConnectDrives'] = _('Connect client drives') . ' (TS)';
			$return['PDF_fields']['tsConnectPrinters'] = _('Connect client printers') . ' (TS)';
			$return['PDF_fields']['tsClientPrinterDefault'] = _('Client printer is default') . ' (TS)';
			$return['PDF_fields']['tsShadowing'] = _('Shadowing') . ' (TS)';
			$return['PDF_fields']['tsBrokenConn'] = _('On broken or timed out connection') . ' (TS)';
			$return['PDF_fields']['tsReconnect'] = _('Reconnect if disconnected') . ' (TS)';
		}
		$return['selfServiceFieldSettings'] = array(
			'syncNTPassword' => _('Sync Samba NT password with Unix password'),
			'syncLMPassword' => _('Sync Samba LM password with Unix password'),
			'syncSambaPwdLastSet' => _('Update attribute "sambaPwdLastSet" on password change'),
			'password' => _('Password'),
			'sambaPwdLastSet' => _('Last password change (read-only)'),
		);
		// self service: fields that cannot be relabeled
		$return['selfServiceNoRelabelFields'] = array('syncNTPassword', 'syncLMPassword', 'syncSambaPwdLastSet');
		// help Entries
		$return['help'] = array (
			"displayName" => array(
				"Headline" => _("Display name"), 'attr' => 'displayName',
				"Text" => _("This is the account's full name on Windows systems.")),
			"password" => array(
				"Headline" => _("Samba password"),
				"Text" => _("This is the account's Windows password.")),
			"resetPassword" => array(
				"Headline" => _("Reset password"),
				"Text" => _("This will reset the host's password to a default value.")),
			"pwdUnix" => array(
				"Headline" => _("Use Unix password"),
				"Text" => _("If checked Unix password will also be used as Samba password.")),
			"pwdUnixUpload" => array(
				"Headline" => _("Use Unix password"),
				"Text" => _("If set to \"true\" Unix password will also be used as Samba password.")),
			"noPassword" => array(
				"Headline" => _("Use no password"),
				"Text" => _("If checked no password will be used.")),
			"noPasswordUpload" => array(
				"Headline" => _("Use no password"),
				"Text" => _("If set to \"true\" no password will be used.")),
			"noExpire" => array(
				"Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire. (Setting X-Flag)")),
			"noExpireUpload" => array(
				"Headline" => _("Password does not expire"),
				"Text" => _("If set to \"true\" password does not expire. (Setting X-Flag)")),
			"deactivated" => array(
				"Headline" => _("Account is deactivated"),
				"Text" => _("If checked then the account will be deactivated. (Setting D-Flag)")),
			"locked" => array(
				"Headline" => _("Account is locked"),
				"Text" => _("If checked then the account will be locked (setting L-Flag). You usually want to use this setting to unlock user accounts which were locked because of failed login attempts.")),
			"deactivatedUpload" => array(
				"Headline" => _("Account is deactivated"),
				"Text" => _("If set to \"true\" account will be deactivated. (Setting D-Flag)")),
			"passwordIsExpired" => array(
				"Headline" => _("Password change at next login"),
				"Text" => _("If you set this option then the user has to change his password at the next login.")),
			"pwdCanChange" => array(
				"Headline" => _("User can change password"),
				"Text" => _("Date after the user is able to change his password.")),
			"pwdMustChange" => array ("Headline" => _("User must change password"),
				"Text" => _("Date after the user must change his password.")),
			"homeDrive" => array(
				"Headline" => _("Home drive"), 'attr' => 'sambaHomeDrive',
				"Text" => _("The home directory will be connected under this drive letter.")),
			"homePath" => array(
				"Headline" => _("Home path"), 'attr' => 'sambaHomePath',
				"Text" => _('UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"profilePath" => array(
				"Headline" => _("Profile path"), 'attr' => 'sambaProfilePath',
				"Text" => _('Path of the user profile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"scriptPath" => array(
				"Headline" => _("Logon script"), 'attr' => 'sambaLogonScript',
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"userWorkstations" => array(
				"Headline" => _("Samba workstations"), 'attr' => 'sambaUserWorkstations',
				"Text" => _("List of Samba workstations the user is allowed to login. Empty means every workstation.")),
			"workstations" => array(
				"Headline" => _("Samba workstations"), 'attr' => 'sambaUserWorkstations',
				"Text" => _("Comma separated list of Samba workstations the user is allowed to login. Empty means every workstation."). ' '. _("Can be left empty.")),
			"group" => array(
				"Headline" => _("Windows primary group"), 'attr' => 'sambaPrimaryGroupSID',
				"Text" => _("This is the user's primary Windows group.")),
			"groupUpload" => array(
				"Headline" => _("Windows primary group SID"), 'attr' => 'sambaPrimaryGroupSID',
				"Text" => _("This is the SID of the user's primary Windows group.")),
			"specialUser" => array(
				"Headline" => _("Special user"),
				"Text" => _("This allows you to define this account as a special user like administrator or guest.")),
			"ridUpload" => array(
				"Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your Windows account. You can either enter a number or one of these special accounts: ") .
				implode(", ", array_keys($this->userRids)) . "<br><br>" . _("If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")),
			"ridUploadHost" => array(
				"Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your host account. If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")),
			"domain" => array(
				"Headline" => _("Domain"), 'attr' => 'sambaDomainName',
				"Text" => _("Windows domain name of account.")),
			"logonHours" => array(
				"Headline" => _("Logon hours"), 'attr' => 'sambaLogonHours',
				"Text" => _("This option defines the allowed logon hours for this account.")),
			"logonHoursUpload" => array(
				"Headline" => _("Logon hours"), 'attr' => 'sambaLogonHours',
				"Text" => _("This option defines the allowed logon hours for this account. The format is the same as for the LDAP attribute. The 24*7 hours are represented as 168 bit which are saved as 21 hex (21*8 = 168) values. The first bit represents Sunday 0:00 - 0:59 in GMT.")),
			'expireDate' => array (
				"Headline" => _("Account expiration date"), 'attr' => 'sambaKickoffTime',
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")),
			'timeZone' => array (
				"Headline" => _("Time zone"),
				"Text" => _("This is the time zone of your Samba server. LAM needs this information to display the logon hours correctly.")),
			'tsAllowLogin' => array (
				"Headline" => _("Allow terminal server login"),
				"Text" => _("Activate this checkbox to allow this user to use the terminal service.")),
			'tsHomeDir' => array (
				"Headline" => _("Home directory"),
				"Text" => _("This is the path to the user's home directory.")),
			'tsProfilePath' => array (
				"Headline" => _("Profile path"),
				"Text" => _("Path of the user profile.")),
			'tsInherit' => array (
				"Headline" => _("Inherit client startup configuration"),
				"Text" => _("Activate this checkbox to inherit the initial program and working directory from the client machine.")),
			'tsInitialProgram' => array (
				"Headline" => _("Initial program"),
				"Text" => _("This program is run after the login.")),
			'tsWorkDirectory' => array (
				"Headline" => _("Working directory"),
				"Text" => _("Working directory of initial program.")),
			'tsTimeLimit' => array (
				"Headline" => _("Time limit"),
				"Text" => _("Please enter the time limit in minutes. 0 means unlimited.")),
			'tsConnectDrives' => array (
				"Headline" => _("Connect client drives"),
				"Text" => _("Activate this checkbox to connect drives from the client machine.")),
			'tsConnectPrinters' => array (
				"Headline" => _("Connect client printers"),
				"Text" => _("Activate this checkbox to connect printers from the client machine.")),
			'tsClientPrinterDefault' => array (
				"Headline" => _("Client printer is default"),
				"Text" => _("Activate this checkbox to set the client's printer as default printer.")),
			'tsShadowing' => array (
				"Headline" => _("Shadowing"),
				"Text" => _("Here you can specify the shadowing mode.")),
			'tsBrokenConn' => array (
				"Headline" => _("On broken or timed out connection"),
				"Text" => _("This specifies what to do when the client connection is broken.")),
			'tsReconnect' => array (
				"Headline" => _("Reconnect if disconnected"),
				"Text" => _("This specifies the reconnect policy.")),
			'terminalServer' => array (
				"Headline" => _("Terminal server options"),
				"Text" => _("Here you can change the settings for the terminal server access.")),
			'lmHash' => array (
				"Headline" => _("Disable LM hashes"),
				"Text" => _("Windows password hashes are saved by default as NT and LM hashes. LM hashes are insecure and only needed for old versions of Windows. You should disable them unless you really need them.")),
			'sambaPwdLastSet' => array (
				"Headline" => _("Last password change"), 'attr' => 'sambaPwdLastSet',
				"Text" => _("This is the date when the user changed his password.")),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")),
			'domainSuffix' => array(
				"Headline" => _("Domain suffix"),
				"Text" => _("Please enter the LDAP suffix where your Samba domain entries are stored.")),
			'history' => array(
				"Headline" => _("Password history"),
				"Text" => _("Enables password history. Depending on your LDAP server you need to select the right server-side ordering (switch ordering here if old passwords are not removed from history).")),
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount', 'inetOrgPerson');
		// upload options
		if ($this->get_scope() == "user") {
			$return['upload_columns'] = array(
				array(
					'name' => 'sambaSamAccount_domain',
					'description' => _('Domain'),
					'required' => true,
					'help' => 'domain',
					'example' => _('mydomain')
				),
				array(
					'name' => 'sambaSamAccount_displayName',
					'description' => _('Display name'),
					'help' => 'displayName',
					'example' => _('Steve Miller')
				),
				array(
					'name' => 'sambaSamAccount_password',
					'description' => _('Password'),
					'help' => 'password',
					'example' => _('secret')
				),
				array(
					'name' => 'sambaSamAccount_pwdUnix',
					'description' => _('Use Unix password'),
					'help' => 'pwdUnixUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				),
				array(
					'name' => 'sambaSamAccount_noPassword',
					'description' => _('Use no password'),
					'help' => 'noPasswordUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				),
				array(
					'name' => 'sambaSamAccount_noExpire',
					'description' => _('Password does not expire'),
					'help' => 'noExpireUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				),
				array(
					'name' => 'sambaSamAccount_deactivated',
					'description' => _('Account is deactivated'),
					'help' => 'deactivatedUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				),
				array(
					'name' => 'sambaSamAccount_expireDate',
					'description' => _('Account expiration date'),
					'help' => 'expireDate',
					'default' => '31-12-2030',
					'example' => '15-10-2020'
				),
				array(
					'name' => 'sambaSamAccount_group',
					'description' => _('Windows group'),
					'help' => 'groupUpload',
					'example' => _('mygroup'),
					'default' => 'Domain Users'
				),
				array(
					'name' => 'sambaSamAccount_rid',
					'description' => _('Samba RID'),
					'help' => 'ridUpload',
					'example' => '1235',
					'default' => '{uidNumber}*2 + {sambaAlgorithmicRidBase}'
				),
			);
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_homeDrive',
					'description' => _('Home drive'),
					'help' => 'homeDrive',
					'example' => 'k:'
				);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_homePath',
					'description' => _('Home path'),
					'help' => 'homePath',
					'example' => _('\\\\server\\homes\\smiller')
				);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_profilePath',
					'description' => _('Profile path'),
					'help' => 'profilePath',
					'example' => _('\\\\server\\profiles\\smiller')
				);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_logonScript',
					'description' => _('Logon script'),
					'help' => 'scriptPath',
					'example' => 'logon.bat'
				);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_workstations',
					'description' => _('Samba workstations'),
					'help' => 'workstations',
					'example' => 'PC01,PC02,PC03'
				);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
				$return['upload_columns'][] = array(
					'name' => 'sambaSamAccount_logonHours',
					'description' => _('Logon hours'),
					'help' => 'logonHoursUpload',
					'example' => 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF'
				);
			}
		}
		elseif ($this->get_scope() == "host") {
			$return['upload_columns'] = array(
				array(
					'name' => 'sambaSamAccount_domain',
					'description' => _('Domain'),
					'required' => true,
					'help' => 'domain',
					'example' => _('mydomain')
				),
				array(
					'name' => 'sambaSamAccount_rid',
					'description' => _('Samba RID'),
					'help' => 'ridUploadHost',
					'example' => '1235',
					'default' => '{uidNumber}*2 + {sambaAlgorithmicRidBase}'
				)
			);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = array()) : void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	* Initializes the module after it became part of an accountContainer
	*
	* @param string $base the name of the accountContainer object ($_SESSION[$base])
	*/
	function init($base) {
		// call parent init
		parent::init($base);
		$this->noexpire = true;
		$this->nopwd = false;
	}

	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		return isset($attrs['gidNumber'][0]) && isset($attrs['uidNumber'][0]) && isset($attrs['uid'][0]);
	}

	/**
	* This function is used to check if all settings for this module have been made.
	*
	* @see baseModule::module_complete
	*
	* @return boolean true, if settings are complete
	*/
	public function module_complete() {
		if (!$this->isExtensionEnabled()) {
			return true;
		}
		if ($this->get_scope() == "host") {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (substr($attrs['uid'][0], -1, 1) != '$') {
				return false;
			}
		}
		if (!isset($this->attributes['sambaSID']) || ($this->attributes['sambaSID'] == '')) {
			return false;
		}
		return true;
	}


	/**
	* This function loads the LDAP attributes for this module.
	*
	* @param array $attr attribute list
	*/
	function load_attributes($attr) {
		parent::load_attributes($attr);
		if (isset($this->attributes['sambaacctflags'][0])) {
			if (strpos($this->attributes['sambaacctflags'][0], "N")) $this->nopwd = true;
			else $this->nopwd = false;
			if (strpos($this->attributes['sambaacctflags'][0], "X")) $this->noexpire = true;
			else $this->noexpire = false;
		}
		if (isset($this->attributes['sambaPwdLastSet'][0]) && ($this->attributes['sambaPwdLastSet'][0] === '0')) {
			$this->expirePassword = true;
		}
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
		if (!in_array('sambaSamAccount', $this->attributes['objectClass']) && !in_array('sambaSamAccount', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		if ($this->isExtensionEnabled()) {
			if ($this->expirePassword === true) {
				$this->attributes['sambaPwdLastSet'][0] = '0';
			}
			elseif ((isset($this->attributes['sambaPwdLastSet'][0])) && ($this->attributes['sambaPwdLastSet'][0] == '0')) {
				$this->attributes['sambaPwdLastSet'][0] = time();
			}
		}
		return parent::save_attributes();
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		// add extension
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'sambaSamAccount';
			return array();
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('sambaSamAccount'), $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < sizeof($attrKeys); $k++) {
				if (strpos($attrKeys[$k], 'samba') > -1) {
					unset($this->attributes[$attrKeys[$k]]);
				}
			}
			if (isset($this->attributes['displayName'])) {
				unset($this->attributes['displayName']);
			}
			return array();
		}
		// skip processing if extension is not active
		if (!$this->isExtensionEnabled()) {
			return array();
		}
		// delete LM hash if needed
		if (!isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) || ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'yes')) {
			if (isset($this->attributes['sambaLMPassword'])) {
				unset($this->attributes['sambaLMPassword']);
			}
		}
		$errors = array();
		$sambaDomains = $this->getDomains();
		if (sizeof($sambaDomains) == 0) {
			return array();
		}
		$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		$unixGroupName = $this->getGroupName($attrs['gidNumber'][0]);
		// Save attributes
		$this->attributes['sambaDomainName'][0] = $_POST['sambaDomainName'];
		// Get Domain SID from name
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
				break;
			}
		}
		$flag = "[";
		if (isset($_POST['sambaAcctFlagsD'])) {
			$flag .= "D";
		}
		if (isset($_POST['sambaAcctFlagsX'])) {
			$flag .= "X";
			$this->noexpire = true;
		}
		else {
			$this->noexpire = false;
		}
		if (isset($_POST['sambaAcctFlagsN'])) {
			$flag .= "N";
			$this->nopwd = true;
		}
		else {
			$this->nopwd = false;
		}
		if (isset($_POST['sambaAcctFlagsS'])) {
			$flag .= "S";
		}
		if (isset($_POST['sambaAcctFlagsW'])) {
			$flag .= "W";
		}
		if (isset($_POST['sambaAcctFlagsU'])) {
			$flag .= "U";
		}
		if (isset($_POST['sambaAcctFlagsL'])) {
			$flag .= "L";
		}
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['sambaacctflags'][0] = $flag;

		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!empty($this->attributes['displayName'][0])) {
			$this->attributes['displayName'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['displayName'][0]);
			$this->attributes['displayName'][0] = str_replace('$group', $unixGroupName, $this->attributes['displayName'][0]);
		}
		if (!($this->attributes['displayName'][0] == '') && !(get_preg($this->attributes['displayName'][0], 'realname'))) {
			$errors[] = $this->messages['displayName'][1];
		}

		// host attributes
		if ($this->get_scope()=='host') {
			$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->groupRids[_('Domain computers')];
			if (isset($_POST['ResetSambaPassword']) || !isset($this->attributes['sambaNTPassword'][0])) {
				$hostname = $attrs['uid'][0];
				$hostname = substr($hostname, 0, strlen($hostname) - 1);
				if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
					$this->attributes['sambaLMPassword'][0] = lmPassword($hostname);
				}
				$this->attributes['sambaNTPassword'][0] = ntPassword($hostname);
				$this->attributes['sambaPwdLastSet'][0] = time();
			}
		}
		// user attributes
		if ($this->get_scope()=='user') {
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				$this->attributes['sambaHomePath'][0] = $_POST['sambaHomePath'];
				$this->attributes['sambaHomePath'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['sambaHomePath'][0]);
				$this->attributes['sambaHomePath'][0] = str_replace('$group', $unixGroupName, $this->attributes['sambaHomePath'][0]);
				if ($this->attributes['sambaHomePath'][0] != $_POST['sambaHomePath']) {
					$errors[] = $this->messages['homePath'][1];
				}
				if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC'))) {
					$errors[] = $this->messages['homePath'][0];
				}
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				if ($_POST['sambaHomeDrive'] == "-") {
					$this->attributes['sambaHomeDrive'][0] = '';
				}
				else {
					$this->attributes['sambaHomeDrive'][0] = $_POST['sambaHomeDrive'];
				}
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				$this->attributes['sambaLogonScript'][0] = $_POST['sambaLogonScript'];
				$this->attributes['sambaLogonScript'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['sambaLogonScript'][0]);
				$this->attributes['sambaLogonScript'][0] = str_replace('$group', $unixGroupName, $this->attributes['sambaLogonScript'][0]);
				if ($this->attributes['sambaLogonScript'][0] != $_POST['sambaLogonScript']) {
					$errors[] = $this->messages['logonScript'][1];
				}
				if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript'))) {
					$errors[] = $this->messages['logonScript'][0];
				}
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				$this->attributes['sambaProfilePath'][0] = $_POST['sambaProfilePath'];
				$this->attributes['sambaProfilePath'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['sambaProfilePath'][0]);
				$this->attributes['sambaProfilePath'][0] = str_replace('$group', $unixGroupName, $this->attributes['sambaProfilePath'][0]);
				if ($this->attributes['sambaProfilePath'][0] != $_POST['sambaProfilePath']) {
					$errors[] = $this->messages['profilePath'][1];
				}
				if (!($this->attributes['sambaProfilePath'][0] == '') &&
					!(get_preg($this->attributes['sambaProfilePath'][0], 'UNC') xor get_preg($this->attributes['sambaProfilePath'][0], 'homeDirectory'))) {
						$errors[] = $this->messages['profilePath'][0];
				}
			}
			$rids = array_keys($this->groupRids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($_POST['sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->groupRids[$rids[$i]];
					}
			}
			if (!$wrid) {
				$gidnumber = $attrs['gidNumber'][0];
				$groups = $this->getGroupSIDList();
				if (isset($groups[$gidnumber]) && ($groups[$gidnumber] != '')) {
					$this->attributes['sambaPrimaryGroupSID'][0] = $groups[$gidnumber];
				}
			}

			$specialRids = array_flip($this->userRids);
			// set special RID if selected
			if (in_array($_POST['sambaSID'], $specialRids)) {
				$this->attributes['sambaSID'][0] = $SID . '-' . $this->userRids[$_POST['sambaSID']];
			}
			// standard RID
			else if ($_POST['sambaSID'] == "-") {
				$rid = '';
				if (isset($this->attributes['sambaSID'][0])) {
					$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
				}
				// change only if not yet set, previously set to special SID or domain changed
				if (!isset($this->attributes['sambaSID'][0])
					|| in_array($rid, $this->userRids)
					|| (strpos($this->attributes['sambaSID'][0], $SID) === false)) {
					$this->attributes['sambaSID'][0] = $SID . "-" . (($attrs['uidNumber'][0]*2) + $RIDbase);
				}
			}
		}
		else {  // host
			if (!isset($this->attributes['sambaSID'][0])) {
				$this->attributes['sambaSID'][0] = $SID."-". (($attrs['uidNumber'][0]*2)+$RIDbase);
			}
		}
		if (isset($_POST['forcePasswordChangeOption'])) {
			$this->expirePassword = true;
		}
		else {
			$this->expirePassword = false;
		}
		return $errors;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_sambaUserWorkstations() {
		// Load attributes
		if ($this->get_scope()=='user') {
			if (isset($_POST['workstations_2']) && isset($_POST['workstations_left'])) { // Add workstations to list
				$workstations = array();
				if (isset($this->attributes['sambaUserWorkstations'][0])) {
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++) {
						if ($workstations[$i]=='') {
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
				$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
					}
				}
			elseif (isset($_POST['workstations_1']) && isset($_POST['workstations_right'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++) {
					if ($workstations[$i]=='') {
						unset($workstations[$i]);
					}
				}
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($_POST['workstations_1'], $workstations);
				// Recreate workstation string
				unset($this->attributes['sambaUserWorkstations'][0]);
				if (sizeof($workstations) > 0) {
					$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
					}
				}
			}
		}
		return array();
	}

	/**
	* Processes user input of the logon hours page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_logonHours() {
		if (isset($_POST['form_subpage_sambaSamAccount_attributes_abort'])) {
			return array();
		}
		// set new logon hours
		$logonHours = '';
		for ($i = 0; $i < 24*7; $i++) {
			$logonHours .= isset($_POST['lh_' . $i]) ? '1' : '0';
		}
		// reconstruct HEX string
		$bitstring2hex = array_flip($this->hex2bitstring);
		$logonHoursNew = '';
		for ($i = 0; $i < 21; $i++) {
			$part = strrev(substr($logonHours, $i * 8, 8));
			$byte['hi'] = substr($part,0,4);
			$byte['low'] = substr($part,4,4);
			$hex = $bitstring2hex[$byte['hi']].$bitstring2hex[$byte['low']];
			$logonHoursNew = $logonHoursNew . $hex;
		}
		$this->attributes['sambaLogonHours'][0] = $logonHoursNew;
		return array();
	}

	/**
	* Processes user input of the time selection page.
	*
	* @return array list of info/error messages
	*/
	function process_time() {
		$return = array();
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (strpos($postKeys[$i], 'form_subpage_sambaSamAccount_attributes_') !== false) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (strpos($buttonName, '_back') !== false)) {
			return array();
		}
		// get attribute name
		$attr = '';
		if (strpos($buttonName, 'sambaKickoffTime') !== false) {
			$attr = 'sambaKickoffTime';
		}
		if ($attr == '') {
			return array();
		}
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
	* Processes user input of the terminal server page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_terminalServer() {
		if (isset($_POST['form_subpage_sambaSamAccount_attributes_abort'])) {
			return array();
		}
		$mDial = new sambaMungedDial();
		if (isset($this->attributes['sambaMungedDial'][0])) {
			$mDial->load($this->attributes['sambaMungedDial'][0]);
		}
		$mDial->setTsLogin(!isset($_POST['tsAllowLogin']));
		$mDial->ctx['CtxWFHomeDir'] = $_POST['tsHomeDir'];
		$mDial->ctx['CtxWFHomeDirDrive'] = $_POST['tsHomeDrive'];
		$mDial->ctx['CtxWFProfilePath'] = $_POST['tsProfilePath'];
		$mDial->setInheritMode(isset($_POST['tsInherit']));
		$mDial->ctx['CtxInitialProgram'] = $_POST['tsInitialProgram'];
		$mDial->ctx['CtxWorkDirectory'] = $_POST['tsWorkDirectory'];
		$mDial->ctx['CtxMaxConnectionTime'] = $_POST['tsConnectionLimit'];
		$mDial->ctx['CtxMaxDisconnectionTime'] = $_POST['tsDisconnectionLimit'];
		$mDial->ctx['CtxMaxIdleTime'] = $_POST['tsIdleLimit'];
		$mDial->setConnectClientDrives(isset($_POST['tsConnectDrives']));
		$mDial->setConnectClientPrinters(isset($_POST['tsConnectPrinters']));
		$mDial->setDefaultPrinter(isset($_POST['tsClientPrinterDefault']));
		$mDial->setShadow(true, $_POST['tsShadowing']);
		$mDial->setBrokenConn($_POST['tsBrokenConn']);
		$mDial->setReConn($_POST['tsReconnect']);
		$this->attributes['sambaMungedDial'][0] = $mDial->getMunged();
		return array();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlResponsiveRow();
		if ($this->isExtensionEnabled()) {
			if ($this->get_scope() == "host") {
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				if (substr($attrs['uid'][0], -1, 1) != '$') {
					$return->add(new htmlStatusMessage("ERROR", _('Host name must end with $!'), _('Please check your settings on the Unix page!')), 12);
				}
			}
			$personalAttributes = array();
			if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				$personalAttributes = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			}
			// Get Domain SID from user SID
			$sambaDomains = $this->getDomains();
			if (sizeof($sambaDomains) == 0) {
				$return->add(new htmlStatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.')), 12);
				return $return;
			}
			if (isset($this->attributes['sambaSID'][0]) && $this->attributes['sambaSID'][0] != '') {
				$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
			}
			$sel_domain = array();
			for ($i=0; $i<count($sambaDomains); $i++ ) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
				if (isset($domainSID)) {
					if ($domainSID == $sambaDomains[$i]->SID) {
						$SID = $domainSID;
						$sel_domain = array($sambaDomains[$i]->name);
					}
				}
				elseif (isset($this->attributes['sambaDomainName'][0]) && ($this->attributes['sambaDomainName'][0]!='')) {
					if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
						$SID = $sambaDomains[$i]->SID;
						$sel_domain = array($sambaDomains[$i]->name);
					}
				}
			}
			// display name
			$displayName = '';
			if (!empty($this->attributes['displayName'][0])) {
				$displayName = $this->attributes['displayName'][0];
			}
			else if ($this->getAccountContainer()->isNewAccount && empty($this->attributes['displayName'][0])) {
				if (isset($personalAttributes['givenName'][0]) && $personalAttributes['givenName'][0] && isset($personalAttributes['sn'][0]) && $personalAttributes['sn'][0]) {
					$displayName = $personalAttributes['givenName'][0] . " " . $personalAttributes['sn'][0];
				}
				elseif (isset($personalAttributes['sn'][0])) {
					$displayName = $personalAttributes['sn'][0];
				}
			}
			$return->add(new htmlResponsiveInputField(_('Display name'), 'displayName', $displayName, 'displayName'), 12);
			if ($this->get_scope()=='user') {
				// user account
				$return->add(new htmlHiddenInput('sambaAcctFlagsU', 'true'), 12);
				// no password
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsN', $this->nopwd, _('Use no password'), 'noPassword'), 12);
				// no password expiry
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsX', $this->noexpire, _('Password does not expire'), 'noExpire'), 12);
				// account deactivated
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsD', $this->isDeactivated(), _('Account is deactivated'), 'deactivated'), 12);
				// account locked
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsL', $this->isLocked(), _('Account is locked'), 'locked'), 12);
				// password change at next login
				$return->add(new htmlResponsiveInputCheckbox('forcePasswordChangeOption', $this->expirePassword, _('Password change at next login'), 'passwordIsExpired'), 12);
				// last password change
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideSambaPwdLastSet')) {
					$sambaPwdLastSet = null;
					if (!empty($this->attributes['sambaPwdLastSet'][0])) {
						$time = new DateTime('@' . $this->attributes['sambaPwdLastSet'][0], new DateTimeZone('UTC'));
						$time->setTimezone(getTimeZone());
						$sambaPwdLastSet = $time->format('d.m.Y H:i');
					}
					if ($sambaPwdLastSet !== null) {
						$return->addLabel(new htmlOutputText(_('Last password change')));
						$sambaPwdLastSetGroup = new htmlGroup();
						$sambaPwdLastSetGroup->addElement(new htmlOutputText($sambaPwdLastSet));
						$sambaPwdLastSetGroup->addElement(new htmlHelpLink('sambaPwdLastSet'));
						$return->addField($sambaPwdLastSetGroup);
					}
				}
				// password can be changed
				$return->addLabel(new htmlOutputText(_('User can change password')));
				$pwdCanChangeGroup = new htmlGroup();
				$pwdCanChangeGroup->addElement(new htmlOutputText($this->getPasswordCanChangeTime($sambaDomains, $sel_domain), false));
				$pwdCanChangeGroup->addElement(new htmlSpacer('0.5rem', null));
				$pwdCanChangeGroup->addElement(new htmlHelpLink('pwdCanChange'));
				$return->addField($pwdCanChangeGroup);
				// password must be changed
				$return->addLabel(new htmlOutputText(_('User must change password')));
				$pwdMustChangeGroup = new htmlGroup();
				$pwdMustChangeGroup->addElement(new htmlOutputText($this->getPasswordMustChangeTime($sambaDomains, $sel_domain), false));
				$pwdMustChangeGroup->addElement(new htmlSpacer('0.5rem', null));
				$pwdMustChangeGroup->addElement(new htmlHelpLink('pwdMustChange'));
				$return->addField($pwdMustChangeGroup);
				// account expiration time
				$dateValue = $this->formatAccountExpirationDate();
				$return->addLabel(new htmlOutputText(_('Account expiration date')));
				$expireDateGroup = new htmlGroup();
				$expireDateGroup->addElement(new htmlOutputText($dateValue, false));
				$expireDateGroup->addElement(new htmlSpacer('0.5rem', null));
				$expireDateGroup->addElement(new htmlAccountPageButton(get_class($this), 'time', 'sambaKickoffTime', _('Change')));
				$expireDateGroup->addElement(new htmlSpacer('0.5rem', null));
				$expireDateGroup->addElement(new htmlHelpLink('expireDate'), true);
				$return->addField($expireDateGroup);
				// home drive
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
					$drives = array('-');
					for ($i=90; $i>67; $i--) {
						$drives[] = chr($i).':';
					}
					if (isset($this->attributes['sambaHomeDrive'][0])) {
						$selected = array ($this->attributes['sambaHomeDrive'][0]);
					}
					else {
						$selected = array('-');
					}
					$return->add(new htmlResponsiveSelect('sambaHomeDrive', $drives, $selected, _('Home drive'), 'homeDrive'), 12);
				}
				// home path
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
					$sambaHomePath = '';
					if (isset($this->attributes['sambaHomePath'][0])) {
						$sambaHomePath = $this->attributes['sambaHomePath'][0];
					}
					$return->add(new htmlResponsiveInputField(_('Home path'), 'sambaHomePath', $sambaHomePath, 'homePath'), 12);
				}
				// profile path
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
					$sambaProfilePath = '';
					if (isset($this->attributes['sambaProfilePath'][0])) {
						$sambaProfilePath = $this->attributes['sambaProfilePath'][0];
					}
					$return->addLabel(new htmlOutputText(_('Profile path')));
					$sambaProfilePathGroup = new htmlGroup();
					$sambaProfilePathGroup->addElement(new htmlInputField('sambaProfilePath', $sambaProfilePath));
					if (($_SESSION['config']->get_scriptPath() != null) && ($_SESSION['config']->get_scriptPath() != '')) {
						if (get_preg($sambaProfilePath, 'homeDirectory')) {
							$sambaProfilePathButton = new htmlAccountPageButton(get_class($this), 'profilePath', 'manage', '../graphics/folder.svg', true);
							$sambaProfilePathButton->setTitle(_('Manage profile directory'));
							$sambaProfilePathGroup->addElement($sambaProfilePathButton);
						}
					}
					$sambaProfilePathGroup->addElement(new htmlHelpLink('profilePath'));
					$return->addField($sambaProfilePathGroup);
				}
				// logon script
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
					$sambaLogonScript = '';
					if (isset($this->attributes['sambaLogonScript'][0])) {
						$sambaLogonScript = $this->attributes['sambaLogonScript'][0];
					}
					$return->add(new htmlResponsiveInputField(_('Logon script'), 'sambaLogonScript', $sambaLogonScript, 'scriptPath'), 12);
				}
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
					$return->addLabel(new htmlOutputText(_('Samba workstations')));
					$userWorkstationsGroup = new htmlGroup();
					$userWorkstationsGroup->addElement(new htmlAccountPageButton(get_class($this), 'sambaUserWorkstations', 'open', _('Edit workstations')));
					$userWorkstationsGroup->addElement(new htmlSpacer('0.5rem', null));
					$userWorkstationsGroup->addElement(new htmlHelpLink('userWorkstations'));
					$return->addField($userWorkstationsGroup);
				}
				// Windows group
				$names = array_keys($this->groupRids);
				$wrid=false;
				$options = array();
				$selected = array();
				for ($i=0; $i<count($names); $i++) {
					if (isset($this->attributes['sambaPrimaryGroupSID'][0]) && ($this->attributes['sambaPrimaryGroupSID'][0] == $SID . "-" . $this->groupRids[$names[$i]])) {
						$selected[] = $names[$i];
						$wrid=true;
					}
					$options[] = $names[$i];
				}
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$options[] = $this->getGroupName($attrs['gidNumber'][0]);
				if (!$wrid) {
					$selected[] = $this->getGroupName($attrs['gidNumber'][0]);
				}
				$return->add(new htmlResponsiveSelect('sambaPrimaryGroupSID', $options, $selected, _('Windows group'), 'group'), 12);
				// display if group SID should be mapped to a well known SID
				$options = array_keys($this->userRids);
				$options[] = '-';
				$selected = array();
				if (isset($this->attributes['sambaSID'][0]) && ($this->attributes['sambaSID'][0] != '')) {
					$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
					$specialRids = array_flip($this->userRids);
					if (in_array($rid, $this->userRids)) {
						$selected = array($specialRids[$rid]);
					}
					else {
						$selected = array('-');
					}
				}
				else {
					$selected[] = "-";
				}
				$return->add(new htmlResponsiveSelect('sambaSID', $options, $selected, _('Special user'), 'specialUser'), 12);
			}
			// domain
			$return->add(new htmlResponsiveSelect('sambaDomainName', $sambaDomainNames, $sel_domain, _('Domain'), 'domain'), 12);
			// logon hours and terminal server options
			if ($this->get_scope()=='user') {
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
					$return->addLabel(new htmlOutputText(_('Logon hours')));
					$logonHoursGroup = new htmlGroup();
					$logonHoursGroup->addElement(new htmlAccountPageButton(get_class($this), 'logonHours', 'open', _('Edit')));
					$logonHoursGroup->addElement(new htmlSpacer('0.5rem', null));
					$logonHoursGroup->addElement(new htmlHelpLink('logonHours'));
					$return->addField($logonHoursGroup);
				}
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideTerminalServer')) {
					$return->addLabel(new htmlOutputText(_('Terminal server options')));
					$terminalServerGroup = new htmlGroup();
					$terminalServerGroup->addElement(new htmlAccountPageButton(get_class($this), 'terminalServer', 'open', _('Edit')));
					$terminalServerGroup->addElement(new htmlSpacer('0.5rem', null));
					$terminalServerGroup->addElement(new htmlHelpLink('terminalServer'));
					$return->addField($terminalServerGroup);
				}
			}
			// reset host password
			if ($this->get_scope()=='host') {
				// host account
				$return->add(new htmlHiddenInput('sambaAcctFlagsW', 'true'), 12);
				// password reset
				$return->addLabel(new htmlOutputText(_('Reset password')));
				$resetPasswordGroup = new htmlGroup();
				$resetPasswordGroup->addElement(new htmlButton('ResetSambaPassword', _('Reset')));
				$resetPasswordGroup->addElement(new htmlSpacer('0.5rem', null));
				$resetPasswordGroup->addElement(new htmlHelpLink('resetPassword'));
				$return->addField($resetPasswordGroup);
			}

			$return->addVerticalSpacer('2rem');

			$remButton = new htmlButton('remObjectClass', _('Remove Samba 3 extension'));
			$remButton->setCSSClasses(array('lam-danger'));
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add Samba 3 extension')), 12);
		}
		return $return;
	}

	/**
	 * Returns the account expiration date in printable form.
	 *
	 * @return string expiration date
	 */
	private function formatAccountExpirationDate() {
		$dateValue =  "-";
		if (isset($this->attributes['sambaKickoffTime'][0])) {
			if ($this->attributes['sambaKickoffTime'][0] > 2147483648) {
				$dateValue = "∞";
			}
			else {
				$date = new DateTime('@' . $this->attributes['sambaKickoffTime'][0], new DateTimeZone('UTC'));
				$dateValue = $date->format('d.m.Y');
			}
		}
		return $dateValue;
	}

	/**
	* This function will create the HTML page to edit the allowed workstations.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_sambaUserWorkstations() {
		$return = new htmlResponsiveRow();
		if ($this->get_scope()=='user') {
			// Get list of all hosts.
			$userWorkstations = array();
			$availableUserWorkstations = array();

			$result = $this->getHostList();
			foreach ($result as $host) {
				$availableUserWorkstations[] = str_replace("$", '', $host);
			}
			sort($availableUserWorkstations, SORT_STRING);
			if (isset($this->attributes['sambaUserWorkstations'][0])) {
				$wsAttr = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$userWorkstations = explode (',', $wsAttr);
			}
			$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);

			$return->add(new htmlSubTitle(_("Allowed workstations")), 12);

			$userWorkstationsOptions = array();
			foreach ($userWorkstations as $userWorkstation) {
				$userWorkstationsOptions[$userWorkstation] = $userWorkstation;
			}
			$availableUserWorkstationsOptions = array();
			foreach ($availableUserWorkstations as $availableUserWorkstation) {
				$availableUserWorkstationsOptions[$availableUserWorkstation] = $availableUserWorkstation;
			}
			$this->addDoubleSelectionArea($return, _("Allowed workstations"), _("Available workstations"), $userWorkstationsOptions, array(), $availableUserWorkstationsOptions, array(), 'workstations', false, true);

			$return->addVerticalSpacer('2rem');

			$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
			$return->add($backButton, 12);
		}
		return $return;
	}

	/**
	* This function will create the HTML page to edit logon hours.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_logonHours() {
		$return = new htmlResponsiveRow();
		$timeZone = getTimeZoneOffsetHours();
		$titles = array(_('Time'), _('Sunday'), _('Monday'), _('Tuesday'), _('Wednesday'), _('Thursday'),
			_('Friday'), _('Saturday'));
		$data = array();
		if (!isset($this->attributes['sambaLogonHours'][0]) || ($this->attributes['sambaLogonHours'][0] == '')) {
			$this->attributes['sambaLogonHours'][0] = 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF';
		}
		// convert existing logonHours string to bit array
		$logonHours = $this->attributes['sambaLogonHours'][0];
		$temp = array();
		for ($i = 0; $i < strlen($logonHours); $i++) {
			$temp[] = $this->hex2bitstring[$logonHours[$i]];
		}
		$logonHoursRev = implode('', $temp);
		// reverse bits low to high (1 is 0:00 sunday, 2 is 1:00 sunday, etc)
		$logonHours = "";
		for ($i = 0; $i < 21; $i++) {
			$logonHours .= strrev(substr($logonHoursRev, $i*8, 8));
		}
		$hour = array();
		for ($i = 0; $i < 24*7; $i++) {
			$hour[$i] = substr($logonHours, $i, 1);
		}
		// display input
		$boxes = array();
		// dynamically place boxes depending on time zone
		for ($i = 0; $i < 24*7; $i++) {
			$hr = $i + $timeZone;
			if ($hr < 0) {
				$hr = $hr + 24*7;
			}
			elseif ($hr >= 24*7) {
				$hr = $hr - 24*7;
			}
			$checkbox = new htmlInputCheckbox('lh_' . $hr, $hour[$hr]);
			$boxes[$i % 24][floor($i/24)] = $checkbox;
		}
		for ($h = 0; $h < 24; $h++) {
			$hour = $h;
			if ($h < 10) {
				$hour = '0' . $h;
			}
			$row = array();
			$row[] = new htmlOutputText("$hour:00 - $hour:59");
			for ($d = 0; $d < 7; $d++) {
				$row[] = $boxes[$h][$d];
			}
			$data[] = $row;
		}
		$return->add(new htmlResponsiveTable($titles, $data), 12);

		$return->addVerticalSpacer('2rem');
		$return->addLabel(new htmlAccountPageButton(get_class($this), 'attributes', 'submit', _('Ok')));
		$return->addField(new htmlAccountPageButton(get_class($this), 'attributes', 'abort', _('Cancel')));
		return $return;
	}

	/**
	* This function will create the meta HTML code to show a page to change time values.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_time() {
		$return = new htmlResponsiveRow();
		$attr = 'sambaKickoffTime';
		$text = _('Account expiration date');
		$help = "expireDate";
		$time = time() + 3600*24*365;
		if (isset($this->attributes[$attr][0])) {
			$time = $this->attributes[$attr][0];
		}
		$date = new DateTime('@' . $time, new DateTimeZone('UTC'));
		for ( $i=1; $i<=31; $i++ ) {
			$mday[] = $i;
		}
		for ( $i=1; $i<=12; $i++ ) {
			$mon[] = $i;
		}
		for ( $i=2003; $i<=2050; $i++ ) {
			$year[] = $i;
		}
		$return->addLabel(new htmlOutputText($text));
		$dateGroup = new htmlGroup();
		$daySelect = new htmlSelect('expire_day', $mday, array($date->format('j')));
		$daySelect->setWidth('3rem');
		$dateGroup->addElement($daySelect);
		$monthSelect = new htmlSelect('expire_mon', $mon, array($date->format('n')));
		$monthSelect->setWidth('3rem');
		$dateGroup->addElement($monthSelect);
		$yearSelect = new htmlSelect('expire_yea', $year, array($date->format('Y')));
		$yearSelect->setWidth('5rem');
		$dateGroup->addElement($yearSelect);
		$dateGroup->addElement(new htmlHelpLink($help));
		$return->addField($dateGroup);
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
		$buttons->addElement(new htmlSpacer('0.5rem', null));
		if (isset($this->attributes[$attr][0])) {
			$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'del' . $attr, _('Remove')));
			$buttons->addElement(new htmlSpacer('0.5rem', null));
		}
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back' . $attr, _('Cancel')));
		$return->add($buttons, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	* This function will create the HTML page to edit the terminal server options.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_terminalServer() {
		$return = new htmlResponsiveRow();
		$mDial = new sambaMungedDial();
		if (isset($this->attributes['sambaMungedDial'][0])) {
			$mDial->load($this->attributes['sambaMungedDial'][0]);
		}
		// terminal server login
		$return->add(new htmlResponsiveInputCheckbox('tsAllowLogin', $mDial->getTsLogin(), _('Allow terminal server login'), 'tsAllowLogin'), 12);
		// home directory
		$return->add(new htmlResponsiveInputField(_('Home directory'), 'tsHomeDir', $mDial->ctx['CtxWFHomeDir'], 'tsHomeDir'), 12);
		// home drive
		$drives = array();
		for ($i=90; $i>67; $i--) {
			$drives[] = chr($i).':';
		}
		$selTsDrive = array();
		if (isset($mDial->ctx['CtxWFHomeDirDrive'])) {
			$selTsDrive = array($mDial->ctx['CtxWFHomeDirDrive']);
		}
		$return->add(new htmlResponsiveSelect('tsHomeDrive', $drives, $selTsDrive, _('Home drive'), 'homeDrive'), 12);
		// profile path
		$return->add(new htmlResponsiveInputField(_('Profile path'), 'tsProfilePath', $mDial->ctx['CtxWFProfilePath'], 'tsProfilePath'), 12);
		// use startup program and working dir from client
		$return->add(new htmlResponsiveInputCheckbox('tsInherit', $mDial->getInheritMode(), _('Inherit client startup configuration'), 'tsInherit'), 12);
		// startup program
		$return->add(new htmlResponsiveInputField(_('Initial program'), 'tsInitialProgram', $mDial->ctx['CtxInitialProgram'], 'tsInitialProgram'), 12);
		// working dir
		$return->add(new htmlResponsiveInputField(_('Working directory'), 'tsWorkDirectory', $mDial->ctx['CtxWorkDirectory'], 'tsWorkDirectory'), 12);
		// connection time limit
		$tsConnectionLimit = new htmlResponsiveInputField(_('Connection time limit'), 'tsConnectionLimit', $mDial->ctx['CtxMaxConnectionTime'], 'tsTimeLimit');
		$tsConnectionLimit->setType('number');
		$return->add($tsConnectionLimit, 12);
		// disconnection time limit
		$tsDisconnectionLimit = new htmlResponsiveInputField(_('Disconnection time limit'), 'tsDisconnectionLimit', $mDial->ctx['CtxMaxDisconnectionTime'], 'tsTimeLimit');
		$tsDisconnectionLimit->setType('number');
		$return->add($tsDisconnectionLimit, 12);
		// idle time limit
		$tsIdleLimit = new htmlResponsiveInputField(_('Idle time limit'), 'tsIdleLimit', $mDial->ctx['CtxMaxIdleTime'], 'tsTimeLimit');
		$tsIdleLimit->setType('number');
		$return->add($tsIdleLimit, 12);
		// connect client drives
		$return->add(new htmlResponsiveInputCheckbox('tsConnectDrives', $mDial->getConnectClientDrives(), _('Connect client drives'), 'tsConnectDrives'), 12);
		// connect client printers
		$return->add(new htmlResponsiveInputCheckbox('tsConnectPrinters', $mDial->getConnectClientPrinters(), _('Connect client printers'), 'tsConnectPrinters'), 12);
		// client printer is default
		$return->add(new htmlResponsiveInputCheckbox('tsClientPrinterDefault', $mDial->getDefaultPrinter(), _('Client printer is default'), 'tsClientPrinterDefault'), 12);
		// shadowing
		$shadowOptions = array(
			_("disabled") => "0",
			_("input on, notify on") => "1",
			_("input on, notify off") => "2",
			_("input off, notify on") => "3",
			_("input off, notify off") => "4");
		$selShadow = array($mDial->getShadow());
		$shadowSelect = new htmlResponsiveSelect('tsShadowing', $shadowOptions, $selShadow, _('Shadowing'), 'tsShadowing');
		$shadowSelect->setHasDescriptiveElements(true);
		$return->add($shadowSelect, 12);
		// broken connection
		$brokenConnOptions = array(
			_("disconnect") => "0",
			_("reset") => "1");
		$selbrokenConn = array($mDial->getBrokenConn());
		$brokenConnSelect = new htmlResponsiveSelect('tsBrokenConn', $brokenConnOptions, $selbrokenConn, _('On broken or timed out connection'), 'tsBrokenConn');
		$brokenConnSelect->setHasDescriptiveElements(true);
		$return->add($brokenConnSelect, 12);
		// reconnect
		$reconnectOptions = array(
			_("from any client") => "0",
			_("from previous client only") => "1");
		$selReconnect = array($mDial->getReConn());
		$reconnectSelect = new htmlResponsiveSelect('tsReconnect', $reconnectOptions, $selReconnect, _('Reconnect if disconnected'), 'tsReconnect');
		$reconnectSelect->setHasDescriptiveElements(true);
		$return->add($reconnectSelect, 12);
		// buttons
		$return->addVerticalSpacer('2rem');
		$return->addLabel(new htmlAccountPageButton(get_class($this), 'attributes', 'submit', _('Ok')));
		$return->addField(new htmlAccountPageButton(get_class($this), 'attributes', 'abort', _('Cancel')));
		return $return;
	}

	/**
	* Displays manage profile path page.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_profilePath() {
		$return = new htmlResponsiveRow();
		$return->addLabel(new htmlOutputText(_('Profile path')));
		$return->addField(new htmlOutputText($this->attributes['sambaProfilePath'][0]));
		$return->addVerticalSpacer('2rem');
		// get list of remote servers
		$remoteServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($i = 0; $i < sizeof($remoteServers); $i++) {
			$remoteServer = $remoteServers[$i];
			$label = $remoteServer->getLabel();
			$remote = new \LAM\REMOTE\Remote();
			try {
				$remote->connect($remoteServer);
			}
			catch (LAMException $e) {
				$return->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
				continue;
			}
			$result = $remote->execute(
				implode(
					self::$SPLIT_DELIMITER,
					array(
						$this->attributes['uid'][0],
						"home",
						"check",
						$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0])
					));
			$remote->disconnect();
			// remote command results
			if (!empty($result)) {
				$returnValue = trim($result);
				if ($returnValue == 'ok') {
					$return->addLabel(new htmlOutputText($label));
					$editGroup = new htmlGroup();
					$editGroup->addElement(new htmlImage('../../graphics/pass.svg', 16, 16));
					$editGroup->addElement(new htmlSpacer('0.5rem', null));
					$editGroup->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'delete_' . $i, _('Delete')));
					$return->addField($editGroup);
				}
				elseif ($returnValue == 'missing') {
					$return->addLabel(new htmlOutputText($label));
					$editGroup = new htmlGroup();
					$editGroup->addElement(new htmlImage('../../graphics/del.svg', 16, 16));
					$editGroup->addElement(new htmlSpacer('0.5rem', null));
					$editGroup->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'create_' . $i, _('Create')));
					$return->addField($editGroup);
				}
				elseif (trim($returnValue) != '') {
					$messageParams = explode(",", $returnValue);
					if (isset($messageParams[2])) {
						$message = new htmlStatusMessage($messageParams[0], htmlspecialchars($messageParams[1]), htmlspecialchars($messageParams[2]));
					}
					elseif (($messageParams[0] == 'ERROR') || ($messageParams[0] == 'WARN') || ($messageParams[0] == 'INFO')) {
						$message = new htmlStatusMessage($messageParams[0], htmlspecialchars($messageParams[1]));
					}
					else {
						$message = new htmlStatusMessage('WARN', htmlspecialchars($messageParams[0]));
					}
					$return->add($message, 12);
				}
			}
		}
		$return->addVerticalSpacer('2rem');
		$return->add(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')), 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	* Processes user input of the profile path check page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_profilePath() {
		$return = array();
		$unixAttrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		$uidNumber = $unixAttrs['uidNumber'][0];
		$gidNumber = $unixAttrs['gidNumber'][0];
		if (empty($uidNumber) || empty($gidNumber)) {
			return;
		}
		// get list of remote servers
		$remoteServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($i = 0; $i < sizeof($remoteServers); $i++) {
			$remoteServer = $remoteServers[$i];
			if (isset($_POST['form_subpage_' . get_class($this) . '_homedir_create_' . $i])) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($remoteServer);
				}
				catch (LAMException $e) {
					$return[] = array('ERROR', $e->getTitle(), $e->getMessage());
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"directory",
							"add",
							$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0],
							"0".$_SESSION['config']->get_scriptRights(),
							$uidNumber,
							$gidNumber)
				));
				$remote->disconnect();
				// remote command results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (is_array($singleresult)
							&& (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO'))) {
						$return[] = $singleresult;
					}
				}
			}
			elseif (isset($_POST['form_subpage_' . get_class($this) . '_homedir_delete_' . $i])) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($remoteServer);
				}
				catch (LAMException $e) {
					$return[] = array('ERROR', $e->getTitle(), $e->getMessage());
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"rem",
							$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0],
							$uidNumber
						)
				));
				$remote->disconnect();
				// remote command results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (is_array($singleresult)
							&& (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO'))) {
						$return[] = $singleresult;
					}
				}
			}
		}
		return $return;
	}

	/**
	* {@inheritDoc}
	*/
	function get_profileOptions($typeId) {
		$return = parent::get_profileOptions($typeId);
		if ($this->get_scope() == 'user') {
			// lists for expiration date
			$day = array();
			$mon = array();
			$year = array();
			for ( $i=1; $i<=31; $i++ ) {
				$day[] = $i;
			}
			for ( $i=1; $i<=12; $i++ ) {
				$mon[] = $i;
			}
			for ( $i=2003; $i<=2030; $i++ ) {
				$year[] = $i;
			}
			// display name
			$return->add(new htmlResponsiveInputField(_('Display name'), 'sambaSamAccount_displayName', '', 'displayName'), 12);
			// use no password at all
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsN', false, _('Use no password'), 'noPassword'), 12);
			// account deactivation
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsD', false, _('Account is deactivated'), 'deactivated'), 12);
			// password never expires
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsX', false, _('Password does not expire'), 'noExpire'), 12);
			// expiration date
			$return->addLabel(new htmlOutputText(_('Account expiration date')));
			$expireContainer = new htmlResponsiveRow();
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_day', $day, array('1')), 2, 2, 2, 'padding-right05');
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_mon', $mon, array('1')), 2, 2, 2, 'padding-left-right05');
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_yea', $year, array('2030')), 6, 6, 6, 'padding-left-right05');
			$expireContainer->add(new htmlHelpLink('expireDate'), 2, 2, 2, 'padding-left-right05');
			$return->addField($expireContainer, 12);
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				// letter of home drive
				$drives = array('-');
				for ($i = 90; $i > 67; $i--) {
					$drives[] = chr($i) . ':';
				}
				$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaHomeDrive', $drives, array('-'), _('Home drive'), 'homeDrive'), 12);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				// path to home directory
				$return->add(new htmlResponsiveInputField(_('Home path'), 'sambaSamAccount_smbhome', '', 'homePath'), 12);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				// profile path
				$return->add(new htmlResponsiveInputField(_('Profile path'), 'sambaSamAccount_profilePath', '', 'profilePath'), 12);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				// logon script
				$return->add(new htmlResponsiveInputField(_('Logon script'), 'sambaSamAccount_logonScript', '', 'scriptPath'), 12);
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
				// allowed workstations
				$return->add(new htmlResponsiveInputField(_('Samba workstations'), 'sambaSamAccount_userWorkstations', '', 'workstations'), 12);
			}
			// domains
			$sambaDomains = $this->getDomains();
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaDomainName', $sambaDomainNames, null, _('Domain'), 'domain'), 12);
			// Windows group
			$groups = array();
			foreach ($this->groupRids as $key => $value) {
				$groups[$key] = $value;
			}
			$groups["-"] = "-";
			$groupSelect = new htmlResponsiveSelect('sambaSamAccount_group', $groups, array('513'), _('Windows group'), 'group');
			$groupSelect->setHasDescriptiveElements(true);
			$return->add($groupSelect, 12);
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
				// logon hours
				$return->add(new htmlResponsiveInputField(_('Logon hours'), 'sambaSamAccount_logonHours', '', 'logonHoursUpload'), 12);
			}
		}
		elseif ($this->get_scope() == 'host') {
			// domains
			$sambaDomains = $this->getDomains();
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaDomainName', $sambaDomainNames, null, _('Domain'), 'domain'), 12);
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
		// add extension
		if (isset($profile['sambaSamAccount_addExt'][0]) && ($profile['sambaSamAccount_addExt'][0] == "true")) {
			if (!in_array('sambaSamAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'sambaSamAccount';
			}
		}
		// use no password
		if (isset($profile['sambaSamAccount_sambaAcctFlagsN'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsN'][0] == "true")) {
			$this->nopwd = true;
		}
		elseif (isset($profile['sambaSamAccount_sambaAcctFlagsN'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsN'][0] == "false")) {
			$this->nopwd = false;
		}
		// password expiration
		if (isset($profile['sambaSamAccount_sambaAcctFlagsX'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsX'][0] == "true")) {
			$this->noexpire = true;
		}
		elseif (isset($profile['sambaSamAccount_sambaAcctFlagsX'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsX'][0] == "false")) {
			$this->noexpire = false;
		}
		// deactivation
		if (isset($profile['sambaSamAccount_sambaAcctFlagsD'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsD'][0] == "true")) {
			$this->deactivate($this->attributes);
		}
		elseif (isset($profile['sambaSamAccount_sambaAcctFlagsD'][0]) && ($profile['sambaSamAccount_sambaAcctFlagsD'][0] == "false")) {
			$this->activate($this->attributes);
		}
		if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
			// home drive
			if (isset($profile['sambaSamAccount_sambaHomeDrive'][0]) && ($profile['sambaSamAccount_sambaHomeDrive'][0] == "-")) {
				$this->attributes['sambaHomeDrive'][0] = '';
			}
			elseif (isset($profile['sambaSamAccount_sambaHomeDrive'][0])) {
				$this->attributes['sambaHomeDrive'][0] = $profile['sambaSamAccount_sambaHomeDrive'][0];
			}
		}
		// expiration date
		if (isset($profile['sambaSamAccount_expire_day'][0]) && ($profile['sambaSamAccount_expire_day'][0] != "")) {
			$date = DateTime::createFromFormat('j.n.Y', $profile['sambaSamAccount_expire_day'][0] . '.' . $profile['sambaSamAccount_expire_mon'][0] . '.' . $profile['sambaSamAccount_expire_yea'][0], getTimeZone());
			$this->attributes['sambaKickoffTime'][0] = $date->format('U');
		}
		// domain -> change SID
		if (isset($this->attributes['sambaSID'][0])) {
			if (isset($profile['sambaSamAccount_sambaDomainName'][0]) && ($profile['sambaSamAccount_sambaDomainName'][0] != "")) {
				$domains = $this->getDomains();
				$domSID = '';
				// find domain SID
				for ($i = 0; $i < sizeof($domains); $i++) {
					if ($domains[$i]->name == $profile['sambaSamAccount_sambaDomainName'][0]) {
						$domSID = $domains[$i]->SID;
						break;
					}
				}
				// replace domain part of SID
				if ($domSID != '') {
					$SID = $this->attributes['sambaSID'][0];
					$rid = substr($SID, strrpos($SID, '-') + 1);
					$SID = $domSID . '-' . $rid;
					$this->attributes['sambaSID'][0] = $SID;
				}
			}
		}
		// primary group
		if (isset($profile['sambaSamAccount_sambaDomainName'][0])) {
			$domains = $this->getDomains();
			$domSID = '';
			// find domain SID
			for ($i = 0; $i < sizeof($domains); $i++) {
				if ($domains[$i]->name == $profile['sambaSamAccount_sambaDomainName'][0]) {
					$domSID = $domains[$i]->SID;
					break;
				}
			}
			if ($domSID != '') {
				// set primary group if selected
				if (isset($profile['sambaSamAccount_group'][0]) && ($profile['sambaSamAccount_group'][0] != "-")) {
					$this->attributes['sambaPrimaryGroupSID'][0] = $domSID . "-" . $profile['sambaSamAccount_group'][0];
				}
			}
		}
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
	* @see baseModule::get_metaData()
	* @see htmlElement
	*/
	public function get_configOptions($scopes, $allScopes) {
		$return = parent::get_configOptions($scopes, $allScopes);
		if (!in_array('user', $scopes)) {
			return $return;
		}
		$configContainer = new htmlResponsiveRow();
		// password history
		$historyOptions = array(
				_('yes - ordered ascending') => 'yes_deleteLast',
				_('yes - ordered descending') => 'yes_deleteFirst',
				_('no') => 'no'
		);
		$historySelect = new htmlResponsiveSelect('sambaSamAccount_history', $historyOptions, array('yes_deleteLast'), _("Password history"), 'history');
		$historySelect->setHasDescriptiveElements(true);
		$configContainer->add($historySelect, 12);
		// disable LM passwords
		$yesNo = array(_('yes') => 'yes', _('no') => 'no');
		$lmYesNoSelect = new htmlResponsiveSelect('sambaSamAccount_lmHash', $yesNo, array('yes'), _("Disable LM hashes"), 'lmHash');
		$lmYesNoSelect->setHasDescriptiveElements(true);
		$configContainer->add($lmYesNoSelect, 12);
		// hidden options
		$configContainer->addVerticalSpacer('1rem');
		$configHiddenLabelGroup = new htmlGroup();
		$configHiddenLabelGroup->addElement(new htmlOutputText(_('Hidden options') . ' '));
		$configHiddenLabelGroup->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainer->add($configHiddenLabelGroup, 12);
		$configContainer->addVerticalSpacer('0.5rem');
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideHomeDrive', false, _('Home drive'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideHomePath', false, _('Home path'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideProfilePath', false, _('Profile path'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideLogonScript', false, _('Logon script'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideSambaPwdLastSet', false, _('Last password change'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideWorkstations', false, _('Samba workstations'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideLogonHours', false, _('Logon hours'), null, true), 12, 4);
		$configContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_hideTerminalServer', false, _('Terminal server options'), null, true), 12, 4);
		$configContainer->add(new htmlOutputText(''), 12, 4);
		$return[] = $configContainer;
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		$this->addSimplePDFField($return, 'sambaHomePath', _('Home path'));
		$this->addSimplePDFField($return, 'sambaHomeDrive', _('Home drive'));
		$this->addSimplePDFField($return, 'sambaLogonScript', _('Logon script'));
		$this->addSimplePDFField($return, 'sambaProfilePath', _('Profile path'));
		$this->addSimplePDFField($return, 'sambaUserWorkstations', _('Samba workstations'));
		$this->addSimplePDFField($return, 'sambaDomainName', _('Domain'));
		$this->addSimplePDFField($return, 'sambaPrimaryGroupSID', _('Windows group'));
		$this->addPDFKeyValue($return, 'sambaKickoffTime', _('Account expiration date'), $this->formatAccountExpirationDate());
		// terminal server options
		if (isset($this->attributes['sambaMungedDial'][0])) {
			$mDial = new sambaMungedDial();
			$mDial->load($this->attributes['sambaMungedDial'][0]);
			$tsAllowLogin = _('yes');
			if (!$mDial->getTsLogin()) {
				$tsAllowLogin = _('no');
			}
			$this->addPDFKeyValue($return, 'tsAllowLogin', _('Allow terminal server login'), $tsAllowLogin);
			$this->addPDFKeyValue($return, 'tsHomeDir', _('Home directory'). ' (TS)', $mDial->ctx['CtxWFHomeDir']);
			$this->addPDFKeyValue($return, 'tsHomeDrive', _('Home drive') . ' (TS)', $mDial->ctx['CtxWFHomeDirDrive']);
			$this->addPDFKeyValue($return, 'tsProfilePath', _('Profile path') . ' (TS)', $mDial->ctx['CtxWFProfilePath']);
			$tsInherit = _('yes');
			if (!$mDial->getInheritMode()) {
				$tsInherit = _('no');
			}
			$this->addPDFKeyValue($return, 'tsInherit', _('Inherit client startup configuration') . ' (TS)', $tsInherit);
			$this->addPDFKeyValue($return, 'tsInitialProgram', _('Initial program') . ' (TS)', $mDial->ctx['CtxInitialProgram']);
			$this->addPDFKeyValue($return, 'tsWorkDirectory', _('Working directory') . ' (TS)', $mDial->ctx['CtxWorkDirectory']);
			$this->addPDFKeyValue($return, 'tsConnectionLimit', _('Connection time limit') . ' (TS)', $mDial->ctx['CtxMaxConnectionTime']);
			$this->addPDFKeyValue($return, 'tsDisconnectionLimit', _('Disconnection time limit') . ' (TS)', $mDial->ctx['CtxMaxDisconnectionTime']);
			$this->addPDFKeyValue($return, 'tsIdleLimit', _('Idle time limit') . ' (TS)', $mDial->ctx['CtxMaxIdleTime']);
			$tsConnectDrives = _('yes');
			if (!$mDial->getConnectClientDrives()) {
				$tsConnectDrives = _('no');
			}
			$this->addPDFKeyValue($return, 'tsConnectDrives', _('Connect client drives') . ' (TS)', $tsConnectDrives);
			$tsConnectPrinters = _('yes');
			if (!$mDial->getConnectClientPrinters()) {
				$tsConnectPrinters = _('no');
			}
			$this->addPDFKeyValue($return, 'tsConnectPrinters', _('Connect client printers') . ' (TS)', $tsConnectPrinters);
			$tsClientPrinterDefault = _('yes');
			if (!$mDial->getDefaultPrinter()) {
				$tsClientPrinterDefault = _('no');
			}
			$this->addPDFKeyValue($return, 'tsClientPrinterDefault', _('Client printer is default') . ' (TS)', $tsClientPrinterDefault);
			$shadowOptions = array(
				'0' => _("disabled"),
				'1' => _("input on, notify on"),
				'2' => _("input on, notify off"),
				'3' => _("input off, notify on"),
				'4' => _("input off, notify off"));
			$tsShadowing = '';
			if (($mDial->getShadow() != null) && is_numeric($mDial->getShadow())) {
				$tsShadowing = $shadowOptions[$mDial->getShadow()];
			}
			$this->addPDFKeyValue($return, 'tsShadowing', _('Shadowing') . ' (TS)', $tsShadowing);
			$brokenConnOptions = array(
				'0' => _("disconnect"),
				'1' => _("reset"));
			$tsBrokenConn = '';
			if (($mDial->getBrokenConn() != null) && is_numeric($mDial->getBrokenConn())) {
				$tsBrokenConn = $brokenConnOptions[$mDial->getBrokenConn()];
			}
			$this->addPDFKeyValue($return, 'tsBrokenConn', _('On broken or timed out connection') . ' (TS)', $tsBrokenConn);
			$reconnectOptions = array(
				'0' => _("from any client"),
				'1' => _("from previous client only"));
			$tsReconnect = '';
			if (($mDial->getReConn() != null) && is_numeric($mDial->getReConn())) {
				$tsReconnect = $reconnectOptions[$mDial->getReConn()];
			}
			$this->addPDFKeyValue($return, 'tsReconnect', _('Reconnect if disconnected') . ' (TS)', $tsReconnect);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = array();
		// get list of Samba 3 domains
		$domains = $this->getDomains();
		// get list of Unix groups and their sambaSID + gidNumber
		$groupList = searchLDAPByFilter('objectClass=posixGroup', array('cn', 'sambaSID', 'gidNumber'), array('group'));
		$groups_cn = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			if (isset($groupList[$i]['sambasid'][0])) {
				$groups_cn[$groupList[$i]['cn'][0]]['SID'] = $groupList[$i]['sambasid'][0];
			}
			if (isset($groupList[$i]['gidnumber'][0])) {
				$groups_cn[$groupList[$i]['cn'][0]]['gid'] = $groupList[$i]['gidnumber'][0];
			}
		}
		if ($this->get_scope() == 'user') {
			for ($i = 0; $i < sizeof($rawAccounts); $i++) {
				if (!in_array("sambaSamAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaSamAccount";
				// displayName
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_displayName', 'displayName',
					'realname', $this->messages['displayName'][0], $errors);
				// password
				$partialAccounts[$i]['sambaPwdLastSet'] = time();
				if (!get_preg($rawAccounts[$i][$ids['sambaSamAccount_password']], 'password')) {
					$errMsg = $this->messages['sambaLMPassword'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// use Unix password
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == "") {  // default: use Unix
					if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
						$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					}
					$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == 'true') {  // use Unix
						if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
							$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
						}
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					}
					else {  // use given password
						if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
							$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
						}
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
					}
				}
				else {
					$errMsg = $this->messages['pwdUnix'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// use no password
				if ($rawAccounts[$i][$ids['sambaSamAccount_noPassword']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_noPassword']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_noPassword']] == 'true') {
							$partialAccounts[$i]['sambaLMPassword'] = 'NO PASSWORD*****';
							$partialAccounts[$i]['sambaNTPassword'] = 'NO PASSWORD*****';
						}
					}
					else {
						$errMsg = $this->messages['noPassword'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// account flags
				$flag_expire = false;
				$flag_deactivated = false;
				// password does not expire
				if ($rawAccounts[$i][$ids['sambaSamAccount_noExpire']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_noExpire']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_noExpire']] == 'false') {
							$flag_expire = true;
						}
					}
					else {
						$errMsg = $this->messages['noExpire'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// account is deactivated
				if ($rawAccounts[$i][$ids['sambaSamAccount_deactivated']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_deactivated']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_deactivated']] == 'true') {
							$flag_deactivated = true;
						}
					}
					else {
						$errMsg = $this->messages['deactivated'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// set flags
				$flags = "[";
				if ($flag_deactivated) $flags = $flags . "D";
				if (!$flag_expire) $flags = $flags . "X";
				$flags = $flags . "U";
				// Expand string to fixed length
				$flags = str_pad($flags, 12);
				// End character
				$flags = $flags . "]";
				$partialAccounts[$i]['sambaacctflags'] = $flags;
				// expiration date
				if ($rawAccounts[$i][$ids['sambaSamAccount_expireDate']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_expireDate']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaSamAccount_expireDate']]);
						$date = DateTime::createFromFormat('j.n.Y', $parts[0] . '.' . $parts[1] . '.' . $parts[2], getTimeZone());
						$partialAccounts[$i]['sambaKickoffTime'] = $date->format('U');
					}
					else {
						$errMsg = $this->messages['expireDate'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home drive
				if ($rawAccounts[$i][$ids['sambaSamAccount_homeDrive']] != "") {
					if (preg_match("/[d-z]:/i", $rawAccounts[$i][$ids['sambaSamAccount_homeDrive']])) {
						$partialAccounts[$i]['sambaHomeDrive'] = $rawAccounts[$i][$ids['sambaSamAccount_homeDrive']];
					}
					else {
						$errMsg = $this->messages['homeDrive'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home path
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_homePath', 'sambaHomePath',
					'UNC', $this->messages['homePath'][2], $errors);
				// profile path
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_profilePath', 'sambaProfilePath',
					'UNC', $this->messages['profilePath'][2], $errors);
				// logon script
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_logonScript', 'sambaLogonScript',
					'logonscript', $this->messages['logonScript'][2], $errors);
				// workstations
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_workstations', 'sambaUserWorkstations',
					'workstations', $this->messages['workstations'][1], $errors);
				// domain
				$domIndex = -1;
				for ($d = 0; $d < sizeof($domains); $d++) {
					if ($domains[$d]->name == $rawAccounts[$i][$ids['sambaSamAccount_domain']]) {
						$domIndex = $d;
						break;
					}
				}
				if ($domIndex > -1) {
					$partialAccounts[$i]['sambaDomainName'] = $domains[$domIndex]->name;
					$partialAccounts[$i]['sambaSID'] = $domains[$domIndex]->SID;
				}
				else {
					$errMsg = $this->messages['domain'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// group
				if ($rawAccounts[$i][$ids['sambaSamAccount_group']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_group']], 'groupname')
					&& (isset($groups_cn[$rawAccounts[$i][$ids['sambaSamAccount_group']]]))) {
						if (isset($groups_cn[$rawAccounts[$i][$ids['sambaSamAccount_group']]]['SID'])) {
							$partialAccounts[$i]['sambaPrimaryGroupSID'] = $groups_cn[$rawAccounts[$i][$ids['sambaSamAccount_group']]]['SID'];
						}
						else {
							$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' .
							($groups_cn[$rawAccounts[$i][$ids['sambaSamAccount_group']]]['gid'] * 2 +
							 $domains[$domIndex]->RIDbase + 1);
						}
					}
					elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_group']], array_keys($this->groupRids))) {
						$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->groupRids[$rawAccounts[$i][$ids['sambaSamAccount_group']]];
					}
					else {
						$errMsg = $this->messages['group'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default domain users
					$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->groupRids[_('Domain users')];
				}
				// special user
				if ($rawAccounts[$i][$ids['sambaSamAccount_rid']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_rid']], array_keys($this->userRids))) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $this->userRids[$rawAccounts[$i][$ids['sambaSamAccount_rid']]];
					}
					elseif (get_preg($rawAccounts[$i][$ids['sambaSamAccount_rid']], 'digit')) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $rawAccounts[$i][$ids['sambaSamAccount_rid']];
					}
					else {
						$errMsg = $this->messages['rid'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default RID uid*2 + RIDBase
					$partialAccounts[$i]['sambaSID'] .= '-' . ($partialAccounts[$i]['uidNumber']*2 + $domains[$domIndex]->RIDbase);
				}
				// logon hours
				$partialAccounts[$i]['sambaLogonHours'] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_logonHours', 'sambaLogonHours',
					'sambaLogonHours', $this->messages['logonHours'][1], $errors);
			}
		}
		else {  // hosts
			for ($i = 0; $i < sizeof($rawAccounts); $i++) {
				if (!in_array("sambaSamAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaSamAccount";
				// domain
				$domIndex = -1;
				for ($d = 0; $d < sizeof($domains); $d++) {
					if ($domains[$d]->name == $rawAccounts[$i][$ids['sambaSamAccount_domain']]) {
						$domIndex = $d;
						break;
					}
				}
				if ($domIndex > -1) {
					$partialAccounts[$i]['sambaDomainName'] = $domains[$domIndex]->name;
					$partialAccounts[$i]['sambaSID'] = $domains[$domIndex]->SID;
					$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . " - 515";
				}
				else {
					$errMsg = $this->messages['domain'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// RID
				if ($rawAccounts[$i][$ids['sambaSamAccount_rid']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_rid']], 'digit')) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $rawAccounts[$i][$ids['sambaSamAccount_rid']];
					}
					else {
						$errMsg = $this->messages['rid'][3];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default RID uid*2 + RIDBase
					$partialAccounts[$i]['sambaSID'] .= '-' . ($partialAccounts[$i]['uidNumber']*2 + $domains[$domIndex]->RIDbase);
				}
				// passwords ( = host name)
				$partialAccounts[$i]['sambaPwdLastSet'] = time();
				if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
					$partialAccounts[$i]['sambaLMPassword'] = lmPassword(substr($partialAccounts[$i]['uid'], 0, strlen($partialAccounts[$i]['uid']) - 1));
				}
				$partialAccounts[$i]['sambaNTPassword'] = ntPassword(substr($partialAccounts[$i]['uid'], 0, strlen($partialAccounts[$i]['uid']) - 1));
				// flags
				$partialAccounts[$i]['sambaacctflags'] = "[W          ]";
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getSelfServiceSettings()
	 */
	public function getSelfServiceSettings($profile) {
		$selfServiceContainer = new htmlResponsiveRow();
		// domain suffix
		$selfServiceDomainSuffix = new htmlResponsiveInputField(_('Domain suffix'), 'sambaSamAccount_domainSuffix', null, array('domainSuffix', get_class($this)));
		$selfServiceContainer->add($selfServiceDomainSuffix, 12);
		// password history
		$historyOptions = array(
				_('yes - ordered ascending') => 'yes_deleteLast',
				_('yes - ordered descending') => 'yes_deleteFirst',
				_('no') => 'no'
		);
		$historySelect = new htmlResponsiveSelect('sambaSamAccount_history', $historyOptions, array('yes_deleteLast'), _("Password history"), array('history', get_class($this)));
		$historySelect->setHasDescriptiveElements(true);
		$selfServiceContainer->add($historySelect, 12);
		return $selfServiceContainer;
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
			return $return; // no input fields as long no LDAP content can be read
		}
		if (!isset($attributes['objectClass']) || !in_array_ignore_case('sambaSamAccount', $attributes['objectClass'])) {
			return $return;
		}
		if (in_array('password', $fields)) {
			$group = new htmlGroup();
			$pwd1 = new htmlResponsiveInputField($this->getSelfServiceLabel('password', _('New password')), 'sambaSamAccount_password');
			$pwd1->setIsPassword(true, true);
			$group->addElement($pwd1);
			$pwd2 = new htmlResponsiveInputField(_('Reenter password'), 'sambaSamAccount_password2');
			$pwd2->setIsPassword(true);
			$pwd2->setSameValueFieldID('sambaSamAccount_password');
			$group->addElement($pwd2);
			$row = new htmlResponsiveRow();
			$row->add($group, 12);
			$return['password'] = $row;
		}
		if (in_array('sambaPwdLastSet', $fields)) {
			$sambaPwdLastSet = '';
			if (isset($attributes['sambaPwdLastSet'][0])) {
				$time = new DateTime('@' . $attributes['sambaPwdLastSet'][0], new DateTimeZone('UTC'));
				$time->setTimezone(getTimeZone());
				$sambaPwdLastSet = $time->format('d.m.Y H:i');
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('sambaPwdLastSet', _('Last password change'))));
			$row->addField(new htmlOutputText($sambaPwdLastSet));
			$return['sambaPwdLastSet'] = $row;
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
		if (!isset($attributes['objectClass']) || !in_array_ignore_case('sambaSamAccount', $attributes['objectClass'])) {
			return $return;
		}
		if (in_array('password', $fields)) {
			if (isset($_POST['sambaSamAccount_password']) && ($_POST['sambaSamAccount_password'] != '')) {
				if ($_POST['sambaSamAccount_password'] != $_POST['sambaSamAccount_password2']) {
					$return['messages'][] = $this->messages['sambaLMPassword'][0];
				}
				else {
					if (!get_preg($_POST['sambaSamAccount_password'], 'password')) {
						$return['messages'][] = $this->messages['sambaLMPassword'][1];
					}
					else {
						$userName = empty($attributes['uid'][0]) ? null : $attributes['uid'][0];
						$additionalAttrs = array();
						if (!empty($attributes['sn'][0])) {
							$additionalAttrs[] = $attributes['sn'][0];
						}
						if (!empty($attributes['givenName'][0])) {
							$additionalAttrs[] = $attributes['givenName'][0];
						}
						$pwdPolicyResult = checkPasswordStrength($_POST['sambaSamAccount_password'], $userName, $additionalAttrs);
						if ($pwdPolicyResult === true) {
							$return['mod']['sambaNTPassword'][0] = ntPassword($_POST['sambaSamAccount_password']);
							$return['info']['sambaUserPasswordClearText'][0] = $_POST['sambaSamAccount_password'];
							$this->doSelfServicePasswordHistoryAndMinAge($attributes, $return);
							if (array_key_exists('sambaLMPassword', $attributes)) {
								$return['mod']['sambaLMPassword'][0] = lmPassword($_POST['sambaSamAccount_password']);
							}
							if (array_key_exists('sambaPwdLastSet', $attributes)) {
								$return['mod']['sambaPwdLastSet'][0] = time();
							}
						}
						else {
							$return['messages'][] = array('ERROR', $pwdPolicyResult);
						}
					}
				}
			}
		}
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return $return;
			}
			else {
				if (!get_preg($_POST['posixAccount_password'], 'password')) {
					return $return;
				}
				else {
					$setPassword = false;
					// sync password
					if (in_array('syncNTPassword', $fields)) {
						$return['mod']['sambaNTPassword'][0] = ntPassword($_POST['posixAccount_password']);
						$setPassword = true;
					}
					if (in_array('syncLMPassword', $fields)) {
						$return['mod']['sambaLMPassword'][0] = lmPassword($_POST['posixAccount_password']);
						$setPassword = true;
					}
					if ($setPassword) {
						$return['info']['sambaUserPasswordClearText'][0] = $_POST['posixAccount_password'];
						$this->doSelfServicePasswordHistoryAndMinAge($attributes, $return);
						if (in_array('syncSambaPwdLastSet', $fields)) {
							$return['mod']['sambaPwdLastSet'][0] = time();
						}
					}
				}
			}
		}
		return $return;
	}

	/**
	 * Checks password history and password minimum age and updates history.
	 *
	 * @param array $attributes LDAP attributes of current account
	 * @param array $return return object of checkSelfServiceOptions()
	 */
	private function doSelfServicePasswordHistoryAndMinAge($attributes, &$return) {
		if (!empty($this->selfServiceSettings->moduleSettings['sambaSamAccount_domainSuffix'][0])) {
			$sambaDomain = $this->getUserDomain($attributes, $_SESSION['ldapHandle']->getServer(), $this->selfServiceSettings->moduleSettings['sambaSamAccount_domainSuffix'][0]);
			if ($sambaDomain == null) {
				return;
			}
			if (!empty($sambaDomain->pwdHistoryLength)
				&& is_numeric($sambaDomain->pwdHistoryLength)
				&& ($sambaDomain->pwdHistoryLength > 0)) {
				if (sambaSamAccount::oldPasswordUsed($return['info']['sambaUserPasswordClearText'][0], $attributes, $sambaDomain)) {
					$return['messages'][] = array('ERROR', _('You are reusing an old password. Please choose a different password.'));
				}
				else {
					// update password history
					if (sambaSamAccount::isPasswordHistoryEnabled($this->selfServiceSettings->moduleSettings)) {
						$sambaPasswordHistory = empty($attributes['sambaPasswordHistory']) ? array() : $attributes['sambaPasswordHistory'];
						while (sizeof($sambaPasswordHistory) > ($sambaDomain->pwdHistoryLength - 1)) {
							if (empty($this->selfServiceSettings->moduleSettings['sambaSamAccount_history'][0]) || ($this->selfServiceSettings->moduleSettings['sambaSamAccount_history'][0] == 'yes_deleteLast')) {
								array_pop($sambaPasswordHistory);
							}
							else {
								array_shift($sambaPasswordHistory);
							}
						}
						if (empty($this->selfServiceSettings->moduleSettings['sambaSamAccount_history'][0]) || ($this->selfServiceSettings->moduleSettings['sambaSamAccount_history'][0] == 'yes_deleteLast')) {
							array_unshift($sambaPasswordHistory, sambaSamAccount::createHistoryEntry($return['info']['sambaUserPasswordClearText'][0]));
						}
						else {
							$sambaPasswordHistory[] = sambaSamAccount::createHistoryEntry($return['info']['sambaUserPasswordClearText'][0]);
						}
						$sambaPasswordHistory = array_values($sambaPasswordHistory);
						if (empty($attributes['sambaPasswordHistory'])) {
							$return['add']['sambaPasswordHistory'] = $sambaPasswordHistory;
						}
						else {
							$return['mod']['sambaPasswordHistory'] = $sambaPasswordHistory;
						}
					}
				}
			}
			// check min age
			if (!empty($sambaDomain->minPwdAge) && ($sambaDomain->minPwdAge > 0) && !empty($attributes['sambaPwdLastSet'][0])) {
				$timeVal = $attributes['sambaPwdLastSet'][0] + $sambaDomain->minPwdAge;
				$time = new DateTime('@' . $timeVal, new DateTimeZone('UTC'));
				$time->setTimezone(getTimeZone());
				$now = new DateTime('now', getTimeZone());
				if ($time > $now) {
					$return['messages'][] = array('ERROR', _('You are not yet allowed to change your password.'));
				}
			}
		}
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		if ($this->get_scope() == "user") {
			return true;
		}
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
	 * @param $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$errors = array();
		if (isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) && ($this->moduleSettings['sambaSamAccount_lmHash'][0] == 'no')) {
			$this->attributes['sambaLMPassword'][0] = lmPassword($password);
		}
		$this->attributes['sambaNTPassword'][0] = ntPassword($password);
		$this->attributes['sambaPwdLastSet'][0] = time();
		if ($forcePasswordChange) {
			$this->attributes['sambaPwdLastSet'][0] = '0';
		}
		// password history entry
		$sambaDomain = $this->getUserDomain($this->attributes);
		if ($sambaDomain != null) {
			// password history check
			$oldPasswordUsed = sambaSamAccount::oldPasswordUsed($password, $this->orig, $sambaDomain);
			if ($oldPasswordUsed) {
				$errors[] = array('ERROR', _('You are reusing an old password. Please choose a different password.'));
			}
			// set new history entry
			$historyLength = $sambaDomain->pwdHistoryLength;
			if (sambaSamAccount::isPasswordHistoryEnabled($this->moduleSettings) && !$oldPasswordUsed && !empty($historyLength) && is_numeric($historyLength) && ($historyLength > 0)) {
				if (!empty($this->orig['sambaPasswordHistory'][0])) {
					$this->attributes['sambaPasswordHistory'] = $this->orig['sambaPasswordHistory'];
				}
				else {
					$this->attributes['sambaPasswordHistory'] = array();
				}
				while (sizeof($this->attributes['sambaPasswordHistory']) > ($historyLength - 1)) {
					if (empty($this->moduleSettings['sambaSamAccount_history'][0]) || ($this->moduleSettings['sambaSamAccount_history'][0] == 'yes_deleteLast')) {
						array_pop($this->attributes['sambaPasswordHistory']);
					}
					else {
						array_shift($this->attributes['sambaPasswordHistory']);
					}
				}
				if (empty($this->moduleSettings['sambaSamAccount_history'][0]) || ($this->moduleSettings['sambaSamAccount_history'][0] == 'yes_deleteLast')) {
					array_unshift($this->attributes['sambaPasswordHistory'], sambaSamAccount::createHistoryEntry($password));
				}
				else {
					$this->attributes['sambaPasswordHistory'][] = sambaSamAccount::createHistoryEntry($password);
				}
				$this->attributes['sambaPasswordHistory'] = array_values($this->attributes['sambaPasswordHistory']);
			}
		}
		return $errors;
	}

	/**
	 * Returns if an old password is used.
	 *
	 * @param String $password new password
	 */
	public static function oldPasswordUsed($password, $attributes, $sambaDomain) {
		$attributes = array_change_key_case($attributes, CASE_LOWER);
		if (empty($attributes['sambapasswordhistory'][0]) || ($sambaDomain == null)
				|| !is_numeric($sambaDomain->pwdHistoryLength) || ($sambaDomain->pwdHistoryLength < 1)) {
			return false;
		}
		foreach ($attributes['sambapasswordhistory'] as $historyEntry) {
			if (sambaSamAccount::validateHistoryEntry($password, $historyEntry)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the domain object of the user's domain.
	 *
	 * @param array $attributes LDAP attributes
	 * @param handle $server LDAP connection (leave empty for admin interface)
	 * @param String $suffix LDAP search suffix (leave empty for admin interface)
	 * @return samba3domain domain
	 */
	public function getUserDomain($attributes, $server = null, $suffix = null) {
		$attributes = array_change_key_case($attributes, CASE_LOWER);
		$sambaDomains = $this->getDomains($server, $suffix);
		if (sizeof($sambaDomains) > 0) {
			$domainSID = null;
			if (isset($attributes['sambasid'][0]) && $attributes['sambasid'][0] != '') {
				$domainSID = substr($attributes['sambasid'][0], 0, strrpos($attributes['sambasid'][0], "-"));
			}
			for ($i = 0; $i < count($sambaDomains); $i++) {
				if (!empty($domainSID)) {
					if (($domainSID == $sambaDomains[$i]->SID) && !empty($sambaDomains[$i]->pwdHistoryLength)) {
						return $sambaDomains[$i];
					}
				}
				elseif (isset($attributes['sambadomainname'][0]) && ($attributes['sambadomainname'][0]!='')) {
					if (($attributes['sambadomainname'][0] == $sambaDomains[$i]->name) && !empty($sambaDomains[$i]->pwdHistoryLength)) {
						return $sambaDomains[$i];
					}
				}
			}
		}
		return null;
	}

	/**
	 * Returns the group name of the group with the given group ID.
	 *
	 * @param String $groupID group ID
	 * @return String group name
	 */
	private function getGroupName($groupID) {
		$results = searchLDAPByAttribute('gidNumber', $groupID, 'posixGroup', array('cn'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['cn'][0])) {
			return $results[0]['cn'][0];
		}
		return null;
	}

	/**
	 * Returns the time when the user needs to change his password.
	 *
	 * @param array $domains list of domain objects
	 * @param String $selectedDomain selected domain name
	 */
	private function getPasswordMustChangeTime($domains, $selectedDomain) {
		if (is_array($selectedDomain) && (sizeof($selectedDomain) > 0)) {
			$selectedDomain = $selectedDomain[0];
		}
		$return = '-';
		// check if password expires at all
		if ($this->noexpire) {
			return $return;
		}
		// check if there is a time set for the last password change
		if (!isset($this->attributes['sambaPwdLastSet'][0])) {
			return $return;
		}
		for ($i = 0; $i < sizeof($domains); $i++) {
			if ($domains[$i]->name == $selectedDomain) {
				// check if a domain policy is set
				if (!isset($domains[$i]->maxPwdAge) || ($domains[$i]->maxPwdAge < 0)) {
					return $return;
				}
				$timeVal = $this->attributes['sambaPwdLastSet'][0] + $domains[$i]->maxPwdAge;
				$time = new DateTime('@' . $timeVal, new DateTimeZone('UTC'));
				$time->setTimezone(getTimeZone());
				return $time->format('d.m.Y H:i');
			}
		}
		return $return;
	}

	/**
	 * Returns the time when the user can change his password.
	 *
	 * @param array $domains list of domain objects
	 * @param String $selectedDomain selected domain name
	 */
	private function getPasswordCanChangeTime($domains, $selectedDomain) {
		if (is_array($selectedDomain) && (sizeof($selectedDomain) > 0)) {
			$selectedDomain = $selectedDomain[0];
		}
		$return = '-';
		// check if there is a time set for the last password change
		if (!isset($this->attributes['sambaPwdLastSet'][0])) {
			return $return;
		}
		for ($i = 0; $i < sizeof($domains); $i++) {
			if ($domains[$i]->name == $selectedDomain) {
				// check if a domain policy is set
				if (!isset($domains[$i]->minPwdAge) || ($domains[$i]->minPwdAge < 0)) {
					return $return;
				}
				$timeVal = $this->attributes['sambaPwdLastSet'][0] + $domains[$i]->minPwdAge;
				$time = new DateTime('@' . $timeVal, new DateTimeZone('UTC'));
				$time->setTimezone(getTimeZone());
				return $time->format('d.m.Y H:i');
			}
		}
		return $return;
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
		$this->cachedHostList = searchLDAPByAttribute('uid', '*', 'sambaSamAccount', array('uid'), array('host'));
		for ($i = 0; $i < sizeof($this->cachedHostList); $i++) {
			$this->cachedHostList[$i] = $this->cachedHostList[$i]['uid'][0];
		}
		return $this->cachedHostList;
	}

	/**
	 * Returns a list of existing hosts.
	 *
	 * @return array host names
	 */
	private function getGroupSIDList() {
		if ($this->cachedGroupSIDList != null) {
			return $this->cachedGroupSIDList;
		}
		$this->cachedGroupSIDList = array();
		$result = searchLDAPByAttribute('sambaSID', '*', 'sambaGroupMapping', array('gidNumber', 'sambaSID'), array('group'));
		for ($i = 0; $i < sizeof($result); $i++) {
			if (isset($result[$i]['gidnumber'][0])) {
				$this->cachedGroupSIDList[$result[$i]['gidnumber'][0]] = $result[$i]['sambasid'][0];
			}
		}
		return $this->cachedGroupSIDList;
	}

	/**
	 * Returns a list of existing Samba 3 domains.
	 *
	 * @param handle $server LDAP connection (leave empty for admin interface)
	 * @param String $suffix LDAP search suffix (leave empty for admin interface)
	 * @return array list of samba3domain objects
	 */
	private function getDomains($server = null, $suffix = null) {
		if ($this->cachedDomainList != null) {
			return $this->cachedDomainList;
		}
		$this->cachedDomainList = search_domains($server, $suffix);
		return $this->cachedDomainList;
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
			unset($this->attributes['sambaKickoffTime']);
			return;
		}
		$date = DateTime::createFromFormat('j.n.Y', $day . '.' . $month . '.' . $year, getTimeZone());
		$this->attributes['sambaKickoffTime'][0] = $date->format('U');
	}

	/**
	 * Returns if the Samba extension is enabled.
	 *
	 * @return boolean Samba extension is active
	 */
	public function isExtensionEnabled() {
		return in_array('sambaSamAccount', $this->attributes['objectClass']);
	}

	/**
	 * Returns if the Samba part of the current account is locked.
	 *
	 * @param ?array $attributes LDAP attributes
	 * @return boolean account is locked
	 */
	public function isLocked(?array $attributes = null) {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		if (!isset($attributes['sambaacctflags'][0])) {
			return false;
		}
		return strpos($attributes['sambaacctflags'][0], 'L') !== false;
	}

	/**
	 * Returns if the Samba part of the current account is deactivated.
	 *
	 * @param ?array $attributes LDAP attributes
	 * @return boolean account is deactivated
	 */
	public function isDeactivated(?array $attributes = null) {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		if (!isset($attributes['sambaacctflags'][0])) {
			return false;
		}
		return strpos($attributes['sambaacctflags'][0], 'D') !== false;
	}

	/**
	 * Deactivates this account.
	 *
	 * @param array $attributes LDAP attributes
	 */
	public function deactivate(array &$attributes) {
		if (!isset($attributes['sambaacctflags'][0])) {
			if ($this->get_scope() === 'host') {
				$attributes['sambaacctflags'][0] = '[W          ]';
			}
			else {
				$attributes['sambaacctflags'][0] = '[ XU        ]';
			}
		}
		$flags = $attributes['sambaacctflags'][0];
		if (strpos($flags, 'D') === false) {
			$flags[strpos($flags, ' ')] = 'D';
		}
		$attributes['sambaacctflags'][0] = $flags;
	}

	/**
	 * Activates this account.
	 *
	 * @param array $attributes LDAP attributes
	 */
	public function activate(array &$attributes) {
		if (!isset($attributes['sambaacctflags'][0])) {
			if ($this->get_scope() === 'host') {
				$attributes['sambaacctflags'][0] = '[W          ]';
			}
			else {
				$attributes['sambaacctflags'][0] = '[ XU        ]';
			}
		}
		$attributes['sambaacctflags'][0] = str_replace('D', ' ', $attributes['sambaacctflags'][0]);
	}

	/**
	 * Unlocks this account.
	 *
	 * @param array $attributes LDAP attributes
	 */
	public function unlock(array &$attributes) {
		$attributes['sambaacctflags'][0] = str_replace('L', ' ', $attributes['sambaacctflags'][0]);
	}

	/**
	 * Creates the value to store in sambaPasswordHistory attribute.
	 *
	 * @param String $password password
	 * @return String value for sambaPasswordHistory
	 */
	public static function createHistoryEntry($password) {
		if (empty($password)) {
			return null;
		}
		$salt = generateSalt(16);
		$saltHex = bin2hex($salt);
		$md4hash = ntPassword($password);
		$md5hash = md5($salt . hex2bin($md4hash));
		return strtoupper($saltHex . $md5hash);
	}

	/**
	 * Checks if the given password matches the history entry.
	 *
	 * @param String $password password
	 * @param String $historyEntry sambaPasswordHistory entry
	 * @return Boolean entry matches password
	 */
	public static function validateHistoryEntry($password, $historyEntry) {
		if (empty($historyEntry) || (strlen($historyEntry) != 64)) {
			return false;
		}
		$salt = hex2bin(substr($historyEntry, 0, 32));
		$hash = substr($historyEntry, 32, 32);
		$md4hash = ntPassword($password);
		$md5hash = md5($salt . hex2bin($md4hash));
		return strtolower($md5hash) == strtolower($hash);
	}

	/**
	 * Returns if password history is enabled.
	 *
	 * @param array $settings server profile or self service settings
	 */
	public static function isPasswordHistoryEnabled($settings) {
		return empty($settings['sambaSamAccount_history']) || ($settings['sambaSamAccount_history'][0] != 'no');
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
		// no details
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeOptions(): array {
		$options = array();
		if ((isset($this->attributes['sambaacctflags'][0])
			&& ((strpos($this->attributes['sambaacctflags'][0], 'L') !== false)
				|| (strpos($this->attributes['sambaacctflags'][0], 'D') !== false)))) {
			$options[] = new PasswordQuickChangeOption('unlockAccountSamba', _('Unlock Samba account'));
		}
		if (in_array_ignore_case('sambaSamAccount', $this->attributes['objectClass'])) {
			$options[] = new PasswordQuickChangeOption('syncSambaNT', _('Change Samba NT password'));
			if (!isset($this->moduleSettings['sambaSamAccount_lmHash'][0]) || ($this->moduleSettings['sambaSamAccount_lmHash'][0] != 'yes')) {
				$options[] = new PasswordQuickChangeOption('syncSambaLM', _('Change Samba LM password'));
			}
			$options[] = new PasswordQuickChangeOption('forcePasswordChangeSamba', _('Force Samba password change'), isPasswordChangeByDefault());
		}
		if (isset($this->attributes['sambaPwdLastSet'][0])) {
			$options[] = new PasswordQuickChangeOption('updateSambaTimestamps', _('Update Samba password timestamp'));
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		$attrs = array();
		if (isset($_POST['unlockAccountSamba']) && isset($this->attributes['sambaacctflags'][0])) {
			if ((strpos($this->attributes['sambaacctflags'][0], 'L') !== false)
				|| (strpos($this->attributes['sambaacctflags'][0], 'D') !== false)) {
				$sambaFlags = $this->attributes['sambaacctflags'][0];
				$sambaFlags = str_replace('L', ' ', $sambaFlags);
				$sambaFlags = str_replace('D', ' ', $sambaFlags);
				$attrs['sambaacctflags'][0] = $sambaFlags;
			}
		}
		if (isset($_POST['syncSambaLM'])) {
			$attrs['sambaLMPassword'][0] = lmPassword($password);
		}
		if (isset($_POST['syncSambaNT'])) {
			$attrs['sambaNTPassword'][0] = ntPassword($password);
		}
		// update Samba password timestamps
		if (isset($_POST['updateSambaTimestamps'])) {
			$attrs['sambaPwdLastSet'][0] = time();
		}
		// password history
		if (isset($_POST['syncSambaLM']) || isset($_POST['syncSambaNT'])) {
			$domain = $this->getUserDomain($this->attributes);
			if (($domain != null) && !empty($domain->pwdHistoryLength) && ($domain->pwdHistoryLength > 0)) {
				if (!empty($this->attributes['sambaPasswordHistory'][0])) {
					$attrs['sambaPasswordHistory'] = $this->attributes['sambaPasswordHistory'];
				}
				else {
					$attrs['sambaPasswordHistory'] = array();
				}
				while (sizeof($attrs['sambaPasswordHistory']) > ($domain->pwdHistoryLength - 1)) {
					array_pop($attrs['sambaPasswordHistory']);
				}
				array_unshift($attrs['sambaPasswordHistory'], self::createHistoryEntry($password));
				$attrs['sambaPasswordHistory'] = array_values($attrs['sambaPasswordHistory']);
			}
		}
		// force password change
		if (isset($_POST['forcePasswordChangeSamba'])) {
			$attrs['sambaPwdLastSet'][0] = '0';
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
		return array();
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool {
		// skip if no password was set
		if (!isset($_POST['syncSambaLM']) && !isset($_POST['syncSambaNT'])) {
			return false;
		}
		$sambaDomain = $this->getUserDomain($this->attributes);
		if ($sambaDomain == null) {
			return false;
		}
		if (!empty($sambaDomain->pwdHistoryLength)
			&& is_numeric($sambaDomain->pwdHistoryLength)
			&& ($sambaDomain->pwdHistoryLength > 0)) {
			return sambaSamAccount::oldPasswordUsed($password, $this->attributes, $sambaDomain);
		}
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusDetails(ConfiguredType $type, ?array &$attributes): array {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$details = array();
		if (self::isDeactivated($attributes)) {
			$details[] = AccountStatusDetails::newPartiallyLocked(_('Samba') . ': ' . _('Deactivated'), self::STATUS_ACCOUNT_DEACTIVATED);
		}
		if (self::isLocked($attributes)) {
			$details[] = AccountStatusDetails::newPartiallyLocked(_('Samba') . ': ' . _('Locked'), self::STATUS_ACCOUNT_LOCKED);
		}
		return $details;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusRequiredAttributes(ConfiguredType $type): array {
		return array('sambaacctflags');
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusPossibleLockOptions(ConfiguredType $type, ?array &$attributes): array {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$options = array();
		if (!self::isDeactivated($attributes)) {
			$options[] = AccountStatusDetails::newPartiallyLocked(_('Samba'), self::STATUS_ACCOUNT_DEACTIVATED);
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
		if (in_array(self::STATUS_ACCOUNT_DEACTIVATED, $lockIds)) {
			self::deactivate($attributes);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformUnlock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_ACCOUNT_DEACTIVATED, $lockIds)) {
			self::activate($attributes);
		}
		if (in_array(self::STATUS_ACCOUNT_LOCKED, $lockIds)) {
			self::unlock($attributes);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListFilterFunction(string $attributeName): ?callable {
		if ($attributeName == 'sambakickofftime') {
			return function(?array $values, ?string $filterValue): bool {
				$regex = str_replace(array('*'), array('.*'), $filterValue);
				$regex = '/' . $regex . '/i';
				if (!empty($values[0])) {
					if ($values[0] > 2147483648) {
						$time = "∞";
					}
					else {
						$date = new DateTime('@' . $values[0], new DateTimeZone('UTC'));
						$time = $date->format('d.m.Y');
					}
					return preg_match($regex, $time) || ($time === $filterValue);
				}
				return false;
			};
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName): ?callable {
		if ($attributeName === 'sambakickofftime') {
			return function(array $entry, string $attribute): ?htmlElement {
				if (!empty($entry[$attribute][0])) {
					if ($entry[$attribute][0] > 2147483648) {
						return new htmlOutputText("∞");
					}
					else {
						$date = new DateTime('@' . $entry[$attribute][0], new DateTimeZone('UTC'));
						return new htmlOutputText($date->format('d.m.Y'));
					}
				}
				return null;
			};
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return array(
			'displayname' => _('Display name'),
			'sambakickofftime' => _('Account expiration date'),
			'sambadomainname' => _('Domain'),
			'sambaprimarygroupsid' => _('Windows group')
		);
	}

}
