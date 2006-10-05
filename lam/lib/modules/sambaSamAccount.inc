<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz

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

/**
* Manages the object class "sambaSamAccount" for users and hosts.
*
* @package modules
*/
class sambaSamAccount extends baseModule {

	// Variables
	/** use Unix password as samba password? */
	var $useunixpwd;
	/** use no password? */
	var $nopwd;
	/** password does not expire? */
	var $noexpire;
	/** account deactivated? */
	var $deactivated;
	/** array of well known rids */
	var $rids;
	/** HEX to binary conversion table */
	var $hex2bitstring = array('0' => '0000', '1' => '0001', '2' => '0010', '3' => '0011', '4' => '0100',
		'5' => '0101', '6' => '0110', '7' => '0111', '8' => '1000', '9' => '1001', 'A' => '1010',
		'B' => '1011', 'C' => '1100', 'D' => '1101', 'E' => '1110', 'F' => '1111');


	/**
	* Creates a new sambaSamAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function sambaSamAccount($scope) {
	// List of well known rids
	$this->rids = array(
		_('Domain Admins') => 512, _('Domain Users') => 513, _('Domain Guests') => 514,
		_('Domain Computers') => 515, _('Domain Controllers') => 516, _('Domain Certificate Admins') => 517,
		_('Domain Schema Admins') => 518, _('Domain Enterprise Admins') => 519, _('Domain Policy Admins') => 520);
	// call parent constructor
	parent::baseModule($scope);
	}

	/** this functin fills the error message array with messages
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
		$this->messages['sambaLMPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['sambaLMPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}=@$ !'));
		$this->messages['sambaLMPassword'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}=@$ !'));
		$this->messages['rid'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('Please enter a RID number or the name of a special account!'));
		$this->messages['rid'][3] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('This is not a valid RID number!'));
		$this->messages['displayName'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_displayName', _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['pwdUnix'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdUnix', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noPassword'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noPassword', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noExpire'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noExpire', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['deactivated'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_deactivated', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['pwdCanChange'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdCanChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['pwdMustChange'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdMustChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['expireDate'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_expireDate', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['homeDrive'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_homeDrive', _('Please enter a valid drive letter.'));
		$this->messages['domain'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_domain', _('LAM was unable to find a domain with this name!'));
		$this->messages['logonHours'][0] = array('ERROR', _('Logon hours'), _('The format of the logon hours field is invalid!'));
		$this->messages['logonHours'][1] = array('ERROR', _('Account %s:') . ' sambaSamAccount_logonHours', _('The format of the logon hours field is invalid!'));
		$this->messages['group'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_group', _('Please enter a valid group name!'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages user and host accounts
		$return["account_types"] = array("user", "host");
		if ($this->get_scope() == "host") {
			// LDAP filter
			$return["ldap_filter"] = array('and' => '(uid=*$)', 'or' => "(objectClass=posixAccount)");
		}
		// alias name
		$return["alias"] = _('Samba 3');
		// RDN attribute
		$return["RDN"] = array("sambaSID" => "low");
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('sambaSamAccount');
		// managed attributes
		$return['attributes'] = array('uid', 'sambaSID', 'sambaLMPassword', 'sambaNTPassword', 'sambaPwdLastSet',
			'sambaLogonTime', 'sambaLogoffTime', 'sambaKickoffTime', 'sambaPwdCanChange', 'sambaPwdMustChange', 'sambaAcctFlags',
			'displayName', 'sambaHomePath', 'sambaHomeDrive', 'sambaLogonScript', 'sambaProfilePath',
			'sambaUserWorkstations', 'sambaPrimaryGroupSID', 'sambaDomainName', 'sambaLogonHours');
		// PHP extensions
		$return['extensions'] = array('mhash', 'iconv');
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
			'sambaSamAccount_smbhome' => 'sambaHomePath',
			'sambaSamAccount_profilePath' => 'sambaProfilePath',
			'sambaSamAccount_logonScript' => 'sambaLogonScript',
			'sambaSamAccount_userWorkstations' => 'sambaUserWorkstations',
			'sambaSamAccount_sambaDomainName' => 'sambaDomainName',
			'sambaSamAccount_logonHours' => 'sambaLogonHours'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'displayName',
			'sambaHomePath',
			'sambaHomeDrive',
			'sambaLogonScript',
			'sambaProfilePath',
			'sambaUserWorkstations',
			'sambaDomainName',
			'sambaPrimaryGroupSID'
		);
		$return['selfServiceFieldSettings'] = array(
			'syncNTPassword' => _('Sync Samba NT password with Unix password'),
			'syncLMPassword' => _('Sync Samba LM password with Unix password')
		);
		// help Entries
		$return['help'] = array (
			"displayName" => array(
				"ext" => "FALSE", "Headline" => _("Display name"),
				"Text" => _("This is the account's full name on Windows systems.")),
			"password" => array(
				"ext" => "FALSE", "Headline" => _("Samba password"),
				"Text" => _("This is the account's Windows password.")),
			"resetPassword" => array(
				"ext" => "FALSE", "Headline" => _("Reset password"),
				"Text" => _("This will reset the host's password to a default value.")),
			"pwdUnix" => array(
				"ext" => "FALSE", "Headline" => _("Use Unix password"),
				"Text" => _("If checked Unix password will also be used as Samba password.")),
			"pwdUnixUpload" => array(
				"ext" => "FALSE", "Headline" => _("Use Unix password"),
				"Text" => _("If set to \"true\" Unix password will also be used as Samba password.")),
			"noPassword" => array(
				"ext" => "FALSE", "Headline" => _("Use no password"),
				"Text" => _("If checked no password will be used.")),
			"noPasswordUpload" => array(
				"ext" => "FALSE", "Headline" => _("Use no password"),
				"Text" => _("If set to \"true\" no password will be used.")),
			"noExpire" => array(
				"ext" => "FALSE", "Headline" => _("Password does not expire"),
				"Text" => _("If checked password does not expire. (Setting X-Flag)")),
			"noExpireUpload" => array(
				"ext" => "FALSE", "Headline" => _("Password does not expire"),
				"Text" => _("If set to \"true\" password does not expire. (Setting X-Flag)")),
			"deactivated" => array(
				"ext" => "FALSE", "Headline" => _("Account is deactivated"),
				"Text" => _("If checked account will be deactivated. (Setting D-Flag)")),
			"deactivatedUpload" => array(
				"ext" => "FALSE", "Headline" => _("Account is deactivated"),
				"Text" => _("If set to \"true\" account will be deactivated. (Setting D-Flag)")),
			"pwdCanChange" => array(
				"ext" => "FALSE", "Headline" => _("User can change password"),
				"Text" => _("Date after the user is able to change his password. Format: DD-MM-YYYY")),
			"pwdMustChange" => array ("ext" => "FALSE", "Headline" => _("User must change password"),
				"Text" => _("Date after the user must change his password. Format: DD-MM-YYYY")),
			"homeDrive" => array(
				"ext" => "FALSE", "Headline" => _("Home drive"),
				"Text" => _("Drive letter assigned on windows workstations as homedirectory.")),
			"homePath" => array(
				"ext" => "FALSE", "Headline" => _("Home path"),
				"Text" => _('UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"profilePath" => array(
				"ext" => "FALSE", "Headline" => _("Profile path"),
				"Text" => _('Path of the user profile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"scriptPath" => array(
				"ext" => "FALSE", "Headline" => _("Logon script"),
				"Text" => _('File name and path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"userWorkstations" => array(
				"ext" => "FALSE", "Headline" => _("Samba workstations"),
				"Text" => _("List of Samba workstations the user is allowed to login. Empty means every workstation.")),
			"workstations" => array(
				"ext" => "FALSE", "Headline" => _("Samba workstations"),
				"Text" => _("Comma separated list of Samba workstations the user is allowed to login. Empty means every workstation."). ' '. _("Can be left empty.")),
			"group" => array(
				"ext" => "FALSE", "Headline" => _("Windows primary group"),
				"Text" => _("This is the user's primary Windows group.")),
			"groupUpload" => array(
				"ext" => "FALSE", "Headline" => _("Windows primary group SID"),
				"Text" => _("This is the SID of the user's primary Windows group.")),
			"specialUser" => array(
				"ext" => "FALSE", "Headline" => _("Special user"),
				"Text" => _("This allows you to define this account as a special user like administrator or guest.")),
			"ridUpload" => array(
				"ext" => "FALSE", "Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your Windows account. You can either enter a number or one of these special accounts: ") .
				implode(", ", array_keys($this->rids)) . "<br><br>" . _("If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")),
			"ridUploadHost" => array(
				"ext" => "FALSE", "Headline" => _("Samba RID"),
				"Text" => _("This is the relative ID number for your host account. If you leave this empty LAM will use: uidNumber*2 + sambaAlgorithmicRidBase.")),
			"domain" => array(
				"ext" => "FALSE", "Headline" => _("Domain"),
				"Text" => _("Windows domain name of account.")),
			"logonHours" => array(
				"ext" => "FALSE", "Headline" => _("Logon hours"),
				"Text" => _("This option defines the allowed logon hours for this account.")),
			"logonHoursUpload" => array(
				"ext" => "FALSE", "Headline" => _("Logon hours"),
				"Text" => _("This option defines the allowed logon hours for this account. The format is the same as for the LDAP attribute. The 24*7 hours are represented as 168 bit which are saved as 21 hex (21*8 = 168) values. The first bit represents Sunday 0:00 - 0:59 in GMT.")),
			'expireDate' => array (
				"Headline" => _("Account expiration date"),
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")),
			'timeZone' => array (
				"Headline" => _("Time zone"),
				"Text" => _("This is the time zone of your Samba server. LAM needs this information to display the logon hours correctly."))
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
					'name' => 'sambaSamAccount_pwdCanChange',
					'description' => _('User can change password'),
					'help' => 'pwdCanChange',
					'default' => '31-12-2030',
					'example' => '15-11-2006'
				),
				array(
					'name' => 'sambaSamAccount_pwdMustChange',
					'description' => _('User must change password'),
					'help' => 'pwdMustChange',
					'default' => '31-12-2030',
					'example' => '15-10-2006'
				),
				array(
					'name' => 'sambaSamAccount_expireDate',
					'description' => _('Account expiration date'),
					'help' => 'expireDate',
					'default' => '31-12-2030',
					'example' => '15-10-2006'
				),
				array(
					'name' => 'sambaSamAccount_homeDrive',
					'description' => _('Home drive'),
					'help' => 'homeDrive',
					'example' => 'k:'
				),
				array(
					'name' => 'sambaSamAccount_homePath',
					'description' => _('Home path'),
					'help' => 'homePath',
					'example' => _('\\\\server\\homes\\smiller')
				),
				array(
					'name' => 'sambaSamAccount_profilePath',
					'description' => _('Profile path'),
					'help' => 'profilePath',
					'example' => _('\\\\server\\profiles\\smiller')
				),
				array(
					'name' => 'sambaSamAccount_logonScript',
					'description' => _('Logon script'),
					'help' => 'scriptPath',
					'example' => 'logon.bat'
				),
				array(
					'name' => 'sambaSamAccount_workstations',
					'description' => _('Samba workstations'),
					'help' => 'workstations',
					'example' => 'PC01,PC02,PC03'
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
					'default' => '&lt;uidNumber&gt;*2 + &lt;sambaAlgorithmicRidBase&gt;'
				),
				array(
					'name' => 'sambaSamAccount_logonHours',
					'description' => _('Logon hours'),
					'help' => 'logonHoursUpload',
					'example' => 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF'
				)
			);
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
					'default' => '&lt;uidNumber&gt;*2 + &lt;sambaAlgorithmicRidBase&gt;'
				)
			);
		}
		// configuration options
		$return['config_options']['user'] = array(
		array(
			0 => array('kind' => 'text', 'text' => '<b>' . _("Time zone") . ': &nbsp;</b>'),
			1 => array('kind' => 'select', 'name' => 'sambaSamAccount_timeZone', 'size' => '1',
				'options' => array(
					array('-12', "GMT-12: " . _("Eniwetok, Kwajalein")),
					array('-11', "GMT-11: " . _("Midway Island, Samoa")),
					array('-10', "GMT-10: " . _("Hawaii")),
					array('-9', "GMT-09: " . _("Alaska")),
					array('-8', "GMT-08: " . _("Pacific Time (US & Canada)")),
					array('-7', "GMT-07: " . _("Mountain Time (US & Canada)")),
					array('-6', "GMT-06: " . _("Central Time (US & Canada), Mexico City")),
					array('-5', "GMT-05: " . _("Eastern Time (US & Canada), Bogota")),
					array('-4', "GMT-04: " . _("Atlantic time (Canada), Caracas")),
					array('-3', "GMT-03: " . _("Brazil, Buenos Aires")),
					array('-2', "GMT-02: " . _("Mid-Atlantic")),
					array('-1', "GMT-01: " . _("Azores, Cape Verde Islands")),
					array('0', 'GMT: ' . _("Western Europe Time, London, Lisbon")),
					array('1', "GMT+01: " . _("Central Europe Time, Paris, Berlin")),
					array('2', "GMT+02: " . _("Eastern Europe Time, South Africa")),
					array('3', "GMT+03: " . _("Baghdad, Riyadh, Moscow")),
					array('4', "GMT+04: " . _("Abu Dhabi, Muscat, Baku")),
					array('5', "GMT+05: " . _("Islamabad, Karachi")),
					array('6', "GMT+06: " . _("Almaty, Dhaka, Colombo")),
					array('7', "GMT+07: " . _("Bangkok, Hanoi, Jakarta")),
					array('8', "GMT+08: " . _("Beijing, Perth, Singapore")),
					array('9', "GMT+09: " . _("Tokyo, Seoul, Osaka, Yakutsk")),
					array('10', "GMT+10: " . _("East Australian Standard, Guam")),
					array('11', "GMT+11: " . _("Magadan, Solomon Islands")),
					array('12', "GMT+12: " . _("Auckland, Fiji, Kamchatka")),
					),
					'options_selected' => array('0'),
					'descriptiveOptions' => true),
			2 => array('kind' => 'help', 'value' => 'timeZone'))
		);
		// configuration descriptions
		$return['config_descriptions'] = array('legend' => _("Samba 3 settings"), 'descriptions' => array());
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
		$this->useunixpwd = true;
		$this->noexpire = true;
		$this->nopwd = false;
		$this->deactivated = false;
		$sambaDomains = search_domains();
		if (sizeof($sambaDomains) == 0) {
			StatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.'), '');
			return;
		}
	}

	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]=='') return false;
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]=='') return false;
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0]=='') return false;
		return true;
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['sambaSID'][0] == '') return false;
		return true;
	}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		parent::load_attributes($attr);
		if (is_string($this->attributes['sambaAcctFlags'][0])) {
			if (strpos($this->attributes['sambaAcctFlags'][0], "D")) $this->deactivated = true;
			else $this->deactivated = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "N")) $this->nopwd = true;
			else $this->nopwd = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "X")) $this->noexpire = true;
			else $this->noexpire = false;
		}
	}

	/* This function returns an array with 3 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* modify are attributes which have to been modified in ldap entry
	*/
	function save_attributes() {
		/* Create sambaSID. Can't create it while loading attributes because
		* it's psssible uidNumber has changed
		*/
		// Get Domain SID from name
		$sambaDomains = search_domains();
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
			}
		}
		$special = false;
		if ($this->attributes['sambaSID'][0] == $SID."-500") $special = true;
		if ($this->attributes['sambaSID'][0] == $SID."-501") $special = true;
		if (!$special) $this->attributes['sambaSID'][0] == $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2+$RIDbase);

		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);

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
		$sambaDomains = search_domains();
		if (sizeof($sambaDomains) == 0) {
			return array(array(array("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.'))));
		}
		// Save attributes
		$this->attributes['sambaDomainName'][0] = $_POST['sambaDomainName'];
		// Get Domain SID from name
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
				}
		$flag = "[";
		if ($_POST['sambaAcctFlagsD']) {
			$flag .= "D";
			$this->deactivated = true;
		}
		else {
			$this->deactivated = false;
		}
		if ($_POST['sambaAcctFlagsX']) {
			$flag .= "X";
			$this->noexpire = true;
		}
		else {
			$this->noexpire = false;
		}
		if ($_POST['sambaAcctFlagsN']) {
			$flag .= "N";
			$this->nopwd = true;
		}
		else {
			$this->nopwd = false;
		}
		if ($_POST['sambaAcctFlagsS']) $flag .= "S";
		if ($_POST['sambaAcctFlagsH']) $flag .= "H";
		if ($_POST['sambaAcctFlagsW']) $flag .= "W";
		if ($_POST['sambaAcctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['sambaAcctFlags'][0] = $flag;

		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!($this->attributes['displayName'][0] == '') && !(get_preg($this->attributes['displayName'][0], 'realname'))) {
			$errors[] = $this->messages['displayName'][1];
		}

		// host attributes
		if ($_SESSION[$this->base]->type=='host') {
			$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[_('Domain Computers')];
			if ($_POST['ResetSambaPassword'] || !$this->attributes['sambaLMPassword'][0]) {
				$hostname = $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0];
				$hostname = substr($hostname, 0, strlen($hostname) - 1);
				$this->attributes['sambaLMPassword'][0] = lmPassword($hostname);
				$this->attributes['sambaNTPassword'][0] = ntPassword($hostname);
				$this->attributes['sambaPwdLastSet'][0] = time();
			}
		}
		// user attributes
		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['sambaPwdCanChange'][0] = mktime(intval($_POST['sambaPwdCanChange_h']), intval($_POST['sambaPwdCanChange_m']),
				intval($_POST['sambaPwdCanChange_s']), intval($_POST['sambaPwdCanChange_mon']), intval($_POST['sambaPwdCanChange_day']),
				intval($_POST['sambaPwdCanChange_yea']));
			$this->attributes['sambaPwdMustChange'][0] = mktime(intval($_POST['sambaPwdMustChange_h']), intval($_POST['sambaPwdMustChange_m']),
				intval($_POST['sambaPwdMustChange_s']), intval($_POST['sambaPwdMustChange_mon']), intval($_POST['sambaPwdMustChange_day']),
				intval($_POST['sambaPwdMustChange_yea']));
			$this->attributes['sambaKickoffTime'][0] = mktime(intval($_POST['sambaExpire_h']), intval($_POST['sambaExpire_m']),
				intval($_POST['sambaExpire_s']), intval($_POST['sambaExpire_mon']), intval($_POST['sambaExpire_day']),
				intval($_POST['sambaExpire_yea']));
			$this->attributes['sambaHomePath'][0] = $_POST['sambaHomePath'];
			if ($_POST['sambaHomeDrive'] == "-")
				$this->attributes['sambaHomeDrive'][0] = '';
			else
				$this->attributes['sambaHomeDrive'][0] = $_POST['sambaHomeDrive'];
			$this->attributes['sambaLogonScript'][0] = $_POST['sambaLogonScript'];
			$this->attributes['sambaProfilePath'][0] = $_POST['sambaProfilePath'];
			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($_POST['sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[$rids[$i]];
					}
				}
			if (!$wrid) {
				$gidnumber = $_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0];
				$groups = $_SESSION['cache']->get_cache(array('gidNumber', 'sambaSID'), 'sambaGroupMapping', 'group');
				$groupKeys = array_keys($groups);
				for ($i = 0; $i < sizeof($groupKeys); $i++) {
					if ($groups[$groupKeys[$i]]['gidNumber'][0] == $gidnumber) {
						if (isset($groups[$groupKeys[$i]]['sambaSID'][0])) {
							$this->attributes['sambaPrimaryGroupSID'][0] = $groups[$groupKeys[$i]]['sambaSID'][0];
						}
						break;
					}
				}
			}

			if ($_POST['useunixpwd']) {
				$this->useunixpwd = true;
				$this->attributes['sambaLMPassword'][0] = lmPassword($_SESSION[$this->base]->module['posixAccount']->clearTextPassword);
				$this->attributes['sambaNTPassword'][0] = ntPassword($_SESSION[$this->base]->module['posixAccount']->clearTextPassword);
				$this->attributes['sambaPwdLastSet'][0] = time();
			}
			else $this->useunixpwd = false;
			if (!$this->useunixpwd && isset($_POST['sambaLMPassword']) && ($_POST['sambaLMPassword'] != '')) {
				if ($_POST['sambaLMPassword'] != $_POST['sambaLMPassword2']) {
					$errors[] = $this->messages['sambaLMPassword'][0];
					unset ($_POST['sambaLMPassword2']);
				}
				else {
					if (!get_preg($_POST['sambaLMPassword'], 'password')) $errors[] = $this->messages['sambaLMPassword'][1];
					else {
						$this->attributes['sambaLMPassword'][0] = lmPassword($_POST['sambaLMPassword']);
						$this->attributes['sambaNTPassword'][0] = ntPassword($_POST['sambaLMPassword']);
						$this->attributes['sambaPwdLastSet'][0] = time();
					}
				}
			}
			$specialRids = array_flip($this->rids);
			// set special RID if selected
			if (in_array($_POST['sambaSID'], $specialRids)) {
				$this->attributes['sambaSID'][0] = $SID . '-' . $this->rids[$_POST['sambaSID']];
			}
			// standard RID
			else if ($_POST['sambaSID'] == "-") {
				$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
				// change only if not yet set or previously set to special SID
				if (!$this->attributes['sambaSID'][0] || in_array($rid, $this->rids)) {
					$this->attributes['sambaSID'][0] = $SID."-". (($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2)+$RIDbase);
				}
			}
			// Check values
			$this->attributes['sambaHomePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0], $this->attributes['sambaHomePath'][0]);
			$this->attributes['sambaHomePath'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]), $this->attributes['sambaHomePath'][0]);
			if ($this->attributes['sambaHomePath'][0] != $_POST['sambaHomePath']) $errors[] = $this->messages['homePath'][1];
			$this->attributes['sambaLogonScript'][0] = str_replace('$user', $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0], $this->attributes['sambaLogonScript'][0]);
			$this->attributes['sambaLogonScript'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]), $this->attributes['sambaLogonScript'][0]);
			if ($this->attributes['sambaLogonScript'][0] != $_POST['sambaLogonScript']) $errors[] = $this->messages['logonScript'][1];
			$this->attributes['sambaProfilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0], $this->attributes['sambaProfilePath'][0]);
			$this->attributes['sambaProfilePath'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]), $this->attributes['sambaProfilePath'][0]);
			if ($this->attributes['sambaProfilePath'][0] != $_POST['sambaProfilePath']) $errors[] = $this->messages['profilePath'][1];
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC')))
				$errors[] = $this->messages['homePath'][0];
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript')))
				$errors[] = $this->messages['logonScript'][0];
			if (!($this->attributes['sambaProfilePath'][0] == '') &&
				!(get_preg($this->attributes['sambaProfilePath'][0], 'UNC') xor get_preg($this->attributes['sambaProfilePath'][0], 'homeDirectory')))
					$errors[] = $this->messages['profilePath'][0];
		}
		else {  // host
			if (!$this->attributes['sambaSID'][0]) {
				$this->attributes['sambaSID'][0] = $SID."-". (($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2)+$RIDbase);
			}
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
		if ($_SESSION[$this->base]->type=='user') {
			if (isset($_POST['availableSambaUserWorkstations']) && isset($_POST['sambaUserWorkstations_add'])) { // Add workstations to list
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++) {
					if ($workstations[$i]=='') unset($workstations[$i]);
				}
				$workstations = array_values($workstations);
				// Add new // Add workstations
				$workstations = array_merge($workstations, $_POST['availableSambaUserWorkstations']);
				// remove doubles
				$workstations = array_flip($workstations);
				array_unique($workstations);
				$workstations = array_flip($workstations);
				// sort workstations
				sort($workstations);
				// Recreate workstation string
				$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
					}
				}
			elseif (isset($_POST['sambaUserWorkstations']) && isset($_POST['sambaUserWorkstations_remove'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++) {
					if ($workstations[$i]=='') unset($workstations[$i]);
				}
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($_POST['sambaUserWorkstations'], $workstations);
				// Recreate workstation string
				$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
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
		if ($_POST['form_subpage_sambaSamAccount_attributes_abort']) return;
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
			$byte[hi] = substr($part,0,4);
			$byte[low] = substr($part,4,4);
			$hex = $bitstring2hex[$byte[hi]].$bitstring2hex[$byte[low]];
			$logonHoursNew = $logonHoursNew . $hex;
		}
		$this->attributes['sambaLogonHours'][0] = $logonHoursNew;
		return array();
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes() {
		// Get Domain SID from user SID
		$sambaDomains = search_domains();
		if (sizeof($sambaDomains) == 0) {
			StatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.'), '');
			return array();
		}
		if ($this->attributes['sambaSID'][0]!='') {
			$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
		}
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			$sambaDomainNames[] = $sambaDomains[$i]->name;
			if (isset($domainSID)) {
				if ($domainSID == $sambaDomains[$i]->SID) {
					$SID = $domainSID;
					$sel_domain = $sambaDomains[$i]->name;
				}
			}
			elseif (isset($this->attributes['sambaDomainName'][0]) && ($this->attributes['sambaDomainName'][0]!='')) {
				if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
					$SID = $sambaDomains[$i]->SID;
					$sel_domain = $sambaDomains[$i]->name;
				}
			}
		}
		if (isset($this->attributes['sambaPwdCanChange'][0])) $canchangedate = getdate(intval($this->attributes['sambaPwdCanChange'][0]));
		else $canchangedate = getdate(0);
		if (isset($this->attributes['sambaPwdMustChange'][0])) $mustchangedate = getdate(intval($this->attributes['sambaPwdMustChange'][0]));
		else $mustchangedate = getdate(0);
		if (isset($this->attributes['sambaKickoffTime'][0])) $expiredate = getdate(intval($this->attributes['sambaKickoffTime'][0]));
		else $expiredate = getdate(0);

		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Display name')),
			1 => array('kind' => 'input', 'type' => 'text', 'name' => 'displayName', 'size' => '20', 'value' => $this->attributes['displayName'][0]),
			2 => array('kind' => 'help', 'value' => 'displayName'));
		if ($_SESSION[$this->base]->type=='user') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_h', 'type' => 'hidden', 'value' => $canchangedate['hours']),
					1 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_m', 'type' => 'hidden', 'value' => $canchangedate['minutes']),
					2 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_s', 'type' => 'hidden', 'value' => $canchangedate['seconds']),
					3 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_h', 'type' => 'hidden', 'value' => $mustchangedate['hours']),
					4 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_m', 'type' => 'hidden', 'value' => $mustchangedate['minutes']),
					5 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_s', 'type' => 'hidden', 'value' => $mustchangedate['seconds']),
					6 => array ( 'kind' => 'input', 'name' => 'sambaExpire_h', 'type' => 'hidden', 'value' => $expiredate['hours']),
					7 => array ( 'kind' => 'input', 'name' => 'sambaExpire_m', 'type' => 'hidden', 'value' => $expiredate['minutes']),
					8 => array ( 'kind' => 'input', 'name' => 'sambaExpire_s', 'type' => 'hidden', 'value' => $expiredate['seconds']),
					9 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsU', 'type' => 'hidden', 'value' => 'true'));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Samba password')),
				1 => array('kind' => 'input', 'name' => 'sambaLMPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'password'));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Repeat password')),
				1 => array('kind' => 'input', 'name' => 'sambaLMPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255'));
			if (isset($_SESSION[$this->base]->module['posixAccount']->clearTextPassword)) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use Unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd),
					2 => array ('kind' => 'help', 'value' => 'useunixpwd'));
			}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => $this->nopwd, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'noPassword'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => $this->noexpire, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'noExpire'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => $this->deactivated, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'deactivated'));
			for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User can change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_day',
						'options' => $mday, 'options_selected' => array($canchangedate['mday'])),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_mon',
						'options' => $mon, 'options_selected' => array($canchangedate['mon'])),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdCanChange_yea',
						'options' => $year, 'options_selected' => array($canchangedate['year']))))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_day',
						'options' => $mday, 'options_selected' => array($mustchangedate['mday'])),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_mon',
						'options' => $mon, 'options_selected' => array($mustchangedate['mon'])),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_yea',
						'options' => $year, 'options_selected' => array($mustchangedate['year']))))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdMustChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account expiration date') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaExpire_day',
						'options' => $mday, 'options_selected' => $expiredate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'sambaExpire_mon',
						'options' => $mon, 'options_selected' => $expiredate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'sambaExpire_yea',
						'options' => $year, 'options_selected' => $expiredate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'expireDate' ));

			$drives = array('-');
			for ($i=90; $i>67; $i--) $drives[] = chr($i).':';
			if ($this->attributes['sambaHomeDrive'][0]) {
				$selected = array ($this->attributes['sambaHomeDrive'][0]);
			}
			else $selected = array('-');
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home drive') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaHomeDrive', 'options' => $drives, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'homeDrive' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaHomePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaHomePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'homePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Profile path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaProfilePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaProfilePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'profilePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Logon script') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaLogonScript', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaLogonScript'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'scriptPath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba workstations') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_sambaSamAccount_sambaUserWorkstations_open', 'value' => _('Edit workstations')),
				2 => array ( 'kind' => 'help', 'value' => 'userWorkstations' ));

			$names = array_keys($this->rids);
			$wrid=false;
			$options = array();
			$selected = array();
			for ($i=0; $i<count($names); $i++) {
				if ($this->attributes['sambaPrimaryGroupSID'][0]==$SID."-".$this->rids[$names[$i]]) {
					$selected[] = $names[$i];
					$wrid=true;
					}
				else $options[] = $names[$i];
				}
			if ($wrid) $options[] = $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
				else $selected[] = $_SESSION['cache']->getgrnam($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]);
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaPrimaryGroupSID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'group' ));
			// display if group SID should be mapped to a well known SID
			$options = array_keys($this->rids);
			$options[] = '-';
			$selected = array();
			if ($this->attributes['sambaSID'][0]) {
				$rid = substr($this->attributes['sambaSID'][0], strrpos($this->attributes['sambaSID'][0], '-') + 1, strlen($this->attributes['sambaSID'][0]));
				$specialRids = array_flip($this->rids);
				if (in_array($rid, $this->rids)) {
					$selected = array($specialRids[$rid]);
				}
				else {
					$selected = array('-');
				}
			}
			else $selected[] = "-";
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaSID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'specialUser' ));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') . "*" ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array($sel_domain)),
			2 => array ( 'kind' => 'help', 'value' => 'domain' ));
		// logon hours
		if ($_SESSION[$this->base]->type=='user') {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Logon hours') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_sambaSamAccount_logonHours_open', 'value' => _('Edit logon hours')),
				2 => array ( 'kind' => 'help', 'value' => 'logonHours' ));
		}
		// reset host password
		if ($_SESSION[$this->base]->type=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword', 'value' => _('Reset')),
				2 => array ( 'kind' => 'help', 'value' => 'resetPassword' ));
			}
		return $return;
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_sambaUserWorkstations() {
		if ($_SESSION[$this->base]->type=='user') {
			// Get list of all hosts.
			$result = $_SESSION['cache']->get_cache('uid', 'sambaSamAccount', 'host');
			if (is_array($result)) {
				foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
				sort($availableUserWorkstations, SORT_STRING);
				$result = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
				$userWorkstations = explode (',', $result);
				$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);
				}

			$return[] = array( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Allowed workstations"), 'value' =>
				array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Allowed workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaUserWorkstations', 'size' => '15', 'multiple' => true, 'options' => $userWorkstations)))),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_add',
					'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_remove', 'value' => '=>' )),
					2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'sambaUserWorkstations' )))),
				2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'availableSambaUserWorkstations', 'size' => '15', 'multiple' => true, 'options' => $availableUserWorkstations))))
					))));

			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'form_subpage_sambaSamAccount_attributes_back', 'type' => 'submit', 'value' => _('Back') ),
				1 => array ( 'kind' => 'text'),
				2 => array ('kind' => 'text'));
			}
		return $return;
	}

	/**
	* This function will create the HTML page to edit logon hours.
	*
	* @return array meta HTML code
	*/
	function display_html_logonHours() {
		$timeZone = 0;
		if (isset($this->moduleSettings['sambaSamAccount_timeZone']) && is_array($this->moduleSettings['sambaSamAccount_timeZone'])) {
			$timeZone = $this->moduleSettings['sambaSamAccount_timeZone'][0];
		}
		$days = array(1 => _('Monday'), 2 => _('Tuesday'), 3 => _('Wednesday'), 4 => _('Thursday'),
			5 => _('Friday'), 6 => _('Saturday'), 0 => _('Sunday'));
		if (!$this->attributes['sambaLogonHours'][0]) {
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
		for ($i = 0; $i < 24*7; $i++) {
			$hr = $i + $timeZone;
			if ($hr < 0) {
				$hr = $hr + 24*7;
			}
			elseif ($hr >= 24*7) {
				$hr = $hr - 24*7;
			}
			if ($i % 7 == 0) {
				$return[0][floor($i / 24)+1] = array('kind' => 'text',
					'text' => '<b>' . $days[floor($i / 24)] . '</b>',
					'td' => array('width' => "11%", 'align' => 'center'));
			}
			$return[$i % 24 + 1][floor($i/24) + 1] = array('kind' => 'input',
				'name' => 'lh_' . $hr,
				'type' => 'checkbox',
				'checked' => $hour[$hr] ? true : false,
				'td' => array('align' => 'center'));
		}
		$return[0][0] = array('kind' => 'text', 'text' => '<b>' . _('Time') . '</b>', 'td' => array('width' => "12.5%"));
		for ($h = 0; $h < 24; $h++) {
			$return[1 + $h][0] = array('kind' => 'text', 'text' => "$h:00 - $h:59");
		}
		$return[] = array(0 => array('kind' => 'text', 'text' => '&nbsp;', 'td' => array('colspan' => 8)));
		$return[] = array(
			0 => array('kind' => 'input', 'name' => 'form_subpage_sambaSamAccount_attributes_submit', 'type' => 'submit', 'value' => _('Ok'), 'td' => array('align' => 'right')),
			1 => array('kind' => 'input', 'name' => 'form_subpage_sambaSamAccount_attributes_abort', 'type' => 'submit', 'value' => _('Cancel'), 'td' => array('align' => 'left'))
		);
		return $return;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		if ($this->get_scope() == 'user') {
			// lists for expiration date
			$day = array(); $mon = array(); $year = array();
			for ( $i=1; $i<=31; $i++ ) $day[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			// use Unix password as Samba password
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use Unix password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_useunixpwd', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'pwdunix')
				);
			// use no password at all
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use no password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'noPassword')
				);
			// account deactivation
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Account is deactivated') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'deactivated')
				);
			// password never expires
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Password does not expire') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'noExpire')
				);
			// expiration date
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Account expiration date')),
				1 => array('kind' => 'table', 'value' => array(
					0 => array (
						0 => array('kind' => 'select', 'name' => 'sambaSamAccount_expire_day', 'options' => $day, 'options_selected' => array('1')),
						1 => array('kind' => 'select', 'name' => 'sambaSamAccount_expire_mon', 'options' => $mon, 'options_selected' => array('1')),
						2 => array('kind' => 'select', 'name' => 'sambaSamAccount_expire_yea', 'options' => $year, 'options_selected' => array('2030'))
					)
				)),
				2 => array('kind' => 'help', 'value' => 'expireDate'));
			// letter of home drive
			$drives = array('-');
			for ($i = 90; $i > 67; $i--) $drives[] = chr($i) . ':';
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home drive') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaHomeDrive', 'options' => $drives, 'options_selected' => array('-')),
				2 => array('kind' => 'help', 'value' => 'homeDrive')
				);
			// path to home directory
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_smbhome', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'homePath')
				);
			// profile path
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Profile path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_profilePath', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'profilePath')
				);
			// logon script
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Logon script') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_logonScript', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'scriptPath')
				);
			// allowed workstations
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Samba workstations') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_userWorkstations', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'workstations')
				);
			// domains
			$sambaDomains = search_domains();
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array()),
				2 => array('kind' => 'help', 'value' => 'domain')
				);
			// Windows group
			$groups = array();
			foreach ($this->rids as $key => $value) {
				$groups[] = array($value, $key);
			}
			$groups[] = array("-", "-");
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Windows group') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_group', 'options' => $groups, 'options_selected' => array('513'), 'descriptiveOptions' => true),
				2 => array('kind' => 'help', 'value' => 'group')
				);
			// logon hours
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Logon hours') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaSamAccount_logonHours', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'logonHoursUpload')
				);
		}
		elseif ($this->get_scope() == 'host') {
			// domains
			$sambaDomains = search_domains();
			$sambaDomainNames = array();
			for ($i = 0; $i < count($sambaDomains); $i++) {
				$sambaDomainNames[] = $sambaDomains[$i]->name;
			}
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array()),
				2 => array('kind' => 'help', 'value' => 'domain')
				);
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
		// special profile options
		// use Unix password
		if ($profile['sambaSamAccount_useunixpwd'][0] == "true") {
			$this->useunixpwd = true;
		}
		elseif ($profile['sambaSamAccount_useunixpwd'][0] == "false") {
			$this->useunixpwd = false;
		}
		// use no password
		if ($profile['sambaSamAccount_sambaAcctFlagsN'][0] == "true") {
			$this->nopwd = true;
		}
		elseif ($profile['sambaSamAccount_sambaAcctFlagsN'][0] == "false") {
			$this->nopwd = false;
		}
		// password expiration
		if ($profile['sambaSamAccount_sambaAcctFlagsX'][0] == "true") {
			$this->noexpire = true;
		}
		elseif ($profile['sambaSamAccount_sambaAcctFlagsX'][0] == "false") {
			$this->noexpire = false;
		}
		// use no password
		if ($profile['sambaSamAccount_sambaAcctFlagsD'][0] == "true") {
			$this->deactivated = true;
		}
		elseif ($profile['sambaSamAccount_sambaAcctFlagsD'][0] == "false") {
			$this->deactivated = false;
		}
		// home drive
		if ($profile['sambaSamAccount_sambaHomeDrive'][0] == "-") {
			$this->attributes['sambaHomeDrive'][0] = '';
		}
		elseif ($profile['sambaSamAccount_sambaHomeDrive'][0]) {
			$this->attributes['sambaHomeDrive'][0] = $profile['sambaSamAccount_sambaHomeDrive'][0];
		}
		// expiration date
		if (isset($profile['sambaSamAccount_expire_day'][0]) && ($profile['sambaSamAccount_expire_day'][0] != "")) {
			$date = mktime(0, 0, 0, intval($profile['sambaSamAccount_expire_mon'][0]),
				intval($profile['sambaSamAccount_expire_day'][0]), intval($profile['sambaSamAccount_expire_yea'][0]));
			$this->attributes['sambaKickoffTime'][0] = $date;
		}
		// domain -> change SID
		if ($this->attributes['sambaSID'][0]) {
			if (isset($profile['sambaSamAccount_sambaDomainName'][0]) && ($profile['sambaSamAccount_sambaDomainName'][0] != "")) {
				$domains = search_domains();
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
			$domains = search_domains();
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
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(	'sambaSamAccount_displayName' => array('<block><key>' . _('Display name') . '</key><value' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomePath' => array('<block><key>' . _('Home path') . '</key><value>' . $this->attributes['sambaHomeDrive'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomeDrive' => array('<block><key>' . _('Home drive') . '</key><value>' . $this->attributes['sambaHomePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaLogonScript' => array('<block><key>' . _('Logon script') . '</key><value>' . $this->attributes['sambaLogonScript'][0] . '</value></block>'),
							'sambaSamAccount_sambaProfilePath' => array('<block><key>' . _('Profile path') . '</key><value>' . $this->attributes['sambaProfilePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaUserWorkstations' => array('<block><key>' . _('Samba workstations') . '</key><value>' . $this->attributes['sambaUserWorkstations'][0] . '</value></block>'),
							'sambaSamAccount_sambaDomainName' => array('<block><key>' . _('Domain') . '</key><value>' . $this->attributes['sambaDomainName'][0] . '</value></block>'),
							'sambaSamAccount_sambaPrimaryGroupSID' => array('<block><key>' . _('Windows group') . '</key><value>' . $this->attributes['sambaPrimaryGroupSID'][0] . '</value></block>'));
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
		$errors = array();
		// get list of Samba 3 domains
		$domains = search_domains();
		// get list of Unix groups and their sambaSID + gidNumber
		$groups = $_SESSION['cache']->get_cache(array('cn', 'sambaSID', 'gidNumber'), 'posixGroup', 'group');
		$groups_k = array_keys($groups);
		$groups_cn = array();
		for ($i = 0; $i < sizeof($groups_k); $i++) {
			if (isset($groups[$groups_k[$i]]['sambaSID'][0])) {
				$groups_cn[$groups[$groups_k[$i]]['cn'][0]]['SID'] = $groups[$groups_k[$i]]['sambaSID'][0];
			}
			if (isset($groups[$groups_k[$i]]['gidNumber'][0])) {
				$groups_cn[$groups[$groups_k[$i]]['cn'][0]]['gid'] = $groups[$groups_k[$i]]['gidNumber'][0];
			}
		}
		if ($this->get_scope() == 'user') {
			for ($i = 0; $i < sizeof($rawAccounts); $i++) {
				if (!in_array("sambaSamAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaSamAccount";
				// displayName
				if ($rawAccounts[$i][$ids['sambaSamAccount_displayName']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_displayName']], 'realname')) {
						$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['sambaSamAccount_displayName']];
					}
					else {
						$errMsg = $this->messages['displayName'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// password
				$partialAccounts[$i]['sambaPwdLastSet'] = time();
				if (!get_preg($rawAccounts[$i][$ids['sambaSamAccount_password']], 'password')) {
					$errMsg = $this->messages['sambaLMPassword'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// use Unix password
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == "") {  // default: use Unix
					$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == 'true') {  // use Unix
						$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					}
					else {  // use given password
						$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
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
				$partialAccounts[$i]['sambaAcctFlags'] = $flags;
				// passsword can be changed
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdCanChange']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_pwdCanChange']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaSamAccount_pwdCanChange']]);
						$time = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
						$partialAccounts[$i]['sambaPwdCanChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdCanChange'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// passsword must be changed
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']]);
						$time = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
						$partialAccounts[$i]['sambaPwdMustChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdMustChange'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// expiration date
				if ($rawAccounts[$i][$ids['sambaSamAccount_expireDate']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_expireDate']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaSamAccount_expireDate']]);
						$time = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
						$partialAccounts[$i]['sambaKickoffTime'] = $time;
					}
					else {
						$errMsg = $this->messages['expireDate'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home drive
				if ($rawAccounts[$i][$ids['sambaSamAccount_homeDrive']] != "") {
					if (eregi("[d-z]:", $rawAccounts[$i][$ids['sambaSamAccount_homeDrive']])) {
						$partialAccounts[$i]['sambaHomeDrive'] = $rawAccounts[$i][$ids['sambaSamAccount_homeDrive']];
					}
					else {
						$errMsg = $this->messages['homeDrive'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home path
				if ($rawAccounts[$i][$ids['sambaSamAccount_homePath']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_homePath']], 'UNC')) {
						$partialAccounts[$i]['sambaHomePath'] = $rawAccounts[$i][$ids['sambaSamAccount_homePath']];
					}
					else {
						$errMsg = $this->messages['homePath'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// profile path
				if ($rawAccounts[$i][$ids['sambaSamAccount_profilePath']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_profilePath']], 'UNC')) {
						$partialAccounts[$i]['sambaProfilePath'] = $rawAccounts[$i][$ids['sambaSamAccount_profilePath']];
					}
					else {
						$errMsg = $this->messages['profilePath'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// logon script
				if ($rawAccounts[$i][$ids['sambaSamAccount_logonScript']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_logonScript']], 'logonscript')) {
						$partialAccounts[$i]['sambaLogonScript'] = $rawAccounts[$i][$ids['sambaSamAccount_logonScript']];
					}
					else {
						$errMsg = $this->messages['logonScript'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// workstations
				if ($rawAccounts[$i][$ids['sambaSamAccount_workstations']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_workstations']], 'workstations')) {
						$partialAccounts[$i]['sambaUserWorkstations'] = $rawAccounts[$i][$ids['sambaSamAccount_workstations']];
					}
					else {
						$errMsg = $this->messages['workstations'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
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
					elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_group']], array_keys($this->rids))) {
						$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->rids[$rawAccounts[$i][$ids['sambaSamAccount_group']]];
					}
					else {
						$errMsg = $this->messages['group'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default domain users
					$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->rids[_('Domain Users')];
				}
				// special user
				if ($rawAccounts[$i][$ids['sambaSamAccount_rid']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaSamAccount_rid']], array_keys($this->rids))) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $this->rids[$rawAccounts[$i][$ids['sambaSamAccount_rid']]];
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
				if ($rawAccounts[$i][$ids['sambaSamAccount_logonHours']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_logonHours']], 'sambaLogonHours')) {
						$partialAccounts[$i]['sambaLogonHours'] = $rawAccounts[$i][$ids['sambaSamAccount_logonHours']];
					}
					else {
						$errMsg = $this->messages['logonHours'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					$partialAccounts[$i]['sambaLogonHours'] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
				}
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
				$partialAccounts[$i]['sambaLMPassword'] = lmPassword(substr($partialAccounts[$i]['uid'], 0, sizeof($partialAccounts[$i]['uid']) - 1));
				$partialAccounts[$i]['sambaNTPassword'] = ntPassword(substr($partialAccounts[$i]['uid'], 0, sizeof($partialAccounts[$i]['uid']) - 1));
				// flags
				$partialAccounts[$i]['sambaAcctFlags'] = "[W          ]";
			}
		}
		return $errors;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP commands which should be executed.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and LDAP commands (array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array());
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return;
			}
			else {
				if (!get_preg($_POST['posixAccount_password'], 'password')) {
					return;
				}
				else {
					
					// sync password
					if (in_array('syncNTPassword', $fields)) {
						$return['mod']['sambaNTPassword'][0] = ntPassword($_POST['posixAccount_password']);
					}
					if (in_array('syncLMPassword', $fields)) {
						$return['mod']['sambaLMPassword'][0] = lmPassword($_POST['posixAccount_password']);
					}
				}
			}
		}
		return $return;
	}

}

?>
