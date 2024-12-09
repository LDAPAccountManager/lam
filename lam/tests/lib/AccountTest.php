<?php
use PHPUnit\Framework\TestCase;
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018 - 2024  Roland Gruber

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
 * account.inc test cases.
 *
 * @author Roland Gruber
 */
class AccountTest extends TestCase {

	/**
	 * Tests unformatShortFormatToSeconds() without characters.
	 */
	function testUnformatShortFormatToSeconds_plainNumber() {
		$this->assertEquals(15, unformatShortFormatToSeconds('15'));
		$this->assertEquals(0, unformatShortFormatToSeconds('0'));
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
		$this->assertEquals(1_209_615, unformatShortFormatToSeconds('2w15s'));
		$this->assertEquals(95_817_615, unformatShortFormatToSeconds('3y2w15s'));
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
		$this->assertEquals("0", formatSecondsToShortFormat('0'));
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
		$this->assertEquals('2w15s', formatSecondsToShortFormat(1_209_615));
		$this->assertEquals('3y2w15s', formatSecondsToShortFormat(95_817_615));
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
		$this->assertEquals('test ❭ test ❭ de', getAbstractDN('cn=test,o=test,c=de'));
		$this->assertEquals('test,user ❭ test ❭ de', getAbstractDN('cn=test\\,user,o=test,c=de'));
		$this->assertEquals('test,user ❭ test ❭ de', getAbstractDN('cn=test\\2Cuser,o=test,c=de'));
	}

	/**
	 * Tests extractRDNAttribute() and extractRDNValue().
	 */
	function testExtractRDN() {
		$dn = 'test';
		$this->assertEquals(null, extractRDNAttribute($dn));
		$this->assertEquals(null, extractRDNValue($dn));
		$dn = 'ou=test';
		$this->assertEquals('ou=test', extractRDN($dn));
		$this->assertEquals('ou', extractRDNAttribute($dn));
		$this->assertEquals('test', extractRDNValue($dn));
		$dn = 'ou=tes\\, tä,dc=com\\, pany,dc=com';
		$this->assertEquals('ou=tes\\2C tä', extractRDN($dn));
		$this->assertEquals('ou', extractRDNAttribute($dn));
		$this->assertEquals('tes\\2C tä', extractRDNValue($dn));
	}

	function testExtractDNSuffix() {
		$dn = 'test';
		$this->assertEquals(null, extractDNSuffix($dn));
		$dn = 'ou=test';
		$this->assertEquals('', extractDNSuffix($dn));
		$dn = 'ou=tes\\, t,dc=com\\, panyä,dc=com';
		$this->assertEquals('dc=com\\2C panyä,dc=com', extractDNSuffix($dn));
	}

	/**
	 * Tests isCommandlineSafeEmailAddress().
	 */
	function testIsCommandlineSafeEmailAddress() {
		$this->assertTrue(isCommandlineSafeEmailAddress(''));
		$this->assertTrue(isCommandlineSafeEmailAddress('test@example.com'));
		$this->assertTrue(isCommandlineSafeEmailAddress('test-123_abc@example.com'));
		$this->assertFalse(isCommandlineSafeEmailAddress('test+abc@example.com'));
	}

	/**
	 * Tests isDeveloperVersion()
	 */
	function testIsDeveloperVersion() {
		$this->assertFalse(isDeveloperVersion('0.4.1'));
		$this->assertFalse(isDeveloperVersion('3.2.RC1'));
		$this->assertTrue(isDeveloperVersion('4.5.DEV'));
	}

