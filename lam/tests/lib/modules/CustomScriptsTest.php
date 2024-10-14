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
				'LAM_SELECTION_ENV: Environment=dev;qa;prod',
				'LAM_SELECTION_TENANT: Tenant=foo;bar',
				'LAM_TEXT_COMMENT: Comment=no comment',
				'LAM_TEXT_AMOUNT:Amount',
				'LAM_GROUP: Group 1',
				'user manual LAMLABEL="echo uid" echo $uid$',
				'LAM_GROUP: Group 2',
				'user manual echo $description$',
				'user postModify echo $dn$',
				'gon preModify echo NEW $member$ OLD $ORIG.member$',
				'',
				'  ',
				'group:group_3 manual echo group3',
				'group:group_3 postCreate echo group3',
				'group preCreate echo group',
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
			$this->assertEquals('Group 1', $script->getGroupLabel());

			$script = $scripts[1];
			$configuredType = new ConfiguredType($typeManager, 'user', 'user');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertTrue($script->isManual());
			$this->assertEquals('echo $description$', $script->getCommand());
			$this->assertEquals('manual', $script->getType());
			$this->assertNull($script->getLabel());
			$this->assertEquals('Group 2', $script->getGroupLabel());

			$script = $scripts[2];
			$configuredType = new ConfiguredType($typeManager, 'user', 'user');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo $dn$', $script->getCommand());
			$this->assertEquals('postModify', $script->getType());
			$this->assertNull($script->getLabel());
			$this->assertEquals(_('Post-modify'), $script->getTypeLabel());

			$script = $scripts[3];
			$configuredType = new ConfiguredType($typeManager, 'gon', 'gon_1');
			$configuredWrongType = new ConfiguredType($typeManager, 'group', 'group_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo NEW $member$ OLD $ORIG.member$', $script->getCommand());
			$this->assertEquals('preModify', $script->getType());
			$this->assertNull($script->getLabel());
			$this->assertEquals(_('Pre-modify'), $script->getTypeLabel());

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
			$this->assertEquals('postCreate', $script->getType());
			$this->assertNull($script->getLabel());
			$this->assertEquals(_('Post-create'), $script->getTypeLabel());

			$script = $scripts[6];
			$configuredType = new ConfiguredType($typeManager, 'group', 'group_3');
			$configuredWrongType = new ConfiguredType($typeManager, 'gon', 'gon_1');
			$this->assertTrue($script->matchesConfiguredType($configuredType));
			$this->assertFalse($script->matchesConfiguredType($configuredWrongType));
			$this->assertFalse($script->isManual());
			$this->assertEquals('echo group', $script->getCommand());
			$this->assertEquals('preCreate', $script->getType());
			$this->assertNull($script->getLabel());
			$this->assertEquals(_('Pre-create'), $script->getTypeLabel());

			$this->assertEquals(4, sizeof($parser->getManualOptions()));
			$this->assertEquals('select', $parser->getManualOptions()[0]['type']);
			$this->assertEquals('select', $parser->getManualOptions()[1]['type']);
			$this->assertEquals('LAM_SELECTION_ENV', $parser->getManualOptions()[0]['name']);
			$this->assertEquals('LAM_SELECTION_TENANT', $parser->getManualOptions()[1]['name']);
			$this->assertEquals('Environment', $parser->getManualOptions()[0]['label']);
			$this->assertEquals('Tenant', $parser->getManualOptions()[1]['label']);
			$this->assertEquals(['dev', 'qa', 'prod'], $parser->getManualOptions()[0]['values']);
			$this->assertEquals(['foo', 'bar'], $parser->getManualOptions()[1]['values']);

			$this->assertEquals('text', $parser->getManualOptions()[2]['type']);
			$this->assertEquals('text', $parser->getManualOptions()[3]['type']);
			$this->assertEquals('LAM_TEXT_COMMENT', $parser->getManualOptions()[2]['name']);
			$this->assertEquals('LAM_TEXT_AMOUNT', $parser->getManualOptions()[3]['name']);
			$this->assertEquals('Comment', $parser->getManualOptions()[2]['label']);
			$this->assertEquals('Amount', $parser->getManualOptions()[3]['label']);
			$this->assertEquals('no comment', $parser->getManualOptions()[2]['default']);
			$this->assertFalse(isset($parser->getManualOptions()[3]['default']));
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
