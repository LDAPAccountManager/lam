<?php
namespace LAM\TOOLS\TESTS;
use LAM\TYPES\TypeManager;
use LAMSubTool;
use LAMTool;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2022  Roland Gruber

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
* This file includes the test tool specification.
*
* @author Roland Gruber
* @package tools
*/

/**
 * Tests page
 *
 * @package tools
 */
class toolTests extends LAMTool {

	/**
	 * {@inheritDoc}
	 */
	public function getName(): string {
	 	return _("Tests");
	 }

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Here you can test if certain LAM features work on your installation.");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getLink(): string {
		return "tests/index.php";
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
		return 'search-color.svg';
	}

	/**
	 * {@inheritDoc}
	 */
	public function getPosition(): int {
		return 1000;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSubTools(): array {
		$return = array();
		$lamdaemonTest = new LAMSubTool();
		$lamdaemonTest->name = _("Lamdaemon test");
		$lamdaemonTest->link = 'tests/lamdaemonTest.php';
		$lamdaemonTest->description = _("Check if quotas and homedirectories can be managed.");
		$lamdaemonTest->image = 'lamdaemonSmall.png';
		$return[] = $lamdaemonTest;
		$schemaTest = new LAMSubTool();
		$schemaTest->name = _("Schema test");
		$schemaTest->link = 'tests/schemaTest.php';
		$schemaTest->description = _("Check if the LDAP schema fits the requirements of the selected account modules.");
		$schemaTest->image = 'search-color.svg';
		$return[] = $schemaTest;
		return $return;
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
