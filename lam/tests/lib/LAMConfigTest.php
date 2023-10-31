<?php
use PHPUnit\Framework\TestCase;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2023  Roland Gruber

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

include_once __DIR__ . '/../utils/configuration.inc';

/**
 * LAMConfig test case.
 *
 * @author Roland Gruber
 */
class LAMConfigTest extends TestCase {

	public const FILE_NAME = 'd_lam_config_test';

	/**
	 *
	 * @var LAMConfig
	 */
	private $lAMConfig;

	private ServerProfilePersistenceManager $serverProfilePersistenceManager;

	/**
	 * Prepares the environment before running a test.
	 * @throws LAMException error occurred
	 */
	protected function setUp(): void {
		parent::setUp();
		$this->serverProfilePersistenceManager = new ServerProfilePersistenceManager();
		testCreateDefaultConfig();
		$profiles = $this->serverProfilePersistenceManager->getProfiles();
		if (in_array(LAMConfigTest::FILE_NAME, $profiles)) {
			$this->serverProfilePersistenceManager->deleteProfile(LAMConfigTest::FILE_NAME);
		}
		$this->serverProfilePersistenceManager->createProfileFromTemplate(LAMConfigTest::FILE_NAME, 'unix.sample', LAMConfigTest::FILE_NAME);
		$this->lAMConfig = $this->serverProfilePersistenceManager->loadProfile(LAMConfigTest::FILE_NAME);
		$profiles = $this->serverProfilePersistenceManager->getProfiles();
		$this->assertTrue(in_array(LAMConfigTest::FILE_NAME, $profiles));
	}

	/**
	 * Cleans up the environment after running a test.
	 * @throws LAMException error occurred
	 */
	protected function tearDown(): void {
		$this->lAMConfig = null;
		$this->serverProfilePersistenceManager->deleteProfile(LAMConfigTest::FILE_NAME);
		$profiles = $this->serverProfilePersistenceManager->getProfiles();
		$this->assertTrue(!in_array(LAMConfigTest::FILE_NAME, $profiles));
		testDeleteDefaultConfig();
		parent::tearDown();
	}

	/**
	 * Tests LAMConfig->getName()
	 */
	public function testGetName() {
		$this->assertEquals(LAMConfigTest::FILE_NAME, $this->lAMConfig->getName());
	}

	public function testIsValidName() {
		$this->assertFalse(LAMConfig::isValidName(''));
		$this->assertFalse(LAMConfig::isValidName('abc.123'));
		$this->assertFalse(LAMConfig::isValidName('abc/123'));
		$this->assertTrue(LAMConfig::isValidName('123-_xyAB'));
	}

	/**
	 * Tests LAMConfig->isWritable()
	 */
	public function testIsWritable() {
		$serverProfilesPersistenceManager = new ServerProfilePersistenceManager();
		$this->assertTrue($serverProfilesPersistenceManager->isWritable(LAMConfigTest::FILE_NAME));
	}

	/**
	 * Tests LAMConfig->get_ServerURL() and LAMConfig->set_ServerURL().
	 */
	public function testServerURL() {
		$url = 'ldap://localhost:123';
		$this->lAMConfig->set_ServerURL($url);
		$this->assertEquals($url, $this->lAMConfig->get_ServerURL());
		$this->doSave();
		$this->assertEquals($url, $this->lAMConfig->get_ServerURL());
	}

	/**
	 * Tests LAMConfig->get_ServerDisplayName() and LAMConfig->set_ServerDisplayName().
	 */
	public function testServerDisplayName() {
		$url = 'ldap://localhost:123';
		$name = 'PROD';
		$this->lAMConfig->set_ServerURL($url);
		$this->lAMConfig->setServerDisplayName('');
		$this->assertEquals('', $this->lAMConfig->getServerDisplayName());
		$this->assertEquals($url, $this->lAMConfig->getServerDisplayNameGUI());
		$this->doSave();
		$this->assertEquals('', $this->lAMConfig->getServerDisplayName());
		$this->assertEquals($url, $this->lAMConfig->getServerDisplayNameGUI());

		$this->lAMConfig->setServerDisplayName($name);
		$this->assertEquals($name, $this->lAMConfig->getServerDisplayNameGUI());
		$this->assertEquals($name, $this->lAMConfig->getServerDisplayName());
		$this->doSave();
		$this->assertEquals($name, $this->lAMConfig->getServerDisplayNameGUI());
		$this->assertEquals($name, $this->lAMConfig->getServerDisplayName());
	}

