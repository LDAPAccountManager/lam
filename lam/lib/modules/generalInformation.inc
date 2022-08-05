<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2022  Roland Gruber

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

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), LAM\TYPES\getTypes());
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
		$return['icon'] = 'info.svg';
		// alias name
		$return["alias"] = _("General information");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed attributes
		$return['attributes'] = array('creatorsname', 'createtimestamp', 'modifiersname',
			'modifytimestamp', 'hassubordinates', 'memberof');
		$return['hiddenAttributes'] = array('creatorsname', 'createtimestamp', 'modifiersname',
			'modifytimestamp', 'hassubordinates');
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
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
			if ($this->attributes['hassubordinates'][0] == 'TRUE') {
				$hasChilds = _('yes');
			}
			$return->addLabel(new htmlOutputText(_('Has subentries')));
			$return->addField(new htmlOutputText($hasChilds));
			$return->addVerticalSpacer('1rem');
		}
		// group memberships
		if (isset($this->attributes['memberof'][0])) {
			$typeManager = new TypeManager();
			$scopes = array('gon', 'group');
			$types = $typeManager->getConfiguredTypesForScopes($scopes);
			$suffixList = array();
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
			foreach ($this->attributes['memberof'] as $memberOf) {
				$groupEntry = new htmlOutputText(getAbstractDN($memberOf));
				foreach ($suffixList as $suffix => $type) {
					if ((stripos($memberOf, $suffix) > 0) && !isAccountTypeHidden($type)) {
						$groupEntry = new htmlLink(getAbstractDN($memberOf), 'edit.php?type=' . $type . '&DN=\'' . $memberOf . '\'');
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
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		return array();
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = array()) : void {
		// no action for this module
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return array(
			'creatorsname' => _('Created by'),
			'createtimestamp' => _('Creation time'),
			'modifiersname' => _('Modified by'),
			'modifytimestamp' => _('Modification time'),
			'hassubordinates' => _('Has subentries'),
			'memberof' => _('Groups')
		);
	}

}

