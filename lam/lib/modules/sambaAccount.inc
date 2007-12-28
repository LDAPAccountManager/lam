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
* Manages Samba 2 accounts for users and hosts.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "sambaAccount" for users and hosts.
*
* @package modules
*/
class sambaAccount extends baseModule {

	/**
	* Creates a new sambaAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	public function __construct($scope) {
		// List of well known rids
		$this->rids = array(
			_('Domain admins') => 512, _('Domain users') => 513,
			_('Domain guests') => 514, _('Domain computers') => 515,
			_('Domain controllers') => 516,	_('Domain certificate admins') => 517,
			_('Domain schema admins') => 518, _('Domain enterprise admins') => 519,
			_('Domain policy admins') => 520 );
		// call parent constructor
		parent::__construct($scope);
	}

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['homePath'][0] = array('ERROR', _('Home path'), _('Home path is invalid.'));
		$this->messages['homePath'][1] = array('INFO', _('Home path'), _('Inserted user or group name in home path.'));
		$this->messages['homePath'][2] = array('INFO', _('Account %s:') . ' sambaAccount_homePath', _('Inserted user or group name in home path.'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('INFO', _('Profile path'), _('Inserted user or group name in profile path.'));
		$this->messages['profilePath'][2] = array('INFO', _('Account %s:') . ' sambaAccount_profilePath', _('Inserted user or group name in profile path.'));
		$this->messages['logonScript'][0] = array('ERROR', _('Logon script'), _('Logon script is invalid!'));
		$this->messages['logonScript'][1] = array('INFO', _('Logon script'), _('Inserted user or group name in logon script.'));
		$this->messages['logonScript'][2] = array('ERROR', _('Account %s:') . ' sambaAccount_logonScript', _('Logon script is invalid!'));
		$this->messages['workstations'][0] = array('ERROR', _('Samba workstations'), _('Please enter a comma separated list of host names!'));
		$this->messages['workstations'][1] = array('ERROR', _('Account %s:') . ' sambaAccount_workstations', _('Please enter a comma separated list of host names!'));
		$this->messages['domain'][0] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));
		$this->messages['domain'][1] = array('ERROR', _('Account %s:') . ' sambaAccount_domain', _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));
		$this->messages['lmPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['lmPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['lmPassword'][2] = array('ERROR', _('Account %s:') . ' sambaAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ !'));
		$this->messages['displayName'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_displayName', _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['pwdUnix'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_pwdUnix', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noPassword'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_noPassword', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noExpire'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_noExpire', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['deactivated'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_deactivated', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['pwdCanChange'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_pwdCanChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['pwdMustChange'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_pwdMustChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['homeDrive'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_homeDrive', _('Please enter a valid drive letter.'));
		$this->messages['group'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_group', _('Please enter a valid group RID.'));
		$this->messages['specialUser'][0] = array('ERROR', _('Account %s:') . ' sambaAccount_specialUser', _('Please enter a valid special user name.'));

	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// manages user and host accounts
		$return["account_types"] = array("user", "host");
		if ($this->get_scope() == "host") {
			// LDAP filter
			$return["ldap_filter"] = array('and' => '(uid=*$)', 'or' => "(objectClass=posixAccount)");
		}
		// alias name
		$return["alias"] = _('Samba 2');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('sambaAccount');
		// managed attributes
		$return['attributes'] = array('rid', 'lmPassword', 'ntPassword', 'pwdLastSet', 'logonTime', 'logoffTime',
			'kickoffTime', 'pwdCanChange', 'pwdMustChange', 'acctFlags', 'displayName', 'smbHome', 'homeDrive',
			'scriptPath', 'profilePath', 'userWorkstations', 'primaryGroupID', 'domain');
		// PHP extensions
		$return['extensions'] = array('mhash', 'iconv');
		// profile options
		if ($this->get_scope() == 'user') {
			// set Unix password for Samba
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Use Unix password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaAccount_useunixpwd', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'pwdUnix')
				);
			// set no password
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Use no password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaAccount_acctFlagsN', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'noPassword')
				);
			// password expiry
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Password does not expire') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaAccount_acctFlagsX', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'noExpire')
				);
			// account deactivation
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Account is deactivated') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaAccount_acctFlagsD', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'deactivated')
				);
			// drive letter
			$drives = array();
			for ($i = 90; $i > 67; $i--) $drives[] = chr($i) . ':';
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Home drive') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaAccount_homeDrive', 'options' => $drives, 'options_selected' => array ('Z:')),
				2 => array('kind' => 'help', 'value' => 'homeDrive')
				);
			// path to home directory
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Home path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaAccount_smbhome', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'homePath')
				);
			// path to profile
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Profile path') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaAccount_profilePath', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'profilePath')
				);
			// logon script
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Logon script') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaAccount_scriptPath', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'scriptPath')
				);
			// allowed workstations
			$return['profile_options'][] = array(
				0 => array('kind' => 'text', 'text' => _('Samba workstations') . ': '),
				1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaAccount_userWorkstations', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'workstations')
				);
		}
		// Samba domain
		$return['profile_options'][] = array(
			0 => array('kind' => 'text', 'text' => _('Domain') . ': '),
			1 => array('kind' => 'input', 'type' => 'text', 'name' => 'sambaAccount_domain', 'size' => '20', 'maxlength' => '255', 'value' => ''),
			2 => array('kind' => 'help', 'value' => 'domain')
			);
		// profile checks
		$return['profile_checks']['sambaAccount_smbhome'] = array('type' => 'ext_preg', 'regex' => 'UNC',
			'error_message' => $this->messages['homePath'][0]);
		$return['profile_checks']['sambaAccount_profilePath'] = array('type' => 'ext_preg', 'regex' => 'UNC',
			'error_message' => $this->messages['profilePath'][0]);
		$return['profile_checks']['sambaAccount_scriptPath'] = array('type' => 'ext_preg', 'regex' => 'logonscript',
			'error_message' => $this->messages['logonScript'][0]);
		$return['profile_checks']['sambaAccount_userWorkstations'] = array('type' => 'ext_preg', 'regex' => 'unixhost',
			'error_message' => $this->messages['workstations'][0]);
		$return['profile_checks']['sambaAccount_domain'] = array('type' => 'ext_preg', 'regex' => 'domainname',
			'error_message' => $this->messages['domain'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'sambaAccount_homeDrive' => 'homeDrive',
			'sambaAccount_smbhome' => 'smbHome',
			'sambaAccount_profilePath' => 'profilePath',
			'sambaAccount_scriptPath' => 'scriptPath',
			'sambaAccount_userWorkstations' => 'userWorkstations',
			'sambaAccount_domain' => 'domain'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'displayName',
			'smbHome',
			'homeDrive',
			'scriptPath',
			'profilePath',
			'userWorkstations',
			'domain',
			'description'
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount', 'inetOrgPerson');
		// upload options
		if ($this->get_scope() == "user") {
			$return['upload_columns'] = array(
				array(
					'name' => 'sambaAccount_displayName',
					'description' => _('Display name'),
					'help' => 'displayName',
					'example' => _('Steve Miller')
				),
				array(
					'name' => 'sambaAccount_password',
					'description' => _('Password'),
					'help' => 'password',
					'example' => _('secret')
				),
				array(
					'name' => 'sambaAccount_pwdUnix',
					'description' => _('Use Unix password'),
					'help' => 'pwdUnixUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				),
				array(
					'name' => 'sambaAccount_noPassword',
					'description' => _('Use no password'),
					'help' => 'noPasswordUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				),
				array(
					'name' => 'sambaAccount_noExpire',
					'description' => _('Password does not expire'),
					'help' => 'noExpireUpload',
					'default' => 'true',
					'values' => 'true, false',
					'example' => 'true'
				),
				array(
					'name' => 'sambaAccount_deactivated',
					'description' => _('Account is deactivated'),
					'help' => 'deactivatedUpload',
					'default' => 'false',
					'values' => 'true, false',
					'example' => 'false'
				),
				array(
					'name' => 'sambaAccount_pwdCanChange',
					'description' => _('User can change password'),
					'help' => 'pwdCanChange',
					'default' => '31-12-2030',
					'example' => '15-11-2006'
				),
				array(
					'name' => 'sambaAccount_pwdMustChange',
					'description' => _('User must change password'),
					'help' => 'pwdMustChange',
					'default' => '31-12-2030',
					'example' => '15-10-2006'
				),
				array(
					'name' => 'sambaAccount_homeDrive',
					'description' => _('Home drive'),
					'help' => 'homeDrive',
					'example' => 'k:'
				),
				array(
					'name' => 'sambaAccount_homePath',
					'description' => _('Home path'),
					'help' => 'homePath',
					'example' => _('\\\\server\\homes\\smiller')
				),
				array(
					'name' => 'sambaAccount_profilePath',
					'description' => _('Profile path'),
					'help' => 'profilePath',
					'example' => _('\\\\server\\profiles\\smiller')
				),
				array(
					'name' => 'sambaAccount_logonScript',
					'description' => _('Logon script'),
					'help' => 'scriptPath',
					'example' => 'logon.bat'
				),
				array(
					'name' => 'sambaAccount_workstations',
					'description' => _('Samba workstations'),
					'help' => 'workstations',
					'example' => 'PC01,PC02,PC03'
				),
				array(
					'name' => 'sambaAccount_group',
					'description' => _('Windows group'),
					'help' => 'groupUpload',
					'example' => '1235',
					'default' => '&lt;gidNumber&gt;*2 + 1001'
				),
				array(
					'name' => 'sambaAccount_specialUser',
					'description' => _('Special user'),
					'help' => 'specialUser',
					'example' => _('Domain admins'),
					'values' => implode(", ", array_keys($this->rids))
				),
				array(
					'name' => 'sambaAccount_domain',
					'description' => _('Domain'),
					'help' => 'domain',
					'example' => _('mydomain')
				)
			);
		}
		elseif ($this->get_scope() == "host") {
			$return['upload_columns'] = array(
				array(
					'name' => 'sambaAccount_domain',
					'description' => _('Domain'),
					'help' => 'domain',
					'example' => _('mydomain')
				)
			);
		}
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
				"Text" => _('File name and path relative to the netlogon-share which should be executed on logon. $user and $group are replaced with user and group name.'). ' '. _("Can be left empty.")),
			"userWorkstations" => array(
				"ext" => "FALSE", "Headline" => _("Samba workstations"),
				"Text" => _("List of Samba workstations the user is allowed to login. Empty means every workstation.")),
			"workstations" => array(
				"ext" => "FALSE", "Headline" => _("Samba workstations"),
				"Text" => _("Comma separated list of Samba workstations the user is allowed to login. Empty means every workstation."). ' '. _("Can be left empty.")),
			"group" => array(
				"ext" => "FALSE", "Headline" => _("Windows group name"),
				"Text" => _("If you want to use a well known RID you can selcet a well known group.")),
			"groupUpload" => array(
				"ext" => "FALSE", "Headline" => _("Windows group RID"),
				"Text" => _("This is the RID of the user's primary Windows group.")),
			"specialUser" => array(
				"ext" => "FALSE", "Headline" => _("Special user"),
				"Text" => _("If you want to create domain administrators or other special users use this option.")),
			"domain" => array(
				"ext" => "FALSE", "Headline" => _("Domain"),
				"Text" => _("Windows domain name of account."). ' '. _("Can be left empty."))
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
		$this->useunixpwd = true;
		$this->noexpire = true;
		$this->nopwd = false;
		$this->deactivated = false;
		}

	// Variables
	/** use Unix password as samba password? */
	private $useunixpwd;
	/** use no password? */
	private $nopwd;
	/** password does not expire? */
	private $noexpire;
	/** account deactivated? */
	private $deactivated;