	/**
	 * Tests LAMConfig->getUseTLS() and LAMConfig->setUseTLS()
	 */
	public function testUseTLS() {
		$this->assertFalse($this->lAMConfig->setUseTLS('123'));
		$val = 'yes';
		$this->lAMConfig->setUseTLS($val);
		$this->assertEquals($val, $this->lAMConfig->getUseTLS());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getUseTLS());
	}

	/**
	 * Tests LAMConfig->getFollowReferrals() and LAMConfig->setFollowReferrals()
	 */
	public function testGetFollowReferrals() {
		$val = 'yes';
		$this->lAMConfig->setFollowReferrals($val);
		$this->assertEquals($val, $this->lAMConfig->getFollowReferrals());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getFollowReferrals());
	}

	/**
	 * Tests LAMConfig->getPagedResults() and LAMConfig->setPagedResults()
	 */
	public function testPagedResults() {
		$val = 'yes';
		$this->lAMConfig->setPagedResults($val);
		$this->assertEquals($val, $this->lAMConfig->getPagedResults());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getPagedResults());
	}

	/**
	 * Tests LAMConfig->getReferentialIntegrityOverlay() and LAMConfig->setReferentialIntegrityOverlay()
	 */
	public function testReferentialIntegrityOverlay() {
		$val = 'true';
		$this->lAMConfig->setReferentialIntegrityOverlay($val);
		$this->assertEquals($val, $this->lAMConfig->getReferentialIntegrityOverlay());
		$this->assertTrue($this->lAMConfig->isReferentialIntegrityOverlayActive());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getReferentialIntegrityOverlay());
		$this->assertTrue($this->lAMConfig->isReferentialIntegrityOverlayActive());
	}

	/**
	 * Tests LAMConfig->getHidePasswordPromptForExpiredPasswords() and LAMConfig->setHidePasswordPromptForExpiredPasswords()
	 */
	public function testHidePasswordPromptForExpiredPasswords() {
		$this->assertFalse($this->lAMConfig->isHidePasswordPromptForExpiredPasswords());
		$val = 'true';
		$this->lAMConfig->setHidePasswordPromptForExpiredPasswords($val);
		$this->assertEquals($val, $this->lAMConfig->getHidePasswordPromptForExpiredPasswords());
		$this->assertTrue($this->lAMConfig->isHidePasswordPromptForExpiredPasswords());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getHidePasswordPromptForExpiredPasswords());
		$this->assertTrue($this->lAMConfig->isHidePasswordPromptForExpiredPasswords());
	}

	/**
	 * Tests LAMConfig->get_Admins()
	 */
	public function testGet_Admins() {
		$val = 'cn=admin,dc=test;cn=admin2,dc=test';
		$valInvalid = 'admin;';
		$this->assertFalse($this->lAMConfig->set_Adminstring($valInvalid));
		$this->lAMConfig->set_Adminstring($val);
		$this->assertEquals(['cn=admin,dc=test', 'cn=admin2,dc=test'], $this->lAMConfig->get_Admins());
		$this->doSave();
		$this->assertEquals(['cn=admin,dc=test', 'cn=admin2,dc=test'], $this->lAMConfig->get_Admins());
	}

	/**
	 * Tests LAMConfig->get_Adminstring() and LAMConfig->set_Adminstring()
	 */
	public function testGet_Adminstring() {
		$val = 'cn=admin,dc=test;cn=admin2,dc=test';
		$this->lAMConfig->set_Adminstring($val);
		$this->assertEquals($val, $this->lAMConfig->get_Adminstring());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_Adminstring());
	}

	/**
	 * Tests LAMConfig->check_Passwd()
	 */
	public function testCheck_Passwd() {
		$val = '12345';
		$this->lAMConfig->set_Passwd($val);
		$this->assertTrue($this->lAMConfig->check_Passwd($val));
		$this->doSave();
		$this->assertTrue($this->lAMConfig->check_Passwd($val));
	}

	/**
	 * Tests LAMConfig->get_Suffix() and LAMConfig->set_Suffix()
	 */
	public function testSuffix() {
		$val = 'ou=test1,dc=test';
		$this->lAMConfig->set_Suffix('user', $val);
		$this->assertEquals($val, $this->lAMConfig->get_Suffix('user'));
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_Suffix('user'));
	}

	/**
	 * Tests LAMConfig->get_listAttributes() and LAMConfig->set_listAttributes()
	 */
	public function testlistAttributes() {
		$this->assertFalse($this->lAMConfig->set_listAttributes('12=3,1=23', 'user'));
		$val = '#uid;gidNumber:number';
		$this->lAMConfig->set_listAttributes($val, 'user');
		$this->assertEquals($val, $this->lAMConfig->get_listAttributes('user'));
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_listAttributes('user'));
	}

	/**
	 * Tests LAMConfig->get_defaultLanguage() and LAMConfig->set_defaultLanguage()
	 */
	public function testdefaultLanguage() {
		$this->assertFalse($this->lAMConfig->set_defaultLanguage(true));
		$val = 'en_GB';
		$this->lAMConfig->set_defaultLanguage($val);
		$this->assertEquals($val, $this->lAMConfig->get_defaultLanguage());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_defaultLanguage());
	}

	/**
	 * Tests LAMConfig->getTimeZone() and LAMConfig->setTimeZone()
	 */
	public function testTimeZone() {
		$this->assertFalse($this->lAMConfig->setTimeZone(true));
		$val = 'Europe/Berlin';
		$this->lAMConfig->setTimeZone($val);
		$this->assertEquals($val, $this->lAMConfig->getTimeZone());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTimeZone());
	}

	/**
	 * Tests LAMConfig->get_scriptPath() and LAMConfig->set_scriptPath()
	 */
	public function testscriptPath() {
		$this->assertFalse($this->lAMConfig->set_scriptPath('script'));
		$val = '/some/script';
		$this->lAMConfig->set_scriptPath($val);
		$this->assertEquals($val, $this->lAMConfig->get_scriptPath());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_scriptPath());
		// empty script
		$val = '';
		$this->lAMConfig->set_scriptPath($val);
		$this->assertEquals($val, $this->lAMConfig->get_scriptPath());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_scriptPath());
	}

	/**
	 * Tests LAMConfig->get_scriptServers() and LAMConfig->set_scriptServers()
	 */
	public function testscriptServers() {
		$this->assertFalse($this->lAMConfig->set_scriptServers(';;..'));
		$val = 'server;server';
		$this->assertTrue($this->lAMConfig->set_scriptServers($val));
		$this->assertEquals($val, $this->lAMConfig->get_scriptServers());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_scriptServers());
	}

	/**
	 * Tests LAMConfig->getConfiguredScriptServers().
	 */
	public function testGetConfiguredScriptServers() {
		$val = 'server1;server2:label2;server3:label3:/prefix';
		$this->assertTrue($this->lAMConfig->set_scriptServers($val));
		$servers = $this->lAMConfig->getConfiguredScriptServers();
		$this->assertEquals(3, sizeof($servers));
		$this->doSave();
		$servers = $this->lAMConfig->getConfiguredScriptServers();
		$this->assertEquals(3, sizeof($servers));
		$this->assertEquals('server1', $servers[0]->getServer());
		$this->assertEquals('server2', $servers[1]->getServer());
		$this->assertEquals('server3', $servers[2]->getServer());
		$this->assertEquals('server1', $servers[0]->getLabel());
		$this->assertEquals('label2', $servers[1]->getLabel());
		$this->assertEquals('label3', $servers[2]->getLabel());
		$this->assertEquals('', $servers[0]->getHomeDirPrefix());
		$this->assertEquals('', $servers[1]->getHomeDirPrefix());
		$this->assertEquals('/prefix', $servers[2]->getHomeDirPrefix());
	}

	/**
	 * Tests LAMConfig->get_scriptRights() and LAMConfig->set_scriptRights()
	 */
	public function testscriptRights() {
		$this->assertFalse($this->lAMConfig->set_scriptRights('12345'));
		$val = '755';
		$this->lAMConfig->set_scriptRights($val);
		$this->assertEquals($val, $this->lAMConfig->get_scriptRights());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_scriptRights());
	}

	/**
	 * Tests LAMConfig->getScriptSSHKey() and LAMConfig->SetScriptSSHKey()
	 */
	public function testScriptSSHKey() {
		$val = '/tmp/test';
		$this->lAMConfig->SetScriptSSHKey($val);
		$this->assertEquals($val, $this->lAMConfig->getScriptSSHKey());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getScriptSSHKey());
	}

	/**
	 * Tests LAMConfig->getScriptSSHKeyPassword() and LAMConfig->setScriptSSHKeyPassword()
	 */
	public function testScriptSSHKeyPassword() {
		$val = '12345';
		$this->lAMConfig->setScriptSSHKeyPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getScriptSSHKeyPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getScriptSSHKeyPassword());
	}

	/**
	 * Tests LAMConfig->getScriptUserName() and LAMConfig->setScriptUserName()
	 */
	public function testScriptUserName() {
		$val = 'admin';
		$this->lAMConfig->setScriptUserName($val);
		$this->assertEquals($val, $this->lAMConfig->getScriptUserName());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getScriptUserName());
	}

	/**
	 * Tests LAMConfig->get_searchLimit() and LAMConfig->set_searchLimit()
	 */
	public function testsearchLimit() {
		$this->assertFalse($this->lAMConfig->set_searchLimit('abc'));
		$val = '1024';
		$this->lAMConfig->set_searchLimit($val);
		$this->assertEquals($val, $this->lAMConfig->get_searchLimit());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_searchLimit());
	}

	/**
	 * Tests LAMConfig->get_AccountModules() and LAMConfig->set_AccountModules()
	 */
	public function testAccountModules() {
		$scope = 'user';
		$this->assertFalse($this->lAMConfig->set_AccountModules('abc', $scope));
		$val = ['posixAccount', 'shadowAccount'];
		$this->lAMConfig->set_AccountModules($val, $scope);
		$this->assertEquals($val, $this->lAMConfig->get_AccountModules($scope));
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_AccountModules($scope));
	}

	/**
	 * Tests LAMConfig->set_moduleSettings() and LAMConfig->get_moduleSettings()
	 */
	public function testmoduleSettings() {
		$this->assertFalse($this->lAMConfig->set_moduleSettings('abc'));
		$val = ['posixAccount_123' => ['123'], 'shadowAccount_123' => ['123']];
		$this->lAMConfig->set_moduleSettings($val);
		$this->assertTrue(array_key_exists('posixAccount_123', $this->lAMConfig->get_moduleSettings()));
		$this->assertTrue(array_key_exists('shadowAccount_123', $this->lAMConfig->get_moduleSettings()));
		$this->doSave();
		$this->assertTrue(array_key_exists('posixAccount_123', $this->lAMConfig->get_moduleSettings()));
		$this->assertTrue(array_key_exists('shadowAccount_123', $this->lAMConfig->get_moduleSettings()));
	}

	/**
	 * Tests LAMConfig->get_ActiveTypes() and LAMConfig->set_ActiveTypes()
	 */
	public function testActiveTypes() {
		$val = ['user', 'group'];
		$this->lAMConfig->set_ActiveTypes($val);
		$this->assertEquals($val, $this->lAMConfig->get_ActiveTypes());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->get_ActiveTypes());
	}

	/**
	 * Tests LAMConfig->set_typeSettings() and LAMConfig->get_typeSettings()
	 */
	public function testtypeSettings() {
		$this->assertFalse($this->lAMConfig->set_typeSettings('abc'));
		$val = ['posixAccount_123' => '123', 'shadowAccount_123' => '123'];
		$this->lAMConfig->set_typeSettings($val);
		$this->assertTrue(array_key_exists('posixAccount_123', $this->lAMConfig->get_typeSettings()));
		$this->assertTrue(array_key_exists('shadowAccount_123', $this->lAMConfig->get_typeSettings()));
		$this->doSave();
		$this->assertTrue(array_key_exists('posixAccount_123', $this->lAMConfig->get_typeSettings()));
		$this->assertTrue(array_key_exists('shadowAccount_123', $this->lAMConfig->get_typeSettings()));
	}

	/**
	 * Tests LAMConfig->getToolSettings() and LAMConfig->setToolSettings()
	 */
	public function testGetToolSettings() {
		$this->assertFalse($this->lAMConfig->setToolSettings('abc'));
		$val = ['user_123' => '123', 'group_123' => '123'];
		$this->lAMConfig->setToolSettings($val);
		$this->assertTrue(array_key_exists('user_123', $this->lAMConfig->getToolSettings()));
		$this->assertTrue(array_key_exists('group_123', $this->lAMConfig->getToolSettings()));
		$this->doSave();
		$this->assertTrue(array_key_exists('user_123', $this->lAMConfig->getToolSettings()));
		$this->assertTrue(array_key_exists('group_123', $this->lAMConfig->getToolSettings()));
	}

	/**
	 * Checks isToolActive().
	 */
	public function testIsToolActive() {
		$this->lAMConfig->setToolSettings([
			'tool_hide_tool1' => 'true',
			'tool_hide_tool2' => 'false',
		]);
		$this->assertFalse($this->lAMConfig->isToolActive('tool1'));
		$this->assertTrue($this->lAMConfig->isToolActive('tool2'));
		$this->assertTrue($this->lAMConfig->isToolActive('tool3'));
	}

	/**
	 * Tests LAMConfig->getAccessLevel() and LAMConfig->setAccessLevel()
	 */
	public function testAccessLevel() {
		$val = 100;
		$this->lAMConfig->setAccessLevel($val);
		$this->assertEquals($val, $this->lAMConfig->getAccessLevel());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getAccessLevel());
	}

	/**
	 * Tests LAMConfig->getLoginMethod() and LAMConfig->setLoginMethod()
	 */
	public function testLoginMethod() {
		$val = 'search';
		$this->lAMConfig->setLoginMethod($val);
		$this->assertEquals($val, $this->lAMConfig->getLoginMethod());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLoginMethod());
	}

	/**
	 * Tests LAMConfig->getLoginSearchFilter() and LAMConfig->setLoginSearchFilter()
	 */
	public function testLoginSearchFilter() {
		$val = '(uid=%USER%)';
		$this->lAMConfig->setLoginSearchFilter($val);
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchFilter());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchFilter());
	}

	/**
	 * Tests LAMConfig->getLoginSearchSuffix() and LAMConfig->setLoginSearchSuffix()
	 */
	public function testLoginSearchSuffix() {
		$val = 'ou=people,dc=test';
		$this->lAMConfig->setLoginSearchSuffix($val);
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchSuffix());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchSuffix());
	}

	/**
	 * Tests LAMConfig->setLoginSearchDN() and LAMConfig->getLoginSearchDN()
	 */
	public function testLoginSearchDN() {
		$val = 'cn=admin,ou=people,dc=test';
		$this->lAMConfig->setLoginSearchDN($val);
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchDN());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchDN());
	}

	/**
	 * Tests LAMConfig->setLoginSearchPassword() and LAMConfig->getLoginSearchPassword()
	 */
	public function testLoginSearchPassword() {
		$val = '123456';
		$this->lAMConfig->setLoginSearchPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLoginSearchPassword());
	}

	/**
	 * Tests LAMConfig->getHttpAuthentication() and LAMConfig->setHttpAuthentication()
	 */
	public function testHttpAuthentication() {
		$val = true;
		$this->lAMConfig->setHttpAuthentication($val);
		$this->assertEquals($val, $this->lAMConfig->getHttpAuthentication());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getHttpAuthentication());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthentication() and LAMConfig->setTwoFactorAuthentication()
	 */
	public function testTwoFactorAuthentication() {
		$val = '2fid';
		$this->lAMConfig->setTwoFactorAuthentication($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthentication());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthentication());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationURL() and LAMConfig->setTwoFactorAuthenticationURL()
	 */
	public function testTwoFactorAuthenticationURL() {
		$val = 'http://example.com';
		$this->lAMConfig->setTwoFactorAuthenticationURL($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationURL());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationURL());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationClientId() and LAMConfig->setTwoFactorAuthenticationClientId()
	 */
	public function testTwoFactorAuthenticationClientId() {
		$val = '1234';
		$this->lAMConfig->setTwoFactorAuthenticationClientId($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationClientId());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationClientId());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationSecretKey() and LAMConfig->setTwoFactorAuthenticationSecretKey()
	 */
	public function testTwoFactorAuthenticationSecretKey() {
		$val = '3333key';
		$this->lAMConfig->setTwoFactorAuthenticationSecretKey($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationSecretKey());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationSecretKey());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationDomain() and LAMConfig->setTwoFactorAuthenticationDomain()
	 */
	public function testTwoFactorAuthenticationDomain() {
		$val = 'test.com';
		$this->lAMConfig->setTwoFactorAuthenticationDomain($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationDomain());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationDomain());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationInsecure() and LAMConfig->setTwoFactorAuthenticationInsecure()
	 */
	public function testTwoFactorAuthenticationInsecure() {
		$val = true;
		$this->lAMConfig->setTwoFactorAuthenticationInsecure($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationInsecure());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationInsecure());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationLabel() and LAMConfig->setTwoFactorAuthenticationLabel()
	 */
	public function testTwoFactorAuthenticationLabel() {
		$val = '2falabel';
		$this->lAMConfig->setTwoFactorAuthenticationLabel($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationLabel());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationLabel());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationOptional() and LAMConfig->setTwoFactorAuthenticationOptional()
	 */
	public function testTwoFactorAuthenticationOptional() {
		$val = true;
		$this->lAMConfig->setTwoFactorAuthenticationOptional($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationOptional());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationOptional());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationCaption() and LAMConfig->setTwoFactorAuthenticationCaption()
	 */
	public function testTwoFactorAuthenticationCaption() {
		$val = '2facaption';
		$this->lAMConfig->setTwoFactorAuthenticationCaption($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationCaption());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationCaption());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAuthenticationAttribute() and LAMConfig->setTwoFactorAuthenticationAttribute()
	 */
	public function testTwoFactorAuthenticationAttribute() {
		$val = 'user';
		$this->lAMConfig->setTwoFactorAuthenticationAttribute($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationAttribute());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAuthenticationAttribute());
	}

	/**
	 * Tests LAMConfig->getTwoFactorAllowToRememberDevice() and LAMConfig->setTwoFactorAllowToRememberDevice()
	 */
	public function testTwoFactorAllowToRememberDevice() {
		$val = 'true';
		$this->lAMConfig->setTwoFactorAllowToRememberDevice($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAllowToRememberDevice());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorAllowToRememberDevice());
	}

	/**
	 * Tests LAMConfig->getTwoFactorRememberDeviceDuration() and LAMConfig->setTwoFactorRememberDeviceDuration()
	 */
	public function testTwoFactorRememberDeviceDuration() {
		$val = '12';
		$this->lAMConfig->setTwoFactorRememberDeviceDuration($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorRememberDeviceDuration());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorRememberDeviceDuration());
	}

	/**
	 * Tests LAMConfig->getTwoFactorRememberDevicePassword() and LAMConfig->setTwoFactorRememberDevicePassword()
	 */
	public function testTwoFactorRememberDevicePassword() {
		$val = '012345678901234567890123456789';
		$this->lAMConfig->setTwoFactorRememberDevicePassword($val);
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorRememberDevicePassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getTwoFactorRememberDevicePassword());
	}

	/**
	 * Tests LAMConfig->getHideDnPart() and LAMConfig->setHideDnPart()
	 */
	public function testHideDnPart() {
		$val = 'dc=example,dc=com';
		$this->lAMConfig->setHideDnPart($val);
		$this->assertEquals($val, $this->lAMConfig->getHideDnPart());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getHideDnPart());
	}

	/**
	 * Tests LAMConfig->getLamProMailFrom() and LAMConfig->setLamProMailFrom()
	 */
	public function testLamProMailFrom() {
		$val = 'nobody@example.com';
		$this->lAMConfig->setLamProMailFrom($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailFrom());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailFrom());
	}

	/**
	 * Tests LAMConfig->getLamProMailReplyTo() and LAMConfig->setLamProMailReplyTo()
	 */
	public function testLamProMailReplyTo() {
		$val = 'nobody@example.com';
		$this->lAMConfig->setLamProMailReplyTo($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailReplyTo());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailReplyTo());
	}

	/**
	 * Tests LAMConfig->getLamProMailSubject() and LAMConfig->setLamProMailSubject()
	 */
	public function testLamProMailSubject() {
		$val = 'nobody@example.com';
		$this->lAMConfig->setLamProMailSubject($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailSubject());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailSubject());
	}

	/**
	 * Tests LAMConfig->getLamProMailIsHTML() and LAMConfig->setLamProMailIsHTML()
	 */
	public function testLamProMailIsHTML() {
		$val = true;
		$this->lAMConfig->setLamProMailIsHTML($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailIsHTML());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailIsHTML());
	}

	/**
	 * Tests LAMConfig->getLamProMailAllowAlternateAddress() and LAMConfig->setLamProMailAllowAlternateAddress()
	 */
	public function testLamProMailAllowAlternateAddress() {
		$val = 'none@example.com';
		$this->lAMConfig->setLamProMailAllowAlternateAddress($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailAllowAlternateAddress());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailAllowAlternateAddress());
	}

	/**
	 * Tests LAMConfig->getLamProMailText() and LAMConfig->setLamProMailText()
	 */
	public function testLamProMailText() {
		$val = 'some\r\ntext';
		$this->lAMConfig->setLamProMailText($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailText());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailText());
	}

	/**
	 * Tests LAMConfig->getLamProMailText() and LAMConfig->setLamProMailText() with a value that contains ": "
	 */
	public function testLamProMailTextColoon() {
		$val = 'some: @@uid@@\r\ntext';
		$this->lAMConfig->setLamProMailText($val);
		$this->assertEquals($val, $this->lAMConfig->getLamProMailText());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getLamProMailText());
	}

	/**
	 * Tests LAMConfig->getJobsBindUser() and LAMConfig->setJobsBindUser()
	 */
	public function testJobsBindUser() {
		$val = 'cn=admin,o=test';
		$this->lAMConfig->setJobsBindUser($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsBindUser());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsBindUser());
	}

	/**
	 * Tests LAMConfig->getJobsBindPassword() and LAMConfig->setJobsBindPassword()
	 */
	public function testJobsBindPassword() {
		$val = '12356';
		$this->lAMConfig->setJobsBindPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsBindPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsBindPassword());
	}

	/**
	 * Tests LAMConfig->getJobsDatabase() and LAMConfig->setJobsDatabase()
	 */
	public function testJobsDatabase() {
		$val = 'mysql';
		$this->lAMConfig->setJobsDatabase($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDatabase());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDatabase());
	}

	/**
	 * Tests LAMConfig->getJobsDBHost() and LAMConfig->setJobsDBHost()
	 */
	public function testJobsDBHost() {
		$val = 'someserver';
		$this->lAMConfig->setJobsDBHost($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDBHost());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDBHost());
	}

	/**
	 * Tests LAMConfig->getJobsDBPort() and LAMConfig->setJobsDBPort()
	 */
	public function testJobsDBPort() {
		$val = '1010';
		$this->lAMConfig->setJobsDBPort($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDBPort());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDBPort());
	}

	/**
	 * Tests LAMConfig->getJobsDBUser() and LAMConfig->setJobsDBUser()
	 */
	public function testJobsDBUser() {
		$val = 'user';
		$this->lAMConfig->setJobsDBUser($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDBUser());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDBUser());
	}

	/**
	 * Tests LAMConfig->getJobsDBPassword() and LAMConfig->setJobsDBPassword()
	 */
	public function testJobsDBPassword() {
		$val = '123456';
		$this->lAMConfig->setJobsDBPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDBPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDBPassword());
	}

	/**
	 * Tests LAMConfig->getJobsDBName() and LAMConfig->setJobsDBName()
	 */
	public function testJobsDBName() {
		$val = 'name';
		$this->lAMConfig->setJobsDBName($val);
		$this->assertEquals($val, $this->lAMConfig->getJobsDBName());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobsDBName());
	}

	/**
	 * Tests LAMConfig->setJobSettings() and LAMConfig->getJobSettings()
	 */
	public function testJobSettings() {
		$val = ['setting' => ['123']];
		$this->lAMConfig->setJobSettings($val);
		$this->assertEquals($val, $this->lAMConfig->getJobSettings());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getJobSettings());
	}

	/**
	 * Tests settings of password reset page.
	 */
	public function testPwdResetPageSettings() {
		$val = 'true';
		$this->lAMConfig->setPwdResetAllowScreenPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getPwdResetAllowScreenPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getPwdResetAllowScreenPassword());

		$val = 'true';
		$this->lAMConfig->setPwdResetAllowSpecificPassword($val);
		$this->assertEquals($val, $this->lAMConfig->getPwdResetAllowSpecificPassword());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getPwdResetAllowSpecificPassword());

		$val = 'true';
		$this->lAMConfig->setPwdResetForcePasswordChange($val);
		$this->assertEquals($val, $this->lAMConfig->getPwdResetForcePasswordChange());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getPwdResetForcePasswordChange());

		$val = LAMConfig::PWDRESET_DEFAULT_MAIL;
		$this->lAMConfig->setPwdResetDefaultPasswordOutput($val);
		$this->assertEquals($val, $this->lAMConfig->getPwdResetDefaultPasswordOutput());
		$this->doSave();
		$this->assertEquals($val, $this->lAMConfig->getPwdResetDefaultPasswordOutput());
	}

	/**
	 * Tests LAMConfig->getJobToken()
	 */
	public function testGetJobToken() {
		$token = $this->lAMConfig->getJobToken();
		$this->assertFalse(empty($token));
	}

	/**
	 * Tests the password policy options.
	 */
	public function testPwdPolicy() {
		$pwdPolicyMinLength = '5';
		$pwdPolicyMinLowercase = '6';
		$pwdPolicyMinUppercase = '7';
		$pwdPolicyMinNumeric = '8';
		$pwdPolicyMinSymbolic = '9';
		$this->lAMConfig->setPwdPolicyMinLength($pwdPolicyMinLength);
		$this->lAMConfig->setPwdPolicyMinLowercase($pwdPolicyMinLowercase);
		$this->lAMConfig->setPwdPolicyMinUppercase($pwdPolicyMinUppercase);
		$this->lAMConfig->setPwdPolicyMinNumeric($pwdPolicyMinNumeric);
		$this->lAMConfig->setPwdPolicyMinSymbolic($pwdPolicyMinSymbolic);
		$this->assertEquals($pwdPolicyMinLength, $this->lAMConfig->getPwdPolicyMinLength());
		$this->assertEquals($pwdPolicyMinLowercase, $this->lAMConfig->getpwdPolicyMinLowercase());
		$this->assertEquals($pwdPolicyMinUppercase, $this->lAMConfig->getPwdPolicyMinUppercase());
		$this->assertEquals($pwdPolicyMinNumeric, $this->lAMConfig->getPwdPolicyMinNumeric());
		$this->assertEquals($pwdPolicyMinSymbolic, $this->lAMConfig->getPwdPolicyMinSymbolic());
		$this->doSave();
		$this->assertEquals($pwdPolicyMinLength, $this->lAMConfig->getPwdPolicyMinLength());
		$this->assertEquals($pwdPolicyMinLowercase, $this->lAMConfig->getpwdPolicyMinLowercase());
		$this->assertEquals($pwdPolicyMinUppercase, $this->lAMConfig->getPwdPolicyMinUppercase());
		$this->assertEquals($pwdPolicyMinNumeric, $this->lAMConfig->getPwdPolicyMinNumeric());
		$this->assertEquals($pwdPolicyMinSymbolic, $this->lAMConfig->getPwdPolicyMinSymbolic());
	}

	/**
	 * Checks that number of settings stays constant over multiple saves.
	 */
	public function testMultiSave() {
		$sizeModSettings = sizeof($this->lAMConfig->get_moduleSettings());
		$sizeTypeSettings = sizeof($this->lAMConfig->get_typeSettings());
		$this->doSave();
		$this->assertEquals($sizeModSettings, sizeof($this->lAMConfig->get_moduleSettings()));
		$this->assertEquals($sizeTypeSettings, sizeof($this->lAMConfig->get_typeSettings()));
		$this->doSave();
		$this->assertEquals($sizeModSettings, sizeof($this->lAMConfig->get_moduleSettings()));
		$this->assertEquals($sizeTypeSettings, sizeof($this->lAMConfig->get_typeSettings()));
	}

	/**
	 * Tests the export.
	 */
	public function testExportData() {
		$this->lAMConfig->set_defaultLanguage('lang');
		$this->lAMConfig->set_ServerURL('myserver');
		$this->lAMConfig->set_typeSettings(['typetest' => '1']);
		$this->lAMConfig->set_moduleSettings(['modtest' => '1']);
		$this->lAMConfig->setToolSettings(['tooltest' => '1']);
		$this->lAMConfig->setJobSettings(['jobtest' => '1']);

		$data = $this->lAMConfig->exportData();

		$this->assertEquals('lang', $data['defaultLanguage']);
		$this->assertEquals('myserver', $data['ServerURL']);
		$this->assertEquals(['typetest' => '1'], $data['typeSettings']);
		$this->assertEquals(['modtest' => '1'], $data['moduleSettings']);
		$this->assertEquals(['tooltest' => '1'], $data['toolSettings']);
		$this->assertEquals(['jobtest' => '1'], $data['jobSettings']);
	}

	/**
	 * Tests the import.
	 */
	public function testImportData() {
		$importData = [];
		$importData['ServerURL'] = 'testserver';
		$importData['defaultLanguage'] = 'de_DE.utf8';
		$importData['typeSettings'] = ['typetest' => 'value'];
		$importData['toolSettings'] = ['tooltest' => 'value'];
		$importData['moduleSettings'] = ['modtest' => 'value'];
		$importData['jobSettings'] = ['jobtest' => 'value'];
		$importData['IGNORE_ME'] = 'ignore';

		$this->lAMConfig->importData($importData);

		$this->assertEquals('testserver', $this->lAMConfig->get_ServerURL());
		$this->assertEquals('de_DE.utf8', $this->lAMConfig->get_defaultLanguage());
		$this->assertEquals(['typetest' => 'value'], $this->lAMConfig->get_typeSettings());
		$this->assertEquals(['tooltest' => 'value'], $this->lAMConfig->getToolSettings());
		$this->assertEquals(['modtest' => 'value'], $this->lAMConfig->get_moduleSettings());
		$this->assertEquals(['jobtest' => 'value'], $this->lAMConfig->getJobSettings());
	}

	/**
	 * Tests the import with invalid data.
	 */
	public function testImportData_invalid() {
		$importData = [];
		$importData['ServerURL'] = 'testserver';
		$importData['typeSettings'] = ['typetest' => 'value'];
		$importData['defaultLanguage'] = new LAMLanguage('de_de', 'UTF-8', 'DE');

		$this->expectException(LAMException::class);
		$this->lAMConfig->importData($importData);
	}

	/**
	 * Saves the config
	 *
	 * @throws LAMException error saving config
	 */
	public function doSave() {
		$this->serverProfilePersistenceManager->saveProfile($this->lAMConfig, LAMConfigTest::FILE_NAME);
		$this->lAMConfig = $this->serverProfilePersistenceManager->loadProfile(LAMConfigTest::FILE_NAME);
	}

}

