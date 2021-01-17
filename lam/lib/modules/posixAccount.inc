<?php
use \LAM\TYPES\TypeManager;
use function LAM\TYPES\getScopeFromTypeId;
use LAM\TYPES\ConfiguredType;
/*

	This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
	Copyright (C) 2003 - 2006  Tilo Lutz
    Copyright (C) 2005 - 2021  Roland Gruber

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

	/** replacements for common umlauts */
	private $umlautReplacements = array(
		'ä' => 'ae', 'Ä' => 'Ae', 'ö' => 'oe', 'Ö' => 'Oe', 'ü' => 'ue', 'Ü' => 'Ue',
		'ß' => 'ss', 'é' => 'e', 'è' => 'e', 'ô' => 'o', 'ç' => 'c'
	);

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
		$this->messages['uid'][5] = array('WARN', _('User name'), _('User name in use (%s). Selected next free user name.'));
		$this->messages['uid'][6] = array('WARN', _('Host name'), _('Host name in use (%s). Selected next free host name.'));
		$this->messages['uid'][7] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][8] = array('ERROR', _('Account %s:') . ' posixAccount_hostName', _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][9] = array('WARN', _('Account %s:') . ' posixAccount_userName', _('User name already exists!') . ' ' . _('You might want to use %s instead of %s.') . ' %s');
		$this->messages['uid'][10] = array('WARN', _('Account %s:') . ' posixAccount_hostName', _('Host name already exists!') . ' ' . _('You might want to use %s instead of %s.') . ' %s');
		$this->messages['gidNumber'][0] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('LAM was unable to find a group with this name!'));
		$this->messages['gidNumber'][1] = array('ERROR', _('Account %s:') . ' posixAccount_group', _('This GID number is invalid! Please provide either a number or a group name.'));
		$this->messages['gidNumber'][2] = array('INFO', _('GID number'), _('GID number has changed. To keep file ownership you have to run the following command as root: \'find / -gid %s -uid %s -exec chgrp %s {} \;\''));
		$this->messages['gecos'][0] = array('ERROR', _('Account %s:') . ' posixAccount_gecos', _('This gecos value is invalid!'));
		$this->messages['shell'][0] = array('ERROR', _('Account %s:') . ' posixAccount_shell', _('This login shell is invalid!'));
		$this->messages['passwordDisabled'][0] = array('ERROR', _('Account %s:') . ' posixAccount_passwordDisabled', _('This value can only be "true" or "false".'));
		$this->messages['cn'][0] = array('ERROR', _('Common name'), _('Please enter a valid common name!'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' posixAccount_cn', _('Please enter a valid common name!'));
		$this->messages['sambaIDPoolDN'][0] = array('ERROR', _('Samba ID pool DN'), _('This is not a valid DN!'));
		$this->messages['windowsIDPoolDN'][0] = array('ERROR', _('Windows domain info DN'), _('This is not a valid DN!'));
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
		$return['icon'] = 'tux.png';
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
		$return['attributes'] = array('uid', 'uidNumber', 'gidNumber',
			'loginShell', 'gecos', 'INFO.userPasswordClearText');
		if ($this->get_scope() == "user") {
			// self service search attributes
			$return['selfServiceSearchAttributes'] = array('uid');
			// self service field settings
			$return['selfServiceFieldSettings'] = array(
					'password' => _('Password'),
					'cn' => _('Common name'),
					'loginShell' => _('Login shell'),
					'syncWindowsPassword' => _('Sync Unix password with Windows password'),
					'unixgroups' => _('Groups (read-only)')
			);
			// possible self service read-only fields
			$return['selfServiceReadOnlyFields'] = array('cn', 'loginShell');
			// self service configuration settings
			$selfServiceContainer = new htmlResponsiveRow();
			$selfServiceContainer->add(new htmlResponsiveSelect('posixAccount_pwdHash', getSupportedHashTypes(),
				array('SSHA'), _("Password hash type"), array('pwdHash', get_class($this))), 12);
			$selfServiceContainer->add(new htmlResponsiveInputTextarea('posixAccount_shells', implode("\r\n", $this->getShells()), 30, 4, _('Login shells'), array('loginShells', get_class($this))), 12);
			$selfServiceContainer->add(new htmlResponsiveInputField(_('Group DN'), 'posixAccount_groupDn', '', array('groupDn', get_class($this))), 12);
			$selfServiceContainer->add(new htmlResponsiveInputCheckbox('posixAccount_useOldPwd', false, _('Password change with old password'), array('useOldPwd', get_class($this))), 12);
			$return['selfServiceSettings'] = $selfServiceContainer;
		}
		// profile checks
		$return['profile_checks']['posixAccount_homeDirectory'] = array('type' => 'ext_preg', 'regex' => 'homeDirectory',
		'error_message' => $this->messages['homeDirectory'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'posixAccount_loginShell' => 'loginShell'
		);
		// upload
		$return['upload_preDepends'] = array('inetOrgPerson');
		// user specific upload options
		if (($this->get_scope() == 'user') && isLoggedIn()) {
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
			$lamdaemonOptions = array();
			foreach ($lamdaemonServers as $lamdaemonServer) {
				$lamdaemonOptions[] = $lamdaemonServer->getServer();
			}
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
				'values' => implode(', ', $lamdaemonOptions)
			),
			array(
				'name' => 'posixAccount_shell',
				'description' => _('Login shell'),
				'help' => 'loginShell',
				'example' => '/bin/bash',
				'values' => implode(", ", $this->getShells()),
				'default' => '/bin/bash'
			),
			);
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
			'userPassword' => _('Password')
		));
		if (self::areGroupOfNamesActive()) {
			$return['PDF_fields']['gon'] = _('Groups of names');
		}
		// help Entries
		$return['help'] = array(
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
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
				"Text" => _("LAM supports a large number of possibilities to generate the hash value of passwords. CRYPT-SHA512 and SSHA are the most common. We do not recommend to use plain text passwords unless passwords are hashed server-side.")
						. ' ' . _('K5KEY is only needed if you use Kerberos with smbk5pwd.')
			),
			'uidNumber' => array(
				"Headline" => _("UID number"), 'attr' => 'uidNumber',
				"Text" => _("If empty UID number will be generated automatically.")
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
			'homeDirectory' => array(
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
			'deleteSudoers' => array(
				"Headline" => _("Delete sudo rights"),
				"Text" => _("Deletes the user from all existing sudo rights.")
			),
			'uidCheckSuffix' => array (
				"Headline" => _("Suffix for UID/user name check"),
				"Text" => _("LAM checks if the entered user name and UID are unique. Here you can enter the LDAP suffix that is used to search for duplicates. By default, the account type suffix is used. You only need to change this if you use multiple server profiles with different OUs but need unique user names or UIDs.")
			),
			'loginShells' => array(
				"Headline" => _("Login shells"),
				"Text" => _("This is the list of valid login shells.")
			),
			'uidGenerator' => array (
				"Headline" => _("UID generator"),
				"Text" => _("LAM will automatically suggest UID/GID numbers. You can either use a fixed range of numbers or an LDAP entry with object class \"sambaUnixIdPool\" or \"msSFU30DomainInfo\".")
					. ' ' . _('Magic number will set a fixed value that must match your server configuration.')
			),
			'sambaIDPoolDN' => array (
				"Headline" => _("Samba ID pool DN"),
				"Text" => _("Please enter the DN of the LDAP entry with object class \"sambaUnixIdPool\".")
			),
			'windowsIDPoolDN' => array (
				"Headline" => _("Windows domain info DN"),
				"Text" => _("Please enter the DN of the LDAP entry with object class \"msSFU30DomainInfo\".")
			),
			'magicNumber' => array(
				"Headline" => _("Magic number"),
				"Text" => _("Please enter the magic number you configured on server side.")
			),
			'noObjectClass' => array(
				"Headline" => _("Do not add object class"),
				"Text" => _("This will not add the posixAccount object class to the account.")
				),
			'excludeFromGroupSync' => array (
				"Headline" => _('Exclude from group sync'),
				"Text" => _('Enter one group per line that should be ignored when syncing groups.')
			),
			'groupDn' => array (
				"Headline" => _('Group DN'),
				"Text" => _('Enter the base DN of your groups here. This is only required if you want to display memberships on the self service page.')
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
				),
				'useOldPwd' => array (
					"Headline" => _('Password change with old password'),
					"Text" => _('Sends the old password together with the new password when the user sets a new password.')
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
		// make optional if needed
		$modules = $_SESSION[$base]->get_type()->getModules();
		$this->autoAddObjectClasses = !$this->isOptional($modules) && !$this->skipObjectClass();
		// call parent init
		parent::init($base);
		$this->groups = array();
		$this->groups_orig = array();
		// list of all group names
		$groups = $this->findGroups($modules);
		if (count($groups)==0) {
			StatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.'), '');
			return;
		}
		$this->gonList = array();
		$this->gonList_orig = array();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	public function getManagedAttributes($typeId) {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		if (!$typeManager->hasConfig()) {
			return $attrs;
		}
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageCn($modules)) {
			$attrs[] = 'cn';
		}
		$attrs[] = $this->getHomedirAttrName($modules);
		$attrs[] = $this->getPasswordAttrName($modules);
		return $attrs;
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (!$this->skipObjectClass() && (!isset($this->attributes['objectClass']) || !in_array('posixAccount', $this->attributes['objectClass']))) {
			// no checks if object class is not set
			return true;
		}
		if (!isset($this->attributes['uid'][0]) || ($this->attributes['uid'][0] == '')) {
			return false;
		}
		if (!isset($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] == '')) {
			return false;
		}
		if (!isset($this->attributes['gidNumber'][0]) || ($this->attributes['gidNumber'][0] == '')) {
			return false;
		}
		if (!isset($this->attributes['loginShell'][0]) || ($this->attributes['loginShell'][0] == '')) {
			return false;
		}
		return true;
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		$typeSettings = $_SESSION['config']->get_typeSettings();
		// get additional group memberships
		if (!isset($attr['uid'][0])) {
			return;
		}
		$groupFilter = '(&(objectClass=posixGroup)(memberUid=' . $attr['uid'][0] . '))';
		if (!empty($typeSettings['filter_group'])) {
			$typeFilter = $typeSettings['filter_group'];
			if (strpos($typeFilter, '(') !== 0) {
				$typeFilter = '(' . $typeFilter . ')';
			}
			$groupFilter = '(&' . $groupFilter . $typeFilter . ')';
		}
		$groupList = searchLDAPByFilter($groupFilter, array('cn'), array('group'));
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$this->groups[] = $groupList[$i]['cn'][0];
		}
		$this->groups_orig = $this->groups;
		// get additional group of names memberships
		if (self::areGroupOfNamesActive()) {
			$types = array('gon', 'group');
			$gonList = array();
			foreach ($types as $type) {
				$gonFilter = '(|(&(objectClass=groupOfNames)(member=' . ldap_escape($this->getAccountContainer()->dn_orig) . '))(&(objectClass=groupOfMembers)(member=' . $this->getAccountContainer()->dn_orig . '))(&(objectClass=groupOfUniqueNames)(uniqueMember=' . $this->getAccountContainer()->dn_orig . ')))';
				if (!empty($typeSettings['filter_' . $type])) {
					$typeFilter = $typeSettings['filter_' . $type];
					if (strpos($typeFilter, '(') !== 0) {
						$typeFilter = '(' . $typeFilter . ')';
					}
					$gonFilter = '(&' . $gonFilter . $typeFilter . ')';
				}
				$gonListPart = searchLDAPByFilter($gonFilter, array('dn'), array($type));
				$gonList = array_merge($gonList, $gonListPart);
			}
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
		if (!$this->skipObjectClass() && (!in_array('posixAccount', $this->attributes['objectClass']) && !in_array('posixAccount', $this->orig['objectClass']))) {
			// skip saving if the extension was not added/modified
			return array();
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		// get default changes
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// add information about clear text password and password status change
		$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordClearText'][0] = $this->clearTextPassword;
		$pwdAttrName = $this->getPasswordAttrName($modules);
		if (isset($this->orig[$pwdAttrName][0]) && isset($this->attributes[$pwdAttrName][0])) {
			if ((pwd_is_enabled($this->orig[$pwdAttrName][0]) && pwd_is_enabled($this->attributes[$pwdAttrName][0]))
				|| (!pwd_is_enabled($this->orig[$pwdAttrName][0]) && !pwd_is_enabled($this->attributes[$pwdAttrName][0]))) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unchanged';
			}
			elseif (pwd_is_enabled($this->orig[$pwdAttrName][0])) {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'locked';
			}
			else {
				$return[$this->getAccountContainer()->dn_orig]['info']['userPasswordStatusChange'][0] = 'unlocked';
			}
		}
		if ($this->skipObjectClass() || in_array('posixAccount', $this->attributes['objectClass'])) {
			// Remove primary group from additional groups
			if (!isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
				|| ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] != 'true')) {
				for ($i = 0; $i < count($this->groups); $i++) {
					if ($this->groups[$i] == $this->getGroupName($this->attributes['gidNumber'][0])) {
						unset($this->groups[$i]);
					}
				}
			}
			else {
				// add user as memberuid in primary group
				$primaryGroupName = $this->getGroupName($this->attributes['gidNumber'][0]);
				if (!in_array($primaryGroupName, $this->groups)) {
					$this->groups[] = $primaryGroupName;
				}
				// add user as member in group of names if auto-sync is activated
				if ($this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
					$allGons = $this->findGroupOfNames();
					foreach ($allGons as $gonDn => $gonData) {
						if (in_array_ignore_case('posixGroup', $gonData['objectclass'])) {
							$gonCn =  $gonData['cn'][0];
							if (($gonCn === $primaryGroupName) && !in_array($gonDn, $this->gonList)) {
								$this->gonList[] = $gonDn;
							}
						}
					}
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
				$cn2dn = array();
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
		}
		elseif (in_array('posixAccount', $this->orig['objectClass']) && !empty($this->orig['uid'][0])) {
			// Unix extension was removed, clean group memberships
			$groupList = searchLDAPByAttribute('memberUid', $this->orig['uid'][0], 'posixGroup', array('dn'), array('group'));
			for ($i = 0; $i < sizeof($groupList); $i++) {
				// remove user name
				$return[$groupList[$i]['dn']]['remove']['memberUid'][] = $this->orig['uid'][0];
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
		$accountContainer = $this->getAccountContainer();
		if ($accountContainer == null) {
			return $messages;
		}
		$modules = $accountContainer->get_type()->getModules();
		// set exop password
		$messages = array_merge($messages, $this->setExopPassword($this->moduleSettings));
		// create home directories if needed
		$homeDirAttr = $this->getHomedirAttrName($modules);
		$lamdaemonServerList = $_SESSION['config']->getConfiguredScriptServers();
		if (sizeof($this->lamdaemonServers) > 0) {
			foreach ($lamdaemonServerList as $lamdaemonServer) {
				if (!in_array($lamdaemonServer->getServer(), $this->lamdaemonServers)) {
					continue;
				}
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServer);
				}
				catch (LAMException $e) {
					$messages[] = array('ERROR', $e->getTitle(), $e->getMessage());
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"add",
							$lamdaemonServer->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0],
							"0".$_SESSION['config']->get_scriptRights(),
							$this->attributes['uidNumber'][0],
							$this->attributes['gidNumber'][0])
						));
				$remote->disconnect();
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'INFO') || ($singleresult[0] == 'WARN')) {
						$messages[] = $singleresult;
					}
					else {
						$messages[] = array('ERROR', $result[0]);
					}
				}
			}
		}
		// move home directory if needed
		if (!empty($this->orig[$homeDirAttr][0]) && !empty($this->attributes[$homeDirAttr][0])
			&& ($this->orig[$homeDirAttr][0] != $this->attributes[$homeDirAttr][0])) {
			foreach ($lamdaemonServerList as $lamdaemonServer) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServer);
				}
				catch (LAMException $e) {
					$messages[] = array('ERROR', $e->getTitle(), $e->getMessage());
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"move",
							$lamdaemonServer->getHomeDirPrefix() . $this->orig[$homeDirAttr][0],
							$this->attributes['uidNumber'][0],
							$lamdaemonServer->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0])
						));
				$remote->disconnect();
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'INFO') || ($singleresult[0] == 'WARN')) {
						$messages[] = $singleresult;
					}
				}
			}
		}
		// set new group on homedirectory
		if (!empty($this->orig[$homeDirAttr][0]) && !empty($this->attributes[$homeDirAttr][0])
			&& ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0])) {
			foreach ($lamdaemonServerList as $lamdaemonServer) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServer);
				}
				catch (LAMException $e) {
					$messages[] = array('ERROR', $e->getTitle(), $e->getMessage());
					continue;
				}
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$this->attributes['uid'][0],
							"home",
							"chgrp",
							$lamdaemonServer->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0],
							$this->attributes['uidNumber'][0],
							$this->attributes['gidNumber'][0])
						));
				$remote->disconnect();
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'INFO') || ($singleresult[0] == 'WARN')) {
						$messages[] = $singleresult;
					}
				}
			}
		}
		// set group of names
		if (self::areGroupOfNamesActive()) {
			$gons = $this->findGroupOfNames();
			$toAdd = array_values(array_diff($this->gonList, $this->gonList_orig));
			$toRem = array_values(array_diff($this->gonList_orig, $this->gonList));
			// update groups if DN changed
			if (isset($accountContainer->dn_orig) && (strtolower($accountContainer->dn_orig) != strtolower($accountContainer->finalDN))) {
				// update owner/member/uniqueMember attributes
				$searchAttrs = array('member', 'uniquemember', 'owner');
				foreach ($searchAttrs as $searchAttr) {
					$ownerGroups = searchLDAPByAttribute($searchAttr, $accountContainer->dn_orig, null, array('dn', $searchAttr), array('gon', 'group'));
					for ($i = 0; $i < sizeof($ownerGroups); $i++) {
						$found = false;
						$newOwners = $ownerGroups[$i][$searchAttr];
						for ($o = 0; $o < sizeof($newOwners); $o++) {
							if ($newOwners[$o] == $accountContainer->dn_orig) {
								$newOwners[$o] = $accountContainer->finalDN;
								$found = true;
								break;
							}
						}
						if ($found) {
							$attributesToModify = array($searchAttr => $newOwners);
							$success = @ldap_mod_replace($_SESSION['ldap']->server(), $ownerGroups[$i]['dn'], $attributesToModify);
							if (!$success) {
								$ldapError = getDefaultLDAPErrorString($_SESSION['ldap']->server());
								logNewMessage(LOG_ERR, 'Unable to modify attributes of DN: ' . $ownerGroups[$i]['dn'] . ' (' . $ldapError . ').');
								logNewMessage(LOG_DEBUG, print_r($attributesToModify, true));
								$messages[] = array('ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $ownerGroups[$i]['dn']), $ldapError);
							}
						}
					}
				}
			}
			// add groups
			for ($i = 0; $i < sizeof($toAdd); $i++) {
				if (isset($gons[$toAdd[$i]])) {
					$attrName = 'member';
					if (in_array_ignore_case('groupOfUniqueNames', $gons[$toAdd[$i]]['objectclass'])) {
						$attrName = 'uniqueMember';
					}
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $toAdd[$i], array($attrName => array($accountContainer->finalDN)));
					if (!$success) {
						logNewMessage(LOG_ERR, 'Unable to add user ' . $accountContainer->finalDN . ' to group: ' . $toAdd[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
						$messages[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toAdd[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					}
					else {
						logNewMessage(LOG_NOTICE, 'Added user ' . $accountContainer->finalDN . ' to group: ' . $toAdd[$i]);
					}
				}
			}
			// remove groups
			for ($i = 0; $i < sizeof($toRem); $i++) {
				if (isset($gons[$toRem[$i]])) {
					$attrName = 'member';
					if (in_array_ignore_case('groupOfUniqueNames', $gons[$toRem[$i]]['objectclass'])) {
						$attrName = 'uniqueMember';
					}
					$success = @ldap_mod_del($_SESSION['ldap']->server(), $toRem[$i], array($attrName => array($accountContainer->dn_orig)));
					if (!$success) {
						logNewMessage(LOG_ERR, 'Unable to delete user ' . $accountContainer->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
						$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					}
					else {
						logNewMessage(LOG_NOTICE, 'Removed user ' . $accountContainer->finalDN . ' from group: ' . $toRem[$i]);
					}
				}
			}
		}
		return $messages;
	}

	/**
	 * Sets the password via ldap_exop if configured.
	 *
	 * @param array $settings settings
	 * @return array error message parameters if any
	 */
	private function setExopPassword($settings) {
		if (!empty($this->clearTextPassword) && !empty($settings['posixAccount_pwdHash'][0])
				&& ($settings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
			$success = ldap_exop_passwd($_SESSION['ldap']->server(), $this->getAccountContainer()->finalDN, null, $this->clearTextPassword);
			if (!$success) {
				return array(array('ERROR', _('Unable to set password'), getExtendedLDAPErrorMessage($_SESSION['ldap']->server())));
			}
		}
		return array();
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
		// stop here if referential integrity overlay is active
		$config = $this->getAccountContainer()->get_type()->getTypeManager()->getConfig();
		if ($config->isReferentialIntegrityOverlayActive()) {
			return $return;
		}
		// remove from group of names
		$dn = $this->getAccountContainer()->dn_orig;
		$gons = searchLDAPByFilter('(|(member=' . $dn . ')(uniqueMember=' . $dn . '))', array('member', 'uniqueMember'), array('group', 'gon'));
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
		// delete home directory
		if (isset($_POST['deletehomedir']) && ($_POST['deletehomedir'] == 'on')) {
			$modules = $this->getAccountContainer()->get_type()->getModules();
			$homeDirAttr = $this->getHomedirAttrName($modules);
			// get list of lamdaemon servers
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
			// try to delete directory on all servers
			foreach ($lamdaemonServers as $lamdaemonServer) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServer);
					$result = $remote->execute(
						implode(
							self::$SPLIT_DELIMITER,
							array(
								$this->attributes['uid'][0],
								"home",
								"rem",
								$lamdaemonServer->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0],
								$this->attributes['uidNumber'][0]
							)
						));
					$remote->disconnect();
				}
				catch (LAMException $e) {
					$return[] = array('ERROR', $e->getTitle(), $e->getMessage());
				}
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (is_array($singleresult)) {
						if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
							$return[] = $singleresult;
						}
					}
				}
			}
		}
		// delete sudo rights
		if (isset($_POST['deleteSudoers']) && ($_POST['deleteSudoers'] == 'on')) {
			$result = searchLDAPByAttribute('sudoUser', $this->attributes['uid'][0], 'sudoRole', array('dn'), array('sudo'));
			foreach ($result as $attrs) {
				$dn = $attrs['dn'];
				$success = @ldap_mod_del($_SESSION['ldap']->server(), $dn, array('sudoUser' => array($this->attributes['uid'][0])));
				if (!$success) {
					$return[] = array('ERROR', getDefaultLDAPErrorString($_SESSION['ldap']->server()));
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
		$keysToReplace = array('cn', 'gecos', 'homeDirectory');
		$this->getAccountContainer()->replaceWildcardsInPOST($keysToReplace);
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		$errors = array();
		if (isset($_POST['addObjectClass'])) {
			if (!isset($this->attributes['objectClass'])) {
				$this->attributes['objectClass'] = array();
			}
			if (!in_array('posixAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'posixAccount';
			}
			return $errors;
		}
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('posixAccount'), $this->attributes['objectClass']);
			$attrs = $this->getManagedAttributes($this->getAccountContainer()->get_type()->getId());
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if ($this->isOptional($modules) && !$this->skipObjectClass() && (!isset($this->attributes['objectClass']) || !in_array('posixAccount', $this->attributes['objectClass']))) {
			return $errors;
		}
		$groups = $this->findGroups($modules); // list of all group names
		if (count($groups)==0) {
			// abort if no groups were found
			return array();
		}
		if (isset($_POST['loginShell'])) {
			$this->attributes['loginShell'][0] = $_POST['loginShell'];
		}
		if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
			if (isset($_POST['gecos'])) {
				$this->attributes['gecos'][0] = $_POST['gecos'];
			}
		}
		if (isset($this->orig['uid'][0]) && ($this->orig['uid'][0] != '') && (trim($_POST['uid']) != $this->attributes['uid'][0])) {
			$errors[] = $this->messages['uid'][0];
		}
		if (isset($this->orig['gidNumber'][0]) && ($this->orig['gidNumber'][0] != '') && ($_POST['gidNumber'] != $this->attributes['gidNumber'][0])) {
			$errorMessage = $this->messages['gidNumber'][2];
			$errorMessage[] = array($this->orig['gidNumber'][0], $this->orig['uidNumber'][0], $_POST['gidNumber']);
			$errors[] = $errorMessage;
			if ($this->isBooleanConfigOptionSet('posixAccount_primaryGroupAsSecondary') && !empty($this->attributes['gidNumber'][0])) {
				// change primary group in $this->groups
				$oldGroupName = $this->getGroupName($this->attributes['gidNumber'][0]);
				$newGroupName = $this->getGroupName($_POST['gidNumber']);
				if (!empty($oldGroupName) && !empty($newGroupName)) {
					$this->groups = array_delete(array($oldGroupName), $this->groups);
					$this->groups[] = $newGroupName;
					// sync group of names if needed
					if ($this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
						$allGons = $this->findGroupOfNames();
						foreach ($allGons as $gonDn => $gonData) {
							if (in_array_ignore_case('posixGroup', $gonData['objectclass'])) {
								$gonCn =  $gonData['cn'][0];
								if (($gonCn === $newGroupName) && !in_array($gonDn, $this->gonList)) {
									$this->gonList[] = $gonDn;
								}
								if (($gonCn === $oldGroupName) && in_array($gonDn, $this->gonList)) {
									$this->gonList = array_delete(array($gonDn), $this->gonList);
								}
							}
						}
					}
				}
			}
		}
		if (isset($this->orig['uidNumber'][0]) && $this->orig['uidNumber'][0]!='' && trim($_POST['uidNumber'])!=$this->attributes['uidNumber'][0]) {
			$errorMessage = $this->messages['uidNumber'][5];
			$errorMessage[] = array($this->orig['uidNumber'][0], $_POST['uidNumber']);
			$errors[] = $errorMessage;
		}
		$homedirAttrName = $this->getHomedirAttrName($modules);
		if (isset($_POST['homeDirectory']) && isset($this->orig[$homedirAttrName][0]) && ($this->orig[$homedirAttrName][0] != '') && ($_POST['homeDirectory'] != $this->attributes[$homedirAttrName][0])) {
			$errorMessage = $this->messages['homeDirectory'][3];
			$errorMessage[] = array($this->orig[$homedirAttrName][0], $_POST['homeDirectory']);
			$errors[] = $errorMessage;
		}
		// get list of DNS names or IPs
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		$this->lamdaemonServers = array();
		for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
			if (isset($_POST['createhomedir_' . $h]) && ($_POST['createhomedir_' . $h] = 'on')) {
				$this->lamdaemonServers[] = $lamdaemonServers[$h]->getServer();
			}
		}
		if (isset($_POST['homeDirectory'])) {
			$this->attributes[$homedirAttrName][0] = $_POST['homeDirectory'];
		}
		// Load attributes
		if ($this->isPasswordManaged()) {
			if (isset($_POST['lockPassword'])) {
				$this->lock($modules);
			}
			if (isset($_POST['unlockPassword'])) {
				$this->unlock($modules);
			}
			if (isset($_POST['removePassword'])) {
				unset($this->attributes[$this->getPasswordAttrName($modules)]);
			}
		}
		if ($this->manageCn($modules)) {
			$this->processMultiValueInputTextField('cn', $errors, 'cn');
		}
		$this->attributes['uidNumber'][0] = trim($_POST['uidNumber']);
		$this->attributes['gidNumber'][0] = $_POST['gidNumber'];
		if ($this->get_scope() == 'user') {
			if (($this->attributes['uid'][0] != $_POST['uid']) && !get_preg($_POST['uid'], '!upper')) {
				$errors[] = $this->messages['uid'][1];
			}
			if ( !get_preg($this->attributes[$homedirAttrName][0], 'homeDirectory' )) {
				$errors[] = $this->messages['homeDirectory'][0];
			}
		}
		$this->attributes['uid'][0] = trim($_POST['uid']);
		// Check if UID is valid. If none value was entered, the next usable value will be inserted
		// load min and may uidNumber
		if ($this->get_scope()=='user') {
			$minID = intval($this->moduleSettings['posixAccount_' . $typeId . '_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_' . $typeId . '_maxUID'][0]);
		}
		if ($this->get_scope()=='host') {
			$minID = intval($this->moduleSettings['posixAccount_' . $typeId . '_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_' . $typeId . '_maxMachine'][0]);
		}
		$uids = $this->getUIDs($typeId);
		if ($this->attributes['uidNumber'][0]=='') {
			// No id-number given
			if (!isset($this->orig['uidNumber'][0]) || ($this->orig['uidNumber'][0] == '')) {
				// new account -> we have to find a free id-number
				$newUID = $this->getNextUIDs(1, $errors, $typeId);
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
				if (($this->get_scope() == 'user') && (!isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers']) || ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'range'))) {
					if (!is_numeric($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] < $minID) || ($this->attributes['uidNumber'][0] > $maxID)) {
						$errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
					}
				}
				if (($this->get_scope() == 'host') && (!isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts']) || ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'range'))) {
					if (!is_numeric($this->attributes['uidNumber'][0]) || ($this->attributes['uidNumber'][0] < $minID) || ($this->attributes['uidNumber'][0] > $maxID)) {
						$errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
					}
				}
				// id-number is in use and account is a new account
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && !isset($this->orig['uidNumber'][0])) $errors[] = $this->messages['uidNumber'][3];
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && isset($this->orig['uidNumber'][0]) && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
					$errors[] = $this->messages['uidNumber'][3];
					$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
				}
			}
		}
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ($this->userNameExists($this->attributes['uid'][0], $typeId) && isset($this->orig['uid'][0]) && ($this->orig['uid'][0]!='')) {
			$this->attributes['uid'][0] = $this->orig['uid'][0];
		}
		else {
			// Change uid to a new uid until a free uid is found
			while ($this->userNameExists($this->attributes['uid'][0], $typeId)) {
				$this->attributes['uid'][0] = $this->getNextUserName($this->attributes['uid'][0], array_keys($this->getAccountContainer()->getAccountModules()));
			}
		}
		// Show warning if LAM has changed username
		if ($this->attributes['uid'][0] != trim($_POST['uid'])) {
			$userNames = $this->getUserNames($typeId);
			if ($this->get_scope() == 'user') {
				$error = $this->messages['uid'][5];
				$error[] = array(htmlspecialchars($userNames[trim($_POST['uid'])]));
				$errors[] = $error;
			}
			if ($this->get_scope() == 'host') {
				$error = $this->messages['uid'][6];
				$error[] = array(htmlspecialchars($userNames[trim($_POST['uid'])]));
				$errors[] = $error;
			}
		}
		if (($this->get_scope() == 'user') && !get_preg($this->attributes['uid'][0], 'username')) {
			// Check if Username contains only valid characters
			$errors[] = $this->messages['uid'][2];
		}
		if ($this->get_scope() == 'host') {
			// Check if Hostname contains only valid characters
			if (!get_preg($this->attributes['uid'][0], 'hostname')) {
				$errors[] = $this->messages['uid'][4];
			}
			if (!isset($this->attributes[$homedirAttrName][0])) {
				$this->attributes[$homedirAttrName][0] = '/dev/null';
			}
			if (!isset($this->attributes['loginShell'][0])) {
				$this->attributes['loginShell'][0] = '/bin/false';
			}
		}
		$attributeList = array($homedirAttrName);
		if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
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
		if ($this->get_scope() == 'user') {
			// set SASL password for new and renamed users
			if (!empty($this->attributes['uid'][0]) && !empty($this->moduleSettings['posixAccount_pwdHash'][0])
					&& ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')
					&& ($this->getAccountContainer()->isNewAccount || ($this->attributes['uid'][0] != $this->orig['uid'][0]))) {
				$this->attributes[$this->getPasswordAttrName($modules)][0] = '{SASL}' . $this->attributes['uid'][0];
			}
			// set K5KEY password for new users
			if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
				$this->attributes[$this->getPasswordAttrName($modules)][0] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
			}
		}
		if (isset($_POST['posixAccount_createGroup'])
			&& !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hideCreateGroup')
			&& ($this->get_scope() == 'user')
			&& $this->getAccountContainer()->isNewAccount && get_preg($this->attributes['uid'][0], 'username')) {
			$groupType = $this->getPosixGroupType();
			$sessionKey = 'TMP' . getRandomNumber();
			$accountContainerTmp = new accountContainer($groupType, $sessionKey);
			$_SESSION[$sessionKey] = &$accountContainerTmp;
			$accountContainerTmp->new_account();
			$posixGroupModule = $accountContainerTmp->getAccountModule('posixGroup');
			$nextGid = $posixGroupModule->getNextGIDs(1, $errors, $groupType);
			if ($nextGid !== null) {
				$newGroupName = $this->attributes['uid'][0];
				$dnNewGroup = 'cn=' . $newGroupName . ',' . $groupType->getSuffix();
				$attributesNewGroup = array(
					'cn' => array($newGroupName),
					'gidNumber' => $nextGid[0],
					'objectClass' => array('posixGroup'),
				);
				$newGroupSuccess = @ldap_add(getLDAPServerHandle(), $dnNewGroup, $attributesNewGroup);
				if ($newGroupSuccess) {
					$errors[] = array('INFO', _('Created new group.'), htmlspecialchars($newGroupName));
					$this->attributes['gidNumber'][0] = $nextGid[0];
					$this->groupCache = null;
				}
				else {
					$errors[] = array('ERROR', _('Unable to create new group.'), getDefaultLDAPErrorString(getLDAPServerHandle()));
				}
			}
		}
		// Return error-messages
		return $errors;
	}

	/**
	 * Returns the first found group type that contains posixGroup.
	 *
	 * @return ConfiguredType|null type
	 */
	private function getPosixGroupType() {
		$typeManager = new TypeManager();
		$groupTypes = $typeManager->getConfiguredTypesForScope('group');
		foreach ($groupTypes as $groupType) {
			$modules = $groupType->getModules();
			if (in_array('posixGroup', $modules)) {
				return $groupType;
			}
		}
		return null;
	}

	/**
	* Checks if an attribute contains only ASCII characters and replaces invalid characters.
	*
	* @param string $attribute attribute value
	* @return string attribute value with replaced non-ASCII characters
	*/
	function checkASCII($attribute) {
		if ($attribute == null) {
			return '';
		}
		// replace special characters
		$attribute = str_replace(array_keys($this->umlautReplacements), array_values($this->umlautReplacements), $attribute);
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
		$typeId = $this->getAccountContainer()->get_type()->getId();
		// Unix groups
		if ($this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
			$this->syncGonToGroups();
		}
		elseif (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hideposixGroups')) {
			if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
				// add new group
				$this->groups = @array_merge($this->groups, $_POST['addgroups']);
			}
			elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
				$this->groups = array_delete($_POST['removegroups'], $this->groups);
			}
		}
		// group of names
		if (self::areGroupOfNamesActive() && !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegon')) {
			if (isset($_POST['addgons']) && isset($_POST['addgons_button'])) { // Add groups to list
				// add new group
				$this->gonList = @array_merge($this->gonList, $_POST['addgons']);
			}
			elseif (isset($_POST['removegons']) && isset($_POST['removegons_button'])) { // remove groups from list
				$this->gonList = array_delete($_POST['removegons'], $this->gonList);
			}
		}
		// sync Unix to GoN
		if (isset($_POST['form_subpage_posixAccount_group_syncU2GON'])) {
			$this->manualSyncUnixToGon($typeId);
		}
		// sync GoN to Unix
		if (isset($_POST['form_subpage_posixAccount_group_syncGON2U'])) {
			$this->manualSyncGonToUnix($this->getAccountContainer()->get_type());
		}
		// sync Windows to Unix
		if (isset($_POST['form_subpage_posixAccount_group_syncWin2U'])) {
			$this->manualSyncWindowsToUnix($this->getAccountContainer()->get_type());
		}
		return array();
	}

	/**
	 * Syncs the Unix groups to group of names.
	 *
	 * @param string $typeId type ID
	 */
	private function manualSyncUnixToGon($typeId) {
		$allGons = $this->findGroupOfNames();
		$namesToIgnore = array();
		if (!empty($this->moduleSettings['posixAccount_' . $typeId . '_syncGroupsExclusions'])) {
			$namesToIgnore = $this->moduleSettings['posixAccount_' . $typeId . '_syncGroupsExclusions'];
			array_map('trim', $namesToIgnore);
		}
		// remove all groups that are not in Unix
		if (isset($_POST['syncDeleteGroups']) && ($_POST['syncDeleteGroups'] == 'on')) {
			$toDelete = array();
			foreach ($this->gonList as $currentGon) {
				$gonName = $this->getGonName($currentGon, $allGons);
				if (in_array($gonName, $namesToIgnore) || in_array($gonName, $this->groups)) {
					continue;
				}
				$toDelete[] = $currentGon;
			}
			$this->gonList = array_delete($toDelete, $this->gonList);
		}
		// add groups that are not yet in groups of names
		foreach ($this->groups as $currentName) {
			if (in_array($currentName, $namesToIgnore)) {
				continue;
			}
			$found = false;
			foreach ($this->gonList as $currentGon) {
				if ($currentName == $this->getGonName($currentGon, $allGons)) {
					$found = true;
					break;
				}
			}
			if ($found) {
				continue;
			}
			foreach ($allGons as $currentGon) {
				if ($currentName == $this->getGonName($currentGon['dn'], $allGons)) {
					$this->gonList[] = $currentGon['dn'];
					break;
				}
			}
		}
	}

	/**
	 * Syncs the group of names to Unix groups.
	 *
	 * @param ConfiguredType $type type
	 */
	private function manualSyncGonToUnix($type) {
		$allGons = $this->findGroupOfNames();
		$modules = $type->getModules();
		$allGroups = $this->findGroups($modules);
		foreach ($allGroups as $index => $groupData) {
			$allGroups[$index] = $groupData[1];
		}
		$namesToIgnore = array();
		if (!empty($this->moduleSettings['posixAccount_' . $type->getId() . '_syncGroupsExclusions'])) {
			$namesToIgnore = $this->moduleSettings['posixAccount_' . $type->getId() . '_syncGroupsExclusions'];
			array_map('trim', $namesToIgnore);
		}
		// remove all groups that are not in group of names
		if (isset($_POST['syncDeleteGroups']) && ($_POST['syncDeleteGroups'] == 'on')) {
			$toDelete = array();
			foreach ($this->groups as $currentName) {
				if (in_array($currentName, $namesToIgnore)) {
					continue;
				}
				$found = false;
				foreach ($this->gonList as $currentGon) {
					$gonName = $this->getGonName($currentGon, $allGons);
					if ($gonName == $currentName) {
						$found = true;
						break;
					}
				}
				if (!$found) {
					$toDelete[] = $currentName;
				}
			}
			$this->groups = array_delete($toDelete, $this->groups);
		}
		// add groups that are not yet in Unix groups
		foreach ($this->gonList as $currentGon) {
			$gonName = $this->getGonName($currentGon, $allGons);
			if (in_array($gonName, $namesToIgnore)) {
				continue;
			}
			if (!in_array($gonName, $this->groups) && in_array($gonName, $allGroups)) {
				$this->groups[] = $gonName;
			}
		}
	}

	/**
	 * Syncs the Windows to Unix groups.
	 *
	 * @param ConfiguredType $type type
	 */
	private function manualSyncWindowsToUnix($type) {
		$windowsGroups = $this->getAccountContainer()->getAccountModule('windowsUser')->getGroupList();
		$allWindowsGroups = searchLDAPByAttribute('gidNumber', '*', null, array('cn'), array('group'));
		$allGroups = $this->findGroups($modules);
		foreach ($allGroups as $index => $groupData) {
			$allGroups[$index] = $groupData[1];
		}
		$namesToIgnore = array();
		if (!empty($this->moduleSettings['posixAccount_' . $type->getId() . '_syncGroupsExclusions'])) {
			$namesToIgnore = $this->moduleSettings['posixAccount_' . $type->getId() . '_syncGroupsExclusions'];
			array_map('trim', $namesToIgnore);
		}
		// remove all groups that are not in Windows groups
		if (isset($_POST['syncDeleteGroups']) && ($_POST['syncDeleteGroups'] == 'on')) {
			$toDelete = array();
			foreach ($this->groups as $currentName) {
				if (in_array($currentName, $namesToIgnore)) {
					continue;
				}
				$found = false;
				foreach ($windowsGroups as $currentWindowsGroup) {
					$windowsGroupName = $this->getWindowsGroupName($allWindowsGroups, $currentWindowsGroup);
					if ($windowsGroupName == $currentName) {
						$found = true;
						break;
					}
				}
				if (!$found) {
					$toDelete[] = $currentName;
				}
			}
			$this->groups = array_delete($toDelete, $this->groups);
		}
		// add groups that are not yet in Unix groups
		foreach ($windowsGroups as $currentWindowsGroup) {
			$windowsGroupName = $this->getWindowsGroupName($allWindowsGroups, $currentWindowsGroup);
			if (in_array($windowsGroupName, $namesToIgnore)) {
				continue;
			}
			if (!in_array($windowsGroupName, $this->groups) && in_array($windowsGroupName, $allGroups)) {
				$this->groups[] = $windowsGroupName;
			}
		}
	}

	/**
	 * Returns the cn of the given group of names.
	 *
	 * @param string $dn DN of group of names
	 * @param $allGons list of all group of names
	 * @return string cn value
	 */
	public function getGonName($dn, &$allGons) {
		if (!empty($allGons[$dn]['cn'][0])) {
			return $allGons[$dn]['cn'][0];
		}
		return extractRDNValue($dn);
	}

	/**
	 * Returns the Windows group name.
	 *
	 * @param array $allWindowsGroups LDAP data of all Windows groups
	 * @param string $dn DN
	 */
	private function getWindowsGroupName(&$allWindowsGroups, $dn) {
		foreach ($allWindowsGroups as $data) {
			if ($data['dn'] == $dn) {
				return $data['cn'][0];
			}
		}
		return null;
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
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$homeDirAttr = $this->getHomedirAttrName($modules);
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			if (isset($_POST['form_subpage_' . get_class($this) . '_homedir_create_' . $i])) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServers[$i]);
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
							"add",
							$lamdaemonServers[$i]->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0],
							"0".$_SESSION['config']->get_scriptRights(),
							$this->attributes['uidNumber'][0],
							$this->attributes['gidNumber'][0])
					));
				$remote->disconnect();
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (is_array($singleresult)) {
						if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
							$return[] = $singleresult;
						}
					}
				}
			}
			elseif (isset($_POST['form_subpage_' . get_class($this) . '_homedir_delete_' . $i])) {
				$remote = new \LAM\REMOTE\Remote();
				try {
					$remote->connect($lamdaemonServers[$i]);
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
							$lamdaemonServers[$i]->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0],
							$this->attributes['uidNumber'][0]
						)
					));
				$remote->disconnect();
				// lamdaemon results
				if (!empty($result)) {
					$singleresult = explode(",", $result);
					if (is_array($singleresult)) {
						if (($singleresult[0] == 'ERROR') || ($singleresult[0] == 'WARN') || ($singleresult[0] == 'INFO')) {
							$return[] = $singleresult;
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
		$return = new htmlResponsiveRow();
		$this->checkForInvalidConfiguration($return);
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		if (!$this->isOptional($modules) || $this->skipObjectClass() || (isset($this->attributes['objectClass']) && in_array('posixAccount', $this->attributes['objectClass']))) {
			$homeDirAttr = $this->getHomedirAttrName($modules);
			$groupList = $this->findGroups($modules); // list of all group names
			$groups = array();
			for ($i = 0; $i < sizeof($groupList); $i++) {
				$groups[$groupList[$i][1]] = $groupList[$i][0];
			}
			if (count($groups)==0) {
				$return->add(new htmlStatusMessage("ERROR", _('No Unix groups found in LDAP! Please create one first.')), 12);
				return $return;
			}
			$shelllist = $this->getShells(); // list of all valid shells

			// set default values
			if (empty($this->attributes['uid'][0])) {
				if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
					// fill default value for user ID with first/last name
					$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
					$this->attributes['uid'][0] = $this->getUserNameSuggestion($attrs, $typeId);
					$firstSuggestion = $this->attributes['uid'][0];
					if (!empty($this->attributes['uid'][0]) && $this->userNameExists($this->attributes['uid'][0], $typeId)) {
						while ($this->userNameExists($this->attributes['uid'][0], $typeId)) {
							$this->attributes['uid'][0] = $this->getNextUserName($this->attributes['uid'][0], array_keys($this->getAccountContainer()->getAccountModules()));
						}
						$users = $this->getUserNames($typeId);
						$msg = new htmlStatusMessage($this->messages['uid'][5][0],
								$this->messages['uid'][5][1], $this->messages['uid'][5][2],
								array(htmlspecialchars($users[$firstSuggestion])));
						$return->add($msg, 12);
					}
				}
				elseif ($this->getAccountContainer()->getAccountModule('windowsUser') != null) {
					// fill default value for user ID with AD user name
					$attrs = $this->getAccountContainer()->getAccountModule('windowsUser')->getAttributes();
					if (!empty($attrs['userPrincipalName'][0])) {
						$parts = explode('@', $attrs['userPrincipalName'][0]);
						$this->attributes['uid'][0] = $parts[0];
					}
				}
			}
			if ($this->manageCn($modules) && (!isset($this->attributes['cn'][0]) || ($this->attributes['cn'][0] == ''))) {
				// set a default value for common name
				if (($this->get_scope() == 'host') && isset($_POST['uid'])) {
					if (substr($_POST['uid'], -1, 1) == '$') {
						$this->attributes['cn'][0] = substr($_POST['uid'], 0, strlen($_POST['uid']) - 1);
					}
					else {
						$this->attributes['cn'][0] = $_POST['uid'];
					}
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
			if (isset($this->attributes['uid'][0])) {
				$userName = $this->attributes['uid'][0];
			}
			$uidLabel = _("User name");
			if ($this->get_scope() == 'host') {
				$uidLabel = _("Host name");
			}
			$uidInput = new htmlResponsiveInputField($uidLabel, 'uid', $userName, 'uid');
			$uidInput->setRequired(true);
			$uidInput->setFieldMaxLength(100);
			$return->add($uidInput, 12);
			if ($this->manageCn($modules)) {
				$this->addMultiValueInputTextField($return, 'cn', _("Common name"));
			}
			$uidNumber = '';
			if (isset($this->attributes['uidNumber'][0])) {
				$uidNumber = $this->attributes['uidNumber'][0];
			}
			$uidNumberInput = new htmlResponsiveInputField(_('UID number'), 'uidNumber', $uidNumber, 'uidNumber');
			$uidNumberInput->setFieldMaxLength(20);
			$uidNumberInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->add($uidNumberInput, 12);
			if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
				$gecos = '';
				if (isset($this->attributes['gecos'][0])) {
					$gecos = $this->attributes['gecos'][0];
				}
				$return->add(new htmlResponsiveInputField(_('Gecos'), 'gecos', $gecos, 'gecos'), 12);
			}
			$primaryGroup = array();
			if (isset($this->attributes['gidNumber'][0])) {
				$primaryGroup[] = $this->attributes['gidNumber'][0];
			}
			$gidNumberSelect = new htmlResponsiveSelect('gidNumber', $groups, $primaryGroup, _('Primary group'), 'gidNumber');
			$gidNumberSelect->setHasDescriptiveElements(true);
			$return->add($gidNumberSelect, 12);

			if ($this->get_scope() == 'user') {
				// new Unix group with same name
				$posixGroupType = $this->getPosixGroupType();
				if ($this->getAccountContainer()->isNewAccount
					&& !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hideCreateGroup')
					&& ($posixGroupType !== null)
					&& (!isset($this->attributes['uid'][0]) || !isset($groups[$this->attributes['uid'][0]]))) {
					$return->addLabel(new htmlOutputText('&nbsp;', false));
					$return->addField(new htmlButton('posixAccount_createGroup', _('Create group with same name')));
				}
				// additional groups
				if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegon') || !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hideposixGroups')) {
					$return->addLabel(new htmlOutputText(_('Additional groups')));
					$additionalGroups = new htmlGroup();
					$additionalGroups->addElement(new htmlAccountPageButton(get_class($this), 'group', 'open', _('Edit groups')));
					$additionalGroups->addElement(new htmlHelpLink('addgroup'));
					$return->addField($additionalGroups);
				}
				// home directory
				$homeDir = isset($this->attributes[$homeDirAttr][0]) ? $this->attributes[$homeDirAttr][0] : '';
				$homedirInput = new htmlResponsiveInputField(_('Home directory'), 'homeDirectory', $homeDir, 'homeDirectory');
				$homedirInput->setRequired(true);
				$return->add($homedirInput, 12);
				if (($_SESSION['config']->get_scriptPath() != null) && ($_SESSION['config']->get_scriptPath() != '')) {
					if ($this->getAccountContainer()->isNewAccount) {
						// get list of lamdaemon servers
						$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
						$homeDirLabel = new htmlOutputText(_('Create home directory'));
						$return->addLabel($homeDirLabel);
						$homeServerContainer = new htmlTable();
						for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
							$homeServerContainer->addElement(new htmlTableExtendedInputCheckbox('createhomedir_' . $h, in_array($lamdaemonServers[$h]->getServer(), $this->lamdaemonServers), $lamdaemonServers[$h]->getLabel(), null, false));
							if ($h === 0) {
								$homeDirHelp = new htmlHelpLink('createhomedir');
								$homeServerContainer->addElement($homeDirHelp);
							}
							$homeServerContainer->addNewLine();
						}
						$return->addField($homeServerContainer);
					}
					else {
						$return->addLabel(new htmlOutputText('&nbsp;', false));
						$return->addField(new htmlAccountPageButton(get_class($this), 'homedir', 'open', _('Check home directories')));
					}
				}
				$selectedShell = array();
				if (isset($this->attributes['loginShell'][0])) {
					$selectedShell = array($this->attributes['loginShell'][0]);
				}
				$return->add(new htmlResponsiveSelect('loginShell', $shelllist, $selectedShell, _('Login shell'), 'loginShell'), 12);
			}
			// password buttons
			if (checkIfWriteAccessIsAllowed($this->get_scope())
					&& isset($this->attributes[$this->getPasswordAttrName($modules)][0])
					&& $this->isPasswordManaged()) {
				$return->addLabel(new htmlOutputText(_('Password')));
				$pwdContainer = new htmlGroup();
				if (pwd_is_enabled($this->attributes[$this->getPasswordAttrName($modules)][0])) {
					$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
				}
				else {
					$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
				}
				$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
				$return->addField($pwdContainer);
			}
			// remove button
			if ($this->isOptional($modules) && !$this->skipObjectClass()) {
				$return->addVerticalSpacer('2rem');
				$remButton = new htmlButton('remObjectClass', _('Remove Unix extension'));
				$return->add($remButton, 12, 12, 12, 'text-center');
			}
		}
		else {
			// add button
			$return->add(new htmlButton('addObjectClass', _('Add Unix extension')), 12);
		}
		return $return;
	}

	/**
	 * Checks if the configuration is valid and prints an error if not.
	 *
	 * @param htmlResponsiveRow $content content area
	 */
	private function checkForInvalidConfiguration(htmlResponsiveRow $content) {
		$typeId = $this->getAccountContainer()->get_type()->getId();
		if ($this->get_scope() == 'user') {
			$generatorOption = 'posixAccount_' . $typeId . '_uidGeneratorUsers';
		}
		else {
			$generatorOption = 'posixAccount_' . $typeId . '_uidGeneratorHosts';
		}
		if (empty($this->moduleSettings[$generatorOption])) {
			$message = new htmlStatusMessage('ERROR', _('Invalid configuration detected. Please edit your server profile (module settings) and fill all required fields.'));
			$content->add($message, 12);
		}
	}

	/**
	* Displays the delete homedir option for the delete page.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_delete() {
		$return = new htmlResponsiveRow();
		if ($this->get_scope() == 'user' && ($_SESSION['config']->get_scriptPath() != null)) {
			$return->add(new htmlResponsiveInputCheckbox('deletehomedir', true, _('Delete home directory'), 'deletehomedir'), 12);
		}
		$typeManager = new TypeManager();
		$sudoTypes = $typeManager->getConfiguredTypesForScope('sudo');
		if (($this->get_scope() == 'user') && !empty($sudoTypes)) {
			$return->add(new htmlResponsiveInputCheckbox('deleteSudoers', true, _('Delete sudo rights'), 'deleteSudoers'), 12);
		}
		return $return;
	}

	/**
	* Displays the group selection.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_group() {
		$return = new htmlResponsiveRow();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		$showUnix = !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hideposixGroups');
		$autoSyncGon = $this->isBooleanConfigOptionSet('posixGroup_autoSyncGon');
		if ($showUnix) {
			// load list with all groups
			$groups = $this->findGroups($modules);
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
			if ($autoSyncGon) {
				$this->syncGonToGroups();
				foreach ($this->groups as $group) {
					$unixContainer->addElement(new htmlOutputText($group), true);
				}
			}
			else {
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
				$unixContainer->addElement($addSelect, true);
			}

			$return->add($unixContainer, 12);
			$return->addVerticalSpacer('3rem');
		}

		$showGon = self::areGroupOfNamesActive() && !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegon');
		if ($showGon) {
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
					$selectedGons[getAbstractDN($this->gonList[$i])] = $this->gonList[$i];
				}
			}
			uksort($selectedGons, 'compareDN');
			$availableGons = array();
			foreach ($gons as $dn => $attr) {
				if (!in_array($dn, $this->gonList)) {
					$availableGons[getAbstractDN($dn)] = $dn;
				}
			}
			uksort($availableGons, 'compareDN');

			$remGonSelect = new htmlSelect('removegons', $selectedGons, null, 15);
			$remGonSelect->setMultiSelect(true);
			$remGonSelect->setTransformSingleSelect(false);
			$remGonSelect->setHasDescriptiveElements(true);
			$remGonSelect->setSortElements(false);
			$remGonSelect->setRightToLeftTextDirection(true);
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
			$addGonSelect->setSortElements(false);
			$addGonSelect->setRightToLeftTextDirection(true);
			$gonContainer->addElement($addGonSelect);
			$gonContainer->addNewLine();
			$return->add($gonContainer, 12);
			$return->addVerticalSpacer('3rem');
		}

		$showGonSync = $showGon && !$autoSyncGon;
		$showUnixSync = $showUnix && !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_syncGroups');
		$moduleList = $this->getAccountContainer()->get_type()->getModules();
		$showWindowsSync = $this->isWindows($moduleList);
		if ($showUnixSync && ($showGonSync || $showWindowsSync)) {
			$return->add(new htmlSubTitle(_('Sync groups')), 12);
			$return->add(new htmlResponsiveInputCheckbox('syncDeleteGroups', true, _('Delete non-matching entries')), 12);
			$return->addVerticalSpacer('1rem');
			if ($showGonSync) {
				$syncButtons = new htmlGroup();
				$u2gonButton = new htmlAccountPageButton(get_class($this), 'group', 'syncU2GON', _('Sync Unix to group of names'));
				$u2gonButton->setIconClass('unixButton');
				$syncButtons->addElement($u2gonButton);
				$syncButtons->addElement(new htmlSpacer('2rem', null));
				$gon2uButton = new htmlAccountPageButton(get_class($this), 'group', 'syncGON2U', _('Sync group of names to Unix'));
				$gon2uButton->setIconClass('groupButton');
				$syncButtons->addElement($gon2uButton);
				$return->add($syncButtons, 12, 12, 12, 'text-center');
				if ($showWindowsSync) {
					$syncButtons->addElement(new htmlSpacer('2rem', null));
				}
			}
			if ($showWindowsSync) {
				$syncButtons = new htmlGroup();
				$gon2uButton = new htmlAccountPageButton(get_class($this), 'group', 'syncWin2U', _('Sync Windows to Unix'));
				$gon2uButton->setIconClass('sambaButton');
				$syncButtons->addElement($gon2uButton);
				$return->add($syncButtons, 12, 12, 12, 'text-center');
			}
		}

		$return->addVerticalSpacer('2rem');
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$return->add($backButton, 12);
		return $return;
	}

	/**
	* Displays the delete homedir option for the homedir page.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_homedir() {
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$homeDirAttr = $this->getHomedirAttrName($modules);
		$return = new htmlResponsiveRow();
		$return->addLabel(new htmlOutputText(_('Home directory')));
		$return->addField(new htmlOutputText($this->attributes[$homeDirAttr][0]));
		$return->addVerticalSpacer('2rem');
		// get list of lamdaemon servers
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($i = 0; $i < sizeof($lamdaemonServers); $i++) {
			$label = $lamdaemonServers[$i]->getLabel();
			$remote = new \LAM\REMOTE\Remote();
			try {
				$remote->connect($lamdaemonServers[$i]);
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
						$lamdaemonServers[$i]->getHomeDirPrefix() . $this->attributes[$homeDirAttr][0])
					));
			$remote->disconnect();
			// lamdaemon results
			if (!empty($result)) {
				$returnValue = trim($result);
				if ($returnValue == 'ok') {
					$return->addLabel(new htmlOutputText($label));
					$okGroup = new htmlGroup();
					$okGroup->addElement(new htmlImage('../../graphics/pass.png', 16, 16));
					$okGroup->addElement(new htmlSpacer('5px', null));
					$okGroup->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'delete_' . $i, _('Delete')));
					$return->addField($okGroup);
				}
				elseif ($returnValue == 'missing') {
					$return->addLabel(new htmlOutputText($label));
					$failGroup = new htmlGroup();
					$failGroup->addElement(new htmlImage('../../graphics/fail.png', 16, 16));
					$failGroup->addElement(new htmlSpacer('5px', null));
					$failGroup->addElement(new htmlAccountPageButton(get_class($this), 'homedir', 'create_' . $i, _('Create')));
					$return->addField($failGroup);
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
	* {@inheritDoc}
	*/
	function get_profileOptions($typeId) {
		$return = new htmlResponsiveRow();
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		$groupList = $this->findGroups($modules);
		$groups = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groups[] = $groupList[$i][1];
		}
		if ($this->get_scope() == 'user') {
			$shelllist = $this->getShells(); // list of all valid shells
			// primary Unix group
			$return->add(new htmlResponsiveSelect('posixAccount_primaryGroup', $groups, array(), _('Primary group'), 'gidNumber'), 12);
			// additional group memberships
			$addGroupSelect = new htmlResponsiveSelect('posixAccount_additionalGroup', $groups, array(), _('Additional groups'), 'addgroup', 10);
			$addGroupSelect->setMultiSelect(true);
			$addGroupSelect->setTransformSingleSelect(false);
			$return->add($addGroupSelect, 12);
			// group of names
			if (self::areGroupOfNamesActive()) {
				$gons = $this->findGroupOfNames();
				$gonList = array();
				foreach ($gons as $dn => $attr) {
					$gonList[$attr['cn'][0]] = $dn;
				}
				$gonSelect = new htmlResponsiveSelect('posixAccount_gon', $gonList, array(), _('Groups of names'), 'addgroup', 10);
				$gonSelect->setHasDescriptiveElements(true);
				$gonSelect->setMultiSelect(true);
				$gonSelect->setTransformSingleSelect(false);
				$return->add($gonSelect, 12);
			}
			// common name
			if ($this->manageCn($modules)) {
				$return->add(new htmlResponsiveInputField(_('Common name'), 'posixAccount_cn', '', 'cn'), 12);
			}
			// home directory
			$return->add(new htmlResponsiveInputField(_('Home directory'), 'posixAccount_homeDirectory', '/home/$user', 'homeDirectory'), 12);
			// login shell
			$return->add(new htmlResponsiveSelect('posixAccount_loginShell', $shelllist, array("/bin/bash"), _('Login shell'), 'loginShell'), 12);
			// lamdaemon settings
			if ($_SESSION['config']->get_scriptPath() != null) {
				$return->add(new htmlSubTitle(_('Create home directory')), 12);
				$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
				for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
					$server = $lamdaemonServers[$h]->getServer();
					$label = $lamdaemonServers[$h]->getLabel();
					$return->add(new htmlResponsiveInputCheckbox('posixAccount_createHomedir_' . $h, in_array($server, $this->lamdaemonServers), $label, 'createhomedir', false), 12);
				}
			}
		}
		elseif ($this->get_scope() == 'host') {
			// primary Unix group
			$return->add(new htmlResponsiveSelect('posixAccount_primaryGroup', $groups, array(), _('Primary group'), 'gidNumber'), 12);
		}
		if ($this->isOptional($modules)) {
			$return->add(new htmlResponsiveInputCheckbox('posixAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'), 12);
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
		$modules = $this->getAccountContainer()->get_type()->getModules();
		// cn
		if ($this->manageCn($modules) && !empty($profile['posixAccount_cn'][0])) {
			$this->attributes['cn'][0] = $profile['posixAccount_cn'][0];
		}
		// home directory
		$homeDirAttr = $this->getHomedirAttrName($modules);
		if (!empty($profile['posixAccount_homeDirectory'][0])) {
			$this->attributes[$homeDirAttr][0] = $profile['posixAccount_homeDirectory'][0];
		}
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
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
			$this->lamdaemonServers = array();
			for ($h = 0; $h < sizeof($lamdaemonServers); $h++) {
				if (isset($profile['posixAccount_createHomedir_' . $h][0]) && ($profile['posixAccount_createHomedir_' . $h][0] == 'true')) {
					$this->lamdaemonServers[] = $lamdaemonServers[$h]->getServer();
				}
			}
		}
		// add extension
		if (isset($profile['posixAccount_addExt'][0]) && ($profile['posixAccount_addExt'][0] == "true")) {
			if (!$this->skipObjectClass() && !in_array('posixAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'posixAccount';
			}
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfFields()
	 */
	public function get_pdfFields($typeId) {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageCn($modules)) {
			$fields['cn'] = _('Common name');
		}
		if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
			$fields['gecos'] = _('Gecos');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$uidLabel = _('User name');
		if ($this->get_scope() == 'host') {
			$uidLabel = _('Host name');
		}
		$additionalGroups = array();
		if (!empty($this->groups)) {
			$additionalGroups = $this->groups;
			natcasesort($additionalGroups);
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$homeDirAttr = $this->getHomedirAttrName($modules);
		$return = array();
		if (isset($this->attributes['gidNumber'][0])) {
			$this->addPDFKeyValue($return, 'primaryGroup', _('Primary group'), $this->getGroupName($this->attributes['gidNumber'][0]));
		}
		$this->addPDFKeyValue($return, 'additionalGroups', _('Additional groups'), implode(", ", $additionalGroups));
		$this->addSimplePDFField($return, 'uid', $uidLabel);
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'uidNumber', _('UID number'));
		$this->addSimplePDFField($return, 'gidNumber', _('GID number'));
		$this->addSimplePDFField($return, 'homeDirectory', _('Home directory'), $homeDirAttr);
		$this->addSimplePDFField($return, 'loginShell', _('Login shell'));
		$this->addSimplePDFField($return, 'gecos', _('Gecos'));
		if (self::areGroupOfNamesActive()) {
			$gons = array();
			for ($i = 0; $i < sizeof($this->gonList); $i++) {
				$gons[] = $this->gonList[$i];
			}
			usort($gons, 'compareDN');
			$gonCount = sizeof($gons);
			for ($i = 0; $i < $gonCount; $i++) {
				$gons[$i] = getAbstractDN($gons[$i]);
			}
			$this->addPDFKeyValue($return, 'gon', _('Groups of names'), $gons, "\n");
		}
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
	 * @see baseModule::get_configOptions()
	 */
	public function get_configOptions($scopes, $allScopes) {
		$typeManager = new TypeManager($_SESSION['conf_config']);
		$isWindows = array_key_exists('windowsUser', $allScopes);
		$return = array();
		$generatorOptions = array(
				_('Fixed range') => 'range',
				_('Samba ID pool') => 'sambaPool',
				_('Windows domain info') => 'windowsDomain',
				_('Magic number') => 'magicNumber'
		);
		$hasUserConfig = false;
		$hasHostConfig = false;
		foreach ($scopes as $typeId) {
			if (getScopeFromTypeId($typeId) === 'user') {
				$hasUserConfig = true;
			}
			elseif (getScopeFromTypeId($typeId) === 'host') {
				$hasHostConfig = true;
			}
		}
		if ($hasUserConfig) {
			// user options
			$configUserContainer = new htmlResponsiveRow();
			$configUserContainer->add(new htmlSubTitle(_("Users")), 12);
			foreach ($allScopes[get_class($this)] as $typeId) {
				if (!(getScopeFromTypeId($typeId) === 'user')) {
					continue;
				}
				if (sizeof($allScopes[get_class($this)]) > 1) {
					$title = new htmlDiv(null, new htmlOutputText($typeManager->getConfiguredType($typeId)->getAlias()));
					$title->setCSSClasses(array('bold', 'responsiveLabel'));
					$configUserContainer->add($title, 12, 6);
					$configUserContainer->add(new htmlOutputText('&nbsp;', false), 0, 6);
				}
				$uidGeneratorSelect = new htmlResponsiveSelect('posixAccount_' . $typeId . '_uidGeneratorUsers', $generatorOptions, array('range'), _('UID generator'), 'uidGenerator');
				$uidGeneratorSelect->setHasDescriptiveElements(true);
				$uidGeneratorSelect->setTableRowsToHide(array(
						'range' => array('posixAccount_' . $typeId . '_sambaIDPoolDNUsers', 'posixAccount_' . $typeId . '_windowsIDPoolDNUsers',
									'posixAccount_' . $typeId . '_magicNumberUser'),
						'sambaPool' => array('posixAccount_' . $typeId . '_minUID', 'posixAccount_' . $typeId . '_maxUID',
									'posixAccount_' . $typeId . '_windowsIDPoolDNUsers', 'posixAccount_' . $typeId . '_magicNumberUser'),
						'windowsDomain' => array('posixAccount_' . $typeId . '_minUID', 'posixAccount_' . $typeId . '_maxUID',
									'posixAccount_' . $typeId . '_sambaIDPoolDNUsers', 'posixAccount_' . $typeId . '_magicNumberUser'),
						'magicNumber' => array('posixAccount_' . $typeId . '_minUID', 'posixAccount_' . $typeId . '_maxUID',
									'posixAccount_' . $typeId . '_windowsIDPoolDNUsers', 'posixAccount_' . $typeId . '_sambaIDPoolDNUsers')
				));
				$uidGeneratorSelect->setTableRowsToShow(array(
						'range' => array('posixAccount_' . $typeId . '_minUID', 'posixAccount_' . $typeId . '_maxUID'),
						'sambaPool' => array('posixAccount_' . $typeId . '_sambaIDPoolDNUsers'),
						'windowsDomain' => array('posixAccount_' . $typeId . '_windowsIDPoolDNUsers'),
						'magicNumber' => array('posixAccount_' . $typeId . '_magicNumberUser')
				));
				$configUserContainer->add($uidGeneratorSelect, 12);
				$uidUsersGeneratorDN = new htmlResponsiveInputField(_('Samba ID pool DN'), 'posixAccount_' . $typeId . '_sambaIDPoolDNUsers', null, 'sambaIDPoolDN');
				$uidUsersGeneratorDN->setRequired(true);
				$configUserContainer->add($uidUsersGeneratorDN, 12);
				$uidUsersGeneratorWinDN = new htmlResponsiveInputField(_('Windows domain info DN'), 'posixAccount_' . $typeId . '_windowsIDPoolDNUsers', null, 'windowsIDPoolDN');
				$uidUsersGeneratorWinDN->setRequired(true);
				$configUserContainer->add($uidUsersGeneratorWinDN, 12);
				$minUid = new htmlResponsiveInputField(_('Minimum UID number'), 'posixAccount_' . $typeId . '_minUID', null, 'minMaxUser');
				$minUid->setRequired(true);
				$configUserContainer->add($minUid, 12);
				$maxUid = new htmlResponsiveInputField(_('Maximum UID number'), 'posixAccount_' . $typeId . '_maxUID', null, 'minMaxUser');
				$maxUid->setRequired(true);
				$configUserContainer->add($maxUid, 12);
				$magicNumberUser = new htmlResponsiveInputField(_('Magic number'), 'posixAccount_' . $typeId . '_magicNumberUser', null, 'magicNumber');
				$magicNumberUser->setRequired(true);
				$configUserContainer->add($magicNumberUser, 12);
				$configUserContainer->add(new htmlResponsiveInputField(_('Suffix for UID/user name check'), 'posixAccount_' . $typeId . '_uidCheckSuffixUser', '', 'uidCheckSuffix'), 12);
				$configUserContainer->add(new htmlResponsiveInputField(_('User name suggestion'), 'posixAccount_' . $typeId . '_userNameSuggestion', '@givenname@%sn%', 'userNameSuggestion'), 12);
				$configUserContainer->addVerticalSpacer('2rem');
				$hiddenOptionsContainerHead = new htmlGroup();
				$hiddenOptionsContainerHead->addElement(new htmlOutputText(_('Hidden options')));
				$hiddenOptionsContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
				$configUserContainer->add($hiddenOptionsContainerHead, 12);
				$configUserContainer->addVerticalSpacer('0.5rem');
				$configUserContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hidegecos', false, _('Gecos'), null, true), 12, 4, 4);
				$configUserContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hidepassword', false, _('Password'), null, true), 12, 4, 4);
				$confActiveGONModules = array_merge($_SESSION['conf_config']->get_AccountModules('group'), $_SESSION['conf_config']->get_AccountModules('gon'));
				if (in_array('groupOfNames', $confActiveGONModules) || in_array('groupOfMembers', $confActiveGONModules) || in_array('groupOfUniqueNames', $confActiveGONModules)) {
					$configUserContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hidegon', false, _('Groups of names'), null, true), 12, 4, 4);
					$configUserContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hideCreateGroup', false, _('Create group with same name'), null, true), 12, 4, 4);
					$configUserContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hideposixGroups', false, _('Unix groups'), null, true), 12, 4, 4);
					$syncGroupsCheckbox = new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_syncGroups', false, _('Sync groups'), null, true);
					$syncGroupsCheckbox->setTableRowsToHide(array('posixAccount_' . $typeId . '_syncGroupsExclusions'));
					$configUserContainer->add($syncGroupsCheckbox, 12, 4, 4);
					$configUserContainer->add(new htmlResponsiveInputTextarea('posixAccount_' . $typeId . '_syncGroupsExclusions', '', 20, 4, _('Exclude from group sync'), 'excludeFromGroupSync'), 12);
				}
			}
			$return[] = $configUserContainer;
		}
		// host options
		if ($hasHostConfig) {
			$configHostContainer = new htmlResponsiveRow();
			$configHostContainer->add(new htmlSubTitle(_("Hosts")), 12);
			foreach ($allScopes[get_class($this)] as $typeId) {
				if (!(getScopeFromTypeId($typeId) === 'host')) {
					continue;
				}
				if (sizeof($allScopes[get_class($this)]) > 1) {
					$title = new htmlDiv(null, new htmlOutputText($typeManager->getConfiguredType($typeId)->getAlias()));
					$title->setCSSClasses(array('bold', 'responsiveLabel'));
					$configHostContainer->add($title, 12, 6);
					$configHostContainer->add(new htmlOutputText('&nbsp;', false), 0, 6);
				}
				$uidHostGeneratorSelect = new htmlResponsiveSelect('posixAccount_' . $typeId . '_uidGeneratorHosts', $generatorOptions, array('range'), _('UID generator'), 'uidGenerator');
				$uidHostGeneratorSelect->setHasDescriptiveElements(true);
				$uidHostGeneratorSelect->setTableRowsToHide(array(
						'range' => array('posixAccount_' . $typeId . '_sambaIDPoolDNHosts', 'posixAccount_' . $typeId . '_windowsIDPoolDNHosts',
										'posixAccount_' . $typeId . '_magicNumberHost'),
						'sambaPool' => array('posixAccount_' . $typeId . '_minMachine', 'posixAccount_' . $typeId . '_maxMachine',
										'posixAccount_' . $typeId . '_windowsIDPoolDNHosts', 'posixAccount_' . $typeId . '_magicNumberHost'),
						'windowsDomain' => array('posixAccount_' . $typeId . '_minMachine', 'posixAccount_' . $typeId . '_maxMachine',
										'posixAccount_' . $typeId . '_sambaIDPoolDNHosts', 'posixAccount_' . $typeId . '_magicNumberHost'),
						'magicNumber' => array('posixAccount_' . $typeId . '_minMachine', 'posixAccount_' . $typeId . '_maxMachine',
										'posixAccount_' . $typeId . '_windowsIDPoolDNHosts', 'posixAccount_' . $typeId . '_sambaIDPoolDNHosts')
				));
				$uidHostGeneratorSelect->setTableRowsToShow(array(
						'range' => array('posixAccount_' . $typeId . '_minMachine', 'posixAccount_' . $typeId . '_maxMachine'),
						'sambaPool' => array('posixAccount_' . $typeId . '_sambaIDPoolDNHosts'),
						'windowsDomain' => array('posixAccount_' . $typeId . '_windowsIDPoolDNHosts'),
						'magicNumber' => array('posixAccount_' . $typeId . '_magicNumberHost')
				));
				$configHostContainer->add($uidHostGeneratorSelect, 12);
				$uidHostsGeneratorDN = new htmlResponsiveInputField(_('Samba ID pool DN'), 'posixAccount_' . $typeId . '_sambaIDPoolDNHosts', null, 'sambaIDPoolDN');
				$uidHostsGeneratorDN->setRequired(true);
				$configHostContainer->add($uidHostsGeneratorDN, 12);
				$uidHostsGeneratorWinDN = new htmlResponsiveInputField(_('Windows domain info DN'), 'posixAccount_' . $typeId . '_windowsIDPoolDNHosts', null, 'windowsIDPoolDN');
				$uidHostsGeneratorWinDN->setRequired(true);
				$configHostContainer->add($uidHostsGeneratorWinDN, 12);
				$minUid = new htmlResponsiveInputField(_('Minimum UID number'), 'posixAccount_' . $typeId . '_minMachine', null, 'minMaxHost');
				$minUid->setRequired(true);
				$configHostContainer->add($minUid, 12);
				$maxUid = new htmlResponsiveInputField(_('Maximum UID number'), 'posixAccount_' . $typeId . '_maxMachine', null, 'minMaxHost');
				$maxUid->setRequired(true);
				$configHostContainer->add($maxUid, 12);
				$magicNumberHost = new htmlResponsiveInputField(_('Magic number'), 'posixAccount_' . $typeId . '_magicNumberHost', null, 'magicNumber');
				$magicNumberHost->setRequired(true);
				$configHostContainer->add($magicNumberHost, 12);
				$configHostContainer->add(new htmlResponsiveInputField(_('Suffix for UID/user name check'), 'posixAccount_' . $typeId . '_uidCheckSuffixHost', '', 'uidCheckSuffix'), 12);
				$hiddenOptionsContainerHead = new htmlGroup();
				$hiddenOptionsContainerHead->addElement(new htmlOutputText(_('Hidden options')));
				$hiddenOptionsContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
				$configHostContainer->addLabel($hiddenOptionsContainerHead, 12);
				$configHostContainer->addField(new htmlOutputText(''));
				$configHostContainer->addVerticalSpacer('0.5rem');
				$configHostContainer->add(new htmlResponsiveInputCheckbox('posixAccount_' . $typeId . '_hidegecos', false, _('Gecos'), null, false), 12);
				$configHostContainer->addVerticalSpacer('2rem');
			}
			$return[] = $configHostContainer;
		}
		// common options
		$configOptionsContainer = new htmlResponsiveRow();
		$configOptionsContainer->add(new htmlSubTitle(_('Options')), 12);
		$configOptionsContainer->add(new htmlResponsiveSelect('posixAccount_pwdHash', getSupportedHashTypes(),
				array('CRYPT-SHA512'), _("Password hash type"), 'pwdHash'), 12);
		$configOptionsContainer->add(new htmlResponsiveInputTextarea('posixAccount_shells', implode("\r\n", $this->getShells()), 30, 4, _('Login shells'), 'loginShells'), 12);
		$configOptionsContainer->add(new htmlResponsiveInputCheckbox('posixAccount_primaryGroupAsSecondary', false, _('Set primary group as memberUid'), 'primaryGroupAsSecondary'), 12);
		if ($isWindows) {
			$configOptionsContainer->add(new htmlResponsiveInputCheckbox('posixAccount_noObjectClass', false, _('Do not add object class'), 'noObjectClass'), 12);
		}
		$return[] = $configOptionsContainer;

		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::check_configOptions()
	 */
	public function check_configOptions($typeIds, &$options) {
		$return = array();
		$scopes = array();
		$ranges = array();
		foreach ($typeIds as $typeId) {
			$scopes[] = getScopeFromTypeId($typeId);
			// user settings
			if (getScopeFromTypeId($typeId) === 'user') {
				if ($options['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'range') {
					// min/maxUID are required, check if they are numeric
					if (!isset($options['posixAccount_' . $typeId . '_minUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_' . $typeId . '_minUID'][0])) {
						$return[] = $this->messages['minUID'][0];
					}
					if (!isset($options['posixAccount_' . $typeId . '_maxUID'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_' . $typeId . '_maxUID'][0])) {
						$return[] = $this->messages['maxUID'][0];
					}
					// minUID < maxUID
					if (isset($options['posixAccount_' . $typeId . '_minUID'][0]) && isset($options['posixAccount_' . $typeId . '_maxUID'][0])) {
						if ($options['posixAccount_' . $typeId . '_minUID'][0] >= $options['posixAccount_' . $typeId . '_maxUID'][0]) {
							$return[] = $this->messages['cmp_UID'][0];
						}
					}
					$ranges[] = array($options['posixAccount_' . $typeId . '_minUID'][0], $options['posixAccount_' . $typeId . '_maxUID'][0]);
				}
				elseif ($options['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'sambaPool') {
					if (!isset($options['posixAccount_' . $typeId . '_sambaIDPoolDNUsers'][0]) || !get_preg($options['posixAccount_' . $typeId . '_sambaIDPoolDNUsers'][0], 'dn')) {
						$return[] = $this->messages['sambaIDPoolDN'][0];
					}
				}
				elseif ($options['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'windowsDomain') {
					if (!isset($options['posixAccount_' . $typeId . '_windowsIDPoolDNUsers'][0]) || !get_preg($options['posixAccount_' . $typeId . '_windowsIDPoolDNUsers'][0], 'dn')) {
						$return[] = $this->messages['windowsIDPoolDN'][0];
					}
				}
			}
			// host settings
			if (getScopeFromTypeId($typeId) === 'host') {
				if ($options['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'range') {
					// min/maxUID are required, check if they are numeric
					if (!isset($options['posixAccount_' . $typeId . '_minMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_' . $typeId . '_minMachine'][0])) {
						$return[] = $this->messages['minMachine'][0];
					}
					if (!isset($options['posixAccount_' . $typeId . '_maxMachine'][0]) || !preg_match('/^[0-9]+$/', $options['posixAccount_' . $typeId . '_maxMachine'][0])) {
						$return[] = $this->messages['maxMachine'][0];
					}
					// minUID < maxUID
					if (isset($options['posixAccount_' . $typeId . '_minMachine'][0]) && isset($options['posixAccount_' . $typeId . '_maxMachine'][0])) {
						if ($options['posixAccount_' . $typeId . '_minMachine'][0] >= $options['posixAccount_' . $typeId . '_maxMachine'][0]) {
							$return[] = $this->messages['cmp_Machine'][0];
						}
					}
					$ranges[] = array($options['posixAccount_' . $typeId . '_minMachine'][0], $options['posixAccount_' . $typeId . '_maxMachine'][0]);
				}
				elseif ($options['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'sambaPool') {
					if (!isset($options['posixAccount_' . $typeId . '_sambaIDPoolDNHosts'][0]) || !get_preg($options['posixAccount_' . $typeId . '_sambaIDPoolDNHosts'][0], 'dn')) {
						$return[] = $this->messages['sambaIDPoolDN'][0];
					}
				}
				elseif ($options['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'windowsDomain') {
					if (!isset($options['posixAccount_' . $typeId . '_windowsIDPoolDNHosts'][0]) || !get_preg($options['posixAccount_' . $typeId . '_windowsIDPoolDNHosts'][0], 'dn')) {
						$return[] = $this->messages['windowsIDPoolDN'][0];
					}
				}
			}
		}
		// check if user and host ranges overlap
		foreach ($ranges as $range) {
			foreach ($ranges as $rangeToCompare) {
				// check if minimum is inside other range
				if (($rangeToCompare[0] > $range[0]) && ($rangeToCompare[0] < $range[1])) {
					$return[] = $this->messages['cmp_both'][0];
					break 2;
				}
				// check if maximum is inside other range
				if (($rangeToCompare[1] > $range[0]) && ($rangeToCompare[1] < $range[1])) {
					$return[] = $this->messages['cmp_both'][0];
					break 2;
				}
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	function get_uploadColumns($selectedModules, &$type) {
		$typeId = $type->getId();
		$return = parent::get_uploadColumns($selectedModules, $type);
		if ($this->isPasswordManaged($typeId)) {
			$return[] = array(
				'name' => 'posixAccount_password',
				'description' => _('Password'),
				'help' => 'userPassword',
				'example' => _('secret')
			);
			$return[] = array(
				'name' => 'posixAccount_passwordDisabled',
				'description' => _('Lock password'),
				'help' => 'userPassword_lock',
				'example' => 'false',
				'values' => 'true, false',
				'default' => 'false'
			);
		}
		if (($this->get_scope() == 'user') && $this->manageCn($selectedModules)) {
			array_unshift($return, array(
				'name' => 'posixAccount_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller')
			));
		}
		if (($this->get_scope() == 'user') && !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
			$return[] = array(
				'name' => 'posixAccount_gecos',
				'description' => _('Gecos'),
				'help' => 'gecos',
				'example' => _('Steve Miller,Room 2.14,123-123-1234,123-123-1234')
			);
		}
		if (($this->get_scope() == 'host') && !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
			$return[] = array(
				'name' => 'posixAccount_gecos',
				'description' => _('Gecos'),
				'help' => 'gecos',
				'example' => _('pc01,Room 2.34')
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
		$typeId = $type->getId();
		$pwdAttrName = $this->getPasswordAttrName($selectedModules);
		$homedirAttrName = $this->getHomedirAttrName($selectedModules);
		$needAutoUID = array();
		// get list of existing users
		$existingUsers = $this->getUserNames($typeId);
		// get list of existing groups
		$groupList = $this->findGroups($selectedModules);
		$groupMap = array();
		for ($i = 0; $i < sizeof($groupList); $i++) {
			$groupMap[$groupList[$i][1]] = $groupList[$i][0];
		}
		$existingGroups = array_keys($groupMap);
		// get list of existing group of names
		if (self::areGroupOfNamesActive()) {
			$gons = $this->findGroupOfNames();
			$gonList = array();
			foreach ($gons as $attr) {
				$gonList[] = $attr['cn'][0];
			}
		}
		// check input
		foreach ($rawAccounts as $i => $rawAccount) {
			if (!$this->skipObjectClass() && !in_array("posixAccount", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "posixAccount";
			}
			// UID
			if ($rawAccount[$ids['posixAccount_uid']] == "") {
				// autoUID
				$needAutoUID[] = $i;
			}
			elseif (get_preg($rawAccount[$ids['posixAccount_uid']], 'digit')) {
				if (($this->get_scope() == 'user') && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'range')) {
					if (($rawAccount[$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_' . $typeId . '_minUID'][0]) &&
						($rawAccount[$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_' . $typeId . '_maxUID'][0])) {
						$partialAccounts[$i]['uidNumber'] = trim($rawAccount[$ids['posixAccount_uid']]);
					}
					else {
						$errMsg = $this->messages['uidNumber'][4];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				elseif (($this->get_scope() == 'host') && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'range')) {
					if (($rawAccount[$ids['posixAccount_uid']] > $this->moduleSettings['posixAccount_' . $typeId . '_minMachine'][0]) &&
						($rawAccount[$ids['posixAccount_uid']] < $this->moduleSettings['posixAccount_' . $typeId . '_maxMachine'][0])) {
						$partialAccounts[$i]['uidNumber'] = trim($rawAccount[$ids['posixAccount_uid']]);
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
			if (get_preg($rawAccount[$ids['posixAccount_group']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccount[$ids['posixAccount_group']];
			}
			if (get_preg($rawAccount[$ids['posixAccount_group']], 'groupname')) {
				$groupName = $rawAccount[$ids['posixAccount_group']];
				$gid = nuLL;
				if (isset($groupMap[$groupName])) {
					$gid = $groupMap[$groupName];
				}
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
			if (!$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidegecos')) {
				if (!empty($rawAccount[$ids['posixAccount_gecos']])) {
					if (get_preg($rawAccount[$ids['posixAccount_gecos']], 'gecos')) {
						$partialAccounts[$i]['gecos'] = $this->checkASCII($rawAccount[$ids['posixAccount_gecos']]);
					}
					else {
						$errMsg = $this->messages['gecos'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
				}
				else {
					$gecos = "";
					if (($rawAccount[$ids['inetOrgPerson_firstName']] != "") && ($rawAccount[$ids['inetOrgPerson_lastName']] != "")) {
						$gecos = $rawAccount[$ids['inetOrgPerson_firstName']] . " " . $rawAccount[$ids['inetOrgPerson_lastName']];
						if (!empty($rawAccount[$ids['inetOrgPerson_telephone']])) {
							$gecos = $gecos . ",," . $rawAccount[$ids['inetOrgPerson_telephone']];  // double "," because room is unknown
							if (!empty($rawAccount[$ids['inetOrgPerson_fax']])) {
								$gecos = $gecos . "," . $rawAccount[$ids['inetOrgPerson_fax']];
							}
						}
					}
					if (!empty($gecos)) {
						$partialAccounts[$i]['gecos'] = $this->checkASCII($gecos);
					}
				}
			}
			// user specific attributes
			if ($this->get_scope() == 'user') {
				// additional groups
				if ($rawAccount[$ids['posixAccount_additionalGroups']] != "") {
					$groups = explode(",", $rawAccount[$ids['posixAccount_additionalGroups']]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $existingGroups)) {
							$errors[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						}
					}
				}
				// group of names
				if (self::areGroupOfNamesActive() && ($rawAccount[$ids['posixAccount_gon']] != "")) {
					$groups = explode(",", $rawAccount[$ids['posixAccount_gon']]);
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $gonList)) {
							$errors[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						}
					}
				}
				// user name
				if (array_key_exists($rawAccount[$ids['posixAccount_userName']], $existingUsers)) {
					$userName = $rawAccount[$ids['posixAccount_userName']];
					while (array_key_exists($userName, $existingUsers)) {
						$userName = $this->getNextUserName($userName, $selectedModules);
					}
					$errMsg = $this->messages['uid'][9];
					array_push($errMsg, array($i, $userName, $rawAccount[$ids['posixAccount_userName']],
						htmlspecialchars($existingUsers[$rawAccount[$ids['posixAccount_userName']]])));
					$errors[] = $errMsg;
				}
				if (get_preg($rawAccount[$ids['posixAccount_userName']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccount[$ids['posixAccount_userName']];
				}
				else {
					$errMsg = $this->messages['uid'][7];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// home directory
				if ($rawAccount[$ids['posixAccount_homedir']] == "") {
					$partialAccounts[$i][$homedirAttrName] = '/home/' . $partialAccounts[$i]['uid'];
				}
				elseif (get_preg($rawAccount[$ids['posixAccount_homedir']], 'homeDirectory')) {
					$partialAccounts[$i][$homedirAttrName] = $rawAccount[$ids['posixAccount_homedir']];
				}
				else {
					$errMsg = $this->messages['homeDirectory'][2];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// login shell
				if ($rawAccount[$ids['posixAccount_shell']] == "") {
					$partialAccounts[$i]['loginShell'] = '/bin/bash';
				}
				elseif (in_array($rawAccount[$ids['posixAccount_shell']], $this->getShells())) {
					$partialAccounts[$i]['loginShell'] = $rawAccount[$ids['posixAccount_shell']];
				}
				else {
					$errMsg = $this->messages['shell'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				if ($this->isPasswordManaged($typeId)) {
					$pwd_enabled = true;
					// password enabled/disabled
					if ($rawAccount[$ids['posixAccount_passwordDisabled']] == "") {
						$pwd_enabled = true;
					}
					elseif (in_array($rawAccount[$ids['posixAccount_passwordDisabled']], array('true', 'false'))) {
						if ($rawAccount[$ids['posixAccount_passwordDisabled']] == 'true') {
							$pwd_enabled = false;
						}
						else {
							$pwd_enabled = true;
						}
					}
					else {
						$errMsg = $this->messages['passwordDisabled'][0];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
					}
					// password
					// delay exop passwords
					if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
						// changed in post action
					}
					// set SASL passwords
					elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'SASL')) {
						$partialAccounts[$i][$pwdAttrName] = '{SASL}' . $partialAccounts[$i]['uid'];
					}
					// set K5KEY password
					elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'K5KEY')) {
						$partialAccounts[$i][$pwdAttrName] = pwd_hash('x', true, $this->moduleSettings['posixAccount_pwdHash'][0]);
					}
					// set normal password
					else {
						if (($rawAccount[$ids['posixAccount_password']] != "") && (get_preg($rawAccount[$ids['posixAccount_password']], 'password'))) {
							$partialAccounts[$i][$pwdAttrName] = pwd_hash($rawAccount[$ids['posixAccount_password']], $pwd_enabled, $this->moduleSettings['posixAccount_pwdHash'][0]);
							$partialAccounts[$i]['INFO.userPasswordClearText'] = $rawAccount[$ids['posixAccount_password']]; // for custom scripts etc.
						}
						elseif ($rawAccount[$ids['posixAccount_password']] != "") {
							$errMsg = $this->messages['userPassword'][4];
							$errMsg[2] = str_replace('%', '%%', $errMsg[2]); // double "%" because of later sprintf
							array_push($errMsg, array($i));
							$errors[] = $errMsg;
						}
					}
				}
				// cn
				if ($this->manageCn($selectedModules)) {
					if ($rawAccount[$ids['posixAccount_cn']] != "") {
						if (get_preg($rawAccount[$ids['posixAccount_cn']], 'cn')) {
							$partialAccounts[$i]['cn'] = $rawAccount[$ids['posixAccount_cn']];
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
			}
			// host specific attributes
			elseif ($this->get_scope() == 'host') {
				// host name
				if (array_key_exists($rawAccount[$ids['posixAccount_hostName']], $existingUsers)) {
					$userName = $rawAccount[$ids['posixAccount_hostName']];
					while (array_key_exists($userName, $existingUsers)) {
						$userName = $this->getNextUserName($userName, $selectedModules);
					}
					$errMsg = $this->messages['uid'][10];
					array_push($errMsg, array($i, $userName, $rawAccount[$ids['posixAccount_hostName']],
						htmlspecialchars($existingUsers[$rawAccount[$ids['posixAccount_hostName']]])));
					$errors[] = $errMsg;
				}
				if (get_preg($rawAccount[$ids['posixAccount_hostName']], 'hostname')) {
					$partialAccounts[$i]['uid'] = $rawAccount[$ids['posixAccount_hostName']];
					$partialAccounts[$i]['cn'] = $rawAccount[$ids['posixAccount_hostName']];
				}
				else {
					$errMsg = $this->messages['uid'][8];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
				// description
				if (isset($ids['posixAccount_description']) && isset($rawAccount[$ids['posixAccount_description']]) && ($rawAccount[$ids['posixAccount_description']] != '')) {
					$partialAccounts[$i]['description'] = $rawAccount[$ids['posixAccount_description']];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccount[$ids['posixAccount_hostName']];
				}
				$partialAccounts[$i][$homedirAttrName] = '/dev/null';
				$partialAccounts[$i]['loginShell'] = '/bin/false';
			}
		}
		// fill in autoUIDs
		if (sizeof($needAutoUID) > 0) {
			$errorsTemp = array();
			$uids = $this->getNextUIDs(sizeof($needAutoUID), $errorsTemp, $typeId);
			if (is_array($uids)) {
				foreach ($needAutoUID as $i => $index) {
					$partialAccounts[$index]['uidNumber'] = $uids[$i];
				}
			}
			else {
				$errors[] = $this->messages['uidNumber'][2];
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
		$homeDirAttr = $this->getHomedirAttrName($selectedModules);
		if ($this->get_scope() != 'user') {
			return array(
				'status' => 'finished',
				'progress' => 100,
				'errors' => array()
			);
		}
		// on first call generate list of ldap operations
		if (!isset($temp['counter'])) {
			$temp['groups'] = array();
			$temp['dn_gon'] = array();
			$temp['createHomes'] = array();
			$temp['exop'] = array();
			$temp['counter'] = 0;
			$col = $ids['posixAccount_additionalGroups'];
			$col_home = $ids['posixAccount_createHomeDir'];
			// get list of existing groups
			$groupList = $this->findGroups($selectedModules);
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
			foreach ($data as $i => $dataRow) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				if ($dataRow[$col] != "") {
					$groups = explode(",", $dataRow[$col]);
					if (isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
						&& ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] == 'true')) {
						if (get_preg($dataRow[$ids['posixAccount_group']], 'digit')) {
							if (!in_array($groupMap[$dataRow[$ids['posixAccount_group']]], $groups)) {
								$groups[] = $groupMap[$dataRow[$ids['posixAccount_group']]];
							}
						}
						else {
							if (!in_array($groupMap[$dataRow[$ids['posixAccount_group']]], $groups)) {
								$groups[] = $dataRow[$ids['posixAccount_group']];
							}
						}
					}
					for ($g = 0; $g < sizeof($groups); $g++) {
						if (!in_array($groups[$g], $temp['groups'])) $temp['groups'][] = $groups[$g];
						$temp['members'][$groups[$g]][] = $dataRow[$ids['posixAccount_userName']];
					}
				}
				if (isset($ids['posixAccount_gon']) && ($dataRow[$ids['posixAccount_gon']] != "")) {
					$gons = explode(",", $dataRow[$ids['posixAccount_gon']]);
					$memberAttr = 'member';
					for ($g = 0; $g < sizeof($gons); $g++) {
						if (in_array_ignore_case('groupOfUniqueNames', $gonList[$gonMap[$gons[$g]]]['objectclass'])) {
							$memberAttr = 'uniqueMember';
						}
						$temp['dn_gon'][$gonMap[$gons[$g]]][$memberAttr][] = $accounts[$i]['dn'];
					}
				}
				if (!empty($dataRow[$col_home])) {
					$temp['createHomes'][] = $i;
				}
				if (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
					if (isset($ids['posixAccount_password']) && !empty($dataRow[$ids['posixAccount_password']])) {
						$temp['exop'][] = array($accounts[$i]['dn'], $dataRow[$ids['posixAccount_password']]);
					}
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
				$memberUid = $temp['members'][$temp['groups'][$temp['counter']]];
				$dnToUpdate = $temp['dn'][$temp['groups'][$temp['counter']]];
				$groupAttrs = ldapGetDN($dnToUpdate, array('memberUID'));
				if (!empty($groupAttrs['memberuid'])) {
					// skip members that are already set
					$memberUid = array_delete($groupAttrs['memberuid'], $memberUid);
				}
				if (!empty($memberUid)) {
					$toAdd = array('memberUID' => $memberUid);
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $dnToUpdate, $toAdd);
					$errors = array();
					if (!$success) {
						$errors[] = array(
							"ERROR",
							_("LAM was unable to modify group memberships for group: %s"),
							getDefaultLDAPErrorString($_SESSION['ldap']->server()),
							array($temp['groups'][$temp['counter']])
						);
					}
				}
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop'])),
					'errors' => $errors
				);
			}
			else {
				$temp['counter']++;
				return array (
					'status' => 'inProgress',
					'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups'] + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop']))),
					'errors' => array(array('ERROR', _('Unable to find group in LDAP.'), $temp['groups'][$temp['counter']]))
				);
			}
		}
		// create home directories
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']))) {
			$pos = $temp['createHomes'][$temp['counter'] - sizeof($temp['groups'])];
			try {
				$remote = new \LAM\REMOTE\Remote();
				$remoteServer = $_SESSION['config']->getScriptServerByName($data[$pos][$ids['posixAccount_createHomeDir']]);
				$remote->connect($remoteServer);
				$result = $remote->execute(
					implode(
						self::$SPLIT_DELIMITER,
						array(
							$data[$pos][$ids['posixAccount_userName']],
							"home",
							"add",
							$remoteServer->getHomeDirPrefix() . $accounts[$pos][$homeDirAttr],
							"0".$_SESSION['config']->get_scriptRights(),
							$accounts[$pos]['uidNumber'],
							$accounts[$pos]['gidNumber'],
						)
					));
				$remote->disconnect();
				$errors = array();
				if (!empty($result)) {
					$parts = explode(",", $result);
					if (in_array($parts[0], array('ERROR', 'WARN'))) {
						$errors[] = $parts;
					}
				}
			}
			catch (LAMException $e) {
				$errors[] = array('ERROR', $e->getTitle(), $e->getMessage());
			}
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop'])),
				'errors' => $errors
			);
		}
		// add users to group of names
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']))) {
			$gonDn = $temp['dn_gon_keys'][$temp['counter'] - sizeof($temp['groups']) - sizeof($temp['createHomes'])];
			$gonAttrToAdd = $temp['dn_gon'][$gonDn];
			$gonAttrNames = array_keys($gonAttrToAdd);
			$gonAttrs = ldapGetDN($gonDn, $gonAttrNames);
			foreach ($gonAttrNames as $gonAttrName) {
				$gonAttrNameLower = strtolower($gonAttrName);
				if (!empty($gonAttrs[$gonAttrNameLower])) {
					$gonAttrToAdd[$gonAttrName] = array_delete($gonAttrs[$gonAttrNameLower], $gonAttrToAdd[$gonAttrName]);
				}
				if (empty($gonAttrToAdd[$gonAttrName])) {
					unset($gonAttrToAdd[$gonAttrName]);
				}
			}
			if (!empty($gonAttrToAdd)) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $gonDn, $gonAttrToAdd);
				$errors = array();
				if (!$success) {
					$errors[] = array(
						"ERROR",
						_("LAM was unable to modify group memberships for group: %s"),
						getDefaultLDAPErrorString($_SESSION['ldap']->server()),
						array($temp['groups'][$temp['counter']])
					);
				}
			}
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop'])),
				'errors' => $errors
			);
		}
		// run password exop commands
		elseif ($temp['counter'] < (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop']))) {
			$data = $temp['exop'][$temp['counter'] - sizeof($temp['groups']) - sizeof($temp['createHomes']) - sizeof($temp['dn_gon'])];
			$dn = $data[0];
			$password = $data[1];
			$success = ldap_exop_passwd($_SESSION['ldap']->server(), $dn, null, $password);
			$errors = array();
			if (!$success) {
				$errors[] = array(
					"ERROR",
					_('Unable to set password'),
					$dn . '<br>' . getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					array($temp['groups'][$temp['counter']])
				);
			}
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / (sizeof($temp['groups']) + sizeof($temp['createHomes']) + sizeof($temp['dn_gon']) + sizeof($temp['exop'])),
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
	* @param string $typeId type id (e.g. user)
	* @return mixed Null if no UIDs are free else an array of free UIDs.
	*/
	function getNextUIDs($count, &$errors, $typeId) {
		// check if UIDs should be taken from Samba pool entry
		if (($this->get_scope() == 'user') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'sambaPool')) {
			return $this->getNextSambaPoolUIDs($count, $errors, $typeId);
		}
		if (($this->get_scope() == 'host') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'sambaPool')) {
			return $this->getNextSambaPoolUIDs($count, $errors, $typeId);
		}
		// check if UIDs should be taken from domain info pool entry
		if (($this->get_scope() == 'user') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'windowsDomain')) {
			return $this->getNextDomainInfoUIDs($count, $errors, $typeId);
		}
		if (($this->get_scope() == 'host') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'windowsDomain')) {
			return $this->getNextDomainInfoUIDs($count, $errors, $typeId);
		}
		// check if a magic number should be used
		if (($this->get_scope() == 'user') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorUsers'][0] == 'magicNumber')) {
			$return = array();
			for ($i = 0; $i < $count; $i++) {
				$return[] = $this->moduleSettings['posixAccount_' . $typeId . '_magicNumberUser'][0];
			}
			return $return;
		}
		if (($this->get_scope() == 'host') && isset($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts']) && ($this->moduleSettings['posixAccount_' . $typeId . '_uidGeneratorHosts'][0] == 'magicNumber')) {
			$return = array();
			for ($i = 0; $i < $count; $i++) {
				$return[] = $this->moduleSettings['posixAccount_' . $typeId . '_magicNumberHost'][0];
			}
			return $return;
		}
		$ret = array();
		if ($this->get_scope() == "user") {
			$minID = intval($this->moduleSettings['posixAccount_' . $typeId . '_minUID'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_' . $typeId . '_maxUID'][0]);
		}
		else {
			$minID = intval($this->moduleSettings['posixAccount_' . $typeId . '_minMachine'][0]);
			$maxID = intval($this->moduleSettings['posixAccount_' . $typeId . '_maxMachine'][0]);
		}
		$uidList = $this->getUIDs($typeId);
		$uids = array();
		foreach ($uidList as $uid) {
			if (($uid <= $maxID) && ($uid >= $minID)) $uids[] = $uid;  // ignore UIDs > maxID and UIDs < minID
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
	 * @param string $typeId type id (e.g. user)
	 * @return mixed null if no UIDs are free else an array of free UIDs
	 */
	private function getNextSambaPoolUIDs($count, &$errors, $typeId) {
		if ($this->get_scope() == 'user') {
			$dn = $this->moduleSettings['posixAccount_' . $typeId . '_sambaIDPoolDNUsers'][0];
		}
		else {
			$dn = $this->moduleSettings['posixAccount_' . $typeId . '_sambaIDPoolDNHosts'][0];
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
	 * Gets the free UID numbers from a domain info entry in LDAP.
	 *
	 * @param integer $count number of needed free UIDs.
	 * @param array $errors list of error messages where errors can be added
	 * @param string $typeId type id (e.g. user)
	 * @return mixed null if no UIDs are free else an array of free UIDs
	 */
	private function getNextDomainInfoUIDs($count, &$errors, $typeId) {
		if ($this->get_scope() == 'user') {
			$dn = $this->moduleSettings['posixAccount_' . $typeId . '_windowsIDPoolDNUsers'][0];
		}
		else {
			$dn = $this->moduleSettings['posixAccount_' . $typeId . '_windowsIDPoolDNHosts'][0];
		}
		$attrs = ldapGetDN($dn, array('msSFU30MaxUidNumber'));
		if (!empty($attrs['mssfu30maxuidnumber'][0])) {
			$newValue = $attrs['mssfu30maxuidnumber'][0] + $count;
			$ldapHandle = $_SESSION['ldap']->server();
			ldap_modify($ldapHandle, $dn, array('mssfu30maxuidnumber' => array($newValue)));
			logNewMessage(LOG_DEBUG, 'Updated domain info ' . $dn . ' with UID number ' . $newValue . ' and LDAP code ' . ldap_errno($ldapHandle));
			if (ldap_errno($ldapHandle) != 0) {
				logNewMessage(LOG_NOTICE, 'Updating domain info ' . $dn . ' with UID number ' . $newValue . ' failed. ' . ldap_error($ldapHandle));
				return null;
			}
			$result = array();
			for ($i = 0; $i < $count; $i++) {
				$result[] = $attrs['mssfu30maxuidnumber'][0] + $i;
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
	 * @return array list of meta HTML elements (field name => htmlResponsiveRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array();
		if (in_array('password', $fields)) {
			$row = new htmlResponsiveRow();
			if (!empty($this->selfServiceSettings->moduleSettings['posixAccount_useOldPwd']) && ($this->selfServiceSettings->moduleSettings['posixAccount_useOldPwd'][0] == 'true')) {
				$pwd0 = new htmlResponsiveInputField(_('Old password'), 'posixAccount_passwordOld');
				$pwd0->setIsPassword(true, true);
				$row->add($pwd0, 12);
			}
			$pwd1 = new htmlResponsiveInputField($this->getSelfServiceLabel('password', _('New password')), 'posixAccount_password');
			$pwd1->setIsPassword(true, true);
			$row->add($pwd1, 12);
			$pwd2 = new htmlResponsiveInputField(_('Reenter password'), 'posixAccount_password2');
			$pwd2->setIsPassword(true);
			$pwd2->setSameValueFieldID('posixAccount_password');
			$row->add($pwd2, 12);
			$return['password'] = $row;
		}
		if ($passwordChangeOnly) {
			return $return; // only password fields as long no LDAP content can be read
		}
		if (in_array('cn', $fields)) {
			$cn = '';
			if (isset($attributes['cn'][0])) {
				$cn = $attributes['cn'][0];
			}
			$cnField = new htmlInputField('posixAccount_cn', $cn);
			if (in_array('cn', $readOnlyFields)) {
				$cnField = new htmlOutputText($cn);
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('cn', _('Common name'))));
			$row->addField($cnField);
			$return['cn'] = $row;
		}
		if (in_array('loginShell', $fields)) {
			$shelllist = $this->getShells(); // list of all valid shells
			$loginShell = '';
			if (isset($attributes['loginShell'][0])) {
				$loginShell = $attributes['loginShell'][0];
			}
			$loginShellField = new htmlSelect('posixAccount_loginShell', $shelllist, array($loginShell));
			if (in_array('loginShell', $readOnlyFields)) {
				$loginShellField = new htmlOutputText($loginShell);
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('loginShell', _('Login shell'))));
			$row->addField($loginShellField);
			$return['loginShell'] = $row;
		}
		if (in_array('unixgroups', $fields) && !empty($this->selfServiceSettings->moduleSettings['posixAccount_groupDn'][0])) {
			$groupDn = $this->selfServiceSettings->moduleSettings['posixAccount_groupDn'][0];
			$gidNumber = $attributes['gidNumber'][0];
			$userName = $attributes['uid'][0];
			if (!empty($userName)) {
				$filter = '(&(objectClass=posixGroup)(|(gidNumber=' . $gidNumber . ')(memberUid=' . $userName . ')))';
				$results = searchLDAP($groupDn, $filter, array('cn'));
				$groups = array();
				foreach ($results as $result) {
					$groups[] = $result['cn'][0];
				}
				$groups = array_unique($groups);
				natcasesort($groups);
				$row = new htmlResponsiveRow();
				$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('unixgroups', _('Groups'))));
				$row->addField(new htmlOutputText(implode(', ', $groups)));
				$return['unixgroups'] = $row;
			}
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
						$userName = empty($attributes['uid'][0]) ? null : $attributes['uid'][0];
						$additionalAttrs = array();
						if (!empty($attributes['sn'][0])) {
							$additionalAttrs[] = $attributes['sn'][0];
						}
						if (!empty($attributes['givenName'][0])) {
							$additionalAttrs[] = $attributes['givenName'][0];
						}
						$pwdPolicyResult = checkPasswordStrength($_POST['posixAccount_password'], $userName, $additionalAttrs);
						if ($pwdPolicyResult === true) {
							$passwordHash = $this->selfServiceSettings->moduleSettings['posixAccount_pwdHash'][0];
							if (empty($this->selfServiceSettings->moduleSettings['posixAccount_useOldPwd']) || ($this->selfServiceSettings->moduleSettings['posixAccount_useOldPwd'][0] != 'true')) {
								// set SASL password
								if (!empty($attributes['uid'][0]) && ($passwordHash === 'SASL')) {
									$return['mod']['userPassword'][0] = '{SASL}' . $attributes['uid'][0];
								}
								elseif ($passwordHash === 'LDAP_EXOP') {
									// no LDAP modify action, use ldap_exop_passwd
									$return['info']['userPasswordModify'][0] = 'exop';
								}
								// set other password hashes
								else {
									$return['mod']['userPassword'][0] = pwd_hash($_POST['posixAccount_password'], true, $passwordHash);
								}

							}
							else {
								$return['add']['userPassword'][0] = pwd_hash($_POST['posixAccount_password'], true, $passwordHash);
								$return['del']['userPassword'][0] = $_POST['posixAccount_passwordOld'];
							}
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
		// stop processing if only a password change is done
		if ($passwordChangeOnly) {
			return $return;
		}
		// sync from Windows password
		if (in_array('syncWindowsPassword', $fields) && !empty($_POST['windowsUser_unicodePwd'])) {
			$password = $_POST['windowsUser_unicodePwd'];
			$return['mod']['unixUserPassword'][0] = pwd_hash($password, true, $this->selfServiceSettings->moduleSettings['posixAccount_pwdHash'][0]);
			if (isset($attributes['shadowLastChange'][0])) {
				$return['mod']['shadowLastChange'][0] = intval(time()/3600/24);
			}
		}
		// cn
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
		// shell
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
	 * {@inheritDoc}
	 * @see baseModule::postModifySelfService()
	 */
	public function postModifySelfService($newAccount, $attributes) {
		if (isset($attributes['INFO.userPasswordModify'][0])
			&& ($attributes['INFO.userPasswordModify'][0] === 'exop')) {
			$password = $attributes['INFO.userPasswordClearText'][0];
			$dn = $attributes['dn'][0];
			$success = ldap_exop_passwd($_SESSION['ldapHandle'], $dn, null, $password);
			if (!$success) {
				StatusMessage('ERROR', _('Unable to set password'), getExtendedLDAPErrorMessage($_SESSION['ldapHandle']));
			}
			else {
				StatusMessage('INFO', _('Password changed.'));
			}
			return $success;
		}
		return true;
	}

	/**
	 * Returns if the module manages the password attribute.
	 *
	 * @param string $typeId account type id
	 * @return boolean manages password
	 */
	private function isPasswordManaged($typeId = null) {
		if ($typeId === null) {
			$typeId = $this->getAccountContainer()->get_type()->getId();
		}
		return !$this->isBooleanConfigOptionSet('posixAccount_' . $typeId . '_hidepassword');
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		return $this->isPasswordManaged();
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
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$accountModules = $this->getAccountContainer()->get_type()->getModules();
		// check password strength
		$user = empty($this->attributes['uid'][0]) ? null : $this->attributes['uid'][0];
		$additionalAttrs = array();
		if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
			$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if (!empty($attrs['sn'][0])) {
				$additionalAttrs[] = $attrs['sn'][0];
			}
			if (!empty($attrs['givenName'][0])) {
				$additionalAttrs[] = $attrs['givenName'][0];
			}
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
			$this->attributes[$this->getPasswordAttrName($accountModules)][0] = '{SASL}' . $this->attributes['uid'][0];
		}
		// delay on ldap_exop
		elseif (!empty($this->moduleSettings['posixAccount_pwdHash'][0]) && ($this->moduleSettings['posixAccount_pwdHash'][0] === 'LDAP_EXOP')) {
			logNewMessage(LOG_DEBUG, 'Setting password in post action, exop');
		}
		// set normal password
		else {
			$this->attributes[$this->getPasswordAttrName($accountModules)][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		}
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
	 * Finds all existing LDAP groups.
	 *
	 * @return array groups array(array(gidnumber, cn), array(gidnumber, cn), ...)
	 */
	private function findGroups(&$modules) {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$this->groupCache = array();
		$typeManager = new TypeManager();
		foreach ($typeManager->getConfiguredTypesForScope('group') as $type) {
			$filter = '(objectClass=posixGroup)';
			if ($this->isWindows($modules)) {
				$filter = '(&(objectClass=group)(gidNumber=*))';
			}
			$typeFilter = $type->getAdditionalLdapFilter();
			if (!empty($typeFilter)) {
				if (strpos($typeFilter, '(') !== 0) {
					$typeFilter = '(' . $typeFilter . ')';
				}
				$filter = '(&' . $filter . $typeFilter . ')';
			}
			$results = searchLDAP($type->getSuffix(), $filter, array('cn', 'gidnumber'));
			for ($i = 0; $i < sizeof($results); $i++) {
				if (isset($results[$i]['cn'][0]) && isset($results[$i]['gidnumber'][0])) {
					$this->groupCache[] = array($results[$i]['gidnumber'][0], $results[$i]['cn'][0]);
				}
			}
		}
		return $this->groupCache;
	}

	/**
	 * Finds all existing LDAP group of names.
	 *
	 * @return array groups array(dn => array('cn' => array('groupName'), 'objectclass' => array('top', 'groupOfNames')))
	 */
	public function findGroupOfNames() {
		if ($this->gonCache != null) {
			return $this->gonCache;
		}
		$return = array();
		$typeManager = new TypeManager();
		$types = $typeManager->getConfiguredTypesForScopes(array('gon', 'group'));
		foreach ($types as $type) {
			$typeFilter = get_ldap_filter($type->getId());
			$results = searchLDAP($type->getSuffix(), $typeFilter, array('cn', 'dn', 'objectClass'));
			for ($i = 0; $i < sizeof($results); $i++) {
				if ((in_array_ignore_case('groupOfNames', $results[$i]['objectclass'])
						|| in_array_ignore_case('groupOfMembers', $results[$i]['objectclass'])
						|| in_array_ignore_case('groupOfUniqueNames', $results[$i]['objectclass']))
						&& isset($results[$i]['cn'][0])) {
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
	 * @param string $typeId type id (e.g. user)
	 * @return array list of UID numbers
	 */
	private function getUIDs($typeId) {
		if ($this->cachedUIDList != null) {
			return $this->cachedUIDList;
		}
		$this->cachedUIDList = array();
		$attrs = array('uidNumber');
		$filter = '(&(objectClass=posixAccount)(uidNumber=*))';
		if ($this->skipObjectClass()) {
			$filter = '(uidNumber=*)';
		}
		$typeManager = new TypeManager();
		$typesUser = $typeManager->getConfiguredTypesForScope('user');
		$typesHost = $typeManager->getConfiguredTypesForScope('host');
		$suffixes = array();
		if (!empty($typesUser)) {
			if (!empty($this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixUser'][0])) {
				$suffixes[] = $this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixUser'][0];
			}
			else {
				foreach ($typesUser as $type) {
					$suffixes[] = $type->getSuffix();
				}
			}
		}
		if (!empty($typesHost)) {
			if (!empty($this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixHost'][0])) {
				$suffixes[] = $this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixHost'][0];
			}
			else {
				foreach ($typesHost as $type) {
					$suffixes[] = $type->getSuffix();
				}
			}
		}
		$suffixes = array_unique($suffixes);
		foreach ($suffixes as $suffix) {
			$result = searchLDAP($suffix, $filter, $attrs);
			foreach ($result as $resultEntry) {
				$this->cachedUIDList[] = $resultEntry['uidnumber'][0];
			}
		}
		$this->cachedUIDList = array_values(array_unique($this->cachedUIDList));
		sort($this->cachedUIDList, SORT_NUMERIC);
		return $this->cachedUIDList;
	}

	/**
	 * Checks if the given user name already exists in LDAP.
	 *
	 * @param String $userName user name
	 * @param string $typeId type id (e.g. user)
	 * @return boolean true if already exists
	 */
	private function userNameExists($userName, $typeId) {
		return array_key_exists($userName, $this->getUserNames($typeId));
	}

	/**
	 * Returns a list of all user names in LDAP.
	 *
	 * @param string $typeId type id (e.g. user)
	 * @return array user names
	 */
	private function getUserNames($typeId) {
		if ($this->cachedUserNameList != null) {
			return $this->cachedUserNameList;
		}
		$this->cachedUserNameList = array();
		$attrs = array('uid');
		$filter = '(&(objectClass=posixAccount)(uid=*))';
		if ($this->skipObjectClass()) {
			$filter = '(uid=*)';
		}
		$typeManager = new TypeManager();
		$typesUser = $typeManager->getConfiguredTypesForScope('user');
		$typesHost = $typeManager->getConfiguredTypesForScope('host');
		$suffixes = array();
		if (!empty($typesUser)) {
			if (!empty($this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixUser'][0])) {
				$suffixes[] = $this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixUser'][0];
			}
			else {
				foreach ($typesUser as $type) {
					$suffixes[] = $type->getSuffix();
				}
			}
		}
		if (!empty($typesHost)) {
			if (!empty($this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixHost'][0])) {
				$suffixes[] = $this->moduleSettings['posixAccount_' . $typeId . '_uidCheckSuffixHost'][0];
			}
			else {
				foreach ($typesHost as $type) {
					$suffixes[] = $type->getSuffix();
				}
			}
		}
		$suffixes = array_unique($suffixes);
		foreach ($suffixes as $suffix) {
			$result = searchLDAP($suffix, $filter, $attrs);
			foreach ($result as $resultEntry) {
				$this->cachedUserNameList[$resultEntry['uid'][0]] = $resultEntry['dn'];
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
		$typeManager = new TypeManager();
		$types = $typeManager->getConfiguredTypesForScopes(array('group', 'gon'));
		foreach ($types as $type) {
			$modules = $type->getModules();
			if (in_array('groupOfNames', $modules)
					|| in_array('groupOfMembers', $modules)
					|| in_array('groupOfUniqueNames', $modules)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns a suggestion for the user name.
	 * By default this will be the first character of the first name plus the last name.
	 *
	 * @param array $attrs LDAP attributes
	 * @param string $typeId type id (e.g. user)
	 * @return String user name
	 */
	protected function getUserNameSuggestion($attrs, $typeId) {
		$attributes = array_change_key_case($attrs, CASE_LOWER);
		$format = '@givenname@%sn%';
		if (isset($this->moduleSettings['posixAccount_' . $typeId . '_userNameSuggestion'][0])) {
			$format = strtolower($this->moduleSettings['posixAccount_' . $typeId . '_userNameSuggestion'][0]);
		}
		// search for @key@ wildcards in format string and replace with first character of attribute
		$wildcards = array();
		if (preg_match_all('/@([^@]|[a-zA-Z_-])+@/', $format, $wildcards) > 0) {
			for ($i = 0; $i < sizeof($wildcards[0]); $i++) {
				$wc = substr($wildcards[0][$i], 1, strlen($wildcards[0][$i]) - 2);
				$value = '';
				if (isset($attributes[$wc][0]) && !empty($attributes[$wc][0])) {
					$value = $this->cleanSuggestionPart($attributes[$wc][0][0]);
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
					$value = $this->cleanSuggestionPart($attributes[$wc][0]);
				}
				$format = str_replace('%' . $wc . '%', $value, $format);
			}
		}
		return $format;
	}

	/**
	 * Cleans a string that is injected in user name suggestion.
	 *
	 * @param string $part injected part
	 * @return string cleaned by removing umlauts, spaces, dashes and underscores
	 */
	private function cleanSuggestionPart($part) {
		$result = str_replace(array_keys($this->umlautReplacements), array_values($this->umlautReplacements), strtolower($part));
		return str_replace(array(' ', '_', '-'), array('', '', ''), $result);
	}

	/**
	 * Returns if this account can be locked.
	 * This is the case if a hashed password is set ("{" at the beginning).
	 *
	 * @param string[] $modules account modules
	 * @return boolean lockable
	 */
	public function isLockable(&$modules) {
		if (isset($this->attributes[$this->getPasswordAttrName($modules)][0])
				&& pwd_is_lockable($this->attributes[$this->getPasswordAttrName($modules)][0])) {
			return true;
		}
		return false;
	}

	/**
	 * Returns if the Unix part of the current account is locked.
	 *
	 * @param string[] $modules account modules
	 * @return boolean password is locked
	 */
	public function isLocked(&$modules) {
		return isset($this->attributes[$this->getPasswordAttrName($modules)][0])
			&& !pwd_is_enabled($this->attributes[$this->getPasswordAttrName($modules)][0]);
	}

	/**
	 * Locks the user password of this account.
	 *
	 * @param string[] $modules account modules
	 */
	public function lock(&$modules) {
		$pwdAttrName = $this->getPasswordAttrName($modules);
		if (isset($this->attributes[$pwdAttrName][0])) {
			$this->attributes[$pwdAttrName][0] = pwd_disable($this->attributes[$pwdAttrName][0]);
		}
	}

	/**
	 * Unlocks the user password of this account.
	 *
	 * @param string[] $modules account modules
	 */
	public function unlock(&$modules) {
		$pwdAttrName = $this->getPasswordAttrName($modules);
		if (isset($this->attributes[$pwdAttrName][0])) {
			$this->attributes[$pwdAttrName][0] = pwd_enable($this->attributes[$pwdAttrName][0]);
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
	 * @param string[] $moduleNames list of account module names
	 * @return String new user name
	 */
	protected function getNextUserName($userName, $moduleNames) {
		if ($this->get_scope() == 'host' && in_array('sambaSamAccount', $moduleNames)) {
			$userName = substr($userName, 0, -1);
		}
		// get last character of username
		$lastchar = substr($userName, strlen($userName) - 1, 1);
		$suffix = '';
		if (($this->get_scope() == 'host') && in_array('sambaSamAccount', $moduleNames)) {
			$suffix = '$';
		}
		// Last character is no number
		if ( !preg_match('/^([0-9])+$/', $lastchar)) {
			// Last character is no number. Therefore we only have to add "2" to it.
			$userName = $userName . '2' . $suffix;
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
			$userName = $firstchars . (intval($lastchars) + 1) . $suffix;
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
		if (!isLoggedIn() && isset($this->selfServiceSettings) && isset($this->selfServiceSettings->moduleSettings['posixAccount_shells'])
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

	/**
	 * Returns if the cn attribute should be managed.
	 * If Windows modules are active then cn will not be managed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean manage cn attribute
	 */
	private function manageCn(&$modules) {
		return !$this->isWindows($modules);
	}

	/**
	 * Returns if the Unix part can be added and removed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean is optional
	 */
	private function isOptional(&$modules) {
		return !$this->manageCn($modules);
	}

	/**
	 * Returns if the Windows module is active.
	 *
	 * @param string[] $modules account modules
	 * @return boolean is Windows
	 */
	private function isWindows(&$modules) {
		return in_array('windowsUser', $modules);
	}

	/**
	 * Returns the password attribute.
	 * Usually, this is userPassword. If Windows modules are active this is unixUserPassword.
	 *
	 * @param string[] $modules account modules
	 * @return boolean attribute name
	 */
	private function getPasswordAttrName(&$modules) {
		if ($this->isWindows($modules)) {
			return 'unixUserPassword';
		}
		return 'userPassword';
	}

	/**
	 * Returns the home directory attribute.
	 * Usually, this is homeDirectory. If Windows modules are active this is unixHomeDirectory.
	 *
	 * @param string[] $modules account modules
	 * @return boolean attribute name
	 */
	private function getHomedirAttrName(&$modules) {
		if ($this->isWindows($modules)) {
			return 'unixHomeDirectory';
		}
		return 'homeDirectory';
	}

	/**
	 * Syncs the group of names with groups.
	 */
	private function syncGonToGroups() {
		$this->groups = array();
		$allGons = $this->findGroupOfNames();
		foreach ($this->gonList as $dn) {
			if (!isset($allGons[$dn])) {
				continue;
			}
			$gon = $this->gonCache[$dn];
			if (in_array_ignore_case('posixGroup', $gon['objectclass']) && !empty($gon['cn'])) {
				$this->groups[] = $gon['cn'][0];
			}
		}
	}

	/**
	 * Returns if the object class should not be added.
	 *
	 * @return do not add
	 */
	private function skipObjectClass() {
		return $this->isBooleanConfigOptionSet('posixAccount_noObjectClass');
	}

	/**
	 * {@inheritdoc}
	 */
	public function getWildCardReplacements() {
		$replacements = array();
		// user name
		if (!empty($_POST['uid'])) {
			$replacements['user'] = $_POST['uid'];
		}
		elseif (!empty($this->attributes['uid'][0])) {
			$replacements['user'] = $this->attributes['uid'][0];
		}
		// group name
		if (!empty($_POST['gidNumber'])) {
			$replacements['group'] = $this->getGroupName($_POST['gidNumber']);
		}
		elseif (!empty($this->attributes['gidNumber'][0])) {
			$replacements['group'] = $this->getGroupName($this->attributes['gidNumber'][0]);
		}
		return $replacements;
	}

	/**
	 * Returns the current group names.
	 *
	 * @return string[] group names
	 */
	public function getGroups() {
		return $this->groups;
	}

	/**
	 * Returns the list of group of names where this user is member.
	 *
	 * @return string[] list of DNs
	 */
	public function getGroupOfNames() {
		return $this->gonList;
	}

}

?>
