<?php

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

	/** translates GID to group name */
	private $trans_primary_hash = [];

	/** ID for config option to translate primary group GIDs to group names */
	private const TRANS_PRIMARY_OPTION_NAME = "LU_TP";

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->labels = [
			'nav' => _("User count: %s"),
			'error_noneFound' => _("No users found!"),
			'newEntry' => _("New user"),
			'deleteEntry' => _("Delete selected users")];
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
	}

	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation() {
		$this->trans_primary_hash = [];
		$attrs = ["cn", "gidNumber"];
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, ['group']);
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
		if (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) {
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
		return $options;
	}

	/**
	 * Called when the configuration options changed.
	 */
	protected function listConfigurationChanged() {
		parent::listConfigurationChanged();
		$tpOption = $this->listGetConfigOptionByID(self::TRANS_PRIMARY_OPTION_NAME);
		$this->trans_primary = $tpOption->isSelected();
	}

	/**
	 * Returns if the given attribute can be filtered.
	 * If filtering is not possible then no filter box will be displayed.
	 * <br>
	 * <br>The user list allows no filtering for photos.
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
	 * @see lamList::isFilterMatching()
	 */
	protected function isFilterMatching(&$data, $filterAttribute, $regex) {
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

}
