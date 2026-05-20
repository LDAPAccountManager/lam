<?php

use LAM\REMOTE\Remote;
use LAM\TYPES\ConfiguredType;
use LDAP\Connection;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2026  Roland Gruber

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

include_once(__DIR__ . '/sambaSamAccount/sambaMungedDial.inc');

/**
 * Manages the object class "sambaSamAccount" for users and hosts.
 *
 * @package modules
 */
class sambaSamAccount extends baseModule implements passwordService, AccountStatusProvider {

	/**
	 * ID for locked account status.
	 */
	private const STATUS_ACCOUNT_LOCKED = "SAMBA_ACCOUNT_LOCKED";

	/**
	 * ID for locked account status.
	 */
	private const STATUS_ACCOUNT_DEACTIVATED = "SAMBA_ACCOUNT_DEACTIVATED";

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['sambaSID', 'sambaNTPassword',
		'sambaPwdLastSet', 'displayName', 'sambaHomePath', 'sambaProfilePath',
		'sambaPasswordHistory', 'sambaPwdCanChange', 'sambaPwdMustChange', 'sambaKickoffTime',
		'sambaLogonTime', 'sambaLogoffTime'];

	// Variables
	/** use no password? */
	private bool $nopwd = false;
	/** password does not expire? */
	private bool $noexpire = true;
	/** @var array<string, int> $groupRids array of well known group rids */
	private array $groupRids;
	/** @var array<string, int> $userRids array of well known user rids */
	private array $userRids;
	/** @var array<string|int, string> $hex2bitstring HEX to binary conversion table */
	private array $hex2bitstring = ['0' => '0000', '1' => '0001', '2' => '0010', '3' => '0011', '4' => '0100',
		'5' => '0101', '6' => '0110', '7' => '0111', '8' => '1000', '9' => '1001', 'A' => '1010',
		'B' => '1011', 'C' => '1100', 'D' => '1101', 'E' => '1110', 'F' => '1111'];
	/** specifies if the password should be expired */
	private bool $expirePassword = false;
	/** @var string[]|null host cache to reduce LDAP queries */
	private ?array $cachedHostList = null;
	/** @var array<string, string>|null $cachedGroupSIDList group cache to reduce LDAP queries */
	private ?array $cachedGroupSIDList = null;
	/** @var samba3domain[]|null $cachedDomainList cache for domain list */
	private ?array $cachedDomainList = null;
	/** delimiter for remote commands */
	private static string $SPLIT_DELIMITER = "###x##y##x###";


