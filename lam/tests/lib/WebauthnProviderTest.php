<?php
namespace LAM\LIB\TWO_FACTOR;
use LAM\LOGIN\WEBAUTHN\WebauthnManager;
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

require_once __DIR__ . '/../../lib/modules.inc';
require_once __DIR__ . '/../../lib/2factor.inc';

/**
 * Tests the webauthn provider.
 *
 * @author Roland Gruber
 */
class WebauthnProviderTest extends TestCase {

	/**
	 * @var TwoFactorConfiguration configuration for 2FA
	 */
	private TwoFactorConfiguration $config;

	protected function setUp(): void {
		$this->config = new TwoFactorConfiguration();
	}

	public function test_getSerials() {
		$provider = new WebauthnProvider($this->config);

		$this->assertNotEmpty($provider->getSerials('user', 'password'));
	}

	public function test_isShowSubmitButton() {
		$provider = new WebauthnProvider($this->config);

		$this->assertFalse($provider->isShowSubmitButton());
	}

	public function test_hasCustomInputForm() {
		$provider = new WebauthnProvider($this->config);

		$this->assertTrue($provider->hasCustomInputForm());
	}

	public function test_addCustomInput() {
		$this->config->twoFactorAuthenticationOptional = true;
		$manager = $this
			->getMockBuilder(WebauthnManager::class)
			->setMethods(['isRegistered'])
			->getMock();
		$manager->method('isRegistered')->willReturn(false);
		$provider = $this
			->getMockBuilder(WebauthnProvider::class)
			->setConstructorArgs([$this->config])
			->setMethods(['getWebauthnManager'])
			->getMock();
		$provider->method('getWebauthnManager')->willReturn($manager);
		$row = new \htmlResponsiveRow();

		$provider->addCustomInput($row, 'userDn');
		ob_start();
		$row->generateHTML(null, [], [], false, null);
		$html = ob_get_contents();
		ob_end_clean();
		$this->assertStringContainsString('skip_webauthn', $html);
	}

}
