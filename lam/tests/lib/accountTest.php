<?php
use PHPUnit\Framework\TestCase;
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018 - 2019  Roland Gruber

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

include_once __DIR__ . '/../../lib/account.inc';
include_once __DIR__ . '/../../lib/security.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class AccountTest extends TestCase {

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
		$this->assertEquals(1209615, unformatShortFormatToSeconds('2w15s'));
	}

	/**
	 * Tests unformatShortFormatToSeconds() with invalid values.
	 */
	function testUnformatShortFormatToSeconds_invalidNumber() {
		$this->assertEquals('abc', unformatShortFormatToSeconds('abc'));
		$this->assertEquals('', unformatShortFormatToSeconds(''));
	}

	/**
	 * Tests formatShortFormatToSeconds() without characters.
	 */
	function testFormatSecondsToShortFormat_basic() {
		$this->assertEquals("15s", formatSecondsToShortFormat('15'));
	}

	/**
	 * Tests formatShortFormatToSeconds() with characters.
	 */
	function testFormatSecondsToShortFormat_conversion() {
		$this->assertEquals('12s', formatSecondsToShortFormat(12));
		$this->assertEquals('3m', formatSecondsToShortFormat(180));
		$this->assertEquals('2h', formatSecondsToShortFormat(7200));
		$this->assertEquals('1d', formatSecondsToShortFormat(86400));
		$this->assertEquals('2m15s', formatSecondsToShortFormat(135));
		$this->assertEquals('2h15s', formatSecondsToShortFormat(7215));
		$this->assertEquals('2d15s', formatSecondsToShortFormat(172815));
		$this->assertEquals('2d15m', formatSecondsToShortFormat(173700));
		$this->assertEquals('2w15s', formatSecondsToShortFormat(1209615));
	}

	/**
	 * Tests formatShortFormatToSeconds() with invalid values.
	 */
	function testFormatSecondsToShortFormat_invalidNumber() {
		$this->assertEquals('', formatSecondsToShortFormat(''));
	}

	/**
	 * Tests getCallingURL().
	 */
	function testGetCallingURL_noBaseUrl_noHost() {
		$_SERVER['REQUEST_URI'] = '/test.php';
		$_SERVER['HTTP_HOST'] = null;
		$_SERVER['HTTP_REFERER'] = 'http://referrer/test.php';
		$_SERVER['HTTPS'] = 'on';
		$this->assertEquals('http://referrer/test.php', getCallingURL());
		$_SERVER['HTTP_REFERER'] = null;
		$this->assertNull(getCallingURL());
	}

	/**
	 * Tests getCallingURL().
	 */
	function testGetCallingURL_noBaseUrl_host() {
		$_SERVER['REQUEST_URI'] = '/test.php';
		$_SERVER['HTTP_HOST'] = 'host';
		$_SERVER['HTTP_REFERER'] = 'http://referrer/test.php';
		$_SERVER['HTTPS'] = 'on';
		$this->assertEquals('https://host/test.php', getCallingURL());
		$_SERVER['HTTP_REFERER'] = null;
		$this->assertEquals('https://host/test.php', getCallingURL());
	}

	/**
	 * Tests getCallingURL().
	 */
	function testGetCallingURL_baseUrl_host() {
		$_SERVER['REQUEST_URI'] = '/test.php';
		$_SERVER['HTTP_HOST'] = 'host';
		$_SERVER['HTTP_REFERER'] = 'http://referrer/test.php';
		$_SERVER['HTTPS'] = 'on';
		$this->assertEquals('http://base/test.php', getCallingURL('http://base'));
		$_SERVER['HTTP_REFERER'] = null;
		$this->assertEquals('http://base/test.php', getCallingURL('http://base'));
	}

	/**
	 * Tests convertCommaEscaping().
	 */
	function testConvertCommaEscaping() {
		$this->assertEquals('cn=test\\2C user,ou=People,o=test,c=de', convertCommaEscaping('cn=test\\, user,ou=People,o=test,c=de'));
	}

	/**
	 * Tests getAbstractDN().
	 */
	function testGetAbstractDN() {
		$this->assertEquals('test > test > de', getAbstractDN('cn=test,o=test,c=de'));
		$this->assertEquals('test,user > test > de', getAbstractDN('cn=test\\,user,o=test,c=de'));
		$this->assertEquals('test,user > test > de', getAbstractDN('cn=test\\2Cuser,o=test,c=de'));
	}

}
