<?php
namespace LAM\PERSISTENCE;
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

include_once __DIR__ . '/../../lib/persistence.inc';

/**
 * ConfigDataExporter test case.
 *
 * @author Roland Gruber
 */
class ConfigDataExporterTest extends TestCase {

	public function testExportAsJson() {
		$mainData = [
			'confMainKey1' => 'val',
			'confMainKey2' => 4,
			'confMainKey3' => '',
		];
		$profileData = [
			'profile1' => ['ServerURL' => 'myserver'],
			'profile2' => ['ServerURL' => 'myserver2'],
		];
		$accountProfileData = [
			'profile1' => ['user' => ['default' => ['key' => 'value']]],
			'profile2' => ['user' => ['default' => ['key' => 'value']], 'group' => ['default' => ['key' => 'value']]],
		];
		$accountProfileTemplateData = [
			'user' => ['default' => ['key' => 'value']],
			'group' => ['default' => ['key' => 'value']],
		];
		$pdfData = [
			'profile1' => [
				'structures' => [
					'user' => [
						'default' => ['key' => 'value']
					]
				]
			],
			'profile2' => [
				'structures' => [
					'user' => ['default' => ['key' => 'value']],
					'group' => ['default' => ['key' => 'value']],
				]
			],
		];
		$pdfTemplateData = [
			'user' => ['default' => ['key' => 'value']],
			'group' => ['default' => ['key' => 'value']],
		];
		$selfServiceData = [
			'profile1' => ['key' => 'value'],
			'profile2' => ['key' => 'value'],
		];
		$webauthn = [];
		$expectedJson = json_encode([
			'mainConfig' => $mainData,
			'certificates' => 'certs',
			'serverProfiles' => $profileData,
			'accountProfiles' => $accountProfileData,
			'accountProfileTemplates' => $accountProfileTemplateData,
			'pdfProfiles' => $pdfData,
			'pdfProfileTemplates' => $pdfTemplateData,
			'selfServiceProfiles' => $selfServiceData,
			'webauthn' => $webauthn,
			'cronJobs' => []
		]);

		$exporter = $this->getMockBuilder('\LAM\PERSISTENCE\ConfigDataExporter')
			->setMethods(['_getMainConfigData', '_getCertificates', '_getServerProfiles',
				'_getAccountProfiles', '_getAccountProfileTemplates', '_getPdfProfiles',
				'_getPdfProfileTemplates', '_getSelfServiceProfiles', '_getWebauthn',
				'_getCronJobData'])
			->getMock();
		$exporter->method('_getMainConfigData')->willReturn($mainData);
		$exporter->method('_getCertificates')->willReturn('certs');
		$exporter->method('_getServerProfiles')->willReturn($profileData);
		$exporter->method('_getAccountProfiles')->willReturn($accountProfileData);
		$exporter->method('_getAccountProfileTemplates')->willReturn($accountProfileTemplateData);
		$exporter->method('_getPdfProfiles')->willReturn($pdfData);
		$exporter->method('_getPdfProfileTemplates')->willReturn($pdfTemplateData);
		$exporter->method('_getSelfServiceProfiles')->willReturn($selfServiceData);
		$exporter->method('_getWebauthn')->willReturn($webauthn);
		$exporter->method('_getCronJobData')->willReturn([]);

		$json = $exporter->exportAsJson();

		$this->assertEquals($expectedJson, $json);
	}

}

