<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2011  Pavel Pozdniak
                2010 - 2020  Roland Gruber

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
 * Manages mailboxes on an IMAP server.
 *
 * @package modules
 * @author Pavel Pozdniak
 * @author Roland Gruber
 */

/**
 * Manages mailboxes on an IMAP server.
 *
 * @package modules
 * @author Pavel Pozdniak
 * @author Roland Gruber
 */
class imapAccess extends baseModule {

	/** quota limit from profile */
	private $profileQuotaLimit = null;

	/** user name */
	private $user;
	/** email address */
	private $email;

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
	function get_metaData() {
		$return = array();
		// alias name
		$return["alias"] = _("Mailbox");
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('inetOrgPerson', 'windowsUser')), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array();
		// PHP extensions
		$return['extensions'] = array('imap');
		// icon
		$return['icon'] = 'mailBig.png';
		// help
		$return['help'] = array(
			'ImapServerAddress' => array(
				"Headline" => _("Server address"),
				"Text" => _("Address of IMAP server (e.g. mail.example.org).")),
			'ImapServerEncryptionProtocol' => array(
				"Headline" => _("Encryption protocol"),
				"Text" => _("Encryption protocol for connecting to IMAP server. LAM requires an encrypted connection.")),
			'ImapValidateServerCert' => array(
				"Headline" => _("Validate server certificate"),
				"Text" => _("This option allows you to disable the certificate check of your IMAP server certificate. Disabling the certificate check is not recommended.")),
			'ImapAdmin' => array(
				"Headline" => _("IMAP admin user"),
				"Text" => _("The login name of your IMAP user who has rights to create/delete mailboxes.") . ' ' . _('Use wildcards like $uid$ for LDAP attributes of the current LAM admin user.')),
			'ImapAdminPasswordSelect' => array(
				"Headline" => _("IMAP password input"),
				"Text" => _("Choose the way how to provide the IMAP admin password. You can use the same password as for the LAM login or LAM will ask you for a password when it is required.")
							. ' ' . _('Storing the password in your server profile is also possible but not recommended.')
			),
			'ImapAdminPassword_Sess' => array(
				"Headline" => _("Password of IMAP admin user"),
				"Text" => _("The password of your IMAP admin user. The login name for the IMAP admin user is stored in the LAM server profile.")),
			'ImapUserPrefix' => array(
				"Headline" => _("Prefix for mailboxes"),
				"Text" => _("Some IMAP servers store mailboxes with a prefix (e.g. \"user\" for Cyrus which results in \"user.username\").")),
			'ImapMailDomain' => array(
				"Headline" => _("Mail domains"),
				"Text" => _("Please enter a comma separated list of domain names (e.g. \"company.com,example.com\"). LAM will only manage mailboxes from these domains.")),
			'ImapUserNameAttr' => array(
				"Headline" => _("User name attribute"),
				"Text" => _("Please choose the attribute to get the IMAP user name. The default is \"mail\" but you can also use \"uid\" or \"userPrincipalName\".")),
			'MailAddress' => array(
				"Headline" => _("Mailbox"),
				"Text" => _("This mailbox will be created/deleted.")),
			'ImapUserQuotaLimit' => array(
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota limit of this mailbox in kilobytes.")),
			'pathSeparator' => array(
				"Headline" => _("Path separator"),
				"Text" => _("This is the separator for the mailbox path. Usually, this is \".\" but e.g. Cyrus with \"unixhierarchysep\" will require \"/\".")),
			'initialFolders' => array(
				"Headline" => _("Initial folders"),
				"Text" => _("Use this to provide a list of folders (e.g. Trash) to add for new accounts.")),
			'createMailbox' => array(
				"Headline" => _("Create mailbox"),
				"Text" => _('Set to "true" to create the mailbox.')),
		);
		// configuration checks
		$return['config_checks']['all']['ImapAccess_ImapServerAddress'] = array (
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'required' => true,
			'required_message' => $this->messages['config'][0],
			'error_message' => $this->messages['config'][0]);
		$return['config_checks']['all']['ImapAccess_ImapDomain'] = array (
			'type' => 'regex_i',
			'regex' => '[\\*a-z0-9\\._-]+(,[a-z0-9\\._-]+)*',
			'required' => true,
			'required_message' => $this->messages['config'][1],
			'error_message' => $this->messages['config'][1]);
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Quota'), 'ImapAccess_QuotaLimit', null, 'ImapUserQuotaLimit'), 12);
		$return['profile_options'] = $profileContainer;
		$return['profile_checks']['ImapAccess_QuotaLimit'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['managemailbox'][8]);
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['config'][0] = array('ERROR', _('Please enter a valid server name where the mailboxes reside.'));
		$this->messages['config'][1] = array('ERROR', _('Please enter a correct list of valid mail domains.'));
		$this->messages['config'][2] = array('ERROR', _('The IMAP admin password is empty.'));
		$this->messages['managemailbox'][0] = array('ERROR', _('Unable to change ACL on IMAP server for mailbox deletion.'));
		$this->messages['managemailbox'][1] = array('ERROR', _('Unable to delete mailbox from IMAP server.'));
		$this->messages['managemailbox'][2] = array('ERROR', _('Unable to create mailbox on IMAP server.'));
		$this->messages['managemailbox'][3] = array('ERROR', _('Unable to locate mailbox on IMAP.'));
		$this->messages['managemailbox'][4] = array('ERROR', _('Your IMAP domains and email address domain do not match.'));
		$this->messages['managemailbox'][5] = array('ERROR', _('Invalid password for IMAP admin or other problem occured.'));
		$this->messages['managemailbox'][6] = array('WARN', _('Your LAM login password was not accepted by the IMAP server.'));
		$this->messages['managemailbox'][7] = array('ERROR', _('Cannot update quota.'));
		$this->messages['managemailbox'][8] = array('ERROR', _('Wrong quota format. Quota must be numeric.'));
		$this->messages['managemailbox'][9] = array('ERROR', _('Account %s:') . ' imapAccess_quota', _('Wrong quota format. Quota must be numeric.'));
		$this->messages['managemailbox'][10] = array('ERROR', _('Cannot read quota.'));
		$this->messages['createMailbox'][0] = array('ERROR', _('Account %s:') . ' imapAccess_createMailbox', _('This value can only be "true" or "false".'));
	}

