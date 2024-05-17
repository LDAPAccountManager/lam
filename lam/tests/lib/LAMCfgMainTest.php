<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2020 - 2023  Roland Gruber

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

	private ?\LAMCfgMain $conf = null;
	private string $file;

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
	}

	/**
	 * Mail related settings
	 */
	public function testMail() {
		$this->assertEquals(LAMCfgMain::MAIL_ATTRIBUTE_DEFAULT, $this->conf->getMailAttribute());
		$this->assertEquals(LAMCfgMain::MAIL_BACKUP_ATTRIBUTE_DEFAULT, $this->conf->getMailBackupAttribute());

		$this->conf->mailServer = 'server:123';
		$this->conf->mailPassword = 'pwd123';
		$this->conf->mailUser = 'user123';
		$this->conf->mailEncryption = LAMCfgMain::SMTP_SSL;
		$this->conf->mailAttribute = 'test';
		$this->conf->mailBackupAttribute = 'test2';

		$this->conf->save();
		$this->conf = new LAMCfgMain($this->file);

		$this->assertEquals('server:123', $this->conf->mailServer);
		$this->assertEquals('pwd123', $this->conf->mailPassword);
		$this->assertEquals('user123', $this->conf->mailUser);
		$this->assertEquals(LAMCfgMain::SMTP_SSL, $this->conf->mailEncryption);
		$this->assertEquals('test', $this->conf->getMailAttribute());
		$this->assertEquals('test2', $this->conf->getMailBackupAttribute());
	}

	/**
	 * License related settings.
	 */
	public function testLicense() {
		$timestamp = '12345';
		$this->assertEquals(LAMCfgMain::LICENSE_WARNING_SCREEN, $this->conf->getLicenseWarningType());
		$this->assertFalse($this->conf->wasLicenseWarningSent($timestamp));
		$this->conf->licenseEmailTo = 'TO';
		$this->conf->licenseEmailFrom = 'FROM';
		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_ALL;
		$this->conf->setLicenseLines(['123', '456']);
		$this->conf->licenseEmailDateSent = $timestamp;

		$this->conf->save();
		$this->conf = new LAMCfgMain($this->file);

		$this->assertEquals('TO', $this->conf->licenseEmailTo);
		$this->assertEquals('FROM', $this->conf->licenseEmailFrom);
		$this->assertEquals($timestamp, $this->conf->licenseEmailDateSent);
		$this->assertTrue($this->conf->wasLicenseWarningSent($timestamp));
		$this->assertEquals(LAMCfgMain::LICENSE_WARNING_ALL, $this->conf->licenseWarningType);
		$this->assertEquals(['123', '456'], $this->conf->getLicenseLines());
	}

	/**
	 * License warning type related settings.
	 */
	public function testLicenseWarningTypes() {
		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_ALL;

		$this->assertTrue($this->conf->sendLicenseWarningByEmail());
		$this->assertTrue($this->conf->showLicenseWarningOnScreen());

		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_EMAIL;

		$this->assertTrue($this->conf->sendLicenseWarningByEmail());
		$this->assertFalse($this->conf->showLicenseWarningOnScreen());

		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_SCREEN;

		$this->assertFalse($this->conf->sendLicenseWarningByEmail());
		$this->assertTrue($this->conf->showLicenseWarningOnScreen());

		$this->conf->licenseWarningType = LAMCfgMain::LICENSE_WARNING_NONE;

		$this->assertFalse($this->conf->sendLicenseWarningByEmail());
		$this->assertFalse($this->conf->showLicenseWarningOnScreen());
	}

	/**
	 * Tests the export.
	 */
	public function testExportData() {
		$this->conf->passwordMinLower = 3;
		$this->conf->sessionTimeout = 240;
		$this->conf->logLevel = LOG_ERR;
		$this->conf->mailServer = 'mailserver';

		$data = $this->conf->exportData();

		$this->assertEquals(3, $data['passwordMinLower']);
		$this->assertEquals(240, $data['sessionTimeout']);
		$this->assertEquals(LOG_ERR, $data['logLevel']);
		$this->assertEquals('mailserver', $data['mailServer']);
	}

	/**
	 * Tests the import.
	 */
	public function testImportData() {
		$importData = [];
		$importData['passwordMinLower'] = 3;
		$importData['sessionTimeout'] = 240;
		$importData['logLevel'] = LOG_ERR;
		$importData['mailServer'] = 'mailserver';
		$importData['allowedHosts'] = null;
		$importData['IGNORE_ME'] = 'ignore';

		$this->conf->importData($importData);

		$this->assertEquals(3, $this->conf->passwordMinLower);
		$this->assertEquals(240, $this->conf->sessionTimeout);
		$this->assertEquals(LOG_ERR, $this->conf->logLevel);
		$this->assertEquals('mailserver', $this->conf->mailServer);
		$this->assertNull($this->conf->allowedHosts);
	}

	/**
	 * Tests the import with invalid data.
	 */
	public function testImportData_invalid() {
		$importData = [];
		$importData['passwordMinLower'] = 3;
		$importData['sessionTimeout'] = 240;
		$importData['logLevel'] = LOG_ERR;
		$importData['mailServer'] = new LAMLanguage('de_de', 'UTF-8', 'DE');

		$this->expectException(LAMException::class);
		$this->conf->importData($importData);
	}

	public function testModuleSettings() {
		$settings = ['abc' => 123];
		$this->conf->setModuleSettings($settings);

		$this->assertEquals($settings, $this->conf->getModuleSettings());
	}

}