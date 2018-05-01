<?php
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2018  Roland Gruber

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

if (is_readable('lam/lib/modules/ppolicyUser.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	include_once 'lam/lib/passwordExpirationJob.inc';
	include_once 'lam/lib/modules/ppolicyUser.inc';

	/**
	 * Checks the ppolicy expire job.
	 *
	 * @author Roland Gruber
	 *
	 */
	class PPolicyUserPasswordNotifyJobTest extends PHPUnit_Framework_TestCase {

		private $job;

		const JOB_ID = 'jobID';
		const WARNING = '14';
		const DEFAULT_POLICY = 'cn=default,dc=test';
		const NOEXPIRE_POLICY = 'cn=noexpire,dc=test';
		const ONE_YEAR_POLICY = 'cn=policy1,dc=test';

		private $options = array();
		private $resultLog = null;

		public function setUp() {
			$this->job = $this->getMockBuilder('PPolicyPasswordNotifyJob')
				->setMethods(array('getDBLastPwdChangeTime', 'setDBLastPwdChangeTime', 'sendMail',
						'findUsers', 'getConfigPrefix', 'getPolicyOptions'))
				->getMock();
			$this->job->method('getConfigPrefix')->willReturn('test');
			$this->job->method('sendMail')->willReturn(true);
			$this->job->method('getPolicyOptions')->willReturn(array(
					PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY => array('pwdmaxage' => 365 * 3600 * 24),
					PPolicyUserPasswordNotifyJobTest::DEFAULT_POLICY => array('pwdmaxage' => 14 * 3600 * 24),
					PPolicyUserPasswordNotifyJobTest::NOEXPIRE_POLICY => array('pwdmaxage' => 0),
			));
			$this->options['test_mailNotificationPeriod' . PPolicyUserPasswordNotifyJobTest::JOB_ID][0] = PPolicyUserPasswordNotifyJobTest::WARNING;
			$this->options['test_mailDefaultPolicy' . PPolicyUserPasswordNotifyJobTest::JOB_ID][0] = PPolicyUserPasswordNotifyJobTest::DEFAULT_POLICY;
			$this->resultLog = new \LAM\JOB\JobResultLog();
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testNoAccounts() {
			$this->job->method('findUsers')->willReturn(array());

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountDoesNotExpire() {
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=noexpire,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::NOEXPIRE_POLICY),
					'pwdchangedtime' => array('20000101112233Z')
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountLocked() {
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=locked,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdaccountlockedtime' => array('20010101112233Z'),
					'pwdchangedtime' => array('20000101112233Z')
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountExpired() {
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=expired,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdchangedtime' => array('20000101112233Z'),
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReached() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=notReached,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdchangedtime' => array($date->format('YmdHis') . 'Z'),
			)));

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAlreadyWarned() {
			$now = new DateTime('now', getTimeZone());
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn($date->format('YmdHis') . 'Z');
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdchangedtime' => array($date->format('YmdHis') . 'Z'),
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarning() {
			$now = new DateTime('now', getTimeZone());
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn('20001111101010Z');
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdchangedtime' => array($date->format('YmdHis') . 'Z'),
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningDryRun() {
			$now = new DateTime('now', getTimeZone());
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn('20001111101010Z');
			$this->job->method('findUsers')->willReturn(array(array(
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => array(PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY),
					'pwdchangedtime' => array($date->format('YmdHis') . 'Z'),
			)));

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, true, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testGetWarningTimeInSeconds() {
			$confDays = 7;
			$policy = array('pwdmaxage' => 365 * 3600 * 24, 'pwdexpirewarning' => 10000);

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals((7*3600*24 + 10000), $seconds);


			$confDays = 0;
			$policy = array('pwdmaxage' => 365 * 3600 * 24, 'pwdexpirewarning' => 10000);

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals(10000, $seconds);


			$confDays = 7;
			$policy = array('pwdmaxage' => 365 * 3600 * 24);

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals(7*3600*24, $seconds);

		}

	}

}

?>