<?php
namespace LAM\TYPES;
use PHPUnit\Framework\TestCase;
use user;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2016 - 2021  Roland Gruber

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

require_once 'lam/lib/types.inc';

/**
 * Checks code in types.inc.
 *
 * @author Roland Gruber
 *
 */
class TypesTest extends TestCase {

	private $type;

	protected function setUp(): void {
		$this->type = $this->getMockBuilder('ConfiguredType')->setMethods(array('getBaseType', 'getModules'))->getMock();
		$scope = new user($this->type);
		$this->type->method('getBaseType')->willReturn($scope);
		$this->type->method('getModules')->willReturn(array('posixAccount'));
	}

	public function testPreTranslated() {
		$attr = new ListAttribute('#uid', $this->type);
		$this->assertEquals('User name', $attr->getAlias());
		$this->assertEquals('uid', $attr->getAttributeName());
	}

	public function testCustomAlias() {
		$attr = new ListAttribute('uid:My translation', $this->type);
		$this->assertEquals('My translation', $attr->getAlias());
		$this->assertEquals('uid', $attr->getAttributeName());
	}

	public function testIsValidTypeId() {
		$this->assertTrue(TypeManager::isValidTypeId('abc'));
		$this->assertTrue(TypeManager::isValidTypeId('abc123'));
		$this->assertTrue(TypeManager::isValidTypeId('abc-123'));
		$this->assertTrue(TypeManager::isValidTypeId('abc_123'));
		$this->assertFalse(TypeManager::isValidTypeId(''));
		$this->assertFalse(TypeManager::isValidTypeId('abc.123'));
	}

}

