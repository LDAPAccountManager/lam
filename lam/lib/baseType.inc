<?php

use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2024  Roland Gruber

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
 * This is the parent class of all account types.
 *
 * @package types
 * @author Roland Gruber
 */

/**
 * This is the parent class of all account types.
 *
 * @package types
 */
class baseType {

	/** label to create another account */
	public $LABEL_CREATE_ANOTHER_ACCOUNT;
	/** label to return to account list */
	public $LABEL_BACK_TO_ACCOUNT_LIST;

	/** type configuration */
	private $type;

	/**
	 * Creates a new type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		$this->type = $type;
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another account');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to account list');
	}

	/**
	 * Returns the alias name of this account type.
	 * This function must be overwritten by the child classes.
	 *
	 * @return string alias name
	 */
	public function getAlias() {
		return "baseType";
	}

	/**
	 * Returns the description of this account type.
	 * This function must be overwritten by the child classes.
	 *
	 * @return string description
	 */
	public function getDescription() {
		return "base type";
	}

	/**
	 * Returns the class name for the list object.
	 *
	 * @return string class name
	 */
	public function getListClassName() {
		return "lamList";
	}

	/**
	 * Returns the default attribute list for this account type.
	 * This function must be overwritten by the child classes.
	 *
	 * @return string attribute list
	 */
	public function getDefaultListAttributes() {
		return "dn:DN,objectClass:Object classes";
	}

	/**
	 * Returns a list of attributes which have a translated description.
	 * This is used for the head row in the list view.
	 *
	 * @return array list of descriptions
	 */
	public function getListAttributeDescriptions() {
		$descriptions = [];
		foreach ($this->getType()->getModules() as $moduleName) {
			$module = moduleCache::getModule($moduleName, $this->getScope());
			if ($module === null) {
				continue;
			}
			$descriptions = array_merge($descriptions, $module->getListAttributeDescriptions($this->getType()));
		}
		return $descriptions;
	}

	/**
	 * Returns if entries of this type may be created via file upload.
	 *
	 * @return boolean true, if file upload is supported
	 */
	public function supportsFileUpload() {
		return true;
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		if ($container->dn_orig == null) {
			return null;
		}
		return htmlspecialchars(getAbstractDN($container->dn_orig));
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarSubtitle($container) {
		return null;
	}

	/**
	 * Builds the HTML code for the icon that shows the account status (locked/unlocked).
	 *
	 * @param accountContainer $container account container
	 * @return htmlElement|null status icon
	 */
	public function buildAccountStatusIcon(accountContainer $container): ?htmlElement {
		$statusContent = new htmlGroup();
		$details = [];
		$lockableOptions = [];
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
		$altText = _('Unlocked');
		if ($accountStatus->isLocked()) {
			$mainIcon = 'locked.svg';
			$altText = _('Locked');
		}
		elseif ($accountStatus->isPartiallyLocked()) {
			$mainIcon = 'partiallyLocked.svg';
			$altText = _('Partially locked');
		}
		elseif ($accountStatus->isExpired()) {
			$mainIcon = 'expired.svg';
			$altText = _('Expired');
		}
		elseif ($accountStatus->isPartiallyExpired()) {
			$mainIcon = 'expired.svg';
			$altText = _('Expired');
		}
		$statusIcon = new htmlImage('../../graphics/' . $mainIcon, null, null, $altText);
		$statusIcon->setCSSClasses(['icon']);
		if (!empty($accountStatus->getDetails()) || !empty($lockableOptions)) {
			$detailsTable = new htmlTable();
			foreach ($accountStatus->getDetails() as $detail) {
				$label = new htmlOutputText($detail->getDetails());
				$label->setNoWrap(true);
				$detailsTable->addElement($label);
				$statusDetailIcon = new htmlImage('../../graphics/' . $detail->getIcon(), '16px', '16px');
				$statusDetailIcon->setCSSClasses(['fixed-size-16']);
				$detailsTable->addElement($statusDetailIcon);
				$detailsTable->addNewLine();
			}
			if (empty($accountStatus->getDetails())) {
				$detailsTable->addElement(new htmlOutputText(_('Unlocked')));
				$statusDetailIcon = new htmlImage('../../graphics/unlocked.svg', '16px', '16px');
				$statusDetailIcon->setCSSClasses(['fixed-size-16']);
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
				$statusIcon->setCSSClasses(['clickable', 'icon']);
			}
		}
		$statusContent->addElement($statusIcon);
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
			$onchange = 'window.lam.accountLists.lockUnlockRadioChange();';
		}
		if (!$hasUnlockOptions) {
			$selectedRadio = 'lock';
		}
		if (!$radioDisabled) {
			$radio = new htmlRadio('lam_accountStatusAction', [_('Lock') => 'lock', _('Unlock') => 'unlock'], $selectedRadio);
			$radio->setOnchangeEvent($onchange);
			$container->add($radio, 12);
		}
		else {
			$radio = new htmlRadio('lam_accountStatusActionDisabled', [_('Lock') => 'lock', _('Unlock') => 'unlock'], $selectedRadio);
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
				$image->setCSSClasses(['align-middle']);
				$lockContentGroup->addElement($image);
				$checkbox = new htmlInputCheckbox('lam_accountStatusLock_' . $lockableOption->getId(), $preSelected);
				$checkbox->setCSSClasses(['align-middle']);
				$lockContentGroup->addElement($checkbox);
				$label = new htmlLabel('lam_accountStatusLock_' . $lockableOption->getId(), $lockableOption->getDetails());
				$label->setCSSClasses(['align-middle']);
				$lockContentGroup->addElement($label);
				$lockContent->add($lockContentGroup);
			}

			$lockDiv = new htmlDiv('lam_accountStatusDialogLockDiv', $lockContent);
			if ($hasUnlockOptions) {
				$lockDiv->setCSSClasses(['hidden']);
			}
			$container->add($lockDiv);
		}
		// unlocking part
		if ($hasUnlockOptions) {
			$unlockContent = new htmlResponsiveRow();
			foreach ($accountStatus->getDetails() as $detail) {
				$unlockContentGroup = new htmlGroup();
				$image = new htmlImage('../../graphics/' . $detail->getIcon(), '16px', '16px');
				$image->setCSSClasses(['align-middle']);
				$unlockContentGroup->addElement($image);
				$checkbox = new htmlInputCheckbox('lam_accountStatusUnlock_' . $detail->getId(), true);
				$checkbox->setCSSClasses(['align-middle']);
				$unlockContentGroup->addElement($checkbox);
				$label = new htmlLabel('lam_accountStatusUnlock_' . $detail->getId(), $detail->getDetails());
				$label->setCSSClasses(['align-middle']);
				$unlockContentGroup->addElement($label);
				$unlockContent->add($unlockContentGroup);
			}
			$unlockDiv = new htmlDiv('lam_accountStatusDialogUnlockDiv', $unlockContent);
			$container->add($unlockDiv);
		}
		$container->addVerticalSpacer('1rem');

