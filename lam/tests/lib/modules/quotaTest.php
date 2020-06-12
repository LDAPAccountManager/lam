<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2020  Roland Gruber

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

include_once __DIR__ . '/../../../lib/baseModule.inc';
include_once __DIR__ . '/../../../lib/modules.inc';
include_once __DIR__ . '/../../../lib/modules/quota.inc';

/**
 * Checks the quota class.
 *
 * @author Roland Gruber
 */
class QuotaTest extends TestCase {
	
	protected function setUp(): void {
		$_SESSION = array('language' => 'de_DE.utf8');
	}
	
	public function testAddBlockUnits() {
		$quota = new quota('user');
		
		$this->assertEquals('123T', $quota->addBlockUnits(1024*1024*1024*123));
		$this->assertEquals('123G', $quota->addBlockUnits(1024*1024*123));
		$this->assertEquals('123M', $quota->addBlockUnits(1024*123));
		$this->assertEquals('123', $quota->addBlockUnits(123));
		$this->assertEquals('1025', $quota->addBlockUnits(1025));
		$this->assertEquals('5000G', $quota->addBlockUnits(1024*1024*5000));
		$this->assertEquals('5000M', $quota->addBlockUnits(1024*5000));
		$this->assertEquals('5000', $quota->addBlockUnits(5000));
	}

	public function testAddInodeUnits() {
		$quota = new quota('user');
		
		$this->assertEquals('123t', $quota->addInodeUnits(1000*1000*1000*1000*123));
		$this->assertEquals('123g', $quota->addInodeUnits(1000*1000*1000*123));
		$this->assertEquals('123m', $quota->addInodeUnits(1000*1000*123));
		$this->assertEquals('123k', $quota->addInodeUnits(1000*123));
		$this->assertEquals('123', $quota->addInodeUnits(123));
		$this->assertEquals('1025', $quota->addInodeUnits(1025));
		$this->assertEquals('5001g', $quota->addInodeUnits(1000*1000*1000*5001));
		$this->assertEquals('5001m', $quota->addInodeUnits(1000*1000*5001));
		$this->assertEquals('5001k', $quota->addInodeUnits(1000*5001));
		$this->assertEquals('5001', $quota->addInodeUnits(5001));
	}

}
