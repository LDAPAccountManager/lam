<?php

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2023  Roland Gruber

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
* The account type for user accounts (e.g. Unix, Samba and Kolab).
*
* @package types
* @author Roland Gruber
*/

/**
* The account type for user accounts (e.g. Unix, Samba and Kolab).
*
* @package types
*/
class user extends baseType {

	/**
	 * Constructs a new user type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another user');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to user list');
	}

	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		return _("Users");
	}

	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("User accounts (e.g. Unix, Samba and Kolab)");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getIcon() {
		return 'user.svg';
	}

	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamUserList";
	}

	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#uid;#givenName;#sn;#uidNumber;#gidNumber";
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		$title = '';
		ob_start();
		$tabIndex = 10000;
		$iconContent = $this->buildAccountStatusIcon($container);
		if ($iconContent !== null) {
			parseHtml(null, $iconContent, array(), false, $tabIndex, $container->get_type()->getScope());
			$title = ob_get_contents();
		}
		ob_end_clean();
		// get attributes
		$personalAttributes = null;
		if ($container->getAccountModule('inetOrgPerson') != null) {
			$personalAttributes = $container->getAccountModule('inetOrgPerson')->getAttributes();
		}
		elseif ($container->getAccountModule('windowsUser') != null) {
			$personalAttributes = $container->getAccountModule('windowsUser')->getAttributes();
		}
		$accountAttributes = null;
		if ($container->getAccountModule('account') != null) {
			$accountAttributes = $container->getAccountModule('account')->getAttributes();
		}
		$sambaAttributes = null;
		if ($container->getAccountModule('sambaSamAccount') != null) {
			$sambaAttributes = $container->getAccountModule('sambaSamAccount')->getAttributes();
		}
		$unixAttributes = null;
		if ($container->getAccountModule('posixAccount') != null) {
			$unixAttributes = $container->getAccountModule('posixAccount')->getAttributes();
		}
		$mitKerberosAttributes = null;
		if ($container->getAccountModule('mitKerberosStructural') != null) {
			$mitKerberosAttributes = $container->getAccountModule('mitKerberosStructural')->getAttributes();
		}
		elseif ($container->getAccountModule('mitKerberos') != null) {
			$mitKerberosAttributes = $container->getAccountModule('mitKerberos')->getAttributes();
		}
		// check if first and last name can be shown
		if (($personalAttributes != null) && isset($personalAttributes['sn'][0]) && !empty($personalAttributes['sn'][0])
				&& isset($personalAttributes['givenName'][0]) && !empty($personalAttributes['givenName'][0])) {
			return $title . htmlspecialchars($personalAttributes['givenName'][0] . ' ' . $personalAttributes['sn'][0]);
		}
		// check if a display name is set
		if (($sambaAttributes != null) && isset($sambaAttributes['displayName'][0]) && !empty($sambaAttributes['displayName'][0])) {
			return $title . htmlspecialchars($sambaAttributes['displayName'][0]);
		}
		// check if a common name is set
		if (($personalAttributes != null) && isset($personalAttributes['cn'][0]) && !empty($personalAttributes['cn'][0])) {
			return $title . htmlspecialchars($personalAttributes['cn'][0]);
		}
		if (($unixAttributes != null) && isset($unixAttributes['cn'][0]) && !empty($unixAttributes['cn'][0])) {
			return $title . htmlspecialchars($unixAttributes['cn'][0]);
		}
		// check if a user name is set
		if (($unixAttributes != null) && isset($unixAttributes['uid'][0]) && !empty($unixAttributes['uid'][0])) {
			return $title . htmlspecialchars($unixAttributes['uid'][0]);
		}
		if (($personalAttributes != null) && isset($personalAttributes['uid'][0]) && !empty($personalAttributes['uid'][0])) {
			return $title . htmlspecialchars($personalAttributes['uid'][0]);
		}
		if (($accountAttributes != null) && isset($accountAttributes['uid'][0]) && !empty($accountAttributes['uid'][0])) {
			return $title . htmlspecialchars($accountAttributes['uid'][0]);
		}
		if (($mitKerberosAttributes != null) && isset($mitKerberosAttributes['krbPrincipalName'][0]) && !empty($mitKerberosAttributes['krbPrincipalName'][0])) {
			return $title . htmlspecialchars($mitKerberosAttributes['krbPrincipalName'][0]);
		}
		if ($container->isNewAccount) {
			return $title . _("New user");
		}
		// fall back to default
		return $title . parent::getTitleBarTitle($container);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarSubtitle($container) {
		$personalAttributes = null;
		if ($container->getAccountModule('inetOrgPerson') != null) {
			$personalAttributes = $container->getAccountModule('inetOrgPerson')->getAttributes();
		}
		elseif ($container->getAccountModule('windowsUser') != null) {
			$personalAttributes = $container->getAccountModule('windowsUser')->getAttributes();
		}
		if ($personalAttributes == null) {
			return '';
		}
		$subtitle = '';
		$spacer = ' &nbsp;&nbsp;&nbsp; ';
		// check if an email address can be shown
		if (isset($personalAttributes['mail'][0]) && !empty($personalAttributes['mail'][0])) {
			$subtitle .= '<a href="mailto:' . htmlspecialchars($personalAttributes['mail'][0]) . '">' . htmlspecialchars($personalAttributes['mail'][0]) . '</a>' . $spacer;
		}
		// check if an telephone number can be shown
		if (isset($personalAttributes['telephoneNumber'][0]) && !empty($personalAttributes['telephoneNumber'][0])) {
			$subtitle .= _('Telephone number') . ' ' . htmlspecialchars($personalAttributes['telephoneNumber'][0]) . $spacer;
		}
		// check if an mobile number can be shown
		if (isset($personalAttributes['mobile'][0]) && !empty($personalAttributes['mobile'][0])) {
			$subtitle .= _('Mobile number') . ' ' . htmlspecialchars($personalAttributes['mobile'][0]);
		}
		if ($subtitle == '') {
			return null;
		}
		return $subtitle;
	}

	/**
	 * Builds the HTML code for the icon that shows the account status (locked/unlocked).
	 *
	 * @param accountContainer $container account container
	 * @return htmlElement|null status icon
	 */
	private function buildAccountStatusIcon(accountContainer $container): ?htmlElement {
		$statusContent = new htmlGroup();
		$details = array();
		$lockableOptions = array();
		$statusSupported = false;
		foreach ($container->getAccountModules() as $module) {
			$interfaces = class_implements($module);
			if (!in_array('AccountStatusProvider', $interfaces)) {
				continue;
			}
			$statusSupported = true;
			$dummyAttributes = null;
			$details = array_merge($details, $module->getAccountStatusDetails($container->get_type(), $dummyAttributes));
			$lockableOptions = array_merge($lockableOptions, $module->getAccountStatusPossibleLockOptions($container->get_type(), $dummyAttributes));
		}
		if (!$statusSupported) {
			return null;
		}
		$accountStatus = new AccountStatus($details);
		$mainIcon = 'unlocked.svg';
		if ($accountStatus->isLocked()) {
			$mainIcon = 'locked.svg';
		}
		elseif ($accountStatus->isPartiallyLocked()) {
			$mainIcon = 'partiallyLocked.svg';
		}
		elseif ($accountStatus->isExpired()) {
			$mainIcon = 'expired.svg';
		}
		elseif ($accountStatus->isPartiallyExpired()) {
			$mainIcon = 'expired.svg';
		}
		$statusIcon = new htmlImage('../../graphics/' . $mainIcon, 16, 16);
		if (!empty($accountStatus->getDetails()) || !empty($lockableOptions)) {
			$detailsTable = new htmlTable();
			foreach ($accountStatus->getDetails() as $detail) {
				$label = new htmlOutputText($detail->getDetails());
				$label->setNoWrap(true);
				$detailsTable->addElement($label);
				$statusDetailIcon = new htmlImage('../../graphics/' . $detail->getIcon(), '16px', '16px');
				$statusDetailIcon->setCSSClasses(array('fixed-size-16'));
				$detailsTable->addElement($statusDetailIcon);
				$detailsTable->addNewLine();
			}
			if (empty($accountStatus->getDetails())) {
				$detailsTable->addElement(new htmlOutputText(_('Unlocked')));
				$statusDetailIcon = new htmlImage('../../graphics/unlocked.svg', '16px', '16px');
				$statusDetailIcon->setCSSClasses(array('fixed-size-16'));
				$detailsTable->addElement($statusDetailIcon);
				$detailsTable->addNewLine();
			}
			$detailsTable->addElement(new htmlSpacer(null, '1rem'), true);
			$hintGroup = new htmlGroup();
			$hintGroup->colspan = 2;
			$hintGroup->addElement(new htmlImage('../../graphics/light.svg', '16px', '16px'));
			$hintGroup->addElement(new htmlOutputText(_('Please click to lock/unlock this account.')));
			$detailsTable->addElement($hintGroup);
			$statusIcon->setHelpData(_('Account status'), $detailsTable);
			if (checkIfWriteAccessIsAllowed($container->get_type()->getScope())) {
				$statusIcon->setOnClick("window.lam.dialog.showModal('.modal-account-status')");
				$statusIcon->setCSSClasses(array('clickable'));
			}
		}
		$statusContent->addElement($statusIcon);
		$statusContent->addElement(new htmlSpacer('1rem', null));
		$dialogDiv = $this->buildAccountStatusDialogDiv($accountStatus, $lockableOptions);
		$statusContent->addElement($dialogDiv);
		return $statusContent;
	}

