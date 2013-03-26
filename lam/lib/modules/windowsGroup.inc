<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
* Manages Windows AD (e.g. Samba 4) groups.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Windows AD (e.g. Samba 4) groups.
*
* @package modules
*/
class windowsGroup extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// manages host accounts
		$return["account_types"] = array('group');
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=group)");
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('group');
		// managed attributes
		$return['attributes'] = array('cn', 'description', 'info', 'mail', 'member', 'sAMAccountName');
		// help Entries
		$return['help'] = array(
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'cn' => array(
				"Headline" => _('Group name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the group\'s name.')
			),
			'description' => array(
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Group description. If left empty group name will be used.')
			),
			'info' => array(
				"Headline" => _('Notes'), 'attr' => 'info',
				"Text" => _('Additional notes to describe this entry.')
			),
			'mail' => array(
				"Headline" => _('Email address'), 'attr' => 'mail',
				"Text" => _('The list\'s email address.')
			),
			'member' => array(
				"Headline" => _('Members'), 'attr' => 'member',
				"Text" => _('This is a list of members of this group.')
			),
		);
		// configuration settings
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainer->addElement($configContainerHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hidemail', false, _('Email address'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		$return['config_options']['all'] = $configContainer;
		// upload fields
		$return['upload_columns'] = array(
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Group name'),
			'description' => _('Description'),
			'info' => _('Notes'),
			'member' => _('Members'),
		);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_mail', _('Please enter a valid email address!'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$container = new htmlTable();
		$this->addSimpleInputTextField($container, 'cn', _('Group name'), true);
		$this->addSimpleInputTextField($container, 'description', _('Description'), false);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$this->addSimpleInputTextField($container, 'mail', _('Email address'), false);
		}
		// group members
		$container->addElement(new htmlSpacer(null, '10px'), true);
		$container->addElement(new htmlOutputText(_("Group members")));
		$container->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Edit members')));
		$container->addElement(new htmlHelpLink('member'), true);
		$memberList = array();
		if (isset($this->attributes['member'])) {
			for ($i = 0; $i < sizeof($this->attributes['member']); $i++) {
				$memberList[] = $this->attributes['member'][$i];
			}
			usort($memberList, 'compareDN');
		}
		$members = new htmlTable();
		$members->alignment = htmlElement::ALIGN_RIGHT;
		$members->colspan = 3;
		for ($i = 0; $i < sizeof($memberList); $i++) {
			$member = new htmlOutputText(getAbstractDN($memberList[$i]));
			$member->alignment = htmlElement::ALIGN_RIGHT;
			$members->addElement($member, true);
		}
		$container->addElement(new htmlOutputText(''));
		$container->addElement($members, true);
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$return = array();
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		$this->attributes['sAMAccountName'][0] = $_POST['cn'];
		if (!get_preg($_POST['cn'], 'groupname')) {
			$return[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// email
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$this->attributes['mail'][0] = $_POST['mail'];
			if (!empty($_POST['mail']) && !get_preg($_POST['mail'], 'email')) {
				$return[] = $this->messages['mail'][0];
			}
		}
		return $return;
	}
	
	/**
	* This function will create the meta HTML code to show a page to change the member attribute.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_user() {
		$return = new htmlTable();
		// show list of possible new members
		if (isset($_POST['form_subpage_' . get_class($this) . '_user_select']) && isset($_POST['type'])) {
			$options = array();
			$filter = get_ldap_filter($_POST['type']);
			$entries = searchLDAPByFilter($filter, array('dn'), array($_POST['type']));
			for ($i = 0; $i < sizeof($entries); $i++) {
				$entries[$i] = $entries[$i]['dn'];
			}
			// sort by DN
			usort($entries, 'compareDN');
			for ($i = 0; $i < sizeof($entries); $i++) {
				if (!isset($this->attributes['member']) || !in_array($entries[$i], $this->attributes['member'])) {
					$options[getAbstractDN($entries[$i])] = $entries[$i];
				}
			}
			$size = 20;
			if (sizeof($options) < 20) $size = sizeof($options);
			$membersSelect = new htmlSelect('members', $options, array(), $size);
			$membersSelect->setHasDescriptiveElements(true);
			$membersSelect->setMultiSelect(true);
			$membersSelect->setRightToLeftTextDirection(true);
			$membersSelect->setSortElements(false);
			$membersSelect->setTransformSingleSelect(false);
			$return->addElement($membersSelect, true);
			$buttonTable = new htmlTable();
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'addMembers', _('Add')));
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'cancel', _('Cancel')));
			$return->addElement($buttonTable);
			return $return;
		}
		// show existing members
		$membersTemp = array();
		if (isset($this->attributes['member'])) {
			$membersTemp = $this->attributes['member'];
		}
		// sort by DN
		usort($membersTemp, 'compareDN');
		$members = array();
		for ($i = 0; $i < sizeof($membersTemp); $i++) {
			$members[getAbstractDN($membersTemp[$i])] = $membersTemp[$i];
		}
		$size = 20;
		if (isset($this->attributes['member']) && (sizeof($this->attributes['member']) < 20)) {
			$size = sizeof($this->attributes['member']);
		}
		if (sizeof($members) > 0) {
			$membersSelect = new htmlSelect('members', $members, array(), $size);
			$membersSelect->setHasDescriptiveElements(true);
			$membersSelect->setMultiSelect(true);
			$membersSelect->setRightToLeftTextDirection(true);
			$membersSelect->setSortElements(false);
			$membersSelect->setTransformSingleSelect(false);
			$return->addElement($membersSelect, true);
			$removeButton = new htmlAccountPageButton(get_class($this), 'user', 'remove', _('Remove selected entries'));
			$removeButton->colspan = 3;
			$return->addElement($removeButton, true);
			$return->addElement(new htmlOutputText('&nbsp;', false), true);
		}
		$types = $_SESSION['config']->get_ActiveTypes();
		$options = array();
		$optionsSelected = array();
		for ($i = 0; $i < sizeof($types); $i++) {
			$options[getTypeAlias($types[$i])] = $types[$i];
			if ($types[$i] == 'user') {
				$optionsSelected[] = $types[$i];
			}
		}
		$typeTable = new htmlTable();
		$typeTable->addElement(new htmlOutputText(_('Add entries of this type:') . ' '));
		$typeSelect = new htmlSelect('type', $options, $optionsSelected);
		$typeSelect->setHasDescriptiveElements(true);
		$typeTable->addElement($typeSelect);
		$typeTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'select', _('Ok')));
		$return->addElement($typeTable, true);
		$return->addElement(new htmlOutputText('&nbsp;', false), true);
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'membersBack', _('Back')));
		return $return;
	}
	
	/**
	* Processes user input of the members page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_user() {
		$return = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_user_remove']) && isset($_POST['members'])) {
			$members = array_flip($this->attributes['member']);
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				if (isset($members[$_POST['members'][$i]])) {
					unset($members[$_POST['members'][$i]]);
				}
			}
			$this->attributes['member'] = array_values(array_flip($members));
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_user_addMembers']) && isset($_POST['members'])) {
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				$this->attributes['member'][] = $_POST['members'][$i];
				$this->attributes['member'] = array_unique($this->attributes['member']);
			}
		}
		// check input
		if (!isset($_POST['form_subpage_' . get_class($this) . '_user_select'])) {
			if (!$this->isBooleanConfigOptionSet('groupOfNames_membersOptional')) {
				if (!isset($this->attributes['member']) || (sizeof($this->attributes['member']) < 1)) {
					$return[] = $this->messages['member'][0];
				}
			}
		}
		return $return;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('group', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'group';
		}
		return $messages;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	public function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Group name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'info', _('Notes'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		// members
		if (sizeof($this->attributes['member']) > 0) {
			$memberList = array();
			if (isset($this->attributes['member']) && is_array($this->attributes['member'])) {
				$memberList = $this->attributes['member'];
			}
			usort($memberList, 'compareDN');
			$return[get_class($this) . '_member'][0] = '<block><key>' . _('Members') . '</key><tr><td align=\"L\">' . $memberList[0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($memberList); $i++) {
				$return[get_class($this) . '_member'][] = '<block><tr><td align=\"L\">' . $memberList[$i] . '</td></tr></block>';
			}
		}
		return $return;
	}

}


?>
