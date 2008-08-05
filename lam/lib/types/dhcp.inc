<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2008  Thomas Manninger

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
			"cn" => ("Subnet"),
			"dhcprange" => ("IP range"),
			"fixed_ips" => ("Fixed IP address / MAC address / description")
			);
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

	// A array with all fixed ips, mac adresses and descriptions
	var $fixed;
	
	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("%s DHCP(s) found"),
			'error_noneFound' => _("No DHCPs found!"),
			'newEntry' => _("New DHCP"),
			'dhcpDefaults' => _("DHCP settings"),
			'deleteEntry' => _("Delete DHCP(s)"));
	}

	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	public function listPrintTableCellContent(&$entry, &$attribute) {
	    // Fixed Ips
        if ($attribute=="fixed_ips") {
        	// find all fixed addresses:
        	$ldap = $_SESSION['ldap'];
        	$suffix = $_SESSION['config']->get_Suffix('dhcp');
        	
        	$sr = ldap_search($ldap->server(),"cn=".$entry['cn'][0].",".$suffix,"objectClass=dhcpHost");
        	$get = ldap_get_entries($ldap->server(),$sr);
        	
        	// Now list all fixed_ips:
			echo "<table border=\"0\" width=\"100%\">";
        	$this->fixed_ips = array();
        	foreach($get AS $id=>$arr) {
        		if (is_numeric($id)) {
        				echo "<tr>";
        					echo "<td width=\"25%\">".array_pop(explode(" ",$get[$id]['dhcpstatements'][0]))."</td>";
        					echo "<td width=\"35%\">".array_pop(explode(" ",$get[$id]['dhcphwaddress'][0]))."</td>";
        					echo "<td width=\"40%\">".$get[$id]['cn'][0]."</td>";
        				echo "</tr>";
        		}
        	}
        	echo "</table>";
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
            if (is_array($entry['dhcprange'])) {
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
		// add/delete/PDF buttons
		echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"new\" value=\"" . $this->labels['newEntry'] . "\">\n");
		if (!$createOnly) {
			echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"del\" value=\"" . $this->labels['deleteEntry'] . "\">\n");
		}
		echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"dhcpDefaults\" value=\"" . $this->labels['dhcpDefaults'] . "\">\n");
		echo "</td>\n";
		echo "<td align=\"right\">\n";
			$this->listShowOUSelection();
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