	/**
	 * Builds the dialog to (un)lock parts of an account.
	 *
	 * @param AccountStatus $accountStatus account status
	 * @param AccountStatusDetails[] $lockableOptions possible lock options
	 * @return htmlElement dialog div
	 */
	private function buildAccountStatusDialogDiv(AccountStatus $accountStatus, array $lockableOptions): htmlElement {
		$hasLockOptions = !empty($lockableOptions);
		$hasUnlockOptions = $accountStatus->isLocked() || $accountStatus->isPartiallyLocked();
		$container = new htmlResponsiveRow();
		$container->add(new htmlTitle(_('Change account status')));

		// show radio buttons for lock/unlock
		$radioDisabled = true;
		$selectedRadio = 'unlock';
		$onchange = '';
		if ($hasLockOptions && $hasUnlockOptions) {
			$radioDisabled = false;
			$onchange = 'if (jQuery(\'#lam_accountStatusAction0:checked\').val()) {' .
								'jQuery(\'#lam_accountStatusDialogLockDiv\').removeClass(\'hidden\');' .
								'jQuery(\'#lam_accountStatusDialogUnlockDiv\').addClass(\'hidden\');' .
							'}' .
							'else {' .
								'jQuery(\'#lam_accountStatusDialogLockDiv\').addClass(\'hidden\');' .
								'jQuery(\'#lam_accountStatusDialogUnlockDiv\').removeClass(\'hidden\');' .
							'};';
		}
		if (!$hasUnlockOptions) {
			$selectedRadio = 'lock';
		}
		if (!$radioDisabled) {
			$radio = new htmlRadio('lam_accountStatusAction', array(_('Lock') => 'lock', _('Unlock') => 'unlock'), $selectedRadio);
			$radio->setOnchangeEvent($onchange);
			$container->add($radio, 12);
		}
		else {
			$radio = new htmlRadio('lam_accountStatusActionDisabled', array(_('Lock') => 'lock', _('Unlock') => 'unlock'), $selectedRadio);
			$radio->setIsEnabled(false);
			$container->add($radio, 12);
			$container->add(new htmlHiddenInput('lam_accountStatusAction', $selectedRadio), 12);
		}
		$container->addVerticalSpacer('1rem');

		$container->add(new htmlHiddenInput('lam_accountStatusResult', 'cancel'), 12);

		// locking part
		if ($hasLockOptions) {
			$lockContent = new htmlResponsiveRow();
			foreach ($lockableOptions as $lockableOption) {
				$preSelected = $lockableOption->isLocked();
				$lockContentGroup = new htmlGroup();
				$image = new htmlImage('../../graphics/' . $lockableOption->getIcon(), '16px', '16px');
				$image->setCSSClasses(array('align-middle'));
				$lockContentGroup->addElement($image);
				$checkbox = new htmlInputCheckbox('lam_accountStatusLock_' . $lockableOption->getId(), $preSelected);
				$checkbox->setCSSClasses(array('align-middle'));
				$lockContentGroup->addElement($checkbox);
				$label = new htmlLabel('lam_accountStatusLock_' . $lockableOption->getId(), $lockableOption->getDetails());
				$label->setCSSClasses(array('align-middle'));
				$lockContentGroup->addElement($label);
				$lockContent->add($lockContentGroup);
			}

			$lockDiv = new htmlDiv('lam_accountStatusDialogLockDiv', $lockContent);
			if ($hasUnlockOptions) {
				$lockDiv->setCSSClasses(array('hidden'));
			}
			$container->add($lockDiv);
		}
		// unlocking part
		if ($hasUnlockOptions) {
			$unlockContent = new htmlResponsiveRow();
			foreach ($accountStatus->getDetails() as $detail) {
				$unlockContentGroup = new htmlGroup();
				$image = new htmlImage('../../graphics/' . $detail->getIcon(), '16px', '16px');
				$image->setCSSClasses(array('align-middle'));
				$unlockContentGroup->addElement($image);
				$checkbox = new htmlInputCheckbox('lam_accountStatusUnlock_' . $detail->getId(), true);
				$checkbox->setCSSClasses(array('align-middle'));
				$unlockContentGroup->addElement($checkbox);
				$label = new htmlLabel('lam_accountStatusUnlock_' . $detail->getId(), $detail->getDetails());
				$label->setCSSClasses(array('align-middle'));
				$unlockContentGroup->addElement($label);
				$unlockContent->add($unlockContentGroup);
			}
			$unlockDiv = new htmlDiv('lam_accountStatusDialogUnlockDiv', $unlockContent);
			$container->add($unlockDiv);
		}
		$container->addVerticalSpacer('1rem');

		$buttonGroup = new htmlGroup();
		$okButton = new htmlButton('changeStatusOk', _('Ok'));
		$okButton->setCSSClasses(array('lam-primary'));
		$okButton->setOnClick("document.getElementById('lam_accountStatusResult').value = 'ok'; document.forms['inputForm'].submit();");
		$buttonGroup->addElement($okButton);
		$cancelButton = new htmlButton('changeStatusCancel', _('Cancel'));
		$cancelButton->setOnClick("document.querySelector('.modal-account-status').classList.remove('show-modal'); return false;");
		$buttonGroup->addElement($cancelButton);
		$container->add($buttonGroup);

		$innerDiv = new htmlDiv('lam_accountStatusDialogContent', $container);
		$innerDiv->setCSSClasses(array('modal-content'));
		$div = new htmlDiv('lam_accountStatusDialog', $innerDiv);
		$div->setCSSClasses(array('modal modal-account-status'));
		return $div;
	}

