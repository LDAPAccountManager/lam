<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Roland Gruber

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
* Manages entries based on the object class nisNetgroup.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages entries based on the object class nisNetgroup.
*
* @package modules
*/
class nisnetgroup extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'groupBig.png';
		// manages netgroup accounts
		$return["account_types"] = array('netgroup');
		// alias name
		$return["alias"] = _("NIS net group");
		// this is a base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=nisNetgroup)");
		// RDN attributes
		$return["RDN"] = array("cn" => "normal");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('nisNetgroup');
		// managed attributes
		$return['attributes'] = array('cn', 'description', 'memberNisNetgroup', 'nisNetgroupTriple');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _("Group name"),
				"Text" => _("This is the name of this group.")
			),
			'description' => array(
				"Headline" => _("Description"),
				"Text" => _("Here you can enter a description for this group.")
			),
			'memberNisNetgroup' => array(
				"Headline" => _("Subgroups"),
				"Text" => _("Here you can specify subgroups which are included in this NIS netgroup. All members of the subgroups will be treated as members of this group.")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'nisnetgroup_cn',
				'description' => _('Group name'),
				'help' => 'cn',
				'example' => _('adminstrators'),
				'required' => true,
				'unique' => true
			),
			array(
				'name' => 'nisnetgroup_description',
				'description' => _('Group description'),
				'help' => 'description',
				'example' => _('Administrators group')
			),
			array(
				'name' => 'nisnetgroup_subgroups',
				'description' => _('Subgroups'),
				'help' => 'memberNisNetgroup',
				'example' => _('group01,group02')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn', 'description',
			'subgroups', 'members'
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['user'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['host'][0] = array('ERROR', _('Host name'), _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['domain'][0] = array('ERROR', _('Domain name'), _('Domain name is invalid!'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return = array();
		// user name
		$groupName = '';
		if (isset($this->attributes['cn'][0])) $groupName = $this->attributes['cn'][0];
		$return[] = array (
			array('kind' => 'text', 'text' => _("Group name").'*'),
			array('kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '30', 'maxlength' => '20', 'value' => $groupName),
			array('kind' => 'help', 'value' => 'cn'));
		$return[] = array(
			array('kind' => 'text', 'text' => _('Description')),
			array('kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['description'][0]),
			array ('kind' => 'help', 'value' => 'description'));
		$return[] = array(
			array('kind' => 'text', 'text' => _('Subgroups')),
			array('kind' => 'input', 'name' => 'form_subpage_nisnetgroup_group_open', 'type' => 'submit', 'value' => _('Edit subgroups')),
			array('kind' => 'help', 'value' => 'memberNisNetgroup'));
		// members
		$memberTable = array(array(
			array('kind' => 'text', 'text' => _('Host')),
			array('kind' => 'text', 'text' => _('User')),
			array('kind' => 'text', 'text' => _('Domain'),
			),
		));
		if (isset($this->attributes['nisNetgroupTriple']) && (sizeof($this->attributes['nisNetgroupTriple']) > 0)) {
			for ($i = 0; $i < sizeof($this->attributes['nisNetgroupTriple']); $i++) {
				$triple = substr($this->attributes['nisNetgroupTriple'][$i], 1, strlen($this->attributes['nisNetgroupTriple'][$i]) - 2);
				$triple = explode(',', $triple);
				$memberTable[] = array(
					array('kind' => 'input', 'type' => 'text', 'name' => 'host_' . $i, 'value' => $triple[0]),
					array('kind' => 'input', 'type' => 'text', 'name' => 'user_' . $i, 'value' => $triple[1]),
					array('kind' => 'input', 'type' => 'text', 'name' => 'domain_' . $i, 'value' => $triple[2]),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'del_' . $i, 'value' => _('Delete'))
				);
			}
		}
		$memberTable[] = array(
			array('kind' => 'text', 'text' => '&nbsp;'),
			array('kind' => 'text', 'text' => '&nbsp;'),
			array('kind' => 'text', 'text' => '&nbsp;'),
		);
		$hostNew = '';
		$userNew = '';
		$domainNew = '';
		if (isset($_POST['host_new'])) $hostNew = $_POST['host_new'];
		if (isset($_POST['user_new'])) $userNew = $_POST['user_new'];
		if (isset($_POST['domain_new'])) $domainNew = $_POST['domain_new'];
		$memberTable[] = array(
			array('kind' => 'input', 'type' => 'text', 'name' => 'host_new', 'value' => $hostNew),
			array('kind' => 'input', 'type' => 'text', 'name' => 'user_new', 'value' => $userNew),
			array('kind' => 'input', 'type' => 'text', 'name' => 'domain_new', 'value' => $domainNew),
			array('kind' => 'input', 'type' => 'submit', 'name' => 'add_new', 'value' => _('Add'))
		);
		$return[] = array(
			array('kind' => 'text', 'text' => _('Members'), 'td' => array('valign' => 'top')),
			array('kind' => 'table', 'value' => $memberTable),
			array('kind' => 'help', 'value' => 'members', 'td' => array('valign' => 'top')));
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// user name
		$this->attributes['cn'][0] = $_POST['cn'];
		if ( !get_preg($this->attributes['cn'][0], 'groupname')) {
			$errors[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// members
		$this->attributes['nisNetgroupTriple'] = array();
		$i = 0;
		while (isset($_POST['host_' . $i]) || isset($_POST['user_' . $i]) || isset($_POST['domain_' . $i])) {
			// build NIS triple
			$this->attributes['nisNetgroupTriple'][] = '(' . $_POST['host_' . $i] . ',' . $_POST['user_' . $i] . ',' . $_POST['domain_' . $i] . ')';
			// check user input
			if (($_POST['host_' . $i] != '') && !get_preg($_POST['host_' . $i], 'DNSname')) {
				$message = $this->messages['host'][0];
				$message[2] = $message[2] . '<br><br>' . $_POST['host_' . $i];
				$errors[] = $message;
			}
			if (($_POST['user_' . $i] != '') && !get_preg($_POST['user_' . $i], 'username')) {
				$message = $this->messages['user'][0];
				$message[2] = $message[2] . '<br><br>' . $_POST['user_' . $i];
				$errors[] = $message;
			}
			if (($_POST['domain_' . $i] != '') && !get_preg($_POST['domain_' . $i], 'DNSname')) {
				$message = $this->messages['domain'][0];
				$message[2] = $message[2] . '<br><br>' . $_POST['domain_' . $i];
				$errors[] = $message;
			}
			$i++;
		}
		// check user input
		if (($_POST['host_new'] != '') && !get_preg($_POST['host_new'], 'DNSname')) {
			$message = $this->messages['host'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['host_new'];
			$errors[] = $message;
		}
		if (($_POST['user_new'] != '') && !get_preg($_POST['user_new'], 'username')) {
			$message = $this->messages['user'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['user_new'];
			$errors[] = $message;
		}
		if (($_POST['domain_new'] != '') && !get_preg($_POST['domain_new'], 'DNSname')) {
			$message = $this->messages['domain'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['domain_new'];
			$errors[] = $message;
		}
		return $errors;
	}
	
	/**
	* Displays the group selection.
	*
	* @return array meta HTML code
	*/
	function display_html_group() {
		// load list with all groups
		$dn_groups = $_SESSION['cache']->get_cache('cn', 'nisNetgroup', 'netgroup');
		$DNs = array_keys($dn_groups);
		$allGroups = array();
		foreach ($DNs as $DN) {
			$allGroups[] = $dn_groups[$DN][0];
		}
		// remove own entry
		if (!$this->getAccountContainer()->isNewAccount) {
			$allGroups = array_delete($this->attributes['cn'][0], $allGroups);
		}
		$subgroups = array();
		if (is_array($this->attributes['memberNisNetgroup'])) {
			$subgroups = $this->attributes['memberNisNetgroup'];
			$allGroups = array_delete($subgroups, $allGroups);
		}
		
		$return[] = array(
			array('kind' => 'fieldset', 'legend' => _("Subgroups"), 'value' => array(
				array(
					array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Selected groups"), 'value' => array(
						array (
							array('kind' => 'select', 'name' => 'removegroups', 'size' => '15', 'multiple' => true, 'options' => $subgroups)))),
						array('kind' => 'table', 'value' => array(
							array(
								array('kind' => 'input', 'type' => 'submit', 'name' => 'addgroups_button', 'value' => '<=', 'td' => array('align' => 'center'))),
							array(
								array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>', 'td' => array('align' => 'center'))),
							array(
								array ( 'kind' => 'help', 'value' => 'memberNisNetgroup', 'td' => array('align' => 'center'))))),
						array('kind' => 'fieldset', 'td' => array('valign' => 'top'), 'legend' => _("Available groups"), 'value' => array(
							array(
								array('kind' => 'select', 'name' => 'addgroups', 'size' => '15', 'multiple' => true, 'options' => $allGroups))))
		))));

		$return[] = array(
			array('kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'form_subpage_nisnetgroup_attributes_back'),
			array('kind' => 'text'),
			array('kind' => 'text'));
		return $return;
	}

	/**
	* Processes user input of the group selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_group() {
		if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
			if (!is_array($this->attributes['memberNisNetgroup'])) {
				$this->attributes['memberNisNetgroup'] = array();
			}
			// Add new group
			$this->attributes['memberNisNetgroup'] = @array_merge($this->attributes['memberNisNetgroup'], $_POST['addgroups']);
		}
		elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
			$this->attributes['memberNisNetgroup'] = array_delete($_POST['removegroups'], $this->attributes['memberNisNetgroup']);
		}
		return array();
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		$messages = array();
		// get list of existing groups
		$dnGroups = $_SESSION['cache']->get_cache('cn', 'nisNetgroup', 'netgroup');
		$existingGroups = array();
		foreach ($dnGroups as $dn) {
			$existingGroups[] = $dn[0];
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('nisNetgroup', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'nisNetgroup';
			// add cn
			$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['nisnetgroup_cn']];
			// description (UTF-8, no regex check needed)
			if ($rawAccounts[$i][$ids['nisnetgroup_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['nisnetgroup_description']];
			}
			// additional groups
			if ($rawAccounts[$i][$ids['nisnetgroup_subgroups']] != "") {
				$groups = explode(",", $rawAccounts[$i][$ids['nisnetgroup_subgroups']]);
				$skipSubgroups = false;
				for ($g = 0; $g < sizeof($groups); $g++) {
					if (!in_array($groups[$g], $existingGroups)) {
						$messages[] = array('ERROR', _('Unable to find group in LDAP.'), $groups[$g]);
						$skipSubgroups = true;
					}
				}
				if (!$skipSubgroups) {
					$partialAccounts[$i]['memberNisNetgroup'] = $groups;
				}
			}
		}
		return $messages;
	}
	
	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		if (isset($this->attributes['cn']) && (sizeof($this->attributes['cn']) > 0)) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		// aliased entry
		$return[get_class($this) . '_cn'] = array('<block><key>' . _('Group name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>');
		if (isset($this->attributes['description'][0])) {
			$return[get_class($this) . '_description'] = array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>');
		}
		if (is_array($this->attributes['memberNisNetgroup'])) {
			$return[get_class($this) . '_subgroups'] = array('<block><key>' . _('Subgroups') . '</key><value>' . implode(', ', $this->attributes['memberNisNetgroup']) . '</value></block>');
		}
		if (sizeof($this->attributes['nisNetgroupTriple']) > 0) {
			$return[get_class($this) . '_members'][0] = '<block><key>' . _('Members') . '</key><tr><td align=\"L\">' . $this->attributes['nisNetgroupTriple'][0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($this->attributes['nisNetgroupTriple']); $i++) {
				$return[get_class($this) . '_members'][] = '<block><tr><td align=\"L\">' . $this->attributes['nisNetgroupTriple'][$i] . '</td></tr></block>'; 
			}
		}
		return $return;
	}

}


?>
