<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2011  Pavel Pozdniak
                2010 - 2011  Roland Gruber

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
	
	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 * 
	 * @see baseModule::get_metaData()
	 */
	function get_metaData() {
		$return = array();
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Mailbox");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
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
				"Text" => _("The login name of your IMAP user who has rights to create/delete mailboxes.")),
			'ImapAdminPasswordSelect' => array(
				"Headline" => _("IMAP password input"),
				"Text" => _("Choose the way how to provide the IMAP admin password. You can use the same password as for the LAM login or LAM will ask you for a password when it is required.")),
			'ImapAdminPassword_Sess' => array(
				"Headline" => _("Password of IMAP admin user"),
				"Text" => _("The password of your IMAP admin user. The login name for the IMAP admin user is stored in the LAM server profile.")),
			'ImapUserPrefix' => array(
				"Headline" => _("Prefix for mailboxes"),
				"Text" => _("Some IMAP servers store mailboxes with a prefix (e.g. \"user\" for Cyrus which results in \"user.username\").")),
			'ImapMailDomain' => array(
				"Headline" => _("Mail domain(s)"),
				"Text" => _("Please enter a comma separated list of domain names (e.g. \"company.com,example.com\"). LAM will only manage mailboxes from these domains.")),
			'ImapUserNameAttr' => array(
				"Headline" => _("User name attribute"),
				"Text" => _("Please choose the attribute to get the IMAP user name. The default is mail but you can also use uid.")),
			'MailAddress' => array(
				"Headline" => _("Mailbox"),
				"Text" => _("This mailbox will be created/deleted.")),
			'ImapUserQuotaLimit' => array(
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota limit of this mailbox in kilobytes.")),
		);
		// configuration settings
		$configContainer = new htmlTable();
		$configServer = new htmlTableExtendedInputField(_('Server address'), 'ImapAccess_ImapServerAddress', '', 'ImapServerAddress');
		$configServer->setRequired(true);
		$configContainer->addElement($configServer, true);
		$configContainer->addElement(new htmlTableExtendedSelect('ImapAccess_ImapServerEncriptionProtocol', array('TLS', 'SSL'), array('TLS'), _("Encryption protocol"), 'ImapServerEncryptionProtocol'), true);
		$configCertValidate = new htmlTableExtendedSelect('ImapAccess_ImapValidateServerCert', array(_('Yes') => 'validate-cert', _('No') => 'novalidate-cert'), array('validate-cert'), _("Validate server certificate"), 'ImapValidateServerCert');
		$configCertValidate->setHasDescriptiveElements(true);
		$configContainer->addElement($configCertValidate, true);
		$configUser = new htmlTableExtendedInputField(_('IMAP admin user'), 'ImapAccess_ImapAdmin', '', 'ImapAdmin');
		$configUser->setRequired(true);
		$configContainer->addElement($configUser, true);
		$configPasswordType = new htmlTableExtendedSelect('ImapAccess_ImapAdminPasswordSelect', array(_('LAM user password') => 'lam_user_pass', _('Ask') => 'ask_pass'), array('ask_pass'), _("IMAP password input"), 'ImapAdminPasswordSelect');
		$configPasswordType->setHasDescriptiveElements(true);
		$configContainer->addElement($configPasswordType, true);
		$configContainer->addElement(new htmlTableExtendedInputField(_('Prefix for mailboxes'), 'ImapAccess_ImapUserPrefix', '', 'ImapUserPrefix'), true);
		$mailDomainsInput = new htmlTableExtendedInputField(_('Mail domain(s)'), 'ImapAccess_ImapDomain', '', 'ImapMailDomain');
		$mailDomainsInput->setRequired(true);
		$configContainer->addElement($mailDomainsInput, true);
		$configUserName = new htmlTableExtendedSelect('ImapAccess_UserNameAttribute', array('mail', 'uid'), array('mail'), _("User name attribute"), 'ImapUserNameAttr');
		$configContainer->addElement($configUserName, true);
		$return['config_options']['all'] = $configContainer;
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
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['config'][0] = array('ERROR', _('Please enter a valid server name where the mailboxes reside.'));
		$this->messages['config'][1] = array('ERROR', _('Please enter a correct list of valid mail domains.'));
		$this->messages['managemailbox'][0] = array('ERROR', _('Unable to change ACL on IMAP server for mailbox deletion.'));
		$this->messages['managemailbox'][1] = array('ERROR', _('Unable to delete mailbox from IMAP server.'));
		$this->messages['managemailbox'][2] = array('ERROR', _('Unable to create mailbox on IMAP server.'));
		$this->messages['managemailbox'][3] = array('ERROR', _('Unable to locate mailbox on IMAP.'));
		$this->messages['managemailbox'][4] = array('ERROR', _('Your IMAP domain(s) and email address domain do not match.'));
		$this->messages['managemailbox'][5] = array('ERROR', _('Invalid password for IMAP admin or other problem occured.'));
		$this->messages['managemailbox'][6] = array('WARN', _('Your LAM login password was not accepted by the IMAP server.'));
		$this->messages['managemailbox'][7] = array('ERROR', _('Cannot update quota.'));
		$this->messages['managemailbox'][8] = array('ERROR', _('Wrong quota format. Quota must be numeric.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$prefix = $this->getMailboxPrefix();
		
		$email = '';
		$attrsPersonal = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
		$email = $attrsPersonal['mail'][0];
		
		if ($email == '') {
			$return->addElement(new htmlStatusMessage('INFO', _("Please enter an email address on this page: %s"), '', array($this->getAccountContainer()->getAccountModule('inetOrgPerson')->get_alias())));
			return $return;
		}
		$email_domain = substr(strstr($email, '@'), 1);
		// extract user name from email address
		if (!isset($this->moduleSettings['ImapAccess_UserNameAttribute'][0]) || $this->moduleSettings['ImapAccess_UserNameAttribute'][0] == 'mail') {
			$email_parts = explode('@', $email, 2);
			$email_username = array_shift($email_parts);
		}
		// extract user name from Unix user name (might be in inetOrgPerson or posixAccount module)
		else {
			if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
				$attrsUnix = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				$email_username = $attrsUnix['uid'][0];
			}
			else {
				$email_username = $attrsPersonal['uid'][0];
			}
		}
		$imap_admin_password = $this->getAdminPassword(); // check for password for fall back mechanism
		if (!isset($_SESSION['imapAdmPass']) && !isset($imap_admin_password)) {
			return $this->display_html_password();
		}
		
		$return->addElement(new htmlOutputText(_('Email address')));
		$return->addElement(new htmlOutputText($email), true);

		$imap_server_address = $this->getServerAddress();
		$imap_admin_user = $this->moduleSettings['ImapAccess_ImapAdmin'][0];
		$imap_admin_password = $this->getAdminPassword();
		$mbox = @imap_open("{" . $imap_server_address . "}", $imap_admin_user, $imap_admin_password, OP_HALFOPEN, 1);
		if (!$mbox) {
			return $this->display_html_password();
		}
		
		$return->addElement(new htmlOutputText(_('Mailbox')));
		$return->addElement(new htmlOutputText($prefix . "." . $email_username));
		$return->addElement(new htmlHelpLink('MailAddress'), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		
		$is_mailbox_exist = false; //default is false
		$list = imap_list($mbox, "{" . $imap_server_address . "}", $prefix . "." . $email_username);
		if (is_array($list) && sizeof($list) == 1) {
			$this->renderQuotasForMailbox($return, $mbox, $prefix . "." . $email_username);
			$mailboxMessage = new htmlOutputText(_("Mailbox already exists on IMAP server."));
			$mailboxMessage->colspan = 3;
			$return->addElement($mailboxMessage, true);
			$return->addElement(new htmlSpacer(null, '10px'), true);
			$return->addElement(new htmlButton('deleteMailbox', _('Delete mailbox')));
		}
		else {
			$mailboxMessage = new htmlOutputText(_("Mailbox does not exist on IMAP server."));
			$mailboxMessage->colspan = 3;
			$return->addElement($mailboxMessage, true);
			$return->addElement(new htmlSpacer(null, '10px'), true);
			$createButton = new htmlButton('createMailbox', _('Create mailbox'));
			$message = '';
			if ($this->isWrongDomain($email_domain)) {
				$createButton->setIsEnabled(false);
				$message = '<< ' . $this->messages['managemailbox'][4][1];
			}
			$return->addElement($createButton);
			$return->addElement(new htmlOutputText($message));
		}
		imap_close($mbox);
		return $return;
	}

	/**
	 * Returns the HTML meta data for the password page.
	 *
	 * @return array HTML meta data
	 */
	function display_html_password() {
		$return = new htmlTable();
		if($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0] == "lam_user_pass"){
			$message =  $this->messages['managemailbox'][6];
			$messageElement = new htmlStatusMessage($message[0], $message[1]);
			$messageElement->colspan = 3;
			$return->addElement($messageElement);
			$return->addElement(new htmlSpacer(null, '10px'), true);
		}
		$passwordInput = new htmlTableExtendedInputField(_("Password of IMAP admin user"), 'ImapAdminPassword', '', 'ImapAdminPassword_Sess');
		$passwordInput->setIsPassword(true);
		$passwordInput->setRequired(true);
		$return->addElement($passwordInput, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$return->addElement(new htmlButton('enterPasswordButton', _('Ok')));
		return $return;
	}

	/**
	 * Display the mailbox quota.
	 * 
	 * @param htmlTable $htmlTable structure that contained information to be displayed
	 * @param stream $mbox stream to open IMAP session
	 * @return htmlTable table with added information about user quotas or controls to add quota
	 */
	function renderQuotasForMailbox($htmlTable, $mbox, $username) {
		$quota_values = @imap_get_quota($mbox, $username);
		imap_errors();
		if (is_array($quota_values) && (sizeof($quota_values) > 0)) {
			if (isset($quota_values['STORAGE']) && is_array($quota_values['STORAGE'])) {
				$htmlTable->addElement(new htmlOutputText(_("Current usage (kB)")));
				$htmlTable->addElement(new htmlOutputText($quota_values['STORAGE']['usage']), true);
				$quotaLimitInput = new htmlTableExtendedInputField(_("Quota limit (kB)"), 'ImapUserQuotaLimit', $quota_values['STORAGE']['limit'], 'ImapUserQuotaLimit');
				$htmlTable->addElement($quotaLimitInput, false);
				$htmlTable->addElement(new htmlSpacer('10px', null), false);
				$htmlTable->addElement(new htmlButton('updateQuota', _('Update quota')), true);
				$htmlTable->addElement(new htmlSpacer(null, '10px'), true);
			}
		}
		else {
			$quotaLimit = "";
			$quotaLimitInput = new htmlTableExtendedInputField(_("Quota limit (kB)"), 'ImapUserQuotaLimit', $quotaLimit, 'ImapUserQuotaLimit');
			$htmlTable->addElement($quotaLimitInput, false);
			$htmlTable->addElement(new htmlSpacer('10px', null), false);
			$htmlTable->addElement(new htmlButton('updateQuota', _('Update quota')), true);
			$htmlTable->addElement(new htmlSpacer(null, '10px'), true);
		}
		return $htmlTable;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_attributes() {
		$errors = array();
		$prefix = $this->getMailboxPrefix();
		
		$imap_server_address = $this->getServerAddress();
		$imap_admin_user = $this->moduleSettings['ImapAccess_ImapAdmin'][0];
		
		if (isset($_POST['ImapAdminPassword'])) {
			$errors = $this->doLogin();
		}
		
		$imap_admin_password = $this->getAdminPassword();
		if ($imap_admin_password) {
			$mbox = @imap_open("{" . $imap_server_address . "}", $imap_admin_user, $imap_admin_password, OP_HALFOPEN, 1);
		}
		if ($mbox) {
			$attrsPersonal = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			$email = $attrsPersonal['mail'][0];
			$email_domain = substr(strstr($email, '@'), 1);
			// extract user name from email address
			if (!isset($this->moduleSettings['ImapAccess_UserNameAttribute'][0]) || $this->moduleSettings['ImapAccess_UserNameAttribute'][0] == 'mail') {
				$email_parts = explode('@', $email, 2);
				$email_username = array_shift($email_parts);
			}
			// extract user name from Unix user name (might be in inetOrgPerson or posixAccount module)
			else {
				if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
					$attrsUnix = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
					$email_username = $attrsUnix['uid'][0];
				}
				else {
					$email_username = $attrsPersonal['uid'][0];
				}
			}
			
			if (isset($_POST['deleteMailbox'])) {
				if ($this->isWrongDomain($email_domain)) {
					$errors[] = $this->messages['managemailbox'][4];
				}
				else {
					if (!imap_setacl($mbox, $prefix . "." . $email_username, $this->moduleSettings['ImapAccess_ImapAdmin'][0], "c")) {
						$errors[] = $this->messages['managemailbox'][0];
					}
					
					$delete_mailbox_arg = "{" . $imap_server_address . "}" . $prefix . "." . $email_username;
					if (!@imap_deletemailbox($mbox, $delete_mailbox_arg)) {
						$errors[] = $this->messages['managemailbox'][1];
					}
				}
			}
			
			if (isset($_POST['createMailbox'])) {
				if ($this->isWrongDomain($email_domain)) {
					$errors[] = $this->messages['managemailbox'][4];
				}
				else {
					$create_mailbox_arg = "{" . $imap_server_address . "}" . $prefix . "." . $email_username;
					if (imap_createmailbox($mbox, imap_utf7_encode($create_mailbox_arg))) {
						$list = imap_list($mbox, "{" . $imap_server_address . "}", $prefix . "." . $email_username);
						if (is_array($list) && sizeof($list) == 1) {
						
						}
						else {
							$errors[] = $this->messages['managemailbox'][3];
						}
					}
					else {
						$errors[] = $this->messages['managemailbox'][2];
					}
				}
			}
			if (isset($_POST['updateQuota'])) {
				if ($this->isWrongDomain($email_domain)) {
					$errors[] = $this->messages['managemailbox'][4];
				}
				else {
					if (!isset($_POST['ImapUserQuotaLimit']) || ($_POST['ImapUserQuotaLimit'] == '')) {
/*						if (!imap_set_quota($mbox, $prefix . "." . $email_username, -1)) {
							$message = $this->messages['managemailbox'][7];
							$message[] = imap_last_error();
							$errors[] = $message;
						}*/
					}
					elseif (isset($_POST['ImapUserQuotaLimit']) && ($_POST['ImapUserQuotaLimit'] != '') && get_preg($_POST['ImapUserQuotaLimit'], 'digit')){
						if (!imap_set_quota($mbox, $prefix . "." . $email_username, $_POST['ImapUserQuotaLimit'])) {
							$message = $this->messages['managemailbox'][7];
							$message[] = imap_last_error();
							$errors[] = $message;
						}
					}
					else {
						$errors[] = $this->messages['managemailbox'][8];
					}
				}
			}
			imap_close($mbox);
		}
		// Return error-messages
		return $errors;
	}

	/**
	 * Returns the admin password.
	 *
	 * @return String password
	 */
	function getAdminPassword() {
		//perform admin password
		$imap_admin_password = null; //default value is null, it can be changed during the work
		if (isset($_SESSION['imapAdmPass'])) {
			$imap_admin_password = $_SESSION['ldap']->decrypt($_SESSION['imapAdmPass']);
		}
		elseif (isset($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0]) && ($this->moduleSettings['ImapAccess_ImapAdminPasswordSelect'][0] == "lam_user_pass")) {
			$credentials = $_SESSION['ldap']->decrypt_login();
			$imap_admin_password = $credentials[1];
		}
		return $imap_admin_password;
	}

	/**
     * Checks the password given by user and save it as session parameter.
     * 
     * @return array list of error messages
     */
	function doLogin() {
		$errors = array();
		$imap_server_address = $this->getServerAddress();
		$imap_admin_user = $this->moduleSettings['ImapAccess_ImapAdmin'][0];
		if (isset($_POST['ImapAdminPassword']) && $_POST['ImapAdminPassword'] != "") {
			$imap_admin_password = $_POST['ImapAdminPassword'];
			$mbox = @imap_open("{" . $imap_server_address . "}", $imap_admin_user, $imap_admin_password, OP_HALFOPEN, 1);
			if ($mbox) {
				$_SESSION['imapAdmPass'] = $_SESSION['ldap']->encrypt($_POST['ImapAdminPassword']);
				@imap_close($mbox);
			}
			else {
				$error = $this->messages['managemailbox'][5];
				$error[] = imap_last_error();
				$errors[] = $error;
			}
		}
		return $errors;
	}

	/**
	 * This function returns the IMAP server address including encryption options.
	 * 
	 * @return String server address
	 */
	function getServerAddress() {
		$imap_encryption_protocol = $this->moduleSettings['ImapAccess_ImapServerEncriptionProtocol'][0];
		if (strrpos($this->moduleSettings['ImapAccess_ImapServerAddress'][0], ":")) {
			$imap_port_number = substr(strstr($this->moduleSettings['ImapAccess_ImapServerAddress'][0], ':'), 1);
			$imap_server_name = array_shift(explode(':', $this->moduleSettings['ImapAccess_ImapServerAddress'][0], 2));
		}
		else {
			$imap_server_name = $this->moduleSettings['ImapAccess_ImapServerAddress'][0];
			if (strcmp($imap_encryption_protocol, "TLS") == 0) {
				$imap_port_number = 143;
			}
			else {
				$imap_port_number = 993;
			}
		}
		if (isset($this->moduleSettings['ImapAccess_ImapValidateServerCert'][0]) && ($this->moduleSettings['ImapAccess_ImapValidateServerCert'][0] == 'novalidate-cert')) {
			$validate_opt = "novalidate-cert";
		}
		else {
			$validate_opt = "validate-cert";
		}
		$imap_server_address = $imap_server_name . ":" . $imap_port_number . "/" . $imap_encryption_protocol . "/" . $validate_opt;
		return $imap_server_address;
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

}

?>
