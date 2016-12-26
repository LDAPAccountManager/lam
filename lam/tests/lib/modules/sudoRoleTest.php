<?php
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

include_once 'lam/lib/baseModule.inc';
include_once 'lam/lib/modules/sudoRole.inc';

/**
 * Checks sudo role functions.
 *
 * @author Roland Gruber
 *
 */
class SudoRoleTest extends PHPUnit_Framework_TestCase {

	public function testIsValidDate() {
		$valid = array('22.10.2014', '05.01.2013', '1.3.2014', '10.5.2014', '4.12.2015',
					'05.01.2013 22:15', '1.3.2014 5:1', '10.5.2014 13:3', '4.12.2015 5:22');
		foreach ($valid as $testDate) {
			$this->assertTrue(sudoRole::isValidDate($testDate));
		}
		$invalid = array('10.25.2014', 'abc', '2014-10-12', '10.022014', '10:12', '22.10.2014 12');
		foreach ($invalid as $testDate) {
			$this->assertNotTrue(sudoRole::isValidDate($testDate), $testDate);
		}
	}

	public function testEncodeDate() {
		$dates = array(
			'1.2.2014' => '20140201000000Z',
			'10.2.2014' => '20140210000000Z',
			'1.11.2014' => '20141101000000Z',
			'20.12.2014' => '20141220000000Z',
			'1.2.2014 1:2' => '20140201010200Z',
			'10.2.2014 1:10' => '20140210011000Z',
			'1.11.2014 10:2' => '20141101100200Z',
			'20.12.2014 10:12' => '20141220101200Z',
		);
		foreach ($dates as $input => $output) {
			$this->assertEquals($output, sudoRole::encodeDate($input), $input . ' ' . $output);
		}
	}

	public function testDecodeDate() {
		$dates = array(
			'01.02.2014 00:00' => '20140201000000Z',
			'10.02.2014 00:00' => '20140210000000Z',
			'01.11.2014 00:00' => '20141101000000Z',
			'20.12.2014 00:00' => '20141220000000Z',
			'01.02.2014 01:02' => '20140201010200Z',
			'10.02.2014 01:10' => '20140210011000Z',
			'01.11.2014 10:02' => '20141101100200Z',
			'20.12.2014 10:12' => '20141220101200Z',
		);
		foreach ($dates as $output => $input) {
			$this->assertEquals($output, sudoRole::decodeDate($input), $input . ' ' . $output);
		}
	}

}

?>