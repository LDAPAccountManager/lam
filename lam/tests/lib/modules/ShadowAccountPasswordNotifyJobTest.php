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

include_once 'lam/lib/baseModule.inc';
include_once 'lam/lib/modules.inc';
if (is_readable('lam/lib/passwordExpirationJob.inc')) {
	include_once 'lam/lib/passwordExpirationJob.inc';
}
include_once 'lam/lib/modules/shadowAccount.inc';

if (is_readable('lam/lib/passwordExpirationJob.inc')) {

	/**
	 * Checks the shadow expire job.
	 *
	 * @author Roland Gruber
	 *
	 */
	class ShadowAccountPasswordNotifyJobTest extends TestCase {

		private $job;

		public const JOB_ID = 'jobID';
		public const WARNING = '14';

		private array $options = [];
		private JobResultLog $resultLog;

		public function setUp(): void {
			$this->job = $this->getMockBuilder('ShadowAccountPasswordNotifyJob')
				->setMethods(['getDBLastPwdChangeTime', 'setDBLastPwdChangeTime', 'sendMail', 'findUsers', 'getConfigPrefix'])
				->getMock();
			$this->job->method('getConfigPrefix')->willReturn('test');
			$this->job->method('sendMail')->willReturn(true);
			$this->options['test_mailNotificationPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = ShadowAccountPasswordNotifyJobTest::WARNING;
			$this->resultLog = new \LAM\JOB\JobResultLog();
		}

		public function testNoAccounts() {
			$this->job->method('findUsers')->willReturn([]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountDoesNotExpire() {
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['0'],
				'shadowlastchange' => ['1']
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAccountExpired() {
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['10'],
				'shadowlastchange' => ['1']
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReached() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['300'],
				'shadowlastchange' => [$lastChangeNow]
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testAlreadyWarned() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn($lastChangeNow);
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['10'],
				'shadowlastchange' => [$lastChangeNow]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarning() {
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 355;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningReachedWithShadowWarning_defaultConfig() {
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 355;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningReachedWithShadowWarning_withWarningPeriod_enabledInConfig() {
			$this->options['test_addWarningPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = 'true';
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 355;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningReachedWithShadowWarning_withWarningPeriod_disabledInConfig() {
			$this->options['test_addWarningPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = 'true';
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 345;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReachedWithShadowWarning_defaultConfig() {
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 340;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReachedWithShadowWarning_enabledInConfig() {
			$this->options['test_addWarningPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = 'true';
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 340;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReachedWithShadowWarning_disabledInConfig() {
			$this->options['test_addWarningPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = 'false';
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 350;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['10'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningReachedWithNegativeShadowWarning() {
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 360;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['20'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->once())->method('sendMail');

			$pdo = [];
			$this->options['test_mailNotificationPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = '-10';
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningNotReachedWithNegativeShadowWarning() {
			$now = new DateTime('now', getTimeZone());
			$lastChange = floor($now->format('U')/3600/24) - 377;
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['365'],
				'shadowwarning' => ['20'],
				'shadowlastchange' => [$lastChange]
			]]);

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->options['test_mailNotificationPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = '-10';
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

		public function testWarningDryRun() {
			$now = new DateTime('now', getTimeZone());
			$lastChangeNow = floor($now->format('U')/3600/24);
			$this->job->method('getDBLastPwdChangeTime')->willReturn('1');
			$this->job->method('findUsers')->willReturn([[
				'dn' => 'cn=some,dc=dn',
				'shadowmax' => ['10'],
				'shadowlastchange' => [$lastChangeNow]
			]]);

			$this->job->expects($this->once())->method('getDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = [];
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, true, $this->resultLog);
			$this->assertFalse($this->resultLog->hasError());
		}

	}

}
