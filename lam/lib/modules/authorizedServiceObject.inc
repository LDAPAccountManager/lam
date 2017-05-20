<?php
/*
$Id$

  This code is not yet part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011         J de Jong
  				2012 - 2015  Roland Gruber

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
* Provides Authorized Service for accounts.
*
* @package modules
* @author J de Jong
*/

/**
* Provides Authorized Service for accounts.
*
* @package modules
*/
class authorizedServiceObject extends baseModule {

	/**
	* Creates a new authorizedServiceObject object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}


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
    */
    function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'services.png';
		// alias name
		$return["alias"] = _("Authorized Services");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('authorizedServiceObject');
		// managed attributes
		$return['attributes'] = array('authorizedService');
		// help Entries
		$return['help'] = array (
			'authorizedService' => array (
				"Headline" => _("Authorized Services"), 'attr' => 'authorizedService',
				"Text" => _("Service name (e.g. sshd, imap, ftp). Enter one service per entry."). ' '. _("Use * for all services.")
			),
			'authorizedServices' => array (
				"Headline" => _("Authorized Services"), 'attr' => 'authorizedService',
				"Text" => _("Comma separated list of services (e.g. sshd, imap, ftp)."). ' '. _("Use * for all services.")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
			'predefinedServices' => array(
				"Headline" => _("Predefined services"),
				"Text" => _("These services will show up as hint if you enter a new service.")
			)
		);
		// config options
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlTableExtendedInputTextarea('authorizedServiceObject_services', "sshd\r\nimap", 30, 5, _('Predefined services'), 'predefinedServices'));
		$return['config_options']['all'] = $configContainer;
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'authorizedService',
				'description' => _('Authorized Services'),
				'help' => 'authorizedServices',
				'example' => 'sshd, imap'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'authorizedService' => _('Authorized Services')
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Authorized Services'), 'authorizedServiceObject_services', null, 'authorizedServices'), true);
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('authorizedServiceObject_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['authorizedServiceObject_services'] = array('type' => 'ext_preg', 'regex' => 'ascii',
		'error_message' => $this->messages['authorizedService'][0]);
		return $return;
    }

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['authorizedService'][0] = array('ERROR', _('Authorized services are invalid.'));  // third array value is set dynamically
		$this->messages['authorizedService'][1] = array('ERROR', _('Account %s:') . ' authorizedService', _('Please enter a valid list of service names.'));
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
	* <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	*/
	function save_attributes() {
		if (!in_array('authorizedServiceObject', $this->attributes['objectClass']) && !in_array('authorizedServiceObject', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_authorizedServiceObject_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'authorizedServiceObject';
		}
		$return = new htmlTable();
		if (in_array('authorizedServiceObject', $this->attributes['objectClass'])) {
			$autocompleteValues = array();
			if (isset($this->moduleSettings['authorizedServiceObject_services'])) {
				$autocompleteValues = $this->moduleSettings['authorizedServiceObject_services'];
			}
			$this->addMultiValueInputTextField($return, 'authorizedService', _('Authorized Services'), false, null, false, $autocompleteValues);
			$return->addElement(new htmlSpacer(null, '10px'),true);
			$remButton = new htmlAccountPageButton('authorizedServiceObject', 'attributes', 'remObjectClass', _('Remove Authorized Service extension'));
			$remButton->colspan = 4;
			$return->addElement($remButton);
		}
		else {
			$return->addElement(new htmlAccountPageButton('authorizedServiceObject', 'attributes', 'addObjectClass', _('Add Authorized Service extension')));
		}


		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (isset($_POST['form_subpage_authorizedServiceObject_attributes_remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('authorizedServiceObject'), $this->attributes['objectClass']);
			if (isset($this->attributes['authorizedService'])) unset($this->attributes['authorizedService']);
			return array();
		}
		if (!in_array('authorizedServiceObject', $this->attributes['objectClass'])) {
			return array();
		}

		$errors = array();
		$this->processMultiValueInputTextField('authorizedService', $errors, 'ascii');
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("authorizedServiceObject", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "authorizedServiceObject";
			// add ASs
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'authorizedService', 'authorizedService', 'ascii', $this->messages['authorizedService'][1], $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		if (in_array('authorizedServiceObject', $this->attributes['objectClass'])) {
			$this->addSimplePDFField($return, 'authorizedService', _('Authorized Services'));
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['authorizedServiceObject_addExt'][0]) && ($profile['authorizedServiceObject_addExt'][0] == "true")) {
			if (!in_array('authorizedServiceObject', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'authorizedServiceObject';
			}
		}
		// add ASs
		if (isset ($profile['authorizedServiceObject_services'][0]) && ($profile['authorizedServiceObject_services'][0] != "")) {
			$services = explode(',', $profile['authorizedServiceObject_services'][0]);
			for ($m = 0; $m < sizeof($services); $m++) {
				if (get_preg($services[$m], 'ascii')) {
					$this->attributes['authorizedService'][] = trim($services[$m]);
				}
			}
		}
	}

}

?>
