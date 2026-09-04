<?php

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2026  Roland Gruber

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

use LAM\TYPES\ConfiguredType;

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
	 * {@inheritDoc}
	 */
	public function getAlias(): string {
		return _("Users");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("User accounts (e.g. Unix, Samba and Kolab)");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getIcon(): string {
		return 'user.svg';
	}

	/**
	 * {@inheritDoc}
	 */
	public function getListClassName(): string {
		return lamUserList::class;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDefaultListAttributes(): string {
		return "#uid;#givenName;#sn;#uidNumber;#gidNumber";
	}

	/**
	 * {@inheritDoc}
	 */
	public function getTitleBarTitle(accountContainer $container): ?string {
		$title = '';
		// get attributes
		$personalAttributes = [];
		if ($container->getAccountModule('inetOrgPerson') != null) {
			$personalAttributes = $container->getAccountModule('inetOrgPerson')->getAttributes();
		}
		elseif ($container->getAccountModule('windowsUser') != null) {
			$personalAttributes = $container->getAccountModule('windowsUser')->getAttributes();
		}
		$accountAttributes = [];
		if ($container->getAccountModule('account') != null) {
			$accountAttributes = $container->getAccountModule('account')->getAttributes();
		}
		$sambaAttributes = [];
		if ($container->getAccountModule('sambaSamAccount') != null) {
			$sambaAttributes = $container->getAccountModule('sambaSamAccount')->getAttributes();
		}
		$unixAttributes = [];
		if ($container->getAccountModule('posixAccount') != null) {
			$unixAttributes = $container->getAccountModule('posixAccount')->getAttributes();
		}
		$mitKerberosAttributes = [];
		if ($container->getAccountModule('mitKerberosStructural') != null) {
			$mitKerberosAttributes = $container->getAccountModule('mitKerberosStructural')->getAttributes();
		}
		elseif ($container->getAccountModule('mitKerberos') != null) {
			$mitKerberosAttributes = $container->getAccountModule('mitKerberos')->getAttributes();
		}
		// check if the first and last name can be shown
		if (!empty($personalAttributes['sn'][0]) && !empty($personalAttributes['givenName'][0])) {
			return $title . htmlspecialchars($personalAttributes['givenName'][0] . ' ' . $personalAttributes['sn'][0]);
		}
		// check if a display name is set
		if (!empty($sambaAttributes['displayName'][0])) {
			return $title . htmlspecialchars($sambaAttributes['displayName'][0]);
		}
		// check if a common name is set
		if (!empty($personalAttributes['cn'][0])) {
			return $title . htmlspecialchars($personalAttributes['cn'][0]);
		}
		if (!empty($unixAttributes['cn'][0])) {
			return $title . htmlspecialchars($unixAttributes['cn'][0]);
		}
		// check if a username is set
		if (!empty($unixAttributes['uid'][0])) {
			return $title . htmlspecialchars($unixAttributes['uid'][0]);
		}
		if (!empty($personalAttributes['uid'][0])) {
			return $title . htmlspecialchars($personalAttributes['uid'][0]);
		}
		if (!empty($accountAttributes['uid'][0])) {
			return $title . htmlspecialchars($accountAttributes['uid'][0]);
		}
		if (!empty($mitKerberosAttributes['krbPrincipalName'][0])) {
			return $title . htmlspecialchars($mitKerberosAttributes['krbPrincipalName'][0]);
		}
		if ($container->isNewAccount) {
			return $title . _("New user");
		}
		// fall back to default
		return $title . parent::getTitleBarTitle($container);
	}

	/**
	 * {@inheritDoc}
	 */
	public function getTitleBarSubtitle(accountContainer $container): string {
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
	private bool $trans_primary = false;

	/**
	 * @var array<string, string> $trans_primary_hash translates GID to group name
	 */
	private array $trans_primary_hash = [];

	/** ID for config option to translate primary group GIDs to group names */
	private const TRANS_PRIMARY_OPTION_NAME = "LU_TP";

	/**
	 * Constructor
	 *
	 * @param ConfiguredType $type account type
	 */
	public function __construct(ConfiguredType $type) {
		parent::__construct($type);
		$this->labels = [
			'nav' => _("User count: %s"),
			'error_noneFound' => _("No users found!"),
			'newEntry' => _("New user"),
			'deleteEntry' => _("Delete selected users")];
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listGetParams(): void {
		parent::listGetParams();
		// generate hash table for group translation
		if ($this->trans_primary == "on" && !$this->refresh && (count($this->trans_primary_hash) === 0)) {
			$this->refreshPrimaryGroupTranslation();
		}
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listRefreshData(): void {
		parent::listRefreshData();
		// show group names
		if ($this->trans_primary == "on") {
			$this->refreshPrimaryGroupTranslation();
		}
	}

	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation(): void {
		$this->trans_primary_hash = [];
		$attrs = ["cn", "gidNumber"];
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, ['group']);
		$entryCount = count($entries);
		for ($i = 0; $i < $entryCount; $i++) {
			$this->trans_primary_hash[$entries[$i]['gidnumber'][0]] = $entries[$i]['cn'][0];
		}
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::getTableCellContent()
	 */
	protected function getTableCellContent(&$entry, &$attribute): ?htmlElement {
		// check if there is something to display at all
		if (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (count($entry[$attribute]) < 1)) {
			return parent::getTableCellContent($entry, $attribute);
		}
		// translate GID to group name
		if (($attribute === "gidnumber") && ($this->trans_primary == "on")
			&& isset($this->trans_primary_hash[$entry[$attribute][0]])) {
			return new htmlOutputText($this->trans_primary_hash[$entry[$attribute][0]]);
		}
		// print all other attributes
		return parent::getTableCellContent($entry, $attribute);
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listGetAllConfigOptions(): array {
		$options = parent::listGetAllConfigOptions();
		$options[] = new lamBooleanListOption(_('Translate GID number to group name'), self::TRANS_PRIMARY_OPTION_NAME);
		return $options;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listConfigurationChanged(): void {
		parent::listConfigurationChanged();
		$tpOption = $this->listGetConfigOptionByID(self::TRANS_PRIMARY_OPTION_NAME);
		$this->trans_primary = ($tpOption instanceof lamBooleanListOption) && $tpOption->isSelected();
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
		return strtolower($attr) !== 'jpegphoto';
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::isFilterMatching()
	 */
	protected function isFilterMatching(&$data, $filterAttribute, $regex): bool {
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
