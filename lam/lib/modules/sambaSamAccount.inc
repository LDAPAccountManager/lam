<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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

class sambaSamAccount extends baseModule {

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
		$this->messages['homePath'][1] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
		$this->messages['homePath'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_homePath', _('Home path is invalid.'));
		$this->messages['profilePath'][0] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		$this->messages['profilePath'][1] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
		$this->messages['profilePath'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_profilePath', _('Profile path is invalid!'));
		$this->messages['logonScript'][0] = array('ERROR', _('Script path'), _('Script path is invalid!'));
		$this->messages['logonScript'][1] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'));
		$this->messages['logonScript'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_logonScript', _('Logon script is invalid!'));
		$this->messages['workstations'][0] = array('ERROR', _('Samba workstations'), _('Please enter a comma separated list of host names!'));
		$this->messages['workstations'][1] = array('ERROR', _('Account %s:') . ' sambaSamAccount_workstations', _('Please enter a comma separated list of host names!'));
		$this->messages['sambaLMPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
		$this->messages['sambaLMPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['sambaLMPassword'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_password', _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		$this->messages['rid'][0] = array('ERROR', _('Special user'), _('There can be only one administrator per domain.'));
		$this->messages['rid'][1] = array('ERROR', _('Special user'), _('There can be only one guest per domain.'));
		$this->messages['rid'][2] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('Please enter a RID number or the name of a special account!'));
		$this->messages['rid'][3] = array('ERROR', _('Account %s:') . ' sambaSamAccount_rid', _('This is not a valid RID number!'));
		$this->messages['displayName'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_displayName', _('Please enter a valid display name!'));
		$this->messages['pwdUnix'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdUnix', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noPassword'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noPassword', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['noExpire'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_noExpire', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['deactivated'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_deactivated', _('This value can only be \"true\" or \"false\"!'));
		$this->messages['pwdCanChange'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdCanChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['pwdMustChange'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_pwdMustChange', _('Please enter a valid date in format DD-MM-YYYY.'));
		$this->messages['homeDrive'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_homeDrive', _('Please enter a valid drive letter.'));
		$this->messages['domain'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_domain', _('LAM was unable to find a domain with this name!'));
		$this->messages['logonHours'][0] = array('ERROR', _('Account %s:') . ' sambaSamAccount_logonHours', _('The format of the logon hours field is invalid!'));
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
			// this is a base module
			$return["is_base"] = true;
			// LDAP filter
			$return["ldap_filter"] = array('and' => '(uid=*$)', 'or' => "(objectClass=posixAccount)");
		}
		// alias name
		$return["alias"] = _('Samba 3');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
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
		// available PDF fields
		$return['PDF_fields'] = array(
			'displayName',
			'uid',
			'sambaHomePath',
			'sambaHomeDrive',
			'sambaLogonScript',
			'sambaProfilePath',
			'sambaUserWorkstations',
			'sambaDomainName',
			'description',
			'sambaPrimaryGroupSID'
		);
		// help Entries
		$return['help'] = array (
			"displayName" => array(
				"ext" => "FALSE", "Headline" => _("Display name"),
				"Text" => _("This is the account's full name on Windows systems.")),
			"password" => array(
				"ext" => "FALSE", "Headline" => _("Samba password"),
				"Text" => _("This is the account's Windows password.")),
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
				"Text" => _('UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
			"profilePath" => array(
				"ext" => "FALSE", "Headline" => _("Profile path"),
				"Text" => _('Path of the user profile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
			"scriptPath" => array(
				"ext" => "FALSE", "Headline" => _("Logon script"),
				"Text" => _('Filename and -path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
			"workstations" => array(
				"ext" => "FALSE", "Headline" => _("Samba workstations"),
				"Text" => _("Comma separated list of Samba workstations the user is allowed to login. Empty means every workstation."). ' '. _("Can be left empty.")),
			"groupUpload" => array(
				"ext" => "FALSE", "Headline" => _("Windows primary group SID"),
				"Text" => _("This is the SID of the user's primary Windows group.")),
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
			"logonHoursUpload" => array(
				"ext" => "FALSE", "Headline" => _("Logon hours"),
				"Text" => _("This option defines the allowed logon hours for this account. The format is the same as for the LDAP attribute. The 24*7 hours are represented as 168 bit which are saved as 21 hex (21*8 = 168) values. The first bit represents Sunday 0:00 - 0:59 in GMT."))
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
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		$this->useunixpwd=false;
		}

	// Variables
	// use unix password as samba password?
	var $useunixpwd;
	// Array of well known rids
	var $rids;


	/* $attribute['sambaLMPassword'] and $attribute['sambaNTPassword'] can't accessed directly because it's enrcypted
	* To read / write password function sambaLMPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function sambaLMPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			if ($newpassword=='')
				$this->attributes['sambaLMPassword'][0] = '';
			else $this->attributes['sambaLMPassword'][0] = base64_encode($_SESSION['ldap']->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->useunixpwd) return $_SESSION[$this->base]->module['posixAccount']->userPassword();
			if ($this->attributes['sambaLMPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION['ldap']->decrypt(base64_decode($this->attributes['sambaLMPassword'][0]));
				}
			else return '';
			}
		}

	function module_ready() {
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]=='') return false;
		if ($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]=='') return false;
		if ($this->attributes['uid'][0]=='') return false;
		return true;
		}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['sambaSID'][0] == '') return false;
		return true;
		}
	
	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'sambaUserWorkstations');
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);
		// Delete password. We don't want to show an encrypted password because it makes no sense
		$this->sambaLMPassword('');
		return 0;
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
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
				}
		$special = false;
		if ($this->attributes['sambaSID'][0] == $SID."-500") $special = true;
		if ($this->attributes['sambaSID'][0] == $SID."-501") $special = true;
		if (!$special) $this->attributes['sambaSID'][0] == $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2+$RIDbase);
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++)
			if ($this->attributes['sambaPrimaryGroupSID'][0] == $SID . "-" . $rids[$i])
				$wrid = true;
		if (!$wrid) $this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+$RIDbase+1;


		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);

		// unset password when needed
		if (isset($return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaLMPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaLMPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword']);
		if (isset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaNTPassword']))
		unset($return[$_SESSION[$this->base]->dn]['notchanged']['sambaNTPassword']);
		if (!isset($this->orig['sambaLMPassword'][0])) {
			// new account
			$return[$_SESSION[$this->base]->dn]['add']['sambaLMPassword'][0] = lmPassword($this->sambaLMPassword());
			$return[$_SESSION[$this->base]->dn]['add']['sambaNTPassword'][0] = ntPassword($this->sambaLMPassword());
			$return[$_SESSION[$this->base]->dn]['add']['sambaPwdLastSet'][0] = time();
			}
		else
			// edit existing account
			if ($this->sambaLMPassword()!='' && $this->attributes['sambaLMPassword'][0] != $this->orig['sambaLMPassword'][0]) {
				$return[$_SESSION[$this->base]->dn]['modify']['sambaLMPassword'][0] = lmPassword($this->sambaLMPassword());
				$return[$_SESSION[$this->base]->dn]['modify']['sambaNTPassword'][0] = ntPassword($this->sambaLMPassword());
				$return[$_SESSION[$this->base]->dn]['modify']['sambaPwdLastSet'][0] = time();
			}
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Save attributes
		$this->attributes['sambaDomainName'][0] = $post['sambaDomainName'];
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
				}
		$flag = "[";
		if ($post['sambaAcctFlagsD']) $flag .= "D";
		if ($post['sambaAcctFlagsX']) $flag .= "X";
		if ($post['sambaAcctFlagsN']) $flag .= "N";
		if ($post['sambaAcctFlagsS']) $flag .= "S";
		if ($post['sambaAcctFlagsH']) $flag .= "H";
		if ($post['sambaAcctFlagsW']) $flag .= "W";
		if ($post['sambaAcctFlagsU']) $flag .= "U";
		// Expand string to fixed length
		$flag = str_pad($flag, 12);
		// End character
		$flag = $flag. "]";
		$this->attributes['sambaAcctFlags'][0] = $flag;

		if ($_SESSION[$this->base]->type=='host') {
			$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[_('Domain Computers')];
			if ($post['ResetSambaPassword']) {
				$this->sambaLMPassword($_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0]);
				$_SESSION[$this->base]->module['posixAccount']->userPassword('');
				}
			}

		if ($_SESSION[$this->base]->type=='user') {
			$this->attributes['sambaPwdCanChange'][0] = mktime($post['sambaPwdCanChange_h'], $post['sambaPwdCanChange_m'], $post['sambaPwdCanChange_s'],
				$post['sambaPwdCanChange_mon'], $post['sambaPwdCanChange_day'], $post['sambaPwdCanChange_yea']);
			$this->attributes['sambaPwdMustChange'][0] = mktime($post['sambaPwdMustChange_h'], $post['sambaPwdMustChange_m'], $post['sambaPwdMustChange_s'],
				$post['sambaPwdMustChange_mon'], $post['sambaPwdMustChange_day'], $post['sambaPwdMustChange_yea']);
			$this->attributes['sambaHomePath'][0] = stripslashes($post['sambaHomePath']);
			$this->attributes['sambaHomeDrive'][0] = $post['sambaHomeDrive'];
			$this->attributes['sambaLogonScript'][0] = stripslashes($post['sambaLogonScript']);
			$this->attributes['sambaProfilePath'][0] = stripslashes($post['sambaProfilePath']);
			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($post['sambaPrimaryGroupSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".$this->rids[$rids[$i]];
					}
				}
			if (!$wrid) $this->attributes['sambaPrimaryGroupSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixAccount']->attributes['gidNumber'][0]*2)+$RIDbase+1;

			if ($post['useunixpwd']) $this->useunixpwd = true;
				else $this->useunixpwd = false;
			if (isset($post['sambaLMPassword'])) {
				if ($post['sambaLMPassword'] != $post['sambaLMPassword2']) {
					$triggered_messages['sambaLMPassword'][] = $this->messages['sambaLMPassword'][0];
					unset ($post['sambaLMPassword2']);
					}
				else {
						if ( !get_preg($this->sambaLMPassword(), 'password')) $triggered_messages['sambaLMPassword'][] = $this->messages['sambaLMPassword'][1];
							else $this->sambaLMPassword($post['sambaLMPassword']);
					}
			}
			if ($post['sambaSID']== _('Administrator')) {
				$this->attributes['sambaSID'][0] = $SID."-500";
				// Do a check if an administrator already exists
				if ($_SESSION['cache']->in_cache($SID."-500", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
					$triggered_messages['sambaSID'][] = $this->messages['rid'][0];
				}
			else if ($post['sambaSID']== _('Guest')) {
				$this->attributes['sambaSID'][0] = $SID."-501";
				// Do a check if an guest already exists
				if ($_SESSION['cache']->in_cache($SID."-501", 'sambaSID', 'user')!=$_SESSION[$this->base]->dn_orig)
					$triggered_messages['sambaSID'][] = $this->messages['rid'][1];
				}
			else if ($post['sambaSID']== "-") {
				$this->attributes['sambaSID'][0] = $SID."-". (($_SESSION[$this->base]->module['posixAccount']->attributes['uidNumber'][0]*2)+$RIDbase);
			}
			// Check values
			$this->attributes['sambaHomePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaHomePath'][0]);
			$this->attributes['sambaHomePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaHomePath'][0]);
			if ($this->attributes['sambaHomePath'][0] != stripslashes($post['sambaHomePath'])) $triggered_messages['sambaHomePath'][] = $this->messages['homePath'][1];
			$this->attributes['sambaLogonScript'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaLogonScript'][0]);
			$this->attributes['sambaLogonScript'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaLogonScript'][0]);
			if ($this->attributes['sambaLogonScript'][0] != stripslashes($post['sambaLogonScript'])) $triggered_messages['sambaLogonScript'][] = $this->messages['logonScript'][1];
			$this->attributes['sambaProfilePath'][0] = str_replace('$user', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['uid'][0], $this->attributes['sambaProfilePath'][0]);
			$this->attributes['sambaProfilePath'][0] = str_replace('$group', $_SESSION[$this->base]->module['inetOrgPerson']->attributes['gid'][0], $this->attributes['sambaProfilePath'][0]);
			if ($this->attributes['sambaProfilePath'][0] != stripslashes($post['sambaProfilePath'])) $triggered_messages['sambaProfilePath'][] = $this->messages['profilePath'][1];
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC')))
				$triggered_messages['sambaHomePath'][] = $this->messages['homePath'][0];
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript')))
				$triggered_messages['sambaScriptPath'][] = $this->messages['logonScript'][0];
			if (!($this->attributes['sambaProfilePath'][0] == '') &&
				!(get_preg($this->attributes['sambaProfilePath'][0], 'UNC') xor get_preg($this->attributes['sambaProfilePath'][0], 'homeDirectory')))
					$triggered_messages['sambaProfilePath'][] = $this->messages['profilePath'][0];
			}
		else {
			$sambaHomePath = str_replace('$user', 'user', $this->attributes['sambaHomePath'][0]);
			$sambaHomePath = str_replace('$group', 'group', $sambaHomePath);
			$sambaLogonScript = str_replace('$user', 'user', $this->attributes['sambaLogonScript'][0]);
			$sambaLogonScript = str_replace('$group', 'group', $sambaLogonScript);
			$sambaProfilePath = str_replace('$user', 'user', $this->attributes['sambaProfilePath'][0]);
			$sambaProfilePath = str_replace('$group', 'group', $sambaProfilePath);
			if ( (!$this->attributes['sambaHomePath'][0]=='') && (!get_preg($this->attributes['sambaHomePath'][0], 'UNC')))
					$triggered_messages[] = $this->messages['homePath'][0];
			if ( (!$this->attributes['sambaLogonScript'][0]=='') && (!get_preg($this->attributes['sambaLogonScript'][0], 'logonscript')))
				$triggered_messages[] = $this->messages['logonScript'][0];
			if ( (!$this->attributes['sambaProfilePath'][0]=='') && (!get_preg($this->attributes['sambaProfilePath'][0], 'UNC'))
				&& (!get_preg($this->attributes['sambaProfilePath'][0], 'UNC')))
					$triggered_messages[] = $this->messages['profilePath'][0];
			}
		if (count($triggered_messages)!=0) {
			$this->triggered_messages = $triggered_messages;
			return $triggered_messages;
		}
		else $this->triggered_messages = array();
		if ($post['sambaUserWorkstations'])  return 'sambaUserWorkstations';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_sambaUserWorkstations($post) {
		// Load attributes
		if ($_SESSION[$this->base]->type=='user') {
			do { // X-Or, only one if() can be true
				if (isset($post['availableSambaUserWorkstations']) && isset($post['sambaUserWorkstations_add'])) { // Add workstations to list
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Add new // Add workstations
					$workstations = array_merge($workstations, $post['availableSambaUserWorkstations']);
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
					break;
					}
				if (isset($post['sambaUserWorkstations']) && isset($post['sambaUserWorkstations_remove'])) { // remove // Add workstations from list
					// Put all workstations in array
					$temp = str_replace(' ', '', $this->attributes['sambaUserWorkstations'][0]);
					$workstations = explode (',', $temp);
					for ($i=0; $i<count($workstations); $i++)
						if ($workstations[$i]=='') unset($workstations[$i]);
					$workstations = array_values($workstations);
					// Remove unwanted workstations from array
					$workstations = array_delete($post['sambaUserWorkstations'], $workstations);
					// Recreate workstation string
					$this->attributes['sambaUserWorkstations'][0] = $workstations[0];
					for ($i=1; $i<count($workstations); $i++) {
						$this->attributes['sambaUserWorkstations'][0] = $this->attributes['sambaUserWorkstations'][0] . "," . $workstations[$i];
						}
					break;
					}
				} while(0);
			if ($post['attributes']) return 'attributes';
			}
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			$sambaDomainNames[] = $sambaDomains[$i]->name;
			if ($this->attributes['sambaDomainName'][0] == $sambaDomains[$i]->name)
				$SID = $sambaDomains[$i]->SID;
			}
		$canchangedate = getdate($this->attributes['sambaPwdCanChange'][0]);
		$mustchangedate = getdate($this->attributes['sambaPwdMustChange'][0]);

		if ($_SESSION[$this->base]->type=='user') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_h', 'type' => 'hidden', 'value' => $canchangedate['hours']),
					1 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_m', 'type' => 'hidden', 'value' => $canchangedate['minutes']),
					2 => array ( 'kind' => 'input', 'name' => 'sambaPwdCanChange_s', 'type' => 'hidden', 'value' => $canchangedate['seconds']),
					3 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_h', 'type' => 'hidden', 'value' => $mustchangedate['hours']),
					4 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_m', 'type' => 'hidden', 'value' => $mustchangedate['minutes']),
					5 => array ( 'kind' => 'input', 'name' => 'sambaPwdMustChange_s', 'type' => 'hidden', 'value' => $mustchangedate['seconds']),
					6 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsU', 'type' => 'hidden', 'value' => 'true'));
			if ($post['sambaLMPassword2']!='') $password2 = $post['sambaLMPassword2'];
			else $password2 = $this->sambaLMPassword();
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $this->sambaLMPassword()));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaLMPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
				2 => array ('kind' => 'help', 'value' => 'sambaLMPassword'));
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use unix password') ),
					1 => array ( 'kind' => 'input', 'name' => 'useunixpwd', 'type' => 'checkbox', 'checked' => $this->useunixpwd),
					2 => array ('kind' => 'help', 'value' => 'useunixpwd'));
				}
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "N")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsN'));
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "X")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password does not expire') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsX'));
			$checked = false;
			if (strpos($this->attributes['sambaAcctFlags'][0], "D")) $checked = true;
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account is deactivated') ),
				1 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => $checked, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'sambaAcctFlagsD'));
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
				2 => array ( 'kind' => 'help', 'value' => 'sambaPwdCanChange' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('User must change password') ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_day',
						'options' => $mday, 'options_selected' => array($mustchangedate['mday'])),
					1 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_mon',
						'options' => $mon, 'options_selected' => array($mustchangedate['mon'])),
					2 => array ( 'kind' => 'select', 'name' => 'sambaPwdMustChange_yea',
						'options' => $year, 'options_selected' => array($mustchangedate['year']))))),
				2 => array ( 'kind' => 'help', 'value' => 'sambaPwdMustChange' ));
			for ($i=90; $i>67; $i--) $drives[] = chr($i).':';
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home drive') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaHomeDrive', 'options' => $drives, 'options_selected' => array ($this->attributes['sambaHomeDrive'][0])),
				2 => array ( 'kind' => 'help', 'value' => 'sambaHomeDrive' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaHomePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaHomePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaHomePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Profile path') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaProfilePath', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaProfilePath'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaProfilePath' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Logon script') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'sambaLogonScript', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['sambaLogonScript'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaLogonScript' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Samba workstations') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations', 'value' => _('Edit workstations')),
				2 => array ( 'kind' => 'help', 'value' => 'sambaUserWorkstations' ));

			$names = array_keys($this->rids);
			$wrid=false;
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
				2 => array ( 'kind' => 'help', 'value' => 'sambaPrimaryGroupSID' ));
			// Display if group SID should be mapped to a well kown SID
			$wrid=false;
			$options = array();
			$selected = array();
			if ($this->attributes['sambaSID'][0]==$SID."-500") {
				$selected[] = _('Administrator');
				$wrid=true;
				}
			else $options[] = _('Administrator');
			if ($this->attributes['sambaSID'][0]==$SID."-501") {
				$selected[] = _('Guest');
				$wrid=true;
				}
			else $options[] = _('Guest');
			if ($wrid) $options[] = "-";
				else $selected[] = "-";
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Special user') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaSID', 'options' => $options, 'options_selected' => $selected),
				2 => array ( 'kind' => 'help', 'value' => 'sambaSID' ));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array($this->attributes['sambaDomainName'][0])),
			2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
		if ($_SESSION[$this->base]->type=='host') {
			$return[] = array ( 0 => array ( 'kind' => 'input', 'name' => 'sambaAcctFlagsW', 'type' => 'hidden', 'value' => 'true' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Reset password') ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'ResetSambaPassword'),
				2 => array ( 'kind' => 'help', 'value' => 'ResetSambaPassword' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
				1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => $this->attributes['sambaDomainName'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
			}
		return $return;
		}

	function display_html_delete($post) {
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_sambaUserWorkstations($post) {
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

			$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Allowed workstations"), 'value' =>
				array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Allowed workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'sambaUserWorkstations[]', 'size' => '15', 'multiple', 'options' => $userWorkstations)))),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_add',
					'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'sambaUserWorkstations_remove', 'value' => '=>' )),
					2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'sambaUserWorkstations' )))),
				2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available workstations"), 'value' =>
					array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'availableSambaUserWorkstations[]', 'size' => '15', 'multiple', 'options' => $availableUserWorkstations))))
					))));

			$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'value' => _('Back') ),
				1 => array ( 'kind' => 'text'),
				2 => array ('kind' => 'text'));
			}
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
			// use Unix password as Samba password
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use unix password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_useunixpwd', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'pwdunix')
				);
			// use no password at all
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Use no password') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsN', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'noPassword')
				);
			// password never expires
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Password does not expire') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsX', 'type' => 'checkbox', 'checked' => true),
				2 => array('kind' => 'help', 'value' => 'noExpire')
				);
			// account deactivation
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Account is deactivated') . ': '),
				1 => array('kind' => 'input', 'name' => 'sambaSamAccount_sambaAcctFlagsD', 'type' => 'checkbox', 'checked' => false),
				2 => array('kind' => 'help', 'value' => 'deactivated')
				);
			// letter of home drive
			$drives = array();
			for ($i = 90; $i > 67; $i--) $drives[] = chr($i) . ':';
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Home drive') . ': '),
				1 => array('kind' => 'select', 'name' => 'sambaSamAccount_sambaHomeDrive', 'options' => $drives, 'options_selected' => array('Z:')),
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
			$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
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
		elseif ($this->get_scope() == 'host') {
			// domains
			$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
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
	
	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array(	'sambaSamAccount_displayName' => array('<block><key>' . _('Display name') . '</key><value' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaSamAccount_uid' => array('<block><key>' . _('Username') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomePath' => array('<block><key>' . _('Home path') . '</key><value>' . $this->attributes['sambaHomeDrive'][0] . '</value></block>'),
							'sambaSamAccount_sambaHomeDrive' => array('<block><key>' . _('Home drive') . '</key><value>' . $this->attributes['sambaHomePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaLogonScript' => array('<block><key>' . _('Logon script') . '</key><value>' . $this->attributes['sambaLogonScript'][0] . '</value></block>'),
							'sambaSamAccount_sambaProfilePath' => array('<block><key>' . _('Profile path') . '</key><value>' . $this->attributes['sambaProfilePath'][0] . '</value></block>'),
							'sambaSamAccount_sambaUserWorkstations' => array('<block><key>' . _('Samba workstations') . '</key><value>' . $this->attributes['sambaUserWorkstations'][0] . '</value></block>'),
							'sambaSamAccount_sambaDomainName' => array('<block><key>' . _('Domain') . '</key><value>' . $this->attributes['sambaDomainName'][0] . '</value></block>'),
							'sambaSamAccount_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'),
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
		$triggered_messages = array();
		// get list of Samba 3 domains
		$domains = search_domains($_SESSION['config']->get_DomainSuffix());
		// get list of Unix groups and their sambaSID + gidNumber
		$groups = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
		$groups_SID = $_SESSION['cache']->get_cache('sambaSID', 'sambaGroupMapping', 'group');
		$groups_gid = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
		$groups_k = array_keys($groups);
		for ($i = 0; $i < sizeof($groups_k); $i++) {
			$groups[$groups_k[$i]] = array('cn' => $groups[$groups_k[$i]][0]);
			if (isset($groups_SID[$groups_k[$i]][0])) $groups[$groups_k[$i]]['SID'] = $groups_SID[$groups_k[$i]][0];
			if (isset($groups_gid[$groups_k[$i]][0])) $groups[$groups_k[$i]]['gid'] = $groups_gid[$groups_k[$i]][0];
		}
		$groups_cn = array();
		for ($i = 0; $i < sizeof($groups_k); $i++) {
			$groups_cn[$groups[$groups_k[$i]]['cn']] = $groups[$groups_k[$i]];
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
						$triggered_messages[] = $errMsg;
					}
				}
				// password
				if (!get_preg($rawAccounts[$i][$ids['sambaSamAccount_password']], 'password')) {
					$errMsg = $this->messages['sambaLMPassword'][2];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
				}
				// use Unix password
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == "") {  // default: use Unix
					$partialAccounts[$i]['sambaLMPassword'] = lmPassword($partialAccounts[$i]['userPassword']);
					$partialAccounts[$i]['sambaNTPassword'] = ntPassword($partialAccounts[$i]['userPassword']);
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['sambaSamAccount_pwdUnix']] == 'true') {  // use Unix
						$partialAccounts[$i]['sambaLMPassword'] = lmPassword($partialAccounts[$i]['userPassword']);
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($partialAccounts[$i]['userPassword']);
					}
					else {  // use given password
						$partialAccounts[$i]['sambaLMPassword'] = lmPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
						$partialAccounts[$i]['sambaNTPassword'] = ntPassword($rawAccounts[$i][$ids['sambaSamAccount_password']]);
					}
				}
				else {
					$errMsg = $this->messages['pwdUnix'][0];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$time = mktime(0, 0, 0, $parts[1], $parts[0], $parts[2]);
						$partialAccounts[$i]['sambaPwdCanChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdCanChange'][0];
						array_push($errMsg, array($i));
						$triggered_messages[] = $errMsg;
					}
				}
				// passsword must be changed
				if ($rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']], 'date')) {
						$parts = explode("-", $rawAccounts[$i][$ids['sambaSamAccount_pwdMustChange']]);
						$time = mktime(0, 0, 0, $parts[1], $parts[0], $parts[2]);
						$partialAccounts[$i]['sambaPwdMustChange'] = $time;
					}
					else {
						$errMsg = $this->messages['pwdMustChange'][0];
						array_push($errMsg, array($i));
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
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
						$triggered_messages[] = $errMsg;
					}
				}
				else {
					// default domain users
					$partialAccounts[$i]['sambaPrimaryGroupSID'] = $domains[$domIndex]->SID . '-' . $this->rids['Domain Users'];
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
						$triggered_messages[] = $errMsg;
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
						$errMsg = $this->messages['logonHours'][0];
						array_push($errMsg, array($i));
						$triggered_messages[] = $errMsg;
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
					$triggered_messages[] = $errMsg;
				}
				// RID
				if ($rawAccounts[$i][$ids['sambaSamAccount_rid']] != "") {
					if (get_preg($rawAccounts[$i][$ids['sambaSamAccount_rid']], 'digit')) {
						$partialAccounts[$i]['sambaSID'] .= '-' . $rawAccounts[$i][$ids['sambaSamAccount_rid']];
					}
					else {
						$errMsg = $this->messages['rid'][3];
						array_push($errMsg, array($i));
						$triggered_messages[] = $errMsg;
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
		return $triggered_messages;
	}

}

?>
