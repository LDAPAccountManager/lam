<?php

declare(strict_types=1);

/**
 * DigestAlgorithmTest.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Sign\Config;
use Com\Tecnick\Pdf\Sign\DigestAlgorithm;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Timestamp\Config as TimestampConfig;
use PHPUnit\Framework\TestCase;

/**
 * DigestAlgorithm enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class DigestAlgorithmTest extends TestCase
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('sha256', DigestAlgorithm::Sha256->value);
        $this->assertSame('sha384', DigestAlgorithm::Sha384->value);
        $this->assertSame('sha512', DigestAlgorithm::Sha512->value);
    }

    public function testCasesMatchBothConfigSets(): void
    {
        $values = \array_map(static fn(DigestAlgorithm $case): string => $case->value, DigestAlgorithm::cases());
        $this->assertSame(Config::DIGEST_ALGORITHMS, $values);
        $this->assertSame(TimestampConfig::HASH_ALGORITHMS, $values);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(DigestAlgorithm::Sha256, DigestAlgorithm::fromLoose('sha256'));
        $this->assertSame(DigestAlgorithm::Sha512, DigestAlgorithm::fromLoose('sha512'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(DigestAlgorithm::Sha384, DigestAlgorithm::fromLoose(DigestAlgorithm::Sha384));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (DigestAlgorithm::cases() as $case) {
            $this->assertSame($case, DigestAlgorithm::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownThrows(): void
    {
        $this->expectException(Exception::class);
        DigestAlgorithm::fromLoose('md5');
    }

    public function testConfigAcceptsEnum(): void
    {
        $cfg = new Config(Config::PROFILE_LEGACY, DigestAlgorithm::Sha384);
        $this->assertSame('sha384', $cfg->digestAlgorithm);
    }

    public function testTimestampConfigAcceptsEnum(): void
    {
        $cfg = new TimestampConfig(host: 'https://tsa.example.org', hashAlgorithm: DigestAlgorithm::Sha512);
        $this->assertSame('sha512', $cfg->hashAlgorithm);
    }
}
