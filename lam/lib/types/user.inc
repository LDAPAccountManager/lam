<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2012  Roland Gruber

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
	 * Constructs a new user type object.
	 */
	public function __construct() {
		parent::__construct();
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another user');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to user list');
	}
	
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
			"mail" => _("Email"),
			"gecos" => _("Description"),
			"jpegphoto" => _('Photo'),
			'shadowexpire' => _('Password expiration'),
			'sambakickofftime' => _('Account expiration date')
			);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param array $attributes list of LDAP attributes for the displayed account (null, if new account)
	 * @return String title text
	 */
	public function getTitleBarTitle($attributes) {
		if ($attributes == null) {
			return _("New user");
		}
		// check if first and last name can be shown
		if (isset($attributes['sn'][0]) && isset($attributes['givenName'][0])) {
			return htmlspecialchars($attributes['givenName'][0] . ' ' . $attributes['sn'][0]);
		}
		// check if a display name is set
		if (isset($attributes['displayName'][0])) {
			return htmlspecialchars($attributes['displayName'][0]);
		}
		// check if a common name is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		// check if a user name is set
		if (isset($attributes['uid'][0])) {
			return htmlspecialchars($attributes['uid'][0]);
		}
		// fall back to default
		return parent::getTitleBarTitle($attributes);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param array $attributes list of LDAP attributes for the displayed account (null, if new account)
	 * @return String title text
	 */
	public function getTitleBarSubtitle($attributes) {
		if ($attributes == null) {
			return null;
		}
		$subtitle = '';
		$spacer = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
		// check if an email address can be shown
		if (isset($attributes['mail'][0])) {
			$subtitle .= '<a href="mailto:' . htmlspecialchars($attributes['mail'][0]) . '">' . htmlspecialchars($attributes['mail'][0]) . '</a>' . $spacer;
		}
		// check if an telephone number can be shown
		if (isset($attributes['telephoneNumber'][0])) {
			$subtitle .= _('Telephone number') . ' ' . htmlspecialchars($attributes['telephoneNumber'][0]) . $spacer;
		}
		// check if an mobile number can be shown
		if (isset($attributes['mobile'][0])) {
			$subtitle .= _('Mobile number') . ' ' . htmlspecialchars($attributes['mobile'][0]);
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
 * @author Roland Gruber
 * 
 */
class lamUserList extends lamList {
	
	/** Controls if GID number is translated to group name */
	private $trans_primary = false;

	/** translates GID to group name */
	private $trans_primary_hash = array();
	
	/** ID for config option */
	const TRANS_PRIMARY_OPTION_NAME = "LU_TP";

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("%s user(s) found"),
			'error_noneFound' => _("No users found!"),
			'newEntry' => _("New user"),
			'deleteEntry' => _("Delete selected users"));
	}
	
	/**
	 * Sets some internal parameters.
	 */
	protected function listGetParams() {
		parent::listGetParams();
		// generate hash table for group translation
		if ($this->trans_primary == "on" && !$this->refresh && (sizeof($this->trans_primary_hash) == 0)) {
			$this->refreshPrimaryGroupTranslation();
		}		
	}
	
	/**
	 * Rereads the entries from LDAP.
	 */
	protected function listRefreshData() {
		parent::listRefreshData();
		if ($this->trans_primary == "on") {
			$this->refreshPrimaryGroupTranslation();
		}
	}
	
	/**
	 * Refreshes the GID to group name cache.
	 */
	protected function refreshPrimaryGroupTranslation() {
		$this->trans_primary_hash = array();
		$grp_suffix = $_SESSION['config']->get_Suffix('group');
		$filter = "objectClass=posixGroup";
		$attrs = array("cn", "gidNumber");
		$entries = searchLDAPByAttribute(null, null, 'posixGroup', $attrs, array('group'));
		for ($i = 0; $i < sizeof($entries); $i++) {
			$this->trans_primary_hash[$entries[$i]['gidnumber'][0]] = $entries[$i]['cn'][0];
		}
	}

	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	protected function listPrintTableCellContent(&$entry, &$attribute) {
		// check if there is something to display at all
		if (!isset($entry[$attribute]) || !is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) return;
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
				$result = @ldap_read($_SESSION['ldap']->server(), escapeDN($entry['dn']), $attribute . "=*", array($attribute), 0, 0, 0, LDAP_DEREF_NEVER);
				if ($result) {
					$tempEntry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($tempEntry) {
						$binData = ldap_get_values_len($_SESSION['ldap']->server(), $tempEntry, $attribute);
						$entry[$attribute] = $binData;
					}
				}
			}
			$imgNumber = $_SESSION['ldap']->new_rand(); 
			$jpeg_filename = 'jpg' . $imgNumber . '.jpg';
			$outjpeg = @fopen(dirname(__FILE__) . '/../../tmp/' . $jpeg_filename, "wb");
			fwrite($outjpeg, $entry[$attribute][0]);
			fclose ($outjpeg);
			$photoFile = '../../tmp/' . $jpeg_filename;
			$imgSize = getimagesize($photoFile);
			$minSize = 64;
			if ($imgSize[0] < 64) {
				$minSize = $imgSize[0];
			}
			$imgTitle = _('Click to switch between thumbnail and original size.');
			echo "<img id=\"img$imgNumber\" title=\"$imgTitle\" height=$minSize src=\"" . $photoFile . "\" alt=\"" . _('Photo') . "\">";
			echo '<script type="text/javascript">';
			echo "addResizeHandler(document.getElementById(\"img$imgNumber\"), $minSize, " . $imgSize[1] . ")";
			echo '</script>';
		}
		elseif (($attribute == 'mail') || ($attribute == 'rfc822Mailbox')) {
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				for ($i = 0; $i < sizeof($entry[$attribute]); $i++) {
					if ($i > 0) {
						echo ", ";
					}
					echo "<a href=\"mailto:" . $entry[$attribute][$i] . "\">" . $entry[$attribute][$i] . "</a>\n";
				}
			}
		}
		// expire dates
		elseif ($attribute == 'shadowexpire') {
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				echo date('d. m. Y', $entry[$attribute][0] * 24 * 3600);
			}
		}
		elseif ($attribute == 'sambakickofftime') {
			if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
				if ($entry[$attribute][0] > 2147483648) {
					echo "∞";
				}
				else {
					$date = getdate($entry[$attribute][0]);
					echo $date['mday'] . "." . $date['mon'] . "." . $date['year'];
				}
			}
		}
		// print all other attributes
		else {
			parent::listPrintTableCellContent($entry, $attribute);
		}
	}

	/**
	 * Returns a list of lamListTool objects to display next to the edit/delete buttons.
	 *
	 * @return lamListTool[] tools
	 */
	protected function getAdditionalTools() {
		if (isLAMProVersion() && checkIfPasswordChangeIsAllowed()) {
			$passwordTool = new lamListTool(_('Change password'), 'key.png', 'changePassword.php');
			return array($passwordTool);
		}
		return array();
	}

	/**
	 * Returns a list of possible configuration options.
	 *
	 * @return array list of lamListOption objects
	 */
	protected function listGetAllConfigOptions() {
		$options = parent::listGetAllConfigOptions();
		$options[] = new lamBooleanListOption(_('Translate GID number to group name'), self::TRANS_PRIMARY_OPTION_NAME);
		return $options;
	}

	/**
	 * Called when the configuration options changed.
	 */
	protected function listConfigurationChanged() {
		parent::listConfigurationChanged();
		$tpOption = $this->listGetConfigOptionByID(self::TRANS_PRIMARY_OPTION_NAME);
		$this->trans_primary = $tpOption->isSelected();
	}
	
}

?>