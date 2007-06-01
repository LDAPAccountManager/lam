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
			"gecos" => _("Description"),
			"jpegphoto" => _('Photo')
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
			'deleteEntry' => _("Delete user(s)"),
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
	}
	
	/**
	 * Sets some internal parameters.
	 */
	function listGetParams() {
		parent::listGetParams();
		// generate hash table for group translation
		if ($this->trans_primary == "on" && !$this->refresh && (sizeof($this->trans_primary_hash) == 0)) {
			$this->refreshPrimaryGroupTranslation();
		}		
	}
	
	/**
	 * Rereads the entries from LDAP.
	 */
	function listRefreshData() {
		parent::listRefreshData();
		if ($this->trans_primary == "on") {
			$this->refreshPrimaryGroupTranslation();
		}
	}
	
	/**
	 * Refreshes the GID to group name cache.
	 */
	function refreshPrimaryGroupTranslation() {
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

	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	function listPrintTableCellContent(&$entry, &$attribute) {
		// check if there is something to display at all
		if (!is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) return;
		if (isset($entry[$attribute]['count'])) unset($entry[$attribute]['count']);
		// translate GID to group name
		if (($attribute == "gidnumber") && ($this->trans_primary == "on")) {
			if (isset($this->trans_primary_hash[$entry[$attribute][0]])) {
				echo $this->trans_primary_hash[$entry[$attribute][0]];
			}
			else {
				parent::listPrintTableCellContent($entry, $attribute);
			}
		}
		// show user photos
		elseif ($attribute == "jpegphoto") {
			if (sizeof($entry[$attribute][0]) < 100) {
				// looks like we have read broken binary data, reread photo
				$result = @ldap_search($_SESSION['ldap']->server(), $entry['dn'], $attribute . "=*", array($attribute));
				if ($result) {
					$tempEntry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($tempEntry) {
						$binData = ldap_get_values_len($_SESSION['ldap']->server(), $tempEntry, $attribute);
						if (isset($binData['count'])) unset($binData['count']);
						$entry[$attribute] = $binData;
					}
				}
			}
			$jpeg_filename = 'jpg' . $_SESSION['ldap']->new_rand() . '.jpg';
			$outjpeg = @fopen($_SESSION['lampath'] . 'tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $entry[$attribute][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			echo "<img src=\"" . $photoFile . "\" alt=\"" . _('Photo') . "\">";
		}
		// print all other attributes
		else {
			parent::listPrintTableCellContent($entry, $attribute);
		}
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