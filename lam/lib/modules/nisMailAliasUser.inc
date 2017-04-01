<?php
use \LAM\TYPES\TypeManager;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014 - 2017  Roland Gruber

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

	/** recipient entries to delete (list of arrays: dn => attributes) */
	private $recipientsToDelete = array();
	/** complete alias entries to delete */
	private $aliasesToDelete = array();
	/** new alias entries (list of arrays: dn => attributes) */
	private $aliasesToAdd = array();
	/** alias entries to extend with new recipients (list of arrays: dn => recipients) */
	private $recipientsToAdd = array();

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
		// icon
		$return['icon'] = 'mailBig.png';
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
			'suffix' => array(
				"Headline" => _("Suffix"),
				"Text" => _("Location where new alias is stored.")
			),
			'newAlias' => array(
				"Headline" => _("Alias name"), 'attr' => 'cn',
				"Text" => _("Name of new alias entry.")
			),
			'aliasNames' => array(
				"Headline" => _("Alias names"), 'attr' => 'cn',
				"Text" => _("Select one or more alias entries from the list to add the recipient.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
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
		$this->messages['alias'][0] = array('ERROR', _('Alias name is invalid.'));
		$this->messages['alias'][1] = array('ERROR', _('Alias name already exists.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		if (!$this->isMailAliasTypeActive()) {
			$return->addElement(new htmlStatusMessage('ERROR', _('Please activate the mail aliases type for this server profile.')));
			return $return;
		}
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
						&& (!isset($this->recipientsToDelete[$dn]) || !in_array($userName, $this->recipientsToDelete[$dn]))
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
			$userTable->addVerticalSpace('5px');
			$addButton = new htmlAccountPageButton(get_class($this), 'add', 'user', _('Add'));
			$addButton->setIconClass('createButton');
			$addButton->colspan = 5;
			$userTable->addElement($addButton, true);
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
							&& (!isset($this->recipientsToDelete[$dn]) || !in_array($mails[$m], $this->recipientsToDelete[$dn]))
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
				$mailTable->addVerticalSpace('5px');
				$addButton = new htmlAccountPageButton(get_class($this), 'add', 'mail' . $m, _('Add'));
				$addButton->setIconClass('createButton');
				$addButton->colspan = 5;
				$mailTable->addElement($addButton, true);
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
		if (!$this->isMailAliasTypeActive()) {
			return $errors;
		}
		$mails = $this->getMailAddresses();
		foreach ($_POST as $key => $value) {
			if (strpos($key, 'rem_') === 0) {
				$index = substr($key, strlen('rem_'));
				$dn = $this->cachedAliasList[$index]['dn'];
				$this->removeRecipient($this->getUserName(), $dn);
			}
			elseif (strpos($key, 'del_') === 0) {
				$index = substr($key, strlen('del_'));
				$dn = $this->cachedAliasList[$index]['dn'];
				$this->deleteAlias($dn);
			}
			elseif (strpos($key, 'remMail_') === 0) {
				$parts = substr($key, strlen('remMail_'));
				$parts = explode('_', $parts);
				$this->removeRecipient($mails[$parts[1]], $this->cachedAliasList[$parts[0]]['dn']);
			}
			elseif (strpos($key, 'delMail_') === 0) {
				$parts = substr($key, strlen('remMail_'));
				$parts = explode('_', $parts);
				$this->deleteAlias($this->cachedAliasList[$parts[0]]['dn']);
			}
		}
		return $errors;
	}

	/**
	 * Removes a recipient from the given DN.
	 *
	 * @param String $recipient recipient as user name or email
	 * @param String $dn alias DN
	 */
	private function removeRecipient($recipient, $dn) {
		if (!isset($this->aliasesToAdd[$dn])) { // no new entry update existing
			if (isset($this->recipientsToAdd[$dn]) && in_array($recipient, $this->recipientsToAdd[$dn])) {
				// undo adding of recipient
				if (sizeof($this->recipientsToAdd[$dn]) == 1) {
					// only one entry as marked for adding, remove whole entry
					unset($this->recipientsToAdd[$dn]);
				}
				else {
					$this->recipientsToAdd[$dn] = array_values(array_delete(array($recipient), $this->recipientsToAdd[$dn]));
				}
			}
			else {
				// mark for removal
				$this->recipientsToDelete[$dn][] = $recipient;
			}
			foreach ($this->cachedAliasList as $index => $attrs) {
				if ($dn == $attrs['dn']) {
					$this->cachedAliasList[$index]['rfc822mailmember'] = array_values(array_delete(array($recipient), $this->cachedAliasList[$index]['rfc822mailmember']));
				}
			}
		}
		else { // new entry
			if (sizeof($this->aliasesToAdd[$dn]['rfc822mailmember']) == 1) {
				// single recipient in new entry, do not create new entry at all
				unset($this->aliasesToAdd[$dn]);
				foreach ($this->cachedAliasList as $index => $attrs) {
					if ($dn == $attrs['dn']) {
						unset($this->cachedAliasList[$index]);
						$this->cachedAliasList = array_values($this->cachedAliasList);
					}
				}
			}
			else {
				$this->aliasesToAdd[$dn]['rfc822mailmember'] = array_values(array_delete(array($recipient), $this->aliasesToAdd[$dn]['rfc822mailmember']));
				foreach ($this->cachedAliasList as $index => &$attrs) {
					if ($dn == $attrs['dn']) {
						$attrs['rfc822mailmember'] = array_values(array_delete(array($recipient), $attrs['rfc822mailmember']));
					}
				}
			}
		}
	}

	/**
	 * Removes an alias with the given DN.
	 *
	 * @param String $dn alias DN
	 */
	private function deleteAlias($dn) {
		if (!isset($this->aliasesToAdd[$dn])) {
			// no new entry, delete existing entry
			$this->aliasesToDelete[] = $dn;
		}
		else {
			unset($this->aliasesToAdd[$dn]);
			foreach ($this->cachedAliasList as $index => $attrs) {
				if ($dn == $attrs['dn']) {
					unset($this->cachedAliasList[$index]);
					$this->cachedAliasList = array_values($this->cachedAliasList);
				}
			}
		}
		if (isset($this->recipientsToAdd[$dn])) {
			unset($this->recipientsToAdd[$dn]);
		}
		if (isset($this->recipientsToDelete[$dn])) {
			unset($this->recipientsToDelete[$dn]);
		}
	}

	/**
	 * Returns the HTML meta data for the add page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_add() {
		$return = new htmlTable();
		$aliases = $this->getMailAliasList();
		$count = sizeof($aliases);
		$userName = $this->getUserName();
		$mails = $this->getMailAddresses();
		$recipient = null;
		// get recipient value to add
		if (isset($_POST['recipient'])) {
			$recipient = $_POST['recipient'];
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_add_user'])) {
			$recipient = $userName;
		}
		else {
			for ($m = 0; $m < sizeof($mails); $m++) {
				if (isset($_POST['form_subpage_' . get_class($this) . '_add_mail' . $m])) {
					$recipient = $mails[$m];
					break;
				}
			}
		}
		$return->addElement(new htmlOutputText(_('Recipient')));
		$return->addElement(new htmlOutputText($recipient), true);
		$return->addElement(new htmlHiddenInput('recipient', $recipient), true);
		// new mail alias
		$return->addElement(new htmlSubTitle(_('Create new alias')), true);
		$typeManager = new \LAM\TYPES\TypeManager();
		$mailAliasTypes = $typeManager->getConfiguredTypesForScope('mailAlias');
		$ous = array();
		foreach ($mailAliasTypes as $type) {
			$ous = array_merge($ous, $type->getSuffixList());
		}
		$ous = array_unique($ous);
		usort($ous, 'compareDN');
		$suffixSelect = new htmlTableExtendedSelect('new_ou', $ous, array(), _('Suffix'), 'suffix');
		$suffixSelect->setRightToLeftTextDirection(true);
		$suffixSelect->setSortElements(false);
        $return->addElement($suffixSelect, true);
        $newAliasCn = empty($_POST['new_cn']) ? '' : $_POST['new_cn'];
        $return->addElement(new htmlTableExtendedInputField(_('Alias name'), 'new_cn', $newAliasCn, 'newAlias'), true);
		$return->addVerticalSpace('5px');
		$addButton = new htmlAccountPageButton(get_class($this), 'attributes', 'create', _('Create'));
		$addButton->setIconClass('createButton');
		$addButton->colspan = 5;
		$return->addElement($addButton, true);

		$return->addVerticalSpace('20px');

		// add to existing alias
		$return->addElement(new htmlSubTitle(_('Add to existing alias')), true);
		$aliasesToAdd = array();
		foreach ($aliases as $index => $attrs) {
			if (!empty($attrs['rfc822mailmember']) && in_array($recipient, $attrs['rfc822mailmember'])) {
				continue;
			}
			$aliasesToAdd[$attrs['cn'][0]] = $index;
		}
		$aliasSelect = new htmlTableExtendedSelect('ex_cn', $aliasesToAdd, array(), _('Alias names'), 'aliasNames', 20);
		$aliasSelect->setHasDescriptiveElements(true);
		$aliasSelect->setMultiSelect(true);
		$return->addElement($aliasSelect, true);
		$return->addVerticalSpace('5px');
		$addButton = new htmlAccountPageButton(get_class($this), 'attributes', 'recipient', _('Add'));
		$addButton->setIconClass('createButton');
		$addButton->colspan = 5;
		$return->addElement($addButton, true);

        $return->addElement(new htmlEqualWidth(array('new_ou', 'new_cn', 'ex_cn')));
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_add() {
		$errors = array();
		$mails = $this->getMailAddresses();
		// create new alias entry
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_create'])) {
			if (empty($_POST['new_cn']) || !get_preg($_POST['new_cn'], 'nis_alias')) {
				$errors[] = $this->messages['alias'][0];
			}
			else {
				// build new alias entry
				$newDN = 'cn=' . $_POST['new_cn'] . ',' . $_POST['new_ou'];
				$found = false;
				foreach ($this->cachedAliasList as $key => $attrs) {
					if ($attrs['dn'] == $newDN) {
						$found = true;
						break;
					}
				}
				if ($found) {
					$errors[] = $this->messages['alias'][1];
				}
				else {
					$newAttrs = array(
						'dn' => $newDN,
						'cn' => array($_POST['new_cn']),
						'objectclass' => array('nisMailAlias'),
						'rfc822mailmember' => array($_POST['recipient'])
					);
					$this->aliasesToAdd[$newDN] = $newAttrs;
					$this->cachedAliasList[] = $newAttrs;
				}
			}
		}
		// add recipient to existing entries
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_recipient'])) {
			$selectedAliases = $_POST['ex_cn'];
			foreach ($selectedAliases as $index) {
				$dn = $this->cachedAliasList[$index]['dn'];
				$this->cachedAliasList[$index]['rfc822mailmember'][] = $_POST['recipient'];
				if (isset($this->aliasesToAdd[$dn])) {
					$this->aliasesToAdd[$dn]['rfc822mailmember'][] = $_POST['recipient'];
				}
				else {
					$this->recipientsToAdd[$dn][] = $_POST['recipient'];
				}
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
		foreach ($this->recipientsToDelete as $dn => $recipients) {
			$success = @ldap_mod_del($_SESSION['ldap']->server(), $dn, array('rfc822mailmember' => $recipients));
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to remove recipients ' . implode(', ', $recipients) . ' from  ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$errors[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
			else {
				logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed recipients ' . implode(', ', $recipients) . ' from ' . $dn);
			}
		}
		// create new aliases
		foreach ($this->aliasesToAdd as $dn => $attrs) {
			unset($attrs['dn']);
			$success = @ldap_add($_SESSION['ldap']->server(), $dn, $attrs);
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to create mail alias ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$errors[] = array('ERROR', sprintf(_('Was unable to create DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
			else {
				logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added mail alias with recipients ' . implode(', ', $attrs['rfc822mailmember']) . ' and DN ' . $dn);
			}
		}
		// add recipients
		foreach ($this->recipientsToAdd as $dn => $recipients) {
			$success = @ldap_mod_add($_SESSION['ldap']->server(), $dn, array('rfc822mailmember' => $recipients));
			if (!$success) {
				logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add recipients ' . implode(', ', $recipients) . ' to  ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
				$errors[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
			else {
				logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added recipients ' . implode(', ', $recipients) . ' to ' . $dn);
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
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
		$this->addPDFKeyValue($return, 'alias', _('Alias names'), implode(', ', $foundAliases));
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

	/**
	 * Returns if the mail alias type is active. Otherwise, aliases cannot be managed.
	 *
	 * @return boolean is active
	 */
	private function isMailAliasTypeActive() {
		$typeManager = new TypeManager();
		$activeTypes = $typeManager->getConfiguredTypesForScope('mailAlias');
		return !empty($activeTypes);
	}

}


?>
