<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2015  Roland Gruber

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
* Manages PyKota billing codes.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages PyKota billing codes.
*
* @package modules
*/
class pykotaBillingCode extends baseModule {

	/** cache for existing codes (array(dn1 => pykotaBillingCode1, dn2 => pykotaBillingCode2)) */
	private $codeCache = null;

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('pykotaBillingCodeType'));
	}

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
		$return['icon'] = 'printerBig.png';
		// alias name
		$return["alias"] = _("PyKota");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=pykotaBilling)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('pykotaObject', 'pykotaBilling');
		// managed attributes
		$return['attributes'] = array('cn', 'pykotaBillingCode', 'description', 'pykotaBalance', 'pykotaPageCounter');
		// help Entries
		$return['help'] = array(
			'pykotaBillingCode' => array(
				"Headline" => _("Billing code"), 'attr' => 'pykotaBillingCode',
				"Text" => _("Billing code name which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ .")
			),
			'description' => array (
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Billing code description.")
			),
			'pykotaBalance' => array (
				"Headline" => _('Balance'), 'attr' => 'pykotaBalance',
				"Text" => _('Used balance for the billing code.')
			),
			'pykotaPageCounter' => array (
				"Headline" => _('Page count'), 'attr' => 'pykotaPageCounter',
				"Text" => _('Number of pages printed with this billing code.')
			),
			'reset' => array (
				"Headline" => _('Reset'), 'attr' => 'pykotaBalance, pykotaPageCounter',
				"Text" => _('Resets the billing code\'s balance and page counter to 0.')
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'pykotaBillingCode_pykotaBillingCode',
				'description' => _('Printer name'),
				'help' => 'cn',
				'example' => _('billingCode01'),
				'required' => true,
				'unique' => true,
			),
			array(
				'name' => 'pykotaBillingCode_description',
				'description' => _('Description'),
				'help' => 'description',
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'pykotaBillingCode' => _('Billing code'),
			'description' => _('Description'),
			'pykotaBalance' => _('Balance'),
			'pykotaPageCounter' => _('Page count'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['pykotaBillingCode'][0] = array('ERROR', _('Billing code'), _('Billing code contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaBillingCode'][1] = array('ERROR', _('Account %s:') . ' pykotaBillingCode_cn', _('Billing code contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaBillingCode'][2] = array('ERROR', _('Billing code'), _('Billing code already exists!'));
		$this->messages['pykotaBillingCode'][3] = array('ERROR', _('Account %s:') . ' pykotaBillingCode_cn', _('Billing code already exists!'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		// pykotaBillingCode
		$this->addSimpleInputTextField($container, 'pykotaBillingCode', _('Billing code'), true);
		// balance
		$container->addElement(new htmlOutputText(_('Balance')));
		$pykotaBalance = '';
		if (isset($this->attributes['pykotaBalance'][0])) {
			$pykotaBalance = $this->attributes['pykotaBalance'][0];
		}
		$container->addElement(new htmlOutputText($pykotaBalance));
		$container->addElement(new htmlHelpLink('pykotaBalance'), true);
		// page count
		$container->addElement(new htmlOutputText(_('Page count')));
		$pykotaPageCounter = '';
		if (isset($this->attributes['pykotaPageCounter'][0])) {
			$pykotaPageCounter = $this->attributes['pykotaPageCounter'][0];
		}
		$container->addElement(new htmlOutputText($pykotaPageCounter));
		$container->addElement(new htmlHelpLink('pykotaPageCounter'), true);
		// description
		$this->addSimpleInputTextField($container, 'description', _('Description'), false, null, true);
		// reset
		$container->addElement(new htmlSpacer(null, '20px'), true);
		$container->addElement(new htmlOutputText(''));
		$container->addElement(new htmlButton('resetCounters', _('Reset')));
		$container->addElement(new htmlHelpLink('reset'), true);
		// same width
		$container->addElement(new htmlEqualWidth(array('pykotaBillingCode', 'description')));
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// pykotaBillingCode
		if (isset($_POST['pykotaBillingCode']) && ($_POST['pykotaBillingCode'] != '')) {
			if (!get_preg($_POST['pykotaBillingCode'], 'username')) {
				$errors[] = $this->messages['pykotaBillingCode'][0];
			}
			else {
				$this->attributes['pykotaBillingCode'][0] = $_POST['pykotaBillingCode'];
				$this->attributes['cn'][0] = $_POST['pykotaBillingCode'];
				if ((!isset($this->orig['pykotaBillingCode'][0]) || ($this->attributes['pykotaBillingCode'][0] != $this->orig['pykotaBillingCode'][0]))
					&& $this->codeExists($_POST['pykotaBillingCode'])) {
					$errors[] = $this->messages['pykotaBillingCode'][2];
				}
			}
		}
		else {
			if (isset($this->attributes['cn'][0])) {
				unset($this->attributes['cn'][0]);
			}
			if (isset($this->attributes['pykotaBillingCode'][0])) {
				unset($this->attributes['pykotaBillingCode'][0]);
			}
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// reset
		if (isset($_POST['resetCounters'])) {
			$this->attributes['pykotaBalance'][0] = '0.0';
			$this->attributes['pykotaPageCounter'][0] = '0';
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		$this->loadCodeCache();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('pykotaBilling', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaBilling';
			}
			if (!in_array('pykotaObject', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaObject';
			}
			// pykotaBillingCode
			if (!get_preg($rawAccounts[$i][$ids['pykotaBillingCode_pykotaBillingCode']], 'username')) {
				$errMsg = $this->messages['pykotaBillingCode'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			elseif ($this->codeExists($rawAccounts[$i][$ids['pykotaBillingCode_pykotaBillingCode']])) {
				$errMsg = $this->messages['pykotaBillingCode'][3];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			else {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['pykotaBillingCode_pykotaBillingCode']];
				$partialAccounts[$i]['pykotaBillingCode'] = $rawAccounts[$i][$ids['pykotaBillingCode_pykotaBillingCode']];
			}
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaBillingCode_description', 'description');
			// balance
			$partialAccounts[$i]['pykotaBalance'] = '0.0';
			// page count
			$partialAccounts[$i]['pykotaPageCounter'] = '0';
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->loadCodeCache();
		$this->addSimplePDFField($return, 'pykotaBillingCode', _('Billing code'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'pykotaBalance', _('Balance'));
		$this->addSimplePDFField($return, 'pykotaPageCounter', _('Page count'));
		return $return;
	}

	/**
	 * Returns if the given billing code already exists.
	 *
	 * @param String $code pykotaBillingCode attribute value
	 * @return boolean pykotaBillingCode exists
	 */
	private function codeExists($code) {
		if ($this->codeCache == null) {
			$this->loadCodeCache();
		}
		foreach ($this->codeCache as $dn => $bCode) {
			if (!empty($bCode) && ($bCode == $code)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Loads the list of billing code names into the cache.
	 */
	private function loadCodeCache() {
		if ($this->codeCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(objectClass=pykotaBilling)', array('pykotaBillingCode', 'dn'), array($this->get_scope()));
		$this->codeCache = array();
		foreach ($results as $result) {
			if (isset($result['pykotabillingcode'][0])) {
				$this->codeCache[$result['dn']] = $result['pykotabillingcode'][0];
			}
		}
	}

}


?>
