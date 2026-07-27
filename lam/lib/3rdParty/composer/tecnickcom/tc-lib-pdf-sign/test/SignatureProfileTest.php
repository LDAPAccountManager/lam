<?php

declare(strict_types=1);

/**
 * SignatureProfileTest.php
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
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\SignatureProfile;
use PHPUnit\Framework\TestCase;

/**
 * SignatureProfile enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class SignatureProfileTest extends TestCase
{
    public function testCaseBackingValuesMatchConfigConstants(): void
    {
        $this->assertSame(Config::PROFILE_LEGACY, SignatureProfile::Legacy->value);
        $this->assertSame(Config::PROFILE_PADES_B_B, SignatureProfile::PadesBB->value);
        $this->assertSame(Config::PROFILE_PADES_B_T, SignatureProfile::PadesBT->value);
        $this->assertSame(Config::PROFILE_PADES_B_LT, SignatureProfile::PadesBLT->value);
        $this->assertSame(Config::PROFILE_PADES_B_LTA, SignatureProfile::PadesBLTA->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(SignatureProfile::Legacy, SignatureProfile::fromLoose('legacy'));
        $this->assertSame(SignatureProfile::PadesBLTA, SignatureProfile::fromLoose('pades-b-lta'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(SignatureProfile::PadesBT, SignatureProfile::fromLoose(SignatureProfile::PadesBT));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (SignatureProfile::cases() as $case) {
            $this->assertSame($case, SignatureProfile::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownThrows(): void
    {
        $this->expectException(Exception::class);
        SignatureProfile::fromLoose('bogus');
    }

    public function testConfigAcceptsEnum(): void
    {
        $cfg = new Config(SignatureProfile::PadesBLTA);
        $this->assertSame(Config::PROFILE_PADES_B_LTA, $cfg->profile);
        $this->assertTrue($cfg->isPades());
    }

    public function testFromArrayAcceptsEnum(): void
    {
        $cfg = Config::fromArray(['profile' => SignatureProfile::PadesBB]);
        $this->assertSame(Config::PROFILE_PADES_B_B, $cfg->profile);
    }
}
