<?php
use \LAM\TYPES\TypeManager;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2022  Roland Gruber

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
* The account type for group accounts (e.g. Unix and Samba).
*
* @package types
* @author Roland Gruber
* @author Ludek Finstrle
*/

/**
* The account type for group accounts (e.g. Unix and Samba).
*
* @package types
*/
class group extends baseType {

	/**
	 * Constructs a new group type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another group');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to group list');
		if ($this->getType() != null) {
			$modules = $this->getType()->getModules();
			if (in_array('organizationalRole', $modules)) {
				$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another role');
				$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to role list');
			}
		}
	}

	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		if ($this->getType() != null) {
		$modules = $this->getType()->getModules();
			if (in_array('organizationalRole', $modules)) {
				return _('Roles');
			}
		}
		return _("Groups");
	}

	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("Group accounts (e.g. Unix and Samba)");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getIcon() {
		return 'group.svg';
	}

	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamGroupList";
	}

	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#cn;#gidNumber;#memberUID;#description";
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		// get attributes
		$attributes = null;
		if ($container->getAccountModule('posixGroup') != null) {
			$attributes = $container->getAccountModule('posixGroup')->getAttributes();
		}
		elseif ($container->getAccountModule('rfc2307bisPosixGroup') != null) {
			$attributes = $container->getAccountModule('rfc2307bisPosixGroup')->getAttributes();
		}
		elseif ($container->getAccountModule('windowsGroup') != null) {
			$attributes = $container->getAccountModule('windowsGroup')->getAttributes();
		}
		$gonAttributes = null;
		if ($container->getAccountModule('groupOfNames') != null) {
			$gonAttributes = $container->getAccountModule('groupOfNames')->getAttributes();
		}
		elseif ($container->getAccountModule('groupOfUniqueNames') != null) {
			$gonAttributes = $container->getAccountModule('groupOfUniqueNames')->getAttributes();
		}
		elseif ($container->getAccountModule('organizationalRole') != null) {
			$gonAttributes = $container->getAccountModule('organizationalRole')->getAttributes();
		}
		// check if a group name is set
		if (isset($attributes['gid'][0])) {
			return htmlspecialchars($attributes['gid'][0]);
		}
		// check if a common name is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		if (isset($gonAttributes['cn'][0])) {
			return htmlspecialchars($gonAttributes['cn'][0]);
		}
		// new entry
		if ($container->isNewAccount) {
			return _("New group");
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
		// get attributes
		$attributes = null;
		if ($container->getAccountModule('posixGroup') != null) {
			$attributes = $container->getAccountModule('posixGroup')->getAttributes();
		}
		elseif ($container->getAccountModule('rfc2307bisPosixGroup') != null) {
			$attributes = $container->getAccountModule('rfc2307bisPosixGroup')->getAttributes();
		}
		elseif ($container->getAccountModule('windowsGroup') != null) {
			$attributes = $container->getAccountModule('windowsGroup')->getAttributes();
		}
		$gonAttributes = null;
		if ($container->getAccountModule('groupOfNames') != null) {
			$gonAttributes = $container->getAccountModule('groupOfNames')->getAttributes();
		}
		elseif ($container->getAccountModule('groupOfUniqueNames') != null) {
			$gonAttributes = $container->getAccountModule('groupOfUniqueNames')->getAttributes();
		}
		elseif ($container->getAccountModule('organizationalRole') != null) {
			$gonAttributes = $container->getAccountModule('organizationalRole')->getAttributes();
		}
		// check if an description can be shown
		if (($attributes != null) && isset($attributes['description'][0])) {
			return htmlspecialchars($attributes['description'][0]);
		}
		if (($gonAttributes != null) && isset($gonAttributes['description'][0])) {
			return htmlspecialchars($gonAttributes['description'][0]);
		}
		return null;
	}

}

/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 *
 */
class lamGroupList extends lamList {

