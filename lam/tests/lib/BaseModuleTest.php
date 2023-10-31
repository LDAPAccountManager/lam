<?php
use PHPUnit\Framework\TestCase;
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2019 - 2023  Roland Gruber

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

include_once __DIR__ . '/../../lib/baseModule.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class BaseModuleTest extends TestCase {

	protected function setup(): void {
		$_SESSION['language'] = 'en_GB.utf8:UTF-8:English (Great Britain)';
	}

	function test_check_profileOptions_ext_preg() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['10'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_ext_preg_fail() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['a'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals([['ERROR', 'error1']], $errors);
	}

	function test_check_profileOptions_regex() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_reg1'] = [
			'type' => 'regex',
			'regex' => 'ab+a',
			'error_message' => ['ERROR', 'error1']];

		$module->setMeta($meta);

		$options = [
			'test_reg1' => ['abbba'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_regex_fail() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_reg1'] = [
			'type' => 'regex',
			'regex' => 'ab+a',
			'error_message' => ['ERROR', 'error1']];

		$module->setMeta($meta);

		$options = [
			'test_reg1' => ['aCa'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals([['ERROR', 'error1']], $errors);
	}

	function test_check_profileOptions_cmp() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['10'],
			'test_val2' => ['20'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmp_fail_equal() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['10'],
			'test_val2' => ['10'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals([['ERROR', 'errorCMP']], $errors);
	}

	function test_check_profileOptions_cmp_fail_smaller() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['20'],
			'test_val2' => ['10'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals([['ERROR', 'errorCMP']], $errors);
	}

	function test_check_profileOptions_cmpEqual_greater() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['10'],
			'test_val2' => ['20'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmpEqual_equal() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['10'],
			'test_val2' => ['10'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmpEqual_fail() {
		$meta = [];
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error1']];
		$meta['profile_checks']['test_val2'] = [
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => ['ERROR', 'error2']];
		$meta['profile_checks']['test_cmp'] = [
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => ['ERROR', 'errorCMP']];

		$module->setMeta($meta);

		$options = [
			'test_val1' => ['20'],
			'test_val2' => ['10'],
		];
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals([['ERROR', 'errorCMP']], $errors);
	}

}

class baseModuleDummy extends baseModule {

	public function setMeta($meta) {
		$this->meta = $meta;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::can_manage()
	 */
	public function can_manage() {
	}


	/**
	 * {@inheritDoc}
	 * @see baseModule::process_attributes()
	 */
	public function process_attributes() {
	}


	/**
	 * {@inheritDoc}
	 * @see baseModule::display_html_attributes()
	 */
	public function display_html_attributes() {
	}

}
