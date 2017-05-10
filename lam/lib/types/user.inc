<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2017  Roland Gruber

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
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array (
			"cn" => _("Common name"),
			'company' => _('Company'),
			'departmentNumber' => _('Department'),
			'displayName' => _('Display name'),
			'employeeNumber' => _('Employee number'),
			"gecos" => _("Description"),
			"gidnumber" => _("GID number"),
			"givenname" => _("First name"),
			"homedirectory" => _("Home directory"),
			"host" => _("Allowed hosts"),
			"jpegphoto" => _('Photo'),
			"loginshell" => _("Login shell"),
			"mail" => _("Email"),
			'manager' => _('Manager'),
			'o' => _('Organisation'),
			'ou' => _('Organisational unit'),
			'proxyAddresses' => _('Proxy-Addresses'),
			'sambakickofftime' => _('Account expiration date'),
			'shadowexpire' => _('Password expiration'),
			"sn" => _("Last name"),
			'streetAddress' => _('Street'),
			'telephoneNumber' => _('Telephone number'),
			'title' => _('Job title'),
			"uid" => _("User name"),
			"uidnumber" => _("UID number"),
			'userPrincipalName' => _('User name'),
		);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
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
			return htmlspecialchars($personalAttributes['givenName'][0] . ' ' . $personalAttributes['sn'][0]);
		}
		// check if a display name is set
		if (($sambaAttributes != null) && isset($sambaAttributes['displayName'][0]) && !empty($sambaAttributes['displayName'][0])) {
			return htmlspecialchars($sambaAttributes['displayName'][0]);
		}
		// check if a common name is set
		if (($personalAttributes != null) && isset($personalAttributes['cn'][0]) && !empty($personalAttributes['cn'][0])) {
			return htmlspecialchars($personalAttributes['cn'][0]);
		}
		if (($unixAttributes != null) && isset($unixAttributes['cn'][0]) && !empty($unixAttributes['cn'][0])) {
			return htmlspecialchars($unixAttributes['cn'][0]);
		}
		// check if a user name is set
		if (($unixAttributes != null) && isset($unixAttributes['uid'][0]) && !empty($unixAttributes['uid'][0])) {
			return htmlspecialchars($unixAttributes['uid'][0]);
		}
		if (($personalAttributes != null) && isset($personalAttributes['uid'][0]) && !empty($personalAttributes['uid'][0])) {
			return htmlspecialchars($personalAttributes['uid'][0]);
		}
		if (($accountAttributes != null) && isset($accountAttributes['uid'][0]) && !empty($accountAttributes['uid'][0])) {
			return htmlspecialchars($accountAttributes['uid'][0]);
		}
		if (($mitKerberosAttributes != null) && isset($mitKerberosAttributes['krbPrincipalName'][0]) && !empty($mitKerberosAttributes['krbPrincipalName'][0])) {
			return htmlspecialchars($mitKerberosAttributes['krbPrincipalName'][0]);
		}
		if ($container->isNewAccount) {
			return _("New user");
		}
		// fall back to default
		return parent::getTitleBarTitle($container);
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
			return $this->buildAccountStatusIcon($container);
		}
		$subtitle = $this->buildAccountStatusIcon($container);
		$spacer = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
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
	 * @return String HTML code for icon
	 */
	private function buildAccountStatusIcon($container) {
		$modules = $this->getType()->getModules();
		// check if there are account parts that can be locked
		$unixAvailable = ($container->getAccountModule('posixAccount') != null) && $container->getAccountModule('posixAccount')->isLockable($modules);
		$sambaAvailable = (($container->getAccountModule('sambaSamAccount') != null) && $container->getAccountModule('sambaSamAccount')->isExtensionEnabled());
		$ppolicyAvailable = ($container->getAccountModule('ppolicyUser') != null);
		$windowsAvailable = ($container->getAccountModule('windowsUser') != null);
		$is389dsAvailable = ($container->getAccountModule('locking389ds') != null);
		$is389dsLocked = $is389dsAvailable && $container->getAccountModule('locking389ds')->isLocked();
		$is389dsDeactivated = $is389dsAvailable && $container->getAccountModule('locking389ds')->isDeactivated();
		if (!$unixAvailable && !$sambaAvailable && !$ppolicyAvailable && !$windowsAvailable && !$is389dsAvailable) {
			return '';
		}
		$isEditable = checkIfWriteAccessIsAllowed('user') && ($unixAvailable  || $sambaAvailable || $ppolicyAvailable || $windowsAvailable || $is389dsAvailable);
		// get locking status
		$unixLocked = false;
		if ($unixAvailable && $container->getAccountModule('posixAccount')->isLocked($modules)) {
			$unixLocked = true;
		}
		$sambaLocked = false;
		if ($sambaAvailable && $container->getAccountModule('sambaSamAccount')->isDeactivated()) {
			$sambaLocked = true;
		}
		$ppolicyLocked = false;
		if ($ppolicyAvailable && $container->getAccountModule('ppolicyUser')->isLocked()) {
			$ppolicyLocked = true;
		}
		$windowsLocked = false;
		$windowsPasswordLockedTime = null;
		$windowsPasswordLocked = false;
		if ($windowsAvailable){
			$attrs = $container->getAccountModule('windowsUser')->getAttributes();
			$attrs['dn'] = $container->dn_orig;
			if (windowsUser::isDeactivated($attrs)) {
				$windowsLocked = true;
			}
			$windowsPasswordLockedTime = windowsUser::getPasswordLocked($attrs, $this->getType());
			if ($windowsPasswordLockedTime != null) {
				$windowsPasswordLocked = true;
			}
		}
		$partiallyLocked = $unixLocked || $sambaLocked
						|| $ppolicyLocked || $windowsLocked || $windowsPasswordLocked
						|| $is389dsDeactivated || $is389dsLocked;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable || $windowsAvailable || $is389dsDeactivated || $is389dsLocked)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked)
							&& (!$windowsAvailable || $windowsLocked);
		// build tooltip
		$icon = 'unlocked.png';
		if ($fullyLocked) {
			$icon = 'lock.png';
		}
		elseif ($partiallyLocked) {
			$icon = 'partiallyLocked.png';
		}
		$statusTable = '<table border=0>';
		// Unix
		if ($unixAvailable) {
			$unixIcon = 'unlocked.png';
			if ($unixLocked) {
				$unixIcon = 'lock.png';
			}
			$statusTable .= '<tr><td>' . _('Unix') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $unixIcon . '&quot;></td></tr>';
		}
		// Samba
		if ($sambaAvailable) {
			$sambaIcon = 'unlocked.png';
			if ($sambaLocked) {
				$sambaIcon = 'lock.png';
			}
			$statusTable .= '<tr><td>' . _('Samba 3') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $sambaIcon . '&quot;></td></tr>';
		}
		// PPolicy
		if ($ppolicyAvailable) {
			$ppolicyIcon = 'unlocked.png';
			if ($ppolicyLocked) {
				$ppolicyIcon = 'lock.png';
			}
			$statusTable .= '<tr><td>' . _('Password policy') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $ppolicyIcon . '&quot;></td></tr>';
		}
		// Windows
		if ($windowsAvailable) {
			$windowsIcon = 'unlocked.png';
			if ($windowsLocked) {
				$windowsIcon = 'lock.png';
			}
			$statusTable .= '<tr><td>' . _('Windows') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $windowsIcon . '&quot;></td></tr>';
		}
		if ($windowsAvailable && $windowsPasswordLocked) {
			$statusTable .= '<tr><td>' . _('Locked till') . '&nbsp;&nbsp;</td><td>' . $windowsPasswordLockedTime->format('Y-m-d H:i:s') . '</td></tr>';
		}
		// 389ds locked
		if ($is389dsLocked) {
			$statusTable .= '<tr><td>' . _('Locked') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/lock.png&quot;></td></tr>';
		}
		// 389ds deactivated
		if ($is389dsAvailable) {
			$text389dsActivation = $is389dsDeactivated ? _('Deactivated') : _('Active');
			$icon389dsActivation = $is389dsDeactivated ? 'lock.png' : 'unlocked.png';
			$statusTable .= '<tr><td>' . $text389dsActivation . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $icon389dsActivation . '&quot;></td></tr>';
		}
		$statusTable .= '</table>';
		$tipContent = $statusTable;
		if ($isEditable) {
			$tipContent .= '<br><img alt=&quot;hint&quot; src=&quot;../../graphics/light.png&quot;> ';
			$tipContent .= _('Please click to lock/unlock this account.');
		}
		$dialogDiv = $this->buildAccountStatusDialogDiv($unixAvailable, $unixLocked, $sambaAvailable, $sambaLocked, $ppolicyAvailable, $ppolicyLocked, $windowsAvailable, $windowsLocked, $windowsPasswordLockedTime, $is389dsAvailable, $is389dsLocked, $is389dsDeactivated);
		$onClick = '';
		if ($isEditable) {
			$onClick = 'onclick="showConfirmationDialog(\'' . _('Change account status') . '\', \'' . _('Ok') . '\', \'' . _('Cancel') . '\', \'lam_accountStatusDialog\', \'inputForm\', \'lam_accountStatusResult\');"';
		}
		return $dialogDiv . '<a href="#"><img id="lam_accountStatus" alt="status" ' . $onClick . ' helptitle="' . _('Account status') . '" helpdata="' . $tipContent . '" height=16 width=16 src="../../graphics/' . $icon . '"></a>&nbsp;&nbsp;&nbsp;';
	}

	/**
	 * Builds the dialog to (un)lock parts of an account.
	 *
	 * @param boolean $unixAvailable Unix part is active
	 * @param boolean $unixLocked Unix part is locked
	 * @param boolean $sambaAvailable Samba part is active
	 * @param boolean $sambaLocked Samba part is locked
	 * @param boolean $ppolicyAvailable PPolicy part is active
	 * @param boolean $ppolicyLocked PPolicy part is locked
	 * @param boolean $windowsAvailable Windows part is active
	 * @param boolean $windowsLocked Windows part is locked
	 * @param DateTime $windowsPasswordLockedTime lock time for Windows or null
	 * @param boolean $is389dsAvailable 389ds is available
	 * @param boolean $is389dsLocked account is locked
	 * @param boolean $is389dsDeactivated account is deactivated
	 */
	private function buildAccountStatusDialogDiv($unixAvailable, $unixLocked, $sambaAvailable, $sambaLocked, $ppolicyAvailable, $ppolicyLocked, $windowsAvailable,
			$windowsLocked, $windowsPasswordLockedTime, $is389dsAvailable, $is389dsLocked, $is389dsDeactivated) {
		$windowsPasswordLocked = ($windowsPasswordLockedTime != null);
		$partiallyLocked = $unixLocked || $sambaLocked || $ppolicyLocked || $windowsLocked || $windowsPasswordLocked || $is389dsLocked || $is389dsDeactivated;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable || $windowsAvailable || $is389dsLocked || $is389dsDeactivated)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked)
							&& (!$windowsAvailable || $windowsLocked || $windowsPasswordLocked);

		$container = new htmlTable();

		// show radio buttons for lock/unlock
		$radioDisabled = true;
		$selectedRadio = 'unlock';
		$onchange = '';
		if ($partiallyLocked && !$fullyLocked) {
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
		if (!$fullyLocked && !$partiallyLocked) {
			$selectedRadio = 'lock';
		}
		if (!$radioDisabled) {
			$radio = new htmlRadio('lam_accountStatusAction', array(_('Lock') => 'lock', _('Unlock') => 'unlock'), $selectedRadio);
			$radio->setOnchangeEvent($onchange);
			$container->addElement($radio, true);
		}
		else {
			$radio = new htmlRadio('lam_accountStatusActionDisabled', array(_('Lock') => 'lock', _('Unlock') => 'unlock'), $selectedRadio);
			$radio->setIsEnabled(false);
			$container->addElement($radio, true);
			$container->addElement(new htmlHiddenInput('lam_accountStatusAction', $selectedRadio), true);
		}

		$container->addElement(new htmlHiddenInput('lam_accountStatusResult', 'cancel'), true);

		// locking part
		if (!$fullyLocked) {
			$lockContent = new htmlTable();

			if ($unixAvailable && !$unixLocked) {
				$lockContent->addElement(new htmlImage('../../graphics/tux.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusLockUnix', true, _('Unix'), null, false), true);
			}
			if ($sambaAvailable && !$sambaLocked) {
				$lockContent->addElement(new htmlImage('../../graphics/samba.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusLockSamba', true, _('Samba 3'), null, false), true);
			}
			if ($ppolicyAvailable && !$ppolicyLocked) {
				$lockContent->addElement(new htmlImage('../../graphics/security.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusLockPPolicy', true, _('PPolicy'), null, false), true);
			}
			if ($is389dsAvailable && !$is389dsDeactivated) {
				$lockContent->addElement(new htmlImage('../../graphics/security.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusDeactivate389ds', true, _('Deactivate'), null, false), true);
			}
			if ($windowsAvailable && !$windowsLocked) {
				$lockContent->addElement(new htmlImage('../../graphics/samba.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusLockWindows', true, _('Windows'), null, false), true);
			}
			if ($unixAvailable) {
				$lockContent->addElement(new htmlImage('../../graphics/groupBig.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusRemoveUnixGroups', true, _('Remove from all Unix groups'), null, false), true);
			}
			if ($unixAvailable && posixAccount::areGroupOfNamesActive()) { // check unixAvailable because Unix module removes group memberships
				$lockContent->addElement(new htmlImage('../../graphics/groupBig.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusRemoveGONGroups', true, _('Remove from all group of (unique) names'), null, false), true);
			}

			$lockDiv = new htmlDiv('lam_accountStatusDialogLockDiv', $lockContent);
			if ($fullyLocked || $partiallyLocked) {
				$lockDiv->setCSSClasses(array('hidden'));
			}
			$container->addElement($lockDiv, true);
		}
		// unlocking part
		if ($partiallyLocked) {
			$unlockContent = new htmlTable();

			if ($unixAvailable && $unixLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/tux.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlockUnix', true, _('Unix'), null, false), true);
			}
			if ($sambaAvailable && $sambaLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/samba.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlockSamba', true, _('Samba 3'), null, false), true);
			}
			if ($ppolicyAvailable && $ppolicyLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/security.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlockPPolicy', true, _('PPolicy'), null, false), true);
			}
			if ($is389dsAvailable && $is389dsDeactivated) {
				$unlockContent->addElement(new htmlImage('../../graphics/security.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusActivate389ds', true, _('Activate'), null, false), true);
			}
			if ($windowsAvailable && $windowsLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/samba.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlockWindows', true, _('Windows'), null, false), true);
			}
			if ($windowsAvailable && $windowsPasswordLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/samba.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlockWindowsPassword', true, _('Locked till') . ' ' . $windowsPasswordLockedTime->format('Y-m-d H:i:s'), null, false), true);
			}
			if ($is389dsLocked) {
				$unlockContent->addElement(new htmlImage('../../graphics/security.png'));
				$unlockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusUnlock389ds', true, _('Unlock'), null, false), true);
			}

			$unlockDiv = new htmlDiv('lam_accountStatusDialogUnlockDiv', $unlockContent);
			if (!$fullyLocked && !$partiallyLocked) {
				$unlockDiv->setCSSClasses(array('hidden'));
			}
			$container->addElement($unlockDiv, true);
		}

		$div = new htmlDiv('lam_accountStatusDialog', $container);
		$div->setCSSClasses(array('hidden'));

		$tabindex = 999;
		ob_start();
		parseHtml(null, $div, array(), false, $tabindex, 'user');
		$output = ob_get_contents();
		ob_clean();

		return $output;
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
				// Unix
				if (isset($_POST['lam_accountStatusLockUnix']) && ($_POST['lam_accountStatusLockUnix'] == 'on')) {
					$container->getAccountModule('posixAccount')->lock($modules);
				}
				// Samba
				if (isset($_POST['lam_accountStatusLockSamba']) && ($_POST['lam_accountStatusLockSamba'] == 'on')) {
					$container->getAccountModule('sambaSamAccount')->deactivate();
				}
				// PPolicy
				if (isset($_POST['lam_accountStatusLockPPolicy']) && ($_POST['lam_accountStatusLockPPolicy'] == 'on')) {
					$container->getAccountModule('ppolicyUser')->lock();
				}
				// 389ds
				if (isset($_POST['lam_accountStatusDeactivate389ds']) && ($_POST['lam_accountStatusDeactivate389ds'] == 'on')) {
					$container->getAccountModule('locking389ds')->deactivate();
				}
				// Windows
				if (isset($_POST['lam_accountStatusLockWindows']) && ($_POST['lam_accountStatusLockWindows'] == 'on')) {
					$container->getAccountModule('windowsUser')->setIsDeactivated(true);
				}
				// remove Unix groups
				if (isset($_POST['lam_accountStatusRemoveUnixGroups']) && ($_POST['lam_accountStatusRemoveUnixGroups'] == 'on')) {
					$container->getAccountModule('posixAccount')->removeFromUnixGroups();
				}
				// remove group of names memberships
				if (isset($_POST['lam_accountStatusRemoveGONGroups']) && ($_POST['lam_accountStatusRemoveGONGroups'] == 'on')) {
					$container->getAccountModule('posixAccount')->removeFromGONGroups();
				}
			}
			// unlock account
			elseif ($_POST['lam_accountStatusAction'] == 'unlock') {
				// Unix
				if (isset($_POST['lam_accountStatusUnlockUnix']) && ($_POST['lam_accountStatusUnlockUnix'] == 'on')) {
					$container->getAccountModule('posixAccount')->unlock($modules);
				}
				// Samba
				if (isset($_POST['lam_accountStatusUnlockSamba']) && ($_POST['lam_accountStatusUnlockSamba'] == 'on')) {
					$container->getAccountModule('sambaSamAccount')->activate();
				}
				// PPolicy
				if (isset($_POST['lam_accountStatusUnlockPPolicy']) && ($_POST['lam_accountStatusUnlockPPolicy'] == 'on')) {
					$container->getAccountModule('ppolicyUser')->unlock();
				}
				// 389ds
				if (isset($_POST['lam_accountStatusActivate389ds']) && ($_POST['lam_accountStatusActivate389ds'] == 'on')) {
					$container->getAccountModule('locking389ds')->activate();
				}
				// Windows
				if (isset($_POST['lam_accountStatusUnlockWindows']) && ($_POST['lam_accountStatusUnlockWindows'] == 'on')) {
					$container->getAccountModule('windowsUser')->setIsDeactivated(false);
				}
				// Windows password
				if (isset($_POST['lam_accountStatusUnlockWindowsPassword']) && ($_POST['lam_accountStatusUnlockWindowsPassword'] == 'on')) {
					$container->getAccountModule('windowsUser')->unlockPassword();
				}
				// 389ds unlocking
				if (isset($_POST['lam_accountStatusUnlock389ds']) && ($_POST['lam_accountStatusUnlock389ds'] == 'on')) {
					$container->getAccountModule('locking389ds')->unlock(false);
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
			$this->injectAccountStatusAttributeAndFilterByStatus();
		}
	}

	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation() {
		$this->trans_primary_hash = array();
		$filter = "objectClass=posixGroup";
		$attrs = array("cn", "gidNumber");
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, array('group'));
		$entryCount = sizeof($entries);
		for ($i = 0; $i < $entryCount; $i++) {
			$this->trans_primary_hash[$entries[$i]['gidnumber'][0]] = $entries[$i]['cn'][0];
		}
	}

	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	protected function listPrintTableCellContent(&$entry, &$attribute) {
		// check if there is something to display at all
		if (($attribute != self::ATTR_ACCOUNT_STATUS) && (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1))) {
			parent::listPrintTableCellContent($entry, $attribute);
			return;
		}
		// translate GID to group name
		if (($attribute == "gidnumber") && ($this->trans_primary == "on")) {
			if (isset($this->trans_primary_hash[$entry[$attribute][0]])) {
				echo $this->trans_primary_hash[$entry[$attribute][0]];
			}
			else {
				parent::listPrintTableCellContent($entry, $attribute);
			}
		}
		// show user photos
		elseif (($attribute == "jpegphoto") && (sizeof($entry[$attribute][0]) > 0)) {
			if (sizeof($entry[$attribute][0]) < 100) {
				// looks like we have read broken binary data, reread photo
				$result = @ldap_read($_SESSION['ldap']->server(), escapeDN($entry['dn']), $attribute . "=*", array($attribute), 0, 0, 0, LDAP_DEREF_NEVER);
				if ($result) {
					$tempEntry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($tempEntry) {
						$binData = ldap_get_values_len($_SESSION['ldap']->server(), $tempEntry, $attribute);
						$entry[$attribute] = $binData;
					}
				}
			}
			$imgNumber = getRandomNumber();
			$jpeg_filename = 'jpg' . $imgNumber . '.jpg';
			$outjpeg = @fopen(dirname(__FILE__) . '/../../tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $entry[$attribute][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			$imgSize = getimagesize($photoFile);
			$minSize = 64;
			if ($imgSize[0] < 64) {
				$minSize = $imgSize[0];
			}
			$imgTitle = _('Click to switch between thumbnail and original size.');
			echo "<img id=\"img$imgNumber\" title=\"$imgTitle\" height=$minSize src=\"" . $photoFile . "\" alt=\"" . _('Photo') . "\">";
			echo '<script type="text/javascript">';
			echo "addResizeHandler(document.getElementById(\"img$imgNumber\"), $minSize, " . $imgSize[1] . ")";
			echo '</script>';
		}
		elseif (($attribute == 'mail') || ($attribute == 'rfc822Mailbox')) {
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				for ($i = 0; $i < sizeof($entry[$attribute]); $i++) {
					if ($i > 0) {
						echo ", ";
					}
					echo "<a href=\"mailto:" . $entry[$attribute][$i] . "\">" . $entry[$attribute][$i] . "</a>\n";
				}
			}
		}
		// expire dates
		elseif ($attribute == 'shadowexpire') {
			if (!empty($entry[$attribute][0])) {
				$time = new DateTime('@' . $entry[$attribute][0] * 24 * 3600, getTimeZone());
				echo $time->format('d.m.Y');
			}
		}
		elseif ($attribute == 'sambakickofftime') {
			if (!empty($entry[$attribute][0])) {
				if ($entry[$attribute][0] > 2147483648) {
					echo "∞";
				}
				else {
					$date = new DateTime('@' . $entry[$attribute][0], new DateTimeZone('UTC'));
					echo $date->format('d.m.Y');
				}
			}
		}
		// account status
		elseif ($attribute == self::ATTR_ACCOUNT_STATUS) {
			$this->printAccountStatus($entry);
		}
		// print all other attributes
		else {
			parent::listPrintTableCellContent($entry, $attribute);
		}
	}

	/**
	 * Returns a list of lamListTool objects to display next to the edit/delete buttons.
	 *
	 * @return lamListTool[] tools
	 */
	protected function getAdditionalTools() {
		if (!isLAMProVersion()) {
			return array();
		}
		if (checkIfWriteAccessIsAllowed('user') || (checkIfPasswordChangeIsAllowed() && !checkIfWriteAccessIsAllowed())) {
			$passwordTool = new lamListTool(_('Change password'), 'key.png', 'changePassword.php');
			return array($passwordTool);
		}
		return array();
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
	 * Prints the content of a single attribute filter area.
	 *
	 * @param String $attrName attribute name
	 * @param boolean $clearFilter true if filter value should be cleared
	 */
	protected function printFilterArea($attrName, $clearFilter) {
		if ($attrName != self::ATTR_ACCOUNT_STATUS) {
			parent::printFilterArea($attrName, $clearFilter);
			return;
		}
		$value = "-";
		if (!$clearFilter) {
			if (isset($this->filters[strtolower($attrName)])) {
				$value = $this->filters[strtolower($attrName)];
			}
		}
		$filterOptions = array(
			'' => '',
			_('Unlocked') => self::FILTER_UNLOCKED,
			_('Partially locked') => self::FILTER_SEMILOCKED,
			_('Locked') => self::FILTER_LOCKED
		);
		$filterInput = new htmlSelect('filter' . strtolower($attrName), $filterOptions, array($value));
		$filterInput->setCSSClasses(array($this->type->getScope() . '-dark'));
		$filterInput->setHasDescriptiveElements(true);
		$filterInput->setOnchangeEvent('document.getElementsByName(\'apply_filter\')[0].click();');
		parseHtml(null, $filterInput, array(), false, $this->tabindex, $this->type->getScope());
	}

	/**
	 * Builds the LDAP filter based on the filter entries in the GUI.
	 *
	 * @return String LDAP filter
	 */
	protected function buildLDAPAttributeFilter() {
		$this->accountStatusFilter = null;
		$text = '';
		foreach ($this->filters as $attr => $filter) {
			if ($attr == self::ATTR_ACCOUNT_STATUS) {
				$this->accountStatusFilter = $filter;
				continue;
			}
			$text .= '(' . $attr . '=' . $filter . ')';
		}
		return $text;
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
			$attrs[] = 'pwdAccountLockedTime';
			$attrs[] = 'sambaAcctFlags';
			$attrs[] = 'userPassword';
			$attrs[] = 'userAccountControl';
			$attrs[] = 'lockoutTime';
			$attrs[] = 'nsAccountLock';
			$attrs[] = 'accountUnlockTime';
			$attrs[] = 'objectClass';
		}
		return $attrs;
	}

	/**
	 * Injects values for the virtual account status attribute to make it sortable.
	 */
	private function injectAccountStatusAttributeAndFilterByStatus() {
		$entryCount = sizeof($this->entries);
		for ($i = 0; $i < $entryCount; $i++) {
			$unixAvailable = self::isUnixAvailable($this->entries[$i]);
			$sambaAvailable = self::isSambaAvailable($this->entries[$i]);
			$ppolicyAvailable = $this->isPPolicyAvailable($this->entries[$i]);
			$windowsAvailable = self::isWindowsAvailable($this->entries[$i]);
			$unixLocked = self::isUnixLocked($this->entries[$i]);
			$sambaLocked = self::isSambaLocked($this->entries[$i]);
			$ppolicyLocked = self::isPPolicyLocked($this->entries[$i]);
			$windowsLocked = self::isWindowsLocked($this->entries[$i]);
			$windowsPasswordLocked = ($this->getWindowsPasswordLockedTime($this->entries[$i]) != null);
			$is389dsLocked = self::is389dsLocked($this->entries[$i]);
			$is389dsDeactivated = self::is389dsDeactivated($this->entries[$i]);
			$hasLocked = ($unixAvailable && $unixLocked)
							|| ($sambaAvailable && $sambaLocked)
							|| ($ppolicyAvailable && $ppolicyLocked)
							|| ($windowsAvailable && ($windowsLocked || $windowsPasswordLocked))
							|| $is389dsDeactivated
							|| $is389dsLocked;
			$hasUnlocked = ($unixAvailable && !$unixLocked)
							|| ($sambaAvailable && !$sambaLocked)
							|| ($ppolicyAvailable && !$ppolicyLocked)
							|| ($windowsAvailable && !$windowsLocked);
			$status = self::FILTER_UNLOCKED;
			if ($hasLocked && $hasUnlocked) {
				$status = self::FILTER_SEMILOCKED;
			}
			elseif (!$hasUnlocked && $hasLocked) {
				$status = self::FILTER_LOCKED;
			}
			// filter accounts
			if (!empty($this->accountStatusFilter)) {
				if ($status != $this->accountStatusFilter) {
					unset($this->entries[$i]);
					continue;
				}
			}
			// add virtual attribute
			$this->entries[$i][self::ATTR_ACCOUNT_STATUS][0] = $status;
		}
		$this->entries = array_values($this->entries);
	}

	/**
	 * Prints the account status.
	 *
	 * @param array $attrs LDAP attributes
	 */
	private function printAccountStatus(&$attrs) {
		// check status
		$unixAvailable = self::isUnixAvailable($attrs);
		$sambaAvailable = self::isSambaAvailable($attrs);
		$ppolicyAvailable = $this->isPPolicyAvailable($attrs);
		$windowsAvailable = self::isWindowsAvailable($attrs);
		$unixLocked = self::isUnixLocked($attrs);
		$sambaLocked = self::isSambaLocked($attrs);
		$ppolicyLocked = self::isPPolicyLocked($attrs);
		$windowsLocked = self::isWindowsLocked($attrs);
		$windowsPasswordLockedTime = $this->getWindowsPasswordLockedTime($attrs);
		$windowsPasswordLocked = ($windowsPasswordLockedTime != null);
		$is389dsDeactivated = self::is389dsDeactivated($attrs);
		$is389dsLocked = self::is389dsLocked($attrs);
		$partiallyLocked = $unixLocked || $sambaLocked
					|| $ppolicyLocked || $windowsLocked || $windowsPasswordLocked
					|| $is389dsDeactivated || $is389dsLocked;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable || $windowsAvailable || $is389dsDeactivated || $is389dsLocked)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked)
							&& (!$windowsAvailable || $windowsLocked);
		$icon = 'unlocked.png';
		if ($fullyLocked) {
			$icon = 'lock.png';
		}
		elseif ($partiallyLocked) {
			$icon = 'partiallyLocked.png';
		}
		// print icon and detail tooltips
		if ($unixAvailable || $sambaAvailable || $ppolicyAvailable || $windowsAvailable || $is389dsDeactivated) {
			$tipContent = '<table border=0>';
			// Unix
			if ($unixAvailable) {
				$unixIcon = 'unlocked.png';
				if ($unixLocked) {
					$unixIcon = 'lock.png';
				}
				$tipContent .= '<tr><td>' . _('Unix') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $unixIcon . '&quot;></td></tr>';
			}
			// Samba
			if ($sambaAvailable) {
				$sambaIcon = 'unlocked.png';
				if ($sambaLocked) {
					$sambaIcon = 'lock.png';
				}
				$tipContent .= '<tr><td>' . _('Samba 3') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $sambaIcon . '&quot;></td></tr>';
			}
			// PPolicy
			if ($ppolicyAvailable) {
				$ppolicyIcon = 'unlocked.png';
				if ($ppolicyLocked) {
					$ppolicyIcon = 'lock.png';
				}
				$tipContent .= '<tr><td>' . _('Password policy') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $ppolicyIcon . '&quot;></td></tr>';
			}
			// Windows
			if ($windowsAvailable) {
				$windowsIcon = 'unlocked.png';
				if ($windowsLocked) {
					$windowsIcon = 'lock.png';
				}
				$tipContent .= '<tr><td>' . _('Windows') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/' . $windowsIcon . '&quot;></td></tr>';
			}
			if ($windowsAvailable && $windowsPasswordLocked) {
				$tipContent .= '<tr><td>' . _('Locked till') . '&nbsp;&nbsp;</td><td>' . $windowsPasswordLockedTime->format('Y-m-d H:i:s') . '</td></tr>';
			}
			// 389 locked
			if ($is389dsLocked) {
				$tipContent .= '<tr><td>' . _('Locked') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/lock.png&quot;></td></tr>';
			}
			// 389 deactivation
			if ($is389dsDeactivated) {
				$tipContent .= '<tr><td>' . _('Deactivated') . '&nbsp;&nbsp;</td><td><img height=16 width=16 src=&quot;../../graphics/lock.png&quot;></td></tr>';
			}
			$tipContent .= '</table>';
			echo '<img helptitle="' . _('Account status') . '" helpdata="' . $tipContent . '" alt="status" height=16 width=16 src="../../graphics/' . $icon . '">';
		}
		else {
			echo '<img alt="status" height=16 width=16 src="../../graphics/' . $icon . '">';
		}
	}

	/**
	 * Returns if the Unix part exists.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Unix part exists
	 */
	public static function isUnixAvailable(&$attrs) {
		return (isset($attrs['objectclass']) && in_array_ignore_case('posixAccount', $attrs['objectclass']) && isset($attrs['userpassword'][0]));
	}

	/**
	 * Returns if the Unix part is locked.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Unix part locked
	 */
	public static function isUnixLocked(&$attrs) {
		return ((isset($attrs['userpassword'][0]) && !pwd_is_enabled($attrs['userpassword'][0])));
	}

	/**
	 * Returns if the Samba part exists.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Samba part exists
	 */
	public static function isSambaAvailable(&$attrs) {
		return (isset($attrs['objectclass']) && in_array_ignore_case('sambaSamAccount', $attrs['objectclass']));
	}

	/**
	 * Returns if the Samba part is locked.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Samba part is locked
	 */
	public static function isSambaLocked(&$attrs) {
		return (isset($attrs['sambaacctflags'][0]) && strpos($attrs['sambaacctflags'][0], "D"));
	}

	/**
	 * Returns if the PPolicy part exists.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean PPolicy part exists
	 */
	public function isPPolicyAvailable(&$attrs) {
		return in_array('ppolicyUser', $this->type->getModules());
	}

	/**
	 * Returns if the PPolicy part is locked.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean PPolicy part is locked
	 */
	public static function isPPolicyLocked(&$attrs) {
		return (isset($attrs['pwdaccountlockedtime'][0]) && ($attrs['pwdaccountlockedtime'][0] != ''));
	}

	/**
	 * Returns if the Windows part exists.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Windows part exists
	 */
	public static function isWindowsAvailable(&$attrs) {
		return (isset($attrs['objectclass']) && in_array_ignore_case('user', $attrs['objectclass']) && isset($attrs['useraccountcontrol'][0]));
	}

	/**
	 * Returns if the Windows part is locked.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean Windows part is locked
	 */
	public static function isWindowsLocked(&$attrs) {
		return windowsUser::isDeactivated($attrs);
	}

	/**
	 * Returns if the Windows password is locked.
	 *
	 * @param array $attrs LDAP attributes
	 * @return DateTime Windows password lock time or null
	 */
	public function getWindowsPasswordLockedTime(&$attrs) {
		return windowsUser::getPasswordLocked($attrs, $this->type);
	}

	/**
	 * Returns if the 389ds is activated.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean 389ds available
	 */
	public function is389dsAvailable(&$attrs) {
		return in_array('locking389ds', $this->type->getModules());
	}

	/**
	 * Returns if deactivated by nsAccountLock.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean account is deactivated
	 */
	public static function is389dsDeactivated(&$attrs) {
		return (isset($attrs['nsaccountlock'][0]) && ($attrs['nsaccountlock'][0] == 'true'));
	}

	/**
	 * Returns if locked by accountUnlockTime.
	 *
	 * @param array $attrs LDAP attributes
	 * @return boolean account is locked
	 */
	public static function is389dsLocked(&$attrs) {
		return (isset($attrs['accountunlocktime'][0]) && !empty($attrs['accountunlocktime'][0]));
	}

}

?>