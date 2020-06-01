<?php
use PHPUnit\Framework\TestCase;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019 - 2020  Roland Gruber

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

require_once __DIR__ . '/../../lib/selfService.inc';

/**
 * Checks selfServiceProfile.
 *
 * @author Roland Gruber
 *
 */
class selfServiceProfileTest extends TestCase {

	public function testBaseUrl() {
		$profile = new selfServiceProfile();
		$profile->setBaseUrl('http://test.com/');
		$this->assertEquals('http://test.com', $profile->getBaseUrl());
		$profile->setBaseUrl('http://test.com');
		$this->assertEquals('http://test.com', $profile->getBaseUrl());
		$profile->setBaseUrl('https://test.com/');
		$this->assertEquals('https://test.com', $profile->getBaseUrl());
		$profile->setBaseUrl('https://test.com');
		$this->assertEquals('https://test.com', $profile->getBaseUrl());
	}

	public function testImportExport() {
		$profile = new selfServiceProfile();
		$moduleSettings = array('x1' => 'y1', 'x2' => 'y2');
		$profile->moduleSettings = $moduleSettings;
		$profile->baseColor = 'green';
		$profile->language = 'de_DE@UTF8';
		$profile->pageHeader = 'header';

		$export = $profile->export();
		$importedProfile = selfServiceProfile::import($export);

		$this->assertEquals($moduleSettings, $importedProfile->moduleSettings);
		$this->assertEquals('green', $importedProfile->baseColor);
		$this->assertEquals('de_DE@UTF8', $importedProfile->language);
		$this->assertEquals('header', $importedProfile->pageHeader);
	}

}

?>