	/**
	 * Extracts user name and email address from inetOrgPerson/posixAccount/windowsUser modules.
	 *
	 * @param array $attrs LDAP attributes (retrieved from other account modules if empty)
	 * @return htmlStatusMessage message if any
	 */
	private function extractUserAndEmail($attrs = null) {
		$this->email = '';
		if ($attrs === null) {
			if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			}
			else {
				$attrs = $this->getAccountContainer()->getAccountModule('windowsUser')->getAttributes();
			}
			if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
				$attrs = array_merge($attrs, $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes());
			}
		}
		$this->email = !empty($attrs['mail'][0]) ? $attrs['mail'][0] : '';
		$this->user = '';
		// extract user name from email address
		if (empty($this->moduleSettings['ImapAccess_UserNameAttribute'][0]) || $this->moduleSettings['ImapAccess_UserNameAttribute'][0] == 'mail') {
			$email_parts = explode('@', $this->email, 2);
			$this->user = array_shift($email_parts);
		}
		elseif ($this->moduleSettings['ImapAccess_UserNameAttribute'][0] == 'userPrincipalName') {
			if (!empty($attrs['userPrincipalName'][0])) {
				$parts = explode('@', $attrs['userPrincipalName'][0], 2);
				$this->user = array_shift($parts);
			}
		}
		// extract user name from Unix user name (might be in inetOrgPerson/windowUser or posixAccount module)
		else {
			$this->user = !empty($attrs['uid'][0]) ? $attrs['uid'][0] : '';
		}

		if (empty($this->email)) {
			$modName = ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) ? 'inetOrgPerson' : 'windowsUser';
			return new htmlStatusMessage('INFO', _("Please enter an email address on this page: %s"), '', array($this->getAccountContainer()->getAccountModule($modName)->get_alias()));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function display_html_attributes() {
		$return = new htmlResponsiveRow();
		if (!checkIfWriteAccessIsAllowed($this->get_scope())) {
			return $return;
		}
		$msg = $this->extractUserAndEmail();
		if ($msg != null) {
			$return->add($msg, 12);
			return $return;
		}
		$prefix = $this->getMailboxPrefix();

		$email_domain = substr(strstr($this->email, '@'), 1);
		$adminPassword = $this->getAdminPassword(); // check for password for fall back mechanism
		if (!isset($_SESSION['imapAdmPass']) && !isset($adminPassword)) {
			return $this->display_html_password();
		}

		$return->addLabel(new htmlOutputText(_('Email address')));
		$return->addField(new htmlOutputText($this->email));

		$adminUser = $this->getAdminUser();
		$adminPassword = $this->getAdminPassword();
		try {
			$client = $this->connect($adminUser, $adminPassword);
		}
		catch (LAMException $e) {
			return $this->display_html_password(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
		}

		$return->addLabel(new htmlOutputText(_('Mailbox')));
		$mailboxGroup = new htmlGroup();
		$mailboxGroup->addElement(new htmlOutputText($prefix . $this->getSep() . $this->user));
		$mailboxGroup->addElement(new htmlHelpLink('MailAddress'));
		$return->addField($mailboxGroup);
		$return->addVerticalSpacer('2rem');

		$list = $client->listMailboxes($prefix . $this->getSep() . $this->user, Horde_Imap_Client::MBOX_ALL);
		if (is_array($list) && sizeof($list) == 1) {
			$this->renderQuotasForMailbox($return, $client, $prefix . $this->getSep() . $this->user);
			$return->addVerticalSpacer('2rem');
			$return->add(new htmlButton('deleteMailbox', _('Delete mailbox')), 12, 12, 12, 'text-center');
		}
		else {
			$mailboxMessage = new htmlOutputText(_("Mailbox does not exist on IMAP server."));
			$return->add($mailboxMessage, 12, 12, 12, 'text-center');
			$return->addVerticalSpacer('2rem');
			if ($this->isWrongDomain($email_domain)) {
				$return->add(new htmlStatusMessage('INFO', $this->messages['managemailbox'][4][1]), 12);
				$return->addVerticalSpacer('1rem');
			}
			else {
				$createButton = new htmlButton('createMailbox', _('Create mailbox'));
				$return->add($createButton, 12, 12, 12, 'text-center');
			}
		}
		$client->logout();
		return $return;
	}

	/**
	 * Returns the HTML meta data for the password page.
	 *
	 * @param htmlStatusMessage|null $message status message
	 * @return htmlResponsiveRow HTML meta data
	 */
	function display_html_password($message = null) {
		$return = new htmlResponsiveRow();
		if ($message !== null) {
			$return->add($message, 12);
			$return->addVerticalSpacer('1rem');
		}
		$passwordInput = new htmlResponsiveInputField(_("Password of IMAP admin user"), 'ImapAdminPassword', '', 'ImapAdminPassword_Sess');
		$passwordInput->setIsPassword(true);
		$passwordInput->setRequired(true);
		$passwordInput->setOnKeyPress('SubmitForm(\'enterPasswordButton\', event);');
		$return->add($passwordInput, 12);
		$return->addVerticalSpacer('2rem');
		$return->add(new htmlButton('enterPasswordButton', _('Ok')), 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * Display the mailbox quota.
	 *
	 * @param htmlResponsiveRow $container structure that contained information to be displayed
	 * @param Horde_Imap_Client_Socket $client IMAP client
	 * @param String $username user name to connect to IMAP server
	 * @return htmlResponsiveRow table with added information about user quotas or controls to add quota
	 */
	function renderQuotasForMailbox($container, $client, $username) {
		if (($this->profileQuotaLimit != null) && ($this->profileQuotaLimit != '')) {
			$client->setQuota($username, array('storage' => $this->profileQuotaLimit));
			$this->profileQuotaLimit = null;
		}
		try {
			$quotaRoot = $client->getQuotaRoot($username);
			$quota_values = array();
			if (!empty($quotaRoot)) {
				$quota_values = $client->getQuota($username);
			}
			if (!empty($quota_values)) {
				if (isset($quota_values['storage']) && is_array($quota_values['storage'])) {
					$quotaLimit = $quota_values['storage']['limit'];
					$container->addLabel(new htmlOutputText(_("Current usage (kB)")));
					$container->addField(new htmlOutputText($quota_values['storage']['usage']), true);
					$quotaLimitInput = new htmlResponsiveInputField(_("Quota limit (kB)"), 'ImapUserQuotaLimit', $quotaLimit, 'ImapUserQuotaLimit');
					$container->add($quotaLimitInput, 12);
					$container->addVerticalSpacer('2rem');
					$container->add(new htmlButton('updateQuota', _('Update quota')), 12, 12, 12, 'text-center');
					$container->addVerticalSpacer('1rem');
				}
			}
			else {
				$quotaLimit = "";
				$quotaLimitInput = new htmlResponsiveInputField(_("Quota limit (kB)"), 'ImapUserQuotaLimit', $quotaLimit, 'ImapUserQuotaLimit');
				$container->add($quotaLimitInput, 12);
				$container->addVerticalSpacer('2rem');
				$container->add(new htmlButton('updateQuota', _('Update quota')), 12, 12, 12, 'text-center');
				$container->addVerticalSpacer('1rem');
			}
		}
		catch (Exception $e) {
			$container->add(new htmlStatusMessage('ERROR', $this->messages['managemailbox'][10][1], $e->getMessage()), 12);
		}
		return $container;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_attributes() {
		$errors = array();
		if (!checkIfWriteAccessIsAllowed($this->get_scope())) {
			return $errors;
		}
		$prefix = $this->getMailboxPrefix();

		$adminUser = $this->getAdminUser();
		if (isset($_POST['ImapAdminPassword']) && isset($_POST['enterPasswordButton'])) {
			$errors = $this->doLogin();
		}
		$adminPassword = $this->getAdminPassword();

		try {
			$client = $this->connect($adminUser, $adminPassword);
			$this->extractUserAndEmail();
			$email_domain = substr(strstr($this->email, '@'), 1);

			if (isset($_POST['deleteMailbox'])) {
				if ($this->isWrongDomain($email_domain)) {
					$errors[] = $this->messages['managemailbox'][4];
				}
				else {
					$root = $prefix . $this->getSep() . $this->user;
					try {
						$client->setACL($root, $adminUser, array('rights' => 'c', 'action' => 'add'));
						$client->deleteMailbox($root);
					}
					catch (Exception $e) {
						$message = $this->messages['managemailbox'][1];
						$message[] = $e->getMessage();
						$errors[] = $message;
					}
				}
			}

			if (isset($_POST['createMailbox'])) {
				$createMessages = $this->createMailbox($client, $this->user, $email_domain);
				$errors = array_merge($errors, $createMessages);
			}
			if (isset($_POST['updateQuota'])) {
				$quota = $_POST['ImapUserQuotaLimit'];
				$quotaMessages = $this->setQuota($client, $this->user, $email_domain, $quota);
				$errors = array_merge($errors, $quotaMessages);
			}
			$client->logout();
		}
		catch (LAMException $e) {
			return array(array('ERROR', $e->getTitle(), $e->getMessage()));
		}
		// Return error-messages
		return $errors;
	}

	/**
	 * Creates the mailbox for a user.
	 *
	 * @param Horde_Imap_Client_Socket $client IMAP client
	 * @param string $userName user name
	 * @param string $email_domain email domain
	 * @return array error messages
	 */
	private function createMailbox($client, $userName, $email_domain) {
		$errors = array();
		$prefix = $this->getMailboxPrefix();
		if ($this->isWrongDomain($email_domain)) {
			$errors[] = $this->messages['managemailbox'][4];
		}
		else {
			$root = $prefix . $this->getSep() . $userName;
			logNewMessage(LOG_DEBUG, 'Creating mailbox: ' . $root);
			try {
				$client->createMailbox($root);
				logNewMessage(LOG_DEBUG, 'Mailbox created');
				$list = $client->listMailboxes($root, Horde_Imap_Client::MBOX_ALL);
				if (empty($list)) {
					$errors[] = $this->messages['managemailbox'][3];
					return $errors;
				}
				// create initial folders
				foreach ($this->getInitialFolders() as $folder) {
					$fullFolderName = $root . $this->getSep() . $folder;
					logNewMessage(LOG_DEBUG, 'Creating folder: ' . $fullFolderName);
					$client->createMailbox($fullFolderName);
					logNewMessage(LOG_DEBUG, 'Folder created: ' . $fullFolderName);
				}
			}
			catch (Exception $e) {
				$message = $this->messages['managemailbox'][2];
				$message[] = $e->getMessage();
				$errors[] = $message;
			}
		}
		return $errors;
	}

	/**
	 * Sets the mailbox quota for a user.
	 *
	 * @param Horde_Imap_Client_Socket $client IMAP client
	 * @param string $userName user name
	 * @param string $email_domain email domain
	 * @param string $quota mailbox quota
	 * @return array error messages
	 */
	private function setQuota($client, $userName, $email_domain, $quota) {
		$prefix = $this->getMailboxPrefix();
		$errors = array();
		$root = $prefix . $this->getSep() . $userName;
		if ($this->isWrongDomain($email_domain)) {
			$errors[] = $this->messages['managemailbox'][4];
		}
		else {
			if ($quota == '') {
				try {
					$client->setQuota($root, array('storage' => '-1'));
				}
				catch (Exception $e) {
					$message = $this->messages['managemailbox'][7];
					$message[] = $e->getMessage();
					$errors[] = $message;
				}
			}
			elseif (get_preg($quota, 'digit')) {
				logNewMessage(LOG_DEBUG, 'Setting quota ' . $quota . ' for ' . $root);
				try {
					$client->setQuota($root, array('storage' => $quota));
				}
				catch (Exception $e) {
					$message = $this->messages['managemailbox'][7];
					$message[] = $e->getMessage();
					$errors[] = $message;
				}
			}
			else {
				$errors[] = $this->messages['managemailbox'][8];
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
		if (isset($profile['ImapAccess_QuotaLimit'][0]) && $profile['ImapAccess_QuotaLimit'][0] != '') {
			$this->profileQuotaLimit =  $profile['ImapAccess_QuotaLimit'][0];
		}
	}

	/**
	 * Returns a list of configuration options.
	 *
	 * @param array $scopes account types (user, group, host)
	 * @param array $allScopes list of all active account modules and their scopes (module => array(scopes))
	 * @return mixed htmlElement or array of htmlElement
	 *
	 * @see htmlElement
	 */
	public function get_configOptions($scopes, $allScopes) {
		// configuration settings
		$configContainer = new htmlResponsiveRow();
		$configServer = new htmlResponsiveInputField(_('Server address'), 'ImapAccess_ImapServerAddress', '', 'ImapServerAddress');
		$configServer->setRequired(true);
		$configContainer->add($configServer, 12);
		$configContainer->add(new htmlResponsiveSelect('ImapAccess_ImapServerEncriptionProtocol', array('TLS', 'SSL'), array('TLS'), _("Encryption protocol"), 'ImapServerEncryptionProtocol'), 12);
		$configCertValidate = new htmlResponsiveSelect('ImapAccess_ImapValidateServerCert', array(_('Yes') => 'validate-cert', _('No') => 'novalidate-cert'), array('validate-cert'), _("Validate server certificate"), 'ImapValidateServerCert');
		$configCertValidate->setHasDescriptiveElements(true);
		$configContainer->add($configCertValidate, 12);
		$configUser = new htmlResponsiveInputField(_('IMAP admin user'), 'ImapAccess_ImapAdmin', '', 'ImapAdmin');
		$configUser->setRequired(true);
		$configContainer->add($configUser, 12);
		$pwdSelectOptions = array(
			_('LAM user password') => 'lam_user_pass',
			_('Ask') => 'ask_pass',
			_('Server profile') => 'config');
		$configPasswordType = new htmlResponsiveSelect('ImapAccess_ImapAdminPasswordSelect', $pwdSelectOptions, array('ask_pass'), _("IMAP password input"), 'ImapAdminPasswordSelect');
		$configPasswordType->setHasDescriptiveElements(true);
		$configPasswordType->setTableRowsToShow(array('config' => array('ImapAccess_ImapAdminPassword')));
		$configPasswordType->setTableRowsToHide(array('lam_user_pass' => array('ImapAccess_ImapAdminPassword'), 'ask_pass' => array('ImapAccess_ImapAdminPassword')));
		$configContainer->add($configPasswordType, 12);
		$adminPwdInput = new htmlResponsiveInputField(_('Admin password'), 'ImapAccess_ImapAdminPassword', null, 'ImapAdminPasswordSelect');
		$adminPwdInput->setIsPassword(true);
		$adminPwdInput->setObfuscate(true);
		$configContainer->add($adminPwdInput, 12);
		$mailDomainsInput = new htmlResponsiveInputField(_('Mail domains'), 'ImapAccess_ImapDomain', '', 'ImapMailDomain');
		$mailDomainsInput->setRequired(true);
		$configContainer->add($mailDomainsInput, 12);
		$configContainer->add(new htmlResponsiveInputField(_('Prefix for mailboxes'), 'ImapAccess_ImapUserPrefix', '', 'ImapUserPrefix'), 12);
		$configContainer->add(new htmlResponsiveInputTextarea('ImapAccess_initialFolders', '', 10, 3, _('Initial folders'), 'initialFolders'), 12);
		$configUserName = new htmlResponsiveSelect('ImapAccess_UserNameAttribute', array('mail', 'uid', 'userPrincipalName'), array('mail'), _("User name attribute"), 'ImapUserNameAttr');
		$configContainer->add($configUserName, 12);
		$configPathSeparator = new htmlResponsiveSelect('ImapAccess_pathSeparator', array('.', '/'), array('.'), _("Path separator"), 'pathSeparator');
		$configContainer->add($configPathSeparator, 12);
		return $configContainer;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::check_configOptions()
	 */
	public function check_configOptions($typeIds, &$options) {
		$errors = parent::check_configOptions($typeIds, $options);
		if (($options['ImapAccess_ImapAdminPasswordSelect'][0] == 'config')
				&& empty($options['ImapAccess_ImapAdminPassword'][0])) {
			$errors[] = $this->messages['config'][2];
		}
		return $errors;
	}

	/**
	 * Returns the user name of the IMAP admin.
	 *
	 * @return String admin user name
	 */
	private function getAdminUser() {
		if (isset($_SESSION['imapAdmUser'])) {
			return $_SESSION['imapAdmUser'];
		}
		$user = $this->moduleSettings['ImapAccess_ImapAdmin'][0];
		// check if user name contains any wildcards that need to be replaced with LDAP attribute values
		$matches = array();
		preg_match_all('/\\$[a-z0-9_-]+\\$/i', $this->moduleSettings['ImapAccess_ImapAdmin'][0], $matches);
		if (sizeof($matches) > 0) {
			// find wildcards
			$attrNames = array();
			foreach ($matches as $match) {
				foreach ($match as $attr) {
					$attrNames[] = substr($attr, 1, -1);
				}
			}
			$attrNames = array_values(array_unique($attrNames));
			$attrNames = array_change_key_case($attrNames, CASE_LOWER);
			// read LAM login user data
			$dn = $_SESSION['ldap']->getUserName();
			$sr = @ldap_read($_SESSION['ldap']->server(), $dn, '(objectclass=*)', $attrNames, 0, 0, 0, LDAP_DEREF_NEVER);
			if ($sr) {
				$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
				if ($info) {
					cleanLDAPResult($info);
					$info = $info[0];
				}
			}
			// replace wildcards
			foreach ($attrNames as $attr) {
				if (empty($info[$attr])) {
					continue;
				}
				$user = preg_replace('/\\$' . $attr . '\\$/i', $info[$attr][0], $user);
			}
		}
		logNewMessage(LOG_DEBUG, 'IMAP admin user: ' . $user);
		$_SESSION['imapAdmUser'] = $user;
		return $user;
	}

	/**
	 * Returns the admin password.
	 *
	 * @return String password
	 */
	private function getAdminPassword() {
		//perform admin password
		$password = null; //default value is null, it can be changed during the work
		if (isset($_SESSION['imapAdmPass'])) {
			$password = lamDecrypt($_SESSION['imapAdmPass']);
		}
		elseif (isset($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0]) && ($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0] == "lam_user_pass")) {
			$password = $_SESSION['ldap']->getPassword();
		}
		elseif (!empty($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0]) && ($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0] == "config")
			&& !empty($this->moduleSettings['ImapAccess_ImapAdminPassword'][0])) {
			$password = deobfuscateText($this->moduleSettings['ImapAccess_ImapAdminPassword'][0]);
		}
		return $password;
	}

	/**
     * Checks the password given by user and save it as session parameter.
     *
     * @return array list of error messages
     */
	function doLogin() {
		$errors = array();
		$adminUser = $this->getAdminUser();
		if (isset($_POST['ImapAdminPassword']) && $_POST['ImapAdminPassword'] != "") {
			$adminPassword = $_POST['ImapAdminPassword'];
			try {
				$client = $this->connect($adminUser, $adminPassword);
				$_SESSION['imapAdmPass'] = lamEncrypt($_POST['ImapAdminPassword']);
				$client->logout();
			}
			catch (LAMException $e) {
				$error = $this->messages['managemailbox'][5];
				$error[] = $e->getMessage();
				$errors[] = $error;
			}
		}
		return $errors;
	}

	/**
	 * Connects to the IMAP server.
	 *
	 * @param string $user user name
	 * @param string $password password
	 * @return Horde_Imap_Client_Socket IMAP client
	 */
	private function connect($user, $password) {
		include_once __DIR__ . '/../3rdParty/composer/autoload.php';
		$encryptionType = $this->moduleSettings['ImapAccess_ImapServerEncriptionProtocol'][0];
		if (strrpos($this->moduleSettings['ImapAccess_ImapServerAddress'][0], ":")) {
			$port = substr(strstr($this->moduleSettings['ImapAccess_ImapServerAddress'][0], ':'), 1);
			$serverName = array_shift(explode(':', $this->moduleSettings['ImapAccess_ImapServerAddress'][0], 2));
		}
		else {
			$serverName = $this->moduleSettings['ImapAccess_ImapServerAddress'][0];
			if ($encryptionType === "TLS") {
				$port = 143;
			}
			else {
				$port = 993;
			}
		}
		$context = array(
			'ssl' => array(
				'cafile' => __DIR__ . '/../../serverCerts.pem'
			)
		);
		if (isset($this->moduleSettings['ImapAccess_ImapValidateServerCert'][0]) && ($this->moduleSettings['ImapAccess_ImapValidateServerCert'][0] == 'novalidate-cert')) {
			$context['ssl']['verify_peer'] = false;
			$context['ssl']['verify_peer_name'] = false;
		}
		try {
			$client = new Horde_Imap_Client_Socket(array(
				'username' => $user,
				'password' => $password,
				'hostspec' => $serverName,
				'port' => $port,
				'secure' => strtolower($encryptionType),
				'context' => $context
			));
			$client->login();
			return $client;
		}
		catch (Exception $e) {
			throw new LAMException(_('Unable to connect to IMAP server.'), $e->getMessage(), $e);
		}
	}

	/**
	 * This function returns the prefix for mailboxes.
	 * If no prefix was given during configuration then "user" will be used (default for Cyrus).
	 *
	 * @return String prefix
	 */
	function getMailboxPrefix() {
		if (!isset($this->moduleSettings['ImapAccess_ImapUserPrefix'][0]) || ($this->moduleSettings['ImapAccess_ImapUserPrefix'][0] == '')) {
			return "user";
		}
		else {
			return $this->moduleSettings['ImapAccess_ImapUserPrefix'][0];
		}
	}

	/**
	 * This function checks if the domain of the mailbox is not in the list of domains listed in the configuration.
	 * If it is in the list then it returns false, otherwise returns true. If the list of domains is not set then it returns true.
	 *
	 * @param String $email_domain email domain
	 * @return boolean true if domains match
	 */
	function isWrongDomain($email_domain) {
		if (isset($this->moduleSettings['ImapAccess_ImapDomain'][0])) {
			$domain_list_string = $this->moduleSettings['ImapAccess_ImapDomain'][0];
			if ($domain_list_string == '*') {
				return false;
			}
			$domains_array = explode(",", $domain_list_string);
			if ((sizeof($domains_array) == 0) || in_array($email_domain, $domains_array)) {
				return false;
			}
		}
		else {
			return false;
		}
		return true;
	}

	/**
	 * Returns the path separator.
	 *
	 * @return String separator char
	 */
	private function getSep() {
		if (isset($this->moduleSettings['ImapAccess_pathSeparator'][0])) {
			return $this->moduleSettings['ImapAccess_pathSeparator'][0];
		}
		return '.'; // default
	}

	/**
	 * Returns the list of initial folders to create for a new mailbox.
	 *
	 * @return array list of folders
	 */
	private function getInitialFolders() {
		$list = array();
		if (!empty($this->moduleSettings['ImapAccess_initialFolders'])) {
			foreach ($this->moduleSettings['ImapAccess_initialFolders'] as $folder) {
				$folder = trim($folder);
				if (!empty($folder)) {
					$list[] = $folder;
				}
			}
		}
		return $list;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_uploadColumns()
	 */
	public function get_uploadColumns($selectedModules, &$type) {
		$pwd = $this->getAdminPassword();
		if (empty($pwd)) {
			return array();
		}
		return array(
			array(
				'name' => 'imapAccess_createMailbox',
				'description' => _('Create mailbox'),
				'example' => 'false',
				'default' => 'false',
				'values' => 'true, false',
				'help' => 'createMailbox'
			),
			array(
				'name' => 'imapAccess_quota',
				'description' => _('Quota limit (kB)'),
				'example' => '1000000',
				'help' => 'ImapUserQuotaLimit'
			),
		);
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = array();
		if (!isset($ids['imapAccess_createMailbox'])) {
			return $errors;
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (isset($rawAccounts[$i][$ids['imapAccess_createMailbox']])
					&& !in_array($rawAccounts[$i][$ids['imapAccess_createMailbox']], array('true', 'false'))) {
				$errMsg = $this->messages['createMailbox'][0];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
			}
			if (isset($rawAccounts[$i][$ids['imapAccess_createMailbox']])
					&& ($rawAccounts[$i][$ids['imapAccess_createMailbox']] === 'true')
					&& !empty($ids['imapAccess_quota'])
					&& isset($rawAccounts[$i][$ids['imapAccess_quota']])
					&& !get_preg($rawAccounts[$i][$ids['imapAccess_quota']], 'digit')) {
				$errMsg = $this->messages['managemailbox'][9];
				array_push($errMsg, array($i));
				$errors[] = $errMsg;
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
		// on first call generate list of IMAP operations
		if (!isset($temp['counter'])) {
			$temp['users'] = array();
			$temp['counter'] = 0;
			$errors = array();
			if (isset($ids['imapAccess_createMailbox'])) {
				foreach ($data as $i => $dataRow) {
					if (in_array($i, $failed)) {
						continue; // ignore failed accounts
					}
					if ($dataRow[$ids['imapAccess_createMailbox']] === 'true') {
						$limit = '';
						if (isset($ids['imapAccess_quota'])
								&& isset($dataRow[$ids['imapAccess_quota']])
								&& ($dataRow[$ids['imapAccess_quota']] !== '')) {
							$limit = $dataRow[$ids['imapAccess_quota']];
						}
						$attributes = $accounts[$i];
						foreach ($attributes as $name => $value) {
							if (!is_array($value)) {
								$attributes[$name] = array($value);
							}
						}
						$extractErrors = $this->extractUserAndEmail($attributes);
						if (!empty($extractErrors)) {
							$errors = array_merge($errors, $extractErrors);
						}
						$temp['users'][] = array(
							'uid' => $this->user,
							'limit' => $limit,
							'email' => substr(strstr($this->email, '@'), 1)
						);
					}
				}
			}
			return array(
				'status' => 'inProgress',
				'progress' => 0,
				'errors' => $errors
			);
		}
		// add mailbox
		elseif ($temp['counter'] < sizeof($temp['users'])) {
			$errors = array();
			$data = $temp['users'][$temp['counter']];
			$uid = $data['uid'];
			$limit = $data['limit'];
			$email_domain = $data['email'];
			ob_start();
			$imapUser = $this->getAdminUser();
			$imapPassword = $this->getAdminPassword();
			try {
				$client = $this->connect($imapUser, $imapPassword);
				$prefix = $this->getMailboxPrefix();
				$list = $client->listMailboxes($prefix . $this->getSep() . $uid);
				if (empty($list)) {
					$createErrors = $this->createMailbox($client, $uid, $email_domain);
					$errors = array_merge($errors, $createErrors);
					if (empty($createErrors)) {
						$quotaErrors = $this->setQuota($client, $uid, $email_domain, $limit);
						$errors = array_merge($errors, $quotaErrors);
					}
				}
				$client->logout();
			}
			catch (Exception $e) {
				$message = $this->messages['managemailbox'][5];
				$message[] = $e->getMessage();
				$errors[] = $message;
			}
			ob_end_clean();
			$temp['counter']++;
			return array (
				'status' => 'inProgress',
				'progress' => ($temp['counter'] * 100) / sizeof($temp['users']),
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

}
