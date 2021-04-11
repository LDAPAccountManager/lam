<?php
namespace LAM\TOOLS\PROFILE_EDITOR;
use LAM\TYPES\TypeManager;
use LAMTool;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2021  Roland Gruber

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
* This file includes the profile editor tool specification.
*
* @author Roland Gruber
* @package tools
*/

/**
 * Profile editor
 *
 * @package tools
 */
class toolProfileEditor extends LAMTool {

	/**
	 * {@inheritDoc}
	 */
	public function getName(): string {
	 	return _("Profile editor");
	 }

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Here you can manage your account profiles.");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getLink(): string {
		return "profedit/profilemain.php";
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
		return 100;
	}

	/**
	 * {@inheritDoc}
	 */
	public function isVisible(): bool {
		$typeManager = new TypeManager();
		$types = $typeManager->getConfiguredTypes();
		return (sizeof($types) > 0);
	}

}
