<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2010  Roland Gruber

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
			'cn' => _('Group name'),
			'description' => _('Description'),
			'subgroups' => _('Subgroups'),
			'members' => _('Members')
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
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// user name
		$groupName = '';
		if (isset($this->attributes['cn'][0])) $groupName = $this->attributes['cn'][0];
		$nameInput = new htmlTableExtendedInputField(_("Group name"), 'cn', $groupName, 'cn');
		$nameInput->setRequired(true);
		$nameInput->setFieldMaxLength(20);
		$return->addElement($nameInput, true);
		$description = '';
		if (isset($this->attributes['description'][0])) $description = $this->attributes['description'][0];
		$return->addElement(new htmlTableExtendedInputField(_('Description'), 'description', $description, 'description'), true);
		$subgroupsContainer = new htmlTable();
		if (isset($this->attributes['memberNisNetgroup']) && is_array($this->attributes['memberNisNetgroup'])) {
			$membergroups = $this->attributes['memberNisNetgroup'];
			while (sizeof($membergroups) > 0) {
				$parts = array_splice($membergroups, 0, 8);
				$subgroupsContainer->addElement(new htmlOutputText(implode(', ', $parts)), true);
			}
		}
		$subgroupsContainer->addElement(new htmlAccountPageButton(get_class($this), 'group', 'open', _('Edit subgroups')));
		$subgroupsLabel = new htmlOutputText(_('Subgroups'));
		$subgroupsLabel->alignment = htmlElement::ALIGN_TOP;
		$return->addElement($subgroupsLabel);
		$return->addElement($subgroupsContainer);
		$return->addElement(new htmlHelpLink('memberNisNetgroup'), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		// members
		$memberTable = new htmlTable();
		$hostLabel = new htmlOutputText(_('Host'));
		$hostLabel->colspan = 2;
		$memberTable->addElement($hostLabel);
		$userLabel = new htmlOutputText(_('User'));
		$userLabel->colspan = 2;
		$memberTable->addElement($userLabel);
		$memberTable->addElement(new htmlOutputText(_('Domain')));
		$memberTable->addElement(new htmlSpacer('5px', null));
		$memberTable->addElement(new htmlHelpLink('members'), true);
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
				$hostField = new htmlInputField('host_' . $i, $triple[0]);
				$hostField->setFieldSize(20);
				$memberTable->addElement($hostField);
				$hostButton = new htmlAccountPageButton(get_class($this), 'select', 'host' . $i, 'host.png', true);
				$hostButton->setTitle(_('Select host'));
				$memberTable->addElement($hostButton);
				$userField = new htmlInputField('user_' . $i, $triple[1]);
				$userField->setFieldSize(20);
				$memberTable->addElement($userField);
				$userButton = new htmlAccountPageButton(get_class($this), 'select', 'user' . $i, 'user.png', true);
				$userButton->setTitle(_('Select user'));
				$memberTable->addElement($userButton);
				$domainField = new htmlInputField('domain_' . $i, $triple[2]);
				$domainField->setFieldSize(20);
				$memberTable->addElement($domainField);
				$memberTable->addElement(new htmlSpacer('5px', null));
				$delButton = new htmlButton('del_' . $i, 'del.png', true);
				$delButton->setTitle(_('Delete'));
				$memberTable->addElement($delButton);
				$memberTable->addNewLine();
			}
		}
		$memberTable->addElement(new htmlSpacer(null, '10px'), true);
		$hostNew = '';
		$userNew = '';
		$domainNew = '';
		if (isset($_POST['host_new'])) $hostNew = $_POST['host_new'];
		if (isset($_POST['user_new'])) $userNew = $_POST['user_new'];
		if (isset($_POST['domain_new'])) $domainNew = $_POST['domain_new'];
		$hostField = new htmlInputField('host_new', $hostNew);
		$hostField->setFieldSize(20);
		$memberTable->addElement($hostField);
		$hostButton = new htmlAccountPageButton(get_class($this), 'select', 'hostNew', 'host.png', true);
		$hostButton->setTitle(_('Select host'));
		$memberTable->addElement($hostButton);
		$userField = new htmlInputField('user_new', $userNew);
		$userField->setFieldSize(20);
		$memberTable->addElement($userField);
		$userButton = new htmlAccountPageButton(get_class($this), 'select', 'userNew', 'user.png', true);
		$userButton->setTitle(_('Select user'));
		$memberTable->addElement($userButton);
		$domainField = new htmlInputField('domain_new', $domainNew);
		$domainField->setFieldSize(20);
		$memberTable->addElement($domainField);
		$memberTable->addElement(new htmlSpacer('5px', null));
		$addButton = new htmlButton('add_new', 'add.png', true);
		$addButton->setTitle(_('Add'));
		$memberTable->addElement($addButton);
		$memberTable->colspan = 2;
		
		$membersLabel = new htmlOutputText(_('Members'));
		$membersLabel->alignment = htmlElement::ALIGN_TOP;
		$return->addElement($membersLabel);
		$return->addElement($memberTable);
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
	* @return htmlElement meta HTML code
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
		$return = new htmlTable();
		$return->addElement(new htmlSubTitle(_("Subgroups")), true);
		$return->addElement(new htmlOutputText(_("Selected groups")));
		$return->addElement(new htmlOutputText(''));
		$return->addElement(new htmlOutputText(_("Available groups")));
		$return->addNewLine();
		$selGroupSelect = new htmlSelect('removegroups', $subgroups, array(), 15);
		$selGroupSelect->setMultiSelect(true);
		$return->addElement($selGroupSelect);
		$buttonContainer = new htmlTable();
		$buttonContainer->addElement(new htmlButton('addgroups_button', 'back.gif', true), true);
		$buttonContainer->addElement(new htmlButton('removegroups_button', 'forward.gif', true), true);
		$buttonContainer->addElement(new htmlHelpLink('memberNisNetgroup'));
		$return->addElement($buttonContainer);
		$availGroupSelect = new htmlSelect('addgroups', $allGroups, array(), 15);
		$availGroupSelect->setMultiSelect(true);
		$return->addElement($availGroupSelect);
		$return->addNewLine();
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$backButton->colspan = 3;
		$return->addElement($backButton);
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
	* @return htmlElement meta HTML code
	*/
	function display_html_select() {
		$return = new htmlTable();
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
		$return->addElement(new htmlOutputText(_('Filter')));
		$return->addElement(new htmlInputField('filter', $filter));
		$return->addElement(new htmlButton($filterButtonName, _('Ok')));
		$return->addElement(new htmlHelpLink('filter'), true);
		$title = _('Host name');
		if (!$selectHost) $title = _('User name');
		$return->addElement(new htmlOutputText($title));
		$return->addElement(new htmlSelect('selectBox', $options), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$type = 'host';
		if (!$selectHost) $type = 'user';
		$buttonContainer = new htmlTable();
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'select', _('Ok')));
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Cancel')));
		$buttonContainer->colspan = 4;
		$return->addElement($buttonContainer, true);
		$return->addElement(new htmlHiddenInput('host_new', $_POST['host_new']));
		$return->addElement(new htmlHiddenInput('user_new', $_POST['user_new']));
		$return->addElement(new htmlHiddenInput('domain_new', $_POST['domain_new']));
		$return->addElement(new htmlHiddenInput('type', $type));
		$return->addElement(new htmlHiddenInput('position', $position));
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
