<?php

/**
 * SeedTest.php
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
 * Seed Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class SeedTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Encrypt\Type\Seed
    {
        return new \Com\Tecnick\Pdf\Encrypt\Type\Seed();
    }

    public function testEncrypt(): void
    {
        $seed = $this->getTestObject();
        $result = $seed->encrypt('hello', 'world');
        $this->assertNotEmpty($result);
    }

    public function testEncryptRaw(): void
    {
        $seed = $this->getTestObject();
        $result = $seed->encrypt('hello', 'world', 'raw');
        $this->assertNotEmpty($result);
    }
}
