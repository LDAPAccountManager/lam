<?php
use PHPUnit\Framework\TestCase;
/*
 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2017 - 2021  Roland Gruber
 */

if (is_readable('lam/lib/modules/customFields.inc')) {

	include_once 'lam/lib/baseModule.inc';
	include_once 'lam/lib/modules.inc';
	include_once 'lam/lib/modules/customFields.inc';

	/**
	 * Checks the ppolicy expire job.
	 *
	 * @author Roland Gruber
	 *
	 */
	class CustomFieldsTest extends TestCase {

		public function testReplaceWildcardsSpaces() {
			$originalMiddle = '123((uid))456';
			$originalStart = '((Uid))456';
			$originalEnd = '123((uid))';
			$originalMultiple = '123((uid))456((uid))789';
			$attributesSet = array('uid' => array('111'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123 456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals(' 456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123 ', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123 456 789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttribute() {
			$originalMiddle = '123%uid%456';
			$originalStart = '%Uid%456';
			$originalEnd = '123%uid%';
			$originalMultiple = '123%uid%456%uid%789';
			$attributesSet = array('uid' => array('111'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123111456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('111456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123111', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123111456111789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeFirst() {
			$originalMiddle = '123@uid@456';
			$originalStart = '@Uid@456';
			$originalEnd = '123@uid@';
			$originalMultiple = '123@uid@456@uid@789';
			$attributesSet = array('uid' => array('aBc'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123a456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('a456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123a', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123a456a789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeFirstLower() {
			$originalMiddle = '123?uid?456';
			$originalStart = '?Uid?456';
			$originalEnd = '123?uid?';
			$originalMultiple = '123?uid?456?uid?789';
			$attributesSet = array('uid' => array('Abc'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123a456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('a456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123a', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123a456a789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeFirstUpper() {
			$originalMiddle = '123!uid!456';
			$originalStart = '!Uid!456';
			$originalEnd = '123!uid!';
			$originalMultiple = '123!uid!456!uid!789';
			$attributesSet = array('uid' => array('abc'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123A456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('A456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123A', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123A456A789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeLower() {
			$originalMiddle = '123??uid??456';
			$originalStart = '??Uid??456';
			$originalEnd = '123??uid??';
			$originalMultiple = '123??uid??456??uid??789';
			$attributesSet = array('uid' => array('Abc'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123abc456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('abc456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123abc', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123abc456abc789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeUpper() {
			$originalMiddle = '123!!uid!!456';
			$originalStart = '!!Uid!!456';
			$originalEnd = '123!!uid!!';
			$originalMultiple = '123!!uid!!456!!uid!!789';
			$attributesSet = array('uid' => array('abc'));
			$attributesNotSet = array('uid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123ABC456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('ABC456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123ABC', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123ABC456ABC789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsAttributeMulti() {
			$originalMiddle = '123§memberUid|, §456';
			$originalStart = '§MemberUid|, §456';
			$originalEnd = '123§memberUid|, §';
			$originalMultiple = '123§memberUid|, §456§memberUid|;§789';
			$attributesSet = array('memberUid' => array('abc', 'cde'));
			$attributesNotSet = array('memberUid' => array(''));
			$attributesNotSet2 = array();

			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMiddle));
			$this->assertEquals('123456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMiddle));
			$this->assertEquals('123abc, cde456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMiddle));

			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalStart));
			$this->assertEquals('456', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalStart));
			$this->assertEquals('abc, cde456', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalStart));

			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalEnd));
			$this->assertEquals('123', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalEnd));
			$this->assertEquals('123abc, cde', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalEnd));

			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet, $originalMultiple));
			$this->assertEquals('123456789', customFieldsConstantEntry::replaceWildcards($attributesNotSet2, $originalMultiple));
			$this->assertEquals('123abc, cde456abc;cde789', customFieldsConstantEntry::replaceWildcards($attributesSet, $originalMultiple));
		}

		public function testReplaceWildcardsMixed() {
			$attributes = array(
				'uid' => array('myuser'),
				'street' => array('some street'),
				'memberUid' => array('abc', 'cde')
			);

			$this->assertEquals('myuser SOME STREET S abc- cde', customFieldsConstantEntry::replaceWildcards($attributes, '%uid% !!street!! !street! §memberuid|- §'));
			$this->assertEquals(' MMYUSER', customFieldsConstantEntry::replaceWildcards($attributes, '((uid))!uid!!!uid!!'));
		}

		public function testIsValidDateValue() {
			$field = new customFieldsTextEntry('user');
			$field->setName('testattr');
			$this->assertTrue($field->isValidDateValue(null));
			$this->assertTrue($field->isValidDateValue(''));
			$_POST['customFields_testattr_required'] = 'on';
			$_POST['customFields_testattr_attribute'] = 'testattr';
			$_POST['customFields_testattr_help'] = '';
			$_POST['customFields_testattr_validationType'] = customFieldsTextEntry::TYPE_DATE;
			$_POST['customFields_testattr_calendarFormat'] = 'dd.mm.yy';
			$field->updateFieldOptionsByPostData(false);
			$this->assertFalse($field->isValidDateValue(null));
			$this->assertFalse($field->isValidDateValue(''));
			$this->assertTrue($field->isValidDateValue('20.05.2021'));
			$this->assertFalse($field->isValidDateValue('32.05.2021'));
		}

	}

}
