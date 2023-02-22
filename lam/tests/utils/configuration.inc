<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2014 - 2021  Roland Gruber

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

include_once (dirname(__FILE__) . '/../../lib/config.inc');
include_once (dirname(__FILE__) . '/../lib/LAMConfigTest.php');

/**
 * Creates a default configuration and adds it to $_SESSION['cfgMain'] and $_SESSION['config'].
 * @throws LAMException
 */
function testCreateDefaultConfig() {
	$cfgMain = new LAMCfgMain();
	$cfgMain->logDestination = 'NONE';
	$_SESSION['cfgMain'] = $cfgMain;
	$cfgPath = dirname(__FILE__) . '/../../config/' . LAMConfigTest::FILE_NAME . '.conf';
	if (file_exists($cfgPath)) {
		unlink($cfgPath);
	}
	touch($cfgPath);
	$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
	$config = $serverProfilePersistenceManager->loadProfile(LAMConfigTest::FILE_NAME);
	$_SESSION['config'] = $config;
	$_SESSION['language'] = 'en_GB.utf8:UTF-8:English (Great Britain)';
}

function testDeleteDefaultConfig() {
	$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
	try {
		$serverProfilePersistenceManager->deleteProfile('phpunit');
	} catch (LAMException $e) {
		logNewMessage(LOG_ERR, $e->getTitle());
	}
}