	/**
	 * Tests ARGON2ID
	 */
	function testPwdHash() {
		$testPassword = '1234556';
		$types = ['ARGON2ID', 'SSHA', 'SHA', 'SMD5', 'MD5', 'CRYPT', 'CRYPT-SHA512'];
		foreach ($types as $type) {
			$hash = pwd_hash($testPassword, true, $type);
			$type = getHashType($hash);
			$hash = explode('}', $hash)[1];
			$this->assertFalse(checkPasswordHash($type, $hash, $testPassword . 'X'), $type . ' ' . $hash);
			$this->assertTrue(checkPasswordHash($type, $hash, $testPassword), $type . ' ' . $hash);
		}
		$hash = pwd_hash($testPassword, true, 'PLAIN');
		$this->assertFalse(checkPasswordHash('PLAIN', $hash, $testPassword . 'X'), $type . ' ' . $hash);
		$this->assertTrue(checkPasswordHash('PLAIN', $hash, $testPassword), $type . ' ' . $hash);
	}

	function testGetHashType() {
		$this->assertEquals('PLAIN', getHashType(''));
		$this->assertEquals('PLAIN', getHashType(null));
		$this->assertEquals('PLAIN', getHashType('abc123'));
		$this->assertEquals('CRYPT', getHashType('{CRYPT}123'));
		$this->assertEquals('PBKDF2-SHA512', getHashType('{PBKDF2-SHA512}123'));
		$this->assertEquals('MD5', getHashType('{MD5}123'));
		$this->assertEquals('SMD5', getHashType('{SMD5}123'));
		$this->assertEquals('K5KEY', getHashType('{K5KEY}123'));
		$this->assertEquals('ARGON2ID', getHashType('{ARGON2}123'));
		$this->assertEquals('SSHA', getHashType('{SSHA}123'));
	}

	function testGetNumberOfCharacterClasses() {
		$this->assertEquals(0, getNumberOfCharacterClasses(null));
		$this->assertEquals(1, getNumberOfCharacterClasses('0'));
		$this->assertEquals(1, getNumberOfCharacterClasses('3'));
		$this->assertEquals(1, getNumberOfCharacterClasses('345'));
		$this->assertEquals(1, getNumberOfCharacterClasses('a'));
		$this->assertEquals(1, getNumberOfCharacterClasses('abc'));
		$this->assertEquals(1, getNumberOfCharacterClasses('A'));
		$this->assertEquals(1, getNumberOfCharacterClasses('ABC'));
		$this->assertEquals(1, getNumberOfCharacterClasses('.'));
		$this->assertEquals(1, getNumberOfCharacterClasses('.-,'));
		$this->assertEquals(2, getNumberOfCharacterClasses('aA'));
		$this->assertEquals(3, getNumberOfCharacterClasses('aA0'));
		$this->assertEquals(4, getNumberOfCharacterClasses('a0A.'));
		$this->assertEquals(4, getNumberOfCharacterClasses('a-0AB.a3'));
	}

	function testGenerateRandomPassword() {
		global $_SESSION;
		$_SESSION = ['cfgMain' => new LAMCfgMain()];
		$this->assertEquals(20, strlen(generateRandomPassword(20)));
	}

	function testGenerateRandomText() {
		$this->assertEquals(20, strlen(generateRandomText(20)));
	}