	/** Array of well known rids */
	private $rids;

	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		if ($attrs['gidNumber'][0]=='') return false;
		if ($attrs['uidNumber'][0]=='') return false;
		return true;
	}

	/**
	* This functions returns true if all needed settings are done
	*
	* @return boolean true if ready to save account
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['rid'][0] == '') return false;
		return true;
	}

	/**
	* This function loads the LDAP attributes for this module.
	*
	* @param array $attributes attribute list
	*/
	function load_attributes($attr) {
		parent::load_attributes($attr);
		if (is_string($this->attributes['acctFlags'][0])) {
			if (strpos($this->attributes['acctFlags'][0], "D")) $this->deactivated = true;
			else $this->deactivated = false;
			if (strpos($this->attributes['acctFlags'][0], "N")) $this->nopwd = true;
			else $this->nopwd = false;
			if (strpos($this->attributes['acctFlags'][0], "X")) $this->noexpire = true;
			else $this->noexpire = false;
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
	*/
	function save_attributes() {
		/* Create sambaSID. Can't create it while loading attributes because
		* it's psssible uidNumber has changed
		*/
		$special = false;
		if ($this->attributes['rid'][0] < 1000) $special = true;
		if (!$special) {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$this->attributes['rid'][0] = $attrs['uidNumber'][0]*2+1000;
		}
		$rids = array_values($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++) {
			if ($this->attributes['primaryGroupID'][0] == $rids[$i]) {
				$wrid = true;
				break;
			}
		}
		if (!$wrid) {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$this->attributes['primaryGroupID'][0] = ($attrs['gidNumber'][0]*2)+1001;
		}
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
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
		$this->attributes['domain'][0] = $_POST['domain'];
		// Start character
		$flag = "[";
		if ($_POST['acctFlagsD']) {
			$flag .= "D";
			$this->deactivated = true;
		}
		else {
			$this->deactivated = false;
		}
		if ($_POST['acctFlagsX']) {
			$flag .= "X";
			$this->noexpire = true;
		}
		else {
			$this->noexpire = false;
		}
		if ($_POST['acctFlagsN']) {
			$flag .= "N";
			$this->nopwd = true;
		}
		else {
			$this->nopwd = false;
		}
		if ($_POST['acctFlagsS']) $flag .= "S";
		if ($_POST['acctFlagsH']) $flag .= "H";
		if ($_POST['acctFlagsW']) $flag .= "W";
		if ($_POST['acctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['acctFlags'][0] = $flag;

		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!($this->attributes['displayName'][0] == '') && !(get_preg($this->attributes['displayName'][0], 'realname'))) {
			$errors[] = $this->messages['displayName'][1];
		}

		// host attributes
		if ($this->get_scope()=='host') {
			$this->attributes['primaryGroupID'][0] = $this->rids[_('Domain computers')];
			if ($_POST['ResetSambaPassword'] || (!$this->attributes['lmPassword'][0])) {
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$hostname = $attrs['uid'][0];
				$hostname = substr($hostname, 0, strlen($hostname) - 1);
				$this->attributes['lmPassword'][0] = lmPassword($hostname);
				$this->attributes['ntPassword'][0] = ntPassword($hostname);
				$this->attributes['pwdLastSet'][0] = time();
				}
			}
		// check values for user account
		if ($this->get_scope() == 'user') {
			$this->attributes['pwdCanChange'][0] = mktime(intval($_POST['pwdCanChange_h']), intval($_POST['pwdCanChange_m']),
				intval($_POST['pwdCanChange_s']), intval($_POST['pwdCanChange_mon']), intval($_POST['pwdCanChange_day']),
				intval($_POST['pwdCanChange_yea']));
			$this->attributes['pwdMustChange'][0] = mktime(intval($_POST['pwdMustChange_h']), intval($_POST['pwdMustChange_m']),
				intval($_POST['pwdMustChange_s']), intval($_POST['pwdMustChange_mon']), intval($_POST['pwdMustChange_day']),
				intval($_POST['pwdMustChange_yea']));
			$this->attributes['smbHome'][0] = $_POST['smbHome'];
			$this->attributes['homeDrive'][0] = $_POST['homeDrive'];
			$this->attributes['scriptPath'][0] = $_POST['scriptPath'];
			$this->attributes['profilePath'][0] = $_POST['profilePath'];
			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($_POST['primaryGroupID'] == $rids[$i]) {
					$wrid = true;
					$this->attributes['primaryGroupID'][0] = $this->rids[$rids[$i]];
					break;
				}
			}
			if (!$wrid) {				
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$this->attributes['primaryGroupID'][0] = ($attrs['gidNumber'][0]*2)+1001;
			}
			if ($_POST['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;
			if ($_POST['useunixpwd']) {
				$this->useunixpwd = true;
				$this->attributes['lmPassword'][0] = lmPassword($this->getAccountContainer()->getAccountModule('posixAccount')->getClearTextPassword());
				$this->attributes['ntPassword'][0] = ntPassword($this->getAccountContainer()->getAccountModule('posixAccount')->getClearTextPassword());
				$this->attributes['pwdLastSet'][0] = time();
			}
			else $this->useunixpwd = false;
			if (!$this->useunixpwd && isset($_POST['lmPassword']) && ($_POST['lmPassword'] != '')) {
				if ($_POST['lmPassword'] != $_POST['lmPassword2']) {
					$errors[] = $this->messages['lmPassword'][0];
					unset ($_POST['lmPassword2']);
				}
				else {
					if ( !get_preg($_POST['lmPassword'], 'password')) $errors[] = $this->messages['lmPassword'][1];
					else {
						$this->attributes['lmPassword'][0] = lmPassword($_POST['lmPassword']);
						$this->attributes['ntPassword'][0] = ntPassword($_POST['lmPassword']);
						$this->attributes['pwdLastSet'][0] = time();
					}
				}
			}
			// rid
			$specialNames = array_keys($this->rids);
			if (in_array($_POST['rid'], $specialNames)) {
				$this->attributes['rid'][0] = $this->rids[$_POST['rid']];
			}
			else {
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$this->attributes['rid'][0] = $attrs['uidNumber'][0]*2+1000;
			}
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$this->attributes['smbHome'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['smbHome'][0]);
			$this->attributes['smbHome'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($attrs['gidNumber'][0]), $this->attributes['smbHome'][0]);
			if ($this->attributes['smbHome'][0] != $_POST['smbHome']) $errors[] = $this->messages['homePath'][1];
			$this->attributes['scriptPath'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['scriptPath'][0]);
			$this->attributes['scriptPath'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($attrs['gidNumber'][0]), $this->attributes['scriptPath'][0]);
			if ($this->attributes['scriptPath'][0] != $_POST['scriptPath']) $errors[] = $this->messages['logonScript'][1];
			$this->attributes['profilePath'][0] = str_replace('$user', $attrs['uid'][0], $this->attributes['profilePath'][0]);
			$this->attributes['profilePath'][0] = str_replace('$group', $_SESSION['cache']->getgrnam($attrs['gidNumber'][0]), $this->attributes['profilePath'][0]);
			if ($this->attributes['profiletPath'][0] != $_POST['profilePath']) $errors[] = $this->messages['profilePath'][1];
			if ( (!$this->attributes['smbHome'][0]=='') && (!get_preg($this->attributes['smbHome'][0], 'UNC')))
					$errors[] = $this->messages['homePath'][0];
			if ( (!$this->attributes['scriptPath'][0]=='') && (!get_preg($this->attributes['scriptPath'][0], 'logonscript')))
				$errors[] = $this->messages['logonScript'][0];
			if ( (!$this->attributes['profilePath'][0]=='') && (!get_preg($this->attributes['profilePath'][0], 'UNC')))
					$errors[] = $this->messages['profilePath'][0];
		}
		// check values for host account
		else {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (!$this->attributes['rid'][0]) {
				$this->attributes['rid'][0] = ($attrs['uidNumber'][0] * 2) + 1000;
			}
			if (!$this->attributes['primaryGroupID'][0]) {
				$this->attributes['primaryGroupID'][0] = ($attrs['gidNumber'][0] * 2) + 1001;
			}
		}

		if ((!$this->attributes['domain'][0]=='') && !get_preg($this->attributes['domain'][0], 'domainname'))
			$errors[] = $this->messages['domain'][0];

		return $errors;
	}

	/**
	* Processes user input of the workstation selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_userWorkstations() {
		// Load attributes
		if ($this->get_scope()=='user') {
			if (isset($_POST['availableUserWorkstations']) && isset($_POST['userWorkstations_add'])) { // Add workstations to list
				$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++) {
					if ($workstations[$i]=='') unset($workstations[$i]);
				}
				$workstations = array_values($workstations);
				// Add new // Add workstations
				$workstations = array_merge($workstations, $_POST['availableUserWorkstations']);
				// remove doubles
				$workstations = array_flip($workstations);
				array_unique($workstations);
				$workstations = array_flip($workstations);
				// sort workstations
				sort($workstations);
				// Recreate workstation string
				$this->attributes['userWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
				}
			}
			elseif (isset($_POST['userWorkstations']) && isset($_POST['userWorkstations_remove'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++) {
					if ($workstations[$i]=='') unset($workstations[$i]);
				}
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($_POST['userWorkstations'], $workstations);
				// Recreate workstation string
				$this->attributes['userWorkstations'][0] = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$this->attributes['userWorkstations'][0] = $this->attributes['userWorkstations'][0] . "," . $workstations[$i];
				}
			}
		}
		return array();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Display name')),
			1 => array('kind' => 'input', 'type' => 'text', 'name' => 'displayName', 'size' => '20', 'value' => $this->attributes['displayName'][0]),
			2 => array('kind' => 'help', 'value' => 'displayName'));
		// user attributes
		if ($this->get_scope()=='user') {
			if (isset($this->attributes['pwdCanChange'][0])) $canchangedate = getdate(intval($this->attributes['pwdCanChange'][0]));
			else $canchangedate = getdate(0);
			if (isset($this->attributes['pwdMustChange'][0])) $mustchangedate = getdate(intval($this->attributes['pwdMustChange'][0]));
			else $mustchangedate = getdate(0);
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_h', 'type' => 'hidden', 'value' => $canchangedate['hours']),
					1 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_m', 'type' => 'hidden', 'value' => $canchangedate['minutes']),
					2 => array ( 'kind' => 'input', 'name' => 'pwdCanChange_s', 'type' => 'hidden', 'value' => $canchangedate['seconds']),
					3 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_h', 'type' => 'hidden', 'value' => $mustchangedate['hours']),
					4 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_m', 'type' => 'hidden', 'value' => $mustchangedate['minutes']),
					5 => array ( 'kind' => 'input', 'name' => 'pwdMustChange_s', 'type' => 'hidden', 'value' => $mustchangedate['seconds']),
					6 => array ( 'kind' => 'input', 'name' => 'acctFlagsU', 'type' => 'hidden', 'value' => 'true'));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Samba password') ),
				1 => array('kind' => 'input', 'name' => 'lmPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ''));
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Repeat password') ),
				1 => array('kind' => 'input', 'name' => 'lmPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => ''),
				2 => array('kind' => 'help', 'value' => 'password'));
			if ($this->getAccountContainer()->getAccountModule('posixAccount')->getClearTextPassword() != null) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use Unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd, 'value' => true),
					2 => array ('kind' => 'help', 'value' => 'pwdUnix'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsN', 'type' => 'checkbox', 'checked' => $this->nopwd),
				2 => array ('kind' => 'help', 'value' => 'noPassword'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsX', 'type' => 'checkbox', 'checked' => $this->noexpire),
				2 => array ('kind' => 'help', 'value' => 'noExpire'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'acctFlagsD', 'type' => 'checkbox', 'checked' => $this->deactivated),
				2 => array ('kind' => 'help', 'value' => 'deactivated'));
			for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
			for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
			for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User can change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_day',
						'options' => $mday, 'options_selectd' => $canchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_mon',
						'options' => $mon, 'options_selectd' => $canchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'pwdCanChange_yes',
						'options' => $year, 'options_selectd' => $canchangedate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_day',
						'options' => $mday, 'options_selectd' => $mustchangedate['mday']),
					1 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_mon',
						'options' => $mon, 'options_selectd' => $mustchangedate['mon']),
					2 => array ( 'kind' => 'select', 'name' => 'pwdMustChange_yes',
						'options' => $year, 'options_selectd' => $mustchangedate['year'])))),
				2 => array ( 'kind' => 'help', 'value' => 'pwdMustChange' ));
			for ($i=90; $i>67; $i--) $drives[] = chr($i).':';
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home drive') ),
				1 => array ( 'kind' => 'select', 'name' => 'homeDrive', 'options' => $drives, 'options_selected' => array ($this->attributes['homeDrive'][0])),
				2 => array ( 'kind' => 'help', 'value' => 'homeDrive' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'smbHome', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['smbHome'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'homePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Profile path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'profilePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['profilePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'profilePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Logon script') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'scriptPath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['scriptPath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'scriptPath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba workstations') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_sambaAccount_userWorkstations_open', 'value' => _('Edit workstations')),
				2 => array ( 'kind' => 'help', 'value' => 'userWorkstations' ));

			$names = array_keys($this->rids);
			$options = array();
			$selected = array();
			$wrid = false;
			for ($i=0; $i<count($names); $i++) {
				if ($this->attributes['primaryGroupID'][0] == $this->rids[$names[$i]]) {
					$selected[] = $names[$i];
					$wrid=true;
				}
				else $options[] = $names[$i];
			}
			if ($wrid) {
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$options[] = $_SESSION['cache']->getgrnam($attrs['gidNumber'][0]);
			}
			else {
				$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$selected[] = $_SESSION['cache']->getgrnam($attrs['gidNumber'][0]);
			}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
				1 => array ( 'kind' => 'select', 'name' => 'primaryGroupID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'group' ));
			$options = array();
			$selected = array();
			$wrid = false;
			for ($i=0; $i<count($names); $i++) {
				if ($this->attributes['rid'][0] == $this->rids[$names[$i]]) {
					$selected[] = $names[$i];
					$wrid=true;
				}
				else $options[] = $names[$i];
			}
			if ($wrid) $options[] = "-";
			else $selected[] = "-";
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
				1 => array ( 'kind' => 'select', 'name' => 'rid', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'specialUser' ));
		}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
			1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'domain', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['domain'][0]),
			2 => array ( 'kind' => 'help', 'value' => 'domain' ));
		if ($this->get_scope()=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'acctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword', 'value' => _('Reset')),
				2 => array ( 'kind' => 'help', 'value' => 'resetPassword' ));
		}
		return $return;
	}

	/**
	* This function will create the HTML page to edit the allowed workstations.
	*
	* @return array meta HTML code
	*/
	function display_html_userWorkstations() {
		if ($this->get_scope()=='user') {
			// Get list of all hosts.
			$result = $_SESSION['cache']->get_cache('uid', 'sambaAccount', 'host');
			if (is_array($result)) {
				foreach ($result as $host) $availableUserWorkstations[] = str_replace("$", '', $host[0]);
				sort($availableUserWorkstations, SORT_STRING);
				$result = str_replace(' ', '', $this->attributes['userWorkstations'][0]);
				$userWorkstations = explode (',', $result);
				$availableUserWorkstations = array_delete($userWorkstations, $availableUserWorkstations);
			}
			$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Allowed workstations"), 'value' =>
				array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Allowed workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'userWorkstations', 'size' => '15', 'multiple' => true, 'options' => $userWorkstations)))),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'userWorkstations_add',
					'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'userWorkstations_remove', 'value' => '=>' )),
					2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'userWorkstations' )))),
				2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'availableUserWorkstations', 'size' => '15', 'multiple' => true, 'options' => $availableUserWorkstations))))
					))));

			$return[] = array(
				0 => array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_sambaAccount_attributes_back', 'value' => _('Back')),
				1 => array('kind' => 'text'),
				2 => array('kind' => 'text'));
		}
		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(	'sambaAccount_displayName' => array('<block><key>' . _('Display name') . '</key><value>' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaAccount_smbHome' => array('<block><key>' . _('Home path') . '</key><value>' . $this->attributes['smbHome'][0] . '</value></block>'),
							'sambaAccount_homeDrive' => array('<block><key>' . _('Home drive') . '</key><value>' . $this->attributes['homePath'][0] . '</value></block>'),
							'sambaAccount_scriptPath' => array('<block><key>' . _('Logon script') . '</key><value>' . $this->attributes['scriptPath'][0] . '</value></block>'),
							'sambaAccount_profilePath' => array('<block><key>' . _('Profile path') . '</key><value>' . $this->attributes['profilePath'][0] . '</value></block>'),
							'sambaAccount_userWorkstations' => array('<block><key>' . _('Samba workstations') . '</key><value>' . $this->attributes['userWorkstations'][0] . '</value></block>'),
							'sambaAccount_domain' => array('<block><key>' . _('Domain') . '</key><value>' . $this->attributes['domain'][0] . '</value></block>'),
							'sambaAccount_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
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
		if ($this->get_scope() == 'user') {
			for ($i = 0; $i < sizeof($rawAccounts); $i++) {
				if (!in_array("sambaAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaAccount";
				// displayName
				if ($rawAccounts[$i][$ids['sambaAccount_displayName']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_displayName']], 'realname')) {
						$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['sambaAccount_displayName']];
					}
					else {
						$errMsg = $this->messages['displayName'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// password
				if (!get_preg($rawAccounts[$i][$ids['sambaAccount_password']], 'password')) {
					$errMsg = $this->messages['lmPassword'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// use Unix password
				if ($rawAccounts[$i][$ids['sambaAccount_pwdUnix']] == "") {  // default: use Unix
					$partialAccounts[$i]['lmPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					$partialAccounts[$i]['ntPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaAccount_pwdUnix']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['sambaAccount_pwdUnix']] == 'true') {  // use Unix
						$partialAccounts[$i]['lmPassword'] = lmPassword($rawAccounts[$i][$ids['posixAccount_password']]);
						$partialAccounts[$i]['ntPassword'] = ntPassword($rawAccounts[$i][$ids['posixAccount_password']]);
					}
					else {  // use given password
						$partialAccounts[$i]['lmPassword'] = lmPassword($rawAccounts[$i][$ids['sambaAccount_password']]);
						$partialAccounts[$i]['ntPassword'] = ntPassword($rawAccounts[$i][$ids['sambaAccount_password']]);
					}
				}
				else {
					$errMsg = $this->messages['pwdUnix'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// use no password
				if ($rawAccounts[$i][$ids['sambaAccount_noPassword']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaAccount_noPassword']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaAccount_noPassword']] == 'true') {
							$partialAccounts[$i]['lmPassword'] = 'NO PASSWORD*****';
							$partialAccounts[$i]['ntPassword'] = 'NO PASSWORD*****';
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
				if ($rawAccounts[$i][$ids['sambaAccount_noExpire']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaAccount_noExpire']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaAccount_noExpire']] == 'false') {
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
				if ($rawAccounts[$i][$ids['sambaAccount_deactivated']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaAccount_deactivated']], array('true', 'false'))) {
						if ($rawAccounts[$i][$ids['sambaAccount_deactivated']] == 'true') {
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
				$partialAccounts[$i]['acctFlags'] = $flags;
				// passsword can be changed
				if ($rawAccounts[$i][$ids['sambaAccount_pwdCanChange']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_pwdCanChange']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaAccount_pwdCanChange']]);
						$time = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
						$partialAccounts[$i]['pwdCanChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdCanChange'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// passsword must be changed
				if ($rawAccounts[$i][$ids['sambaAccount_pwdMustChange']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_pwdMustChange']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaAccount_pwdMustChange']]);
						$time = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
						$partialAccounts[$i]['pwdMustChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdMustChange'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home drive
				if ($rawAccounts[$i][$ids['sambaAccount_homeDrive']] != "") {
					if (eregi("[d-z]:", $rawAccounts[$i][$ids['sambaAccount_homeDrive']])) {
						$partialAccounts[$i]['homeDrive'] = $rawAccounts[$i][$ids['sambaAccount_homeDrive']];
					}
					else {
						$errMsg = $this->messages['homeDrive'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// home path
				if ($rawAccounts[$i][$ids['sambaAccount_homePath']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_homePath']], 'UNC')) {
						$partialAccounts[$i]['smbHome'] = $rawAccounts[$i][$ids['sambaAccount_homePath']];
					}
					else {
						$errMsg = $this->messages['homePath'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// profile path
				if ($rawAccounts[$i][$ids['sambaAccount_profilePath']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_profilePath']], 'UNC')) {
						$partialAccounts[$i]['profilePath'] = $rawAccounts[$i][$ids['sambaAccount_profilePath']];
					}
					else {
						$errMsg = $this->messages['profilePath'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// logon script
				if ($rawAccounts[$i][$ids['sambaAccount_logonScript']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_logonScript']], 'logonscript')) {
						$partialAccounts[$i]['scriptPath'] = $rawAccounts[$i][$ids['sambaAccount_logonScript']];
					}
					else {
						$errMsg = $this->messages['logonScript'][2];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// workstations
				if ($rawAccounts[$i][$ids['sambaAccount_workstations']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_workstations']], 'workstations')) {
						$partialAccounts[$i]['userWorkstations'] = $rawAccounts[$i][$ids['sambaAccount_workstations']];
					}
					else {
						$errMsg = $this->messages['workstations'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// group
				if ($rawAccounts[$i][$ids['sambaAccount_group']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_group']], 'digit')) {
						$partialAccounts[$i]['primaryGroupID'] = $rawAccounts[$i][$ids['sambaAccount_group']];
					}
					else {
						$errMsg = $this->messages['group'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default GID*2 + 1001
					$partialAccounts[$i]['primaryGroupID'] = $partialAccounts[$i]['gidNumber']*2 + 1001;
				}
				// domain
				if ($rawAccounts[$i][$ids['sambaAccount_domain']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_domain']], 'domainname')) {
						$partialAccounts[$i]['domain'] = $rawAccounts[$i][$ids['sambaAccount_domain']];
					}
					else {
						$errMsg = $this->messages['domain'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// special user
				if ($rawAccounts[$i][$ids['sambaAccount_specialUser']] != "") {
					if (in_array($rawAccounts[$i][$ids['sambaAccount_specialUser']], array_keys($this->rids))) {
						$partialAccounts[$i]['rid'] = $this->rids[$rawAccounts[$i][$ids['sambaAccount_specialUser']]];
					}
					else {
						$errMsg = $this->messages['specialUser'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					// default RID uid*2 + 1000
					$partialAccounts[$i]['rid'] = $partialAccounts[$i]['uidNumber']*2 + 1000;
				}
			}
		}
		else {  // hosts
			for ($i = 0; $i < sizeof($rawAccounts); $i++) {
				if (!in_array("sambaAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaAccount";
				// domain
				if ($rawAccounts[$i][$ids['sambaAccount_domain']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaAccount_domain']], 'domainname')) {
						$partialAccounts[$i]['domain'] = $rawAccounts[$i][$ids['sambaAccount_domain']];
					}
					else {
						$errMsg = $this->messages['domain'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				// RID (RID uid*2 + 1000)
				$partialAccounts[$i]['rid'] = $partialAccounts[$i]['uidNumber']*2 + 1000;
				// passwords ( = host name)
				$partialAccounts[$i]['lmPassword'] = lmPassword(substr($partialAccounts[$i]['uid'], 0, sizeof($partialAccounts[$i]['uid']) - 1));
				$partialAccounts[$i]['ntPassword'] = ntPassword(substr($partialAccounts[$i]['uid'], 0, sizeof($partialAccounts[$i]['uid']) - 1));
				// flags
				$partialAccounts[$i]['acctFlags'] = "[W          ]";
			}
		}
		return $errors;
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
		if ($profile['sambaAccount_useunixpwd'][0] == "true") {
			$this->useunixpwd = true;
		}
		elseif ($profile['sambaAccount_useunixpwd'][0] == "false") {
			$this->useunixpwd = false;
		}
		// use no password
		if ($profile['sambaAccount_acctFlagsN'][0] == "true") {
			$this->nopwd = true;
		}
		elseif ($profile['sambaAccount_acctFlagsN'][0] == "false") {
			$this->nopwd = false;
		}
		// password expiration
		if ($profile['sambaAccount_acctFlagsX'][0] == "true") {
			$this->noexpire = true;
		}
		elseif ($profile['sambaAccount_acctFlagsX'][0] == "false") {
			$this->noexpire = false;
		}
		// use no password
		if ($profile['sambaAccount_acctFlagsD'][0] == "true") {
			$this->deactivated = true;
		}
		elseif ($profile['sambaAccount_acctFlagsD'][0] == "false") {
			$this->deactivated = false;
		}
	}

}

?>
