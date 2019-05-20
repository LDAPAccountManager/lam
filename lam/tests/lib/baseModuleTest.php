<?php
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

include_once __DIR__ . '/../../lib/baseModule.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class BaseModuleTest extends PHPUnit_Framework_TestCase {

	function setup() {
		$_SESSION['language'] = 'en_GB.utf8:UTF-8:English (Great Britain)';
	}

	function test_check_profileOptions_ext_preg() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('10'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_ext_preg_fail() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('a'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals(array(array('ERROR', 'error1')), $errors);
	}

	function test_check_profileOptions_regex() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_reg1'] = array(
			'type' => 'regex',
			'regex' => 'ab+a',
			'error_message' => array('ERROR', 'error1'));

		$module->setMeta($meta);

		$options = array(
			'test_reg1' => array('abbba'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_regex_fail() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_reg1'] = array(
			'type' => 'regex',
			'regex' => 'ab+a',
			'error_message' => array('ERROR', 'error1'));

		$module->setMeta($meta);

		$options = array(
			'test_reg1' => array('aCa'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals(array(array('ERROR', 'error1')), $errors);
	}

	function test_check_profileOptions_cmp() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('10'),
			'test_val2' => array('20'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmp_fail_equal() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('10'),
			'test_val2' => array('10'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals(array(array('ERROR', 'errorCMP')), $errors);
	}

	function test_check_profileOptions_cmp_fail_smaller() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('20'),
			'test_val2' => array('10'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals(array(array('ERROR', 'errorCMP')), $errors);
	}

	function test_check_profileOptions_cmpEqual_greater() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('10'),
			'test_val2' => array('20'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmpEqual_equal() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('10'),
			'test_val2' => array('10'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEmpty($errors, print_r($errors, true));
	}

	function test_check_profileOptions_cmpEqual_fail() {
		$module = new baseModuleDummy('user');

		$meta['profile_checks']['test_val1'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error1'));
		$meta['profile_checks']['test_val2'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => array('ERROR', 'error2'));
		$meta['profile_checks']['test_cmp'] = array(
			'type' => 'int_greaterOrEqual',
			'cmp_name1' => 'test_val2',
			'cmp_name2' => 'test_val1',
			'error_message' => array('ERROR', 'errorCMP'));

		$module->setMeta($meta);

		$options = array(
			'test_val1' => array('20'),
			'test_val2' => array('10'),
		);
		$errors = $module->check_profileOptions($options, 'user1');
		$this->assertEquals(array(array('ERROR', 'errorCMP')), $errors);
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
