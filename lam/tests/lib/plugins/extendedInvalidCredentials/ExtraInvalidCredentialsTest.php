<?php
namespace LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS;

use DateInterval;
use DateTime;
use PHPUnit\Framework\TestCase;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020 - 2023  Roland Gruber

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

require_once __DIR__ . '/../../../../lib/modules.inc';
require_once __DIR__ . '/../../../../lib/plugins/extendedInvalidCredentials/ExtraInvalidCredentials.inc';

if (file_exists(__DIR__ . '/../../../../lib/plugins/extendedInvalidCredentials/PPolicyExtraInvalidCredentialsProvider.inc')) {

/**
 * Checks the ExtraInvalidCredentials functionality.
 *
 * @author Roland Gruber
 */
class ExtraInvalidCredentialsTest extends TestCase {

	/**
	 * No attributes that indicate an issue -> no message.
	 */
	public function test_getExtraMessage_noMessage() {
		$extraInvalidCredentials = $this
			->getMockBuilder(ExtraInvalidCredentials::class)
			->setMethods(['getLdapData'])
			->getMock();
		$extraInvalidCredentials->method('getLdapData')->willReturn([]);

		$this->assertNull($extraInvalidCredentials->getExtraMessage(null, 'testDn'));
	}

	/**
	 * PPolicy issue.
	 */
	public function test_getExtraMessage_ppolicy() {
		$extraInvalidCredentials = $this
			->getMockBuilder(ExtraInvalidCredentials::class)
			->setMethods(['getLdapData'])
			->getMock();
		$extraInvalidCredentials->method('getLdapData')->willReturn(
			[
				'dn' => 'uid=test',
				'pwdaccountlockedtime' => ['1234']
			]
		);

		$this->assertNotNull($extraInvalidCredentials->getExtraMessage(null, 'testDn'));
	}

	/**
	 * Kerberos password issue.
	 */
	public function test_getExtraMessage_mitKerberosPassword() {
		$time = new DateTime('now', getTimeZone());
		$time = $time->sub(new DateInterval('P1M'));
		$extraInvalidCredentials = $this
			->getMockBuilder(ExtraInvalidCredentials::class)
			->setMethods(['getLdapData'])
			->getMock();
		$extraInvalidCredentials->method('getLdapData')->willReturn(
			[
				'dn' => 'uid=test',
				'krbpasswordexpiration' => [$time->format('YmdHis') . 'Z']
			]
		);

		$this->assertNotNull($extraInvalidCredentials->getExtraMessage(null, 'testDn'));
	}

	/**
	 * Kerberos account issue.
	 */
	public function test_getExtraMessage_mitKerberosAccount() {
		$time = new DateTime('now', getTimeZone());
		$time = $time->sub(new DateInterval('P1M'));
		$extraInvalidCredentials = $this
			->getMockBuilder(ExtraInvalidCredentials::class)
			->setMethods(['getLdapData'])
			->getMock();
		$extraInvalidCredentials->method('getLdapData')->willReturn(
			[
				'dn' => 'uid=test',
				'krbprincipalexpiration' => [$time->format('YmdHis') . 'Z']
			]
		);

		$this->assertNotNull($extraInvalidCredentials->getExtraMessage(null, 'testDn'));
	}

}

}

