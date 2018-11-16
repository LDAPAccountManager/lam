<?php
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018  Roland Gruber
 */

if (is_readable('lam/lib/modules/bindDLZ.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	include_once 'lam/lib/modules/bindDLZ.inc';

	/**
	 * Checks the bindDLZ module.
	 *
	 * @author Roland Gruber
	 */
	class bindDLZTest extends PHPUnit_Framework_TestCase {

		public function testIncreaseSerial() {
			$this->assertEquals('1', bindDLZ::increaseSerial(''));
			$this->assertEquals('4', bindDLZ::increaseSerial('3'));
			$this->assertEquals('10', bindDLZ::increaseSerial('9'));
			$date = new DateTime('now', new DateTimeZone('UTC'));
			$dateStr = $date->format('Ymd');
			$this->assertEquals($dateStr . '2', bindDLZ::increaseSerial($dateStr . '1'));
			$this->assertEquals($dateStr . '06', bindDLZ::increaseSerial($dateStr . '05'));
			$this->assertEquals($dateStr . '0010', bindDLZ::increaseSerial($dateStr . '0009'));
			$this->assertEquals($dateStr . '100', bindDLZ::increaseSerial($dateStr . '99'));
			$this->assertEquals($dateStr . '001', bindDLZ::increaseSerial('20010517003'));
			$this->assertEquals('20990517004', bindDLZ::increaseSerial('20990517003'));
		}

	}

}
