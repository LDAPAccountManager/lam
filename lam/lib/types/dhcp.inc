<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2009 - 2013  Roland Gruber

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
	 */
	public function __construct() {
		parent::__construct();
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
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	public function getListAttributeDescriptions() {
		return array (
			"cn" => _("Subnet"),
			"dhcprange" => _("Ranges"),
			"fixed_ips" => _("IP address") . ' / ' . _('MAC address') . ' / ' . _("Description")
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
		$this->labels = array(
			'nav' => _("DHCP count: %s"),
			'error_noneFound' => _("No DHCPs found!"),
			'newEntry' => _("New DHCP"),
			'dhcpDefaults' => _("DHCP settings"),
			'deleteEntry' => _("Delete selected DHCP entries"));
	}

	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	public function listPrintTableCellContent(&$entry, &$attribute) {
	    // Fixed IPs
        if ($attribute=="fixed_ips") {
        	// find all fixed addresses:
			$entries = searchLDAP($entry['dn'], 'objectClass=dhcpHost', array('dhcpstatements', 'dhcphwaddress', 'cn'));
			if (sizeof($entries) > 0) {
				echo "<table border=\"0\" width=\"100%\">";
				for ($i = 0; $i < sizeof($entries); $i++) {
					$dhcpstatements = array();
					if (isset($entries[$i]['dhcpstatements'][0])) {
						$dhcpstatements = $entries[$i]['dhcpstatements'];
					}
					$style = '';
					if (!fixed_ip::isActive($dhcpstatements)) {
						$style = 'style="text-decoration: line-through;"';
					}
					echo "<tr " . $style . ">";
					echo "<td width=\"25%\">" . fixed_ip::extractIP($dhcpstatements) . "</td>";
					$dhcphwaddress = explode(" ",$entries[$i]['dhcphwaddress'][0]);
					echo "<td width=\"35%\">".array_pop($dhcphwaddress)."</td>";
					echo "<td width=\"40%\">".$entries[$i]['cn'][0]."</td>";
					echo "</tr>";
				}
				echo "</table>";
			}
        }
		// fixed ip address
        elseif ($attribute=="dhcpstatements") {
			// Search after the fixed ip entry
			if (is_array($entry['dhcpstatements'])) {
				foreach($entry['dhcpstatements'] AS $id=>$value) {
				
					if (!is_array($value) && array_shift( explode(" ", $value) ) == "fixed-address") {
					    $ip = explode(" ", $value);
						echo $ip['1'];
					}
				}
			}
        }
        elseif ($attribute=="dhcprange") { // DHCP Range
            if (isset($entry['dhcprange'])) {
                echo"<table cellspacing=\"0\">";
				foreach($entry['dhcprange'] AS $id=>$value) {
					if (!empty($value) && !is_numeric($value)) {
						$ex = explode(" ", $value);
						echo "<tr><td>".$ex[0]."</td><td width=\"20\"><center>-</center></td><td>".$ex[1]."</td></tr>";
					}
				}
				echo"</table>";
			}
        }
		else
		{
			parent::listPrintTableCellContent($entry, $attribute);
		}
	}
	
	/**
	 * Add DCP main settings button.
	 * 
	 * @param htmlGroup $left left part
	 * @param htmlGroup $right right part
	 */
	protected function addExtraInputElementsToTopArea(&$left, &$right) {
		if (checkIfWriteAccessIsAllowed()) {
			$left->addElement(new htmlSpacer('20px', null));
			$dhcpButton = new htmlButton('dhcpDefaults', $this->labels['dhcpDefaults']);
			$dhcpButton->setIconClass('settingsButton');
			$left->addElement($dhcpButton);
		}
	}

	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	* 
	* @return String HTML fragment to insert into beginning of account list
	*/
	function listDoPost() {
		$fragment = parent::listDoPost();
        if (isset($_POST['dhcpDefaults'])) {
        	metaRefresh("../account/edit.php?type=dhcp&DN='".$_SESSION['config']->get_suffix('dhcp')."'");
        }
        else {
        	return $fragment;
        }
	}
}
?>