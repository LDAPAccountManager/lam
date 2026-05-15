<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2024  Roland Gruber

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
 * Manages the hosts to which a user may login.
 *
 * @package modules
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * Manages the hosts to which a user may login.
 *
 * @package modules
 */
class hostObject extends baseModule {

	/**
	 * Creates a new hostObject object.
	 *
	 * @param string $scope account type (user, group, host)
	 */
	public function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return in_array($this->get_scope(), ['user', 'group']);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'computer.svg';
		// alias name
		$return["alias"] = _("Hosts");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['hostObject'];
		// managed attributes
		$return['attributes'] = ['host'];
		// help Entries
		$return['help'] = [
			'host' => [
				"Headline" => _("Hosts"), 'attr' => 'host',
				"Text" => _("Here you can specify the list of host names where this account has login privileges. The wildcard \"*\" represents all hosts. You may also use \"!\" in front of a host name to deny access to a host.")
			],
			'hostList' => [
				"Headline" => _("Hosts"), 'attr' => 'host',
				"Text" => _("Here you can specify the list of host names where this account has login privileges. The wildcard \"*\" represents all hosts. You may also use \"!\" in front of a host name to deny access to a host.")
					. ' ' . _("Multiple values are separated by comma.")
			],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			]
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'hostObject_hosts',
				'description' => _('Host list'),
				'help' => 'hostList',
				'example' => _('pc01,pc02')
			]
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'hosts' => _('Host list')
		];
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('hostObject_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$profileContainer->add(new htmlResponsiveInputField(_('Host'), 'hostObject_host', null, 'hostList'));
		$return['profile_options'] = $profileContainer;
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['host'][0] = ['ERROR', 'Please enter a valid host name.'];  // third array value is set dynamically
		$this->messages['host'][1] = ['ERROR', _('Account %s:') . ' hostObject_hosts', _('Please enter a valid list of host names.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('hostObject', $this->attributes['objectClass']) && !in_array('hostObject', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'hostObject';
		}
		$return = new htmlResponsiveRow();
		if (in_array('hostObject', $this->attributes['objectClass'])) {
			$this->addMultiValueInputTextField($return, 'host', _('Host'));
			$return->addVerticalSpacer('2rem');
			$remButton = new htmlButton('remObjectClass', _('Remove host extension'));
			$remButton->setCSSClasses(['lam-danger']);
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add host extension')));
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['hostObject'], $this->attributes['objectClass']);
			if (isset($this->attributes['host'])) {
				unset($this->attributes['host']);
			}
			return [];
		}
		if (!in_array('hostObject', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];
		$this->processMultiValueInputTextField('host', $errors, 'hostObject');
		return $errors;
	}


	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object class
			if (!in_array("hostObject", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "hostObject";
			}
			// add hosts
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'hostObject_hosts', 'host', 'hostObject', $this->messages['host'][1], $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'hosts', _('Host list'), 'host');
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['hostObject_addExt'][0])
			&& ($profile['hostObject_addExt'][0] == "true")
			&& !in_array('hostObject', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'hostObject';
		}
		$hostList = preg_split('/,[ ]*/', $profile['hostObject_host'][0]);
		$this->attributes['host'] = $hostList;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'host' => _('Host list')
		];
	}

}
