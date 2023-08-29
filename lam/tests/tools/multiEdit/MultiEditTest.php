<?php
namespace LAM\TOOLS\MULTI_EDIT;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023  Roland Gruber

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

use PHPUnit\Framework\TestCase;

include_once (dirname(__FILE__) . '/../../../lib/multiEditTool.inc');

/**
 * Checks code in multiEdit.php.
 *
 * @author Roland Gruber
 *
 */
class MultiEditTest extends TestCase {

	public function testExtractWildcards() {
		$this->assertEquals(array('abc'), extractWildcards('((abc))'));
		$this->assertEquals(array('abc'), extractWildcards('%abc%'));
		$this->assertEquals(array('abc'), extractWildcards('@abc@'));
		$this->assertEquals(array('abc'), extractWildcards('??abc??'));
		$this->assertEquals(array('abc'), extractWildcards('!!abc!!'));
		$this->assertEquals(array('abc'), extractWildcards('?abc?'));
		$this->assertEquals(array('abc'), extractWildcards('!abc!'));
		$this->assertEquals(array('abc'), extractWildcards('§abc|;§'));

		$this->assertEquals(array('abc', 'xyz'), extractWildcards('%abc% %xyz%'));
		$this->assertEquals(array('abc', 'xyz'), extractWildcards('%abc% ?xyz?'));

		$this->assertEquals(array('abc', 'xyz'), extractWildcards('adc %abc% %xyz% 123'));
		$this->assertEquals(array('abc', 'xyz'), extractWildcards('adc %abc% ?xyz? 123'));
	}

	public function testReplaceWildcards() {
		$entry = array(
			'dn' => 'cn=admin,dc=example,dc=com',
			'sn' => array('Steve'),
			'givenName' => array('Miller'),
			'uid' => array('smiller'),
			'description' => array('line1', 'line2')
		);
		$this->assertEquals('Steve', replaceWildcards('%Sn%', $entry));
		$this->assertEquals('S', replaceWildcards('@Sn@', $entry));
		$this->assertEquals('s', replaceWildcards('?Sn?', $entry));
		$this->assertEquals('S', replaceWildcards('!uid!', $entry));
		$this->assertEquals('steve', replaceWildcards('??Sn??', $entry));
		$this->assertEquals('STEVE', replaceWildcards('!!Sn!!', $entry));
		$this->assertEquals(' ', replaceWildcards('((Sn))', $entry));
		$this->assertEquals('', replaceWildcards('((unknown))', $entry));
		$this->assertEquals('line1;line2', replaceWildcards('§Description|;§', $entry));
		$this->assertEquals('cn=admin,dc=example,dc=com', replaceWildcards('%dn%', $entry));

		$this->assertEquals('Steve Miller', replaceWildcards('%Sn%((Sn))%givenName%', $entry));

		$this->assertEquals('123 Steve Miller 123', replaceWildcards('123 %Sn%((Sn))%givenName% 123', $entry));
	}

}
