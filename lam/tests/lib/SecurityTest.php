<?php
use PHPUnit\Framework\TestCase;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014 - 2023  Roland Gruber

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

	private $cfg;

	/**
	 * @var LAMConfig
	 */
	private $serverProfile;

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
		$this->checkPwd(['55555', '666666'], ['1', '22', '333', '4444']);
		$this->serverProfile->setPwdPolicyMinLength('7');
		$this->checkPwd(['7777777'], ['1', '22', '333', '4444', '55555', '666666']);
		$this->serverProfile->setPwdPolicyMinLength('3');
		$this->checkPwd(['333', '4444', '55555', '666666', '7777777'], ['1', '22']);
	}

	public function testMinUpper() {
		$this->cfg->passwordMinUpper = 3;
		$this->checkPwd(['55A5AA55', '6BB666BB66', 'ABC'], ['1A', '2C2C', 'AB3', '44BB']);
		$this->serverProfile->setPwdPolicyMinUppercase('5');
		$this->checkPwd(['5AA5AAA5', '6BBB66BBB6', 'ABCDE'], ['1A', '2C2C', 'AB3', '44BB']);
		$this->serverProfile->setPwdPolicyMinUppercase('2');
		$this->checkPwd(['5555A5A5', '6BBB666666', 'AB'], ['1A', '2C22', 'A33', '444B']);
	}

	public function testMinLower() {
		$this->cfg->passwordMinLower = 3;
		$this->checkPwd(['55a5aa55', '6bb666bb66', 'abc'], ['1a', '2c2c', 'ab3', '44bbABC']);
		$this->serverProfile->setPwdPolicyMinLowercase('5');
		$this->checkPwd(['5aa5aaa5', '6bbb66bb66', 'abcde'], ['1abcd', '2c2c', 'ab3', '44bbABC']);
		$this->serverProfile->setPwdPolicyMinLowercase('2');
		$this->checkPwd(['5555aa55', '6bb6666b66', 'ab'], ['1a', '2c23', 'a13', '441bABC']);
	}

	public function testMinNumeric() {
		$this->cfg->passwordMinNumeric = 3;
		$this->checkPwd(['333', '4444'], ['1', '22', '33A', '44bb']);
		$this->serverProfile->setPwdPolicyMinNumeric('5');
		$this->checkPwd(['55555'], ['1', '22', '33A', '44bb', '333', '4444']);
		$this->serverProfile->setPwdPolicyMinNumeric('2');
		$this->checkPwd(['22', '33A', '44bb', '333', '4444'], ['1', 'X']);
	}

	public function testMinSymbol() {
		$this->cfg->passwordMinSymbol = 3;
		$this->checkPwd(['---', '++++'], ['1.', '2.2.', '3+3+A', '44bb']);
		$this->serverProfile->setPwdPolicyMinSymbolic('5');
		$this->checkPwd(['---++', '++--++'], ['1.', '2.2.', '3+3+A--', '44bb']);
		$this->serverProfile->setPwdPolicyMinSymbolic('2');
		$this->checkPwd(['-1-', '+x++'], ['1.', '2.', '3+3A', '44bb']);
	}

	public function testMinClasses() {
		$this->cfg->passwordMinClasses = 3;
		$this->checkPwd(['aB.', 'aB.1', 'aa.B99'], ['1', '2.', '3+-', '44bb']);
	}

	public function testRulesCount() {
		$this->cfg->passwordMinUpper = 3;
		$this->cfg->passwordMinLower = 3;
		$this->cfg->passwordMinNumeric = 3;
		$this->cfg->passwordMinSymbol = 3;
		$this->cfg->passwordMinClasses = 3;
		// all rules
		$this->cfg->checkedRulesCount = -1;
		$this->checkPwd(['ABC---abc123', 'ABC123xxx.-.-'], ['1', '2.', '3+-', '44bb', 'ABCabc---22']);
		// at least 3 rules
		$this->cfg->checkedRulesCount = 3;
		$this->checkPwd(['ABC---abc', 'ABC123.-.-', 'ABCabc-'], ['1', '2.', '3+-', '44bb', 'ABC--22']);
	}

	public function testUser() {
		$this->cfg->passwordMustNotContainUser = 'true';
		$this->checkPwd(['u', 'us', 'use', 'use1r'], ['user', '2user', 'user3'], 'user');
		$this->checkPwd(['u', 'us', 'use', 'use1r'], ['user', '2user', 'user3', 'test'], ['user', 'test']);
	}

	public function testUserAttributes() {
		$this->cfg->passwordMustNotContain3Chars = 'true';
		$this->checkPwd(['u', 'us', 'us1e', 'us1er'], ['use', 'user', '2user', 'user3'], 'user');
		$this->checkPwd(
			['uf', 'usfi', 'us1ela3s', 'us1er.la#st'],
			['use', 'user', '2user', 'user3', 'las', 'last', 'fir', 'first'],
			'user',
			['first', 'last']);
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
			$otherUserAttrs = [];
		}
		foreach ($pwdsToAccept as $pwd) {
			$this->assertTrue(checkPasswordStrength($pwd, $userName, $otherUserAttrs));
		}
		foreach ($pwdsToReject as $pwd) {
			$this->assertNotTrue(checkPasswordStrength($pwd, $userName, $otherUserAttrs));
		}
	}

}
