<?php

/**
 * MDFiveSixteenTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Test;

/**
 * MD5-16 encryption Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class MDFiveSixteenTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Encrypt\Type\MDFiveSixteen
    {
        return new \Com\Tecnick\Pdf\Encrypt\Type\MDFiveSixteen();
    }

    public function testEncrypt(): void
    {
        $mdFiveSixteen = $this->getTestObject();
        $result = $mdFiveSixteen->encrypt('hello');
        $this->assertEquals('5d41402abc4b2a76b9719d911017c592', \bin2hex($result));
    }
}
