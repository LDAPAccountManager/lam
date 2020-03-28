<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2020  Roland Gruber

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

include_once __DIR__ . '/../../lib/config.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class LAMCfgMainTest extends TestCase {

	private $conf;
	private $file;

	/**
	 * Prepares the environment before running a test.
	 */
	protected function setUp(): void {
		parent::setUp();
		$tmpFile = tmpfile();
		$this->file = stream_get_meta_data($tmpFile)['uri'];
		fclose($tmpFile);
		$tmpFile = fopen($this->file, 'w+');
		fwrite($tmpFile, "\n");
		fclose($tmpFile);
		$this->conf = new LAMCfgMain($this->file);
	}

	/**
	 * Cleans up the environment after running a test.
	 */
	protected function tearDown(): void {
		$this->conf = null;
		//unlink($this->file);
	}

	/**
	 * Mail related settings
	 */
	public function testMail() {
		$this->conf->mailServer = 'server:123';
		$this->conf->mailPassword = 'pwd123';
		$this->conf->mailUser = 'user123';

		$this->conf->save();
		$this->conf = new LAMCfgMain($this->file);

		$this->assertEquals('server:123', $this->conf->mailServer);
		$this->assertEquals('pwd123', $this->conf->mailPassword);
		$this->assertEquals('user123', $this->conf->mailUser);
	}

	/**
	 * License related settings.
	 */
	public function testLicense() {
		$this->assertEquals(LAMCfgMain::LICENSE_WARNING_SCREEN, $this->conf->getLicenseWarningType());
		$this->conf->licenseEmailTo = 'TO';
		$this->conf->licenseEmailFrom = 'FROM';
		$this->conf->licenseEmailDateSent = 'date';
		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_ALL;
		$this->conf->setLicenseLines(array('123', '456'));

		$this->conf->save();
		$this->conf = new LAMCfgMain($this->file);

		$this->assertEquals('TO', $this->conf->licenseEmailTo);
		$this->assertEquals('FROM', $this->conf->licenseEmailFrom);
		$this->assertEquals('date', $this->conf->licenseEmailDateSent);
		$this->assertEquals(LAMCfgMain::LICENSE_WARNING_ALL, $this->conf->licenseWarningType);
		$this->assertEquals(array('123', '456'), $this->conf->getLicenseLines());
	}

}