<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2005 - 2006  Roland Gruber

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
			"mail" => _("E-Mail"),
			"gecos" => _("Description")
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
class lamUserList extends lamList {
	
	/** Controls if GID number is translated to group name */
	var $trans_primary = false;

	/** translates GID to group name */
	var $trans_primary_hash = array();
	
	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function lamUserList($type) {
		parent::lamList($type);
		$this->labels = array(
			'nav' => _("%s user(s) found"),
			'error_noneFound' => _("No users found!"),
			'newEntry' => _("New user"),
			'deleteEntry' => _("Delete user"),
			'createPDF' => _("Create PDF for selected user(s)"),
			'createPDFAll' => _("Create PDF for all users"));
	}
	
	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	*/
	function listDoPost() {
		parent::listDoPost();
		// check if primary group should be translated
		if (isset($_POST['apply_trans_primary'])) {
			$this->trans_primary = $_POST['trans_primary'];
		}
		// generate hash table for group translation
		if ($this->trans_primary == "on" && ($this->refresh || (sizeof($this->trans_primary_hash) == 0))) {
			$this->trans_primary_hash = array();
			$grp_suffix = $_SESSION['config']->get_Suffix('group');
			$filter = "objectClass=posixGroup";
			$attrs = array("cn", "gidNumber");
			$sr = @ldap_search($_SESSION["ldap"]->server(), $grp_suffix, $filter, $attrs);
			if ($sr) {
				$info = @ldap_get_entries($_SESSION["ldap"]->server(), $sr);
				unset($info['count']); // delete count entry
				for ($i = 0; $i < sizeof($info); $i++) {
					$this->trans_primary_hash[$info[$i]['gidnumber'][0]] = $info[$i]['cn'][0];
				}
			}
		}
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
		// translate GIDs and resort array if selected
		if ($this->trans_primary == "on") {
			// translate GIDs
			for ($i = 0; $i < sizeof($info); $i++) {
				if (isset($this->trans_primary_hash[$info[$i]['gidnumber'][0]])) {
					$info[$i]['gidnumber'][0] = $this->trans_primary_hash[$info[$i]['gidnumber'][0]];
				}
			}
			// resort if needed
			if ($this->sortColumn == "gidnumber") {
				$info = $this->listSort($info);
			}
		}
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
					if (is_array($info[$i][$attrName])) {
						// sort array
						sort($info[$i][$attrName]);
						echo implode("; ", $info[$i][$attrName]);
					}
					else echo $info[$i][$attrName];
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
	}
	
	/**
	 * Prints additional option fields for specific object types.
	 */
	function listPrintAdditionalOptions() {
		// show translate GID to group name box if there is a column with gidnumber
		if (in_array("gidnumber", $this->attrArray)) {
			echo "<p align=\"left\">\n";
			echo "<b>" . _("Translate GID number to group name") . ": </b>";
			if ($this->trans_primary == "on") {
				echo "<input class=\"" . $this->type . "\" type=\"checkbox\" name=\"trans_primary\" checked>";
			}
			else echo "<input class=\"" . $this->type . "\" type=\"checkbox\" name=\"trans_primary\">";
			echo ("&nbsp;&nbsp;<input class=\"" . $this->type . "\" type=\"submit\" name=\"apply_trans_primary\" value=\"" . _("Apply") . "\">");
			echo "</p>\n";
		}
	}

}

?>