	/**
	 * This function is called after the edit page is processed and before the page content is generated.
	 * This can be used to run custom handlers after each page processing.
	 *
	 * @param accountContainer $container account container
	 */
	public function runEditPagePostAction(&$container) {
		$modules = $this->getType()->getModules();
		// check if account status should be changed
		if (isset($_POST['lam_accountStatusResult']) && ($_POST['lam_accountStatusResult'] == 'ok')) {
			// lock account
			if ($_POST['lam_accountStatusAction'] == 'lock') {
				$lockIds = array();
				foreach ($_POST as $key => $value) {
					if (($value === 'on') && (strpos($key, 'lam_accountStatusLock_') === 0)) {
						$lockIds[] = substr($key, strlen('lam_accountStatusLock_'));
					}
				}
				foreach ($container->getAccountModules() as $module) {
					$interfaces = class_implements($module);
					if (!in_array('AccountStatusProvider', $interfaces)) {
						continue;
					}
					$dummyAttributes = null;
					$module->accountStatusPerformLock($container->get_type(), $dummyAttributes, $lockIds);
				}
			}
			// unlock account
			elseif ($_POST['lam_accountStatusAction'] == 'unlock') {
				$unlockIds = array();
				foreach ($_POST as $key => $value) {
					if (($value === 'on') && (strpos($key, 'lam_accountStatusUnlock_') === 0)) {
						$unlockIds[] = substr($key, strlen('lam_accountStatusUnlock_'));
					}
				}
				foreach ($container->getAccountModules() as $module) {
					$interfaces = class_implements($module);
					if (!in_array('AccountStatusProvider', $interfaces)) {
						continue;
					}
					$dummyAttributes = null;
					$module->accountStatusPerformUnlock($container->get_type(), $dummyAttributes, $unlockIds);
				}
			}
		}
	}

}

