/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014  Roland Gruber

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
<?php

$_SERVER ['REMOTE_ADDR'] = '127.0.0.1';

include_once (dirname ( __FILE__ ) . '/../utils/configuration.inc');
include_once (dirname ( __FILE__ ) . '/../../lib/security.inc');

/**
 * Checks password checking functions.
 * 
 * @author Roland Gruber
 *
 */
class SecurityTest extends PHPUnit_Framework_TestCase {
	
	private $cfg = null;	
	
	protected function setUp() {
		testCreateDefaultConfig ();
		$this->cfg = &$_SESSION ['cfgMain'];
		$this->resetPasswordRules();
	}
	
	public function testMinLength() {
		$this->cfg->passwordMinLength = 5;
		$this->checkPwd(array('55555', '666666'), array('1', '22', '333', '4444'));
	}
	
	public function testMinUpper() {
		$this->cfg->passwordMinUpper = 3;
		$this->checkPwd(array('55A5AA55', '6BB666BB66', 'ABC'), array ('1A', '2C2C', 'AB3', '44BB'));
	}
	
	public function testMinLower() {
		$this->cfg->passwordMinLower = 3;
		$this->checkPwd(array('55a5aa55', '6bb666bb66', 'abc'), array ('1a', '2c2c', 'ab3', '44bbABC'));
	}
	
	public function testMinNumeric() {
		$this->cfg->passwordMinNumeric = 3;
		$this->checkPwd(array('333', '4444'), array('1', '22', '33A', '44bb'));
	}
	
	public function testMinSymbol() {
		$this->cfg->passwordMinSymbol = 3;
		$this->checkPwd(array('---', '++++'), array('1.', '2.2.', '3+3+A', '44bb'));
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

?>