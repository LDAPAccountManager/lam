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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Sign\Config;
use Com\Tecnick\Pdf\Sign\Exception;
use PHPUnit\Framework\TestCase;

/**
 * Config Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class ConfigTest extends TestCase
{
    public function testDefaultsAreLegacy(): void
    {
        $cfg = new Config();
        $this->assertSame(Config::PROFILE_LEGACY, $cfg->profile);
        $this->assertSame('sha256', $cfg->digestAlgorithm);
        $this->assertSame(2, $cfg->certType);
        $this->assertFalse($cfg->isPades());
        $this->assertSame('adbe.pkcs7.detached', $cfg->subFilter());
    }

    public function testPadesProfile(): void
    {
        $cfg = new Config(Config::PROFILE_PADES_B_T, 'sha384', 1);
        $this->assertTrue($cfg->isPades());
        $this->assertSame('ETSI.CAdES.detached', $cfg->subFilter());
        $this->assertSame('sha384', $cfg->digestAlgorithm);
        $this->assertSame(1, $cfg->certType);
    }

    public function testInvalidProfileThrows(): void
    {
        $this->expectException(Exception::class);
        new Config('bogus');
    }

    public function testInvalidDigestThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(Config::PROFILE_PADES_B_B, 'md5');
    }

    public function testInvalidCertTypeThrows(): void
    {
        $this->expectException(Exception::class);
        new Config(Config::PROFILE_LEGACY, 'sha256', 4);
    }

    public function testFromArrayDefaults(): void
    {
        $cfg = Config::fromArray([]);
        $this->assertSame(Config::PROFILE_LEGACY, $cfg->profile);
        $this->assertSame('sha256', $cfg->digestAlgorithm);
        $this->assertSame(2, $cfg->certType);
    }

    public function testFromArrayValues(): void
    {
        $cfg = Config::fromArray([
            'profile' => Config::PROFILE_PADES_B_LTA,
            'digest_algorithm' => 'sha512',
            'cert_type' => 3,
        ]);
        $this->assertSame(Config::PROFILE_PADES_B_LTA, $cfg->profile);
        $this->assertSame('sha512', $cfg->digestAlgorithm);
        $this->assertSame(3, $cfg->certType);
        $this->assertTrue($cfg->isPades());
    }

    public function testFromArrayInvalidTypeThrows(): void
    {
        $this->expectException(Exception::class);
        Config::fromArray(['cert_type' => '2']);
    }

    public function testFromArrayRejectsNonStringProfile(): void
    {
        $this->expectException(Exception::class);
        Config::fromArray(['profile' => 123]);
    }

    public function testFromArrayRejectsNonStringDigest(): void
    {
        $this->expectException(Exception::class);
        Config::fromArray(['digest_algorithm' => 123]);
    }
}
