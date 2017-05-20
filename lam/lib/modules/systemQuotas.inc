<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2017  Roland Gruber

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
* Manages user quotas with the object class systemQuotas.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages user quotas with the object class systemQuotas.
*
* @package modules
*/
class systemQuotas extends baseModule {

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user'));
	}

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
		$return['icon'] = 'hard-driveBig.png';
		// alias name
		$return["alias"] = _("Quota");
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('systemQuotas');
		// managed attributes
		$return['attributes'] = array('quota');
		// help Entries
		$return['help'] = array(
			'quota' => array(
				"Headline" => _("Quota"), 'attr' => 'quota',
				"Text" => _("Please enter the quota settings for this user. The syntax is: {mount point},{soft block limit},{hard block limit},{soft inode limit},{hard inode limit}.")
							. ' ' . _('Multiple values are separated by semicolon.')
			)
		);
		// profile elements
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Quota'), 'systemQuotas_quota', null, 'quota'));
		$return['profile_options'] = $profileContainer;
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'systemQuotas_quota',
				'description' => _('Quota'),
				'help' => 'quota',
				'example' => '/home/smiller,50000,60000,10000,12000',
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'quota' => _('Quota')
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['path'][0] = array('ERROR', _('Mountpoint'), _('Mountpoint contains invalid characters.'));
		$this->messages['path'][1] = array('ERROR', _('Account %s:'), _('Mountpoint contains invalid characters.'));
		$this->messages['softblock'][0] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softblock'][1] = array('ERROR', _('Account %s:'), _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardblock'][0] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardblock'][1] = array('ERROR', _('Account %s:'), _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softinode'][0] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['softinode'][1] = array('ERROR', _('Account %s:'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardinode'][0] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['hardinode'][1] = array('ERROR', _('Account %s:'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
		$this->messages['block_cmp'][0] = array('ERROR', _('Block quota'), _('Block soft quota must be smaller than block hard quota.'));
		$this->messages['block_cmp'][1] = array('ERROR', _('Account %s:'), _('Block soft quota must be smaller than block hard quota.'));
		$this->messages['inode_cmp'][0] = array('ERROR', _('Inode quota'), _('Inode soft quota must be smaller than inode hard quota.'));
		$this->messages['inode_cmp'][1] = array('ERROR', _('Account %s:'), _('Inode soft quota must be smaller than inode hard quota.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$container = new htmlTable();
		$spacer = new htmlSpacer('10px', null);
		// caption
		$container->addElement(new htmlOutputText(_('Mountpoint')));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Soft block limit')));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Hard block limit')));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Soft inode limit')));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Hard inode limit')), true);
		// existing entries
		if (isset($this->attributes['quota'][0])) {
			natcasesort($this->attributes['quota']);
			$this->attributes['quota'] = array_values($this->attributes['quota']);
			for ($i = 0; $i < sizeof($this->attributes['quota']); $i++) {
				$parts = explode(',', $this->attributes['quota'][$i]);
				$container->addElement(new htmlInputField('path_' . $i, $parts[0], 20));
				$container->addElement($spacer);
				$softBlockInput = new htmlInputField('softBlock_' . $i, $parts[1], 10);
				$softBlockInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$container->addElement($softBlockInput);
				$container->addElement($spacer);
				$hardBlockInput = new htmlInputField('hardBlock_' . $i, $parts[2], 10);
				$hardBlockInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$container->addElement($hardBlockInput);
				$container->addElement($spacer);
				$softInodeInput = new htmlInputField('softInode_' . $i, $parts[3], 10);
				$softInodeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$container->addElement($softInodeInput);
				$container->addElement($spacer);
				$hardInodeInput = new htmlInputField('hardInode_' . $i, $parts[4], 10);
				$hardInodeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$container->addElement($hardInodeInput);
				$container->addElement(new htmlButton('del_' . $i, 'del.png', true), true);
			}
		}
		// new entry
		$container->addElement(new htmlInputField('path', null, 20));
		$container->addElement($spacer);
		$newSoftBlockInput = new htmlInputField('softBlock', 0, 10);
		$newSoftBlockInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$container->addElement($newSoftBlockInput);
		$container->addElement($spacer);
		$newHardBlockInput = new htmlInputField('hardBlock', 0, 10);
		$newHardBlockInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$container->addElement($newHardBlockInput);
		$container->addElement($spacer);
		$newSoftInodeInput = new htmlInputField('softInode', 0, 10);
		$newSoftInodeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$container->addElement($newSoftInodeInput);
		$container->addElement($spacer);
		$newHardInodeInput = new htmlInputField('hardInode', 0, 10);
		$newHardInodeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$container->addElement($newHardInodeInput);
		$container->addElement(new htmlButton('add', 'add.png', true));
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
		if (!isset($this->attributes['quota'][0])) {
			$this->attributes['quota'] = array();
		}
		// check existing entries
		for ($i = 0; $i < sizeof($this->attributes['quota']); $i++) {
			if (isset($_POST['del_' . $i])) {
				unset($this->attributes['quota'][$i]);
				$this->attributes['quota'] = array_values($this->attributes['quota']);
				$i--;
				continue;
			}
			$path = $_POST['path_' . $i];
			$softBlock = $_POST['softBlock_' . $i];
			if ($softBlock == '') $softBlock = '0';
			$hardBlock = $_POST['hardBlock_' . $i];
			if ($hardBlock == '') $hardBlock = '0';
			$softInode = $_POST['softInode_' . $i];
			if ($softInode == '') $softInode = '0';
			$hardInode = $_POST['hardInode_' . $i];
			if ($hardInode == '') $hardInode = '0';
			$this->attributes['quota'][$i] = $path . ',' . $softBlock . ',' . $hardBlock . ',' .
											$softInode . ',' . $hardInode;
			$return = array_merge($return, $this->checkQuota($path, $softBlock, $hardBlock, $softInode, $hardInode));
		}
		// check for new entry
		if (isset($_POST['add'])) {
			$path = $_POST['path'];
			$softBlock = $_POST['softBlock'];
			if ($softBlock == '') $softBlock = '0';
			$hardBlock = $_POST['hardBlock'];
			if ($hardBlock == '') $hardBlock = '0';
			$softInode = $_POST['softInode'];
			if ($softInode == '') $softInode = '0';
			$hardInode = $_POST['hardInode'];
			if ($hardInode == '') $hardInode = '0';
			$this->attributes['quota'][] = $path . ',' . $softBlock . ',' . $hardBlock . ',' .
											$softInode . ',' . $hardInode;
			$return = array_merge($return, $this->checkQuota($path, $softBlock, $hardBlock, $softInode, $hardInode));
		}
		$this->attributes['quota'] = array_unique($this->attributes['quota']);
		return $return;
	}

	/**
	 * Checks if the quota parameters are valid.
	 *
	 * @param String $path mountpoint
	 * @param int $softBlock soft block limit
	 * @param int $hardBlock hard block limit
	 * @param int $softInode soft inode limit
	 * @param int $hardInode hard inode limit
	 * @param boolean $uploadIndex position is upload table
	 * @return array array where error messages are returned
	 */
	private function checkQuota($path, $softBlock, $hardBlock, $softInode, $hardInode, $uploadIndex = null) {
		$return = array();
		if (!get_preg($path, 'filePath')) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['path'][0];
			}
			else {
				$error = $this->messages['path'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if (!get_preg($softBlock, 'digit')) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['softblock'][0];
			}
			else {
				$error = $this->messages['softblock'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if (!get_preg($hardBlock, 'digit')) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['hardblock'][0];
			}
			else {
				$error = $this->messages['hardblock'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if (!get_preg($softInode, 'digit')) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['softinode'][0];
			}
			else {
				$error = $this->messages['softinode'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if (!get_preg($hardInode, 'digit')) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['hardinode'][0];
			}
			else {
				$error = $this->messages['hardinode'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if ($softBlock > $hardBlock) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['block_cmp'][0];
			}
			else {
				$error = $this->messages['block_cmp'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		if ($softInode > $hardInode) {
			if ($uploadIndex == null) {
				$return[] = $this->messages['inode_cmp'][0];
			}
			else {
				$error = $this->messages['inode_cmp'][1];
				$error[] = array($uploadIndex);
				$return[] = $error;
			}
		}
		return $return;
	}

	/**
	* {@inheritDoc}
	*/
	function check_profileOptions($options, $typeId) {
		$messages = parent::check_profileOptions($options, $typeId);
		$quotas = explode(';', $options['systemQuotas_quota'][0]);
		for ($q = 0; $q < sizeof($quotas); $q++) {
			if ($quotas[$q] == '') {
				continue;
			}
			$parts = explode(',', $quotas[$q]);
			$messages = array_merge($messages, $this->checkQuota($parts[0], $parts[1], $parts[2], $parts[3], $parts[4]));
		}
		return $messages;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		if (isset($profile['systemQuotas_quota'][0]) && ($profile['systemQuotas_quota'][0] != '')) {
			$this->attributes['quota'] = explode(';', $profile['systemQuotas_quota'][0]);
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('systemQuotas', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'systemQuotas';
			// add quota
			if (isset($rawAccounts[$i][$ids['systemQuotas_quota']]) && ($rawAccounts[$i][$ids['systemQuotas_quota']] != '')) {
				$quotas = explode(';', $rawAccounts[$i][$ids['systemQuotas_quota']]);
				for ($q = 0; $q < sizeof($quotas); $q++) {
					$parts = explode(',', $quotas[$q]);
					$messages = array_merge($messages, $this->checkQuota($parts[0], $parts[1], $parts[2], $parts[3], $parts[4], $i));
					$partialAccounts[$i]['quota'][] = $quotas[$q];
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		if (isset($this->attributes['quota'][0])) {
			$pdfTable = new PDFTable();
			$pdfRow = new PDFTableRow();
			$pdfRow->cells[] = new PDFTableCell(_('Mountpoint'), '28%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Soft block'), '18%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Hard block'), '18%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Soft inode'), '18%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Hard inode'), '18%', null, true);
			$pdfTable->rows[] = $pdfRow;
			for ($i = 0; $i < sizeof($this->attributes['quota']); $i++) {
				$parts = explode(',', $this->attributes['quota'][$i]);
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($parts[0], '28%');
				$pdfRow->cells[] = new PDFTableCell($parts[1], '18%');
				$pdfRow->cells[] = new PDFTableCell($parts[2], '18%');
				$pdfRow->cells[] = new PDFTableCell($parts[3], '18%');
				$pdfRow->cells[] = new PDFTableCell($parts[4], '18%');
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'quota', $pdfTable);
		}
		return $return;
	}

}


?>
