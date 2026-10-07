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
 * Shows general information like the creation time of an account.
 *
 * @package modules
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;
use LAM\TYPES\TypeManager;

/**
 * Shows general information like the creation time of an account.
 *
 * @package modules
 */
class generalInformation extends baseModule {

	/** display groups with DN */
	private const DISPLAY_DN = 'dn';
	/** display groups with cn */
	private const DISPLAY_CN = 'cn';
	/** display groups with cn+dn */
	private const DISPLAY_CN_DN = 'cn_dn';

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return in_array($this->get_scope(), LAM\TYPES\getTypes());
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'info.svg';
		// alias name
		$return["alias"] = _("General information");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed attributes
		$return['attributes'] = ['creatorsname', 'createtimestamp', 'modifiersname',
			'modifytimestamp', 'hassubordinates', 'memberof'];
		$return['hiddenAttributes'] = ['creatorsname', 'createtimestamp', 'modifiersname',
			'modifytimestamp', 'hassubordinates'];
		$return['help'] = [
			'displayGroups' => [
				"Headline" => _('Display format'),
				"Text" => _('Specifies how groups are displayed.')
			],
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		// creation info
		if (isset($this->attributes['creatorsname'][0])) {
			$return->addLabel(new htmlOutputText(_('Created by')));
			$return->addField(new htmlOutputText(getAbstractDN($this->attributes['creatorsname'][0])));
		}
		if (isset($this->attributes['createtimestamp'][0])) {
			$return->addLabel(new htmlOutputText(_('Creation time')));
			$return->addField(new htmlOutputText(formatLDAPTimestamp($this->attributes['createtimestamp'][0])));
		}
		if (isset($this->attributes['creatorsname'][0]) || isset($this->attributes['createtimestamp'][0])) {
			$return->addVerticalSpacer('1rem');
		}
		// modification info
		if (isset($this->attributes['modifiersname'][0])) {
			$return->addLabel(new htmlOutputText(_('Modified by')));
			$return->addField(new htmlOutputText(getAbstractDN($this->attributes['modifiersname'][0])));
		}
		if (isset($this->attributes['modifytimestamp'][0])) {
			$return->addLabel(new htmlOutputText(_('Modification time')));
			$return->addField(new htmlOutputText(formatLDAPTimestamp($this->attributes['modifytimestamp'][0])));
		}
		if (isset($this->attributes['modifiersname'][0]) || isset($this->attributes['modifytimestamp'][0])) {
			$return->addVerticalSpacer('1rem');
		}
		// children
		if (isset($this->attributes['hassubordinates'][0])) {
			$hasChilds = _('no');
			if (strtolower($this->attributes['hassubordinates'][0]) === 'true') {
				$hasChilds = _('yes');
			}
			$return->addLabel(new htmlOutputText(_('Has subentries')));
			$return->addField(new htmlOutputText($hasChilds));
			$return->addVerticalSpacer('1rem');
		}
		// group memberships
		if (isset($this->attributes['memberof'][0])) {
			$typeManager = new TypeManager();
			$scopes = ['gon', 'group'];
			$types = $typeManager->getConfiguredTypesForScopes($scopes);
			$suffixList = [];
			foreach ($types as $type) {
				$suffix = $type->getSuffix();
				// continue if suffixes are not unique
				if (isset($suffixList[$suffix])) {
					continue;
				}
				if (!empty($suffix)) {
					$suffixList[$suffix] = $type->getId();
				}
			}
			$groupLabel = new htmlOutputText(_('Groups'));
			$groupLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addLabel($groupLabel);
			$groups = new htmlTable();
			$memberOfList = $this->attributes['memberof'];
			if ($this->formatContainsDN()) {
				// sort by DN
				usort($memberOfList, compareDN(...));
				$groups->setCSSClasses(['text-right']);
			}
			else {
				natcasesort($memberOfList);
			}
			foreach ($memberOfList as $memberOf) {
				$groupEntry = new htmlOutputText($this->getGroupLabel($memberOf));
				foreach ($suffixList as $suffix => $type) {
					if ((stripos($memberOf, $suffix) > 0) && !isAccountTypeHidden($type)) {
						$groupEntry = new htmlLink($this->getGroupLabel($memberOf), 'edit.php?type=' . $type . '&DN=\'' . $memberOf . '\'');
						$groupEntry->setTargetWindow('_blank');
						break;
					}
				}
				$groups->addElement($groupEntry, true);
			}
			$return->addField($groups);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		return [];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		// no action for this module
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'creatorsname' => _('Created by'),
			'createtimestamp' => _('Creation time'),
			'modifiersname' => _('Modified by'),
			'modifytimestamp' => _('Modification time'),
			'hassubordinates' => _('Has subentries'),
			'memberof' => _('Groups')
		];
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_configOptions(array $scopes, array $allScopes): htmlElement|array {
		$configContainer = new htmlResponsiveRow();
		$displayOptions = [
			'dn' => self::DISPLAY_DN,
			'cn' => self::DISPLAY_CN,
			'cn - dn' => self::DISPLAY_CN_DN,
		];
		$displaySelect = new htmlResponsiveSelect(static::class . '_displayGroups', $displayOptions, [self::DISPLAY_CN_DN], _('Display format'), 'displayGroups');
		$displaySelect->setHasDescriptiveElements(true);
		$configContainer->add($displaySelect);
		return $configContainer;
	}

	/**
	 * Returns if the display format contains a DN.
	 * DN values should be sorted by DN and aligned right.
	 *
	 * @return bool contains DN
	 */
	private function formatContainsDN(): bool {
		$format = self::DISPLAY_CN_DN;
		if (!empty($this->moduleSettings[static::class . '_displayGroups'][0])) {
			$format = $this->moduleSettings[static::class . '_displayGroups'][0];
		}
		return in_array($format, [self::DISPLAY_DN, self::DISPLAY_CN_DN]);
	}

	/**
	 * Returns the display label for groups.
	 *
	 * @param string $dn DN
	 * @return string label
	 */
	private function getGroupLabel(string $dn): string {
		if (empty($dn)) {
			return '';
		}
		$format = self::DISPLAY_CN_DN;
		if (!empty($this->moduleSettings[static::class . '_displayGroups'][0])) {
			$format = $this->moduleSettings[static::class . '_displayGroups'][0];
		}
		return match ($format) {
			self::DISPLAY_CN_DN => $this->getGroupLabelCN_DN($dn),
			self::DISPLAY_CN => $this->getGroupLabelCN($dn),
			default => getAbstractDN($dn),
		};
	}

	/**
	 * Returns the display label for groups in "cn (dn)" format.
	 *
	 * @param string $dn DN
	 * @return string label
	 */
	private function getGroupLabelCN_DN(string $dn): string {
		if (str_starts_with($dn, 'cn=')) {
			return extractRDNValue($dn) . ' - ' . getAbstractDN(extractDNSuffix($dn));
		}
		return getAbstractDN($dn);
	}

	/**
	 * Returns the display label for groups in "cn" format.
	 *
	 * @param string $dn DN
	 * @return string label
	 */
	private function getGroupLabelCN(string $dn): string {
		return extractRDNValue($dn);
	}

}

