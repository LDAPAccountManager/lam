<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Roland Gruber

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

use LAM\WHITE_PAGES\WhitePagesEmailItemRenderer;
use LAM\WHITE_PAGES\WhitePagesTelephoneItemRenderer;
use LAM\WHITE_PAGES\WhitePagesTextItemRenderer;
use PHPUnit\Framework\TestCase;

if (is_readable(__DIR__ . '/../../lib/whitePagesData.inc')) {

	include_once __DIR__ . '/../../lib/whitePagesData.inc';

	class WhitePagesItemRendererTest extends TestCase {

		public function testTextGetAttributesToRead() {
			$renderer = new WhitePagesTextItemRenderer('mail');
			$this->assertEquals(['mail'], $renderer->getAttributesToRead());
			$renderer = new WhitePagesTextItemRenderer('$cn$');
			$this->assertEquals(['cn'], $renderer->getAttributesToRead());
			$renderer = new WhitePagesTextItemRenderer('$gn$ $sn$');
			$this->assertEquals(['gn', 'sn'], $renderer->getAttributesToRead());
			$renderer = new WhitePagesTextItemRenderer('$gn$$sn$');
			$this->assertEquals(['gn', 'sn'], $renderer->getAttributesToRead());
		}

		public function testEmailGetAttributesToRead() {
			$renderer = new WhitePagesEmailItemRenderer('mail');
			$this->assertEquals(['mail'], $renderer->getAttributesToRead());
		}

		public function testTelephoneGetAttributesToRead() {
			$renderer = new WhitePagesTelephoneItemRenderer('telephoneNumber');
			$this->assertEquals(['telephoneNumber'], $renderer->getAttributesToRead());
		}

		public function testTextRenderData() {
			$renderer = new WhitePagesTextItemRenderer('Cn');
			$this->assertEquals('first last', $renderer->renderData(['cn' => [0 => 'first last']]));
			$renderer = new WhitePagesTextItemRenderer('dn');
			$this->assertEquals('mydn', $renderer->renderData(['dn' => 'mydn']));
			$renderer = new WhitePagesTextItemRenderer('$cn$');
			$this->assertEquals('first last', $renderer->renderData(['cn' => [0 => 'first last']]));
			$renderer = new WhitePagesTextItemRenderer('$gn$ $Sn$');
			$this->assertEquals('first last', $renderer->renderData(['gn' => [0 => 'first'], 'sn' => [0 => 'last']]));
			$renderer = new WhitePagesTextItemRenderer('$gn$$sn$');
			$this->assertEquals('firstlast', $renderer->renderData(['gn' => [0 => 'first'], 'sn' => [0 => 'last']]));
			$renderer = new WhitePagesTextItemRenderer('$gn$$sn$');
			$this->assertEquals('first', $renderer->renderData(['gn' => [0 => 'first']]));
		}

		public function testEmailRenderData() {
			$renderer = new WhitePagesEmailItemRenderer('mail');
			$this->assertEquals('test@example.com', $renderer->renderData(['mail' => [0 => 'test@example.com']]));
		}

		public function testTelephoneRenderData() {
			$renderer = new WhitePagesTelephoneItemRenderer('telephoneNumber');
			$this->assertEquals('12345', $renderer->renderData(['telephonenumber' => [0 => '12345']]));
		}

	}

}
