<?php
use \LAM\TOOLS\IMPORT_EXPORT\Importer;
use LAM\TOOLS\IMPORT_EXPORT\MultiTask;
use LAM\TOOLS\IMPORT_EXPORT\AddAttributesTask;
use LAM\TOOLS\IMPORT_EXPORT\AddEntryTask;
use LAM\TOOLS\IMPORT_EXPORT\RenameEntryTask;
use LAM\TOOLS\IMPORT_EXPORT\DeleteEntryTask;
use LAM\TOOLS\IMPORT_EXPORT\DeleteAttributesTask;
use LAM\TOOLS\IMPORT_EXPORT\ReplaceAttributesTask;
use PHPUnit\Framework\TestCase;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2021  Roland Gruber

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
class ImporterTest extends TestCase {

	/**
	 * No LDIF at all.
	 */
	public function testCompletelyInvalid() {
		$lines = array(
			"this is no LDIF"
		);

		$this->expectException(LAMException::class, 'this is no LDIF');

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

		$this->expectException(LAMException::class, 'version: 3');

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

		$this->expectException(LAMException::class);

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

		$this->expectException(LAMException::class);

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

		$this->expectException(LAMException::class, 'dn: uid=test,dc=example,dc=com');

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
			"changetype: invalid",
			"uid: test",
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com - changetype: invalid');

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
			"changetype: add",
			"uid: test",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(AddEntryTask::class, get_class($task));
	}

	/**
	 * Change entry with modrdn changetype and invalid options.
	 */
	public function testChangeModRdnInvalidData() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modrdn",
			"uid: test",
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with modrdn changetype and invalid deleteoldrdn.
	 */
	public function testChangeModRdnInvalidDeleteoldrdn() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modrdn",
			"newrdn: uid1=test",
			"deleteoldrdn: x",
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with modrdn changetype.
	 */
	public function testChangeModRdn() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modrdn",
			"newrdn: uid1=test",
			"deleteoldrdn: 0",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(RenameEntryTask::class, get_class($task));
	}

	/**
	 * Change entry with delete changetype with extra line.
	 */
	public function testChangeDeleteInvalid() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: delete",
			"uid: test",
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with delete changetype.
	 */
	public function testChangeDelete() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: delete",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(DeleteEntryTask::class, get_class($task));
	}

	/**
	 * Change entry with modify changetype with invalid operation.
	 */
	public function testChangeModifyInvalid() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"invalid: test",
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with modify changetype and add operation.
	 */
	public function testChangeModifyAddInvalid() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"add: uid",
			"uid: uid1",
			"invalid: uid2"
		);

		$this->expectException(LAMException::class, 'uid=test,dc=example,dc=com');

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
	}

	/**
	 * Change entry with modify changetype and add operation.
	 */
	public function testChangeModifyAdd() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"add: uid",
			"uid: uid1",
			"uid: uid2"
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(MultiTask::class, get_class($task));
		$subtasks = $task->getTasks();
		$this->assertEquals(1, sizeof($subtasks));
		$subTask = $subtasks[0];
		$this->assertEquals(AddAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertEquals(1, sizeof($attributes));
		$this->assertEquals(2, sizeof($attributes['uid']));
		$this->assertTrue(in_array('uid1', $attributes['uid']));
		$this->assertTrue(in_array('uid2', $attributes['uid']));
	}

	/**
	 * Change entry with modify changetype and two add operations.
	 */
	public function testChangeModifyAddTwice() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"add: uid",
			"uid: uid1",
			"uid: uid2",
			"-",
			"add: gn",
			"gn: name1",
			"gn: name2"
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(MultiTask::class, get_class($task));
		$subtasks = $task->getTasks();
		$this->assertEquals(2, sizeof($subtasks));
		$subTask = $subtasks[0];
		$this->assertEquals(AddAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertEquals(1, sizeof($attributes));
		$this->assertEquals(2, sizeof($attributes['uid']));
		$this->assertTrue(in_array('uid1', $attributes['uid']));
		$this->assertTrue(in_array('uid2', $attributes['uid']));
		$subTask = $subtasks[1];
		$this->assertEquals(AddAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertEquals(1, sizeof($attributes));
		$this->assertEquals(2, sizeof($attributes['gn']));
		$this->assertTrue(in_array('name1', $attributes['gn']));
		$this->assertTrue(in_array('name2', $attributes['gn']));
	}

	/**
	 * Change entry with modify changetype and delete operation.
	 */
	public function testChangeModifyDelete() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"delete: uid",
			"uid: uid1",
			"uid: uid2"
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(MultiTask::class, get_class($task));
		$subtasks = $task->getTasks();
		$this->assertEquals(1, sizeof($subtasks));
		$subTask = $subtasks[0];
		$this->assertEquals(DeleteAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertEquals(1, sizeof($attributes));
		$this->assertEquals(2, sizeof($attributes['uid']));
		$this->assertTrue(in_array('uid1', $attributes['uid']));
		$this->assertTrue(in_array('uid2', $attributes['uid']));
	}

	/**
	 * Change entry with modify changetype and delete operation.
	 */
	public function testChangeModifyDeleteAll() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"delete: uid",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(MultiTask::class, get_class($task));
		$subtasks = $task->getTasks();
		$this->assertEquals(1, sizeof($subtasks));
		$subTask = $subtasks[0];
		$this->assertEquals(DeleteAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertTrue(empty($attributes));
	}

	/**
	 * Change entry with modify changetype and replace operation.
	 */
	public function testChangeModifyReplace() {
		$lines = array(
			"version: 1",
			"",
			"dn: uid=test,dc=example,dc=com",
			"changetype: modify",
			"replace: uid",
			"uid: uid1",
			"uid: uid2",
		);

		$importer = new Importer();
		$tasks = $importer->getTasks($lines);
		$this->assertEquals(1, sizeof($tasks));
		$task = $tasks[0];
		$this->assertEquals(MultiTask::class, get_class($task));
		$subtasks = $task->getTasks();
		$this->assertEquals(1, sizeof($subtasks));
		$subTask = $subtasks[0];
		$this->assertEquals(ReplaceAttributesTask::class, get_class($subTask));
		$this->assertEquals($subTask->getDn(), 'uid=test,dc=example,dc=com');
		$attributes = $subTask->getAttributes();
		$this->assertEquals(1, sizeof($attributes));
		$this->assertEquals(2, sizeof($attributes['uid']));
		$this->assertTrue(in_array('uid1', $attributes['uid']));
		$this->assertTrue(in_array('uid2', $attributes['uid']));
	}

}
