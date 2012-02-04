<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2010  Pavel Pozdnyak
                2009 - 2012  Roland Gruber

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
* Manages Asterisk extensions.
*
* @package modules
*
* @author Pavel Pozdnyak
* @author Roland Gruber
*/

/**
 * Manages Asterisk extensions.
 *
 * @package modules
 */
class asteriskExtension extends baseModule {
	
	/** if true then only Asterisk accounts are shown as possible owners */
	private $filterOwnerForAsteriskAccounts = true;

	/** if true then the tree suffix will be used to search potential owners */
	private $searchOwnersInTreeSuffix = false;

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 */
	function get_metaData() {
		$return = array();
		// manages users accounts
		$return["account_types"] = array("asteriskExt");
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "normal");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=AsteriskExtension)"); 
		// alias name
		$return["alias"] = _("Asterisk extension");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('AsteriskExtension', 'groupOfNames');
		// managed attributes
		$return['attributes'] = array('cn', 'member', 'AstExtension', 'AstPriority',
			'AstApplication', 'AstApplicationData', 'AstContext');
		//ldap aliases
		$return['LDAPaliases'] = array('commonName' => 'cn');
		// icon
		$return['icon'] = 'asterisk.png';
		// help
		$return['help'] = array(
			'AstApplicationData' => array(
				"Headline" => _("Application data"), 'attr' => 'AstApplicationData',
				"Text" => _("This defines what application Asterisk has to start for this user (e.g. SIP/user1).")
			),			
			'AstContext' => array(
				"Headline" => _("Account context"), 'attr' => 'AstContext',
				"Text" => _("The account context stores information about the dial plan.")
			),
			'AstExtension' => array(
				"Headline" => _("Extension name"), 'attr' => 'AstExtension',
				"Text" => _("The name of the extension (e.g. voicemail or sip).")
			),
			'AstPriority' => array(
				"Headline" => _("Priority"), 'attr' => 'AstPriority',
				"Text" => _("The priority is a sequence number used to order the execution of commands. Commands are executed beginning with the lowest sequence number.")
			),
			'AstApplication' => array(
				"Headline" => _("Application"), 'attr' => 'AstApplication',
				"Text" => _("The name of a command to execute. The available commands may be listed at the Asterisk command line by executing \"core show applications\".")
			),
			'member' => array(
				"Headline" => _("Extension owners"), 'attr' => 'member',
				"Text" => _("DNs of the extension owners")
			),
			'cn' => array(
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("Common name of the Asterisk extension.")
			),
			'ownerOptions' => array(
				"Headline" => _("Options"),
				"Text" => _("These options change the list of potential extension owners below. You can select to show Asterisk accounts or all users. It is also possible to search the tree suffix if you have users which are not in the standard user suffix.")
			)
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Account context'), 'AsteriskExtension_AstContext', null, 'AstContext'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Priority'), 'AsteriskExtension_AstPriority', null, 'AstPriority'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Application'), 'AsteriskExtension_AstApplication', null, 'AstApplication'), true);
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings'] = array(
			'AsteriskExtension_AstContext' => 'AstContext',
			'AsteriskExtension_AstPriority' => 'AstPriority',
			'AsteriskExtension_AstApplication' => 'AstApplication',
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'AstApplicationData' => _('Application data'),
			'AstContext' => _('Account context'),
			'AstExtension' => _('Extension name'),
			'AstPriority' => _('Priority'),
			'AstApplication' => _('Application'),
			'member' => _('Extension owners'),
			'cn' => _('Common name')
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'asteriskExtension_AstExtension',
				'description' => _('Extension name'),
				'help' => 'AstExtension',
				'example' => '500',
				'required' => true
			),
			array(
				'name' => 'asteriskExtension_AstPriority',
				'description' => _('Priority'),
				'help' => 'AstPriority',
				'example' => '3',
				'required' => true
			),
			array(
				'name' => 'asteriskExtension_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => '500-1',
				'required' => true
			),
			array(
				'name' => 'asteriskExtension_AstApplication',
				'description' => _('Application'),
				'help' => 'AstApplication',
				'example' => 'Playback',
				'required' => true
			),
			array(
				'name' => 'asteriskExtension_AstApplicationData',
				'description' => _('Application data'),
				'help' => 'AstApplicationData',
				'example' => _('test-start'),
			),
			array(
				'name' => 'asteriskExtension_AstContext',
				'description' => _('Account context'),
				'help' => 'AstContext',
				'example' => _('test'),
				'required' => true
			),
			array(
				'name' => 'asteriskExtension_member',
				'description' => _('Extension owners'),
				'help' => 'member',
				'example' => 'uid=user1,o=test;uid=user2,o=test',
				'required' => true
			),
		);
		
		return $return;
	}


	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {		
		$this->messages['cn'][0] = array('ERROR', _('Please enter a common name.'));
		$this->messages['AstApplication'][0] = array('ERROR', _('Please enter the application.'));
		$this->messages['AstApplicationData'][0] = array('ERROR', _('Please enter the application data.'));
		$this->messages['AstContext'][0] = array('ERROR', _('Please enter the extension context.'));
		$this->messages['AstExtension'][0] = array('ERROR', _('Please enter the extension name.'));
		$this->messages['AstPriority'][0] = array('ERROR', _('Please enter the priority.'));
		$this->messages['AstExtensionAstPriority'][0] = array('ERROR', _('This pair of extension name and priority already exists.'));
		$this->messages['member'][0] = array('ERROR', _('Please add at least one member.'));
	}

	/**
	 * This function will create the meta HTML code to show a page with all attributes.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// cn
		$cn = '';
		if (isset($this->attributes['cn'][0])) {
			$cn = $this->attributes['cn'][0];
		}
		$cnInput = new htmlTableExtendedInputField(_("Common name"), 'cn', $cn, 'cn');
		$cnInput->setRequired(true);
		$return->addElement($cnInput, true);
		// account context
		$accountContext = '';
		if (isset($this->attributes['AstContext'][0])) {
			$accountContext = $this->attributes['AstContext'][0];
		}
		$accountContextInput = new htmlTableExtendedInputField(_("Account context"), 'AstContext', $accountContext, 'AstContext');
		$accountContextInput->setRequired(true);
		$return->addElement($accountContextInput, true);
		// extension name
		$extName = '';
		if (isset($this->attributes['AstExtension'][0])) {
			$extName = $this->attributes['AstExtension'][0];
		}
		$extNameInput = new htmlTableExtendedInputField(_("Extension name"), 'AstExtension', $extName, 'AstExtension');
		$extNameInput->setRequired(true);
		$return->addElement($extNameInput, true);
		// priority
		$priority = '';
		if (isset($this->attributes['AstPriority'][0])) {
			$priority = $this->attributes['AstPriority'][0];
		}
		$priorityInput = new htmlTableExtendedInputField(_("Priority"), 'AstPriority', $priority, 'AstPriority');
		$priorityInput->setRequired(true);
		$return->addElement($priorityInput, true);
		// application
		$application = '';
		if (isset($this->attributes['AstApplication'][0])) {
			$application = $this->attributes['AstApplication'][0];
		}
		$applicationInput = new htmlTableExtendedInputField(_("Application"), 'AstApplication', $application, 'AstApplication');
		$applicationInput->setRequired(true);
		$return->addElement($applicationInput, true);
		// application data
		$applicationData = '';
		if (isset($this->attributes['AstApplicationData'][0])) {
			$applicationData = $this->attributes['AstApplicationData'][0];
		}
		$return->addElement(new htmlTableExtendedInputField(_("Application data"), 'AstApplicationData', $applicationData, 'AstApplicationData'), true);
		// owners
		$return->addElement(new htmlOutputText(_("Extension owners") . '*'));
		$return->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Change')));
		$return->addElement(new htmlHelpLink('member'));
		$return->addNewLine();
		$return->addElement(new htmlOutputText(''));
		$ownerList = new htmlTable();
		if (isset($this->attributes['member'])) {
			for ($i = 0; $i < sizeof($this->attributes['member']); $i++) {
				$ownerList->addElement(new htmlOutputText(getAbstractDN($this->attributes['member'][$i])), true);
			}
		}
		$return->addElement($ownerList);
		return $return;
	}

	/**
	 * Displays a list of possible owners of this extension.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_user() {
		$return = new htmlTable();
		// load list with all potential owners
		$searchScope = 'user';
		if ($this->searchOwnersInTreeSuffix) {
			$searchScope = 'tree';
		}
		$searchClass = 'inetOrgPerson';
		if ($this->filterOwnerForAsteriskAccounts) {
			$searchClass = 'AsteriskSIPUser';
		}
		$entries = searchLDAPByAttribute(null, null, $searchClass, array('dn'), array($searchScope));
		$users_dn = array();
		for ($i = 0; $i < sizeof($entries); $i++) {
			$dn = $entries[$i]['dn'];
			if (isset($dn) && (!isset($this->attributes['member']) || !in_array($dn, $this->attributes['member']))) {
				$users_dn[getAbstractDN($dn)] = $dn;
			}
		}
		$memberList = $this->attributes['member'];
		$members = array();
		for ($i = 0; $i < sizeof($memberList); $i++) {
			$members[getAbstractDN($memberList[$i])] = $memberList[$i];
		}
		// options
		$return->addElement(new htmlTableExtendedInputCheckbox('filterAsteriskUsers', $this->filterOwnerForAsteriskAccounts, _('Show only Asterisk accounts'), 'ownerOptions', false));
		$refreshButton = new htmlButton('changeFilter', _('Refresh'));
		$refreshButton->rowspan = 2;
		$return->addElement($refreshButton, true);
		$return->addElement(new htmlTableExtendedInputCheckbox('useTreeSuffix', $this->searchOwnersInTreeSuffix, _('Search tree suffix for users'), 'ownerOptions', false), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		// owners
		$ownerContent = new htmlTable();
		$ownerContent->addElement(new htmlOutputText(_("Selected users")));
		$ownerContent->addElement(new htmlOutputText(""));
		$ownerContent->addElement(new htmlOutputText(_("Available users")));
		$ownerContent->addNewLine();
		$selectedList = new htmlSelect('removeusers', $members, null, 15);
		$selectedList->setHasDescriptiveElements(true);
		$selectedList->setMultiSelect(true);
		$selectedList->setRightToLeftTextDirection(true);
		$ownerContent->addElement($selectedList);
		$ownerButtons = new htmlTable();
		$ownerButtons->addElement(new htmlButton('addusers_button', 'back.gif', true), true);
		$ownerButtons->addElement(new htmlButton('removeusers_button', 'forward.gif', true));
		$ownerContent->addElement($ownerButtons);
		$availableList = new htmlSelect('addusers', $users_dn, null, 15);
		$availableList->setHasDescriptiveElements(true);
		$availableList->setMultiSelect(true);
		$availableList->setRightToLeftTextDirection(true);
		$ownerContent->addElement($availableList);
		$ownerContent->colspan = 4;
		$return->addElement(new htmlSubTitle(_("Extension owners")), true);
		$return->addElement($ownerContent, true);
		// back button
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Ok'));
		$backButton->colspan = 3;
		$return->addElement($backButton);
		return $return;
	}


	/**
	 * Processes user input of the user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_user() {
		if (isset($_POST['changeFilter'])) {
			// update filter value
			$this->filterOwnerForAsteriskAccounts = ($_POST['filterAsteriskUsers'] == 'on');
			// update search suffix
			$this->searchOwnersInTreeSuffix = ($_POST['useTreeSuffix'] == 'on');
		}
		if (!isset($this->attributes['member'])) $this->attributes['member'] = array();
		if (isset($_POST['addusers']) && isset($_POST['addusers_button'])) { // Add users to list
		// Add new user
			$this->attributes['member'] = @array_merge($this->attributes['member'], $_POST['addusers']);
		}
		elseif (isset($_POST['removeusers']) && isset($_POST['removeusers_button'])) { // remove users from list
			$this->attributes['member'] = array_delete($_POST['removeusers'], $this->attributes['member']);
		}
		return array();
	}


	/**
	 * Write variables into object and do some regex checks
	 *
	 * @param array $_POST HTTP-POST values
	 */
	function process_attributes() {
		$errors = array();
		$this->attributes['AstAccountCallerID'] = array();
		$this->attributes['AstContext'] = array();
		$this->attributes['AstExtension'] = array();
		$this->attributes['AstPriority'] = array();
		$this->attributes['AstApplication'] = array();
		$this->attributes['AstApplicationData'] = array();
		$this->attributes['cn'] = array();
		if (isset($_POST['AstContext'])) {
			$this->attributes['AstContext'][0] = $_POST['AstContext'];
			if($this->attributes['AstContext'][0] == '') {
				$errors[] = $this->messages['AstContext'][0];
			}
		}
		if (isset($_POST['AstExtension'])) {
			$this->attributes['AstExtension'][0] = $_POST['AstExtension'];
			if($this->attributes['AstExtension'][0] == '') {
				$errors[] = $this->messages['AstExtension'][0];
			}
		}
		if (isset($_POST['AstPriority'])) {
			$this->attributes['AstPriority'][0] = $_POST['AstPriority'];
			if($this->attributes['AstPriority'][0] == '') {
				$errors[] = $this->messages['AstPriority'][0];
			}
		}

		if (!isset($this->orig['AstExtension']) || !isset($this->orig['AstPriority']) || ($this->orig['AstExtension'][0] != $this->attributes['AstExtension'][0])
			|| ($this->orig['AstPriority'][0] != $this->attributes['AstPriority'][0])) {
			$filter = '(& (objectClass=AsteriskExtension) (AstExtension=' . $this->attributes['AstExtension'][0] .') (AstPriority=' . $this->attributes['AstPriority'][0] .') )';
			$entries = searchLDAPByFilter($filter, array('dn'), array($this->get_scope()));
			if (sizeof($entries) > 0) {
				$errors[] = $this->messages['AstExtensionAstPriority'][0];	
			}
		}

		if (isset($_POST['AstApplication'])) {
			$this->attributes['AstApplication'][0] = $_POST['AstApplication'];
			if($this->attributes['AstApplication'][0] == '') {
				$errors[] = $this->messages['AstApplication'][0];
			}
		}
		if (isset($_POST['AstApplicationData'])) {
			$this->attributes['AstApplicationData'][0] = $_POST['AstApplicationData'];
		}
		if (isset($_POST['cn'])) {
			$this->attributes['cn'][0] = $_POST['cn'];
			if($this->attributes['cn'][0] == '') {
				$errors[] = $this->messages['cn'][0];
			}
		}
		if (!isset($this->attributes['member']) || (sizeof($this->attributes['member']) == 0)) {
			if (!isset($_POST['form_subpage_' . get_class($this) . '_user_open'])) {
				$errors[] = $this->messages['member'][0];
			}
		}
		return $errors;
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
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("AsteriskExtension", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "AsteriskExtension";
			if (!in_array("groupOfNames", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "groupOfNames";
			// attributes
			$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['asteriskExtension_cn']];
			$partialAccounts[$i]['AstExtension'] = $rawAccounts[$i][$ids['asteriskExtension_AstExtension']];
			$partialAccounts[$i]['AstPriority'] = $rawAccounts[$i][$ids['asteriskExtension_AstPriority']];
			$partialAccounts[$i]['member'] = explode(';', $rawAccounts[$i][$ids['asteriskExtension_member']]);
			if ($rawAccounts[$i][$ids['asteriskExtension_AstApplication']] != "") {
				$partialAccounts[$i]['AstApplication'] = $rawAccounts[$i][$ids['asteriskExtension_AstApplication']];
			}
			if ($rawAccounts[$i][$ids['asteriskExtension_AstApplicationData']] != "") {
				$partialAccounts[$i]['AstApplicationData'] = $rawAccounts[$i][$ids['asteriskExtension_AstApplicationData']];
			}
			if ($rawAccounts[$i][$ids['asteriskExtension_AstContext']] != "") {
				$partialAccounts[$i]['AstContext'] = $rawAccounts[$i][$ids['asteriskExtension_AstContext']];
			}
		}
		return $messages;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (isset($this->attributes['AstContext'])) {
			$return[get_class($this) . '_AstContext'] = array('<block><key>' . _('Account context') . '</key><value>' . $this->attributes['AstContext'][0] . '</value></block>');
		}
		if (isset($this->attributes['AstApplicationData'])) {
			$return[get_class($this) . '_AstApplicationData'] = array('<block><key>' . _('Application data') . '</key><value>' . $this->attributes['AstApplicationData'][0] . '</value></block>');
		}
		if (isset($this->attributes['AstApplication'])) {
			$return[get_class($this) . '_AstApplication'] = array('<block><key>' . _('Application') . '</key><value>' . $this->attributes['AstApplication'][0] . '</value></block>');
		}
		if (isset($this->attributes['AstPriority'])) {
			$return[get_class($this) . '_AstPriority'] = array('<block><key>' . _('Priority') . '</key><value>' . $this->attributes['AstPriority'][0] . '</value></block>');
		}
		if (isset($this->attributes['AstExtension'])) {
			$return[get_class($this) . '_AstExtension'] = array('<block><key>' . _('Extension name') . '</key><value>' . $this->attributes['AstExtension'][0] . '</value></block>');
		}
		if (isset($this->attributes['member'])) {
			$return[get_class($this) . '_member'] = array('<block><key>' . _('Extension owners') . '</key><value>' . implode('; ', $this->attributes['member']) . '</value></block>');
		}
		if (isset($this->attributes['cn'])) {
			$return[get_class($this) . '_cn'] = array('<block><key>' . _('Common name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>');
		}
		return $return;
	}

}
?>
