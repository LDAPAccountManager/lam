<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014  Roland Gruber

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
* Provides NIS mail alias management.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides NIS mail alias management.
*
* @package modules
*/
class nisMailAliasUser extends baseModule {

	/** alias cache */
	private $cachedAliasList = null;
	
	/** recipient entries to delete */
	private $receipientsToDelete = array();
	/** complete alias entries to delete */
	private $aliasesToDelete = array();
	
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
		$return['icon'] = 'mailBig.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Mail aliases");
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('inetOrgPerson', 'posixAccount')), 'conflicts' => array());
		// help Entries
		$return['help'] = array(
			'aliasUser' => array(
				"Headline" => _("Alias names with user name"),
				"Text" => _('Sets the alias names linked to the current user name.')
			),
			'aliasUserList' => array(
				"Headline" => _("Alias names with user name"),
				"Text" => _('Sets the alias names linked to the current user name.') . ' ' . _("Multiple values are separated by semicolon.")
			),
			'aliasMail' => array(
				"Headline" => _("Alias names with email address"),
				"Text" => _('Sets the alias names linked to the user\'s email address.')
			),
			'aliasUserList' => array(
				"Headline" => _("Alias names with email address"),
				"Text" => _('Sets the alias names linked to the user\'s email address.') . ' ' . _("Multiple values are separated by semicolon.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'nisMailAliasUser_usernameAliases',
				'description' => _('Aliases for user name'),
				'help' => 'aliasUserList',
			),
			array(
				'name' => 'nisMailAliasUser_mailAliases',
				'description' => _('Aliases for email'),
				'help' => 'aliasMailList',
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'alias' => _('Alias names'),
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' nisMailAliasUser_usernameAliases', _('Unable to create alias %s.'));
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' nisMailAliasUser_mailAliases', _('Unable to create alias %s.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$aliases = $this->getMailAliasList();
		$count = sizeof($aliases);
		$userName = $this->getUserName();
		$mails = $this->getMailAddresses();
		if (!$this->isBooleanConfigOptionSet('nisMailAliasUser_hideUserAliases') && ($userName != null)) {
			$return->addElement(new htmlSubTitle(_('Aliases for user name')), true);
			$userTable = new htmlTable();
			for ($i = 0; $i < $count; $i++) {
				if (empty($aliases[$i]['rfc822mailmember'])) {
					continue;
				}
				$dn = $aliases[$i]['dn'];
				$members = $aliases[$i]['rfc822mailmember'];
				if (in_array($userName, $members)
						&& (!isset($this->receipientsToDelete[$dn]) || !in_array($userName, $this->receipientsToDelete[$dn]))
						&& !in_array($dn, $this->aliasesToDelete)) {
					$userTable->addElement(new htmlOutputText($aliases[$i]['cn'][0]));
					$buttonGroup = new htmlGroup();
					$remButton = new htmlButton('rem_' . $i, 'del.png', true);
					$remButton->setTitle(_('Remove user from alias entry.'));
					$buttonGroup->addElement($remButton);
					$delButton = new htmlButton('del_' . $i, 'trash.png', true);
					$delButton->setTitle(sprintf(_('Delete whole alias entry which includes %s recipients.'), sizeof($members)));
					$buttonGroup->addElement($delButton);
					$userTable->addElement($buttonGroup, true);
				}
			}
			$return->addElement($userTable, true);
		}
		if (!$this->isBooleanConfigOptionSet('nisMailAliasUser_hideUserAliases') && !empty($mails)) {
			$return->addElement(new htmlSubTitle(_('Aliases for email')), true);
			$mailTable = new htmlTable();
			for ($m = 0; $m < sizeof($mails); $m++) {
				if (sizeof($mails) > 1) {
					$label = new htmlOutputText($mails[$m]);
					$label->colspan = 5;
					$mailTable->addElement($label, true);
				}
				$found = false;
				for ($i = 0; $i < $count; $i++) {
					if (empty($aliases[$i]['rfc822mailmember'])) {
						continue;
					}
					$dn = $aliases[$i]['dn'];
					$members = $aliases[$i]['rfc822mailmember'];
					if (in_array($mails[$m], $members)
							&& (!isset($this->receipientsToDelete[$dn]) || !in_array($mails[$m], $this->receipientsToDelete[$dn]))
							&& !in_array($dn, $this->aliasesToDelete)) {
						$found = true;
						$mailTable->addSpace('5px');
						$mailTable->addElement(new htmlOutputText($aliases[$i]['cn'][0]));
						$buttonGroup = new htmlGroup();
						$remButton = new htmlButton('remMail_' . $i . '_' . $m, 'del.png', true);
						$remButton->setTitle(_('Remove user from alias entry.'));
						$buttonGroup->addElement($remButton);
						$delButton = new htmlButton('delMail_' . $i . '_' . $m, 'trash.png', true);
						$delButton->setTitle(sprintf(_('Delete whole alias entry which includes %s recipients.'), sizeof($members)));
						$buttonGroup->addElement($delButton);
						$mailTable->addElement($buttonGroup, true);
					}
				}
				if ((sizeof($mails) > 1) && ($m < (sizeof($mails) - 1))) {
					$mailTable->addVerticalSpace('20px');
				}
			}
			$return->addElement($mailTable, true);
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
		$mails = $this->getMailAddresses();
		foreach ($_POST as $key => $value) {
			if (strpos($key, 'rem_') === 0) {
				$index = substr($key, strlen('rem_'));
				$this->receipientsToDelete[$this->cachedAliasList[$index]['dn']][] = $this->getUserName();
			}
			elseif (strpos($key, 'del_') === 0) {
				$index = substr($key, strlen('del_'));
				$this->aliasesToDelete[] = $this->cachedAliasList[$index]['dn'];
			}
			elseif (strpos($key, 'remMail_') === 0) {
				$parts = substr($key, strlen('remMail_'));
				$parts = explode('_', $parts);
				$this->receipientsToDelete[$this->cachedAliasList[$parts[0]]['dn']][] = $mails[$parts[1]];
			}
			elseif (strpos($key, 'delMail_') === 0) {
				$parts = substr($key, strlen('remMail_'));
				$parts = explode('_', $parts);
				$this->aliasesToDelete[] = $this->cachedAliasList[$parts[0]]['dn'];
			}
		}
		return $errors;
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 *
	 * @param boolean $newAccount new account
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
		$errors = array();
		$ldapUser = $_SESSION['ldap']->decrypt_login();
		$ldapUser = $ldapUser[0];
		// delete complete aliases
		foreach ($this->aliasesToDelete as $dn) {
			$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$errors[] = array('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
			else {
				logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed ' . $dn);
			}
		}
		// delete recipient entries
		foreach ($this->receipientsToDelete as $dn => $recipients) {
			$success = @ldap_mod_del($_SESSION['ldap']->server(), $dn, array('rfc822mailmember' => $recipients));
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to remove recipients ' . implode(', ', $recipients) . ' from  ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$errors[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
			else {
				logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed recipients ' . implode(', ', $recipients) . ' from ' . $dn);
			}
		}
		return $errors;
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
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("nisMailAlias", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "nisMailAlias";
			// add alias name
			// check format
			if (get_preg($rawAccounts[$i][$ids['nisMailAlias_alias']], 'nis_alias')) {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['nisMailAlias_alias']];
			}
			else {
				$errMsg = $this->messages['alias'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// add recipients
			if ($rawAccounts[$i][$ids['nisMailAlias_recipients']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['nisMailAlias_recipients']]);
				// check format
				for ($a = 0; $a < sizeof($aliases); $a++) {
					if (get_preg($aliases[$a], 'nis_recipient') || get_preg($aliases[$a], 'email')) {
						$partialAccounts[$i]['rfc822MailMember'][] = $aliases[$a];
					}
					else {
						$errMsg = $this->messages['recipient'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$aliases = $this->getMailAliasList();
		$foundAliases = array();
		$mails = $this->getMailAddresses();
		$user = $this->getUserName();
		foreach ($aliases as $alias) {
			if (empty($alias['rfc822mailmember'][0])) {
				continue;
			}
			if (!empty($user) && in_array($user, $alias['rfc822mailmember'])) {
				$foundAliases[] = $alias['cn'][0];
			}
			if (!empty($mails)) {
				foreach ($mails as $mail) {
					if (in_array($mail, $alias['rfc822mailmember'])) {
						$foundAliases[] = $alias['cn'][0];
					}
				}
			}
		}
		$foundAliases = array_unique($foundAliases);
		$return[get_class($this) . '_alias'] = array('<block><key>' . _('Alias names') . '</key><value>' . implode(', ', $foundAliases) . '</value></block>');
		return $return;
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
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainer->addElement($configContainerHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('nisMailAliasUser_hideUserAliases', false, _('Aliases for user name'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('nisMailAliasUser_hideMailAliases', false, _('Aliases for email'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		return $configContainer;
	}

	/**
	 * Returns a list of existing email aliases.
	 *
	 * @return array email aliases
	 */
	private function getMailAliasList() {
		if ($this->cachedAliasList != null) {
			return $this->cachedAliasList;
		}
		$this->cachedAliasList = searchLDAPByAttribute('cn', '*', 'nisMailAlias', array('dn', 'cn', 'rfc822MailMember'), array('mailAlias'));
		return $this->cachedAliasList;
	}
	
	/**
	 * Returns the user name of this account.
	 * 
	 * @return String user name
	 */
	private function getUserName() {
		if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (!empty($attrs['uid'][0])) {
				return $attrs['uid'][0];
			}
		}
		elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
			$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if (!empty($attrs['uid'][0])) {
				return $attrs['uid'][0];
			}
		}
		return null;
	}
	
	/**
	 * Returns the email addresses of this account.
	 * 
	 * @return String mail addresses
	 */
	private function getMailAddresses() {
		if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
			$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if (!empty($attrs['mail'])) {
				return $attrs['mail'];
			}
		}
		return null;
	}

}


?>