	/** Primary group members hash */
	private $primary_hash = array();
	/** Controls if primary group members needs refresh */
	private $refresh_primary = false;
	/** ID for config option */
	private const TRANS_PRIMARY_OPTION_NAME = "LG_TP";
	/** specifies if primary group members are visible */
	private $include_primary = false;
	/** LDAP suffix of membership types */
	private $suffixList = array();

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("Group count: %s"),
			'error_noneFound' => _("No groups found!"),
			'newEntry' => _("New group"),
			'deleteEntry' => _("Delete selected groups"));
		$modules = $this->type->getModules();
		if (in_array('organizationalRole', $modules)) {
			$this->labels = array(
				'nav' => _("Role count: %s"),
				'error_noneFound' => _("No roles found!"),
				'newEntry' => _("New role"),
				'deleteEntry' => _("Delete selected roles"));
		}
		// build suffix list for account types
		$typeManager = new TypeManager();
		$scopes = array('user', 'gon', 'group');
		$types = $typeManager->getConfiguredTypesForScopes($scopes);
		foreach ($types as $type) {
			$suffix = $type->getSuffix();
			// stop if suffixes are not unique
			if (isset($this->suffixList[$suffix])) {
				$this->suffixList = array();
				break;
			}
			if (!empty($suffix)) {
				$this->suffixList[$suffix] = $type->getId();
			}
		}
	}

	/**
	 * Sets some internal parameters.
	 */
	function listGetParams() {
		parent::listGetParams();
		// generate list primary group members
		// after parent::listGetParams is $this->refresh set to correct value
		if ($this->include_primary && !$this->refresh && ($this->refresh_primary || (sizeof($this->primary_hash) == 0))) {
			$this->groupRefreshPrimary();
		}
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::getTableCellContent()
	 */
	protected function getTableCellContent(&$entry, &$attribute) {
		if ($attribute == "memberuid") {
			// $gid is used for linking primary group members
			$gid = -1;
			$use_primary = false;
			if ($this->include_primary == "on") {
				// Get the gid number
				if (isset($entry['gidnumber']) && is_array($entry['gidnumber'])) {
					$gid = $entry['gidnumber'][0];
				}
				$use_primary = (($gid >= 0) && (sizeof($this->primary_hash) > 0) &&
					isset($this->primary_hash[$gid]) && is_array($this->primary_hash[$gid]) &&
					(sizeof($this->primary_hash[$gid]) > 0));
			}
			if (!$use_primary) {
				if (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) {
					return;
				}
				// sort array
				sort($entry[$attribute]);
			}
			// make a link for each member of the group
			$linklist = array();
			if ($use_primary) {
				$primary_hash = $this->primary_hash[$gid];
				// merge primary members into secondary ones
				$primaryvals = array_flip(array_values($primary_hash));
				// test if group has some secondary members
				if (isset($entry[$attribute])) {
					$attr = array_merge($primary_hash,$entry[$attribute]);
				}
				else {
					$attr = $primary_hash;
				}
				// sort array
				sort($attr);

				// make a link for each member of the group
				for ($d = 0; $d < sizeof($attr); $d++) {
					$user = $attr[$d]; // user name
					if (isAccountTypeHidden('user')) {
						$linklist[$d] = $user;
					}
					elseif (isset($primaryvals[$user])) {
						$linklist[$d] = "<b><a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a></b>";
					}
					else {
						$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
					}
				}
			}
			else {
				// make a link for each member of the group
				for ($d = 0; $d < sizeof($entry[$attribute]); $d++) {
					$user = $entry[$attribute][$d]; // user name
					if (!isAccountTypeHidden('user')) {
						$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
					}
					else {
						$linklist[$d] = $user;
					}
				}
			}
			return new htmlOutputText(implode("; ", $linklist), false);
		}
		// print all other attributes
		else {
			return parent::getTableCellContent($entry, $attribute);
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	function listRefreshData() {
		parent::listRefreshData();
		if ($this->include_primary) {
			$this->groupRefreshPrimary();
		}
	}

	/**
	 * Refreshes the primary group members list.
	 */
	function groupRefreshPrimary() {
		$this->refresh_primary = false;
		// return unless some entries
		if (sizeof($this->ldapEntries) <= 0) {
			return;
		}

		$scope = "user";
		// configure search filter
		$module_filter = get_ldap_filter($scope);  // basic filter is provided by modules
		$attrs = array( "uid" );
		for ($i = 0; $i < sizeof($this->ldapEntries); $i++) {
			if (empty($this->ldapEntries[$i]['gidnumber'][0])) {
				continue;
			}
			$gid = $this->ldapEntries[$i]['gidnumber'][0];
			$filter = "(&(&" . $module_filter  . ")(gidNumber=" . $gid . "))";
			$entries = searchLDAPByFilter($filter, $attrs, array($scope));
			for ($j = 0; $j < sizeof($entries); $j++) {
				$this->primary_hash[$gid][$j] = $entries[$j]['uid'][0];
			}
		}
	}

	/**
	 * Returns a list of possible configuration options.
	 *
	 * @return array list of lamListOption objects
	 */
	protected function listGetAllConfigOptions() {
		$options = parent::listGetAllConfigOptions();
		$options[] = new lamBooleanListOption(_('Show primary group members as normal group members'), self::TRANS_PRIMARY_OPTION_NAME);
		return $options;
	}

	/**
	 * Called when the configuration options changed.
	 */
	protected function listConfigurationChanged() {
		parent::listConfigurationChanged();
		$tpOption = $this->listGetConfigOptionByID(self::TRANS_PRIMARY_OPTION_NAME);
		$use_primary = $this->include_primary;
 		$this->include_primary = $tpOption->isSelected();
		if (!$use_primary && $this->include_primary) {
			$this->refresh_primary = true;
		}
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::getAdditionalLDAPAttributesToRead()
	 */
	protected function getAdditionalLDAPAttributesToRead() {
		$attrs = $this->type->getAttributes();
		$ret = array();
		$countLen = strlen('_count');
		foreach ($attrs as $attr) {
			$attrName = $attr->getAttributeName();
			if (strrpos($attrName, '_count') === (strlen($attrName) - $countLen)) {
				$ret[] = substr($attrName, 0, strlen($attrName) - $countLen);
			}
		}
		return $ret;
	}

}