	function testGetPreg() {
		$this->assertTrue(get_preg('abc123', 'password'));
		$this->assertTrue(get_preg('abc ^|#*,.;:_+!%&/?{}()[]$§°@=-123', 'password'));
		$this->assertFalse(get_preg('abc\\123', 'password'));

		$this->assertTrue(get_preg('abc123', 'username'));
		$this->assertTrue(get_preg('abc%#@. _$-123', 'username'));
		$this->assertFalse(get_preg('abc?123', 'username'));

		$this->assertTrue(get_preg('abc123', 'krbUserName'));
		$this->assertTrue(get_preg('abc#@. _$-123', 'krbUserName'));
		$this->assertFalse(get_preg('abc?123', 'krbUserName'));

		$this->assertTrue(get_preg('abc123', 'hostObject'));
		$this->assertTrue(get_preg('!abc123', 'hostObject'));
		$this->assertTrue(get_preg('abc@. _$:*-123', 'hostObject'));
		$this->assertFalse(get_preg('abc!123', 'hostObject'));

		$this->assertTrue(get_preg('abc123', 'usernameList'));
		$this->assertTrue(get_preg('abc123,def456', 'usernameList'));
		$this->assertTrue(get_preg('abc123,def456,ghi789', 'usernameList'));
		$this->assertTrue(get_preg('abc%#@. _-123,def%#@. _-456,ghi%#@. _-789', 'usernameList'));
		$this->assertFalse(get_preg('abc!123', 'usernameList'));
		$this->assertFalse(get_preg('abcdef,abc!123', 'usernameList'));

		$this->assertTrue(get_preg('abc123', 'cn'));
		$this->assertTrue(get_preg('abc123$', 'cn'));
		$this->assertTrue(get_preg('abc#@. _-123', 'cn'));
		$this->assertFalse(get_preg('abc\123', 'cn'));
		$this->assertFalse(get_preg('abc<123', 'cn'));
		$this->assertFalse(get_preg('abc>123', 'cn'));
		$this->assertFalse(get_preg('abc=123', 'cn'));
		$this->assertFalse(get_preg('abc$123', 'cn'));
		$this->assertFalse(get_preg('abc?123', 'cn'));

		$this->assertTrue(get_preg('abc123ABC', 'telephone'));
		$this->assertTrue(get_preg('abc -.(123)12/3', 'telephone'));
		$this->assertFalse(get_preg('abc?123', 'telephone'));

		$this->assertTrue(get_preg('abc@abc123.com', 'email'));
		$this->assertTrue(get_preg('ab!~#+*%$/._-c@abc123.com', 'email'));
		$this->assertFalse(get_preg('abc?abc123.com', 'email'));
		$this->assertFalse(get_preg('abc@abc~123.com', 'email'));
		$this->assertFalse(get_preg('abc', 'email'));

		$this->assertTrue(get_preg('abc abc <abc@abc123.com>', 'emailWithName'));
		$this->assertTrue(get_preg('abc \'!~#+*%$()_- abc <ab!~#+*%$/._-c@abc123.com>', 'emailWithName'));
		$this->assertFalse(get_preg('abc <abc?abc123.com>', 'emailWithName'));
		$this->assertFalse(get_preg('abc <abc@abc~123.com>', 'emailWithName'));
		$this->assertFalse(get_preg('abc <ab.c>', 'emailWithName'));
		$this->assertFalse(get_preg('a?bc <ab.c>', 'emailWithName'));

		$this->assertTrue(get_preg('31-12-2012', 'date'));
		$this->assertTrue(get_preg('01-02-2012', 'date'));
		$this->assertTrue(get_preg('1-2-2012', 'date'));
		$this->assertFalse(get_preg('42-20-9999', 'date'));
		$this->assertFalse(get_preg('10-10-12345', 'date'));

		$this->assertTrue(get_preg('2012-12-31 10:11:30', 'dateTime'));
		$this->assertTrue(get_preg('2012-01-02 10:11:30', 'dateTime'));
		$this->assertTrue(get_preg('2012-02-01 10:11:30', 'dateTime'));
		$this->assertFalse(get_preg('2012-2-1 10:11:30', 'dateTime'));
		$this->assertFalse(get_preg('9999-20-41 10:11:30', 'dateTime'));
		$this->assertFalse(get_preg('12345-10-10 10:11:30', 'dateTime'));
		$this->assertFalse(get_preg('1234-10-10 99:11:30', 'dateTime'));

		$this->assertTrue(get_preg('abc.123:6567', 'hostAndPort'));
		$this->assertTrue(get_preg('ab_-c.123:132', 'hostAndPort'));
		$this->assertFalse(get_preg('abc', 'hostAndPort'));
		$this->assertFalse(get_preg('abc:abc', 'hostAndPort'));
		$this->assertFalse(get_preg('ab?c:80', 'hostAndPort'));
	}

}
