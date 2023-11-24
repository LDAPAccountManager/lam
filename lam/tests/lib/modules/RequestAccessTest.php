<?php
use PHPUnit\Framework\TestCase;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023 Roland Gruber

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

if (is_readable('lam/lib/modules/requestAccess.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules/requestAccess.inc';

	/**
	 * Checks sudo role functions.
	 *
	 * @author Roland Gruber
	 *
	 */
	class RequestAccessTest extends TestCase {

		public function testReplaceWildcards() {
			$user = [
				'dn' => 'userdn',
				'cn' => ['User'],
				'multi' => ['val1', 'val2']
			];
			$approver = [
				'dn' => 'approverdn',
				'cn' => ['Approver'],
				'multi' => ['val3', 'val4']
			];

			$input = 'Some $$Cn$$ with DN $$dn$$ and $$muLTI$$ requested at @@cn@@ with DN @@Dn@@ and @@multi@@';

			$output = requestAccess::replaceWildcards($input, $user, $approver);

			$this->assertEquals('Some User with DN userdn and val1, val2 requested at Approver with DN approverdn and val3, val4', $output);
		}

		public function testReplaceWildcardsHtml() {
			$user = [
				'dn' => 'user<dn>',
				'cn' => ['<User>'],
				'multi' => ['<val1>', 'val2']
			];
			$approver = [
				'dn' => 'approver<dn>',
				'cn' => ['<Approver>'],
				'multi' => ['<val3>', 'val4']
			];

			$input = 'Some $$Cn$$ with DN $$dn$$ and $$muLTI$$ requested at @@cn@@ with DN @@Dn@@ and @@multi@@';

			$output = requestAccess::replaceWildcards($input, $user, $approver);

			$this->assertEquals('Some &lt;User&gt; with DN user&lt;dn&gt; and &lt;val1&gt;, val2 requested at &lt;Approver&gt; with DN approver&lt;dn&gt; and &lt;val3&gt;, val4', $output);
		}

	}

}
