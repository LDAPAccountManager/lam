<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2009 - 2024  Roland Gruber

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
* The account type for DHCP
*
* @package types
* @author Thomas Manninger
**/

/**
* The account type for DHCP
*
* @package types
**/

class dhcp extends baseType {

	/**
	 * Constructs a new DHCP type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another DHCP entry');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to DHCP list');
	}

	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	public function getAlias() {
		return _("DHCP");
	}

	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	public function getDescription() {
		return _("DHCP administration");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getIcon() {
		return 'world-color.svg';
	}

	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	public function getListClassName() {
		return "lamDHCPList";
	}

	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	public 	function getDefaultListAttributes() {
		return "#cn;#dhcpRange;#fixed_ips";
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
		if ($container->getAccountModule('dhcp_settings') != null) {
			$attributes = $container->getAccountModule('dhcp_settings')->getAttributes();
		}
		// check if a common name is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		// new entry
		if ($container->isNewAccount) {
			return _("New DHCP");
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
		if ($container->getAccountModule('dhcp_settings') != null) {
			$attributes = $container->getAccountModule('dhcp_settings')->getAttributes();
		}
		if ($attributes == null) {
			return null;
		}
		$subtitle = '';
		// check if an description can be shown
		if (isset($attributes['dhcpComments'][0])) {
			$subtitle .= htmlspecialchars($attributes['dhcpComments'][0]);
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
 * @author Thomas Manninger
 *
 */
class lamDHCPList extends lamList {

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->labels = [
			'nav' => _("DHCP count: %s"),
			'error_noneFound' => _("No DHCPs found!"),
			'newEntry' => _("New DHCP"),
			'dhcpDefaults' => _("DHCP settings"),
			'deleteEntry' => _("Delete selected DHCP entries")
		];
	}

	/**
	 * {@inheritDoc}
	 * @see lamList::isFilterMatching()
	 */
	protected function isFilterMatching(&$data, $filterAttribute, $regex) {
		if ($filterAttribute === "fixed_ips") {
			if (empty($regex)) {
				return true;
			}
			$entries = searchLDAP($data['dn'], 'objectClass=dhcpHost', ['dhcpstatements', 'dhcphwaddress', 'cn']);
			if (sizeof($entries) > 0) {
				foreach ($entries as $entry) {
					foreach ($entry as $attrValues) {
						if (!is_array($attrValues)) {
							continue;
						}
						foreach ($attrValues as $attrValue) {
							if (preg_match($regex, $attrValue)) {
								return true;
							}
						}
						if (!empty($entry['dhcpstatements'])) {
							$ip = fixed_ip::extractIP($entry['dhcpstatements']);
							if (!empty($ip)) {
								if (preg_match($regex, $ip)) {
									return true;
								}
							}
						}
					}
				}
			}
			return false;
		}
        elseif ($filterAttribute === "dhcprange") {
            if (isset($data['dhcprange'])) {
            	foreach ($data['dhcprange'] as $range) {
            		if (preg_match($regex, $range)) {
            			return true;
            		}
            	}
            }
            $pooledRanges = searchLDAP($data['dn'], '(objectclass=dhcpPool)', ['dhcprange']);
            foreach ($pooledRanges as $pooledRange) {
            	if (empty($pooledRange['dhcprange'])) {
            		continue;
            	}
            	foreach ($pooledRange['dhcprange'] as $range) {
            		if (preg_match($regex, $range)) {
            			return true;
            		}
            	}
            }
        }
		return parent::isFilterMatching($data, $filterAttribute, $regex);
	}

	/**
	 * Add DCP main settings button.
	 *
	 * @param htmlGroup $left left part
	 * @param htmlGroup $right right part
	 */
	protected function addExtraInputElementsToTopArea(&$left, &$right) {
		if (checkIfWriteAccessIsAllowed($this->type->getId())) {
			$dhcpButton = new htmlButton('dhcpDefaults', $this->labels['dhcpDefaults']);
			$dhcpButton->setCSSClasses(['fullwidth-mobile-only lam-secondary']);
			$left->addElement($dhcpButton);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	function listDoPost() {
		$fragment = parent::listDoPost();
        if (isset($_POST['dhcpDefaults'])) {
        	metaRefresh("../account/edit.php?type=" . $this->type->getId() . "&DN='" . $this->type->getSuffix() . "'");
        	die();
        }
        else {
        	return $fragment;
        }
	}
}
