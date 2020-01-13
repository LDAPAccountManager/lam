<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2019  Roland Gruber

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

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	if (is_readable('lam/lib/passwordExpirationJob.inc')) {
		include_once 'lam/lib/passwordExpirationJob.inc';
	}
	include_once 'lam/lib/modules/shadowAccount.inc';

	/**
	 * Checks the shadowAccount class.
	 *
	 * @author Roland Gruber
	 */
	class ShadowAccountTest extends TestCase {

		public function test_isAccountExpired_noAttr() {
			$attrs = array('objectClass' => array('shadowAccount'));

			$this->assertFalse(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_notExpired() {
			$expire = intval(time() / (24*3600)) + 10000;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'sHadoweXpirE' => array(0 => $expire)
			);

			$this->assertFalse(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_expired() {
			$expire = intval(time() / (24*3600)) - 10000;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'sHadoweXpirE' => array(0 => $expire)
			);

			$this->assertTrue(shadowAccount::isAccountExpired($attrs));
		}

		public function test_isPasswordExpired_noAttr() {
			$attrs = array('objectClass' => array('shadowAccount'));

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_notExpired() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'shadoWlastCHange' => array(0 => $change),
				'shadowmax' => array(0 => '14'),
			);

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_expired() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'shadoWlastCHange' => array(0 => $change),
				'shadowmax' => array(0 => '7'),
			);

			$this->assertTrue(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_notExpiredInactiveSet() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'shadoWlastCHange' => array(0 => $change),
				'shadowmax' => array(0 => '7'),
				'shaDowinactIVe' => array(0 => '14'),
			);

			$this->assertFalse(shadowAccount::isPasswordExpired($attrs));
		}

		public function test_isPasswordExpired_expiredInactiveSet() {
			$change = intval(time() / (24*3600)) - 10;
			$attrs = array(
				'objectClass' => array('shadowAccount'),
				'shadoWlastCHange' => array(0 => $change),
				'shadowmax' => array(0 => '7'),
				'shaDowinactIVe' => array(0 => '2'),
			);

			$this->assertTrue(shadowAccount::isPasswordExpired($attrs));
		}

	}

if (is_readable('lam/lib/passwordExpirationJob.inc')) {

	/**
	 * Checks the shadow expire job.
	 *
	 * @author Roland Gruber
	 *
	 */
	class ShadowAccountPasswordNotifyJobTest extends TestCase {

		private $job;

		const JOB_ID = 'jobID';
		const WARNING = '14';

		private $options = array();
		private $resultLog = null;

		public function setUp(): void {
			$this->job = $this->getMockBuilder('ShadowAccountPasswordNotifyJob')
				->setMethods(array('getDBLastPwdChangeTime', 'setDBLastPwdChangeTime', 'sendMail', 'findUsers', 'getConfigPrefix'))
				->getMock();
			$this->job->method('getConfigPrefix')->willReturn('test');
			$this->job->method('sendMail')->willReturn(true);
			$this->options['test_mailNotificationPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = ShadowAccountPasswordNotifyJobTest::WARNING;
			$this->resultLog = new \LAM\JOB\JobResultLog();
		}

		public function testNoAccounts() {
			$this->job->method('findUsers')->willReturn(array());

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountDoesNotExpire() {
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('0'),
					'shadowlastchange' => array('1')
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountExpired() {
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('10'),
					'shadowlastchange' => array('1')
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReached() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('300'),
					'shadowlastchange' => array($lastChangeNow)
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAlreadyWarned() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('10'),
					'shadowlastchange' => array($lastChangeNow)
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarning() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('10'),
					'shadowlastchange' => array($lastChangeNow)
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningDryRun() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=some,dc=dn',
					'shadowmax' => array('10'),
					'shadowlastchange' => array($lastChangeNow)
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, true, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

	}

}

?>
