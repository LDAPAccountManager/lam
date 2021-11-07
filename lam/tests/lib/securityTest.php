<?php
use PHPUnit\Framework\TestCase;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014 - 2016  Roland Gruber

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

$_SERVER ['REMOTE_ADDR'] = '127.0.0.1';

include_once 'lam/tests/utils/configuration.inc';
include_once 'lam/lib/security.inc';

/**
 * Checks password checking functions.
 *
 * @author Roland Gruber
 *
 */
class SecurityTest extends TestCase {

	private $cfg = null;

	/**
	 * @var LAMConfig
	 */
	private $serverProfile = null;

	protected function setUp(): void {
		testCreateDefaultConfig();
		$this->cfg = &$_SESSION['cfgMain'];
		$this->serverProfile = &$_SESSION['config'];
		$this->resetPasswordRules();
	}

	/**
	 * Cleans up the environment after running a test.
	 */
	protected function tearDown(): void {
		testDeleteDefaultConfig();
		parent::tearDown();
	}

	public function testMinLength() {
		$this->cfg->passwordMinLength = 5;
		$this->checkPwd(array('55555', '666666'), array('1', '22', '333', '4444'));
		$this->serverProfile->setPwdPolicyMinLength('7');
		$this->checkPwd(array('7777777'), array('1', '22', '333', '4444', '55555', '666666'));
		$this->serverProfile->setPwdPolicyMinLength('3');
		$this->checkPwd(array('333', '4444', '55555', '666666', '7777777'), array('1', '22'));
	}

	public function testMinUpper() {
		$this->cfg->passwordMinUpper = 3;
		$this->checkPwd(array('55A5AA55', '6BB666BB66', 'ABC'), array ('1A', '2C2C', 'AB3', '44BB'));
		$this->serverProfile->setPwdPolicyMinUppercase('5');
		$this->checkPwd(array('5AA5AAA5', '6BBB66BBB6', 'ABCDE'), array ('1A', '2C2C', 'AB3', '44BB'));
		$this->serverProfile->setPwdPolicyMinUppercase('2');
		$this->checkPwd(array('5555A5A5', '6BBB666666', 'AB'), array ('1A', '2C22', 'A33', '444B'));
	}

	public function testMinLower() {
		$this->cfg->passwordMinLower = 3;
		$this->checkPwd(array('55a5aa55', '6bb666bb66', 'abc'), array ('1a', '2c2c', 'ab3', '44bbABC'));
		$this->serverProfile->setPwdPolicyMinLowercase('5');
		$this->checkPwd(array('5aa5aaa5', '6bbb66bb66', 'abcde'), array ('1abcd', '2c2c', 'ab3', '44bbABC'));
		$this->serverProfile->setPwdPolicyMinLowercase('2');
		$this->checkPwd(array('5555aa55', '6bb6666b66', 'ab'), array ('1a', '2c23', 'a13', '441bABC'));
	}

	public function testMinNumeric() {
		$this->cfg->passwordMinNumeric = 3;
		$this->checkPwd(array('333', '4444'), array('1', '22', '33A', '44bb'));
		$this->serverProfile->setPwdPolicyMinNumeric('5');
		$this->checkPwd(array('55555'), array('1', '22', '33A', '44bb', '333', '4444'));
		$this->serverProfile->setPwdPolicyMinNumeric('2');
		$this->checkPwd(array('22', '33A', '44bb', '333', '4444'), array('1', 'X'));
	}

	public function testMinSymbol() {
		$this->cfg->passwordMinSymbol = 3;
		$this->checkPwd(array('---', '++++'), array('1.', '2.2.', '3+3+A', '44bb'));
		$this->serverProfile->setPwdPolicyMinSymbolic('5');
		$this->checkPwd(array('---++', '++--++'), array('1.', '2.2.', '3+3+A--', '44bb'));
		$this->serverProfile->setPwdPolicyMinSymbolic('2');
		$this->checkPwd(array('-1-', '+x++'), array('1.', '2.', '3+3A', '44bb'));
	}

	public function testMinClasses() {
		$this->cfg->passwordMinClasses = 3;
		$this->checkPwd(array('aB.', 'aB.1', 'aa.B99'), array('1', '2.', '3+-', '44bb'));
	}

	public function testRulesCount() {
		$this->cfg->passwordMinUpper = 3;
		$this->cfg->passwordMinLower = 3;
		$this->cfg->passwordMinNumeric = 3;
		$this->cfg->passwordMinSymbol = 3;
		$this->cfg->passwordMinClasses = 3;
		// all rules
		$this->cfg->checkedRulesCount = -1;
		$this->checkPwd(array('ABC---abc123', 'ABC123xxx.-.-'), array('1', '2.', '3+-', '44bb', 'ABCabc---22'));
		// at least 3 rules
		$this->cfg->checkedRulesCount = 3;
		$this->checkPwd(array('ABC---abc', 'ABC123.-.-', 'ABCabc-'), array('1', '2.', '3+-', '44bb', 'ABC--22'));
	}

	public function testUser() {
		$this->cfg->passwordMustNotContainUser = 'true';
		$this->checkPwd(array('u', 'us', 'use', 'use1r'), array('user', '2user', 'user3'), 'user');
		$this->checkPwd(array('u', 'us', 'use', 'use1r'), array('user', '2user', 'user3', 'test'), array('user', 'test'));
	}

	public function testUserAttributes() {
		$this->cfg->passwordMustNotContain3Chars = 'true';
		$this->checkPwd(array('u', 'us', 'us1e', 'us1er'), array('use', 'user', '2user', 'user3'), 'user');
		$this->checkPwd(
			array('uf', 'usfi', 'us1ela3s', 'us1er.la#st'),
			array('use', 'user', '2user', 'user3', 'las', 'last', 'fir', 'first'),
			'user',
			array('first', 'last'));
	}

	/**
	 * Resets the password rules to do no checks at all.
	 */
	private function resetPasswordRules() {
		$this->cfg->passwordMinLength = 0;
		$this->cfg->passwordMinUpper = 0;
		$this->cfg->passwordMinLower = 0;
		$this->cfg->passwordMinNumeric = 0;
		$this->cfg->passwordMinSymbol = 0;
		$this->cfg->passwordMinClasses = 0;
		$this->cfg->checkedRulesCount = -1;
		$this->cfg->passwordMustNotContainUser = 'false';
		$this->cfg->passwordMustNotContain3Chars = 'false';
		$this->serverProfile->setPwdPolicyMinLength('');
		$this->serverProfile->setPwdPolicyMinUppercase('');
		$this->serverProfile->setPwdPolicyMinLowercase('');
		$this->serverProfile->setPwdPolicyMinNumeric('');
		$this->serverProfile->setPwdPolicyMinSymbolic('');
	}

	/**
	 * Checks if the given passwords are correctly accepted/rejected.
	 *
	 * @param array $pwdsToAccept passwords that must be accepted
	 * @param array $pwdsToReject passwords that must be rejected
	 * @param String $userName user name
	 * @param array $otherUserAttrs other user attributes to check
	 */
	private function checkPwd($pwdsToAccept, $pwdsToReject, $userName = null, $otherUserAttrs = null) {
		if ($userName == null) {
			$userName = 'username';
		}
		if ($otherUserAttrs == null) {
			$otherUserAttrs = array ();
		}
		foreach ($pwdsToAccept as $pwd) {
			$this->assertTrue(checkPasswordStrength($pwd, $userName, $otherUserAttrs));
		}
		foreach ($pwdsToReject as $pwd) {
			$this->assertNotTrue(checkPasswordStrength($pwd, $userName, $otherUserAttrs));
		}
	}

}
