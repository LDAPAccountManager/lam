<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2024  Roland Gruber

*/

/**
 * Manages TAK User roles.
 *
 * @package modules
 * @author Mark Halliday
 * Copyright (C) 2024  Mark Halliday
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

use LAM\TYPES\ConfiguredType;

/**
 * Manages TAK User roles.
 *
 * @package modules
 */
class takUser extends baseModule {

	/** regex for date values */
	const DATE_REGEX = '/^([\d]{1,2})\\.([\d]{1,2})\\.([\d]{4})( ([\d]{1,2}):([\d]{1,2}))?$/';

	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return in_array($this->get_scope(), ['user']);
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'tak.png';
		// alias name
		$return["alias"] = _("TAK");
		// this is a base module
		$return["is_base"] = false;
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=takUser)"];
		// RDN attribute
		$return["RDN"] = ["cn" => "normal"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['takUser'];
		// managed attributes
		$return['attributes'] = ['takCallsign', 'takRole', 'takColor'
        ];
		// help Entries
		$return['help'] = [
			'takCallsign' => [
				"Headline" => _("TAK callsign"), 'attr' => 'takCallsign',
				"Text" => _("The user's callsign to be displayed to other TAK users.")
			],
			'takRole' => [
				"Headline" => _("TAK team role"), 'attr' => 'takRole',
				"Text" => _("The user's role to be displayed to other TAK users.")
			],
			'takColor' => [
				"Headline" => _("TAK Team color"), 'attr' => 'takColor',
				"Text" => _("The user's team color to be displayed to other TAK users.")
            ],
		];
		// upload fields
		$return['upload_columns'] = [
            [
				'name' => 'takUser_takCallsign',
				'description' => _('TAK callsign'),
				'help' => 'takCallsign',
				'example' => _('UK-ORG-01'),
                'required' => true
			],
            [
				'name' => 'takUser_takRole',
				'description' => _('TAK team role'),
				'help' => 'takRole',
                'default' => 'Team Member',
				'values' => 'Team Member, Team Leader, HQ, Sniper, Medic, Forward Observer, RTO, K9',
				'example' => _('Team Member'),
                'required' => true
			],
            [
				'name' => 'takUser_takColor',
				'description' => _('TAK team color'),
				'help' => 'takColor',
                'default' => 'Cyan',
				'values' => 'Blue, Brown, Cyan, Dark Blue, Dark Green, Green, Magenta, Maroon, Orange, Purple, Red, Teal, White, Yellow',
				'example' => _('Cyan'),
                'required' => true
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'takCallsign' => _('TAK callsign'),
			'takRole' => _('TAK team role'),
			'takColor' => _('TAK team color'),
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('TAK Callsign'), 'takUser_takCallsign', null, 'takUserCallsign'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('TAK Team Role'), 'takUser_takRole', null, 'takUserRole'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('TAK Team Color'), 'takUser_takColor', null, 'takUserColor'), 12);
		$return['profile_options'] = $profileContainer;
		/**
		* not working: will not allow -
		* // profile checks
		* $return['profile_checks']['takUser_takCallsign'] = [
		*	'type' => 'ext_preg',
		*	'regex' => 'ascii',
		*	'error_message' => $this->messages['takCallsign'][0]];
		*/
		return $return;
	}

	/**
	 * This function fills the $messages variable with output messages from this module.
	 */
	function load_Messages() {
		$this->messages['takCallsign'][0] = ['ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_')];
		$this->messages['takCallsign'][1] = ['ERROR', _('Please enter a unique callsign.')];
		$this->messages['takCallsign'][2] = ['ERROR', _('A TAK login with this callsign already exists. Please choose another callsign.')];
		$this->messages['takRole'][1] = ['ERROR', _('Role'), _('Please select a role.')];
		$this->messages['takColor'][1] = ['ERROR', _('Please select a team color.')];
		$this->messages['takUsers'][0] = ['ERROR', _('takUsers'), _('Unable to add the user to the takUsers group')];
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlResponsiveRow();
		// Section title
		$return->add(new htmlSubTitle(_('General')));
		// takCallsign
		$this->addSimpleInputTextField($return, 'takCallsign', _('TAK callsign'), true);
		// takRole
		$return->add(new htmlResponsiveSelect('takRole', ['Team Member', 'Team Leader', 'HQ', 'Sniper', 'Medic', 'Forward Observer', 'RTO', 'K9'], ['Team Member'], _('TAK team role'), 'takRole'), 12);
		//takColor
		$return->add(new htmlResponsiveSelect('takColor', ['Blue', 'Brown', 'Cyan', 'Dark Blue', 'Dark Green', 'Green', 'Magenta', 'Maroon', 'Orange', 'Purple', 'Red', 'Teal', 'White', 'Yellow'], ['Cyan'], _('TAK team color'), 'takColor'), 12);
		return $return;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_attributes() {
		$return = [];
		$this->attributes['takCallsign'][0] = $_POST['takCallsign'];
		$this->attributes['takRole'][0] = $_POST['takRole'];
		$this->attributes['takColor'][0] = $_POST['takColor'];
		// check if takCallsign is filled
		if ($_POST['takCallsign'] == '') {
			$return[] = $this->messages['takCallsign'][0];
		}
		// check if takCallsign is unique
		else if ($this->getAccountContainer()->isNewAccount || ($this->attributes['takCallsign'][0] != $this->orig['takCallsign'][0])) {
			$suffix = $_POST['accountContainerSuffix'];
			$search = searchLDAP($suffix, 'takCallsign=' . $_POST['takCallsign'], ['dn']);
			if (sizeof($search) > 0) {
				$return[] = $this->messages['takCallsign'][2];
			}
		}
		// check if takRole is filled
		if ($_POST['takRole'] == '') {
			$return[] = $this->messages['takRole'][1];
		}
		// check if takColor is filled
		if ($_POST['takColor'] == '') {
			$return[] = $this->messages['takColor'][1];
		}
		return $return;
    }

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('takUser', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'takUser';
			}
            // takCallsign
			$partialAccounts[$i]['takCallsign'] = $rawAccounts[$i][$ids['takUser_takCallsign']];
            // takRole
			$partialAccounts[$i]['takRole'] = $rawAccounts[$i][$ids['takUser_takRole']];
            // takColor
			$partialAccounts[$i]['takColor'] = $rawAccounts[$i][$ids['takUser_takColor']];
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		$this->addSimplePDFField($return, 'takCallsign', _('TAK callsign'));
		$this->addSimplePDFField($return, 'takRole', _('TAK team role'));
		$this->addSimplePDFField($return, 'takColor', _('TAK team color'));
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
		// special profile options
		if (isset($profile['takUser_takCallsign'][0]) && ($profile['takUser_takCallsign'][0] != '')) {
			$this->attributes['takCallsign'] = explode(',', $profile['takUser_takCallsign'][0]);
		}
		if (isset($profile['takUser_takRole'][0]) && ($profile['takUser_takRole'][0] != '')) {
			$this->attributes['takRole'] = explode(',', $profile['takUser_takRole'][0]);
		}
		if (isset($profile['takUser_takColor'][0]) && ($profile['takUser_takColor'][0] != '')) {
			$this->attributes['takColor'] = explode(',', $profile['takUser_takColor'][0]);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"takCallsign" => _("TAK callsign"),
			"takRole" => _("TAK team role"),
			"takColor" => _("TAK team color"),
		];
	}
}