<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2009 - 2012  Roland Gruber

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
					echo "<tr>";
					$dhcpstatements = array('');
					if (isset($entries[$i]['dhcpstatements'][0])) {
						$dhcpstatements = explode(" ",$entries[$i]['dhcpstatements'][0]);
					}
					echo "<td width=\"25%\">".array_pop($dhcpstatements)."</td>";
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
					if (!is_numeric($value)) {
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
	 * Prints the create, delete and PDF buttons.
	 *
	 * @param boolean $createOnly true if only the create button should be displayed
	 */
	function listPrintButtons($createOnly) {
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo "<td align=\"left\">\n";
		if (checkIfWriteAccessIsAllowed()) {
			// add/delete buttons
			echo ("<button id=\"newButton\" name=\"new\">" . $this->labels['newEntry'] . "</button>\n");
			if (!$createOnly) {
				echo ("<button id=\"delButton\" name=\"del\">" . $this->labels['deleteEntry'] . "</button>\n");
			}
			$toolSettings = $_SESSION['config']->getToolSettings();
			if (!(isset($toolSettings['tool_hide_toolFileUpload']) && ($toolSettings['tool_hide_toolFileUpload'] == 'true'))) {
				echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
				echo "<button id=\"fileUploadButton\" name=\"fileUpload\">" . _('File upload') . "</button>\n";
			}
			echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
			echo ("<button id=\"dhcpDefaultsButton\" name=\"dhcpDefaults\">" . $this->labels['dhcpDefaults'] . "</button>\n");
			echo "&nbsp;&nbsp;&nbsp;";
			?>
				<script type="text/javascript">
					jQuery(document).ready(function() {
						jQuery('#newButton').button({
					        icons: {
					      	  primary: 'createButton'
					    	}
						});
						jQuery('#delButton').button({
					        icons: {
					      	  primary: 'deleteButton'
					    	}
						});
						jQuery('#fileUploadButton').button({
					        icons: {
					      	  primary: 'upButton'
					    	}
						});
						jQuery('#dhcpDefaultsButton').button({
					        icons: {
					      	  primary: 'settingsButton'
					    	}
						});
					});
				</script>
			<?php
		}
		echo "</td>\n";
		echo "<td align=\"right\">\n";
		$this->listShowOUSelection();
		echo "<input class=\"smallImageButton\" style=\"background-image: url(../../graphics/refresh.png);\" type=\"submit\" value=\" \" name=\"refresh\" title=\"" . _("Refresh") . "\">";
		echo '&nbsp;<a href="#" onclick="listShowSettingsDialog(\'' . _('Change list settings') . '\', \'' . _('Ok') . '\', \'' . _('Cancel') . '\');">';
			echo '<img height=16 width=16 src="../../graphics/tools.png" alt="' . _('Change settings') . '" title="' . _('Change settings') . '">';
		echo '</a>';
		echo "</td>\n";
		echo "</tr>\n";
		echo "</table>\n";
	}
	
	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	*/
	function listDoPost() {
		parent::listDoPost();
        if (isset($_POST['dhcpDefaults'])) {
        	metaRefresh("../account/edit.php?type=dhcp&DN='".$_SESSION['config']->get_suffix('dhcp')."'");
        }
	}
}
?>