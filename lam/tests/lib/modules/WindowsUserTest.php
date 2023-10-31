<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2017 - 2023  Roland Gruber

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

	include_once __DIR__ . '/../../../lib/baseModule.inc';
	include_once __DIR__ . '/../../../lib/modules.inc';
	include_once __DIR__ . '/../../../lib/modules/windowsUser.inc';

	/**
	 * Checks the windowsUser class.
	 *
	 * @author Roland Gruber
	 */
	class WindowsUserTest extends TestCase {

		public function test_isAccountExpired_noAttr() {
			$attrs = ['objectClass' => ['user']];

			$this->assertFalse(windowsUser::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_notExpired() {
			$expire = $this->getTimeStamp(14);
			$attrs = [
				'objectClass' => ['user'],
				'accounTExpIRes' => [0 => $expire]
			];

			$this->assertFalse(windowsUser::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_expired() {
			$expire = $this->getTimeStamp(-14);
			$attrs = [
				'objectClass' => ['user'],
				'accounTExpIRes' => [0 => $expire]
			];

			$this->assertTrue(windowsUser::isAccountExpired($attrs));
		}

		/**
		 * Returns the timestamp from now with given time difference.
		 *
		 * @param int $diff time difference in days
		 */
		private function getTimeStamp($diff) {
			$timeBase = new DateTime('1601-01-01', getTimeZone());
			$time = new DateTime('now', getTimeZone());
			if ($diff > 0) {
				$time->add(new DateInterval('P' . $diff . 'D'));
			}
			else {
				$time->sub(new DateInterval('P' . abs($diff) . 'D'));
			}
			$timeDiff = $time->diff($timeBase);
			$days = $timeDiff->format('%a');
			$seconds = $days * 24 * 3600 - ($time->getOffset());
			return $seconds . '0000000';
		}

		public function testWindowsManagedGroupsNotifyJob_getLastEffectiveExecutionDate() {
			if (!interface_exists('\LAM\JOB\Job', false)) {
				$this->markTestSkipped();
				return;
			}
			$resultLog = new \LAM\JOB\JobResultLog();
			$baseDate = DateTime::createFromFormat('Y-m-d', '2020-08-21', getTimeZone());
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 12, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-07-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 6, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-07-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 3, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-08-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 1, $resultLog)->format('Y-m-d'));
			$baseDate = DateTime::createFromFormat('Y-m-d', '2020-12-31', getTimeZone());
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 12, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-07-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 6, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-10-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 3, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-12-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 1, $resultLog)->format('Y-m-d'));
			$baseDate = DateTime::createFromFormat('Y-m-d', '2020-01-01', getTimeZone());
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 12, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 6, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 3, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 1, $resultLog)->format('Y-m-d'));
			$baseDate = DateTime::createFromFormat('Y-m-d', '2020-06-05', getTimeZone());
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 12, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-01-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 6, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-04-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 3, $resultLog)->format('Y-m-d'));
			$this->assertEquals('2020-06-01', WindowsManagedGroupsNotifyJob::getLastEffectiveExecutionDate($baseDate, 1, $resultLog)->format('Y-m-d'));
		}

	}
