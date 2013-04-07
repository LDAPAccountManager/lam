<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
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
	 */
	public function __construct() {
		parent::__construct();
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
			"uid" => _("User ID"),
			"uidnumber" => _("UID number"),
			"gidnumber" => _("GID number"),
			"cn" => _("User name"),
			"host" => _("Allowed hosts"),
			"givenname" => _("First name"),
			"sn" => _("Last name"),
			"homedirectory" => _("Home directory"),
			"loginshell" => _("Login shell"),
			"mail" => _("Email"),
			"gecos" => _("Description"),
			"jpegphoto" => _('Photo'),
			'shadowexpire' => _('Password expiration'),
			'sambakickofftime' => _('Account expiration date')
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
		if (($personalAttributes != null) && isset($personalAttributes['sn'][0]) && isset($personalAttributes['givenName'][0])) {
			return htmlspecialchars($personalAttributes['givenName'][0] . ' ' . $personalAttributes['sn'][0]);
		}
		// check if a display name is set
		if (($sambaAttributes != null) && isset($sambaAttributes['displayName'][0])) {
			return htmlspecialchars($sambaAttributes['displayName'][0]);
		}
		// check if a common name is set
		if (($personalAttributes != null) && isset($personalAttributes['cn'][0])) {
			return htmlspecialchars($personalAttributes['cn'][0]);
		}
		if (($unixAttributes != null) && isset($unixAttributes['cn'][0])) {
			return htmlspecialchars($unixAttributes['cn'][0]);
		}
		// check if a user name is set
		if (($unixAttributes != null) && isset($unixAttributes['uid'][0])) {
			return htmlspecialchars($unixAttributes['uid'][0]);
		}
		if (($personalAttributes != null) && isset($personalAttributes['uid'][0])) {
			return htmlspecialchars($personalAttributes['uid'][0]);
		}
		if (($accountAttributes != null) && isset($accountAttributes['uid'][0])) {
			return htmlspecialchars($accountAttributes['uid'][0]);
		}
		if (($mitKerberosAttributes != null) && isset($mitKerberosAttributes['krbPrincipalName'][0])) {
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
		if (isset($personalAttributes['mail'][0])) {
			$subtitle .= '<a href="mailto:' . htmlspecialchars($personalAttributes['mail'][0]) . '">' . htmlspecialchars($personalAttributes['mail'][0]) . '</a>' . $spacer;
		}
		// check if an telephone number can be shown
		if (isset($personalAttributes['telephoneNumber'][0])) {
			$subtitle .= _('Telephone number') . ' ' . htmlspecialchars($personalAttributes['telephoneNumber'][0]) . $spacer;
		}
		// check if an mobile number can be shown
		if (isset($personalAttributes['mobile'][0])) {
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
		// check if there are account parts that can be locked
		$unixAvailable = ($container->getAccountModule('posixAccount') != null) && $container->getAccountModule('posixAccount')->isLockable();
		$sambaAvailable = (($container->getAccountModule('sambaSamAccount') != null) && $container->getAccountModule('sambaSamAccount')->isExtensionEnabled());
		$ppolicyAvailable = ($container->getAccountModule('ppolicyUser') != null);
		if (!$unixAvailable && !$sambaAvailable && !$ppolicyAvailable) {
			return '';
		}
		// get locking status
		$unixLocked = false;
		if ($unixAvailable && $container->getAccountModule('posixAccount')->isLocked()) {
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
		$partiallyLocked = $unixLocked || $sambaLocked || $ppolicyLocked;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked);
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
		$statusTable .= '</table>';
		$tipContent = $statusTable;
		if (checkIfWriteAccessIsAllowed()) {
			$tipContent .= '<br><img alt=&quot;hint&quot; src=&quot;../../graphics/light.png&quot;> ';
			$tipContent .= _('Please click to lock/unlock this account.');
		}
		$tooltip = "'" . $tipContent . "', TITLE, '" . _('Account status') . "'";
		$dialogDiv = $this->buildAccountStatusDialogDiv($unixAvailable, $unixLocked, $sambaAvailable, $sambaLocked, $ppolicyAvailable, $ppolicyLocked);
		$onClick = '';
		if (checkIfWriteAccessIsAllowed()) {
			$onClick = 'onclick="showConfirmationDialog(\'' . _('Change account status') . '\', \'' . _('Ok') . '\', \'' . _('Cancel') . '\', \'lam_accountStatusDialog\', \'inputForm\', \'lam_accountStatusResult\');"';
		}
		return $dialogDiv . '<a href="#"><img id="lam_accountStatus" alt="status" ' . $onClick . ' onmouseout="UnTip()" onmouseover="Tip(' . $tooltip . ')" height=16 width=16 src="../../graphics/' . $icon . '"></a>&nbsp;&nbsp;&nbsp;';
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
	 */
	private function buildAccountStatusDialogDiv($unixAvailable, $unixLocked, $sambaAvailable, $sambaLocked, $ppolicyAvailable, $ppolicyLocked) {
		$partiallyLocked = $unixLocked || $sambaLocked || $ppolicyLocked;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked);
		
		$container = new htmlTable();
		
		// show radio buttons for lock/unlock
		$radioDisabled = true;
		$selectedRadio = 'lock';
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
		if ($fullyLocked) {
			$selectedRadio = 'unlock';
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
			if ($unixAvailable) {
				$lockContent->addElement(new htmlImage('../../graphics/groupBig.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusRemoveUnixGroups', true, _('Remove from all Unix groups'), null, false), true);
			}
			if ($unixAvailable && posixAccount::areGroupOfNamesActive()) { // check unixAvailable because Unix module removes group memberships
				$lockContent->addElement(new htmlImage('../../graphics/groupBig.png'));
				$lockContent->addElement(new htmlTableExtendedInputCheckbox('lam_accountStatusRemoveGONGroups', true, _('Remove from all group of (unique) names'), null, false), true);
			}

			$lockDiv = new htmlDiv('lam_accountStatusDialogLockDiv', $lockContent);
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

			$unlockDiv = new htmlDiv('lam_accountStatusDialogUnlockDiv', $unlockContent);
			if (!$fullyLocked) {
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
	public function runEditPagePostAction($container) {
		// check if account status should be changed
		if (isset($_POST['lam_accountStatusResult']) && ($_POST['lam_accountStatusResult'] == 'ok')) {
			// lock account
			if ($_POST['lam_accountStatusAction'] == 'lock') {
				// Unix
				if (isset($_POST['lam_accountStatusLockUnix']) && ($_POST['lam_accountStatusLockUnix'] == 'on')) {
					$container->getAccountModule('posixAccount')->lock();
				}
				// Samba
				if (isset($_POST['lam_accountStatusLockSamba']) && ($_POST['lam_accountStatusLockSamba'] == 'on')) {
					$container->getAccountModule('sambaSamAccount')->deactivate();
				}
				// PPolicy
				if (isset($_POST['lam_accountStatusLockPPolicy']) && ($_POST['lam_accountStatusLockPPolicy'] == 'on')) {
					$container->getAccountModule('ppolicyUser')->lock();
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
					$container->getAccountModule('posixAccount')->unlock();
				}
				// Samba
				if (isset($_POST['lam_accountStatusUnlockSamba']) && ($_POST['lam_accountStatusUnlockSamba'] == 'on')) {
					$container->getAccountModule('sambaSamAccount')->activate();
				}
				// PPolicy
				if (isset($_POST['lam_accountStatusUnlockPPolicy']) && ($_POST['lam_accountStatusUnlockPPolicy'] == 'on')) {
					$container->getAccountModule('ppolicyUser')->unlock();
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
	
	/** ID for config option to translate primary group GIDs to group names */
	const TRANS_PRIMARY_OPTION_NAME = "LU_TP";
	/** ID for config option to show account status */
	const ACCOUNT_STATUS_OPTION_NAME = "LU_AS";
	
	/** virtual attribute name for account status column */
	const ATTR_ACCOUNT_STATUS = 'lam_virtual_account_status';
	
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
		if ($this->trans_primary == "on") {
			$this->refreshPrimaryGroupTranslation();
		}
		if ($this->showAccountStatus) {
			$this->injectAccountStatusAttribute();
		}
	}
	
	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation() {
		$this->trans_primary_hash = array();
		$grp_suffix = $_SESSION['config']->get_Suffix('group');
		$filter = "objectClass=posixGroup";
		$attrs = array("cn", "gidNumber");
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, array('group'));
		for ($i = 0; $i < sizeof($entries); $i++) {
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
		elseif ($attribute == "jpegphoto") {
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
			$imgNumber = $_SESSION['ldap']->new_rand(); 
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
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				echo date('d. m. Y', $entry[$attribute][0] * 24 * 3600);
			}
		}
		elseif ($attribute == 'sambakickofftime') {
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				if ($entry[$attribute][0] > 2147483648) {
					echo "∞";
				}
				else {
					$date = getdate($entry[$attribute][0]);
					echo $date['mday'] . "." . $date['mon'] . "." . $date['year'];
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
		if (isLAMProVersion() && checkIfPasswordChangeIsAllowed()) {
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
		if ($attr == self::ATTR_ACCOUNT_STATUS) {
			return false;
		}
		elseif (strtolower($attr) == 'jpegphoto') {
			return false;
		}
		return true;
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
			$attrs[] = 'objectClass';
		}
		return $attrs;
	}
	
	/**
	 * Injects values for the virtual account status attribute to make it sortable.
	 */
	private function injectAccountStatusAttribute() {
		for ($i = 0; $i < sizeof($this->entries); $i++) {
			$status = 0;
			if (!$this->isUnixLocked($this->entries[$i])) {
				$status++;
			}
			if (!$this->isSambaLocked($this->entries[$i])) {
				$status++;
			}
			if (!$this->isPPolicyLocked($this->entries[$i])) {
				$status++;
			}
			$this->entries[$i][self::ATTR_ACCOUNT_STATUS][0] = $status;
		}
	}
	
	/**
	 * Prints the account status.
	 * 
	 * @param array $attrs LDAP attributes
	 */
	private function printAccountStatus(&$attrs) {
		// check status
		$unixAvailable = self::isUnixAvailable($attrs);
		$unixLocked = self::isUnixLocked($attrs);
		$sambaAvailable = self::isSambaAvailable($attrs);
		$sambaLocked = self::isSambaLocked($attrs);
		$ppolicyAvailable = self::isPPolicyAvailable($attrs);
		$ppolicyLocked = self::isPPolicyLocked($attrs);
		$partiallyLocked = $unixLocked || $sambaLocked || $ppolicyLocked;
		$fullyLocked = ($unixAvailable || $sambaAvailable || $ppolicyAvailable)
							&& (!$unixAvailable || $unixLocked)
							&& (!$sambaAvailable || $sambaLocked)
							&& (!$ppolicyAvailable || $ppolicyLocked);
		$icon = 'unlocked.png';
		if ($fullyLocked) {
			$icon = 'lock.png';
		}
		elseif ($partiallyLocked) {
			$icon = 'partiallyLocked.png';
		}
		// print icon and detail tooltips
		if ($unixAvailable || $sambaAvailable || $ppolicyAvailable) {
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
			$tipContent .= '</table>';
			$tooltip = "'" . $tipContent . "', TITLE, '" . _('Account status') . "'";
			echo '<img alt="status" onmouseout="UnTip()" onmouseover="Tip(' . $tooltip . ')" height=16 width=16 src="../../graphics/' . $icon . '">';
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
	public static function isPPolicyAvailable(&$attrs) {
		return in_array('ppolicyUser', $_SESSION['config']->get_AccountModules('user'));
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
	
}

?>