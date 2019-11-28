<?php
use PHPUnit\Framework\TestCase;
/*

This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
Copyright (C) 2018 - 2019  Roland Gruber

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

define('LIBDIR','lam/templates/3rdParty/pla/lib/');
include 'lam/templates/3rdParty/pla/lib/functions.php';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class PlaFunctionsTest extends TestCase {

	public function test_masort() {
		$data = array(
			'a' => array('key1' => '1'),
			'b' => array('key1' => '5', 'key2' => 3),
			'c' => array('key1' => '2'),
			'd' => array('key1' => '3'),
		);

		masort($data, 'key1', 0);

		$dataWanted = array(
			'a' => array('key1' => '1'),
			'c' => array('key1' => '2'),
			'd' => array('key1' => '3'),
			'b' => array('key1' => '5', 'key2' => 3),
		);

		$this->compareArray($dataWanted, $data);
	}

	public function test_masortRev() {
		$data = array(
			'a' => array('key1' => '1'),
			'b' => array('key1' => '5', 'key2' => 3),
			'c' => array('key1' => '2'),
			'd' => array('key1' => '3'),
		);

		masort($data, 'key1', true);

		$dataWanted = array(
			'b' => array('key1' => '5', 'key2' => 3),
			'd' => array('key1' => '3'),
			'c' => array('key1' => '2'),
			'a' => array('key1' => '1'),
		);

		$this->compareArray($dataWanted, $data);
	}

	public function test_masortPartialData() {
		$data = array(
			'a' => array('key1' => '1'),
			'b' => array('key1' => '5', 'key2' => 3),
		);

		masort($data, 'key2', 0);

		$dataWanted = array(
			'b' => array('key1' => '5', 'key2' => 3),
			'a' => array('key1' => '1'),
		);

		$this->compareArray($dataWanted, $data);
	}

	public function test_masortMultiSort() {
		$data = array(
			'a' => array('key1' => '1', 'key2' => 4),
			'b' => array('key1' => '5', 'key2' => 3),
			'c' => array('key1' => '1', 'key2' => 1),
			'd' => array('key1' => '5', 'key2' => 2),
			'e' => array('key1' => '6', 'key2' => 2),
		);

		masort($data, 'key1,key2', 0);

		$dataWanted = array(
			'c' => array('key1' => '1', 'key2' => 1),
			'a' => array('key1' => '1', 'key2' => 4),
			'd' => array('key1' => '5', 'key2' => 2),
			'b' => array('key1' => '5', 'key2' => 3),
			'e' => array('key1' => '6', 'key2' => 2),
		);

		$this->compareArray($dataWanted, $data);
	}

	public function test_masortObject() {
		$data = array(
			'a' => (object) ['key1' => '1'],
			'b' => (object) ['key1' => '5', 'key2' => 3],
			'c' => (object) ['key1' => '2'],
			'd' => (object) ['key1' => '3'],
		);

		masort($data, 'key1', 0);

		$dataWanted = array(
			'a' => (object) ['key1' => '1'],
			'c' => (object) ['key1' => '2'],
			'd' => (object) ['key1' => '3'],
			'b' => (object) ['key1' => '5', 'key2' => 3],
		);

		$this->compareArray($dataWanted, $data);
	}

	public function test_masortObjectMultiSort() {
		$data = array(
			'a' => (object) ['key1' => '1'],
			'b' => (object) ['key1' => '5', 'key2' => 3],
			'c' => (object) ['key1' => '2'],
			'd' => (object) ['key1' => '5', 'key2' => 1],
		);

		masort($data, 'key1,key2', 0);

		$dataWanted = array(
			'a' => (object) ['key1' => '1'],
			'c' => (object) ['key1' => '2'],
			'd' => (object) ['key1' => '5', 'key2' => 1],
			'b' => (object) ['key1' => '5', 'key2' => 3],
		);

		$this->compareArray($dataWanted, $data);
	}

	private function compareArray($dataWanted, $dataNew) {
		$this->assertEquals(sizeof($dataWanted), sizeof($dataNew));
		$keysWanted = array_keys($dataWanted);
		$keysNew = array_keys($dataNew);
		foreach ($keysWanted as $index => $key) {
			$this->assertEquals($keysWanted[$index], $keysNew[$index]);
			if (is_array($dataWanted[$key])) {
				$this->compareArray($dataWanted[$key], $dataNew[$keysNew[$index]]);
			}
			elseif (is_object($dataWanted[$key])) {
				$this->compareObject($dataWanted[$key], $dataNew[$keysNew[$index]]);
			}
			else {
				$this->assertEquals($dataWanted[$key], $dataNew[$keysNew[$index]]);
			}
		}
	}

	private function compareObject($dataWanted, $dataNew) {
		$membersWanted = get_object_vars($dataWanted);
		$membersNew = get_object_vars($dataNew);
		$this->assertEquals(sizeof($membersWanted), sizeof($membersNew));
		foreach ($membersWanted as $name => $value) {
			if (is_array($dataWanted->$name)) {
				$this->compareArray($dataWanted->$name, $dataNew->$name);
			}
			else {
				$this->assertEquals($dataWanted->$name, $dataNew->$name);
			}
		}
	}

}

?>
