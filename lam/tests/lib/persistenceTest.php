<?php
namespace LAM\PERSISTENCE;
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

include_once __DIR__ . '/../../lib/persistence.inc';

/**
 * ConfigDataExporter test case.
 *
 * @author Roland Gruber
 */
class ConfigDataExporterTest extends TestCase {

	public function testExportAsJson() {
		$mainData = array(
			'confMainKey1' => 'val',
			'confMainKey2' => 4,
			'confMainKey3' => '',
		);
		$profileData = array(
			'profile1' => array('ServerURL' => 'myserver'),
			'profile2' => array('ServerURL' => 'myserver2'),
		);
		$accountProfileData = array(
			'profile1' => array('user' => array('default' => array('key' => 'value'))),
			'profile1' => array(
				'user' => array('default' => array('key' => 'value')),
				'group' => array('default' => array('key' => 'value')),
			),
		);
		$accountProfileTemplateData = array(
			'user' => array('default' => array('key' => 'value')),
			'group' => array('default' => array('key' => 'value')),
		);
		$pdfData = array(
			'profile1' => array('structures' => array(
				'user' => array(
					'default' => array('key' => 'value'))
			)),
			'profile1' => array('structures' => array(
				'user' => array('default' => array('key' => 'value')),
				'group' => array('default' => array('key' => 'value')),
			)),
		);
		$pdfTemplateData = array(
			'user' => array('default' => array('key' => 'value')),
			'group' => array('default' => array('key' => 'value')),
		);
		$selfServiceData = array(
			'profile1' => array('key' => 'value'),
			'profile2' => array('key' => 'value'),
		);
		$expectedJson = json_encode(array(
			'mainConfig' => $mainData,
			'certificates' => 'certs',
			'serverProfiles' => $profileData,
			'accountProfiles' => $accountProfileData,
			'accountProfileTemplates' => $accountProfileTemplateData,
			'pdfProfiles' => $pdfData,
			'pdfProfileTemplates' => $pdfTemplateData,
			'selfServiceProfiles' => $selfServiceData,
		));

		$exporter = $this->getMockBuilder('\LAM\PERSISTENCE\ConfigDataExporter')
			->setMethods(array('_getMainConfigData', '_getCertificates', '_getServerProfiles',
				'_getAccountProfiles', '_getAccountProfileTemplates', '_getPdfProfiles',
				'_getPdfProfileTemplates', '_getSelfServiceProfiles'))
			->getMock();
		$exporter->method('_getMainConfigData')->willReturn($mainData);
		$exporter->method('_getCertificates')->willReturn('certs');
		$exporter->method('_getServerProfiles')->willReturn($profileData);
		$exporter->method('_getAccountProfiles')->willReturn($accountProfileData);
		$exporter->method('_getAccountProfileTemplates')->willReturn($accountProfileTemplateData);
		$exporter->method('_getPdfProfiles')->willReturn($pdfData);
		$exporter->method('_getPdfProfileTemplates')->willReturn($pdfTemplateData);
		$exporter->method('_getSelfServiceProfiles')->willReturn($selfServiceData);

		$json = $exporter->exportAsJson();

		$this->assertEquals($expectedJson, $json);
	}

}

