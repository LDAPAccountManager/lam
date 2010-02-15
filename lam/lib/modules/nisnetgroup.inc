<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
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
			),
			'members' => array(
				"Headline" => _("Members"),
				"Text" => _("These entries specify the members of the netgroup. You can limit the set to a host name, a user name, a domain name or any combination of them.")
						),
			'membersUpload' => array(
				"Headline" => _("Members"),
				"Text" => _("These entries specify the members of the netgroup. You can limit the set to a host name, a user name, a domain name or any combination of them.") . ' ' .
						_("For the upload please specify the entries in the format \"(HOST,USER,DOMAIN)\". Multiple entries are separated by semicolon.")
						),
			'filter' => array(
				"Headline" => _("Filter"),
				"Text" => _("Here you can enter a filter value. Only entries which contain the filter text will be shown.")
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
			),
			array(
				'name' => 'nisnetgroup_members',
				'description' => _('Members'),
				'help' => 'membersUpload',
				'example' => _('(host1,user1,example.com);(host2,user2,example.com)')
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
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	*/
	function save_attributes() {
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// nisNetgroupTriple needs special changing
		if (isset($return[$this->getAccountContainer()->dn]['remove']['nisNetgroupTriple'])) {
			$return[$this->getAccountContainer()->dn]['modify']['nisNetgroupTriple'] = $this->attributes['nisNetgroupTriple'];
			unset($return[$this->getAccountContainer()->dn]['remove']['nisNetgroupTriple']);
		}
		if (isset($return[$this->getAccountContainer()->dn]['add']['nisNetgroupTriple'])) {
			$return[$this->getAccountContainer()->dn]['modify']['nisNetgroupTriple'] = $this->attributes['nisNetgroupTriple'];
			unset($return[$this->getAccountContainer()->dn]['add']['nisNetgroupTriple']);
		}
		return $return;
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
		$subgroups = array();
		if (isset($this->attributes['memberNisNetgroup']) && is_array($this->attributes['memberNisNetgroup'])) {
			$membergroups = $this->attributes['memberNisNetgroup'];
			while (sizeof($membergroups) > 0) {
				$parts = array_splice($membergroups, 0, 8);
				$subgroups[] = array(array('kind' => 'text', 'text' => implode(', ', $parts)));
			}
		}
		$subgroups[] = array(array('kind' => 'input', 'name' => 'form_subpage_nisnetgroup_group_open', 'type' => 'submit', 'value' => _('Edit subgroups'))); 	
		$return[] = array(
			array('kind' => 'text', 'text' => _('Subgroups'), 'td' => array('valign' => 'top')),
			array('kind' => 'table', 'value' => $subgroups),
			array('kind' => 'help', 'value' => 'memberNisNetgroup'));
		$return[] = array(
			array('kind' => 'text', 'text' => '&nbsp;'),
			array('kind' => 'text', 'text' => '&nbsp;'),
			array('kind' => 'text', 'text' => '&nbsp;'),
		);
		// members
		$memberTable = array(array(
			array('kind' => 'text', 'text' => _('Host'), 'td' => array('colspan' => 2)),
			array('kind' => 'text', 'text' => _('User'), 'td' => array('colspan' => 2)),
			array('kind' => 'text', 'text' => _('Domain'),
			),
		));
		if (isset($this->attributes['nisNetgroupTriple']) && (sizeof($this->attributes['nisNetgroupTriple']) > 0)) {
			for ($i = 0; $i < sizeof($this->attributes['nisNetgroupTriple']); $i++) {
				$triple = substr($this->attributes['nisNetgroupTriple'][$i], 1, strlen($this->attributes['nisNetgroupTriple'][$i]) - 2);
				$triple = explode(',', $triple);
				if (isset($_POST['form_subpage_nisnetgroup_attributes_select']) && ($_POST['type'] == 'host') && ($_POST['position'] == $i)) {
					$triple[0] = $_POST['selectBox'];
				}
				if (isset($_POST['form_subpage_nisnetgroup_attributes_select']) && ($_POST['type'] == 'user') && ($_POST['position'] == $i)) {
					$triple[1] = $_POST['selectBox'];
				}
				$memberTable[] = array(
					array('kind' => 'input', 'type' => 'text', 'name' => 'host_' . $i, 'value' => $triple[0]),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_nisnetgroup_select_host' . $i, 'image' => 'host.png', 'value' => ' ', 'title' => _('Select host')),
					array('kind' => 'input', 'type' => 'text', 'name' => 'user_' . $i, 'value' => $triple[1]),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_nisnetgroup_select_user' . $i, 'image' => 'user.png', 'value' => ' ', 'title' => _('Select user')),
					array('kind' => 'input', 'type' => 'text', 'name' => 'domain_' . $i, 'value' => $triple[2]),
					array('kind' => 'text', 'text' => '&nbsp;'),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'del_' . $i, 'image' => 'del.png', 'value' => ' ', 'title' => _('Delete'))
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
			array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_nisnetgroup_select_hostNew', 'image' => 'host.png', 'value' => ' ', 'title' => _('Select host')),
			array('kind' => 'input', 'type' => 'text', 'name' => 'user_new', 'value' => $userNew),
			array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_nisnetgroup_select_userNew', 'image' => 'user.png', 'value' => ' ', 'title' => _('Select user')),
			array('kind' => 'input', 'type' => 'text', 'name' => 'domain_new', 'value' => $domainNew),
			array('kind' => 'text', 'text' => '&nbsp;'),
			array('kind' => 'input', 'type' => 'submit', 'name' => 'add_new', 'image' => 'add.png', 'value' => ' ', 'title' => _('Add'))
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
			if (isset($_POST['del_' . $i])) {
				$i++;
				continue;
			}
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
		$addNewTriple = isset($_POST['add_new']);
		// check user input
		if (($_POST['host_new'] != '') && !get_preg($_POST['host_new'], 'DNSname')) {
			$message = $this->messages['host'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['host_new'];
			$errors[] = $message;
			$addNewTriple = false;
		}
		if (($_POST['user_new'] != '') && !get_preg($_POST['user_new'], 'username')) {
			$message = $this->messages['user'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['user_new'];
			$errors[] = $message;
			$addNewTriple = false;
		}
		if (($_POST['domain_new'] != '') && !get_preg($_POST['domain_new'], 'DNSname')) {
			$message = $this->messages['domain'][0];
			$message[2] = $message[2] . '<br><br>' . $_POST['domain_new'];
			$errors[] = $message;
			$addNewTriple = false;
		}
		if ($addNewTriple) {
			$this->attributes['nisNetgroupTriple'][] = '(' . $_POST['host_new'] . ',' . $_POST['user_new'] . ',' . $_POST['domain_new'] . ')';
			unset($_POST['host_new']);
			unset($_POST['user_new']);
			unset($_POST['domain_new']);
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
								array('kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>', 'td' => array('align' => 'center'))),
							array(
								array('kind' => 'help', 'value' => 'memberNisNetgroup', 'td' => array('align' => 'center'))))),
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
	* Displays the host/user selection.
	*
	* @return array meta HTML code
	*/
	function display_html_select() {
		$return = array();
		$selectHost = true;
		$postKeys = array_keys($_POST);
		$position = 'New';
		$filterButtonName = '';
		$filter = '';
		if (isset($_POST['filter'])) {
			$filter = $_POST['filter'];
		}
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if 	(substr($postKeys[$i], 0, 36) == 'form_subpage_nisnetgroup_select_user') {
				$selectHost = false;
				$position = substr($postKeys[$i], 36);
				$filterButtonName = $postKeys[$i];
				break;
			}
			if 	(substr($postKeys[$i], 0, 36) == 'form_subpage_nisnetgroup_select_host') {
				$position = substr($postKeys[$i], 36);
				$filterButtonName = $postKeys[$i];
				break;
			}
		}
		$options = array();
		if ($selectHost) {
			// load list with all hosts
			$dn_hosts = $_SESSION['cache']->get_cache('uid', 'account', 'host');
			$DNs = array_keys($dn_hosts);
			foreach ($DNs as $DN) {
				if (get_preg($dn_hosts[$DN][0], 'DNSname')) {
					if (($filter == '') || !(strpos($dn_hosts[$DN][0], $filter) === false)) {
						$options[] = $dn_hosts[$DN][0];
					}
				}
			}
		}
		else {
			// load list with all users
			$dn_users = $_SESSION['cache']->get_cache('uid', 'posixAccount', 'user');
			$DNs = array_keys($dn_users);
			foreach ($DNs as $DN) {
				if (($filter == '') || !(strpos($dn_users[$DN][0], $filter) === false)) {
					$options[] = $dn_users[$DN][0];
				}
			}
		}
		$return[] = array(
			array('kind' => 'text', 'text' => _('Filter')),
			array('kind' => 'input', 'type' => 'text', 'name' => 'filter', 'value' => $filter),
			array('kind' => 'input', 'type' => 'submit', 'name' => $filterButtonName, 'value' => _('Ok')),
			array('kind' => 'help', 'value' => 'filter')
		);
		$title = _('Host name');
		if (!$selectHost) $title = _('User name');
		$return[] = array(
			array('kind' => 'text', 'text' => $title),
			array('kind' => 'select', 'name' => 'selectBox', 'options' => $options)
		);
		$return[] = array(
			array('kind' => 'text', 'text' => '&nbsp;')
		);
		$type = 'host';
		if (!$selectHost) $type = 'user';
		$return[] = array(
			array('kind' => 'table', 'td' => array('colspan' => 2), 'value' => array(array(
				array('kind' => 'input', 'type' => 'submit', 'value' => _('Ok'), 'name' => 'form_subpage_nisnetgroup_attributes_select'),
				array('kind' => 'input', 'type' => 'submit', 'value' => _('Cancel'), 'name' => 'form_subpage_nisnetgroup_attributes_back'),
			))),
			array('kind' => 'input', 'type' => 'hidden', 'name' => 'host_new', 'value' => $_POST['host_new']),
			array('kind' => 'input', 'type' => 'hidden', 'name' => 'user_new', 'value' => $_POST['user_new']),
			array('kind' => 'input', 'type' => 'hidden', 'name' => 'domain_new', 'value' => $_POST['domain_new']),
			array('kind' => 'input', 'type' => 'hidden', 'name' => 'type', 'value' => $type),
			array('kind' => 'input', 'type' => 'hidden', 'name' => 'position', 'value' => $position),
			);
		return $return;
	}
	
	/**
	* Processes user input of the host/user selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_select() {
		if (isset($_POST['form_subpage_nisnetgroup_attributes_back'])) {
			return array();
		}
		if ($_POST['position'] == 'New') {
			$_POST[$_POST['type'] . '_new'] = $_POST['selectBox'];
		}
		return array();
	}
	
	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
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
			// members
			if ($rawAccounts[$i][$ids['nisnetgroup_members']] != "") {
				$members = explode(';', $rawAccounts[$i][$ids['nisnetgroup_members']]);
				$partialAccounts[$i]['nisNetgroupTriple'] = $members;
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
			$return[get_class($this) . '_members'] = array('<block><key>' . _('Members') . '</key><value> </value></block>');
			$return[get_class($this) . '_members'][] = '<block><tr>' .
					'<td width="20%"><b>' . _('Host') . '</b></td>' .
					'<td width="20%"><b>' . _('User') . '</b></td>' .
					'<td width="30%"><b>' . _('Domain') . '</b></td>' .
					'</tr></block>';
			for ($i = 0; $i < sizeof($this->attributes['nisNetgroupTriple']); $i++) {
				$triple = substr($this->attributes['nisNetgroupTriple'][$i], 1, strlen($this->attributes['nisNetgroupTriple'][$i]) - 2);
				$triple = explode(',', $triple);
				$return[get_class($this) . '_members'][] = '<block><tr>' .
					'<td width="20%" align=\"L\">' . $triple[0] . ' </td>' .
					'<td width="20%" align=\"L\">' . $triple[1] . ' </td>' .
					'<td width="30%" align=\"L\">' . $triple[2] . ' </td>' .
					'</tr></block>';
			}
			$return[get_class($this) . '_members'][] = '<block><tr><td width="80%"> </td></tr></block>';
		}
		return $return;
	}

}


?>
