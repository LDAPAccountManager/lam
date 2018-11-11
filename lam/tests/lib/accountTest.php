<?php
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018  Roland Gruber

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

include_once 'lam/lib/account.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class AccountTest extends PHPUnit_Framework_TestCase {

	/**
	 * Tests unformatShortFormatToSeconds() without characters.
	 */
	function testUnformatShortFormatToSeconds_plainNumber() {
		$this->assertEquals(15, unformatShortFormatToSeconds('15'));
	}

	/**
	 * Tests unformatShortFormatToSeconds() with characters.
	 */
	function testUnformatShortFormatToSeconds_conversion() {
		$this->assertEquals(15, unformatShortFormatToSeconds('15'));
		$this->assertEquals(12, unformatShortFormatToSeconds('12s'));
		$this->assertEquals(180, unformatShortFormatToSeconds('3m'));
		$this->assertEquals(7200, unformatShortFormatToSeconds('2h'));
		$this->assertEquals(86400, unformatShortFormatToSeconds('1d'));
		$this->assertEquals(135, unformatShortFormatToSeconds('2m15s'));
		$this->assertEquals(7215, unformatShortFormatToSeconds('2h15s'));
		$this->assertEquals(172815, unformatShortFormatToSeconds('2d15s'));
		$this->assertEquals(173700, unformatShortFormatToSeconds('2d15m'));
	}

	/**
	 * Tests unformatShortFormatToSeconds() with invalid values.
	 */
	function testUnformatShortFormatToSeconds_invalidNumber() {
		$this->assertEquals('abc', unformatShortFormatToSeconds('abc'));
		$this->assertEquals('', unformatShortFormatToSeconds(''));
	}

}
