<?php
/*
 $Id$

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2017  Roland Gruber

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
	include_once 'lam/lib/modules/windowsUser.inc';

	/**
	 * Checks the windowsUser class.
	 *
	 * @author Roland Gruber
	 */
	class WindowsUserTest extends PHPUnit_Framework_TestCase {

		public function test_isAccountExpired_noAttr() {
			$attrs = array('objectClass' => array('user'));

			$this->assertFalse(windowsUser::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_notExpired() {
			$expire = $this->getTimeStamp(14);
			$attrs = array(
				'objectClass' => array('user'),
				'accounTExpIRes' => array(0 => $expire)
			);

			$this->assertFalse(windowsUser::isAccountExpired($attrs));
		}

		public function test_isAccountExpired_expired() {
			$expire = $this->getTimeStamp(-14);
			$attrs = array(
				'objectClass' => array('user'),
				'accounTExpIRes' => array(0 => $expire)
			);

			$this->assertTrue(windowsUser::isAccountExpired($attrs));
		}

		/**
		 * Returns the timestamp from now with given time difference.
		 *
		 * @param int $diff time difference in days
		 */
		private function getTimeStamp($diff) {
			$timeBase = new DateTime('1601-01-01', getTimeZone());
			$time = new DateTime(null, getTimeZone());
			if ($diff > 0) {
				$time->add(new DateInterval('P' . $diff . 'D'));
			}
			else {
				$time->sub(new DateInterval('P' . abs($diff) . 'D'));
			}
			$timeDiff = $time->diff($timeBase);
			$days = $timeDiff->format('%a');
			$seconds = $days * 24 * 3600 - ($time->getOffset());
			echo $seconds . ' ';
			return $seconds . '0000000';
		}

	}

?>
