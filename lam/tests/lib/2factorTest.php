<?php
namespace LAM\LIB\TWO_FACTOR;
use LAMConfig;
use PHPUnit\Framework\TestCase;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023  Roland Gruber

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

include_once 'lam/tests/utils/configuration.inc';
include_once 'lam/tests/utils/2factorUtils.inc';
require_once 'lam/lib/2factor.inc';

/**
 * Checks code in 2factor.inc.
 *
 * @author Roland Gruber
 *
 */
class TwoFactorTest extends TestCase {

	const USER_NAME = 'uid=test,dc=example,dc=com';

	private ?LAMConfig $serverProfile = null;

	protected function setUp(): void {
		testCreateDefaultConfig();
		$this->serverProfile = &$_SESSION['config'];
		$this->serverProfile->setTwoFactorAllowToRememberDevice('true');
		$this->serverProfile->setTwoFactorAuthentication(TwoFactorProviderService::TWO_FACTOR_YUBICO);
		$this->serverProfile->setTwoFactorRememberDevicePassword('123456789');
		$this->serverProfile->setTwoFactorRememberDeviceDuration('3600');
	}

	/**
	 * Cleans up the environment after running a test.
	 */
	protected function tearDown(): void {
		testDeleteDefaultConfig();
		parent::tearDown();
	}

	public function testTwoFactorProviderService_rememberDevice_invalid() {
		$this->serverProfile->setTwoFactorAuthentication(TwoFactorProviderService::TWO_FACTOR_DUO);
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$this->assertNull($service->getCookieValue());

		$this->serverProfile->setTwoFactorAuthentication(TwoFactorProviderService::TWO_FACTOR_OKTA);
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$this->assertNull($service->getCookieValue());

		$this->serverProfile->setTwoFactorAuthentication(TwoFactorProviderService::TWO_FACTOR_OPENID);
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$this->assertNull($service->getCookieValue());

		$this->serverProfile->setTwoFactorAllowToRememberDevice('false');
		$this->serverProfile->setTwoFactorAuthentication(TwoFactorProviderService::TWO_FACTOR_YUBICO);
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$this->assertNull($service->getCookieValue());
	}

	/**
	 * @runInSeparateProcess
	 */
	public function testTwoFactorProviderService_rememberDevice_valid() {
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$this->assertNotNull($service->getCookieValue());
	}

	/**
	 * @runInSeparateProcess
	 */
	public function testTwoFactorProviderService_isValidRememberedDevice_invalid() {
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$this->assertFalse($service->isValidRememberedDevice(self::USER_NAME));
	}

	/**
	 * @runInSeparateProcess
	 */
	public function testTwoFactorProviderService_isValidRememberedDevice_differentUser() {
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice("invalid");
		$_COOKIE['lam_remember_2fa'] = $service->getCookieValue();
		$this->assertFalse($service->isValidRememberedDevice(self::USER_NAME));
	}

	/**
	 * @runInSeparateProcess
	 */
	public function testTwoFactorProviderService_isValidRememberedDevice_valid() {
		$service = new TwoFactorProviderServiceSpy($this->serverProfile);
		$service->rememberDevice(self::USER_NAME);
		$_COOKIE['lam_remember_2fa'] = $service->getCookieValue();
		$this->assertTrue($service->isValidRememberedDevice(self::USER_NAME));
	}

}

