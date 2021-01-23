<?php
namespace LAM\LOGIN\WEBAUTHN;

use \PHPUnit\Framework\TestCase;
use \Webauthn\PublicKeyCredentialDescriptor;
use \Webauthn\PublicKeyCredentialSource;
use \Webauthn\TrustPath\CertificateTrustPath;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019 - 2021  Roland Gruber

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
require_once __DIR__ . '/../../lib/webauthn.inc';

/**
 * Checks the webauthn functionality.
 *
 * @author Roland Gruber
 */
class WebauthnManagerTest extends TestCase {

	/**
	 * @var \PHPUnit_Framework_MockObject_MockObject|PublicKeyCredentialSourceRepositorySQLiteNoSave
	 */
	private $database;
	/**
	 * @var \PHPUnit_Framework_MockObject_MockObject|WebauthnManager
	 */
	private $manager;

	protected function setup(): void {
		$this->database = $this
			->getMockBuilder(PublicKeyCredentialSourceRepositorySQLiteNoSave::class)
			->onlyMethods(array('findOneByCredentialId', 'findAllForUserEntity', 'saveCredentialSource'))
			->getMock();
		$this->database->method('findOneByCredentialId')->willReturn(null);
		$this->database->method('findAllForUserEntity')->willReturn(array());

		$this->manager = $this
			->getMockBuilder(WebauthnManager::class)
			->onlyMethods(array('getDatabase'))
			->getMock();
		$this->manager->method('getDatabase')->willReturn($this->database);

		$cfgMain = new \LAMCfgMain();
		$cfgMain->passwordMinLength = 3;
		$logFile = tmpfile();
		$logFilePath = stream_get_meta_data($logFile)['uri'];
		$cfgMain->logDestination = $logFilePath;
		$_SESSION['cfgMain'] = $cfgMain;

		$file = tmpfile();
		$filePath = stream_get_meta_data($file)['uri'];
		$config = new \LAMConfig($filePath);
		$config->setTwoFactorAuthenticationDomain('domain');
		$_SESSION['config'] = $config;
	}

	public function test_getAuthenticationObject() {
		$authenticationObj = $this->manager->getAuthenticationObject('userDN', false);
		$this->assertEquals(40, sizeof($authenticationObj->getChallenge()));
		$this->assertEquals('domain', $authenticationObj->getRpId());
	}

	public function test_getRegistrationObject() {
		$registrationObject = $this->manager->getRegistrationObject('userDn', false);
		$this->assertEquals(40, sizeof($registrationObject->getChallenge()));
		$this->assertEquals('domain', $registrationObject->getRp()->getId());
	}

	public function test_isRegistered() {
		$this->database->method('findAllForUserEntity')->willReturn(array());
		$isRegistered = $this->manager->isRegistered('userDN');
		$this->assertFalse($isRegistered);
		$this->database->method('findAllForUserEntity')->willReturn(array(
			new PublicKeyCredentialSource(
				"id1",
				PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY,
				array(),
				"atype",
				new CertificateTrustPath(array('x5c' => 'test')),
				\Ramsey\Uuid\Uuid::uuid1(),
				"p1",
				"uh1",
				1)
		));
		$isRegistered = $this->manager->isRegistered('userDN');
		$this->assertTrue($isRegistered);
	}

}

/**
 * Test class to deactivate saving.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class PublicKeyCredentialSourceRepositorySQLiteNoSave extends PublicKeyCredentialSourceRepositorySQLite {

	/**
	 * No saving
	 *
	 * @param PublicKeyCredentialSource $publicKeyCredentialSource source
	 */
	public function saveCredentialSource(PublicKeyCredentialSource $publicKeyCredentialSource): void {
	}

}