		$buttonGroup = new htmlGroup();
		$okButton = new htmlButton('changeStatusOk', _('Ok'));
		$okButton->setCSSClasses(['lam-primary']);
		$okButton->setOnClick("document.getElementById('lam_accountStatusResult').value = 'ok'; document.forms['inputForm'].submit();");
		$buttonGroup->addElement($okButton);
		$cancelButton = new htmlButton('changeStatusCancel', _('Cancel'));
		$cancelButton->setOnClick("document.querySelector('.modal-account-status').classList.remove('show-modal'); return false;");
		$buttonGroup->addElement($cancelButton);
		$container->add($buttonGroup);

		$innerDiv = new htmlDiv('lam_accountStatusDialogContent', $container);
		$innerDiv->setCSSClasses(['modal-content']);
		$div = new htmlDiv('lam_accountStatusDialog', $innerDiv);
		$div->setCSSClasses(['modal modal-account-status']);
		return $div;
	}

	/**
	 * Returns the LDAP filter to find the possible suffixes for this account type.
	 *
	 * @return string LDAP filter
	 */
	public function getSuffixFilter() {
		return "(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container)(dc=*))";
	}

	/**
	 * This function is called after the edit page is processed and before the page content is generated.
	 * This can be used to run custom handlers after each page processing.
	 *
	 * @param accountContainer $container account container
	 */
	public function runEditPagePostAction(accountContainer $container): void {
		// check if account status should be changed
		if (isset($_POST['lam_accountStatusResult']) && ($_POST['lam_accountStatusResult'] == 'ok')) {
			// lock account
			if ($_POST['lam_accountStatusAction'] == 'lock') {
				$lockIds = [];
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
				$unlockIds = [];
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

	/**
	 * Returns a list of configuration options.
	 *
	 * The field names are used as keywords to load and save settings.
	 * We recommend to use the type name as prefix for them (e.g. user_someSetting) to avoid naming conflicts.
	 *
	 * @return mixed htmlElement or array of htmlElement
	 *
	 * @see htmlElement
	 */
	public function get_configOptions() {
		return null;
	}

	/**
	 * Checks input values of config settings.
	 * <br>
	 * If the input data is invalid the return value is an array that contains subarrays to build StatusMessages ('message type', 'message head', 'message text').
	 * <br>If no errors occurred the function returns an empty array.
	 *
	 * @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @return array list of error messages
	 */
	public function check_configOptions(&$options) {
		return [];
	}

	/**
	 * Returns if this account type supports multiple configurations.
	 *
	 * @return boolean multiple configs supported
	 */
	public function supportsMultipleConfigs() {
		return true;
	}

	/**
	 * Returns the type configuration.
	 *
	 * @return ConfiguredType type configuration
	 */
	protected function getType() {
		return $this->type;
	}

	/**
	 * Returns the file name of the type icon.
	 * It needs to be 16x16px and located in graphics folder.
	 * By default, this is "{type name}.png"
	 *
	 * @return string file name
	 */
	public function getIcon(): string {
		return get_class($this) . '.png';
	}

	/**
	 * Returns the scope name.
	 *
	 * @return string scope
	 */
	public function getScope() {
		return get_class($this);
	}

}
