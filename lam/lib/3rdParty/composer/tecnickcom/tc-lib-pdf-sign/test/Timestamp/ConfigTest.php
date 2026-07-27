<?php

declare(strict_types=1);

/**
 * ConfigTest.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Test\Timestamp;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Timestamp\Config;
use PHPUnit\Framework\TestCase;

/**
 * Timestamp Config Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class ConfigTest extends TestCase
{
    public function testDefaults(): void
    {
        $cfg = new Config(host: 'https://tsa.example.org/tsr');
        $this->assertSame('https://tsa.example.org/tsr', $cfg->host);
        $this->assertSame('sha256', $cfg->hashAlgorithm);
        $this->assertSame('', $cfg->policyOid);
        $this->assertTrue($cfg->nonceEnabled);
        $this->assertSame(5, $cfg->timeout);
        $this->assertTrue($cfg->verifyPeer);
    }

    public function testAcceptsValidPolicyOid(): void
    {
        $cfg = new Config(host: 'https://tsa.example.org', policyOid: '1.2.3.4.5');
        $this->assertSame('1.2.3.4.5', $cfg->policyOid);
    }

    public function testEmptyHostThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(host: '');
    }

    public function testInvalidHashAlgorithmThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(host: 'https://tsa.example.org', hashAlgorithm: 'md5');
    }

    public function testInvalidPolicyOidThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(host: 'https://tsa.example.org', policyOid: 'not-an-oid');
    }

    public function testInvalidTimeoutThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(host: 'https://tsa.example.org', timeout: 0);
    }
}
