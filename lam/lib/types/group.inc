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

	/** Controls if include primary group members into group memebers */
	private $include_primary = "";
	/** Primary group members hash */
	private $primary_hash = array();
	/** Controls if primary group members needs refresh */
	private $refresh_primary = false;
	/** Controls if primary group members are using */
	// This is here for future use with primary group members listing
	private $use_primary = false;

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
			'deleteEntry' => _("Delete group(s)"),
			'createPDF' => _("Create PDF for selected group(s)"),
			'createPDFAll' => _("Create PDF for all groups"));
	}
	
	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	*/
	function listDoPost() {
		parent::listDoPost();
		// check if list primary group members
		if (isset($_POST['apply_group_addopts'])) {
			$use_primary = $this->use_primary;
 			$this->include_primary = $_POST['addopts_include_primary'];
			$this->groupSetUsePrimary();
			if (!$use_primary && $this->use_primary)
				$this->refresh_primary = true;
		}
	}

	/**
	 * Sets some internal parameters.
	 */
	function listGetParams() {
		parent::listGetParams();
		// generate list primary group memebers
		// after parent::listGetParams is $this->refresh set to correct value
		if ($this->use_primary && !$this->refresh && ($this->refresh_primary || (sizeof($this->primary_hash) == 0)))
			$this->groupRefreshPrimary();
	}

	/**
	 * Prints additional option field for primary member display.
	 */
	function listPrintAdditionalOptions() {
		parent::listPrintAdditionalOptions();
		// show primary group members option
		if (in_array("memberuid", $this->attrArray)) {
			echo "<b>" . _("Show primary group members as normal group members") . ": </b>";
			echo "<input class=\"" . $this->type . "\" type=\"checkbox\" name=\"addopts_include_primary\"";
			if ($this->include_primary == "on") 
				echo " checked";
			echo ">";
			echo ("&nbsp;&nbsp;<input class=\"" . $this->type . "\" type=\"submit\" name=\"apply_group_addopts\" value=\"" . _("Apply") . "\">");
			echo "</p>\n";
		}
	}
  	
	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	function listPrintTableCellContent(&$entry, &$attribute) {
		if ($attribute == "memberuid") {
			// $gid is used for linking primary group memebers
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
			if ($use_primary) {
				if (isset($entry[$attribute]) && isset($entry[$attribute]['count'])) unset($entry[$attribute]['count']);
			} else {
				if (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) return;
				if (isset($entry[$attribute]['count'])) unset($entry[$attribute]['count']);
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
					if (isset($primaryvals[$user])) {
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
					$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
				}
			}
			echo implode("; ", $linklist);
		}
		// print all other attributes
		else {
			parent::listPrintTableCellContent($entry, $attribute);
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	function listRefreshData() {
		parent::listRefreshData();
		if ($this->use_primary) {
			$this->groupRefreshPrimary();
		}
	}

	/**
	 * Refreshes the primary group members list.
	 */
	function groupRefreshPrimary() {
		$this->refresh_primary = false;
		// return unless some entries
		if (sizeof($this->entries) <= 0) return;

		$scope = "user";
		// get search suffix
		$module_suffix = $_SESSION["config"]->get_Suffix($scope);
		// configure search filter
		$module_filter = get_ldap_filter($scope);  // basic filter is provided by modules
		$attrs = array( "uid" );
		for ($i = 0; $i < sizeof($this->entries); $i++) {
			$this->primary_hash[$i]['memberuid_primary'] = array();
			$gid = $this->entries[$i]['gidnumber'][0];
			$filter = "(&(&" . $module_filter  . ")(gidNumber=" . $gid . "))";
			$sr = @ldap_search($_SESSION["ldap"]->server(), $module_suffix, $filter, $attrs);
			if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
				StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
				$this->refresh_primary = true;
			}
			if ($sr) {
				$members = ldap_get_entries($_SESSION["ldap"]->server(), $sr);
				ldap_free_result($sr);
				// delete first array entry which is "count"
				unset($members['count']);
				for ($j = 0; $j < sizeof($members); $j++) {
					$this->primary_hash[$gid][$j] = $members[$j]['uid'][0];
				}
			}
		}
	}

	/**
	 * Checks if primary group members should be included.
	 */
	function groupSetUsePrimary() {
		$this->use_primary = ($this->include_primary == "on");
	}
 
}


?>