<?php
namespace LAM\LOGIN\WEBAUTHN;

use LAMCfgMain;
use LAMConfig;
use LAMConfigTest;
use LAMException;
use \PHPUnit\Framework\TestCase;
use ServerProfilePersistenceManager;
use \Webauthn\PublicKeyCredentialDescriptor;
use \Webauthn\PublicKeyCredentialSource;
use \Webauthn\TrustPath\CertificateTrustPath;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019 - 2024  Roland Gruber

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
	 * @var \PHPUnit_Framework_MockObject_MockObject|PublicKeyCredentialSourceRepositorySQLite
	 */
	private $database;
	/**
	 * @var \PHPUnit_Framework_MockObject_MockObject|WebauthnManager
	 */
	private $manager;

	/**
	 * Setup
	 *
	 * @throws LAMException
	 */
	protected function setup(): void {
		$this->database = $this
			->getMockBuilder(PublicKeyCredentialSourceRepositorySQLite::class)
			->onlyMethods(['getPdoUrl', 'findOneByCredentialId', 'findAllForUserEntity'])
			->getMock();
		$file = tmpfile();
		$filePath = stream_get_meta_data($file)['uri'];
		$this->database->method('getPdoUrl')->willReturn('sqlite:' . $filePath);
		$this->database->method('findOneByCredentialId')->willReturn(null);

		$this->manager = $this
			->getMockBuilder(WebauthnManager::class)
			->onlyMethods(['getDatabase'])
			->getMock();
		$this->manager->method('getDatabase')->willReturn($this->database);

		$cfgMain = new LAMCfgMain();
		$cfgMain->passwordMinLength = 3;
		$logFile = tmpfile();
		$cfgMain->logDestination = stream_get_meta_data($logFile)['uri'];
		$_SESSION['cfgMain'] = $cfgMain;

		$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
		$config = $serverProfilePersistenceManager->loadProfile(LAMConfigTest::FILE_NAME);
		$config->setTwoFactorAuthenticationDomain('domain');
		$_SESSION['config'] = $config;
	}

	public function test_getAuthenticationObject() {
		$this->database->method('findAllForUserEntity')->willReturn([]);

		$authenticationObj = $this->manager->getAuthenticationObject('uid=test,o=test', false);
		$this->assertEquals(32, strlen($authenticationObj->getChallenge()));
		$this->assertEquals('domain', $authenticationObj->getRpId());
	}

	public function test_getRegistrationObject() {
		$registrationObject = $this->manager->getRegistrationObject('uid=test,o=test', false);
		$this->assertEquals(32, strlen($registrationObject->getChallenge()));
		$this->assertEquals('domain', $registrationObject->getRp()->getId());
	}

	public function test_isRegistered_notRegistered() {
		$this->database->method('findAllForUserEntity')->willReturn([]);

		$isRegistered = $this->manager->isRegistered('uid=test,o=test');
		$this->assertFalse($isRegistered);
	}

	public function test_isRegistered_registered() {
		$this->database->method('findAllForUserEntity')->willReturn([new PublicKeyCredentialSource(
				"id1",
				PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY,
				[],
				"atype",
				new CertificateTrustPath(['x5c' => 'test']),
				\Symfony\Component\Uid\Uuid::fromString('00000000-0000-0000-0000-000000000000'),
				"p1",
				"uh1",
				1)]);

		$isRegistered = $this->manager->isRegistered('uid=test,o=test');
		$this->assertTrue($isRegistered);
	}

}
