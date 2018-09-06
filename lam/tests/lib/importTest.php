<?php
use \LAM\TOOLS\IMPORT_EXPORT\Importer;
use LAM\TOOLS\IMPORT_EXPORT\MultiTask;
use LAM\TOOLS\IMPORT_EXPORT\AddAttributesTask;
use LAM\TOOLS\IMPORT_EXPORT\AddEntryTask;
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

require_once 'lam/lib/import.inc';

/**
 * Checks the LDIF importer.
 *
 * @author Roland Gruber
 */
class ImporterTest extends PHPUnit_Framework_TestCase {

	/**
	 * No LDIF at all.
	 */
	public function testCompletelyInvalid() {
		$lines = array(
			"this is no LDIF"
		);

		$this->setExpectedException(LAMException::class, 'this is no LDIF');

		$importer = new Importer();
		$importer->getTasks($lines);
	}

	/**
	 * Wrong format version.
	 */
	public function testWrongVersion() {
		$lines = array(
			"version: 3"
		);

		$this->setExpectedException(LAMException::class, 'version: 3');

		$importer = new Importer();
		$importer->getTasks($lines);
	}

	/**
	 * Multiple versions.
	 */
	public function testMultipleVersions() {
		$lines = array(
			"version: 1",
			"",
			"version: 1"
		);

		$this->setExpectedException(LAMException::class);

		$importer = new Importer();
		$importer->getTasks($lines);
	}

	/**
	 * Data after version.
	 */
	public function testDataAfterVersion() {
		$lines = array(
			"version: 1",
			"some: data"
		);

		$this->setExpectedException(LAMException::class);

		$importer = new Importer();
		$importer->getTasks($lines);
	}

	/**
	 * DN line without any data.
	 */
	public function testDnNoData() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com"
		);

		$this->setExpectedException(LAMException::class, 'dn: uid=test,dc=example,dc=com');

		$importer = new Importer();
		$importer->getTasks($lines);
	}

	/**
	 * One complete entry.
	 */
	public function testSingleFullEntry() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"objectClass: inetOrgPerson",
			"uid: test",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
	}

	/**
	 * Change entry with invalid changetype.
	 */
	public function testChangeInvalidType() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changeType: invalid",
			"uid: test",
		);

		$this->setExpectedException(LAMException::class, 'uid=test,dc=example,dc=com - changeType: invalid');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with add changetype.
	 */
	public function testChangeAdd() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changeType: add",
			"uid: test",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(AddEntryTask::class, get_class($task));
	}

}
