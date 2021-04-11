<?php
namespace LAM\TOOLS\MULTI_EDIT;
use LAMTool;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2021  Roland Gruber

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
* Multi edit tool that allows LDAP operations on multiple entries.
*
* @author Roland Gruber
* @package tools
*/

/**
 * Multi edit tool that allows LDAP operations on multiple entries.
 *
 * @package tools
 */
class toolMultiEdit implements LAMTool {

	/**
	 * {@inheritDoc}
	 */
	public function getName(): string {
	 	return _("Multi edit");
	 }

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Performs modifications on multiple LDAP entries.");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getLink(): string {
		return "tools/multiEdit.php";
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
		return 'edit.png';
	}

	/**
	 * {@inheritDoc}
	 */
	public function getPosition(): int {
		return 400;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSubTools(): array {
		return array();
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
	public function isHideable(): bool {
		return true;
	}

}
