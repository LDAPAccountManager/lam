<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2016 - 2017  Roland Gruber

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
* Manages memberships in NIS net groups.
*
* @package modules
* @author Roland Gruber
*/

/** include parent class */
include_once("nisNetGroupUser.inc");

/**
* Manages memberships in NIS net groups.
*
* @package modules
*/
class nisNetGroupHost extends nisNetGroupUser {

	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return in_array($this->get_scope(), array('host'));
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	public function get_metaData() {
		$return = parent::get_metaData();
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('account', 'posixAccount')), 'conflicts' => array());
		// upload columns
		$return['upload_columns'] = array(array(
			'name' => 'nisNetGroup_memberships',
			'description' => _('Memberships'),
			'help' => 'memberships_upload',
			'example' => 'group1#user#domain,group2#user#domain'
		));
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		parent::load_Messages();
		$this->messages['user'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
	}

	/**
	 * Checks if the netgroup matches this entry.
	 *
	 * @param String $user netgroup user name
	 * @param String $host netgroup host name
	 * @param String $domain netgroup domain name
	 * @param String $uid user name of this entry
	 */
	protected function isMatchingNetGroup($user, $host, $domain, $uid) {
		return $host == $uid;
	}

	/**
	* Displays the group selection.
	*
	* @return htmlElement meta HTML code
	*/
	public function display_html_attributes() {
		$return = new htmlTable();
		$return->addElement(new htmlOutputText(_('Group')));
		$return->addElement(new htmlOutputText(_('User name')));
		$return->addElement(new htmlOutputText(_('Domain name')), true);
		for ($i = 0; $i < sizeof($this->groups); $i++) {
			$group = $this->groups[$i];
			$return->addElement(new htmlOutputText($group['name']));
			$return->addElement(new htmlInputField('user_' . $i, $group['user']));
			$return->addElement(new htmlInputField('domain_' . $i, $group['domain']));
			$delButton = new htmlButton('del_' . $i, 'del.png', true);
			$delButton->setTitle(_('Delete'));
			$return->addElement($delButton, true);
		}
		$return->addVerticalSpace('40px');

		// new entry
		$groupList = array();
		$groupData = $this->findGroups();
		if (sizeof($groupData) > 0) {
			$filterGroup = new htmlGroup();
			$filterGroup->addElement(new htmlOutputText(_('Filter') . ' '));
			$filter = new htmlInputField('group_filter');
			$filter->setFieldSize('5em');
			$filter->filterSelectBox('group_add');
			$filterGroup->addElement($filter);
			$return->addElement($filterGroup, true);

			foreach ($groupData as $group) {
				$groupList[$group['cn'][0]] = $group['cn'][0] . '#+#' . $group['dn'];
			}
			$groupSelect = new htmlSelect('group_add', $groupList);
			$groupSelect->setHasDescriptiveElements(true);
			$return->addElement($groupSelect);
			$return->addElement(new htmlInputField('user_add'));
			$return->addElement(new htmlInputField('domain_add'));
			$addButton = new htmlButton('addGroup', 'add.png', true);
			$addButton->setTitle(_('Add'));
			$return->addElement($addButton, true);
		}
		return $return;
	}

	/**
	* Processes user input of the group selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$errors = array();
		// add new entry
		if (isset($_POST['addGroup'])) {
			$parts = explode('#+#', $_POST['group_add']);
			$this->groups[] = array(
				'name' => $parts[0],
				'dn' => $parts[1],
				'user' => $_POST['user_add'],
				'host' => $this->uidOrig,
				'domain' => $_POST['domain_add']
			);
			if (!empty($_POST['user_add']) && !get_preg($_POST['user_add'], 'username')) {
				$message = $this->messages['user'][0];
				$message[2] = $message[2] . '<br><br>' . $_POST['user_add'];
				$errors[] = $message;
			}
			if (!empty($_POST['domain_add']) && !get_preg($_POST['domain_add'], 'DNSname')) {
				$message = $this->messages['domain'][0];
				$message[2] = $message[2] . '<br><br>' . $_POST['domain_add'];
				$errors[] = $message;
			}
		}
		// check existing
		$counter = 0;
		while (isset($_POST['user_' . $counter])) {
			if (isset($_POST['del_' . $counter])) {
				unset($this->groups[$counter]);
			}
			else {
				$this->groups[$counter]['user'] = $_POST['user_' . $counter];
				if (!empty($_POST['user_' . $counter]) && !get_preg($_POST['user_' . $counter], 'username')) {
					$message = $this->messages['user'][0];
					$message[2] = $message[2] . '<br><br>' . $_POST['user_' . $counter];
					$errors[] = $message;
				}
				$this->groups[$counter]['domain'] = $_POST['domain_' . $counter];
				if (!empty($_POST['domain_' . $counter]) && !get_preg($_POST['domain_' . $counter], 'DNSname')) {
					$message = $this->messages['domain'][0];
					$message[2] = $message[2] . '<br><br>' . $_POST['domain_' . $counter];
					$errors[] = $message;
				}
			}
			$counter++;
		}
		$this->groups = array_values($this->groups);
		usort($this->groups, array($this, 'sortTriple'));
		return $errors;
	}

	/**
	 * Creates a netgroup triple from a group object.
	 *
	 * @param String $group group object
	 * @param String $uid own uid
	 */
	protected function createNetGroupValue($group, $uid) {
		return '(' . $uid . ',' . $group['user'] . ',' . $group['domain'] . ')';
	}

	/**
	* {@inheritDoc}
	*/
	function get_profileOptions($typeId) {
		$groups = $this->findGroups();
		$groupOptions = array('' => '');
		foreach ($groups as $group) {
			$groupOptions[$group['cn'][0]] = $group['cn'][0] . '#+#' . $group['dn'];
		}
		$return = new htmlTable();
		$return->addElement(new htmlOutputText(_('Group')));
		$return->addElement(new htmlOutputText(_('User name')));
		$return->addElement(new htmlOutputText(_('Domain name')), true);
		for ($i = 0; $i < 5; $i++) {
			$select = new htmlSelect('nisNetGroupUser_group' . $i, $groupOptions, array(''));
			$select->setHasDescriptiveElements(true);
			$return->addElement($select);
			$return->addElement(new htmlInputField('nisNetGroupUser_user' . $i));
			$return->addElement(new htmlInputField('nisNetGroupUser_domain' . $i), true);
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		for ($i = 0; $i < 5; $i++) {
			if (!empty($profile['nisNetGroupUser_group' . $i][0])) {
				$parts = explode('#+#', $profile['nisNetGroupUser_group' . $i][0]);
				$this->groups[] = array(
					'name' => $parts[0],
					'dn' => $parts[1],
					'user' => $profile['nisNetGroupUser_user' . $i][0],
					'host' => $this->uidOrig,
					'domain' => $profile['nisNetGroupUser_domain' . $i][0],
				);
			}
		}
		usort($this->groups, array($this, 'sortTriple'));
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$pdfTable = new PDFTable();
		$pdfRow = new PDFTableRow();
		$pdfRow->cells[] = new PDFTableCell(_('Group'), '25%', null, true);
		$pdfRow->cells[] = new PDFTableCell(_('User name'), '25%', null, true);
		$pdfRow->cells[] = new PDFTableCell(_('Domain name'), '25%', null, true);
		$pdfTable->rows[] = $pdfRow;
		foreach ($this->groups as $group) {
			$pdfRow = new PDFTableRow();
			$pdfRow->cells[] = new PDFTableCell($group['name'], '25%');
			$pdfRow->cells[] = new PDFTableCell($group['user'], '25%');
			$pdfRow->cells[] = new PDFTableCell($group['domain'], '25%');
			$pdfTable->rows[] = $pdfRow;
		}
		$this->addPDFTable($return, 'memberships', $pdfTable);
		return $return;
	}

	/**
	 * Creates a netgroup triple from the input value of file upload.
	 *
	 * @param array $value upload value (e.g. array(group1, host, domain))
	 * @param String $uid own uid
	 * @return String netgroup triple
	 */
	protected function buildNetGroupTripleFromUploadValue($value, $uid) {
		$user = empty($value[1]) ? '' : $value[1];
		$domain = empty($value[2]) ? '' : $value[2];
		return '(' . $uid . ',' . $user . ',' . $domain . ')';
	}

}