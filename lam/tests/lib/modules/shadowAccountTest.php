<?php
/*
 $Id$

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016  Roland Gruber

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

if (is_readable('lam/lib/passwordExpirationJob.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	include_once 'lam/lib/passwordExpirationJob.inc';
	include_once 'lam/lib/modules/shadowAccount.inc';

	/**
	 * Checks the shadow expire job.
	 *
	 * @author Roland Gruber
	 *
	 */
	class ShadowAccountPasswordNotifyJobTest extends PHPUnit_Framework_TestCase {

		private $job;

		const JOB_ID = 'jobID';
		const WARNING = '14';

		private $options = array();

		public function setUp() {
			$this->job = $this->getMockBuilder('ShadowAccountPasswordNotifyJob')
				->setMethods(array('getDBLastPwdChangeTime', 'setDBLastPwdChangeTime', 'sendMail', 'findUsers', 'getConfigPrefix'))
				->getMock();
			$this->job->method('getConfigPrefix')->willReturn('test');
			$this->job->method('sendMail')->willReturn(true);
			$this->options['test_mailNotificationPeriod' . ShadowAccountPasswordNotifyJobTest::JOB_ID][0] = ShadowAccountPasswordNotifyJobTest::WARNING;
		}

		public function testNoAccounts() {
			$this->job->method('findUsers')->willReturn(array());

			$this->job->expects($this->never())->method('setDBLastPwdChangeTime');
			$this->job->expects($this->never())->method('sendMail');

			$pdo = array();
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, false);
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
			$this->job->execute(ShadowAccountPasswordNotifyJobTest::JOB_ID, $this->options, $pdo, true);
		}

	}

}

?>