<?php
use PHPUnit\Framework\TestCase;
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2022  Roland Gruber

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

include_once __DIR__ . '/../../lib/modules.inc';

/**
 * modules.inc test cases.
 *
 * @author Roland Gruber
 */
class ModulesTest extends TestCase {

	/**
	 * Tests unformatShortFormatToSeconds() without characters.
	 */
	function testScopeAndModuleValidation() {
		$this->assertTrue(ScopeAndModuleValidation::isValidModuleName('posixAccount'));
		$this->assertTrue(ScopeAndModuleValidation::isValidModuleName('inetOrgPerson'));
		$this->assertFalse(ScopeAndModuleValidation::isValidModuleName('notExistingModule'));
		$this->assertFalse(ScopeAndModuleValidation::isValidModuleName('.'));
		$this->assertFalse(ScopeAndModuleValidation::isValidModuleName('abc/def'));
		$this->assertFalse(ScopeAndModuleValidation::isValidModuleName("posixAccount\n"));

		$this->assertTrue(ScopeAndModuleValidation::isValidScopeName('user'));
		$this->assertTrue(ScopeAndModuleValidation::isValidScopeName('group'));
		$this->assertFalse(ScopeAndModuleValidation::isValidScopeName('notExistingScope'));
		$this->assertFalse(ScopeAndModuleValidation::isValidScopeName('.'));
		$this->assertFalse(ScopeAndModuleValidation::isValidScopeName('abc/def'));
		$this->assertFalse(ScopeAndModuleValidation::isValidScopeName("user\n"));
	}

}
