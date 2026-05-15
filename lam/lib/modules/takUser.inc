<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Mark Halliday
                2025  Roland Gruber

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
 * Manages the object class "takUser" for users.
 *
 * @package modules
 *
 * @author Mark Halliday
 * @author Roland Gruber
 */
class takUser extends baseModule {

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'user';
	}

	/** possible roles */
	private const ROLE_TYPES = ['Team Member', 'Team Leader', 'HQ', 'RTO', 'K9', 'Medic', 'Forward Observer', 'Sniper'];

	/** possible colors */
	private const COLOR_TYPES = ['Blue', 'Brown', 'Cyan', 'Dark Blue', 'Dark Green', 'Green', 'Magenta', 'Maroon', 'Orange', 'Purple', 'Red', 'Teal', 'White', 'Yellow'];



	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'tak.png';
		// alias name
		$return["alias"] = 'TAK';
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=takUser)"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['takUser'];
		// managed attributes
		$return['attributes'] = ['takcallsign', 'takrole', 'takcolor'
        ];
		// help Entries
		$return['help'] = [
			'takcallsign' => [
				"Headline" => _("Callsign"), 'attr' => 'takCallsign',
				"Text" => _("The user's callsign to be displayed to other TAK users. It must be unique.")
			],
			'takrole' => [
				"Headline" => _("Team role"), 'attr' => 'takRole',
				"Text" => _("The user's role to be displayed to other TAK users.")
			],
			'takcolor' => [
				"Headline" => _("Team color"), 'attr' => 'takColor',
				"Text" => _("The user's team color to be displayed to other TAK users.")
            ],
		];
		// upload fields
		$return['upload_columns'] = [
            [
				'name' => 'takuser_takcallsign',
				'description' => _('Callsign'),
				'help' => 'takcallsign',
				'example' => 'UK-ORG-01',
                'required' => true,
				'unique' => true
			],
            [
				'name' => 'takuser_takrole',
				'description' => _('Team role'),
				'help' => 'takrole',
                'default' => 'Team Member',
				'values' => implode(", ", self::ROLE_TYPES),
				'example' => 'Team Member',
                'required' => true
			],
            [
				'name' => 'takuser_takcolor',
				'description' => _('Team color'),
				'help' => 'takcolor',
                'default' => 'Cyan',
				'values' => implode(", ", self::COLOR_TYPES),
				'example' => 'Cyan',
                'required' => true
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'takcallsign' => _('Callsign'),
			'takrole' => _('Team role'),
			'takcolor' => _('Team color'),
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Callsign'), 'takuser_takcallsign', null, 'takusercallsign'));
		$profileContainer->add(new htmlResponsiveInputField(_('Team role'), 'takuser_takrole', null, 'takuserrole'));
		$profileContainer->add(new htmlResponsiveInputField(_('Team color'), 'takuser_takcolor', null, 'takusercolor'));
		$return['profile_options'] = $profileContainer;
		// self-service field settings
		$return['selfServiceFieldSettings'] = [
			'takcallsign' => _('Callsign'),
			'takrole' => _('Team role'),
			'takcolor' => _('Team color'),
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['takcallsign'][0] = ['ERROR', _('Callsign'), _('Callsign contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_.')];
		$this->messages['takcallsign'][1] = ['ERROR', _('A TAK login with this callsign already exists. Please choose a different callsign.')];
		$this->messages['takcallsign'][2] = ['ERROR', _('Callsign'), _('This field is required.')];
		$this->messages['takcallsign'][3] = ['ERROR', _('Account %s:') . ' takuser_takcallsign', _('Callsign contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_.')];
		$this->messages['takrole'][0] = ['ERROR', _('Account %s:') . ' takuser_takrole', _('Please enter a valid option:') . ' ' . implode(', ', self::ROLE_TYPES)];
		$this->messages['takcolor'][0] = ['ERROR', _('Account %s:') . ' takuser_takcolor', _('Please enter a valid option:') . ' ' . implode(', ', self::COLOR_TYPES)];
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		// takCallsign
		$this->addSimpleInputTextField($return, 'takcallsign', _('Callsign'), true);
		// takRole
		$takRole = $this->attributes['takrole'][0] ?? '';
		$return->add(new htmlResponsiveSelect('takrole', self::ROLE_TYPES, [$takRole], _('Team role'), 'takrole'));
		//takColor
		$takColor = $this->attributes['takcolor'][0] ?? 'Cyan';
		$return->add(new htmlResponsiveSelect('takcolor', self::COLOR_TYPES, [$takColor], _('Team color'), 'takcolor'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$return = [];
		$this->attributes['takcallsign'][0] = $_POST['takcallsign'];
		$this->attributes['takrole'][0] = $_POST['takrole'];
		$this->attributes['takcolor'][0] = $_POST['takcolor'];
		// check if callsign is filled
		if ($_POST['takcallsign'] == '') {
			$return[] = $this->messages['takcallsign'][2];
		}
		elseif (!get_preg($_POST['takcallsign'], 'username')) {
			$return[] = $this->messages['takcallsign'][0];
		}
		// check if the callsign is unique
		elseif (empty($this->orig['takcallsign'][0]) || ($this->attributes['takcallsign'][0] !== $this->orig['takcallsign'][0])) {
			$suffix = $this->getAccountContainer()->get_type()->getSuffix();
			$search = searchLDAP($suffix, 'takcallsign=' . ldap_escape($_POST['takcallsign'], '', LDAP_ESCAPE_FILTER), ['dn']);
			if (!empty($search)) {
				$return[] = $this->messages['takcallsign'][1];
			}
		}
		return $return;
    }

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$errors = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object class
			if (!in_array('takUser', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'takUser';
			}
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'takuser_takcallsign',
				'takcallsign', 'username', $this->messages['takcallsign'][3], $errors);
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'takuser_takrole',
				'takrole', null, [], $errors);
			if (!in_array($partialAccounts[$i]['takrole'][0], self::ROLE_TYPES)) {
				$errors[] = array_merge($this->messages['takrole'][0], [[$i]]);
			}
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'takuser_takcolor',
				'takcolor', null, [], $errors);
			if (!in_array($partialAccounts[$i]['takcolor'][0], self::COLOR_TYPES)) {
				$errors[] = array_merge($this->messages['takcolor'][0], [[$i]]);
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'takcallsign', _('Callsign'));
		$this->addSimplePDFField($return, 'takrole', _('Team role'));
		$this->addSimplePDFField($return, 'takcolor', _('Team color'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// special profile options
		if (!empty($profile['takuser_takcallsign'][0])) {
			$this->attributes['takcallsign'][0] = $profile['takuser_takcallsign'][0];
		}
		if (!empty($profile['takuser_takrole'][0])) {
			$this->attributes['takrole'][0] = $profile['takuser_takrole'][0];
		}
		if (!empty($profile['takuser_takcolor'][0])) {
			$this->attributes['takcolor'][0] = $profile['takuser_takcolor'][0];
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // only password fields as long no LDAP content can be read
		}
		if (!in_array_ignore_case('takUser', $attributes['objectClass'])) {
			return $return;
		}
		$this->addSimpleSelfServiceTextField($return, 'takcallsign', _('Callsign'), $fields, $attributes, $readOnlyFields, true);
		if (in_array('takrole', $fields)) {
			$role = '';
			if (isset($attributes['takrole'][0])) {
				$role = $attributes['takrole'][0];
			}
			if (in_array('takrole', $readOnlyFields)) {
				$field = new htmlOutputText($role);
			}
			else {
				$selected = [];
				if (!empty($attributes['takrole'][0])) {
					$selected = [$attributes['takrole'][0]];
				}
				$field = new htmlSelect('takUser_takrole', self::ROLE_TYPES, $selected);
			}
			$return['takrole'] = new htmlResponsiveRow(
				new htmlLabel('takUser_takrole', $this->getSelfServiceLabel('takrole', _('Team role'))), $field
			);
		}
		if (in_array('takcolor', $fields)) {
			$color = '';
			if (isset($attributes['takcolor'][0])) {
				$color = $attributes['takcolor'][0];
			}
			if (in_array('takcolor', $readOnlyFields)) {
				$field = new htmlOutputText($color);
			}
			else {
				$selected = [];
				if (!empty($attributes['takcolor'][0])) {
					$selected = [$attributes['takcolor'][0]];
				}
				$field = new htmlSelect('takUser_takcolor', self::COLOR_TYPES, $selected);
			}
			$return['takcolor'] = new htmlResponsiveRow(
				new htmlLabel('takUser_takcolor', $this->getSelfServiceLabel('takcolor', _('Team color'))), $field
			);
		}
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function checkSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		if (!in_array_ignore_case('takUser', $attributes['objectClass'])) {
			return $return;
		}
		$this->checkSimpleSelfServiceTextField($return, 'takcallsign', $attributes, $fields, $readOnlyFields, 'username', $this->messages['takcallsign'][0], $this->messages['takcallsign'][2]);
		$this->checkSimpleSelfServiceTextField($return, 'takrole', $attributes, $fields, $readOnlyFields);
		$this->checkSimpleSelfServiceTextField($return, 'takcolor', $attributes, $fields, $readOnlyFields);
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"takcallsign" => _("Callsign"),
			"takrole" => _("Team role"),
			"takcolor" => _("Team color"),
		];
	}

}
