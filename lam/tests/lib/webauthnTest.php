<?php
use PHPUnit\Framework\TestCase;
use \LAM\LOGIN\WEBAUTHN\PublicKeyCredentialSourceRepositorySQLite;
use Webauthn\PublicKeyCredentialDescriptor;
use \Webauthn\PublicKeyCredentialUserEntity;
use \Webauthn\PublicKeyCredentialSource;
use Webauthn\TrustPath\CertificateTrustPath;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019  Roland Gruber

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

require_once 'lam/lib/modules.inc';
require_once 'lam/lib/webauthn.inc';

/**
 * Checks the webauthn database functionality.
 *
 * @author Roland Gruber
 */
class PublicKeyCredentialSourceRepositorySQLiteTest extends TestCase {

	/**
	 * @var PublicKeyCredentialSourceRepositorySQLite
	 */
	private $database;

	protected function setUp(): void {
		$this->database = $this
			->getMockBuilder(PublicKeyCredentialSourceRepositorySQLite::class)
			->setMethods(array('getPdoUrl'))
			->getMock();
		$file = tmpfile();
		$filePath = stream_get_meta_data($file)['uri'];
		$this->database->method('getPdoUrl')->willReturn('sqlite:' . $filePath);
	}

	/**
	 * Empty DB test
	 */
	public function test_findOneByCredentialId_emptyDb() {
		$result = $this->database->findOneByCredentialId("test");
		$this->assertNull($result);
	}

	/**
	 * Empty DB test
	 */
	public function test_findAllForUserEntity_emptyDb() {
		$entity = new PublicKeyCredentialUserEntity("cn=test,dc=example", "cn=test,dc=example", "test", null);

		$result = $this->database->findAllForUserEntity($entity);
		$this->assertEmpty($result);
	}

	/**
	 * Save multiple credentials and read them.
	 */
	public function test_saveCredentialSource() {
		$source1 = new PublicKeyCredentialSource(
			"id1",
			PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY,
			array(),
			"atype",
			new CertificateTrustPath(array('x5c' => 'test')),
			\Ramsey\Uuid\Uuid::uuid1(),
			"p1",
			"uh1",
			1);
		$this->database->saveCredentialSource($source1);
		$source2 = new PublicKeyCredentialSource(
			"id2",
			PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY,
			array(),
			"atype",
			new CertificateTrustPath(array('x5c' => 'test')),
			\Ramsey\Uuid\Uuid::uuid1(),
			"p2",
			"uh1",
			1);
		$this->database->saveCredentialSource($source2);
		$source3 = new PublicKeyCredentialSource(
			"id3",
			PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY,
			array(),
			"atype",
			new CertificateTrustPath(array('x5c' => 'test')),
			\Ramsey\Uuid\Uuid::uuid1(),
			"p3",
			"uh2",
			1);
		$this->database->saveCredentialSource($source3);

		$this->assertNotNull($this->database->findOneByCredentialId("id1"));
		$this->assertNotNull($this->database->findOneByCredentialId("id2"));
		$this->assertNotNull($this->database->findOneByCredentialId("id3"));
		$this->assertEquals(2, sizeof(
			$this->database->findAllForUserEntity(new PublicKeyCredentialUserEntity("uh1", "uh1", "uh1", null))
		));
		$this->assertEquals(1, sizeof(
			$this->database->findAllForUserEntity(new PublicKeyCredentialUserEntity("uh2", "uh2", "uh2", null))
		));
	}

}