/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 *
 */
class lamUserList extends lamList {

	/** Controls if GID number is translated to group name */
	private $trans_primary = false;

	/** Controls if the account status is shown */
	private $showAccountStatus = false;

	/** translates GID to group name */
	private $trans_primary_hash = array();

	/** filter value for account status */
	private $accountStatusFilter = null;

	/** ID for config option to translate primary group GIDs to group names */
	const TRANS_PRIMARY_OPTION_NAME = "LU_TP";
	/** ID for config option to show account status */
	const ACCOUNT_STATUS_OPTION_NAME = "LU_AS";

	/** virtual attribute name for account status column */
	const ATTR_ACCOUNT_STATUS = 'lam_virtual_account_status';

	/** filter value for expired accounts */
	const FILTER_EXPIRED = 1;
	/** filter value for locked accounts */
	const FILTER_LOCKED = 2;
	/** filter value for partially locked accounts */
	const FILTER_SEMILOCKED = 3;
	/** filter value for unlocked accounts */
	const FILTER_UNLOCKED = 4;

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("User count: %s"),
			'error_noneFound' => _("No users found!"),
			'newEntry' => _("New user"),
			'deleteEntry' => _("Delete selected users"));
	}

	/**
	 * Sets some internal parameters.
	 */
	protected function listGetParams() {
		parent::listGetParams();
		// generate hash table for group translation
		if ($this->trans_primary == "on" && !$this->refresh && (sizeof($this->trans_primary_hash) == 0)) {
			$this->refreshPrimaryGroupTranslation();
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	protected function listRefreshData() {
		parent::listRefreshData();
		// show group names
		if ($this->trans_primary == "on") {
			$this->refreshPrimaryGroupTranslation();
		}
		// show account status
		if ($this->showAccountStatus) {
			$this->injectAccountStatusAttribute();
		}
	}

	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation() {
		$this->trans_primary_hash = array();
		$attrs = array("cn", "gidNumber");
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, array('group'));
		$entryCount = sizeof($entries);
		for ($i = 0; $i < $entryCount; $i++) {
			$this->trans_primary_hash[$entries[$i]['gidnumber'][0]] = $entries[$i]['cn'][0];
		}
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::getTableCellContent()
	 */
	protected function getTableCellContent(&$entry, &$attribute) {
		// check if there is something to display at all
		if (($attribute != self::ATTR_ACCOUNT_STATUS) && (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1))) {
			return parent::getTableCellContent($entry, $attribute);
		}
		// translate GID to group name
		if (($attribute == "gidnumber") && ($this->trans_primary == "on")) {
			if (isset($this->trans_primary_hash[$entry[$attribute][0]])) {
				return new htmlOutputText($this->trans_primary_hash[$entry[$attribute][0]]);
			}
			else {
				return parent::getTableCellContent($entry, $attribute);
			}
		}
		// account status
		elseif ($attribute == self::ATTR_ACCOUNT_STATUS) {
			return $this->getAccountStatus($entry);
		}
		// print all other attributes
		else {
			return parent::getTableCellContent($entry, $attribute);
		}
	}

	/**
	 * Returns a list of possible configuration options.
	 *
	 * @return array list of lamListOption objects
	 */
	protected function listGetAllConfigOptions() {
		$options = parent::listGetAllConfigOptions();
		$options[] = new lamBooleanListOption(_('Translate GID number to group name'), self::TRANS_PRIMARY_OPTION_NAME);
		$options[] = new lamBooleanListOption(_('Show account status'), self::ACCOUNT_STATUS_OPTION_NAME);
		return $options;
	}

	/**
	 * Called when the configuration options changed.
	 */
	protected function listConfigurationChanged() {
		parent::listConfigurationChanged();
		$tpOption = $this->listGetConfigOptionByID(self::TRANS_PRIMARY_OPTION_NAME);
		$this->trans_primary = $tpOption->isSelected();
		$asOption = $this->listGetConfigOptionByID(self::ACCOUNT_STATUS_OPTION_NAME);
		// if account status was activated, reload LDAP data
		$asOptionOldValue = $this->showAccountStatus;
		$this->showAccountStatus = $asOption->isSelected();
		if ($this->showAccountStatus && !$asOptionOldValue) {
			$this->forceRefresh();
		}
	}

	/**
	* Returns an hash array containing with all attributes to be shown and their descriptions.
	* <br>Format: array(attribute => description)
	* <br>
	* <br>The user list may display an additional account status column
	*
	* @return array attribute list
	*/
	protected function listGetAttributeDescriptionList() {
		$list = parent::listGetAttributeDescriptionList();
		if ($this->showAccountStatus) {
			$list[self::ATTR_ACCOUNT_STATUS] = _('Account status');
		}
		return $list;
	}

	/**
	 * Returns if the given attribute can be filtered.
	 * If filtering is not possible then no filter box will be displayed.
	 * <br>
	 * <br>The user list allows no filtering for account status.
	 *
	 * @param String $attr attribute name
	 * @return boolean filtering possible
	 */
	protected function canBeFiltered($attr) {
		if (strtolower($attr) == 'jpegphoto') {
			return false;
		}
		return true;
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::getFilterArea()
	 */
	protected function getFilterArea($attrName, $clearFilter) {
		if ($attrName != self::ATTR_ACCOUNT_STATUS) {
			return parent::getFilterArea($attrName, $clearFilter);
		}
		$value = "-";
		if (!$clearFilter && isset($this->filters[$attrName])) {
			$value = $this->filters[$attrName];
		}
		$filterOptions = array(
			'' => '',
			_('Unlocked') => self::FILTER_UNLOCKED,
			_('Partially locked') => self::FILTER_SEMILOCKED,
			_('Locked') => self::FILTER_LOCKED,
			_('Expired') => self::FILTER_EXPIRED,
		);
		$filterInput = new htmlSelect('filter' . strtolower($attrName), $filterOptions, array($value));
		$filterInput->setCSSClasses(array($this->type->getScope() . '-bright'));
		$filterInput->setHasDescriptiveElements(true);
		$filterInput->setOnchangeEvent('document.getElementsByName(\'apply_filter\')[0].click();');
		return $filterInput;
	}

	/**
	 * Builds the LDAP filter based on the filter entries in the GUI.
	 *
	 * @return String LDAP filter
	 */
	protected function buildLDAPAttributeFilter() {
		$this->accountStatusFilter = null;
		foreach ($this->filters as $attr => $filter) {
			if ($attr == self::ATTR_ACCOUNT_STATUS) {
				$this->accountStatusFilter = $filter;
				break;
			}
		}
		return parent::buildLDAPAttributeFilter();
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::isAttributeFilteredByServer()
	 */
	protected function isAttributeFilteredByServer($attrName) {
		// do not filter status server side
		if ($attrName == self::ATTR_ACCOUNT_STATUS) {
			return false;
		}
		return parent::isAttributeFilteredByServer($attrName);
	}

	/**
	 * Returns a list of additional LDAP attributes that should be read.
	 * This can be used to show additional data even if the user selected other attributes to show in the list.
	 * <br>
	 * <br>The user list reads pwdAccountLockedTime, sambaAcctFlags and userPassword
	 *
	 * @return array additional attribute names
	 */
	protected function getAdditionalLDAPAttributesToRead() {
		$attrs = parent::getAdditionalLDAPAttributesToRead();
		if ($this->showAccountStatus) {
			$modules = $_SESSION['config']->get_AccountModules($this->type->getId());
			foreach ($modules as $module) {
				$interfaces = class_implements($module);
				if (!in_array('AccountStatusProvider', $interfaces)) {
					continue;
				}
				$moduleObject = moduleCache::getModule($module, $this->type->getScope());
				$attrs = array_merge($attrs, $moduleObject->getAccountStatusRequiredAttributes($this->type));
			}
		}
		return $attrs;
	}

	/**
	 * Injects values for the virtual account status attribute to make it sortable.
	 */
	private function injectAccountStatusAttribute() {
		$entryCount = sizeof($this->ldapEntries);
		for ($i = 0; $i < $entryCount; $i++) {
			$accountStatus = AccountStatus::fromAttributes($this->type, $this->ldapEntries[$i]);
			$status = self::FILTER_UNLOCKED;
			if ($accountStatus->isLocked()) {
				$status = self::FILTER_LOCKED;
			}
			elseif ($accountStatus->isPartiallyLocked()) {
				$status = self::FILTER_SEMILOCKED;
			}
			elseif ($accountStatus->isPartiallyExpired() || $accountStatus->isExpired()) {
				$status = self::FILTER_EXPIRED;
			}
			// add virtual attribute
			$this->ldapEntries[$i][self::ATTR_ACCOUNT_STATUS][0] = $status;
		}
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::isFilterMatching()
	 */
	protected function isFilterMatching(&$data, $filterAttribute, $regex) {
		if ($filterAttribute == self::ATTR_ACCOUNT_STATUS) {
			return preg_match($regex, $data[self::ATTR_ACCOUNT_STATUS][0]);
		}
		if (($filterAttribute == 'gidnumber') && ($this->trans_primary == "on")) {
			if (!isset($data[$filterAttribute])) {
				return false;
			}
			if (!isset($this->trans_primary_hash[$data[$filterAttribute][0]])) {
				return false;
			}
			return preg_match($regex, $this->trans_primary_hash[$data[$filterAttribute][0]]);
		}
		return parent::isFilterMatching($data, $filterAttribute, $regex);
	}

	/**
	 * Returns the account status.
	 *
	 * @param array $attrs LDAP attributes
	 * @return htmlElement content
	 */
	private function getAccountStatus(&$attrs) {
		$statusContent = new htmlGroup();
		$accountStatus = AccountStatus::fromAttributes($this->type, $attrs);
		$mainIcon = 'unlocked.svg';
		if ($accountStatus->isLocked()) {
			$mainIcon = 'locked.svg';
		}
		elseif ($accountStatus->isPartiallyLocked()) {
			$mainIcon = 'partiallyLocked.svg';
		}
		elseif ($accountStatus->isExpired()) {
			$mainIcon = 'expired.svg';
		}
		elseif ($accountStatus->isPartiallyExpired()) {
			$mainIcon = 'expired.svg';
		}
		$statusIcon = new htmlImage('../../graphics/' . $mainIcon, 16, 16);
		if (!empty($accountStatus->getDetails())) {
			$detailsTable = new htmlTable();
			foreach ($accountStatus->getDetails() as $detail) {
				$label = new htmlOutputText($detail->getDetails());
				$label->setNoWrap(true);
				$detailsTable->addElement($label);
				$statusDetailIcon = new htmlImage('../../graphics/' . $detail->getIcon(), '16px', '16px');
				$statusDetailIcon->setCSSClasses(array('fixed-size-16'));
				$detailsTable->addElement($statusDetailIcon);
				$detailsTable->addNewLine();
			}
			$statusIcon->setHelpData(_('Account status'), $detailsTable);
		}
		$statusContent->addElement($statusIcon);
		return $statusContent;
	}

}
