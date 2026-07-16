<?php

/**
 * OutputTest.php
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
 * Output Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class OutputTest extends TestUtil
{
    /** @param array<string,mixed> $data */
    protected function setRawEncryptData(OutputTestDouble $output, array $data): void
    {
        $property = new \ReflectionProperty(\Com\Tecnick\Pdf\Encrypt\Output::class, 'encryptdata');
        $property->setValue($output, $data);
    }

    /** @return array<string,mixed> */
    protected function getRawEncryptData(OutputTestDouble $output): array
    {
        $property = new \ReflectionProperty(\Com\Tecnick\Pdf\Encrypt\Output::class, 'encryptdata');
        /** @var array<string,mixed> */
        return $property->getValue($output);
    }

    protected function getOutputTestDouble(): OutputTestDouble
    {
        return new OutputTestDouble();
    }

    public function testGetPdfEncryptionObjZero(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, ['print'], 'alpha', 'beta');
            $pon = 122;
            $result = $encrypt->getPdfEncryptionObj($pon);
            $expected =
                '3132332030206f626a0a3c3c0a2f46696c746572202f5374616e646172640a2f5620310a2f4c656e6774682034300a2'
                . 'f5220320a2f4f20280542fa0e15496869a825cd08c633ac10675c5c02167661241f5369895d768278b1290a2f552028550539dc185'
                . 'e79d4c676f803babbdc50acf8a4427d2de5303d59e7c315b30eba290a2f5020323134373432323030380a2f456e63727970744d657'
                . '4616461746120747275650a3e3e0a656e646f626a0a';
            $this->assertEquals($expected, \bin2hex($result));
        });
    }

    public function testGetPdfEncryptionObjOne(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 1, ['print'], 'alpha', 'beta');
            $pon = 122;
            $result = $encrypt->getPdfEncryptionObj($pon);
            $this->assertTrue(\strlen($result) > 150);
        });
    }

    public function testGetPdfEncryptionObjTwo(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 2, ['print'], 'alpha', 'beta');
        $pon = 122;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertTrue(\strlen($result) > 200);
    }

    public function testGetPdfEncryptionObjThree(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $pon = 122;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertTrue(\strlen($result) > 300);
    }

    public function testGetPdfEncryptionObjThreePub(): void
    {
        $pubkeys = [[
            'c' => __DIR__ . '/data/cert.pem',
            'p' => ['print'],
        ]];
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta', $pubkeys);
        $pon = 122;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertTrue(\strlen($result) > 200);
    }

    public function testGetPdfEncryptionObjOnePub(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $pubkeys = [[
                'c' => __DIR__ . '/data/cert.pem',
                'p' => ['print'],
            ]];
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
                true,
                \md5('file_id'),
                1,
                ['print'],
                'alpha',
                'beta',
                $pubkeys,
            );
            $pon = 122;
            $result = $encrypt->getPdfEncryptionObj($pon);
            $this->assertTrue(\strlen($result) > 100);
        });
    }

    public function testGetPdfEncryptionObjFour(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 4, ['print'], 'alpha', 'beta');
        $pon = 122;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertTrue(\strlen($result) > 300);
        $this->assertStringContainsString('/V 6', $result);
        $this->assertStringContainsString('/R 6', $result);
        $this->assertStringContainsString('/Length 256', $result);
    }

    /** Issue 1: EFF entry must appear for V >= 4 when embedded file encryption is enabled. */
    public function testGetPdfEncryptionObjEff(): void
    {
        // V >= 4 (mode 2 = AES-128, V=4) with embedded file encryption enabled (default)
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
            true,
            \md5('file_id'),
            2,
            ['print'],
            'alpha',
            'beta',
            null,
            true, // encryptMetadata
            true, // encryptEmbeddedFiles
        );
        $pon = 0;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertStringContainsString('/EFF /StdCF', $result);
    }

    /** Issue 1: No EFF entry when embedded file encryption is disabled. */
    public function testGetPdfEncryptionObjNoEff(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
            true,
            \md5('file_id'),
            2,
            ['print'],
            'alpha',
            'beta',
            null,
            true, // encryptMetadata
            false, // encryptEmbeddedFiles = false
        );
        $pon = 0;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertStringNotContainsString('/EFF', $result);
    }

    /** Issue 3: EncryptMetadata=false must appear in standard-mode output. */
    public function testGetPdfEncryptionObjEncryptMetadataFalse(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
            true,
            \md5('file_id'),
            3,
            ['print'],
            'alpha',
            'beta',
            null,
            false, // encryptMetadata = false
        );
        $pon = 0;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertStringContainsString('/EncryptMetadata false', $result);
    }

    /** Issue 3: EncryptMetadata=true (default) must appear as true in output. */
    public function testGetPdfEncryptionObjEncryptMetadataTrue(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $pon = 0;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertStringContainsString('/EncryptMetadata true', $result);
    }

    /** Issue 4: mode 4 pubkey output must contain Recipients. */
    public function testGetPdfEncryptionObjFourPub(): void
    {
        $pubkeys = [[
            'c' => __DIR__ . '/data/cert.pem',
            'p' => ['print'],
        ]];
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 4, ['print'], 'alpha', 'beta', $pubkeys);
        $pon = 122;
        $result = $encrypt->getPdfEncryptionObj($pon);
        $this->assertTrue(\strlen($result) > 200);
        $this->assertStringContainsString('/V 6', $result);
    }

    public function testSetMissingValuesCopiesEncryptMetadataFalseToCf(): void
    {
        $output = $this->getOutputTestDouble();
        $data = $this->getRawEncryptData($output);
        if (!isset($data['CF']) || !\is_array($data['CF'])) {
            $this->fail('Missing CF array in encryptdata');
        }

        /** @var array<string,mixed> $cfData */
        $cfData = $data['CF'];
        $data['EncryptMetadata'] = false;
        $cfData['EncryptMetadata'] = true;
        $data['CF'] = $cfData;
        $this->setRawEncryptData($output, $data);

        $output->callSetMissingValues();

        $result = $this->getRawEncryptData($output);
        if (!isset($result['CF']) || !\is_array($result['CF'])) {
            $this->fail('Missing CF array in encryptdata');
        }

        /** @var array<string,mixed> $cfData */
        $cfData = $result['CF'];
        if (!\array_key_exists('EncryptMetadata', $cfData) || !\is_bool($cfData['EncryptMetadata'])) {
            $this->fail('Missing boolean EncryptMetadata in CF array');
        }

        $this->assertFalse($cfData['EncryptMetadata']);
    }

    public function testSetMissingValuesCopiesEncryptMetadataTrueToCf(): void
    {
        $output = $this->getOutputTestDouble();
        $data = $this->getRawEncryptData($output);
        if (!isset($data['CF']) || !\is_array($data['CF'])) {
            $this->fail('Missing CF array in encryptdata');
        }

        /** @var array<string,mixed> $cfData */
        $cfData = $data['CF'];
        $data['EncryptMetadata'] = true;
        $cfData['EncryptMetadata'] = false;
        $data['CF'] = $cfData;
        $this->setRawEncryptData($output, $data);

        $output->callSetMissingValues();

        $result = $this->getRawEncryptData($output);
        if (!isset($result['CF']) || !\is_array($result['CF'])) {
            $this->fail('Missing CF array in encryptdata');
        }

        /** @var array<string,mixed> $cfData */
        $cfData = $result['CF'];
        if (!\array_key_exists('EncryptMetadata', $cfData) || !\is_bool($cfData['EncryptMetadata'])) {
            $this->fail('Missing boolean EncryptMetadata in CF array');
        }

        $this->assertTrue($cfData['EncryptMetadata']);
    }
}
