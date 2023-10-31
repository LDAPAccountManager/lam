<?php

use LAM\JOB\JobResultLog;
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2023  Roland Gruber

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
	class PPolicyUserPasswordNotifyJobTest extends TestCase {

		private $job;

		public const JOB_ID = 'jobID';
		public const WARNING = '14';
		public const DEFAULT_POLICY = 'cn=default,dc=test';
		public const NOEXPIRE_POLICY = 'cn=noexpire,dc=test';
		public const ONE_YEAR_POLICY = 'cn=policy1,dc=test';

		private array $options = [];
		private JobResultLog $resultLog;

		protected function setUp(): void {
			$this->job = $this->getMockBuilder('PPolicyPasswordNotifyJob')
				->setMethods(['getDBLastPwdChangeTime', 'setDBLastPwdChangeTime', 'sendMail', 'findUsers', 'getConfigPrefix', 'getPolicyOptions'])
				->getMock();
			$this->job->method('getConfigPrefix')->willReturn('test');
			$this->job->method('sendMail')->willReturn(true);
			$this->job->method('getPolicyOptions')->willReturn([
					PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY => ['pwdmaxage' => 365 * 3600 * 24],
					PPolicyUserPasswordNotifyJobTest::DEFAULT_POLICY => ['pwdmaxage' => 14 * 3600 * 24],
					PPolicyUserPasswordNotifyJobTest::NOEXPIRE_POLICY => ['pwdmaxage' => 0],
			]);
			$this->options['test_mailNotificationPeriod' . PPolicyUserPasswordNotifyJobTest::JOB_ID][0] = PPolicyUserPasswordNotifyJobTest::WARNING;
			$this->options['test_mailDefaultPolicy' . PPolicyUserPasswordNotifyJobTest::JOB_ID][0] = PPolicyUserPasswordNotifyJobTest::DEFAULT_POLICY;
			$this->resultLog = new \LAM\JOB\JobResultLog();
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testNoAccounts() {
			$this->job->method('findUsers')->willReturn([]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountDoesNotExpire() {
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=noexpire,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::NOEXPIRE_POLICY],
					'pwdchangedtime' => ['20000101112233Z']
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountLocked() {
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=locked,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdaccountlockedtime' => ['20010101112233Z'],
					'pwdchangedtime' => ['20000101112233Z']
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountExpired() {
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=expired,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdchangedtime' => ['20000101112233Z'],
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReached() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=notReached,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdchangedtime' => [$date->format('YmdHis') . 'Z'],
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAlreadyWarned() {
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn($date->format('YmdHis') . 'Z');
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdchangedtime' => [$date->format('YmdHis') . 'Z'],
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarning() {
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn('20001111101010Z');
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdchangedtime' => [$date->format('YmdHis') . 'Z'],
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningDryRun() {
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$date->sub(new DateInterval('P360D'));
			$this->job->method('getDBLastPwdChangeTime')->willReturn('20001111101010Z');
			$this->job->method('findUsers')->willReturn([[
					'dn' => 'cn=alreadyWarned,dc=dn',
					'pwdpolicysubentry' => [PPolicyUserPasswordNotifyJobTest::ONE_YEAR_POLICY],
					'pwdchangedtime' => [$date->format('YmdHis') . 'Z'],
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(PPolicyUserPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, true, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testGetWarningTimeInSeconds() {
			$confDays = 7;
			$policy = ['pwdmaxage' => 365 * 3600 * 24, 'pwdexpirewarning' => 10000];

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals((7*3600*24 + 10000), $seconds);


			$confDays = -7;
			$policy = ['pwdmaxage' => 365 * 3600 * 24, 'pwdexpirewarning' => 10000];

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals((-7*3600*24 + 10000), $seconds);


			$confDays = 0;
			$policy = ['pwdmaxage' => 365 * 3600 * 24, 'pwdexpirewarning' => 10000];

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals(10000, $seconds);


			$confDays = 7;
			$policy = ['pwdmaxage' => 365 * 3600 * 24];

			$seconds = $this->job->getWarningTimeInSeconds($confDays, $policy);

			$this->assertEquals(7*3600*24, $seconds);

		}

	}

}
