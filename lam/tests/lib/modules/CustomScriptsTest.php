<?php

namespace lib\modules;

use LAM\TYPES\ConfiguredType;
use LAM\TYPES\TypeManager;
use PHPUnit\Framework\TestCase;

/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2024  Roland Gruber

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

if (is_readable('lam/lib/modules/customScripts.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	include_once 'lam/lib/modules/customScripts.inc';

	/**
	 * Checks the custom scripts.
	 *
	 * @author Roland Gruber
	 *
	 */
	class CustomScriptsTest extends TestCase {

		private $configLines = [];
		private $configLinesSelfService = [];

		protected function setUp(): void {
			$this->configLines = [
				'user manual LAMLABEL="echo uid" echo $uid$',
				'user manual echo $description$',
				'user postModify echo $dn$',
				'gon preModify echo NEW $member$ OLD $ORIG.member$',
				'group:group_3 manual echo group3',
				'group:group_3 postModify echo group3',
				'group preModify echo group',
			];
			$this->configLinesSelfService = [
				'postModify echo $dn$',
				'preModify echo NEW $member$ OLD $ORIG.member$',
			];
		}

		public function testCustomScriptParser() {
			$parser = new \CustomScriptParser();
			$scripts = $parser->parse($this->configLines, false);

			$this->assertNotEmpty($scripts);
			$this->assertEquals(7, count($scripts));
			$typeManager = new TypeManager();

			$script = $scripts[0];
			$configuredType = new ConfiguredType($typeManager, 'user', 'user');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertTrue($script->isManual());
			$this->assertEquals('echo $uid$', $script->getCommand());
			$this->assertEquals('manual', $script->getType());
			$this->assertEquals('echo uid', $script->getLabel());

			$script = $scripts[1];
			$configuredType = new ConfiguredType($typeManager, 'user', 'user');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertTrue($script->isManual());
			$this->assertEquals('echo $description$', $script->getCommand());
			$this->assertEquals('manual', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[2];
			$configuredType = new ConfiguredType($typeManager, 'user', 'user');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo $dn$', $script->getCommand());
			$this->assertEquals('postModify', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[3];
			$configuredType = new ConfiguredType($typeManager, 'gon', 'gon_1');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo NEW $member$ OLD $ORIG.member$', $script->getCommand());
			$this->assertEquals('preModify', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[4];
			$configuredType = new ConfiguredType($typeManager, 'group', 'group_3');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertTrue($script->isManual());
			$this->assertEquals('echo group3', $script->getCommand());
			$this->assertEquals('manual', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[5];
			$configuredType = new ConfiguredType($typeManager, 'group', 'group_3');
			$configuredWrongType = new ConfiguredType($typeManager, 'gon', 'gon_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo group3', $script->getCommand());
			$this->assertEquals('postModify', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[6];
			$configuredType = new ConfiguredType($typeManager, 'group', 'group_3');
			$configuredWrongType = new ConfiguredType($typeManager, 'gon', 'gon_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo group', $script->getCommand());
			$this->assertEquals('preModify', $script->getType());
			$this->assertNull($script->getLabel());
		}

		public function testCustomScriptParserSelfService() {
			$parser = new \CustomScriptParser();
			$scripts = $parser->parse($this->configLinesSelfService, true);

			$this->assertNotEmpty($scripts);
			$this->assertEquals(2, count($scripts));

			$script = $scripts[0];
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo $dn$', $script->getCommand());
			$this->assertEquals('postModify', $script->getType());
			$this->assertNull($script->getLabel());

			$script = $scripts[1];
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo NEW $member$ OLD $ORIG.member$', $script->getCommand());
			$this->assertEquals('preModify', $script->getType());
			$this->assertNull($script->getLabel());
		}

	}

}
