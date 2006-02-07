<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2005  Roland Gruber

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
*/

/**
* The account type for group accounts (e.g. Unix and Samba).
*
* @package types
*/
class group extends baseType {
	
	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
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
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array (
			"cn" => _("Group name"),
			"gidnumber" => _("GID number"),
			"memberuid" => _("Group members"),
			"member" => _("Group member DNs"),
			"description" => _("Group description")
			);
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

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function lamGroupList($type) {
		parent::lamList($type);
		$this->labels = array(
			'nav' => _("%s group(s) found"),
			'error_noneFound' => _("No groups found!"),
			'newEntry' => _("New group"),
			'deleteEntry' => _("Delete group"),
			'createPDF' => _("Create PDF for selected group(s)"),
			'createPDFAll' => _("Create PDF for all groups"));
	}
	
	/**
	* Prints the entry list
	* 
	* @param array $info entries
	*/
	function listPrintTableBody($info) {
		// calculate which rows to show
		$table_begin = ($this->page - 1) * $this->maxPageEntries;
		if (($this->page * $this->maxPageEntries) > sizeof($info)) $table_end = sizeof($info);
		else $table_end = ($this->page * $this->maxPageEntries);
		// print account list
		for ($i = $table_begin; $i < $table_end; $i++) {
			echo("<tr class=\"" . $this->type . "list\" onMouseOver=\"list_over(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onMouseOut=\"list_out(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=" . $this->type . "&amp;DN=" . $info[$i]['dn'] . "'\">\n");
			if (isset($_GET['selectall'])) {
				echo " <td height=22 align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" checked name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			else {
				echo " <td height=22 align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			echo (" <td align='center'><a href=\"../account/edit.php?type=" . $this->type . "&amp;DN='" . $info[$i]['dn'] . "'\">" . _("Edit") . "</a></td>\n");
			for ($k = 0; $k < sizeof($this->attrArray); $k++) {
				echo ("<td>");
				// print all attribute entries seperated by "; "
				$attrName = strtolower($this->attrArray[$k]);
				if (isset($info[$i][$attrName]) && sizeof($info[$i][$attrName]) > 0) {
					// delete "count" entry
					unset($info[$i][$attrName]['count']);
					// generate links for group members
					if ($attrName == "memberuid") {
						// sort array
						sort($info[$i][$attrName]);
						// make a link for each member of the group
						$linklist = array();
						for ($d = 0; $d < sizeof($info[$i][$attrName]); $d++) {
							$user = $info[$i][$attrName][$d]; // user name
							$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
						}
						echo implode("; ", $linklist);
					}
					// print all other attributes
					else {
						if (is_array($info[$i][$attrName])) {
							// sort array
							sort($info[$i][$attrName]);
							echo implode("; ", $info[$i][$attrName]);
						}
						else echo $info[$i][$attrName];
					}
				}
				echo ("</td>\n");
			}
			echo("</tr>\n");
		}
		// display select all link
		$colspan = sizeof($this->attrArray) + 1;
		echo "<tr class=\"" . $this->type . "list\">\n";
		echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
		echo "<td colspan=$colspan>&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=y&amp;page=" . $this->page . 
			"&amp;sort=" . $this->sortColumn . $this->filterText . "&amp;selectall=yes\">" .
			"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
		echo "</tr>\n";
		echo ("</table>");
		
		echo ("<br>");
	}
	
}


?>