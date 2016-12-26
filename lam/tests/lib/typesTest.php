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

require_once '../../lib/types.inc';

/**
 * Checks ListAttribute.
 *
 * @author Roland Gruber
 *
 */
class ListAttributeTest extends PHPUnit_Framework_TestCase {

	public function testPreTranslated() {
		$attr = new \LAM\TYPES\ListAttribute('#uid', 'user');
		$this->assertEquals('User name', $attr->getAlias());
		$this->assertEquals('uid', $attr->getAttributeName());
	}

	public function testCustomAlias() {
		$attr = new \LAM\TYPES\ListAttribute('uid:My translation', 'user');
		$this->assertEquals('My translation', $attr->getAlias());
		$this->assertEquals('uid', $attr->getAttributeName());
	}

}

?>