	/**
	 * Creates a new sambaSamAccount object.
	 *
	 * @param string $scope account type (user, group, host)
	 */
	public function __construct($scope) {
		// List of well known group rids
		$this->groupRids = [
			_('Domain admins') => 512, _('Domain users') => 513, _('Domain guests') => 514,
			_('Domain computers') => 515, _('Domain controllers') => 516, _('Domain certificate admins') => 517,
			_('Domain schema admins') => 518, _('Domain enterprise admins') => 519, _('Domain policy admins') => 520];
		// List of well known user rids
		$this->userRids = [
			_('Domain admins') => 500, _('Domain guests') => 501, _('Domain KRBTGT') => 502];
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		// error messages for input checks
		$this->messages['homePath'][0] = ['ERROR', _('Home path'), _('Home path is invalid.')];
		$this->messages['homePath'][1] = ['INFO', _('Home path'), _('Inserted user or group name in home path.')];
		$this->messages['homePath'][2] = ['ERROR', _('Account %s:') . ' sambaSamAccount_homePath', _('Home path is invalid.')];
		$this->messages['profilePath'][0] = ['ERROR', _('Profile path'), _('Profile path is invalid!')];
		$this->messages['profilePath'][1] = ['INFO', _('Profile path'), _('Inserted user or group name in profile path.')];
		$this->messages['profilePath'][2] = ['ERROR', _('Account %s:') . ' sambaSamAccount_profilePath', _('Profile path is invalid!')];
		$this->messages['logonScript'][0] = ['ERROR', _('Logon script'), _('Logon script is invalid!')];
		$this->messages['logonScript'][1] = ['INFO', _('Logon script'), _('Inserted user or group name in logon script.')];
		$this->messages['logonScript'][2] = ['ERROR', _('Account %s:') . ' sambaSamAccount_logonScript', _('Logon script is invalid!')];
		$this->messages['workstations'][0] = ['ERROR', _('Samba workstations'), _('Please enter a comma separated list of host names!')];
		$this->messages['workstations'][1] = ['ERROR', _('Account %s:') . ' sambaSamAccount_workstations', _('Please enter a comma separated list of host names!')];
		$this->messages['sambaNTPassword'][0] = ['ERROR', _('Password'), _('Please enter the same password in both password fields.')];
		$this->messages['sambaNTPassword'][1] = ['ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!'];
		$this->messages['sambaNTPassword'][2] = ['ERROR', _('Account %s:') . ' sambaSamAccount_password', _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!'];
		$this->messages['rid'][2] = ['ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('Please enter a RID number or the name of a special account!')];
		$this->messages['rid'][3] = ['ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('This is not a valid RID number!')];
		$this->messages['displayName'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_displayName', _('Please enter a valid display name!')];
		$this->messages['displayName'][1] = ['ERROR', _('Display name'), _('Please enter a valid display name!')];
		$this->messages['pwdUnix'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_pwdUnix', _('This value can only be "true" or "false".')];
		$this->messages['noPassword'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_noPassword', _('This value can only be "true" or "false".')];
		$this->messages['noExpire'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_noExpire', _('This value can only be "true" or "false".')];
		$this->messages['deactivated'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_deactivated', _('This value can only be "true" or "false".')];
		$this->messages['expireDate'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_expireDate', _('Please enter a valid date in format DD-MM-YYYY.')];
		$this->messages['homeDrive'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_homeDrive', _('Please enter a valid drive letter.')];
		$this->messages['domain'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_domain', _('LAM was unable to find a domain with this name!')];
		$this->messages['logonHours'][0] = ['ERROR', _('Logon hours'), _('The format of the logon hours field is invalid!')];
		$this->messages['logonHours'][1] = ['ERROR', _('Account %s:') . ' sambaSamAccount_logonHours', _('The format of the logon hours field is invalid!')];
		$this->messages['group'][0] = ['ERROR', _('Account %s:') . ' sambaSamAccount_group', _('Please enter a valid group name!')];
		$this->messages['profileCanMustChange'][0] = ['ERROR', _('The value for the Samba 3 field "User can/must change password" needs to be a number.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return in_array($this->get_scope(), ['user', 'host']);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'samba.svg';
		// alias name
		$return["alias"] = _('Samba 3');
		// RDN attribute
		$return["RDN"] = ["sambaSID" => "low"];
		// module dependencies
		$return['dependencies'] = ['depends' => ['posixAccount'], 'conflicts' => []];
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=sambaSamAccount)"];
		// managed object classes
		$return['objectClasses'] = ['sambaSamAccount'];
		// managed attributes
		$return['attributes'] = ['uid', 'sambaSID', 'sambaNTPassword', 'sambaPwdLastSet',
			'sambaLogonTime', 'sambaLogoffTime', 'sambaKickoffTime', 'sambaacctflags',
			'sambaPwdLastSet', 'displayName', 'sambaHomePath', 'sambaHomeDrive', 'sambaLogonScript', 'sambaProfilePath',
			'sambaUserWorkstations', 'sambaPrimaryGroupSID', 'sambaDomainName', 'sambaLogonHours', 'sambaMungedDial',
			'sambaPasswordHistory', 'sambaPwdCanChange', 'sambaPwdMustChange']; // sambaPwdCanChange/sambaPwdMustChange only for extension removal
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('sambaSamAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['sambaSamAccount_smbhome'] = [
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['homePath'][0]];
		$return['profile_checks']['sambaSamAccount_profilePath'] = [
			'type' => 'ext_preg',
			'regex' => 'UNC',
			'error_message' => $this->messages['profilePath'][0]];
		$return['profile_checks']['sambaSamAccount_logonScript'] = [
			'type' => 'ext_preg',
			'regex' => 'logonscript',
			'error_message' => $this->messages['logonScript'][0]];
		$return['profile_checks']['sambaSamAccount_userWorkstations'] = [
			'type' => 'ext_preg',
			'regex' => 'unixhost',
			'error_message' => $this->messages['workstations'][0]];
		$return['profile_checks']['sambaSamAccount_logonHours'] = [
			'type' => 'ext_preg',
			'regex' => 'sambaLogonHours',
			'error_message' => $this->messages['logonHours'][0]];
		// profile mappings
		$return['profile_mappings'] = [
			'sambaSamAccount_sambaDomainName' => 'sambaDomainName',
			'sambaSamAccount_displayName' => 'displayName',
		];
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
		$return['PDF_fields'] = [
			'displayName' => _('Display name'),
			'sambaKickoffTime' => _('Account expiration date'),
			'sambaDomainName' => _('Domain'),
			'sambaPrimaryGroupSID' => _('Windows group')
		];
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
		$return['selfServiceFieldSettings'] = [
			'syncNTPassword' => _('Sync Samba NT password with Unix password'),
			'syncSambaPwdLastSet' => _('Update attribute "sambaPwdLastSet" on password change'),
			'password' => _('Password'),
			'sambaPwdLastSet' => _('Last password change (read-only)'),
		];
		// self-service: fields that cannot be relabeled
		$return['selfServiceNoRelabelFields'] = ['syncNTPassword', 'syncSambaPwdLastSet'];
		// help Entries
		$return['help'] = [
			"displayName" => [
				"Headline" => _("Display name"), 'attr' => 'displayName',
				"Text" => _("This is the account's full name on Windows systems.")],
			"password" => [
				"Headline" => _("Samba password"),
				"Text" => _("This is the account's Windows password.")],
			"resetPassword" => [
				"Headline" => _("Reset password"),
				"Text" => _("This will reset the host's password to a default value.")],
			"pwdUnix" => [
				"Headline" => _("Use Unix password"),
				"Text" => _("If checked Unix password will also be used as Samba password.")],
			"pwdUnixUpload" => [
				"Headline" => _("Use Unix password"),
				"Text" => _("If set to \"true\" Unix password will also be used as Samba password.")],
			"noPassword" => [
				"Headline" => _("Use no password"),
				"Text" => _("If checked no password will be used.")],
			"noPasswordUpload" => [
				"Headline" => _("Use no password"),
				"Text" => _("If set to \"true\" no password will be used.")],
			"noExpire" => [
				"Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire. (Setting X-Flag)")],
			"noExpireUpload" => [
				"Headline" => _("Password does not expire"),
				"Text" => _("If set to \"true\" password does not expire. (Setting X-Flag)")],
			"deactivated" => [
				"Headline" => _("Account is deactivated"),
				"Text" => _("If checked then the account will be deactivated. (Setting D-Flag)")],
			"locked" => [
				"Headline" => _("Account is locked"),
				"Text" => _("If checked then the account will be locked (setting L-Flag). You usually want to use this setting to unlock user accounts which were locked because of failed login attempts.")],
			"deactivatedUpload" => [
				"Headline" => _("Account is deactivated"),
				"Text" => _("If set to \"true\" account will be deactivated. (Setting D-Flag)")],
			"passwordIsExpired" => [
				"Headline" => _("Password change at next login"),
				"Text" => _("If you set this option then the user has to change his password at the next login.")],
			"pwdCanChange" => [
				"Headline" => _("User can change password"),
				"Text" => _("Date after the user is able to change his password.")],
			"pwdMustChange" => ["Headline" => _("User must change password"),
				"Text" => _("Date after the user must change his password.")],
			"homeDrive" => [
				"Headline" => _("Home drive"), 'attr' => 'sambaHomeDrive',
				"Text" => _("The home directory will be connected under this drive letter.")],
			"homePath" => [
				"Headline" => _("Home path"), 'attr' => 'sambaHomePath',
				"Text" => _('UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user and group name.') . ' ' . _("Can be left empty.")],
			"profilePath" => [
				"Headline" => _("Profile path"), 'attr' => 'sambaProfilePath',
				"Text" => _('Path of the user profile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user and group name.') . ' ' . _("Can be left empty.")],
			"scriptPath" => [
				"Headline" => _("Logon script"), 'attr' => 'sambaLogonScript',
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user and group name.') . ' ' . _("Can be left empty.")],
			"userWorkstations" => [
				"Headline" => _("Samba workstations"), 'attr' => 'sambaUserWorkstations',
				"Text" => _("List of Samba workstations the user is allowed to login. Empty means every workstation.")],
			"workstations" => [
				"Headline" => _("Samba workstations"), 'attr' => 'sambaUserWorkstations',
				"Text" => _("Comma separated list of Samba workstations the user is allowed to login. Empty means every workstation.") . ' ' . _("Can be left empty.")],
			"group" => [
				"Headline" => _("Windows primary group"), 'attr' => 'sambaPrimaryGroupSID',
				"Text" => _("This is the user's primary Windows group.")],
			"groupUpload" => [
				"Headline" => _("Windows primary group SID"), 'attr' => 'sambaPrimaryGroupSID',
				"Text" => _("This is the SID of the user's primary Windows group.")],
			"specialUser" => [
				"Headline" => _("Special user"),
				"Text" => _("This allows you to define this account as a special user like administrator or guest.")],
			"ridUpload" => [
				"Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your Windows account. You can either enter a number or one of these special accounts: ") .
					implode(", ", array_keys($this->userRids)) . "<br><br>" . _("If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")],
			"ridUploadHost" => [
				"Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your host account. If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")],
			"domain" => [
				"Headline" => _("Domain"), 'attr' => 'sambaDomainName',
				"Text" => _("Windows domain name of account.")],
			"logonHours" => [
				"Headline" => _("Logon hours"), 'attr' => 'sambaLogonHours',
				"Text" => _("This option defines the allowed logon hours for this account.")],
			"logonHoursUpload" => [
				"Headline" => _("Logon hours"), 'attr' => 'sambaLogonHours',
				"Text" => _("This option defines the allowed logon hours for this account. The format is the same as for the LDAP attribute. The 24*7 hours are represented as 168 bit which are saved as 21 hex (21*8 = 168) values. The first bit represents Sunday 0:00 - 0:59 in GMT.")],
			'expireDate' => [
				"Headline" => _("Account expiration date"), 'attr' => 'sambaKickoffTime',
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")],
			'timeZone' => [
				"Headline" => _("Time zone"),
				"Text" => _("This is the time zone of your Samba server. LAM needs this information to display the logon hours correctly.")],
			'tsAllowLogin' => [
				"Headline" => _("Allow terminal server login"),
				"Text" => _("Activate this checkbox to allow this user to use the terminal service.")],
			'tsHomeDir' => [
				"Headline" => _("Home directory"),
				"Text" => _("This is the path to the user's home directory.")],
			'tsProfilePath' => [
				"Headline" => _("Profile path"),
				"Text" => _("Path of the user profile.")],
			'tsInherit' => [
				"Headline" => _("Inherit client startup configuration"),
				"Text" => _("Activate this checkbox to inherit the initial program and working directory from the client machine.")],
			'tsInitialProgram' => [
				"Headline" => _("Initial program"),
				"Text" => _("This program is run after the login.")],
			'tsWorkDirectory' => [
				"Headline" => _("Working directory"),
				"Text" => _("Working directory of initial program.")],
			'tsTimeLimit' => [
				"Headline" => _("Time limit"),
				"Text" => _("Please enter the time limit in minutes. 0 means unlimited.")],
			'tsConnectDrives' => [
				"Headline" => _("Connect client drives"),
				"Text" => _("Activate this checkbox to connect drives from the client machine.")],
			'tsConnectPrinters' => [
				"Headline" => _("Connect client printers"),
				"Text" => _("Activate this checkbox to connect printers from the client machine.")],
			'tsClientPrinterDefault' => [
				"Headline" => _("Client printer is default"),
				"Text" => _("Activate this checkbox to set the client's printer as default printer.")],
			'tsShadowing' => [
				"Headline" => _("Shadowing"),
				"Text" => _("Here you can specify the shadowing mode.")],
			'tsBrokenConn' => [
				"Headline" => _("On broken or timed out connection"),
				"Text" => _("This specifies what to do when the client connection is broken.")],
			'tsReconnect' => [
				"Headline" => _("Reconnect if disconnected"),
				"Text" => _("This specifies the reconnect policy.")],
			'terminalServer' => [
				"Headline" => _("Terminal server options"),
				"Text" => _("Here you can change the settings for the terminal server access.")],
			'sambaPwdLastSet' => [
				"Headline" => _("Last password change"), 'attr' => 'sambaPwdLastSet',
				"Text" => _("This is the date when the user changed his password.")],
			'hiddenOptions' => [
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")],
			'domainSuffix' => [
				"Headline" => _("Domain suffix"),
				"Text" => _("Please enter the LDAP suffix where your Samba domain entries are stored.")],
			'history' => [
				"Headline" => _("Password history"),
				"Text" => _("Enables password history. Depending on your LDAP server you need to select the right server-side ordering (switch ordering here if old passwords are not removed from history).")],
		];
		// upload dependencies
		$return['upload_preDepends'] = ['posixAccount', 'inetOrgPerson'];
		// upload options
		if ($this->get_scope() == "user") {
			$return['upload_columns'] = [
				[
					'name' => 'sambaSamAccount_domain',
					'description' => _('Domain'),
					'required' => true,
					'help' => 'domain',
					'example' => _('mydomain')
				],
				[
					'name' => 'sambaSamAccount_displayName',
					'description' => _('Display name'),
					'help' => 'displayName',
					'example' => _('Steve Miller')
				],
				[
					'name' => 'sambaSamAccount_password',
					'description' => _('Password'),
					'help' => 'password',
					'example' => _('secret')
				],
				[
					'name' => 'sambaSamAccount_pwdUnix',
					'description' => _('Use Unix password'),
					'help' => 'pwdUnixUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				],
				[
					'name' => 'sambaSamAccount_noPassword',
					'description' => _('Use no password'),
					'help' => 'noPasswordUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				],
				[
					'name' => 'sambaSamAccount_noExpire',
					'description' => _('Password does not expire'),
					'help' => 'noExpireUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				],
				[
					'name' => 'sambaSamAccount_deactivated',
					'description' => _('Account is deactivated'),
					'help' => 'deactivatedUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				],
				[
					'name' => 'sambaSamAccount_expireDate',
					'description' => _('Account expiration date'),
					'help' => 'expireDate',
					'default' => '31-12-2030',
					'example' => '15-10-2020'
				],
				[
					'name' => 'sambaSamAccount_group',
					'description' => _('Windows group'),
					'help' => 'groupUpload',
					'example' => _('mygroup'),
					'default' => 'Domain Users'
				],
				[
					'name' => 'sambaSamAccount_rid',
					'description' => _('Samba RID'),
					'help' => 'ridUpload',
					'example' => '1235',
					'default' => '{uidNumber}*2 + {sambaAlgorithmicRidBase}'
				],
			];
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_homeDrive',
					'description' => _('Home drive'),
					'help' => 'homeDrive',
					'example' => 'k:'
				];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_homePath',
					'description' => _('Home path'),
					'help' => 'homePath',
					'example' => _('\\\\server\\homes\\smiller')
				];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_profilePath',
					'description' => _('Profile path'),
					'help' => 'profilePath',
					'example' => _('\\\\server\\profiles\\smiller')
				];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_logonScript',
					'description' => _('Logon script'),
					'help' => 'scriptPath',
					'example' => 'logon.bat'
				];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_workstations',
					'description' => _('Samba workstations'),
					'help' => 'workstations',
					'example' => 'PC01,PC02,PC03'
				];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
				$return['upload_columns'][] = [
					'name' => 'sambaSamAccount_logonHours',
					'description' => _('Logon hours'),
					'help' => 'logonHoursUpload',
					'example' => 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF'
				];
			}
		}
		elseif ($this->get_scope() == "host") {
			$return['upload_columns'] = [
				[
					'name' => 'sambaSamAccount_domain',
					'description' => _('Domain'),
					'required' => true,
					'help' => 'domain',
					'example' => _('mydomain')
				],
				[
					'name' => 'sambaSamAccount_rid',
					'description' => _('Samba RID'),
					'help' => 'ridUploadHost',
					'example' => '1235',
					'default' => '{uidNumber}*2 + {sambaAlgorithmicRidBase}'
				]
			];
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		parent::loadAttributesFromAccountCopy($ldapAttributes, array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY));
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_ready(): bool {
		$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		return isset($attrs['gidNumber'][0]) && isset($attrs['uidNumber'][0]) && isset($attrs['uid'][0]);
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		if (!$this->isExtensionEnabled()) {
			return true;
		}
		if ($this->get_scope() == "host") {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (!str_ends_with($attrs['uid'][0], '$')) {
				return false;
			}
		}
		return isset($this->attributes['sambaSID']) && ($this->attributes['sambaSID'] != '');
	}


	/**
	 * {@inheritDoc}
	 */
	public function load_attributes(array $attributes): void {
		parent::load_attributes($attributes);
		if (isset($this->attributes['sambaacctflags'][0])) {
			$this->nopwd = str_contains($this->attributes['sambaacctflags'][0], "N");
			$this->noexpire = str_contains($this->attributes['sambaacctflags'][0], "X");
		}
		if (isset($this->attributes['sambaPwdLastSet'][0]) && ($this->attributes['sambaPwdLastSet'][0] === '0')) {
			$this->expirePassword = true;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('sambaSamAccount', $this->attributes['objectClass']) && !in_array('sambaSamAccount', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		if ($this->isExtensionEnabled()) {
			if ($this->expirePassword) {
				$this->attributes['sambaPwdLastSet'][0] = '0';
			}
			elseif ((isset($this->attributes['sambaPwdLastSet'][0])) && ($this->attributes['sambaPwdLastSet'][0] == '0')) {
				$this->attributes['sambaPwdLastSet'][0] = (string) time();
			}
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritdoc}
	 */
	public function getWildcardTargetAttributeNames(): array {
		return ['displayName', 'sambaHomePath', 'sambaLogonScript', 'sambaProfilePath'];
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		// add extension
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'sambaSamAccount';
			return [];
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['sambaSamAccount'], $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < count($attrKeys); $k++) {
				if (strpos($attrKeys[$k], 'samba') > -1) {
					unset($this->attributes[$attrKeys[$k]]);
				}
			}
			if (isset($this->attributes['displayName'])) {
				unset($this->attributes['displayName']);
			}
			return [];
		}
		// skip processing if extension is not active
		if (!$this->isExtensionEnabled()) {
			return [];
		}
		$this->getAccountContainer()->replaceWildcardsInPOST($this->getWildcardTargetAttributeNames());
		$errors = [];
		$sambaDomains = $this->getDomains();
		if (count($sambaDomains) === 0) {
			return [];
		}
		$unixAttributes = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		// Save attributes
		$this->attributes['sambaDomainName'][0] = $_POST['sambaDomainName'];
		// Get Domain SID from name
		$SID = '';
		$RIDbase = 1000;
		for ($i = 0; $i < count($sambaDomains); $i++) {
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
		$flag .= "]";
		$this->attributes['sambaacctflags'][0] = $flag;

		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (($this->attributes['displayName'][0] != '') && !get_preg($this->attributes['displayName'][0], 'realname')) {
			$errors[] = $this->messages['displayName'][1];
		}

		// host attributes
		if ($this->get_scope() == 'host') {
			$this->attributes['sambaPrimaryGroupSID'][0] = $SID . "-" . $this->groupRids[_('Domain computers')];
			if (isset($_POST['ResetSambaPassword']) || !isset($this->attributes['sambaNTPassword'][0])) {
				$hostname = $unixAttributes['uid'][0];
				$hostname = substr($hostname, 0, strlen($hostname) - 1);
				$this->attributes['sambaNTPassword'][0] = ntPassword($hostname);
				$this->attributes['sambaPwdLastSet'][0] = time();
			}
		}
		// user attributes
		if ($this->get_scope() == 'user') {
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				$this->attributes['sambaHomePath'][0] = $_POST['sambaHomePath'];
				if ($this->attributes['sambaHomePath'][0] != $_POST['sambaHomePath']) {
					$errors[] = $this->messages['homePath'][1];
				}
				if ((!$this->attributes['sambaHomePath'][0] == '') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC'))) {
					$errors[] = $this->messages['homePath'][0];
				}
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				$this->attributes['sambaHomeDrive'][0] = ($_POST['sambaHomeDrive'] === "-") ? '' : $_POST['sambaHomeDrive'];
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				$this->attributes['sambaLogonScript'][0] = $_POST['sambaLogonScript'];
				if ($this->attributes['sambaLogonScript'][0] != $_POST['sambaLogonScript']) {
					$errors[] = $this->messages['logonScript'][1];
				}
				if ((!$this->attributes['sambaLogonScript'][0] == '') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript'))) {
					$errors[] = $this->messages['logonScript'][0];
				}
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				$this->attributes['sambaProfilePath'][0] = $_POST['sambaProfilePath'];
				if ($this->attributes['sambaProfilePath'][0] != $_POST['sambaProfilePath']) {
					$errors[] = $this->messages['profilePath'][1];
				}
				if (($this->attributes['sambaProfilePath'][0] != '') &&
					!(get_preg($this->attributes['sambaProfilePath'][0], 'UNC') xor get_preg($this->attributes['sambaProfilePath'][0], 'homeDirectory'))) {
					$errors[] = $this->messages['profilePath'][0];
				}
			}
			$rids = array_keys($this->groupRids);
			$wrid = false;
			for ($i = 0; $i < count($rids); $i++) {
				if ($_POST['sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID . "-" . $this->groupRids[$rids[$i]];
				}
			}
			if (!$wrid) {
				$gidnumber = $unixAttributes['gidNumber'][0];
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
			elseif ($_POST['sambaSID'] == "-") {
				$rid = '';
				if (isset($this->attributes['sambaSID'][0])) {
					$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
				}
				// change only if not yet set, previously set to special SID or domain changed
				if (!isset($this->attributes['sambaSID'][0])
					|| in_array($rid, $this->userRids)
					|| (!str_contains($this->attributes['sambaSID'][0], $SID))) {
					$this->attributes['sambaSID'][0] = $SID . "-" . ((intval($unixAttributes['uidNumber'][0]) * 2) + $RIDbase);
				}
			}
		}
		elseif (!isset($this->attributes['sambaSID'][0])) {
			// host
			$this->attributes['sambaSID'][0] = $SID . "-" . ((intval($unixAttributes['uidNumber'][0]) * 2) + $RIDbase);
		}
		$this->expirePassword = isset($_POST['forcePasswordChangeOption']);
		return $errors;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return string[][] list of info/error messages
	 */
	public function process_sambaUserWorkstations(): array {
		// Load attributes
		if ($this->get_scope() == 'user') {
			if (isset($_POST['workstations_2']) && isset($_POST['workstations_left'])) { // Add workstations to list
				$workstations = [];
				if (isset($this->attributes['sambaUserWorkstations'][0])) {
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode(',', $temp);
					for ($i = 0; $i < count($workstations); $i++) {
						if ($workstations[$i] === '') {
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
				for ($i = 1; $i < count($workstations); $i++) {
					$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
				}
			}
			elseif (isset($_POST['workstations_1']) && isset($_POST['workstations_right'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode(',', $temp);
				for ($i = 0; $i < count($workstations); $i++) {
					if ($workstations[$i] === '') {
						unset($workstations[$i]);
					}
				}
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($_POST['workstations_1'], $workstations);
				// Recreate workstation string
				unset($this->attributes['sambaUserWorkstations'][0]);
				if (count($workstations) > 0) {
					$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
					for ($i = 1; $i < count($workstations); $i++) {
						$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
					}
				}
			}
		}
		return [];
	}

	/**
	 * Processes user input of the logon hours page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return string[][] list of info/error messages
	 */
	public function process_logonHours(): array {
		if (isset($_POST['form_subpage_sambaSamAccount_attributes_abort'])) {
			return [];
		}
		// set new logon hours
		$logonHours = '';
		for ($i = 0; $i < 24 * 7; $i++) {
			$logonHours .= isset($_POST['lh_' . $i]) ? '1' : '0';
		}
		// reconstruct HEX string
		$bitstring2hex = array_flip($this->hex2bitstring);
		$logonHoursNew = '';
		for ($i = 0; $i < 21; $i++) {
			$part = strrev(substr($logonHours, $i * 8, 8));
			$byte['hi'] = substr($part, 0, 4);
			$byte['low'] = substr($part, 4, 4);
			$hex = $bitstring2hex[$byte['hi']] . $bitstring2hex[$byte['low']];
			$logonHoursNew .= $hex;
		}
		$this->attributes['sambaLogonHours'][0] = $logonHoursNew;
		return [];
	}

	/**
	 * Processes user input of the time selection page.
	 *
	 * @return string[][] list of info/error messages
	 */
	public function process_time(): array {
		$return = [];
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < count($postKeys); $i++) {
			if (str_contains($postKeys[$i], 'form_subpage_sambaSamAccount_attributes_')) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (str_contains($buttonName, '_back'))) {
			return [];
		}
		// get attribute name
		$attr = '';
		if (str_contains($buttonName, 'sambaKickoffTime')) {
			$attr = 'sambaKickoffTime';
		}
		if ($attr === '') {
			return [];
		}
		// determine action
		if (str_contains($buttonName, '_change')) {
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
		elseif (str_contains($buttonName, '_del')) {
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
	 * @return string[][] list of info/error messages
	 */
	public function process_terminalServer(): array {
		if (isset($_POST['form_subpage_sambaSamAccount_attributes_abort'])) {
			return [];
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
		return [];
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		if ($this->isExtensionEnabled()) {
			$this->getAccountContainer()->replaceWildcardsInArray($this->getWildcardTargetAttributeNames(), $this->attributes);
			$unixAttributes = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (($this->get_scope() == "host") && !str_ends_with($unixAttributes['uid'][0], '$')) {
				$return->add(new htmlStatusMessage("ERROR", _('Host name must end with $!'), _('Please check your settings on the Unix page!')));
			}
			$personalAttributes = [];
			if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				$personalAttributes = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			}
			// Get Domain SID from user SID
			$sambaDomains = $this->getDomains();
			if (count($sambaDomains) === 0) {
				$return->add(new htmlStatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.')));
				return $return;
			}
			if (isset($this->attributes['sambaSID'][0]) && $this->attributes['sambaSID'][0] != '') {
				$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
			}
			$sel_domain = null;
			$SID = '';
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
				if (isset($domainSID)) {
					if ($domainSID == $sambaDomains[$i]->SID) {
						$SID = $domainSID;
						$sel_domain = $sambaDomains[$i]->name;
					}
				}
				elseif (isset($this->attributes['sambaDomainName'][0]) && ($this->attributes['sambaDomainName'][0] != '')) {
					if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
						$SID = $sambaDomains[$i]->SID;
						$sel_domain = $sambaDomains[$i]->name;
					}
				}
			}
			// display name
			$displayName = '';
			if (!empty($this->attributes['displayName'][0])) {
				$displayName = $this->attributes['displayName'][0];
			}
			elseif ($this->getAccountContainer()->isNewAccount && empty($this->attributes['displayName'][0])) {
				if (isset($personalAttributes['givenName'][0]) && $personalAttributes['givenName'][0] && isset($personalAttributes['sn'][0]) && $personalAttributes['sn'][0]) {
					$displayName = $personalAttributes['givenName'][0] . " " . $personalAttributes['sn'][0];
				}
				elseif (isset($personalAttributes['sn'][0])) {
					$displayName = $personalAttributes['sn'][0];
				}
			}
			$return->add(new htmlResponsiveInputField(_('Display name'), 'displayName', $displayName, 'displayName'));
			if ($this->get_scope() == 'user') {
				// user account
				$return->add(new htmlHiddenInput('sambaAcctFlagsU', 'true'));
				// no password
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsN', $this->nopwd, _('Use no password'), 'noPassword'));
				// no password expiry
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsX', $this->noexpire, _('Password does not expire'), 'noExpire'));
				// account deactivated
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsD', $this->isDeactivated(), _('Account is deactivated'), 'deactivated'));
				// account locked
				$return->add(new htmlResponsiveInputCheckbox('sambaAcctFlagsL', $this->isLocked(), _('Account is locked'), 'locked'));
				// password change at next login
				$return->add(new htmlResponsiveInputCheckbox('forcePasswordChangeOption', $this->expirePassword, _('Password change at next login'), 'passwordIsExpired'));
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
				$expireDateGroup->addElement(new htmlAccountPageButton(static::class, 'time', 'sambaKickoffTime', _('Change')));
				$expireDateGroup->addElement(new htmlSpacer('0.5rem', null));
				$expireDateGroup->addElement(new htmlHelpLink('expireDate'));
				$return->addField($expireDateGroup);
				// home drive
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
					$drives = ['-'];
					for ($i = 90; $i > 67; $i--) {
						$drives[] = chr($i) . ':';
					}
					$selected = isset($this->attributes['sambaHomeDrive'][0]) ? [$this->attributes['sambaHomeDrive'][0]] : ['-'];
					$return->add(new htmlResponsiveSelect('sambaHomeDrive', $drives, $selected, _('Home drive'), 'homeDrive'));
				}
				// home path
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
					$sambaHomePath = '';
					if (isset($this->attributes['sambaHomePath'][0])) {
						$sambaHomePath = $this->attributes['sambaHomePath'][0];
					}
					$return->add(new htmlResponsiveInputField(_('Home path'), 'sambaHomePath', $sambaHomePath, 'homePath'));
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
					if (($_SESSION['config']->get_scriptPath() != null)
						&& ($_SESSION['config']->get_scriptPath() != '')
						&& get_preg($sambaProfilePath, 'homeDirectory')) {
						$sambaProfilePathButton = new htmlAccountPageButton(static::class, 'profilePath', 'manage', '../graphics/folder.svg', true);
						$sambaProfilePathButton->setTitle(_('Manage profile directory'));
						$sambaProfilePathGroup->addElement($sambaProfilePathButton);
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
					$return->add(new htmlResponsiveInputField(_('Logon script'), 'sambaLogonScript', $sambaLogonScript, 'scriptPath'));
				}
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
					$return->addLabel(new htmlOutputText(_('Samba workstations')));
					$userWorkstationsGroup = new htmlGroup();
					$userWorkstationsGroup->addElement(new htmlAccountPageButton(static::class, 'sambaUserWorkstations', 'open', _('Edit workstations')));
					$userWorkstationsGroup->addElement(new htmlSpacer('0.5rem', null));
					$userWorkstationsGroup->addElement(new htmlHelpLink('userWorkstations'));
					$return->addField($userWorkstationsGroup);
				}
				// Windows group
				$names = array_keys($this->groupRids);
				$wrid = false;
				$options = [];
				$selected = [];
				for ($i = 0; $i < count($names); $i++) {
					if (isset($this->attributes['sambaPrimaryGroupSID'][0]) && ($this->attributes['sambaPrimaryGroupSID'][0] == $SID . "-" . $this->groupRids[$names[$i]])) {
						$selected[] = $names[$i];
						$wrid = true;
					}
					$options[] = $names[$i];
				}
				$options[] = $this->getGroupName($unixAttributes['gidNumber'][0]);
				if (!$wrid) {
					$selected[] = $this->getGroupName($unixAttributes['gidNumber'][0]);
				}
				$return->add(new htmlResponsiveSelect('sambaPrimaryGroupSID', $options, $selected, _('Windows group'), 'group'));
				// display if group SID should be mapped to a well known SID
				$options = array_keys($this->userRids);
				$options[] = '-';
				$selected = [];
				if (isset($this->attributes['sambaSID'][0]) && ($this->attributes['sambaSID'][0] != '')) {
					$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
					$specialRids = array_flip($this->userRids);
					$selected = in_array($rid, $this->userRids) ? [$specialRids[$rid]] : ['-'];
				}
				else {
					$selected[] = "-";
				}
				$return->add(new htmlResponsiveSelect('sambaSID', $options, $selected, _('Special user'), 'specialUser'));
			}
			// domain
			$return->add(new htmlResponsiveSelect('sambaDomainName', $sambaDomainNames, [$sel_domain], _('Domain'), 'domain'));
			// logon hours and terminal server options
			if ($this->get_scope() == 'user') {
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
					$return->addLabel(new htmlOutputText(_('Logon hours')));
					$logonHoursGroup = new htmlGroup();
					$logonHoursGroup->addElement(new htmlAccountPageButton(static::class, 'logonHours', 'open', _('Edit')));
					$logonHoursGroup->addElement(new htmlSpacer('0.5rem', null));
					$logonHoursGroup->addElement(new htmlHelpLink('logonHours'));
					$return->addField($logonHoursGroup);
				}
				if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideTerminalServer')) {
					$return->addLabel(new htmlOutputText(_('Terminal server options')));
					$terminalServerGroup = new htmlGroup();
					$terminalServerGroup->addElement(new htmlAccountPageButton(static::class, 'terminalServer', 'open', _('Edit')));
					$terminalServerGroup->addElement(new htmlSpacer('0.5rem', null));
					$terminalServerGroup->addElement(new htmlHelpLink('terminalServer'));
					$return->addField($terminalServerGroup);
				}
			}
			// reset host password
			if ($this->get_scope() == 'host') {
				// host account
				$return->add(new htmlHiddenInput('sambaAcctFlagsW', 'true'));
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
			$remButton->setCSSClasses(['lam-danger']);
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add Samba 3 extension')));
		}
		return $return;
	}

	/**
	 * Returns the account expiration date in printable form.
	 *
	 * @return string expiration date
	 */
	private function formatAccountExpirationDate() {
		$dateValue = "-";
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
	public function display_html_sambaUserWorkstations() {
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
			if (isset($this->attributes['sambaUserWorkstations'][0])) {
				$wsAttr = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$userWorkstations = explode(',', $wsAttr);
			}
			$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);

			$return->add(new htmlSubTitle(_("Allowed workstations")));

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

			$backButton = new htmlAccountPageButton(static::class, 'attributes', 'back', _('Back'));
			$return->add($backButton);
		}
		return $return;
	}

	/**
	 * This function will create the HTML page to edit logon hours.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_logonHours() {
		$return = new htmlResponsiveRow();
		$timeZone = getTimeZoneOffsetHours();
		$titles = [_('Time'), _('Sunday'), _('Monday'), _('Tuesday'), _('Wednesday'), _('Thursday'),
			_('Friday'), _('Saturday')];
		$data = [];
		if (!isset($this->attributes['sambaLogonHours'][0]) || ($this->attributes['sambaLogonHours'][0] == '')) {
			$this->attributes['sambaLogonHours'][0] = 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF';
		}
		// convert existing logonHours string to bit array
		$logonHours = $this->attributes['sambaLogonHours'][0];
		$temp = [];
		for ($i = 0; $i < strlen($logonHours); $i++) {
			$temp[] = $this->hex2bitstring[$logonHours[$i]];
		}
		$logonHoursRev = implode('', $temp);
		// reverse bits low to high (1 is 0:00 sunday, 2 is 1:00 sunday, etc)
		$logonHours = "";
		for ($i = 0; $i < 21; $i++) {
			$logonHours .= strrev(substr($logonHoursRev, $i * 8, 8));
		}
		$hour = [];
		for ($i = 0; $i < 24 * 7; $i++) {
			$hour[$i] = substr($logonHours, $i, 1);
		}
		// display input
		$boxes = [];
		// dynamically place boxes depending on time zone
		for ($i = 0; $i < 24 * 7; $i++) {
			$hr = $i + $timeZone;
			if ($hr < 0) {
				$hr += 24 * 7;
			}
			elseif ($hr >= 24 * 7) {
				$hr -= 24 * 7;
			}
			$checkbox = new htmlInputCheckbox('lh_' . $hr, (bool) $hour[(int) $hr]);
			$boxes[$i % 24][(int) floor($i / 24)] = $checkbox;
		}
		for ($h = 0; $h < 24; $h++) {
			$hour = $h;
			if ($h < 10) {
				$hour = '0' . $h;
			}
			$row = [];
			$row[] = new htmlOutputText("$hour:00 - $hour:59");
			for ($d = 0; $d < 7; $d++) {
				$row[] = $boxes[$h][$d];
			}
			$data[] = $row;
		}
		$return->add(new htmlResponsiveTable($titles, $data));

		$return->addVerticalSpacer('2rem');
		$return->addLabel(new htmlAccountPageButton(static::class, 'attributes', 'submit', _('Ok')));
		$return->addField(new htmlAccountPageButton(static::class, 'attributes', 'abort', _('Cancel')));
		return $return;
	}

	/**
	 * This function will create the meta HTML code to show a page to change time values.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_time() {
		$return = new htmlResponsiveRow();
		$attr = 'sambaKickoffTime';
		$text = _('Account expiration date');
		$help = "expireDate";
		$time = time() + 3600 * 24 * 365;
		if (isset($this->attributes[$attr][0])) {
			$time = $this->attributes[$attr][0];
		}
		$date = new DateTime('@' . $time, new DateTimeZone('UTC'));
		for ($i = 1; $i <= 31; $i++) {
			$mday[] = (string) $i;
		}
		for ($i = 1; $i <= 12; $i++) {
			$mon[] = (string) $i;
		}
		for ($i = 2003; $i <= 2050; $i++) {
			$year[] = (string) $i;
		}
		$return->addLabel(new htmlOutputText($text));
		$dateGroup = new htmlGroup();
		$daySelect = new htmlSelect('expire_day', $mday, [$date->format('j')]);
		$daySelect->setWidth('3rem');
		$dateGroup->addElement($daySelect);
		$monthSelect = new htmlSelect('expire_mon', $mon, [$date->format('n')]);
		$monthSelect->setWidth('3rem');
		$dateGroup->addElement($monthSelect);
		$yearSelect = new htmlSelect('expire_yea', $year, [$date->format('Y')]);
		$yearSelect->setWidth('5rem');
		$dateGroup->addElement($yearSelect);
		$dateGroup->addElement(new htmlHelpLink($help));
		$return->addField($dateGroup);
		if ($this->getAccountContainer()->getAccountModule('shadowAccount') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncShadow', false, _('Set also for Shadow')));
		}
		if ($this->getAccountContainer()->getAccountModule('heimdalKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncHeimdal', false, _('Set also for Kerberos')));
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMIT', false, _('Set also for Kerberos')));
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberosStructural') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMITStructural', false, _('Set also for Kerberos')));
		}
		$return->addVerticalSpacer('2rem');
		$buttons = new htmlGroup();
		$buttons->addElement(new htmlAccountPageButton(static::class, 'attributes', 'change' . $attr, _('Change')));
		$buttons->addElement(new htmlSpacer('0.5rem', null));
		if (isset($this->attributes[$attr][0])) {
			$buttons->addElement(new htmlAccountPageButton(static::class, 'attributes', 'del' . $attr, _('Remove')));
			$buttons->addElement(new htmlSpacer('0.5rem', null));
		}
		$buttons->addElement(new htmlAccountPageButton(static::class, 'attributes', 'back' . $attr, _('Cancel')));
		$return->add($buttons, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * This function will create the HTML page to edit the terminal server options.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_terminalServer() {
		$return = new htmlResponsiveRow();
		$mDial = new sambaMungedDial();
		if (isset($this->attributes['sambaMungedDial'][0])) {
			$mDial->load($this->attributes['sambaMungedDial'][0]);
		}
		// terminal server login
		$return->add(new htmlResponsiveInputCheckbox('tsAllowLogin', $mDial->getTsLogin(), _('Allow terminal server login'), 'tsAllowLogin'));
		// home directory
		$return->add(new htmlResponsiveInputField(_('Home directory'), 'tsHomeDir', $mDial->ctx['CtxWFHomeDir'], 'tsHomeDir'));
		// home drive
		$drives = [];
		for ($i = 90; $i > 67; $i--) {
			$drives[] = chr($i) . ':';
		}
		$selTsDrive = [];
		if (isset($mDial->ctx['CtxWFHomeDirDrive'])) {
			$selTsDrive = [$mDial->ctx['CtxWFHomeDirDrive']];
		}
		$return->add(new htmlResponsiveSelect('tsHomeDrive', $drives, $selTsDrive, _('Home drive'), 'homeDrive'));
		// profile path
		$return->add(new htmlResponsiveInputField(_('Profile path'), 'tsProfilePath', $mDial->ctx['CtxWFProfilePath'], 'tsProfilePath'));
		// use startup program and working dir from client
		$return->add(new htmlResponsiveInputCheckbox('tsInherit', $mDial->getInheritMode(), _('Inherit client startup configuration'), 'tsInherit'));
		// startup program
		$return->add(new htmlResponsiveInputField(_('Initial program'), 'tsInitialProgram', $mDial->ctx['CtxInitialProgram'], 'tsInitialProgram'));
		// working dir
		$return->add(new htmlResponsiveInputField(_('Working directory'), 'tsWorkDirectory', $mDial->ctx['CtxWorkDirectory'], 'tsWorkDirectory'));
		// connection time limit
		$tsConnectionLimit = new htmlResponsiveInputField(_('Connection time limit'), 'tsConnectionLimit', $mDial->ctx['CtxMaxConnectionTime'], 'tsTimeLimit');
		$tsConnectionLimit->setType('number');
		$return->add($tsConnectionLimit);
		// disconnection time limit
		$tsDisconnectionLimit = new htmlResponsiveInputField(_('Disconnection time limit'), 'tsDisconnectionLimit', $mDial->ctx['CtxMaxDisconnectionTime'], 'tsTimeLimit');
		$tsDisconnectionLimit->setType('number');
		$return->add($tsDisconnectionLimit);
		// idle time limit
		$tsIdleLimit = new htmlResponsiveInputField(_('Idle time limit'), 'tsIdleLimit', $mDial->ctx['CtxMaxIdleTime'], 'tsTimeLimit');
		$tsIdleLimit->setType('number');
		$return->add($tsIdleLimit);
		// connect client drives
		$return->add(new htmlResponsiveInputCheckbox('tsConnectDrives', (bool) $mDial->getConnectClientDrives(), _('Connect client drives'), 'tsConnectDrives'));
		// connect client printers
		$return->add(new htmlResponsiveInputCheckbox('tsConnectPrinters', (bool) $mDial->getConnectClientPrinters(), _('Connect client printers'), 'tsConnectPrinters'));
		// client printer is default
		$return->add(new htmlResponsiveInputCheckbox('tsClientPrinterDefault', (bool) $mDial->getDefaultPrinter(), _('Client printer is default'), 'tsClientPrinterDefault'));
		// shadowing
		$shadowOptions = [
			_("disabled") => "0",
			_("input on, notify on") => "1",
			_("input on, notify off") => "2",
			_("input off, notify on") => "3",
			_("input off, notify off") => "4"];
		$selShadow = [$mDial->getShadow()];
		$shadowSelect = new htmlResponsiveSelect('tsShadowing', $shadowOptions, $selShadow, _('Shadowing'), 'tsShadowing');
		$shadowSelect->setHasDescriptiveElements(true);
		$return->add($shadowSelect);
		// broken connection
		$brokenConnOptions = [
			_("disconnect") => "0",
			_("reset") => "1"];
		$selbrokenConn = [$mDial->getBrokenConn()];
		$brokenConnSelect = new htmlResponsiveSelect('tsBrokenConn', $brokenConnOptions, $selbrokenConn, _('On broken or timed out connection'), 'tsBrokenConn');
		$brokenConnSelect->setHasDescriptiveElements(true);
		$return->add($brokenConnSelect);
		// reconnect
		$reconnectOptions = [
			_("from any client") => "0",
			_("from previous client only") => "1"];
		$selReconnect = [$mDial->getReConn()];
		$reconnectSelect = new htmlResponsiveSelect('tsReconnect', $reconnectOptions, $selReconnect, _('Reconnect if disconnected'), 'tsReconnect');
		$reconnectSelect->setHasDescriptiveElements(true);
		$return->add($reconnectSelect);
		// buttons
		$return->addVerticalSpacer('2rem');
		$return->addLabel(new htmlAccountPageButton(static::class, 'attributes', 'submit', _('Ok')));
		$return->addField(new htmlAccountPageButton(static::class, 'attributes', 'abort', _('Cancel')));
		return $return;
	}

	/**
	 * Displays manage profile path page.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_profilePath(): htmlElement {
		$return = new htmlResponsiveRow();
		$return->addLabel(new htmlOutputText(_('Profile path')));
		$return->addField(new htmlOutputText($this->attributes['sambaProfilePath'][0]));
		$return->addVerticalSpacer('2rem');
		// get list of remote servers
		$remoteServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($i = 0; $i < count($remoteServers); $i++) {
			$remoteServer = $remoteServers[$i];
			$label = $remoteServer->getLabel();
			$remote = new Remote();
			try {
				$remote->connect($remoteServer);
			}
			catch (LAMException $e) {
				$return->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
				continue;
			}
			$result = $remote->execute(
				implode(
					self::$SPLIT_DELIMITER,
					[
						$this->attributes['uid'][0],
						"home",
						"check",
						$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0]]
				));
			$remote->disconnect();
			// remote command results
			if (!empty($result)) {
				$returnValue = trim($result);
				if ($returnValue === 'ok') {
					$return->addLabel(new htmlOutputText($label));
					$editGroup = new htmlGroup();
					$editGroup->addElement(new htmlImage('../../graphics/pass.svg', '16', '16'));
					$editGroup->addElement(new htmlSpacer('0.5rem', null));
					$editGroup->addElement(new htmlAccountPageButton(static::class, 'homedir', 'delete_' . $i, _('Delete')));
					$return->addField($editGroup);
				}
				elseif ($returnValue === 'missing') {
					$return->addLabel(new htmlOutputText($label));
					$editGroup = new htmlGroup();
					$editGroup->addElement(new htmlImage('../../graphics/del.svg', '16', '16'));
					$editGroup->addElement(new htmlSpacer('0.5rem', null));
					$editGroup->addElement(new htmlAccountPageButton(static::class, 'homedir', 'create_' . $i, _('Create')));
					$return->addField($editGroup);
				}
				elseif (trim($returnValue) !== '') {
					$messageParams = explode(",", $returnValue);
					if (isset($messageParams[2])) {
						$message = new htmlStatusMessage($messageParams[0], htmlspecialchars($messageParams[1]), htmlspecialchars($messageParams[2]));
					}
					elseif (in_array($messageParams[0], ['ERROR', 'WARN', 'INFO'], true)) {
						$message = new htmlStatusMessage($messageParams[0], htmlspecialchars($messageParams[1]));
					}
					else {
						$message = new htmlStatusMessage('WARN', htmlspecialchars($messageParams[0]));
					}
					$return->add($message);
				}
			}
		}
		$return->addVerticalSpacer('2rem');
		$return->add(new htmlAccountPageButton(static::class, 'attributes', 'back', _('Back')), 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * Processes user input of the profile path check page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return string[][] list of info/error messages
	 */
	public function process_profilePath(): array {
		$return = [];
		$unixAttrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		$uidNumber = $unixAttrs['uidNumber'][0];
		$gidNumber = $unixAttrs['gidNumber'][0];
		if (empty($uidNumber) || empty($gidNumber)) {
			return $return;
		}
		// get list of remote servers
		$remoteServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($i = 0; $i < count($remoteServers); $i++) {
			$remoteServer = $remoteServers[$i];
			if (isset($_POST['form_subpage_' . static::class . '_homedir_create_' . $i])) {
				$remote = new Remote();
				try {
					$remote->connect($remoteServer);
				}
				catch (LAMException $e) {
					$return[] = ['ERROR', $e->getTitle(), $e->getMessage()];
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						[
							$this->attributes['uid'][0],
							"directory",
							"add",
							$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0],
							"0" . $_SESSION['config']->get_scriptRights(),
							$uidNumber,
							$gidNumber]
					));
				$remote->disconnect();
				// remote command results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (in_array($singleresult[0], ['ERROR', 'WARN', 'INFO'], true)) {
						$return[] = $singleresult;
					}
				}
			}
			elseif (isset($_POST['form_subpage_' . static::class . '_homedir_delete_' . $i])) {
				$remote = new Remote();
				try {
					$remote->connect($remoteServer);
				}
				catch (LAMException $e) {
					$return[] = ['ERROR', $e->getTitle(), $e->getMessage()];
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						[
							$this->attributes['uid'][0],
							"home",
							"rem",
							$remoteServer->getHomeDirPrefix() . $this->attributes['sambaProfilePath'][0],
							$uidNumber
						]
					));
				$remote->disconnect();
				// remote command results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (in_array($singleresult[0], ['ERROR', 'WARN', 'INFO'], true)) {
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
	public function get_profileOptions(string $typeId): ?htmlResponsiveRow {
		$return = parent::get_profileOptions($typeId);
		if ($this->get_scope() == 'user') {
			// lists for expiration date
			$day = [];
			$mon = [];
			$year = [];
			for ($i = 1; $i <= 31; $i++) {
				$day[] = (string) $i;
			}
			for ($i = 1; $i <= 12; $i++) {
				$mon[] = (string) $i;
			}
			for ($i = 2003; $i <= 2030; $i++) {
				$year[] = (string) $i;
			}
			// display name
			$return->add(new htmlResponsiveInputField(_('Display name'), 'sambaSamAccount_displayName', '', 'displayName'));
			// use no password at all
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsN', false, _('Use no password'), 'noPassword'));
			// account deactivation
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsD', false, _('Account is deactivated'), 'deactivated'));
			// password never expires
			$return->add(new htmlResponsiveInputCheckbox('sambaSamAccount_sambaAcctFlagsX', false, _('Password does not expire'), 'noExpire'));
			// expiration date
			$return->addLabel(new htmlOutputText(_('Account expiration date')));
			$expireContainer = new htmlResponsiveRow();
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_day', $day, ['1']), 2, 2, 2, 'padding-right05');
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_mon', $mon, ['1']), 2, 2, 2, 'padding-left-right05');
			$expireContainer->add(new htmlSelect('sambaSamAccount_expire_yea', $year, ['2030']), 6, 6, 6, 'padding-left-right05');
			$expireContainer->add(new htmlHelpLink('expireDate'), 2, 2, 2, 'padding-left-right05');
			$return->addField($expireContainer);
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomeDrive')) {
				// letter of home drive
				$drives = ['-'];
				for ($i = 90; $i > 67; $i--) {
					$drives[] = chr($i) . ':';
				}
				$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaHomeDrive', $drives, ['-'], _('Home drive'), 'homeDrive'));
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideHomePath')) {
				// path to home directory
				$return->add(new htmlResponsiveInputField(_('Home path'), 'sambaSamAccount_smbhome', '', 'homePath'));
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideProfilePath')) {
				// profile path
				$return->add(new htmlResponsiveInputField(_('Profile path'), 'sambaSamAccount_profilePath', '', 'profilePath'));
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonScript')) {
				// logon script
				$return->add(new htmlResponsiveInputField(_('Logon script'), 'sambaSamAccount_logonScript', '', 'scriptPath'));
			}
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideWorkstations')) {
				// allowed workstations
				$return->add(new htmlResponsiveInputField(_('Samba workstations'), 'sambaSamAccount_userWorkstations', '', 'workstations'));
			}
			// domains
			$sambaDomains = $this->getDomains();
			$sambaDomainNames = [];
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaDomainName', $sambaDomainNames, [], _('Domain'), 'domain'));
			// Windows group
			$groups = [];
			foreach ($this->groupRids as $key => $value) {
				$groups[$key] = $value;
			}
			$groups["-"] = "-";
			$groupSelect = new htmlResponsiveSelect('sambaSamAccount_group', $groups, ['513'], _('Windows group'), 'group');
			$groupSelect->setHasDescriptiveElements(true);
			$return->add($groupSelect);
			if (!$this->isBooleanConfigOptionSet('sambaSamAccount_hideLogonHours')) {
				// logon hours
				$return->add(new htmlResponsiveInputField(_('Logon hours'), 'sambaSamAccount_logonHours', '', 'logonHoursUpload'));
			}
		}
		elseif ($this->get_scope() == 'host') {
			// domains
			$sambaDomains = $this->getDomains();
			$sambaDomainNames = [];
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return->add(new htmlResponsiveSelect('sambaSamAccount_sambaDomainName', $sambaDomainNames, [], _('Domain'), 'domain'));
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['sambaSamAccount_addExt'][0])
			&& ($profile['sambaSamAccount_addExt'][0] == "true")
			&& !in_array('sambaSamAccount', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'sambaSamAccount';
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
		if (isset($this->attributes['sambaSID'][0])
			&& isset($profile['sambaSamAccount_sambaDomainName'][0])
			&& ($profile['sambaSamAccount_sambaDomainName'][0] != "")) {
			$domains = $this->getDomains();
			$domSID = '';
			// find domain SID
			for ($i = 0; $i < count($domains); $i++) {
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
		// primary group
		if (isset($profile['sambaSamAccount_sambaDomainName'][0])) {
			$domains = $this->getDomains();
			$domSID = '';
			// find domain SID
			for ($i = 0; $i < count($domains); $i++) {
				if ($domains[$i]->name == $profile['sambaSamAccount_sambaDomainName'][0]) {
					$domSID = $domains[$i]->SID;
					break;
				}
			}
			// set primary group if selected
			if (($domSID != '')
				&& isset($profile['sambaSamAccount_group'][0])
				&& ($profile['sambaSamAccount_group'][0] != "-")) {
				$this->attributes['sambaPrimaryGroupSID'][0] = $domSID . "-" . $profile['sambaSamAccount_group'][0];
			}
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_configOptions(array $scopes, array $allScopes): htmlElement|array {
		$return = parent::get_configOptions($scopes, $allScopes);
		if (!in_array('user', $scopes)) {
			return $return;
		}
		$configContainer = new htmlResponsiveRow();
		// password history
		$historyOptions = [
			_('yes - ordered ascending') => 'yes_deleteLast',
			_('yes - ordered descending') => 'yes_deleteFirst',
			_('no') => 'no'
		];
		$historySelect = new htmlResponsiveSelect('sambaSamAccount_history', $historyOptions, ['yes_deleteLast'], _("Password history"), 'history');
		$historySelect->setHasDescriptiveElements(true);
		$configContainer->add($historySelect);
		// hidden options
		$configContainer->addVerticalSpacer('1rem');
		$configHiddenLabelGroup = new htmlGroup();
		$configHiddenLabelGroup->addElement(new htmlOutputText(_('Hidden options') . ' '));
		$configHiddenLabelGroup->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainer->add($configHiddenLabelGroup);
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
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
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
			$this->addPDFKeyValue($return, 'tsHomeDir', _('Home directory') . ' (TS)', $mDial->ctx['CtxWFHomeDir']);
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
			$shadowOptions = [
				'0' => _("disabled"),
				'1' => _("input on, notify on"),
				'2' => _("input on, notify off"),
				'3' => _("input off, notify on"),
				'4' => _("input off, notify off")];
			$tsShadowing = '';
			if (($mDial->getShadow() != null) && is_numeric($mDial->getShadow())) {
				$tsShadowing = $shadowOptions[$mDial->getShadow()];
			}
			$this->addPDFKeyValue($return, 'tsShadowing', _('Shadowing') . ' (TS)', $tsShadowing);
			$brokenConnOptions = [
				'0' => _("disconnect"),
				'1' => _("reset")];
			$tsBrokenConn = '';
			if (($mDial->getBrokenConn() != null) && is_numeric($mDial->getBrokenConn())) {
				$tsBrokenConn = $brokenConnOptions[$mDial->getBrokenConn()];
			}
			$this->addPDFKeyValue($return, 'tsBrokenConn', _('On broken or timed out connection') . ' (TS)', $tsBrokenConn);
			$reconnectOptions = [
				'0' => _("from any client"),
				'1' => _("from previous client only")];
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
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$errors = [];
		// get list of Samba 3 domains
		$domains = $this->getDomains();
		// get list of Unix groups and their sambaSID + gidNumber
		$groupList = searchLDAPByFilter('objectClass=posixGroup', ['cn', 'sambaSID', 'gidNumber'], ['group']);
		$groups_cn = [];
		for ($i = 0; $i < count($groupList); $i++) {
			if (isset($groupList[$i]['sambasid'][0])) {
				$groups_cn[$groupList[$i]['cn'][0]]['SID'] = $groupList[$i]['sambasid'][0];
			}
			if (isset($groupList[$i]['gidnumber'][0])) {
				$groups_cn[$groupList[$i]['cn'][0]]['gid'] = $groupList[$i]['gidnumber'][0];
			}
		}
		if ($this->get_scope() == 'user') {
			for ($i = 0; $i < count($rawAccounts); $i++) {
				if (!in_array("sambaSamAccount", $partialAccounts[$i]['objectClass'])) {
					$partialAccounts[$i]['objectClass'][] = "sambaSamAccount";
				}
				// displayName
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_displayName', 'displayName',
					'realname', $this->messages['displayName'][0], $errors);
				// password
				$partialAccounts[$i]['sambaPwdLastSet'] = time();
				if (!get_preg($rawAccounts[$i][$ids['sambaSamAccount_password']], 'password')) {
					$errMsg = $this->messages['sambaNTPassword'][2];
					$errMsg[] = [$i];
					$errors[] = $errMsg;
				}
				// use Unix password
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == "") {  // default: use Unix
					$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']], ['true', 'false'])) {
					if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == 'true') {  // use Unix
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					}
					else {  // use given password
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
					}
				}
				else {
					$errMsg = $this->messages['pwdUnix'][0];
					$errMsg[] = [$i];
					$errors[] = $errMsg;
				}
				// use no password
				if ($rawAccounts[$i][$ids['sambaSamAccount_noPassword']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_noPassword']], ['true', 'false'])) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_noPassword']] == 'true') {
							$partialAccounts[$i]['sambaNTPassword'] = 'NO PASSWORD*****';
						}
					}
					else {
						$errMsg = $this->messages['noPassword'][0];
						$errMsg[] = [$i];
						$errors[] = $errMsg;
					}
				}
				// account flags
				$flag_expire = false;
				$flag_deactivated = false;
				// password does not expire
				if ($rawAccounts[$i][$ids['sambaSamAccount_noExpire']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_noExpire']], ['true', 'false'])) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_noExpire']] == 'false') {
							$flag_expire = true;
						}
					}
					else {
						$errMsg = $this->messages['noExpire'][0];
						$errMsg[] = [$i];
						$errors[] = $errMsg;
					}
				}
				// account is deactivated
				if ($rawAccounts[$i][$ids['sambaSamAccount_deactivated']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_deactivated']], ['true', 'false'])) {
						if ($rawAccounts[$i][$ids['sambaSamAccount_deactivated']] == 'true') {
							$flag_deactivated = true;
						}
					}
					else {
						$errMsg = $this->messages['deactivated'][0];
						$errMsg[] = [$i];
						$errors[] = $errMsg;
					}
				}
				// set flags
				$flags = "[";
				if ($flag_deactivated) {
					$flags .= "D";
				}
				if (!$flag_expire) {
					$flags .= "X";
				}
				$flags .= "U";
				// Expand string to fixed length
				$flags = str_pad($flags, 12);
				// End character
				$flags .= "]";
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
						$errMsg[] = [$i];
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
						$errMsg[] = [$i];
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
				for ($d = 0; $d < count($domains); $d++) {
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
					$errMsg[] = [$i];
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
								(intval($groups_cn[$rawAccounts[$i][$ids['sambaSamAccount_group']]]['gid']) * 2 +
									$domains[$domIndex]->RIDbase + 1);
						}
					}
					elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_group']], array_keys($this->groupRids))) {
						$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->groupRids[$rawAccounts[$i][$ids['sambaSamAccount_group']]];
					}
					else {
						$errMsg = $this->messages['group'][0];
						$errMsg[] = [$i];
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
						$errMsg[] = [$i];
						$errors[] = $errMsg;
					}
				}
				else {
					// default RID uid*2 + RIDBase
					$partialAccounts[$i]['sambaSID'] .= '-' . ($partialAccounts[$i]['uidNumber'] * 2 + $domains[$domIndex]->RIDbase);
				}
				// logon hours
				$partialAccounts[$i]['sambaLogonHours'] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'sambaSamAccount_logonHours', 'sambaLogonHours',
					'sambaLogonHours', $this->messages['logonHours'][1], $errors);
			}
		}
		else {  // hosts
			for ($i = 0; $i < count($rawAccounts); $i++) {
				if (!in_array("sambaSamAccount", $partialAccounts[$i]['objectClass'])) {
					$partialAccounts[$i]['objectClass'][] = "sambaSamAccount";
				}
				// domain
				$domIndex = -1;
				for ($d = 0; $d < count($domains); $d++) {
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
					$errMsg[] = [$i];
					$errors[] = $errMsg;
				}
				// RID
				if ($rawAccounts[$i][$ids['sambaSamAccount_rid']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_rid']], 'digit')) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $rawAccounts[$i][$ids['sambaSamAccount_rid']];
					}
					else {
						$errMsg = $this->messages['rid'][3];
						$errMsg[] = [$i];
						$errors[] = $errMsg;
					}
				}
				else {
					// default RID uid*2 + RIDBase
					$partialAccounts[$i]['sambaSID'] .= '-' . ($partialAccounts[$i]['uidNumber'] * 2 + $domains[$domIndex]->RIDbase);
				}
				// passwords ( = host name)
				$partialAccounts[$i]['sambaPwdLastSet'] = time();
				$partialAccounts[$i]['sambaNTPassword'] = ntPassword(substr($partialAccounts[$i]['uid'], 0, strlen($partialAccounts[$i]['uid']) - 1));
				// flags
				$partialAccounts[$i]['sambaacctflags'] = "[W          ]";
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceSettings(selfServiceProfile $profile): ?htmlElement {
		$selfServiceContainer = new htmlResponsiveRow();
		// domain suffix
		$selfServiceDomainSuffix = new htmlResponsiveInputField(_('Domain suffix'), 'sambaSamAccount_domainSuffix', null, ['domainSuffix', static::class]);
		$selfServiceContainer->add($selfServiceDomainSuffix);
		// password history
		$historyOptions = [
			_('yes - ordered ascending') => 'yes_deleteLast',
			_('yes - ordered descending') => 'yes_deleteFirst',
			_('no') => 'no'
		];
		$historySelect = new htmlResponsiveSelect('sambaSamAccount_history', $historyOptions, ['yes_deleteLast'], _("Password history"), ['history', static::class]);
		$historySelect->setHasDescriptiveElements(true);
		$selfServiceContainer->add($historySelect);
		return $selfServiceContainer;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = [];
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
			$row->add($group);
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
	 * {@inheritDoc}
	 */
	public function checkSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if (!isset($attributes['objectClass']) || !in_array_ignore_case('sambaSamAccount', $attributes['objectClass'])) {
			return $return;
		}
		if (in_array('password', $fields)
			&& isset($_POST['sambaSamAccount_password'])
			&& ($_POST['sambaSamAccount_password'] != '')) {
			if ($_POST['sambaSamAccount_password'] != $_POST['sambaSamAccount_password2']) {
				$return['messages'][] = $this->messages['sambaNTPassword'][0];
			}
			elseif (!get_preg($_POST['sambaSamAccount_password'], 'password')) {
				$return['messages'][] = $this->messages['sambaNTPassword'][1];
			}
			else {
				$userName = empty($attributes['uid'][0]) ? null : $attributes['uid'][0];
				$additionalAttrs = [];
				if (!empty($attributes['sn'][0])) {
					$additionalAttrs[] = $attributes['sn'][0];
				}
				if (!empty($attributes['givenName'][0])) {
					$additionalAttrs[] = $attributes['givenName'][0];
				}
				$pwdPolicyResult = checkPasswordStrength($_POST['sambaSamAccount_password'], $userName, $additionalAttrs);
				if ($pwdPolicyResult->isValid()) {
					$return['mod']['sambaNTPassword'][0] = ntPassword($_POST['sambaSamAccount_password']);
					$return['info']['sambaUserPasswordClearText'][0] = $_POST['sambaSamAccount_password'];
					$this->doSelfServicePasswordHistoryAndMinAge($attributes, $return);
					if (array_key_exists('sambaPwdLastSet', $attributes)) {
						$return['mod']['sambaPwdLastSet'][0] = time();
					}
				}
				else {
					$return['messages'][] = ['ERROR', $pwdPolicyResult->getTextMessage()];
				}
			}
		}
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return $return;
			}
			elseif (!get_preg($_POST['posixAccount_password'], 'password')) {
				return $return;
			}
			else {
				$setPassword = false;
				// sync password
				if (in_array('syncNTPassword', $fields)) {
					$return['mod']['sambaNTPassword'][0] = ntPassword($_POST['posixAccount_password']);
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
		return $return;
	}

	/**
	 * Checks password history and password minimum age and updates history.
	 *
	 * @param array<string, string[]|string> $attributes LDAP attributes of current account
	 * @param array<string, mixed> $return return object of checkSelfServiceOptions()
	 */
	private function doSelfServicePasswordHistoryAndMinAge($attributes, &$return): void {
		if (!empty($this->selfServiceSettings->moduleSettings['sambaSamAccount_domainSuffix'][0])) {
			$sambaDomain = $this->getUserDomain($attributes, $_SESSION['ldapHandle']->getServer(), $this->selfServiceSettings->moduleSettings['sambaSamAccount_domainSuffix'][0]);
			if ($sambaDomain == null) {
				return;
			}
			if (!empty($sambaDomain->pwdHistoryLength)
				&& ($sambaDomain->pwdHistoryLength > 0)) {
				if (sambaSamAccount::oldPasswordUsed($return['info']['sambaUserPasswordClearText'][0], $attributes, $sambaDomain)) {
					$return['messages'][] = ['ERROR', _('You are reusing an old password. Please choose a different password.')];
				}
				elseif (sambaSamAccount::isPasswordHistoryEnabled($this->selfServiceSettings->moduleSettings)) {
					// update password history
					$sambaPasswordHistory = empty($attributes['sambaPasswordHistory']) ? [] : $attributes['sambaPasswordHistory'];
					while (count($sambaPasswordHistory) > ($sambaDomain->pwdHistoryLength - 1)) {
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
			// check min age
			if (!empty($sambaDomain->minPwdAge) && ($sambaDomain->minPwdAge > 0) && !empty($attributes['sambaPwdLastSet'][0])) {
				$timeVal = intval($attributes['sambaPwdLastSet'][0]) + $sambaDomain->minPwdAge;
				$time = new DateTime('@' . $timeVal, new DateTimeZone('UTC'));
				$time->setTimezone(getTimeZone());
				$now = new DateTime('now', getTimeZone());
				if ($time > $now) {
					$return['messages'][] = ['ERROR', _('You are not yet allowed to change your password.')];
				}
			}
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function managesPasswordAttributes(): bool {
		return ($this->get_scope() === "user");
	}

	/**
	 * {@inheritDoc}
	 */
	public function supportsForcePasswordChange(): bool {
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function passwordChangeRequested(string $password, array $modules, bool $forcePasswordChange): array {
		if (!in_array(static::class, $modules)) {
			return [];
		}
		$errors = [];
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
				$errors[] = ['ERROR', _('You are reusing an old password. Please choose a different password.')];
			}
			// set new history entry
			$historyLength = $sambaDomain->pwdHistoryLength;
			if (sambaSamAccount::isPasswordHistoryEnabled($this->moduleSettings) && !$oldPasswordUsed && !empty($historyLength) && ($historyLength > 0)) {
				if (!empty($this->orig['sambaPasswordHistory'][0])) {
					$this->attributes['sambaPasswordHistory'] = $this->orig['sambaPasswordHistory'];
				}
				else {
					$this->attributes['sambaPasswordHistory'] = [];
				}
				while (count($this->attributes['sambaPasswordHistory']) > ($historyLength - 1)) {
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
	 * @param string $password new password
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 * @param samba3domain|null $sambaDomain domain
	 * @return bool old password used
	 */
	public static function oldPasswordUsed($password, $attributes, $sambaDomain): bool {
		$attributes = array_change_key_case($attributes);
		if (empty($attributes['sambapasswordhistory'][0]) || ($sambaDomain === null)
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
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 * @param Connection|null $server LDAP connection (leave empty for admin interface)
	 * @param string $suffix LDAP search suffix (leave empty for admin interface)
	 * @return samba3domain|null domain
	 */
	public function getUserDomain($attributes, $server = null, $suffix = null): ?samba3domain {
		$attributes = array_change_key_case($attributes);
		$sambaDomains = $this->getDomains($server, $suffix);
		if (count($sambaDomains) > 0) {
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
				elseif (isset($attributes['sambadomainname'][0]) && ($attributes['sambadomainname'][0] != '')) {
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
	 * @param string $groupID group ID
	 * @return string|null group name
	 */
	private function getGroupName($groupID) {
		$results = searchLDAPByAttribute('gidNumber', $groupID, 'posixGroup', ['cn'], ['group']);
		if ((count($results) > 0) && isset($results[0]['cn'][0])) {
			return $results[0]['cn'][0];
		}
		return null;
	}

	/**
	 * Returns the time when the user needs to change his password.
	 *
	 * @param samba3domain[] $domains list of domain objects
	 * @param string|null $selectedDomain selected domain name
	 * @return string formatted time
	 */
	private function getPasswordMustChangeTime($domains, $selectedDomain): string {
		$return = '-';
		// check if password expires at all
		if ($this->noexpire) {
			return $return;
		}
		// check if there is a time set for the last password change
		if (!isset($this->attributes['sambaPwdLastSet'][0])) {
			return $return;
		}
		for ($i = 0; $i < count($domains); $i++) {
			if ($domains[$i]->name == $selectedDomain) {
				// check if a domain policy is set
				if (!isset($domains[$i]->maxPwdAge) || ($domains[$i]->maxPwdAge < 0)) {
					return $return;
				}
				$timeVal = intval($this->attributes['sambaPwdLastSet'][0]) + $domains[$i]->maxPwdAge;
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
	 * @param samba3domain[] $domains list of domain objects
	 * @param string $selectedDomain selected domain name
	 * @return string formatted time
	 */
	private function getPasswordCanChangeTime($domains, $selectedDomain): string {
		$return = '-';
		// check if there is a time set for the last password change
		if (!isset($this->attributes['sambaPwdLastSet'][0])) {
			return $return;
		}
		for ($i = 0; $i < count($domains); $i++) {
			if ($domains[$i]->name == $selectedDomain) {
				// check if a domain policy is set
				if (!isset($domains[$i]->minPwdAge) || ($domains[$i]->minPwdAge < 0)) {
					return $return;
				}
				$timeVal = intval($this->attributes['sambaPwdLastSet'][0]) + $domains[$i]->minPwdAge;
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
	 * @return string[] host names
	 */
	private function getHostList(): array {
		if ($this->cachedHostList !== null) {
			return $this->cachedHostList;
		}
		$this->cachedHostList = [];
		$entries = searchLDAPByAttribute('uid', '*', 'sambaSamAccount', ['uid'], ['host']);
		foreach ($entries as $entry) {
			$this->cachedHostList[] = $entry['uid'][0];
		}
		return $this->cachedHostList;
	}

	/**
	 * Returns a list of existing hosts.
	 *
	 * @return array<string, string> GID => sambaSID
	 */
	private function getGroupSIDList(): array {
		if ($this->cachedGroupSIDList !== null) {
			return $this->cachedGroupSIDList;
		}
		$this->cachedGroupSIDList = [];
		$result = searchLDAPByAttribute('sambaSID', '*', 'sambaGroupMapping', ['gidNumber', 'sambaSID'], ['group']);
		for ($i = 0; $i < count($result); $i++) {
			if (isset($result[$i]['gidnumber'][0])) {
				$this->cachedGroupSIDList[$result[$i]['gidnumber'][0]] = $result[$i]['sambasid'][0];
			}
		}
		return $this->cachedGroupSIDList;
	}

	/**
	 * Returns a list of existing Samba 3 domains.
	 *
	 * @param Connection|null $server LDAP connection (leave empty for admin interface)
	 * @param String $suffix LDAP search suffix (leave empty for admin interface)
	 * @return samba3domain[] list of samba3domain objects
	 */
	private function getDomains($server = null, $suffix = null): array {
		if ($this->cachedDomainList !== null) {
			return $this->cachedDomainList;
		}
		$this->cachedDomainList = search_domains($server, $suffix);
		return $this->cachedDomainList;
	}

	/**
	 * Sets the expiration date of this account.
	 * If all parameters are null the expiration date will be removed.
	 *
	 * @param int|null $year year (e.g. 2040)
	 * @param int|null $month month (e.g. 8)
	 * @param int|null $day day (e.g. 27)
	 */
	public function setExpirationDate($year, $month, $day): void {
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
	 * @return bool Samba extension is active
	 */
	public function isExtensionEnabled(): bool {
		return in_array('sambaSamAccount', $this->attributes['objectClass']);
	}

	/**
	 * Returns if the Samba part of the current account is locked.
	 *
	 * @param array<string, string[]|string>|null $attributes LDAP attributes
	 * @return bool account is locked
	 */
	public function isLocked(?array $attributes = null): bool {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		if (!isset($attributes['sambaacctflags'][0])) {
			return false;
		}
		return str_contains($attributes['sambaacctflags'][0], 'L');
	}

	/**
	 * Returns if the Samba part of the current account is deactivated.
	 *
	 * @param array<string, string[]|string>|null $attributes LDAP attributes
	 * @return bool account is deactivated
	 */
	public function isDeactivated(?array $attributes = null): bool {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		if (!isset($attributes['sambaacctflags'][0])) {
			return false;
		}
		return str_contains($attributes['sambaacctflags'][0], 'D');
	}

	/**
	 * Deactivates this account.
	 *
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 */
	public function deactivate(array &$attributes): void {
		if (!isset($attributes['sambaacctflags'][0])) {
			$attributes['sambaacctflags'][0] = ($this->get_scope() === 'host') ? '[W          ]' : '[ XU        ]';
		}
		$flags = $attributes['sambaacctflags'][0];
		if (!str_contains($flags, 'D')) {
			$flags[strpos($flags, ' ')] = 'D';
		}
		$attributes['sambaacctflags'][0] = (string) $flags;
	}

	/**
	 * Activates this account.
	 *
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 */
	public function activate(array &$attributes): void {
		if (!isset($attributes['sambaacctflags'][0])) {
			$attributes['sambaacctflags'][0] = ($this->get_scope() === 'host') ? '[W          ]' : '[ XU        ]';
		}
		$attributes['sambaacctflags'][0] = str_replace('D', ' ', $attributes['sambaacctflags'][0]);
	}

	/**
	 * Unlocks this account.
	 *
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 */
	public function unlock(array &$attributes): void {
		$attributes['sambaacctflags'][0] = str_replace('L', ' ', $attributes['sambaacctflags'][0]);
	}

	/**
	 * Creates the value to store in sambaPasswordHistory attribute.
	 *
	 * @param string $password password
	 * @return string|null value for sambaPasswordHistory
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
		if (empty($historyEntry) || (strlen($historyEntry) !== 64)) {
			return false;
		}
		$salt = hex2bin(substr($historyEntry, 0, 32));
		$hash = substr($historyEntry, 32, 32);
		$md4hash = ntPassword($password);
		$md5hash = md5($salt . hex2bin($md4hash));
		return strtolower($md5hash) === strtolower($hash);
	}

	/**
	 * Returns if password history is enabled.
	 *
	 * @param array<string, string[]> $settings server profile or self-service settings
	 * @return bool enabled
	 */
	public static function isPasswordHistoryEnabled($settings): bool {
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
	public function getPasswordQuickChangeOptions(bool $forcePasswordChangeByDefault): array {
		$options = [];
		if (isset($this->attributes['sambaacctflags'][0])
			&& (str_contains($this->attributes['sambaacctflags'][0], 'L')
				|| str_contains($this->attributes['sambaacctflags'][0], 'D'))) {
			$options[] = new PasswordQuickChangeOption('unlockAccountSamba', _('Unlock Samba account'));
		}
		if (in_array_ignore_case('sambaSamAccount', $this->attributes['objectClass'])) {
			$options[] = new PasswordQuickChangeOption('syncSambaNT', _('Change Samba NT password'));
			$options[] = new PasswordQuickChangeOption('forcePasswordChangeSamba', _('Force Samba password change'), $forcePasswordChangeByDefault);
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
		$attrs = [];
		if (isset($_POST['unlockAccountSamba'])
			&& isset($this->attributes['sambaacctflags'][0])
			&& ((str_contains($this->attributes['sambaacctflags'][0], 'L')) || (str_contains($this->attributes['sambaacctflags'][0], 'D')))) {
			$sambaFlags = $this->attributes['sambaacctflags'][0];
			$sambaFlags = str_replace('L', ' ', $sambaFlags);
			$sambaFlags = str_replace('D', ' ', $sambaFlags);
			$attrs['sambaacctflags'][0] = $sambaFlags;
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
					$attrs['sambaPasswordHistory'] = [];
				}
				while (count($attrs['sambaPasswordHistory']) > ($domain->pwdHistoryLength - 1)) {
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
		return $this->attributes['uid'][0] ?? null;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array {
		return [];
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
		$details = [];
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
		return ['sambaacctflags'];
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
		if ($attributeName === 'sambakickofftime') {
			return function(?array $values, ?string $filterValue): bool {
				$regex = str_replace(['*'], ['.*'], $filterValue);
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
	public function getListRenderFunction(string $attributeName, ConfiguredType $type): ?callable {
		if ($attributeName === 'sambakickofftime') {
			return function(array $entry, string $attribute): htmlElement {
				if (!empty($entry[$attribute][0])) {
					if ($entry[$attribute][0] > 2147483648) {
						return new htmlOutputText("∞");
					}
					else {
						$date = new DateTime('@' . $entry[$attribute][0], new DateTimeZone('UTC'));
						return new htmlOutputText($date->format('d.m.Y'));
					}
				}
				return new htmlGroup();
			};
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'displayname' => _('Display name'),
			'sambakickofftime' => _('Account expiration date'),
			'sambadomainname' => _('Domain'),
			'sambaprimarygroupsid' => _('Windows group')
		];
	}

}
