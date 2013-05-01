<?php
/*
	$Id$

	This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
	Copyright (C) 2003 - 2006  Tilo Lutz
    Copyright (C) 2005 - 2013  Roland Gruber

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
* Manages Unix accounts for users and hosts.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
* @author Thomas Manninger
*/

/**
* Manages the object class "posixAccount" for users and hosts.
*
* @package modules
*/
class posixAccount extends baseModule implements passwordService {

	// Variables
	/** delimiter for lamdaemon commands */
	private static $SPLIT_DELIMITER = "###x##y##x###";
	
	/* These two variables keep an array of groups the user is also member of. */
	/** current group list */
	private $groups;
	/** original group list */
	private $groups_orig;

	/* list of group of names that the user is member of */
	/** current group of names list */
	private $gonList = array();
	/** original group of names list */
	private $gonList_orig = array();
	/** lamdaemon servers */
	private $lamdaemonServers = array();
	/** cache for group objects */
	private $groupCache = null;
	/** cache for group of names objects */
	private $gonCache = null;
	/** clear text password */
	private $clearTextPassword;
	/** caches the list of known UIDs */
	private $cachedUIDList = null;
	/** caches the list of known user names */
	private $cachedUserNameList = null;

	/**
	 *  This function fills the error message array with messages.
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['minUID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Minimum UID number'), _("Minimum UID number is invalid!"));
		$this->messages['maxUID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number is invalid!"));
		$this->messages['minMachine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Minimum UID number'), _("Minimum UID number is invalid!"));
		$this->messages['maxMachine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number is invalid!"));
		$this->messages['cmp_UID'][0] = array('ERROR', _('Users') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number must be greater than minimum UID number!"));
		$this->messages['cmp_Machine'][0] = array('ERROR', _('Hosts') . ': &nbsp;' . _('Maximum UID number'), _("Maximum UID number must be greater than minimum UID number!"));
		$this->messages['cmp_both'][0] = array('ERROR', _('UID ranges for Unix accounts'), _("The UID ranges for users and hosts overlap! This is a problem because LAM uses the highest UID in use + 1 for new accounts. Please set the minimum UID to equal values or use independent ranges."));
		$this->messages['homeDirectory'][0] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
		$this->messages['homeDirectory'][1] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
		$this->messages['homeDirectory'][2] = array('ERROR', _('Account %s:') . ' posixAccount_homedir', _('Homedirectory contains invalid characters.'));
		$this->messages['homeDirectory'][3] = array('INFO', _('Home directory'), _('Home directory changed. To keep home directory you have to run the following command as root: \'mv %s %s\''));
		$this->messages['uidNumber'][1] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
		$this->messages['uidNumber'][2] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
		$this->messages['uidNumber'][3] = array('ERROR', _('ID-Number'), _('ID is already in use'));
		$this->messages['uidNumber'][4] = array('ERROR', _('Account %s:') . ' posixAccount_uid', _('UID must be a number. It has to be inside the UID range which is defined in your configuration profile.'));
		$this->messages['uidNumber'][5] = array('INFO', _('UID number'), _('UID number has changed. To keep file ownership you have to run the following command as root: \'find / -uid %s -exec chown %s {} \;\''));
		$this->messages['userPassword'][0] = array('ERROR', _('Password'), _('Please enter the same password in both password fields.'));
		$this->messages['userPassword'][1] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['userPassword'][4] = array('ERROR', _('Account %s:') . ' posixAccount_password', _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!');
		$this->messages['uid'][0] = array('INFO', _('UID'), _('UID has changed. Do you want to change home directory?'));
		$this->messages['uid'][1] = array('WARN', _('User name'), _('You are using capital letters. This can cause problems because Windows is not case-sensitive.'));
		$this->messages['uid'][2] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][3] = array('WARN', _('Host name'), _('You are using capital letters. This can cause problems because Windows is not case-sensitive.'));
		$this->messages['uid'][4] = array('ERROR', _('Host name'), _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][5] = array('WARN', _('User name'), _('User name in use. Selected next free user name.'));
		$this->messages['uid'][6] = array('WARN', _('Host name'), _('Host name in use. Selected next free host name.'));
		$this->messages['uid'][7] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][8] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][9] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name already exists!') . ' ' . _('You might want to use %s instead of %s.'));
		$this->messages['uid'][10] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Host name already exists!') . ' ' . _('You might want to use %s instead of %s.'));
		$this->messages['gidNumber'][0] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('LAM was unable to find a group with this name!'));
		$this->messages['gidNumber'][1] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('This GID number is invalid! Please provide either a number or a group name.'));
		$this->messages['gidNumber'][2] = array('INFO', _('GID number'), _('GID number has changed. To keep file ownership you have to run the following command as root: \'find / -gid %s -uid %s -exec chgrp %s {} \;\''));
		$this->messages['gecos'][0] = array('ERROR', _('Account %s:') . ' posixAccount_gecos', _('This gecos value is invalid!'));
		$this->messages['shell'][0] = array('ERROR', _('Account %s:') . ' posixAccount_shell', _('This login shell is invalid!'));
		$this->messages['passwordDisabled'][0] = array('ERROR', _('Account %s:') . ' posixAccount_passwordDisabled', _('This value can only be "true" or "false".'));
		$this->messages['cn'][0] = array('ERROR', _('Common name'), _('Please enter a valid common name!'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' inetOrgPerson_cn', _('Please enter a valid common name!'));
		$this->messages['sambaIDPoolDN'][0] = array('ERROR', _('Samba ID pool DN'), _('This is not a valid DN!'));
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
		$return['icon'] = 'tux.png';
		// manages user and host accounts
		$return["account_types"] = array("user", "host");
		// user specific data
		if ($this->get_scope() == "user") {
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixAccount)", 'and' => "(!(uid=*$))");
			// module dependencies
			$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		}
		elseif ($this->get_scope() == "host") {
			// LDAP filter
			$return["ldap_filter"] = array('or' => "(objectClass=posixAccount)");
			// module dependencies
			$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		}
		// alias name
		$return["alias"] = _("Unix");
		// RDN attributes
		$return["RDN"] = array("uid" => "high", "cn" => "low");
		// managed object classes
		$return['objectClasses'] = array('posixAccount');
		// LDAP aliases
		$return['LDAPaliases'] = array('commonName' => 'cn', 'userid' => 'uid');
		// managed attributes
		$return['attributes'] = array('cn', 'uid', 'uidNumber', 'gidNumber', 'homeDirectory',
			'userPassword', 'loginShell', 'gecos', 'INFO.userPasswordClearText');
		if ($this->get_scope() == "user") {
			// self service search attributes
			$return['selfServiceSearchAttributes'] = array('uid');
			// self service field settings
			$return['selfServiceFieldSettings'] = array('password' => _('Password'), 'cn' => _('Common name'), 'loginShell' => _('Login shell'));
			// possible self service read-only fields
			$return['selfServiceReadOnlyFields'] = array('cn', 'loginShell');
			// self service configuration settings
			$selfServiceContainer = new htmlTable();
			$selfServiceContainer->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', getSupportedHashTypes(),
				array('SSHA'), _("Password hash type")));
			$selfServiceContainer->addElement(new htmlHelpLink('pwdHash', get_class($this)), true);
			$selfServiceContainer->addElement(new htmlTableExtendedInputTextarea('posixAccount_shells', implode("\r\n", $this->getShells()), 30, 4, _('Login shells')));
			$loginShellsHelp = new htmlHelpLink('loginShells', get_class($this));
			$loginShellsHelp->alignment = htmlElement::ALIGN_TOP;
			$selfServiceContainer->addElement($loginShellsHelp, true);
			$return['selfServiceSettings'] = $selfServiceContainer;
		}
		// profile checks
		$return['profile_checks']['posixAccount_homeDirectory'] = array('type' => 'ext_preg', 'regex' => 'homeDirectory',
		'error_message' => $this->messages['homeDirectory'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'posixAccount_homeDirectory' => 'homeDirectory',
			'posixAccount_loginShell' => 'loginShell'
		);
		// configuration options
		$configUserContainer = new htmlTable();
		$configUserContainer->addElement(new htmlSubTitle(_("Users")), true);
		$uidGeneratorSelect = new htmlTableExtendedSelect('posixAccount_uidGeneratorUsers', array(_('Fixed range') => 'range', _('Samba ID pool') => 'sambaPool'), array('range'), _('UID generator'), 'uidGenerator');
		$uidGeneratorSelect->setHasDescriptiveElements(true);
		$uidGeneratorSelect->setTableRowsToHide(array('range' => array('posixAccount_sambaIDPoolDNUsers'), 'sambaPool' => array('posixAccount_minUID', 'posixAccount_maxUID')));
		$uidGeneratorSelect->setTableRowsToShow(array('range' => array('posixAccount_minUID', 'posixAccount_maxUID'), 'sambaPool' => array('posixAccount_sambaIDPoolDNUsers')));
		$configUserContainer->addElement($uidGeneratorSelect, true);
		$uidUsersGeneratorDN = new htmlTableExtendedInputField(_('Samba ID pool DN'), 'posixAccount_sambaIDPoolDNUsers', null, 'sambaIDPoolDN');
		$uidUsersGeneratorDN->setRequired(true);
		$configUserContainer->addElement($uidUsersGeneratorDN, true);
		$minUid = new htmlTableExtendedInputField(_('Minimum UID number'), 'posixAccount_minUID', null, 'minMaxUser');
		$minUid->setRequired(true);
		$configUserContainer->addElement($minUid, true);
		$maxUid = new htmlTableExtendedInputField(_('Maximum UID number'), 'posixAccount_maxUID', null, 'minMaxUser');
		$maxUid->setRequired(true);
		$configUserContainer->addElement($maxUid, true);
		$configUserContainer->addElement(new htmlTableExtendedInputField(_('Suffix for UID/user name check'), 'posixAccount_uidCheckSuffixUser', '', 'uidCheckSuffix'), true);
		$return['config_options']['user'] = $configUserContainer;
		$configHostContainer = new htmlTable();
		$configHostContainer->addElement(new htmlSubTitle(_("Hosts")), true);
		$uidHostGeneratorSelect = new htmlTableExtendedSelect('posixAccount_uidGeneratorHosts', array(_('Fixed range') => 'range', _('Samba ID pool') => 'sambaPool'), array('range'), _('UID generator'), 'uidGenerator');
		$uidHostGeneratorSelect->setHasDescriptiveElements(true);
		$uidHostGeneratorSelect->setTableRowsToHide(array('range' => array('posixAccount_sambaIDPoolDNHosts'), 'sambaPool' => array('posixAccount_minMachine', 'posixAccount_maxMachine')));
		$uidHostGeneratorSelect->setTableRowsToShow(array('range' => array('posixAccount_minMachine', 'posixAccount_maxMachine'), 'sambaPool' => array('posixAccount_sambaIDPoolDNHosts')));
		$configHostContainer->addElement($uidHostGeneratorSelect, true);
		$uidHostsGeneratorDN = new htmlTableExtendedInputField(_('Samba ID pool DN'), 'posixAccount_sambaIDPoolDNHosts', null, 'sambaIDPoolDN');
		$uidHostsGeneratorDN->setRequired(true);
		$configHostContainer->addElement($uidHostsGeneratorDN, true);
		$minUid = new htmlTableExtendedInputField(_('Minimum UID number'), 'posixAccount_minMachine', null, 'minMaxHost');
		$minUid->setRequired(true);
		$configHostContainer->addElement($minUid, true);
		$maxUid = new htmlTableExtendedInputField(_('Maximum UID number'), 'posixAccount_maxMachine', null, 'minMaxHost');
		$maxUid->setRequired(true);
		$configHostContainer->addElement($maxUid, true);
		$configHostContainer->addElement(new htmlTableExtendedInputField(_('Suffix for UID/user name check'), 'posixAccount_uidCheckSuffixHost', '', 'uidCheckSuffix'), true);
		$return['config_options']['host'] = $configHostContainer;
		$configOptionsContainer = new htmlTable();
		$configOptionsContainer->addElement(new htmlSubTitle(_('Options')), true);
		$configOptionsContainer->addElement(new htmlTableExtendedSelect('posixAccount_pwdHash', getSupportedHashTypes(),
			array('SSHA'), _("Password hash type"), 'pwdHash'), true);
		$configOptionsContainer->addElement(new htmlTableExtendedInputTextarea('posixAccount_shells', implode("\r\n", $this->getShells()), 30, 4, _('Login shells'), 'loginShells'), true);
		$hiddenOptionsContainer = new htmlGroup();
		$hiddenOptionsContainer->colspan = 5;
		$hiddenOptionsContainerHead = new htmlTable();
		$hiddenOptionsContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$hiddenOptionsContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$hiddenOptionsContainer->addElement($hiddenOptionsContainerHead);
		$configContainerOptions = new htmlTable();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('posixAccount_hidegecos', false, _('Gecos'), null, false));
		if (isset($_SESSION['conf_config'])) {
			$confActiveGONModules = array_merge($_SESSION['conf_config']->get_AccountModules('group'), $_SESSION['conf_config']->get_AccountModules('gon'));
			if (in_array('groupOfNames', $confActiveGONModules) || in_array('groupOfUniqueNames', $confActiveGONModules)) {
				$configContainerOptions->addElement(new htmlOutputText(' '));
				$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('posixAccount_hidegon', false, _('Groups of names'), null, false));
				$configContainerOptions->addElement(new htmlOutputText(' '));
				$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('posixAccount_hideposixGroups', false, _('Unix groups'), null, false));
			}
		}		
		$hiddenOptionsContainer->addElement($configContainerOptions);
		$configOptionsContainer->addElement($hiddenOptionsContainer, true);
		$advancedOptions = new htmlTable();
		$advancedOptions->addElement(new htmlTableExtendedInputCheckbox('posixAccount_primaryGroupAsSecondary', false, _('Set primary group as memberUid'), 'primaryGroupAsSecondary'), true);
		$advancedOptions->addElement(new htmlTableExtendedInputField(_('User name suggestion'), 'posixAccount_userNameSuggestion', '@givenname@%sn%', 'userNameSuggestion'));
		$advancedOptionsAccordion = new htmlAccordion('posixAccountAdvancedOptions', array(_('Advanced options') => $advancedOptions), false);
		$advancedOptionsAccordion->colspan = 5;
		$configOptionsContainer->addElement($advancedOptionsAccordion);
		
		$return['config_options']['all'] = $configOptionsContainer;
		// upload
		$return['upload_preDepends'] = array('inetOrgPerson');
		// user specific upload options
		if (($this->get_scope() == 'user') && isset($_SESSION['loggedIn']) && ($_SESSION['loggedIn'] === true)) {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_userName',
				'description' => _('User name'),
				'help' => 'uid',
				'example' => _('smiller'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller')
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uidNumber',
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group_upload',
				'example' => _('users'),
				'required' => true
			),
			array(
				'name' => 'posixAccount_additionalGroups',
				'description' => _('Additional groups'),
				'help' => 'addgroup_upload',
				'example' => _('group01,group02')
			),
			array(
				'name' => 'posixAccount_homedir',
				'description' => _('Home directory'),
				'help' => 'homeDirectory_upload',
				'example' => _('/home/smiller'),
				'default' => '/home/{posixAccount_userName}'
			),
			array(
				'name' => 'posixAccount_createHomeDir',
				'description' => _('Create home directory'),
				'help' => 'createhomedir',
				'example' => 'localhost',
				'values' => $_SESSION['config']->get_scriptServers()
			),
			array(
				'name' => 'posixAccount_shell',
				'description' => _('Login shell'),
				'help' => 'loginShell',
				'example' => '/bin/bash',
				'values' => implode(", ", $this->getShells()),
				'default' => '/bin/bash'
			),
			array(
				'name' => 'posixAccount_password',
				'description' => _('Password'),
				'help' => 'userPassword',
				'example' => _('secret')
			),
			array(
				'name' => 'posixAccount_passwordDisabled',
				'description' => _('Lock password'),
				'help' => 'userPassword_lock',
				'example' => 'false',
				'values' => 'true, false',
				'default' => 'false'
			),
			);
			if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
				$return['upload_columns'][] = array(
					'name' => 'posixAccount_gecos',
					'description' => _('Gecos'),
					'help' => 'gecos',
					'example' => _('Steve Miller,Room 2.14,123-123-1234,123-123-1234')
				);
			}
			if (self::areGroupOfNamesActive()) {
				$return['upload_columns'][] = array(
					'name' => 'posixAccount_gon',
					'description' => _('Groups of names'),
					'help' => 'addgroup_upload',
					'example' => _('group01,group02')
				);
			}
		}
		// host specific upload options
		elseif ($this->get_scope() == 'host') {
			$return['upload_columns'] = array(
			array(
				'name' => 'posixAccount_hostName',
				'description' => _('Host name'),
				'help' => 'uid',
				'example' => _('pc01$'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'posixAccount_uid',
				'description' => _('UID number'),
				'help' => 'uidNumber',
				'example' => '1234'
			),
			array(
				'name' => 'posixAccount_group',
				'description' => _('Primary group'),
				'help' => 'group_upload',
				'example' => _('machines'),
				'required' => true
			),
			);
			if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
				$return['upload_columns'][] = array(
					'name' => 'posixAccount_gecos',
					'description' => _('Gecos'),
					'help' => 'gecos',
					'example' => _('pc01,Room 2.34')
				); 
			}
		}
		// available PDF fields
		if ($this->get_scope() == 'host') {
			$return['PDF_fields'] = array('uid' => _('Host name'));
		}
		else {
			$return['PDF_fields'] = array('uid' => _('User name'));
		}
		$return['PDF_fields'] = array_merge($return['PDF_fields'], array(
			'uidNumber' => _('UID number'),
			'gidNumber' => _('GID number'),
			'primaryGroup' => _('Primary group'),
			'additionalGroups' => _('Additional groups'),
			'homeDirectory' => _('Home directory'),
			'loginShell' => _('Login shell'),
			'cn' => _('Common name'),
			'userPassword' => _('Password')
		));
		if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
			$return['PDF_fields']['gecos'] = _('Gecos');
		}
		if (self::areGroupOfNamesActive()) {
			$return['PDF_fields']['gon'] = _('Groups of names');
		}
		// help Entries
		$return['help'] = array(
			'userNameSuggestion' => array(
				"Headline" => _("User name suggestion"),
				"Text" => _("LAM will suggest a user name based on e.g. first and last name. Here you can specify the suggestion. %sn% will be replaced by the last name. @givenname@ will be replaced by the first character of first name. Only attributes of tab Personal may be used.")
							. '<br>' . _('Common examples are "@givenname@%sn%" or "%givenname%.%sn%".')
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'primaryGroupAsSecondary' => array(
				'Headline' => _('Set primary group as memberUid'),
				'Text' => _('Usually, users are not added to groups as memberUid if they have this group as primary group. If your application ignores primary groups then you can select this option to override this behaviour.')
			),
			'minMaxUser' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for user IDs when creating new user accounts. The range should be different from that of machines. New user accounts will always get the highest number in use plus one.')
			),
			'minMaxHost' => array(
				'Headline' => _('UID number'),
				'Text' => _('These are the minimum and maximum numbers to use for machine IDs when creating new accounts for hosts. The range should be different from that of users. New host accounts will always get the highest number in use plus one.')
			),
			'pwdHash' => array(
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports CRYPT, CRYPT-SHA512, SHA, SSHA, MD5 and SMD5 to generate the hash value of passwords. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.")
			),
			'uidNumber' => array(
				"Headline" => _("UID number"), 'attr' => 'uidNumber',
				"Text" => _("If empty UID number will be generated automaticly.")
			),
			'group_upload' => array(
				"Headline" => _("Primary group"),
				"Text" => _("The primary group for this account. You can insert a GID number or a group name.")
			),
			'addgroup_upload' => array(
				"Headline" => _("Additional groups"),
				"Text" => _("Here you can enter a list of additional group memberships. The group names are separated by commas.")
			),
			'homeDirectory_upload' => array(
				"Headline" => _("Home directory"),
				"Text" => _("Please enter the path to the user's home directory.")
			),
			'deletehomedir' => array(
				"Headline" => _("Home directory"),
				"Text" => _("Activating this checkbox will remove the user's home directory.")
			),
			'createhomedir' => array(
				"Headline" => _("Home directory"),
				"Text" => _("This will create the user's home directory on the specified server.")
			),
			'uidCheckSuffix' => array (
				"Headline" => _("Suffix for UID/user name check"),
				"Text" => _("LAM checks if the entered user name and UID are unique. Here you can enter the LDAP suffix that is used to search for duplicates. By default the account type suffix is used. You only need to change this if you use multiple server profiles with different OUs but need unique user names or UIDs.")
			),
			'loginShells' => array(
				"Headline" => _("Login shells"),
				"Text" => _("This is the list of valid login shells.")
			),
			'uidGenerator' => array (
				"Headline" => _("UID generator"),
				"Text" => _("LAM will automatically suggest UID/GID numbers. You can either use a fixed range of numbers or an LDAP entry with object class \"sambaUnixIdPool\".")
			),
			'sambaIDPoolDN' => array (
				"Headline" => _("Samba ID pool DN"),
				"Text" => _("Please enter the DN of the LDAP entry with object class \"sambaUnixIdPool\".")
			),
			'user' => array(
				'uid' => array(
					"Headline" => _("User name"), 'attr' => 'uid',
					"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, @.-_. If user name is already used user name will be expanded with a number. The next free number will be used.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("User description. If left empty first and last name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"), 'attr' => 'gidNumber',
					"Text" => _("The primary group the user should be member of.")
				),
				'homeDirectory' => array(
					"Headline" => _("Home directory"), 'attr' => 'homeDirectory',
					"Text" => _('$user and $group will be replaced with user name and primary group name.')
				),
				'userPassword' => array(
					"Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account.")
				),
				'userPassword_lock' => array(
					"Headline" => _("Account deactivated"),
					"Text" => _("If checked account will be deactivated by putting a \"!\" before the encrypted password.")
				),
				'loginShell' => array(
					"Headline" => _("Login shell"),
					"Text" => _("To disable login use /bin/false.")
				),
				'addgroup' => array(
					"Headline" => _("Additional groups"),
					"Text" => _("Hold the CTRL-key to (de)select multiple groups."). ' '. _("Can be left empty.")
				),
				'cn' => array (
					"Headline" => _("Common name"), 'attr' => 'cn',
					"Text" => _("This is the natural name of the user. If empty, the first and last name or user name is used.")
				)
			),
			'host' => array(
				'uid' => array(
					"Headline" => _("Host name"), 'attr' => 'uid',
					"Text" => _("Host name of the host which should be created. Valid characters are: a-z,A-Z,0-9, .-_$. Host names are always ending with $. If last character is not $ it will be added. If host name is already used host name will be expanded with a number. The next free number will be used.")
				),
				'gecos' => array(
					"Headline" => _("Gecos"),
					"Text" => _("Host description. If left empty host name will be used.")
				),
				'gidNumber' => array(
					"Headline" => _("Primary group"), 'attr' => 'gidNumber',
					"Text" => _("The primary group the host should be member of.")
				),
				'description' => array (
					"Headline" => _("Description"),
					"Text" => _("Host description. If left empty host name will be used.")
				),
				'cn' => array (
					"Headline" => _("Common name"), 'attr' => 'cn',
					"Text" => _("This is the natural name of the host. If empty, the host name will be used.")
				)
			)
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
		$this->groups = array();
		$this->groups_orig = array();
		$groups = $this->findGroups(); // list of all groupnames
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return;
		}
		$this->gonList = array();
		$this->gonList_orig = array();
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (!isset($this->attributes['uid'][0]) || ($this->attributes['uid'][0] == '')) return false;
		if (!isset($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] == '')) return false;
		if (!isset($this->attributes['gidNumber'][0]) || ($this->attributes['gidNumber'][0] == '')) return false;
		if (!isset($this->attributes['homeDirectory'][0]) || ($this->attributes['homeDirectory'][0] == '')) return false;
		if (!isset($this->attributes['loginShell'][0]) || ($this->attributes['loginShell'][0] == '')) return false;
		return true;
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// get additional group memberships
		if (!isset($attr['uid'][0])) return;
		$groupList = searchLDAPByAttribute('memberUid', $attr['uid'][0], 'posixGroup', array('cn'), array('group'));
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$this->groups[] = $groupList[$i]['cn'][0];
		}
		$this->groups_orig = $this->groups;
		// get additional group of names memberships
		if (self::areGroupOfNamesActive()) {
			$gonList1 = searchLDAPByAttribute('member', $this->getAccountContainer()->dn_orig, 'groupOfNames', array('dn'), array('gon', 'group'));
			$gonList2 = searchLDAPByAttribute('uniqueMember', $this->getAccountContainer()->dn_orig, 'groupOfUniqueNames', array('dn'), array('gon', 'group'));
			$gonList = array_merge($gonList1, $gonList2);
			$this->gonList_orig = array();
			for ($i = 0; $i < sizeof($gonList); $i++) {
				$this->gonList_orig[] = $gonList[$i]['dn'];
			}
			$this->gonList_orig = array_values(array_unique($this->gonList_orig));
			$this->gonList = $this->gonList_orig;
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
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// add information about clear text password and password status change
		$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordClearText'][0] = $this->clearTextPassword;
		if (isset($this->orig['userPassword'][0]) && isset($this->attributes['userPassword'][0])) {
			if ((pwd_is_enabled($this->orig['userPassword'][0]) && pwd_is_enabled($this->attributes['userPassword'][0]))
				|| (!pwd_is_enabled($this->orig['userPassword'][0]) && !pwd_is_enabled($this->attributes['userPassword'][0]))) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unchanged'; 
			}
			elseif (pwd_is_enabled($this->orig['userPassword'][0])) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'locked';
			}
			else {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unlocked';
			}
		}
		// Remove primary group from additional groups
		if (!isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
			|| ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] != 'true')) {
			for ($i=0; $i<count($this->groups); $i++) {
				if ($this->groups[$i] == $this->getGroupName($this->attributes['gidNumber'][0])) {
					unset($this->groups[$i]);
				}
			}
		}
		else {
			// add user as memberuid in primary group
			if (!in_array($this->getGroupName($this->attributes['gidNumber'][0]), $this->groups)) {
				$this->groups[] = $this->getGroupName($this->attributes['gidNumber'][0]);
			}
		}

		// Set additional group memberships
		if (isset($this->orig['uid'][0]) && ($this->orig['uid'][0] != '') && ($this->attributes['uid'][0] != $this->orig['uid'][0])) {
			// find affected groups
			$groupList = searchLDAPByAttribute('memberUid', $this->orig['uid'][0], 'posixGroup', array('dn'), array('group'));
			for ($i = 0; $i < sizeof($groupList); $i++) {
				// replace old user name with new one
				$return[$groupList[$i]['dn']]['remove']['memberUid'][] = $this->orig['uid'][0];
				$return[$groupList[$i]['dn']]['add']['memberUid'][] = $this->attributes['uid'][0];
			}
		}
		else {
			// update groups.
			$add = array_delete($this->groups_orig, $this->groups);
			$remove = array_delete($this->groups, $this->groups_orig);
			$groupList = searchLDAPByAttribute('cn', '*', 'posixGroup', array('cn', 'dn'), array('group'));
			$dn2cn = array();
			for ($i = 0; $i < sizeof($groupList); $i++) {
				$cn2dn[$groupList[$i]['cn'][0]] = $groupList[$i]['dn'];
			}
			for ($i = 0; $i < sizeof($add); $i++) {
				if (isset($cn2dn[$add[$i]])) {
					$return[$cn2dn[$add[$i]]]['add']['memberUid'][] = $this->attributes['uid'][0];
				}
			}
			for ($i = 0; $i < sizeof($remove); $i++) {
				if (isset($cn2dn[$remove[$i]])) {
					$return[$cn2dn[$remove[$i]]]['remove']['memberUid'][] = $this->attributes['uid'][0];
				}
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
		// create home directories if needed
		if (sizeof($this->lamdaemonServers) > 0) {
			$server = null;
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				$server = $temp[0];
				if (isset($temp[1])) {
					if (!in_array($temp[1], $this->lamdaemonServers)) {
						continue;
					}
				}
				elseif (!in_array($temp[0], $this->lamdaemonServers)) {
					continue;
				}
				$result = lamdaemon(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"add",
							$this->attributes['homeDirectory'][0],
							"0".$_SESSION['config']->get_scriptRights(),
							$this->attributes['uidNumber'][0],
							$this->attributes['gidNumber'][0])
						),
					$server);
				// lamdaemon results
				if (is_array($result)) {
					$singleresult = explode(",", $result[0]);
					if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'INFO') || ($singleresult[0] == 'WARN')) {
						$messages[] = $singleresult;
					}
					else {
						$messages[] = array('ERROR', $result[0]);
					}
				}
			}
		}
		// set group of names
		if (self::areGroupOfNamesActive()) {
			$gons = $this->findGroupOfNames();
			$toAdd = array_values(array_diff($this->gonList, $this->gonList_orig));
			$toRem = array_values(array_diff($this->gonList_orig, $this->gonList));
			$toUpdate = array_values(array_intersect($this->gonList, $this->gonList_orig));
			$ldapUser = $_SESSION['ldap']->decrypt_login();
			$ldapUser = $ldapUser[0];
			// update groups if DN changed
			if (isset($this->getAccountContainer()->dn_orig) && ($this->getAccountContainer()->dn_orig != $this->getAccountContainer()->finalDN)) {
				for ($i = 0; $i < sizeof($toUpdate); $i++) {
					if (isset($gons[$toUpdate[$i]])) {
						$attrName = 'member';
						if (in_array('groupOfUniqueNames', $gons[$toUpdate[$i]]['objectclass'])) {
							$attrName = 'uniqueMember';
						}
						$success = @ldap_mod_add($_SESSION['ldap']->server(), $toUpdate[$i], array($attrName => array($this->getAccountContainer()->finalDN)));
						if (!$success) {
							logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add changed user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toUpdate[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
							$messages[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toUpdate[$i]), ldap_error($_SESSION['ldap']->server()));
						}
						else {
							logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added changed user ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toUpdate[$i]);
						}
						$success = @ldap_mod_del($_SESSION['ldap']->server(), $toUpdate[$i], array($attrName => array($this->getAccountContainer()->dn_orig)));
						if (!$success) {
							logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to remove changed user ' . $this->getAccountContainer()->dn_orig . ' from group: ' . $toUpdate[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
							$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toUpdate[$i]), ldap_error($_SESSION['ldap']->server()));
						}
						else {
							logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed changed user ' . $this->getAccountContainer()->dn_orig . ' from group: ' . $toUpdate[$i]);
						}
					}
				}
			}
			// add groups
			for ($i = 0; $i < sizeof($toAdd); $i++) {
				if (isset($gons[$toAdd[$i]])) {
					$attrName = 'member';
					if (in_array('groupOfUniqueNames', $gons[$toAdd[$i]]['objectclass'])) {
						$attrName = 'uniqueMember';
					}
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $toAdd[$i], array($attrName => array($this->getAccountContainer()->finalDN)));
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
				if (isset($gons[$toRem[$i]])) {
					$attrName = 'member';
					if (in_array('groupOfUniqueNames', $gons[$toRem[$i]]['objectclass'])) {
						$attrName = 'uniqueMember';
					}
					$success = @ldap_mod_del($_SESSION['ldap']->server(), $toRem[$i], array($attrName => array($this->getAccountContainer()->dn_orig)));
					if (!$success) {
						logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
						$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), ldap_error($_SESSION['ldap']->server()));
					}
					else {
						logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed user ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i]);
					}
				}
			}
		}
		return $messages;
	}
	
	/**
	* Additional LDAP operations on delete.
	*
	* @return List of LDAP operations, same as for save_attributes()
	*/
	function delete_attributes() {
		$return = array();
		// remove memberUids if set
		$groups = searchLDAPByAttribute('memberUid', $this->attributes['uid'][0], 'posixGroup', array('dn'), array('group'));
		for ($i = 0; $i < sizeof($groups); $i++) {
			$return[$groups[$i]['dn']]['remove']['memberUid'][] = $this->attributes['uid'][0];
		}
		// remove from group of names
		$dn = $this->getAccountContainer()->dn_orig;
		$gonTypes = array('group');
		$types = $_SESSION['config']->get_ActiveTypes();
		if (in_array('gon', $types)) {
			$gonTypes[] = 'gon';
		}
		$gons = searchLDAPByFilter('(|(member=' . $dn . ')(uniqueMember=' . $dn . '))', array('member', 'uniqueMember'), $gonTypes);
		for ($i = 0; $i < sizeof($gons); $i++) {
			if (isset($gons[$i]['member'])) {
				$return[$gons[$i]['dn']]['remove']['member'][] = $dn;
			}
			elseif (isset($gons[$i]['uniquemember'])) {
				$return[$gons[$i]['dn']]['remove']['uniqueMember'][] = $dn;
			}
		}
		return $return;
	}

	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * 
	 * @return array Array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	function preDeleteActions() {
		$return = array();
		if (isset($_POST['deletehomedir'])) {
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				$lamdaemonServers[$i] = $temp[0];
			}
			// try to delete directory on all servers
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$result = lamdaemon(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"rem",
							$this->attributes['homeDirectory'][0],
							$this->attributes['uidNumber'][0]
						)
					),
					$lamdaemonServers[$i]);
				// lamdaemon results
				if (is_array($result)) {
					foreach ($result as $singleresult) {
						$singleresult = explode(",", $singleresult);
						if (is_array($singleresult)) {
							if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
								$return[] = $singleresult;
							}
						}
					}
				}
			}
		}
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
		$groups = $this->findGroups(); // list of all groupnames
		if (count($groups)==0) {
			// abort if no groups were found
			return array();
		}
		if (isset($_POST['loginShell'])) {
			$this->attributes['loginShell'][0] = $_POST['loginShell'];
		}
		if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
			if (isset($_POST['gecos'])) $this->attributes['gecos'][0] = $_POST['gecos'];
		}
		if (isset($this->orig['uid'][0]) && ($this->orig['uid'][0] != '') && ($_POST['uid'] != $this->attributes['uid'][0])) {
			$errors[] = $this->messages['uid'][0];
		}
		if (isset($this->orig['gidNumber'][0]) && ($this->orig['gidNumber'][0] != '') && ($_POST['gidNumber'] != $this->attributes['gidNumber'][0])) {
			$errorMessage = $this->messages['gidNumber'][2];
			$errorMessage[] = array($this->orig['gidNumber'][0], $this->orig['uidNumber'][0], $_POST['gidNumber']);
			$errors[] = $errorMessage;
		}
		if (isset($this->orig['uidNumber'][0]) && $this->orig['uidNumber'][0]!='' && $_POST['uidNumber']!=$this->attributes['uidNumber'][0]) {
			$errorMessage = $this->messages['uidNumber'][5];
			$errorMessage[] = array($this->orig['uidNumber'][0], $_POST['uidNumber']);
			$errors[] = $errorMessage;
		}
		if (isset($_POST['homeDirectory']) && isset($this->orig['homeDirectory'][0]) && ($this->orig['homeDirectory'][0] != '') && ($_POST['homeDirectory'] != $this->attributes['homeDirectory'][0])) {
			$errorMessage = $this->messages['homeDirectory'][3];
			$errorMessage[] = array($this->orig['homeDirectory'][0], $_POST['homeDirectory']);
			$errors[] = $errorMessage;
		}
		// get list of DNS names or IPs
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			$temp = explode(":", $lamdaemonServers[$i]);
			if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
			else $lamdaemonServers[$i] = $temp[0];
		}
		$this->lamdaemonServers = array();
		for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
			if (isset($_POST['createhomedir_' . $h]) && ($_POST['createhomedir_' . $h] = 'on')) {
				$this->lamdaemonServers[] = $lamdaemonServers[$h];
			}
		}
		if (isset($_POST['homeDirectory'])) {
			$this->attributes['homeDirectory'][0] = $_POST['homeDirectory'];
		}
		// Load attributes
		if (isset($_POST['lockPassword'])) {
			$this->lock();
		}
		if (isset($_POST['unlockPassword'])) {
			$this->unlock();
		}
		if (isset($_POST['removePassword'])) {
			unset($this->attributes['userPassword']);
		}
		$this->attributes['cn'][0] = $_POST['cn'];
		if (!get_preg($this->attributes['cn'][0], 'cn')) {
			$errors[] = $this->messages['cn'][0];
		}
		$this->attributes['uidNumber'][0] = $_POST['uidNumber'];
		$this->attributes['gidNumber'][0] = $_POST['gidNumber'];
		if ($this->get_scope()=='user') {
			if (($this->attributes['uid'][0] != $_POST['uid']) && !get_preg($_POST['uid'], '!upper')) {
				$errors[] = $this->messages['uid'][1];
			}
			if ( !get_preg($this->attributes['homeDirectory'][0], 'homeDirectory' )) {
				$errors[] = $this->messages['homeDirectory'][0];
			}
		}
		$this->attributes['uid'][0] = $_POST['uid'];
		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		if ($this->get_scope()=='user') {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		if ($this->get_scope()=='host') {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$uids = $this->getUIDs();
		if ($this->attributes['uidNumber'][0]=='') {
			// No id-number given
			if (!isset($this->orig['uidNumber'][0]) || ($this->orig['uidNumber'][0] == '')) {
				// new account -> we have to find a free id-number
				$newUID = $this->getNextUIDs(1, $errors);
				if (is_array($newUID)) {
					$this->attributes['uidNumber'][0] = $newUID[0];
				}
				else {
					$errors[] = $this->messages['uidNumber'][3];
				}
			}
			else $this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
			// old account -> return id-number which has been used
		}
		else {
			// check manual ID
			if ($this->getAccountContainer()->isNewAccount || !isset($this->orig['uidNumber'][0]) || ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0])) {
				// check range
				if (($this->get_scope() == 'user') && (!isset($this->moduleSettings['posixAccount_uidGeneratorUsers']) || ($this->moduleSettings['posixAccount_uidGeneratorUsers'][0] != 'sambaPool'))) {
					if (!is_numeric($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] < $minID) || ($this->attributes['uidNumber'][0] > $maxID)) {
						$errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
					}
				}
				if (($this->get_scope() == 'host') && (!isset($this->moduleSettings['posixAccount_uidGeneratorHosts']) || ($this->moduleSettings['posixAccount_uidGeneratorHosts'][0] != 'sambaPool'))) {
					if (!is_numeric($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] < $minID) || ($this->attributes['uidNumber'][0] > $maxID)) {
						$errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
					}
				}
				// id-number is in use and account is a new account
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
					$errors[] = $this->messages['uidNumber'][3];
					$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
				}
			}
		}
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ($this->userNameExists($this->attributes['uid'][0]) && isset($this->orig['uid'][0]) && ($this->orig['uid'][0]!='')) {
			$this->attributes['uid'][0] = $this->orig['uid'][0];
		}
		else {
			// Change uid to a new uid until a free uid is found
			while ($this->userNameExists($this->attributes['uid'][0])) {
				$this->attributes['uid'][0] = $this->getNextUserName($this->attributes['uid'][0]);
			}
		}
		// Show warning if LAM has changed username
		if ($this->attributes['uid'][0] != $_POST['uid']) {
			if ($this->get_scope()=='user') $errors[] = $this->messages['uid'][5];
			if ($this->get_scope()=='host') $errors[] = $this->messages['uid'][6];
		}
		if ($this->get_scope()=='user') {
			$this->attributes['homeDirectory'][0] = str_replace('$group', $this->getGroupName($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
			if ($this->attributes['uid'][0] != '') {
				$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
			}
			if ($this->attributes['homeDirectory'][0] != $_POST['homeDirectory']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
			// Check if Username contains only valid characters
			if (!get_preg($this->attributes['uid'][0], 'username'))
			$errors[] = $this->messages['uid'][2];
		}
		if ($this->get_scope()=='host') {
			// Check if Hostname contains only valid characters
			if (!get_preg($this->attributes['uid'][0], 'hostname'))
			$errors[] = $this->messages['uid'][4];
			if (!isset($this->attributes['homeDirectory'][0])) {
				$this->attributes['homeDirectory'][0] = '/dev/null';
			}
			if (!isset($this->attributes['loginShell'][0])) {
				$this->attributes['loginShell'][0] = '/bin/false';
			}
		}
		$attributeList = array('homeDirectory');
		if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
			$attributeList[] = 'gecos';
		}
		for ($i = 0; $i < sizeof($attributeList); $i++) {
			if (isset($this->attributes[$attributeList[$i]][0])) {
				$value = $this->attributes[$attributeList[$i]][0];
				$replacedValue = $this->checkASCII($value);
				if ($value != $replacedValue) {
					$this->attributes[$attributeList[$i]][0] = $replacedValue;
					$errors[] = array('WARN', $attributeList[$i], _('Changed value because only ASCII characters are allowed.'));
				}
			}
		}
		// Return error-messages
		return $errors;
	}

	/**
	* Checks if an attribute contains only ASCII charaters and replaces invalid characters.
	*
	* @param string $attribute attribute value
	* @return string attribute value with replaced non-ASCII characters
	*/
	function checkASCII($attribute) {
		// convert UTF8 to ASCII
		$convert = array ( 'ä' => 'ae', 'Ä' => 'Ae', 'ö' => 'oe', 'Ö' => 'Oe', 'ü' => 'ue', 'Ü' => 'Ue',
			'ß' => 'ss', 'é' => 'e', 'è' => 'e', 'ô' => 'o', 'ç' => 'c'
			);
		// replace special characters
		foreach ($convert as $key => $value) {
			$attribute = str_replace($key, $value, $attribute);
		}
		// remove remaining UTF-8 characters
		for ($c = 0; $c < strlen($attribute); $c++) {
			if (ord($attribute[$c]) > 127) {
				$attribute = str_replace($attribute[$c], "", $attribute);
				$c--;
			}
		}
		return $attribute;
	}

	/**
	* Processes user input of the group selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_group() {
		// Unix groups
		if (!$this->isBooleanConfigOptionSet('posixAccount_hideposixGroups')) {
			if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
				// add new group
				$this->groups = @array_merge($this->groups, $_POST['addgroups']);
			}
			elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
				$this->groups = array_delete($_POST['removegroups'], $this->groups);
			}
		}
		// group of names
		if (self::areGroupOfNamesActive() && !$this->isBooleanConfigOptionSet('posixAccount_hidegon')) {
			if (isset($_POST['addgons']) && isset($_POST['addgons_button'])) { // Add groups to list
				// add new group
				$this->gonList = @array_merge($this->gonList, $_POST['addgons']);
			}
			elseif (isset($_POST['removegons']) && isset($_POST['removegons_button'])) { // remove groups from list
				$this->gonList = array_delete($_POST['removegons'], $this->gonList);
			}
		}
		return array();
	}

	/**
	* Processes user input of the homedir check page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_homedir() {
		$return = array();
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			$temp = explode(":", $lamdaemonServers[$i]);
			$server = $temp[0];
			if (isset($_POST['form_subpage_' . get_class($this) . '_homedir_create_' . $i])) {
				$result = lamdaemon(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"add",
							$this->attributes['homeDirectory'][0],
							"0".$_SESSION['config']->get_scriptRights(),
							$this->attributes['uidNumber'][0],
							$this->attributes['gidNumber'][0])
					),
					$server);
				// lamdaemon results
				if (is_array($result)) {
					foreach ($result as $singleresult) {
						$singleresult = explode(",", $singleresult);
						if (is_array($singleresult)) {
							if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
								$return[] = $singleresult;
							}
						}
					}
				}
			}
			elseif (isset($_POST['form_subpage_' . get_class($this) . '_homedir_delete_' . $i])) {
				$result = lamdaemon(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"rem",
							$this->attributes['homeDirectory'][0],
							$this->attributes['uidNumber'][0]
						)
					),
					$server);
				// lamdaemon results
				if (is_array($result)) {
					foreach ($result as $singleresult) {
						$singleresult = explode(",", $singleresult);
						if (is_array($singleresult)) {
							if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
								$return[] = $singleresult;
							}
						}
					}
				}
			}
		}
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$groupList = $this->findGroups(); // list of all group names
		$groups = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groups[$groupList[$i][1]] = $groupList[$i][0];
		}
		if (count($groups)==0) {
			$return->addElement(new htmlStatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.')));
			return $return;
		}
		$shelllist = $this->getShells(); // list of all valid shells

		// set default values
		if (!isset($this->attributes['uid'][0]) && ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null)) {
			// fill default value for user ID with first/last name
			$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			$this->attributes['uid'][0] = $this->getUserNameSuggestion($attrs);
		}
		if (!isset($this->attributes['cn'][0]) || ($this->attributes['cn'][0] == '')) {
			// set a default value for common name
			if (($this->get_scope() == 'host') && isset($_POST['uid']) && (substr($_POST['uid'], -1, 1) == '$')) {
				$this->attributes['cn'][0] = substr($_POST['uid'], 0, strlen($_POST['uid']) - 1);
			}
			elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
				if ($attrs['givenName'][0]) {
					$this->attributes['cn'][0] = $attrs['givenName'][0] . " " . $attrs['sn'][0];
				}
				elseif ($attrs['sn'][0]) {
					$this->attributes['cn'][0] = $attrs['sn'][0];
				}
				else {
					$this->attributes['cn'][0] = $_POST['uid'];
				}
			}
			elseif (isset($_POST['uid'])) {
				$this->attributes['cn'][0] = $_POST['uid'];
			}
		}
		
		$userName = '';
		if (isset($this->attributes['uid'][0])) $userName = $this->attributes['uid'][0];
		$uidLabel = _("User name");
		if ($this->get_scope() == 'host') {
			$uidLabel = _("Host name");
		}
		$uidInput = new htmlTableExtendedInputField($uidLabel, 'uid', $userName, 'uid');
		$uidInput->setRequired(true);
		$uidInput->setFieldMaxLength(100);
		$return->addElement($uidInput, true);
		$commonName = '';
		if (isset($this->attributes['cn'][0])) $commonName = $this->attributes['cn'][0];
		$return->addElement(new htmlTableExtendedInputField(_("Common name"), 'cn', $commonName, 'cn'), true);
		$uidNumber = '';
		if (isset($this->attributes['uidNumber'][0])) $uidNumber = $this->attributes['uidNumber'][0];
		$uidNumberInput = new htmlTableExtendedInputField(_('UID number'), 'uidNumber', $uidNumber, 'uidNumber');
		$uidNumberInput->setFieldMaxLength(20);
		$uidNumberInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($uidNumberInput, true);
		if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
			$gecos = '';
			if (isset($this->attributes['gecos'][0])) $gecos = $this->attributes['gecos'][0];
			$return->addElement(new htmlTableExtendedInputField(_('Gecos'), 'gecos', $gecos, 'gecos'), true);
		}
		$primaryGroup = array();
		if (isset($this->attributes['gidNumber'][0])) {
			$primaryGroup[] = $this->attributes['gidNumber'][0];
		}
		$gidNumberSelect = new htmlTableExtendedSelect('gidNumber', $groups, $primaryGroup, _('Primary group'), 'gidNumber');
		$gidNumberSelect->setHasDescriptiveElements(true);
		$return->addElement($gidNumberSelect, true);

		if ($this->get_scope()=='user') {
			// additional groups
			if (!$this->isBooleanConfigOptionSet('posixAccount_hidegon') || !$this->isBooleanConfigOptionSet('posixAccount_hideposixGroups')) {
				$return->addElement(new htmlOutputText(_('Additional groups')));
				$return->addElement(new htmlAccountPageButton(get_class($this), 'group', 'open', _('Edit groups')));
				$return->addElement(new htmlHelpLink('addgroup'), true);
			}
			// home directory
			$homedirInput = new htmlTableExtendedInputField(_('Home directory'), 'homeDirectory', $this->attributes['homeDirectory'][0], 'homeDirectory');
			$homedirInput->setRequired(true);
			$return->addElement($homedirInput, true);
			if (($_SESSION['config']->get_scriptPath() != null) && ($_SESSION['config']->get_scriptPath() != '')) {
				if ($this->getAccountContainer()->isNewAccount) {
					// get list of lamdaemon servers
					$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
					for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
						$temp = explode(":", $lamdaemonServers[$i]);
						if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
						else $lamdaemonServers[$i] = $temp[0];
					}
					$homeDirLabel = new htmlOutputText(_('Create home directory'));
					$homeDirLabel->alignment = htmlElement::ALIGN_TOP;
					$return->addElement($homeDirLabel);
					$homeServerContainer = new htmlTable();
					for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
						$homeServerContainer->addElement(new htmlTableExtendedInputCheckbox('createhomedir_' . $h, in_array($lamdaemonServers[$h], $this->lamdaemonServers), $lamdaemonServers[$h], null, false), true);
					}
					$return->addElement($homeServerContainer);
					$homeDirHelp = new htmlHelpLink('createhomedir');
					$homeDirHelp->alignment = htmlElement::ALIGN_TOP;
					$return->addElement($homeDirHelp, true);
				}
				else {
					$return->addElement(new htmlOutputText(''));
					$return->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'open', _('Check home directories')), true);
				}
			}
			$selectedShell = array();
			if (isset($this->attributes['loginShell'][0])) {
				$selectedShell = array($this->attributes['loginShell'][0]);
			}
			$return->addElement(new htmlTableExtendedSelect('loginShell', $shelllist, $selectedShell, _('Login shell'), 'loginShell'), true);
		}
		// password buttons
		if (checkIfWriteAccessIsAllowed() && isset($this->attributes['userPassword'][0])) {
			$return->addElement(new htmlOutputText(_('Password')));
			$pwdContainer = new htmlTable();
			if (pwd_is_enabled($this->attributes['userPassword'][0])) {
				$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
			}
			else {
				$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
			}
			$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
			$pwdContainer->colspan = 2;
			$return->addElement($pwdContainer);
		}
	return $return;
	}

	/**
	* Displays the delete homedir option for the delete page.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_delete() {
		$return = null;
		if ($this->get_scope() == 'user' && ($_SESSION['config']->get_scriptPath() != null)) {
			$return = new htmlTable();
			$return->addElement(new htmlTableExtendedInputCheckbox('deletehomedir', false, _('Delete home directory'), 'deletehomedir'));
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
		if (!$this->isBooleanConfigOptionSet('posixAccount_hideposixGroups')) {
			// load list with all groups
			$groups = $this->findGroups();
			for ($i = 0; $i < sizeof($groups); $i++) {
				$groups[$i] = $groups[$i][1];
			}
			// remove groups the user is member of from grouplist
			$groups = array_delete($this->groups, $groups);
			// Remove primary group from grouplist
			$group = $this->getGroupName($this->attributes['gidNumber'][0]);
			$groups = array_flip($groups);
			unset ($groups[$group]);
			$groups = array_flip($groups);
	
			$unixContainer = new htmlTable();
			$unixContainer->alignment = htmlElement::ALIGN_TOP;
			$unixContainer->addElement(new htmlSubTitle(_("Unix groups")), true);
			$unixContainer->addElement(new htmlOutputText(_("Selected groups")));
			$unixContainer->addElement(new htmlOutputText(''));
			$unixContainer->addElement(new htmlOutputText(_("Available groups")));
			$unixContainer->addNewLine();
			
			$remSelect = new htmlSelect('removegroups', $this->groups, null, 15);
			$remSelect->setMultiSelect(true);
			$remSelect->setTransformSingleSelect(false);
			$unixContainer->addElement($remSelect);
			$buttonContainer = new htmlTable();
			$buttonContainer->addElement(new htmlButton('addgroups_button', 'back.gif', true), true);
			$buttonContainer->addElement(new htmlButton('removegroups_button', 'forward.gif', true), true);
			$buttonContainer->addElement(new htmlHelpLink('addgroup'));
			$unixContainer->addElement($buttonContainer);
			$addSelect = new htmlSelect('addgroups', $groups, null, 15);
			$addSelect->setMultiSelect(true);
			$addSelect->setTransformSingleSelect(false);
			$unixContainer->addElement($addSelect);
			$unixContainer->addNewLine();
		
			$return->addElement($unixContainer);
		}

		if (self::areGroupOfNamesActive() && !$this->isBooleanConfigOptionSet('posixAccount_hidegon')) {
			if (!$this->isBooleanConfigOptionSet('posixAccount_hideposixGroups')) {
				$return->addElement(new htmlSpacer('100px', null));
			}
			
			$gons = $this->findGroupOfNames();
			
			$gonContainer = new htmlTable();
			$gonContainer->alignment = htmlElement::ALIGN_TOP;
			$gonContainer->addElement(new htmlSubTitle(_("Groups of names")), true);
			$gonContainer->addElement(new htmlOutputText(_("Selected groups")));
			$gonContainer->addElement(new htmlOutputText(''));
			$gonContainer->addElement(new htmlOutputText(_("Available groups")));
			$gonContainer->addNewLine();
			
			$selectedGons = array();
			for ($i = 0; $i < sizeof($this->gonList); $i++) {
				if (isset($gons[$this->gonList[$i]])) {
					$selectedGons[$gons[$this->gonList[$i]]['cn'][0]] = $this->gonList[$i];
				}
			}
			$availableGons = array();
			foreach ($gons as $dn => $attr) {
				if (!in_array($dn, $this->gonList)) {
					$availableGons[$attr['cn'][0]] = $dn;
				}
			}
			
			$remGonSelect = new htmlSelect('removegons', $selectedGons, null, 15);
			$remGonSelect->setMultiSelect(true);
			$remGonSelect->setTransformSingleSelect(false);
			$remGonSelect->setHasDescriptiveElements(true);
			$gonContainer->addElement($remGonSelect);
			$buttonGonContainer = new htmlTable();
			$buttonGonContainer->addElement(new htmlButton('addgons_button', 'back.gif', true), true);
			$buttonGonContainer->addElement(new htmlButton('removegons_button', 'forward.gif', true), true);
			$buttonGonContainer->addElement(new htmlHelpLink('addgroup'));
			$gonContainer->addElement($buttonGonContainer);
			$addGonSelect = new htmlSelect('addgons', $availableGons, null, 15);
			$addGonSelect->setMultiSelect(true);
			$addGonSelect->setHasDescriptiveElements(true);
			$addGonSelect->setTransformSingleSelect(false);
			$gonContainer->addElement($addGonSelect);
			$gonContainer->addNewLine();
			$return->addElement($gonContainer);
		}
		
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
	* Displays the delete homedir option for the homedir page.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_homedir() {
		$return = new htmlTable();
		$return->addElement(new htmlOutputText(_('Home directory')));
		$return->addElement(new htmlSpacer('5px', null));
		$return->addElement(new htmlOutputText($this->attributes['homeDirectory'][0]), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$homeServerContainer = new htmlTable();
		$homeServerContainer->colspan = 5;
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			$temp = explode(":", $lamdaemonServers[$i]);
			$server = $temp[0];
			$label = $temp[0];
			if (isset($temp[1])) {
				$label = $temp[1];
			}
			$result = lamdaemon(
				implode(
					self::$SPLIT_DELIMITER,
					array(
						$this->attributes['uid'][0],
						"home",
						"check",
						$this->attributes['homeDirectory'][0])
					),
				$server);
			// lamdaemon results
			if (is_array($result)) {
				$returnValue = trim($result[0]);
				if ($returnValue == 'ok') {
					$homeServerContainer->addElement(new htmlOutputText($label));
					$homeServerContainer->addElement(new htmlSpacer('5px', null));
					$homeServerContainer->addElement(new htmlImage('../../graphics/pass.png', 16, 16));
					$homeServerContainer->addElement(new htmlSpacer('5px', null));
					$homeServerContainer->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'delete_' . $i, _('Delete')));
				}
				elseif ($returnValue == 'missing') {
					$homeServerContainer->addElement(new htmlOutputText($label));
					$homeServerContainer->addElement(new htmlSpacer('5px', null));
					$homeServerContainer->addElement(new htmlImage('../../graphics/fail.png', 16, 16));
					$homeServerContainer->addElement(new htmlSpacer('5px', null));
					$homeServerContainer->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'create_' . $i, _('Create')));
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
					$message->colspan = 5;
					$homeServerContainer->addElement($message, true);
				}
				$homeServerContainer->addNewLine();
			}
		}
		$return->addElement($homeServerContainer, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back')));
		return $return;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = new htmlTable();
		$groupList = $this->findGroups();
		$groups = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groups[] = $groupList[$i][1];
		}
		if ($this->get_scope() == 'user') {
			$shelllist = $this->getShells(); // list of all valid shells
			// primary Unix group
			$return->addElement(new htmlTableExtendedSelect('posixAccount_primaryGroup', $groups, array(), _('Primary group'), 'gidNumber'), true);
			// additional group memberships
			$addGroupSelect = new htmlTableExtendedSelect('posixAccount_additionalGroup', $groups, array(), _('Additional groups'), 'addgroup', 10);
			$addGroupSelect->setMultiSelect(true);
			$addGroupSelect->setTransformSingleSelect(false);
			$return->addElement($addGroupSelect, true);
			// group of names
			if (self::areGroupOfNamesActive()) {
				$gons = $this->findGroupOfNames();
				$gonList = array();
				foreach ($gons as $dn => $attr) {
					$gonList[$attr['cn'][0]] = $dn;
				}
				$gonSelect = new htmlTableExtendedSelect('posixAccount_gon', $gonList, array(), _('Groups of names'), 'addgroup', 10);
				$gonSelect->setHasDescriptiveElements(true);
				$gonSelect->setMultiSelect(true);
				$gonSelect->setTransformSingleSelect(false);
				$return->addElement($gonSelect, true);
			}
			// home directory
			$return->addElement(new htmlTableExtendedInputField(_('Home directory'), 'posixAccount_homeDirectory', '/home/$user', 'homeDirectory'), true);
			// login shell
			$return->addElement(new htmlTableExtendedSelect('posixAccount_loginShell', $shelllist, array("/bin/bash"), _('Login shell'), 'loginShell'), true);
			// lamdaemon settings
			if (($_SESSION['config']->get_scriptPath() != null)) {
				$homeDirLabel = new htmlOutputText(_('Create home directory'));
				$homeDirLabel->alignment = htmlElement::ALIGN_TOP;
				$return->addElement($homeDirLabel);
				$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
				for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
					$temp = explode(":", $lamdaemonServers[$i]);
					if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
					else $lamdaemonServers[$i] = $temp[0];
				}
				$homeServerContainer = new htmlTable();
				for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
					$homeServerContainer->addElement(new htmlTableExtendedInputCheckbox('posixAccount_createHomedir_' . $h, in_array($lamdaemonServers[$h], $this->lamdaemonServers), $lamdaemonServers[$h], null, false), true);
				}
				$return->addElement($homeServerContainer);
				$homeDirHelp = new htmlHelpLink('createhomedir');
				$homeDirHelp->alignment = htmlElement::ALIGN_TOP;
				$return->addElement($homeDirHelp, true);
			}
		}
		elseif ($this->get_scope() == 'host') {
			// primary Unix group
			$return->addElement(new htmlTableExtendedSelect('posixAccount_primaryGroup', $groups, array(), _('Primary group'), 'gidNumber'));
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
		// GID
		if (isset($profile['posixAccount_primaryGroup'][0])) {
			$gid = $this->getGID($profile['posixAccount_primaryGroup'][0]);
			if ($gid != null) {
				$this->attributes['gidNumber'][0] = $gid;
			}
		}
		// other group memberships
		if (isset($profile['posixAccount_additionalGroup'][0])) {
			$this->groups = $profile['posixAccount_additionalGroup'];
		}
		// group of names
		if (isset($profile['posixAccount_gon'][0])) {
			$this->gonList = $profile['posixAccount_gon'];
		}
		// lamdaemon
		if (($this->get_scope() == 'user') && $this->getAccountContainer()->isNewAccount) {
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
				$temp = explode(":", $lamdaemonServers[$i]);
				if (isset($temp[1])) $lamdaemonServers[$i] = $temp[1];
				else $lamdaemonServers[$i] = $temp[0];
			}
			$this->lamdaemonServers = array();
			for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
				if (isset($profile['posixAccount_createHomedir_' . $h][0]) && ($profile['posixAccount_createHomedir_' . $h][0] == 'true')) {
					$this->lamdaemonServers[] = $lamdaemonServers[$h];
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
		$uidLabel = _('User name');
		if ($this->get_scope() == 'host') {
			$uidLabel = _('Host name');
		}
		$return = array(
			'posixAccount_uid' => array('<block><key>' . $uidLabel . '</key><value>' . $this->attributes['uid'][0] . '</value></block>'),
			'posixAccount_cn' => array('<block><key>' . _('Common name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			'posixAccount_uidNumber' => array('<block><key>' . _('UID number') . '</key><value>' . $this->attributes['uidNumber'][0] . '</value></block>'),
			'posixAccount_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
			'posixAccount_primaryGroup' => array('<block><key>' . _('Primary group') . '</key><value>' . $this->getGroupName($this->attributes['gidNumber'][0]) . '</value></block>'),
			'posixAccount_additionalGroups' => array('<block><key>' . _('Additional groups') . '</key><value>' . implode(", ", $this->groups) . '</value></block>'),
			'posixAccount_homeDirectory' => array('<block><key>' . _('Home directory') . '</key><value>' . $this->attributes['homeDirectory'][0] . '</value></block>'),
			'posixAccount_loginShell' => array('<block><key>' . _('Login shell') . '</key><value>' . $this->attributes['loginShell'][0] . '</value></block>'),
		);
		if (isset($this->attributes['gecos'][0])) {
			$return['posixAccount_gecos'] = array('<block><key>' . _('Gecos') . '</key><value>' . $this->attributes['gecos'][0] . '</value></block>');
		}
		if (self::areGroupOfNamesActive()) {
			$allGons = $this->findGroupOfNames();
			$gons = array();
			for ($i = 0; $i < sizeof($this->gonList); $i++) {
				if (isset($allGons[$this->gonList[$i]])) {
					$gons[] = $allGons[$this->gonList[$i]]['cn'][0];
				}
			}
			$return['posixAccount_gon'] = array('<block><key>' . _('Groups of names') . '</key><value>' . implode(", ", $gons) . '</value></block>');
		}
		if (isset($this->clearTextPassword)) {
			$return['posixAccount_userPassword'] = array('<block><key>' . _('Password') . '</key><value>' . $this->clearTextPassword . '</value></block>');
		}
		else if (isset($this->attributes['INFO.userPasswordClearText'])) {
			$return['posixAccount_userPassword'] = array('<block><key>' . _('Password') . '</key><value>' . $this->attributes['INFO.userPasswordClearText'] . '</value></block>');
		}
		return $return;
	}

	/**
	* Checks input values of module settings.
	*
	* @param array $scopes list of account types which are used
	* @param array $options hash array containing the settings (array('option' => array('value')))
	* @return array list of error messages
	*/
	function check_configOptions($scopes, &$options) {
		$return = array();
		// user settings
		if (in_array('user', $scopes)) {
			if ($options['posixAccount_uidGeneratorUsers'][0] == 'range') {
				// min/maxUID are required, check if they are numeric
				if (!isset($options['posixAccount_minUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_minUID'][0])) {
					$return[] = $this->messages['minUID'][0];
				}
				if (!isset($options['posixAccount_maxUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_maxUID'][0])) {
					$return[] = $this->messages['maxUID'][0];
				}
				// minUID < maxUID
				if (isset($options['posixAccount_minUID'][0]) && isset($options['posixAccount_maxUID'][0])) {
					if ($options['posixAccount_minUID'][0] > $options['posixAccount_maxUID'][0]) {
						$return[] = $this->messages['cmp_UID'][0];
					}
				}
			}
			else {
				if (!isset($options['posixAccount_sambaIDPoolDNUsers'][0]) || !get_preg($options['posixAccount_sambaIDPoolDNUsers'][0], 'dn')) {
					$return[] = $this->messages['sambaIDPoolDN'][0];
				}
			}
		}
		// host settings
		if (in_array('host', $scopes)) {
			if ($options['posixAccount_uidGeneratorHosts'][0] == 'range') {
				// min/maxUID are required, check if they are numeric
				if (!isset($options['posixAccount_minMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_minMachine'][0])) {
					$return[] = $this->messages['minMachine'][0];
				}
				if (!isset($options['posixAccount_maxMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_maxMachine'][0])) {
					$return[] = $this->messages['maxMachine'][0];
				}
				// minUID < maxUID
				if (isset($options['posixAccount_minMachine'][0]) && isset($options['posixAccount_maxMachine'][0])) {
					if ($options['posixAccount_minMachine'][0] > $options['posixAccount_maxMachine'][0]) {
						$return[] = $this->messages['cmp_Machine'][0];
					}
				}
			}
			else {
				if (!isset($options['posixAccount_sambaIDPoolDNHosts'][0]) || !get_preg($options['posixAccount_sambaIDPoolDNHosts'][0], 'dn')) {
					$return[] = $this->messages['sambaIDPoolDN'][0];
				}
			}
		}
		// check if user and host ranges overlap
		if (in_array('user', $scopes) && ($options['posixAccount_uidGeneratorUsers'][0] == 'range')
			&& in_array('host', $scopes) && ($options['posixAccount_uidGeneratorHosts'][0] == 'range')) {
			if (isset($options['posixAccount_minUID'][0]) && isset($options['posixAccount_maxUID'][0]) &&
			isset($options['posixAccount_minMachine'][0]) && isset($options['posixAccount_maxMachine'][0])) {
				if (($options['posixAccount_minMachine'][0] > $options['posixAccount_minUID'][0]) &&
				($options['posixAccount_minMachine'][0] < $options['posixAccount_maxUID'][0])) {
					$return[] = $this->messages['cmp_both'][0];
				}
				if (($options['posixAccount_minUID'][0] > $options['posixAccount_minMachine'][0]) &&
				($options['posixAccount_minUID'][0] < $options['posixAccount_maxMachine'][0])) {
					$return[] = $this->messages['cmp_both'][0];
				}
			}
		}
		return $return;
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
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$errors = array();
		$needAutoUID = array();
		// get list of existing users
		$existingUsers = $this->getUserNames();
		// get list of existing groups
		$groupList = $this->findGroups();
		$groupMap = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groupMap[$groupList[$i][1]] = $groupList[$i][0];
		}
		$existingGroups = array_keys($groupMap);
		// get list of existing group of names
		if (self::areGroupOfNamesActive()) {
			$gons = $this->findGroupOfNames();
			$gonList = array();
			foreach ($gons as $dn => $attr) {
				$gonList[] = $attr['cn'][0];
			}
		}
		// check input
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array("posixAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "posixAccount";
			// UID
			if ($rawAccounts[$i][$ids['posixAccount_uid']] == "") {
				// autoUID
				$needAutoUID[] = $i;
			}
			elseif (get_preg($rawAccounts[$i][$ids['posixAccount_uid']], 'digit')) {
				if ($this->get_scope() == 'user') {
					if (($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minUID'][0]) &&
						($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxUID'][0])) {
						$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
					}
					else {
						$errMsg = $this->messages['uidNumber'][4];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				elseif ($this->get_scope() == 'host') {
					if (($rawAccounts[$i][$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_minMachine'][0]) &&
						($rawAccounts[$i][$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_maxMachine'][0])) {
						$partialAccounts[$i]['uidNumber'] = $rawAccounts[$i][$ids['posixAccount_uid']];
					}
					else {
						$errMsg = $this->messages['uidNumber'][4];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
			}
			else {
				$errMsg = $this->messages['uidNumber'][4];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// GID number
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixAccount_group']];
			}
			if (get_preg($rawAccounts[$i][$ids['posixAccount_group']], 'groupname')) {
				$gid = $groupMap[$rawAccounts[$i][$ids['posixAccount_group']]];
				if (is_numeric($gid)) {
					$partialAccounts[$i]['gidNumber'] = $gid;
				}
				else {
					$errMsg = $this->messages['gidNumber'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$errMsg = $this->messages['gidNumber'][1];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			// GECOS
			if (!$this->isBooleanConfigOptionSet('posixAccount_hidegecos')) {
				if ($rawAccounts[$i][$ids['posixAccount_gecos']] != "") {
					if (get_preg($rawAccounts[$i][$ids['posixAccount_gecos']], 'gecos')) {
						$partialAccounts[$i]['gecos'] = $this->checkASCII($rawAccounts[$i][$ids['posixAccount_gecos']]);
					}
					else {
						$errMsg = $this->messages['gecos'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					$gecos = "";
					if (($rawAccounts[$i][$ids['inetOrgPerson_firstName']] != "") && ($rawAccounts[$i][$ids['inetOrgPerson_lastName']] != "")) {
						$gecos = $rawAccounts[$i][$ids['inetOrgPerson_firstName']] . " " . $rawAccounts[$i][$ids['inetOrgPerson_lastName']];
						if ($rawAccounts[$i][$ids['inetOrgPerson_telephone']] != "") {
							$gecos = $gecos . ",," . $rawAccounts[$i][$ids['inetOrgPerson_telephone']];  // double "," because room is unknown
							if ($rawAccounts[$i][$ids['inetOrgPerson_fax']] != "") {
								$gecos = $gecos . "," . $rawAccounts[$i][$ids['inetOrgPerson_fax']];
							}
						}
					}
					if ($gecos != "") {
						$partialAccounts[$i]['gecos'] = $this->checkASCII($gecos);
					}
				}
			}
			// user specific attributes
			if ($this->get_scope() == 'user') {
				// additional groups
				if ($rawAccounts[$i][$ids['posixAccount_additionalGroups']] != "") {
					$groups = explode(",", $rawAccounts[$i][$ids['posixAccount_additionalGroups']]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $existingGroups)) {
							$errors[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						}
					}
				}
				// group of names
				if (self::areGroupOfNamesActive() && ($rawAccounts[$i][$ids['posixAccount_gon']] != "")) {
					$groups = explode(",", $rawAccounts[$i][$ids['posixAccount_gon']]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $gonList)) {
							$errors[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						}
					}
				}
				// user name
				if (in_array($rawAccounts[$i][$ids['posixAccount_userName']], $existingUsers)) {
					$userName = $rawAccounts[$i][$ids['posixAccount_userName']];
					while (in_array($userName, $existingUsers)) {
						$userName = $this->getNextUserName($userName);
					}
					$errMsg = $this->messages['uid'][9];
					array_push($errMsg, array($i, $userName, $rawAccounts[$i][$ids['posixAccount_userName']]));
					$errors[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][7];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// home directory
				if ($rawAccounts[$i][$ids['posixAccount_homedir']] == "") {
					$partialAccounts[$i]['homeDirectory'] = '/home/' . $partialAccounts[$i]['uid'];
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_homedir']], 'homeDirectory')) {
					$partialAccounts[$i]['homeDirectory'] = $rawAccounts[$i][$ids['posixAccount_homedir']];
				}
				else {
					$errMsg = $this->messages['homeDirectory'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// login shell
				if ($rawAccounts[$i][$ids['posixAccount_shell']] == "") {
					$partialAccounts[$i]['loginShell'] = '/bin/bash';
				}
				elseif (in_array($rawAccounts[$i][$ids['posixAccount_shell']], $this->getShells())) {
					$partialAccounts[$i]['loginShell'] = $rawAccounts[$i][$ids['posixAccount_shell']];
				}
				else {
					$errMsg = $this->messages['shell'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				$pwd_enabled = true;
				// password enabled/disabled
				if ($rawAccounts[$i][$ids['posixAccount_passwordDisabled']] == "") {
					$pwd_enabled = true;
				}
				elseif (in_array($rawAccounts[$i][$ids['posixAccount_passwordDisabled']], array('true', 'false'))) {
					if ($rawAccounts[$i][$ids['posixAccount_passwordDisabled']] == 'true') $pwd_enabled = false;
					else $pwd_enabled = true;
				}
				else {
					$errMsg = $this->messages['passwordDisabled'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// password
				if (($rawAccounts[$i][$ids['posixAccount_password']] != "") && (get_preg($rawAccounts[$i][$ids['posixAccount_password']], 'password'))) {
					$partialAccounts[$i]['userPassword'] = pwd_hash($rawAccounts[$i][$ids['posixAccount_password']], $pwd_enabled, $this->moduleSettings['posixAccount_pwdHash'][0]);
					$partialAccounts[$i]['INFO.userPasswordClearText'] = $rawAccounts[$i][$ids['posixAccount_password']]; // for custom scripts etc.
				}
				elseif ($rawAccounts[$i][$ids['posixAccount_password']] != "") {
					$errMsg = $this->messages['userPassword'][4];
					$errMsg[2] = str_replace('%', '%%', $errMsg[2]); // double "%" because of later sprintf
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// cn
				if ($rawAccounts[$i][$ids['posixAccount_cn']] != "") {
					if (get_preg($rawAccounts[$i][$ids['posixAccount_cn']], 'cn')) {
						$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixAccount_cn']];
					}
					else {
						$errMsg = $this->messages['cn'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					if ($partialAccounts[$i]['givenName']) {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['givenName'] . " " . $partialAccounts[$i]['sn'];
					}
					elseif ($partialAccounts[$i]['sn']) {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['sn'];
					}
					else {
						$partialAccounts[$i]['cn'] = $partialAccounts[$i]['uid'];
					}
				}
			}
			// host specific attributes
			elseif ($this->get_scope() == 'host') {
				// host name
				if (in_array($rawAccounts[$i][$ids['posixAccount_hostName']], $existingUsers)) {
					$userName = $rawAccounts[$i][$ids['posixAccount_hostName']];
					while (in_array($userName, $existingUsers)) {
						$userName = $this->getNextUserName($userName);
					}
					$errMsg = $this->messages['uid'][10];
					array_push($errMsg, array($i, $userName, $rawAccounts[$i][$ids['posixAccount_hostName']]));
					$errors[] = $errMsg;
				}
				elseif (get_preg($rawAccounts[$i][$ids['posixAccount_hostName']], 'hostname')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
				}
				else {
					$errMsg = $this->messages['uid'][8];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// description
				if (isset($ids['posixAccount_description']) && isset($rawAccounts[$i][$ids['posixAccount_description']]) && ($rawAccounts[$i][$ids['posixAccount_description']] != '')) {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_description']];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['posixAccount_hostName']];
				}
				$partialAccounts[$i]['homeDirectory'] = '/dev/null';
				$partialAccounts[$i]['loginShell'] = '/bin/false';
			}
		}
		// fill in autoUIDs
		if (sizeof($needAutoUID) > 0) {
			$errorsTemp = array();
			$uids = $this->getNextUIDs(sizeof($needAutoUID), $errorsTemp);
			if (is_array($uids)) {
				for ($i = 0; $i < sizeof($needAutoUID); $i++) {
					$partialAccounts[$i]['uidNumber'] = $uids[$i];
				}
			}
			else {
				$errors[] = $this->messages['uidNumber'][2];
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
			$temp['groups'] = array();
			$temp['dn_gon'] = array();
			$temp['createHomes'] = array();
			$temp['counter'] = 0;
			$col = $ids['posixAccount_additionalGroups'];
			$col_home = $ids['posixAccount_createHomeDir'];
			// get list of existing groups
			$groupList = $this->findGroups();
			$groupMap = array();
			for ($i = 0; $i < sizeof($groupList); $i++) {
				$groupMap[$groupList[$i][0]] = $groupList[$i][1];
			}
			// get list of existing group of names
			if (self::areGroupOfNamesActive()) {
				$gonList = $this->findGroupOfNames();
				$gonMap = array();
				foreach ($gonList as $dn => $attr) {
					$gonMap[$attr['cn'][0]] = $dn;
				}
			}
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				if ($data[$i][$col] != "") {
					$groups = explode(",", $data[$i][$col]);
					if (isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
						&& ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] == 'true')) {
						if (get_preg($data[$i][$ids['posixAccount_group']], 'digit')) {
							if (!in_array($groupMap[$data[$i][$ids['posixAccount_group']]], $groups)) {
								$groups[] = $groupMap[$data[$i][$ids['posixAccount_group']]];
							}
						}
						else {
							if (!in_array($groupMap[$data[$i][$ids['posixAccount_group']]], $groups)) {
								$groups[] = $data[$i][$ids['posixAccount_group']];
							}
						}
					}
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $temp['groups'])) $temp['groups'][] = $groups[$g];
						$temp['members'][$groups[$g]][] = $data[$i][$ids['posixAccount_userName']];
					}
				}
				if (isset($ids['posixAccount_gon']) && ($data[$i][$ids['posixAccount_gon']] != "")) {
					$gons = explode(",", $data[$i][$ids['posixAccount_gon']]);
					$memberAttr = 'member';
					for ($g = 0; $g < sizeof($gons); $g++) {
						if (in_array('groupOfUniqueNames', $gonList[$gonMap[$gons[$g]]]['objectclass'])) {
							$memberAttr = 'uniqueMember';
						}
						$temp['dn_gon'][$gonMap[$gons[$g]]][$memberAttr][] = $accounts[$i]['dn'];
					}
				}
				if ($data[$i][$col_home] != "") {
					$temp['createHomes'][] = $i;
				}
			}
			$temp['dn_gon_keys'] = array_keys($temp['dn_gon']);
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => array()
			);
		}
		// get DNs of groups
		elseif (!isset($temp['dn'])) {
			$temp['dn'] = array();
			$ldapEntries = searchLDAPByAttribute('cn', '*', 'posixGroup', array('dn', 'cn'), array('group'));
			for ($i = 0; $i < sizeof($ldapEntries); $i++) {
				$temp['dn'][$ldapEntries[$i]['cn'][0]] = $ldapEntries[$i]['dn'];
			}
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => array()
			);
		}
		// add users to groups
		elseif ($temp['counter'] < sizeof($temp['groups'])) {
			if (isset($temp['dn'][$temp['groups'][$temp['counter']]])) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $temp['dn'][$temp['groups'][$temp['counter']]], array('memberUID' => $temp['members'][$temp['groups'][$temp['counter']]]));
				$errors = array();
				if (!$success) {
					$errors[] = array(
						"ERROR",
						_("LAM was unable to modify group memberships for group: %s"),
						ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
						array($temp['groups'][$temp['counter']])
					);
				}
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon'])),
					'errors' => $errors
				);
			}
			else {
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups'] + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']))),
					'errors' => array(array('ERROR', _('Unable to find group in LDAP.'), $temp['groups'][$temp['counter']]))
				);
			}
		}
		// create home directories
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']))) {
			$pos = $temp['createHomes'][$temp['counter'] - sizeof($temp['groups'])];
			$result = lamdaemon(
				implode(
					self::$SPLIT_DELIMITER,
					array(
						$data[$pos][$ids['posixAccount_userName']],
						"home",
						"add",
						$accounts[$pos]['homeDirectory'],
						"0".$_SESSION['config']->get_scriptRights(),
						$accounts[$pos]['uidNumber'],
						$accounts[$pos]['gidNumber'],
					)
				),
				$data[$pos][$ids['posixAccount_createHomeDir']]);
			$errors = array();
			if (($result != false) && (sizeof($result) == 1)) {
				$parts = explode(",", $result[0]);
				if (in_array($parts[0], array('ERROR', 'WARN'))) { 
					$errors[] = $parts;
				}
			}
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon'])),
				'errors' => $errors
			);
		}
		// add users to group of names
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']))) {
			$gonDn = $temp['dn_gon_keys'][$temp['counter'] - sizeof($temp['groups']) - sizeof($temp['createHomes'])];
			$gonAttr = $temp['dn_gon'][$gonDn];
			$success = @ldap_mod_add($_SESSION['ldap']->server(), $gonDn, $gonAttr);
			$errors = array();
			if (!$success) {
				$errors[] = array(
					"ERROR",
					_("LAM was unable to modify group memberships for group: %s"),
					ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
					array($temp['groups'][$temp['counter']])
				);
			}
			$temp['counter']++;
			$errors = array();
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon'])),
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
	* Returns one or more free UID numbers.
	*
	* @param integer $count Number of needed free UIDs.
	* @param array $errors list of error messages where errors can be added
	* @return mixed Null if no UIDs are free else an array of free UIDs.
	*/
	function getNextUIDs($count, &$errors) {
		// check if UIDs should be taken from Samba pool entry
		if (($this->get_scope() == 'user') && isset($this->moduleSettings['posixAccount_uidGeneratorUsers']) && ($this->moduleSettings['posixAccount_uidGeneratorUsers'][0] == 'sambaPool')) {
			return $this->getNextSambaPoolUIDs($count, $errors);
		}
		if (($this->get_scope() == 'host') && isset($this->moduleSettings['posixAccount_uidGeneratorHosts']) && ($this->moduleSettings['posixAccount_uidGeneratorHosts'][0] == 'sambaPool')) {
			return $this->getNextSambaPoolUIDs($count, $errors);
		}
		$ret = array();
		if ($this->get_scope() == "user") {
			$minID = intval($this->moduleSettings['posixAccount_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxUID'][0]);
		}
		else {
			$minID = intval($this->moduleSettings['posixAccount_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_maxMachine'][0]);
		}
		$uidList = $this->getUIDs();
		$uids = array();
		for ($i = 0; $i < sizeof($uidList); $i++) {
			if (($uidList[$i] <= $maxID) && ($uidList[$i] >= $minID)) $uids[] = $uidList[$i];  // ignore UIDs > maxID and UIDs < minID
		}
		for ($i = 0; $i < $count; $i++) {
			if (count($uids) != 0) {
				// there already are some uids
				// store highest id-number
				$id = $uids[count($uids)-1];
				// Return minimum allowed id-number if all found id-numbers are too low
				if ($id < $minID) {
					$ret[] = $minID;
					$uids[] = $minID;
				}
				// return highest used id-number + 1 if it's still in valid range
				elseif ($id < $maxID) {
					$ret[] = $id + 1;
					$uids[] = $id + 1;
				}
				// find free numbers between existing ones
				else {
					$k = intval($minID);
					while (in_array($k, $uids)) $k++;
					if ($k > $maxID) return null;
					else {
						$ret[] = $k;
						$uids[] = $k;
						sort ($uids, SORT_NUMERIC);
					}
					// show warning message
					$errors[] = $this->messages['uidNumber'][2];
				}
			}
			else {
				// return minimum allowed id-number if no id-numbers are found
				$ret[] = $minID;
				$uids[] = $minID;
			}
		}
		return $ret;
	}

	/**
	 * Gets the free UID numbers from an Samba pool entry in LDAP.
	 * 
	 * @param integer $count number of needed free UIDs.
	 * @param array $errors list of error messages where errors can be added
	 * @return mixed null if no UIDs are free else an array of free UIDs
	 */
	private function getNextSambaPoolUIDs($count, &$errors) {
		if ($this->get_scope() == 'user') {
			$dn = $this->moduleSettings['posixAccount_sambaIDPoolDNUsers'][0];
		}
		else {
			$dn = $this->moduleSettings['posixAccount_sambaIDPoolDNHosts'][0];
		}
		$attrs = ldapGetDN($dn, array('uidNumber'));
		if (isset($attrs['uidnumber'][0]) && ($attrs['uidnumber'][0] != '')) {
			$newValue = $attrs['uidnumber'][0] + $count;
			$ldapHandle = $_SESSION['ldap']->server();
			ldap_modify($ldapHandle, $dn, array('uidnumber' => array($newValue)));
			logNewMessage(LOG_DEBUG, 'Updated Samba ID pool ' . $dn . ' with UID number ' . $newValue . ' and LDAP code ' . ldap_errno($ldapHandle));
			if (ldap_errno($ldapHandle) != 0) {
				logNewMessage(LOG_NOTICE, 'Updating Samba ID pool ' . $dn . ' with UID number ' . $newValue . ' failed. ' . ldap_error($ldapHandle));
				return null;
			}
			$result = array();
			for ($i = 0; $i < $count; $i++) {
				$result[] = $attrs['uidnumber'][0] + $i;
			}
			return $result;
		}
		return null;
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
	 * @return array list of meta HTML elements (field name => htmlTableRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array();
		if (in_array('password', $fields)) {
			$pwdTable = new htmlTable();
			$pwdTable->colspan = 3;
			$pwd1 = new htmlTableExtendedInputField(_('New password'), 'posixAccount_password');
			$pwd1->setIsPassword(true);
			$pwdTable->addElement($pwd1, true);
			$pwd2 = new htmlTableExtendedInputField(_('Reenter password'), 'posixAccount_password2');
			$pwd2->setIsPassword(true);
			$pwdTable->addElement($pwd2);
			$return['password'] = new htmlTableRow(array(
				$pwdTable
			));
		}
		if ($passwordChangeOnly) {
			return $return; // only password fields as long no LDAP content can be read
		}
		if (in_array('cn', $fields)) {
			$cn = '';
			if (isset($attributes['cn'][0])) $cn = $attributes['cn'][0];
			$cnField = new htmlInputField('posixAccount_cn', $cn);
			if (in_array('cn', $readOnlyFields)) {
				$cnField = new htmlOutputText($cn);
			}
			$return['cn'] = new htmlTableRow(array(
				new htmlOutputText(_('Common name')), $cnField
			));
		}
		if (in_array('loginShell', $fields)) {
			$shelllist = $this->getShells(); // list of all valid shells
			$loginShell = '';
			if (isset($attributes['loginShell'][0])) $loginShell = $attributes['loginShell'][0];
			$loginShellField = new htmlSelect('posixAccount_loginShell', $shelllist, array($loginShell));
			if (in_array('loginShell', $readOnlyFields)) {
				$loginShellField = new htmlOutputText($loginShell);
			}
			$return['loginShell'] = new htmlTableRow(array(
				new htmlOutputText(_('Login shell')), $loginShellField
			));
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
	 * <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
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
		if (in_array('password', $fields)) {
			if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
				if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
					$return['messages'][] = $this->messages['userPassword'][0];
				}
				else {
					if (!get_preg($_POST['posixAccount_password'], 'password')) {
						$return['messages'][] = $this->messages['userPassword'][1];
					}
					else {
						$pwdPolicyResult = checkPasswordStrength($_POST['posixAccount_password']);
						if ($pwdPolicyResult === true) {
							$return['mod']['userPassword'][0] = pwd_hash($_POST['posixAccount_password'], true, $this->selfServiceSettings->moduleSettings['posixAccount_pwdHash'][0]);
							$return['info']['userPasswordClearText'][0] = $_POST['posixAccount_password'];
							if (isset($attributes['shadowLastChange'][0])) {
								$return['mod']['shadowLastChange'][0] = intval(time()/3600/24);
							}
							$_SESSION['selfService_clientPasswordNew'] = $_POST['posixAccount_password'];
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
		if (in_array('cn', $fields) && !in_array('cn', $readOnlyFields)) {
			if (isset($_POST['posixAccount_cn']) && ($_POST['posixAccount_cn'] != '')) {
				if (!get_preg($_POST['posixAccount_cn'], 'cn')) {
					$return['messages'][] = $this->messages['cn'][0];
				}
				else if (!isset($attributes['cn']) || ($attributes['cn'][0] != $_POST['posixAccount_cn'])) {
					$return['mod']['cn'][0] = $_POST['posixAccount_cn'];
				}
			}
			else {
				$return['messages'][] = $this->messages['cn'][0];
			}
		}
		if (in_array('loginShell', $fields) && !in_array('loginShell', $readOnlyFields)) {
			$shelllist = $this->getShells(); // list of all valid shells
			if (in_array($_POST['posixAccount_loginShell'], $shelllist)
					&& (!isset($attributes['loginShell']) || ($attributes['loginShell'][0] != $_POST['posixAccount_loginShell']))) {
				$return['mod']['loginShell'][0] = $_POST['posixAccount_loginShell'];
			}
		}
		return $return;
	}
	
	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
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
		$this->clearTextPassword = $password;
		$this->attributes['userPassword'][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		return array();
	}
	
	/**
	 * Returns the group ID of the given group.
	 *
	 * @param String $groupname group name
	 * @return String GID
	 */
	private function getGID($groupname) {
		$results = searchLDAPByAttribute('cn', $groupname, 'posixGroup', array('gidnumber'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['gidnumber'][0])) {
			return $results[0]['gidnumber'][0];
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
	 * Returns the group DN of the given group.
	 *
	 * @param String $groupname group name
	 * @return String DN
	 */
	private function getGroupDN($groupname) {
		$results = searchLDAPByAttribute('cn', $groupname, 'posixGroup', array('dn'), array('group'));
		if ((sizeof($results) > 0) && isset($results[0]['dn'][0])) {
			return $results[0]['dn'];
		}
		return null;
	}
	
	/**
	 * Finds all existing LDAP groups.
	 *
	 * @return array groups array(array(gidnumber, cn), array(gidnumber, cn), ...)
	 */
	private function findGroups() {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$results = searchLDAPByAttribute(null, null, 'posixGroup', array('cn', 'gidnumber'), array('group'));
		$return = array();
		for ($i = 0; $i < sizeof($results); $i++) {
			if (isset($results[$i]['cn'][0]) && isset($results[$i]['gidnumber'][0])) {
				$return[] = array($results[$i]['gidnumber'][0], $results[$i]['cn'][0]);
			}
		}
		$this->groupCache = $return;
		return $return;
	}
	
	/**
	 * Finds all existing LDAP group of names.
	 *
	 * @return array groups array(dn => array('cn' => array('groupName'), 'objectClass' => array('top', 'groupOfNames')))
	 */
	private function findGroupOfNames() {
		if ($this->gonCache != null) {
			return $this->gonCache;
		}
		$return = array();
		$types = array();
		if (in_array('group', $_SESSION['config']->get_ActiveTypes())) {
			$types[] = 'group';
		}
		if (in_array('gon', $_SESSION['config']->get_ActiveTypes())) {
			$types[] = 'gon';
		}
		if (sizeof($types) > 0) {
			$results = searchLDAPByFilter('(|(objectClass=groupOfNames)(objectClass=groupOfUniqueNames))', array('cn', 'dn', 'objectClass'), $types);
			for ($i = 0; $i < sizeof($results); $i++) {
				if (isset($results[$i]['cn'][0]) && isset($results[$i]['dn'])) {
					$return[$results[$i]['dn']] = $results[$i];
				}
			}
		}
		$this->gonCache = $return;
		return $return;
	}
	
	/**
	 * Returns a list of existing UID numbers.
	 *
	 * @return array list of UID numbers
	 */
	private function getUIDs() {
		if ($this->cachedUIDList != null) {
			return $this->cachedUIDList;
		}
		$this->cachedUIDList = array();
		$attrs = array('uidNumber');
		$filter = '(&(objectClass=posixAccount)(uidNumber=*))';
		$types = $_SESSION['config']->get_ActiveTypes();
		// get user UIDs
		if (in_array('user', $types)) {
			$suffixUsers = $_SESSION['config']->get_Suffix('user');
			if (isset($this->moduleSettings['posixAccount_uidCheckSuffixUser'][0]) && ($this->moduleSettings['posixAccount_uidCheckSuffixUser'][0] != '')) {
				$suffixUsers = $this->moduleSettings['posixAccount_uidCheckSuffixUser'][0];
			}
			$result = searchLDAP($suffixUsers, $filter, $attrs);
			for ($i = 0; $i < sizeof($result); $i++) {
				$this->cachedUIDList[] = $result[$i]['uidnumber'][0];
			}
		}
		// get host UIDs
		if (in_array('host', $types)) {
			$suffixHosts = $_SESSION['config']->get_Suffix('host');
			if (isset($this->moduleSettings['posixAccount_uidCheckSuffixHost'][0]) && ($this->moduleSettings['posixAccount_uidCheckSuffixHost'][0] != '')) {
				$suffixHosts = $this->moduleSettings['posixAccount_uidCheckSuffixHost'][0];
			}
			if ($suffixUsers != $suffixHosts) {
				$result = searchLDAP($suffixHosts, $filter, $attrs);
				for ($i = 0; $i < sizeof($result); $i++) {
					$this->cachedUIDList[] = $result[$i]['uidnumber'][0];
				}
			}
		}
		sort($this->cachedUIDList, SORT_NUMERIC);
		return $this->cachedUIDList;
	}
	
	/**
	 * Checks if the given user name already exists in LDAP.
	 * 
	 * @param String $userName user name
	 * @return boolean true if already exists
	 */
	private function userNameExists($userName) {
		return in_array($userName, $this->getUserNames());
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
		$this->cachedUserNameList = array();
		$attrs = array('uid');
		$filter = '(&(objectClass=posixAccount)(uid=*))';
		$types = $_SESSION['config']->get_ActiveTypes();
		// get user names
		if (in_array('user', $types)) {
			$suffixUsers = $_SESSION['config']->get_Suffix('user');
			if (isset($this->moduleSettings['posixAccount_uidCheckSuffixUser'][0]) && ($this->moduleSettings['posixAccount_uidCheckSuffixUser'][0] != '')) {
				$suffixUsers = $this->moduleSettings['posixAccount_uidCheckSuffixUser'][0];
			}
			$result = searchLDAP($suffixUsers, $filter, $attrs);
			for ($i = 0; $i < sizeof($result); $i++) {
				$this->cachedUserNameList[] = $result[$i]['uid'][0];
			}
		}
		// get host UIDs
		if (in_array('host', $types)) {
			$suffixHosts = $_SESSION['config']->get_Suffix('host');
			if (isset($this->moduleSettings['posixAccount_uidCheckSuffixHost'][0]) && ($this->moduleSettings['posixAccount_uidCheckSuffixHost'][0] != '')) {
				$suffixHosts = $this->moduleSettings['posixAccount_uidCheckSuffixHost'][0];
			}
			if ($suffixUsers != $suffixHosts) {
				$result = searchLDAP($suffixHosts, $filter, $attrs);
				for ($i = 0; $i < sizeof($result); $i++) {
					$this->cachedUserNameList[] = $result[$i]['uid'][0];
				}
			}
		}
		return $this->cachedUserNameList;
	}
	
	/**
	 * Returns if LAM manages group of names entries.
	 * 
	 * @return boolean group of names are active
	 */
	public static function areGroupOfNamesActive() {
		if (!isset($_SESSION['config'])) {
			return false;
		}
		if (in_array('group', $_SESSION['config']->get_ActiveTypes())) {
			$groupModules = $_SESSION['config']->get_AccountModules('group');
			if (in_array('groupOfNames', $groupModules) || in_array('groupOfUniqueNames', $groupModules)) {
				return true;
			}
		}
		if (in_array('gon', $_SESSION['config']->get_ActiveTypes())) {
			$gonModules = $_SESSION['config']->get_AccountModules('gon');
			if (in_array('groupOfNames', $gonModules) || in_array('groupOfUniqueNames', $gonModules)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Returns a suggestion for the user name.
	 * By deafult this wil be the first character of the first name plus the last name.
	 *
	 * @param array $attrs LDAP attributes
	 * @return String user name
	 */
	protected function getUserNameSuggestion($attrs) {
		$attributes = array_change_key_case($attrs, CASE_LOWER);
		$format = '@givenname@%sn%';
		if (isset($this->moduleSettings['posixAccount_userNameSuggestion'][0])) {
			$format = strtolower($this->moduleSettings['posixAccount_userNameSuggestion'][0]);
		}
		// search for @key@ wildcards in format string and replace with first character of attribute
		$wildcards = array();
		if (preg_match_all('/@([^@]|[a-zA-Z_-])+@/', $format, $wildcards) > 0) {
			for ($i = 0; $i < sizeof($wildcards[0]); $i++) {
				$wc = substr($wildcards[0][$i], 1, strlen($wildcards[0][$i]) - 2);
				$value = '';
				if (isset($attributes[$wc][0]) && !empty($attributes[$wc][0])) {
					$value = $attributes[$wc][0][0];
				}
				$format = str_replace('@' . $wc . '@', $value, $format);
			}
		}
		// search for %key% wildcards in format string and replace with attribute
		$wildcards = array();
		if (preg_match_all('/%([^%]|[a-zA-Z_-])+%/', $format, $wildcards) > 0) {
			for ($i = 0; $i < sizeof($wildcards[0]); $i++) {
				$wc = substr($wildcards[0][$i], 1, strlen($wildcards[0][$i]) - 2);
				$value = '';
				if (isset($attributes[$wc][0])) {
					$value = $attributes[$wc][0];
				}
				$format = str_replace('%' . $wc . '%', $value, $format);
			}
		}
		return $format;
	}
	
	/**
	 * Returns if this account can be locked.
	 * This is the case if a hashed password is set ("{" at the beginning).
	 */
	public function isLockable() {
		if (isset($this->attributes['userPassword'][0]) && ((substr($this->attributes['userPassword'][0], 0, 1) == "{") || (substr($this->attributes['userPassword'][0], 1, 1) == "{"))) {
			return true;
		}
		return false;
	}
	
	/**
	 * Returns if the Unix part of the current account is locked.
	 * 
	 * @return boolean password is locked
	 */
	public function isLocked() {
		return isset($this->attributes['userPassword'][0]) && !pwd_is_enabled($this->attributes['userPassword'][0]);
	}
	
	/**
	 * Locks the user password of this account.
	 */
	public function lock() {
		if (isset($this->attributes['userPassword'][0])) {
			$this->attributes['userPassword'][0] = pwd_disable($this->attributes['userPassword'][0]);
		}
	}
	
	/**
	 * Unlocks the user password of this account.
	 */
	public function unlock() {
		if (isset($this->attributes['userPassword'][0])) {
			$this->attributes['userPassword'][0] = pwd_enable($this->attributes['userPassword'][0]);
		}
	}
	
	/**
	 * Removes all Unix group memberships from this user.
	 */
	public function removeFromUnixGroups() {
		$this->groups = array();
	}
	
	/**
	 * Removes all group of names memberships from this user.
	 */
	public function removeFromGONGroups() {
		$this->gonList = array();
	}
	
	/**
	 * Returns the next possible user name based on the given one.
	 * If the user name does not end with a number then a "2" is added.
	 * User names with numbers at the end are simply increased by one.
	 * <br>
	 * <br>Attention: This user name might still be in use. This needs to be checked separately. 
	 * 
	 * @param String $userName user name
	 * @return String new user name
	 */
	protected function getNextUserName($userName) {
		if ($this->get_scope()=='host') {
			$userName = substr($userName, 0, -1);
		}
		// get last character of username
		$lastchar = substr($userName, strlen($userName) - 1, 1);
		// Last character is no number
		if ( !preg_match('/^([0-9])+$/', $lastchar)) {
			// Last character is no number. Therefore we only have to add "2" to it.
			if ($this->get_scope()=='host') {
				$userName = $userName . '2$';
			}
			else {
				$userName = $userName . '2';
			}
		}
		else {
			/* Last character is a number -> we have to increase the number until we've
			* found a groupname with trailing number which is not in use.
			*
			* $i will show us were we have to split groupname so we get a part
			* with the groupname and a part with the trailing number
			*/
			$i = strlen($userName) - 1;
			$mark = false;
			// Set $i to the last character which is a number in $account_new->general_username
			while (!$mark) {
				if (preg_match('/^([0-9])+$/', substr($userName, $i, strlen($userName) - $i))) {
					$i--;
				}
				else {
					$mark=true;
				}
			}
			// increase last number with one
			$firstchars = substr($userName, 0, $i + 1);
			$lastchars = substr($userName, $i + 1, strlen($userName) - $i);
			// Put username together
			if ($this->get_scope()=='host') {
				$userName = $firstchars . (intval($lastchars) + 1) . "$";
			}
			else {
				$userName = $firstchars . (intval($lastchars) + 1);
			}
		}
		return $userName;
	}
	
	/**
	 * Returns the list of possible login shells.
	 * 
	 * @return array login shells
	 */
	private function getShells() {
		// self service
		if (!isset($_SESSION['loggedIn']) && isset($this->selfServiceSettings) && isset($this->selfServiceSettings->moduleSettings['posixAccount_shells'])
			&& (sizeof($this->selfServiceSettings->moduleSettings['posixAccount_shells'])) > 0) {
			return $this->selfServiceSettings->moduleSettings['posixAccount_shells'];
		}
		// server profile
		if (!isset($this->selfServiceSettings) && isset($this->moduleSettings) && isset($this->moduleSettings['posixAccount_shells'])
			&& (sizeof($this->moduleSettings['posixAccount_shells'])) > 0) {
			return $this->moduleSettings['posixAccount_shells'];
		}
		// fall back to default
		return array(
			'/bin/bash',
			'/bin/csh',
			'/bin/dash',
			'/bin/false',
			'/bin/ksh',
			'/bin/sh'
		);
	}
	
}

?>
