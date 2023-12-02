<?php

use LAM\JOB\JobResultLog;
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2023  Roland Gruber

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

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	if (is_readable('lam/lib/passwordExpirationJob.inc')) {
		include_once 'lam/lib/passwordExpirationJob.inc';
	}
	include_once 'lam/lib/modules/shadowAccount.inc';

	/**
	 * Checks the shadowAccount class.
	 *
	 * @author Roland Gruber
	 */
	class ShadowAccountTest extends TestCase {

		public function test_isAccountExpired_noAttr() {
			$attrs = ['objectClass' => ['shadowAccount']];

			$this->assertFalse(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_notExpired() {
			$expire = intval(time() / (24*3600)) + 10000;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowexpire' => [0 => $expire]
			];

			$this->assertFalse(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_expired() {
			$expire = intval(time() / (24*3600)) - 10000;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowexpire' => [0 => $expire]
			];

			$this->assertTrue(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isPasswordExpired_noAttr() {
			$attrs = ['objectClass' => ['shadowAccount']];

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_notExpired() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowlastchange' => [0 => $change],
				'shadowmax' => [0 => '14'],
			];

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_expired() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowlastchange' => [0 => $change],
				'shadowmax' => [0 => '7'],
			];

			$this->assertTrue(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_notExpiredInactiveSet() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowlastchange' => [0 => $change],
				'shadowmax' => [0 => '7'],
				'shadowinactive' => [0 => '14'],
			];

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_expiredInactiveSet() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = [
				'objectClass' => ['shadowAccount'],
				'shadowlastchange' => [0 => $change],
				'shadowmax' => [0 => '7'],
				'shadowinactive' => [0 => '2'],
			];

			$this->assertTrue(shadowAccount::isPasswordExpired($attrs));
		}

	}
