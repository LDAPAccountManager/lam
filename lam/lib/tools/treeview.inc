<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlElement;
use LAMTool;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021  Roland Gruber

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
* LDAP tree view.
*
* @author Roland Gruber
* @package tools
*/

/**
 * LDAP tree view.
 *
 * @package tools
 */
class TreeViewTool extends LAMTool {

	const TREE_SUFFIX_CONFIG = 'treeViewSuffix';

	/**
	 * {@inheritDoc}
	 */
	public function getName(): string {
	 	return _('Tree view');
	 }

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("This is an LDAP browser to work on raw LDAP data.");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getLink(): string {
		return "tools/treeView.php";
	}

	/**
	 * {@inheritDoc}
	 */
	public function getRequiresWriteAccess(): bool {
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getRequiresPasswordChangeRights(): bool {
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getImageLink(): string {
		return 'process.png';
	}

	/**
	 * {@inheritDoc}
	 */
	public function getPosition(): int {
		return 550;
	}

	/**
	 * {@inheritDoc}
	 */
	public function isVisible(): bool {
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getConfigOptions(array $settings): ?htmlElement {
		$row = new htmlResponsiveRow();
		$row->add(new htmlOutputText(_('Tree view')), 12);
		$suffix = !empty($settings[self::TREE_SUFFIX_CONFIG]) ? $settings[self::TREE_SUFFIX_CONFIG] : '';
		$row->add(new htmlResponsiveInputField(_('Tree suffix'), self::TREE_SUFFIX_CONFIG, $suffix, '203'), 12);
		$row->addVerticalSpacer('1rem');
		return $row;
	}

	/**
	 * {@inheritDoc}
	 */
	public function checkConfigurationOptions(array $settings): array {
		$errors = array();
		if (!empty($settings[self::TREE_SUFFIX_CONFIG]) && !get_preg($settings[self::TREE_SUFFIX_CONFIG], 'dn')) {
			$errors[] = array('ERROR', _('Tree suffix is invalid.'));
		}
		return $errors;
	}

}
