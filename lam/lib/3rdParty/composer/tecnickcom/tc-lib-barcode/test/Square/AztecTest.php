<?php

/**
 * AztecTest.php
 *
 * @since     2023-10-20
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test\Square;

use PHPUnit\Framework\Attributes\DataProvider;
use Test\TestUtil;

/**
 * AZTEC Barcode class test
 *
 * @since     2023-10-20
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 */
class AztecTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Barcode\Barcode
    {
        return new \Com\Tecnick\Barcode\Barcode();
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testInvalidInput(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('AZTEC', '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testCapacityException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $code = \str_pad('', 2000, '0123456789');
        $barcode->getBarcodeObj('AZTEC,100,B,F,3', $code);
    }

    /**
     * Regression: the ECC percentage parameter used to be silently ignored (always 33%).
     *
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testEccParameterRespected(): void
    {
        $barcode = $this->getTestObject();
        $low = $barcode->getBarcodeObj('AZTEC,10', 'TEST DATA 123')->getGrid();
        $high = $barcode->getBarcodeObj('AZTEC,90', 'TEST DATA 123')->getGrid();
        $this->assertNotSame($low, $high);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getGridDataProvider')]
    public function testGetGrid(string $options, string $code, mixed $expected): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('AZTEC' . $options, $code);
        $grid = $type->getGrid();
        $this->assertEquals($expected, \md5($grid));
    }

    /**
     * @return array<array{string, string, string}>
     */
    public static function getGridDataProvider(): array
    {
        return [
            [
                ',100,A,A,0',
                'A',
                'c48da49052f674edc66fa02e52334b17',
            ],
            [
                '',
                ' ABCDEFGHIJKLMNOPQRSTUVWXYZ',
                '74f1e68830f0c635cd01167245743098',
            ],
            [
                '',
                ' abcdefghijklmnopqrstuvwxyz',
                '100ebf910c88922b0ccee88256ba0c81',
            ],
            [
                '',
                ' ,.0123456789',
                'ee2a70b7c88a9e0956b1896983e93f91',
            ],
            [
                '',
                "\r" . '!"#$%&\'()*+,-./:;<=>?[]{}',
                '6965459e50f7c3029de42ef5dc5c1fdf',
            ],
            [
                '',
                "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A"
                    . "\x0B\x0C\x0D\x1B\x1C\x1D\x1E\x1F\x40\x5C"
                    . "\x5E\x5F\x60\x7C\x7E\x7F",
                'b8961abf38519b529f7dc6a20e8f3e59',
            ],
            [
                '',
                'AaB0C#D' . "\x7E",
                '9b1f2af28b8d9d222de93dfe6a09a047',
            ],
            [
                '',
                'aAb0c#d' . "\x7E",
                'f4c58cabbdb5d94fa0cc1c31d510936a',
            ],
            [
                '',
                '#A$a%0&' . "\x7E",
                'a17634a1db6372efbf8ea25a303c38f8',
            ],
            [
                '',
                "\x01A\x01a\x010\x01#",
                'c1a585888c7a1eb424ff98bbf7b32d46',
            ],
            [
                '',
                'PUNCT pairs , . : ' . "\r\n",
                '35281793cc5247b291abb8e3fe5ed853',
            ],
            [
                '',
                'ABCDEabcdeABCDE012345ABCDE?[]{}ABCDE' . "\x01\x02\x03\x04\x05",
                '4ae19b80469a1afff8e490f5afaa8b73',
            ],
            [
                '',
                'abcdeABCDEabcde012345abcde?[]{}abcde' . "\x01\x02\x03\x04\x05",
                'b0158bfe19c6fe20042128d59e40ca3b',
            ],
            [
                '',
                '?[]{}ABCDE?[]{}abcde?[]{}012345?[]{}' . "\x01\x02\x03\x04\x05",
                '71ba0ed8c308c93af6af7cd23a76355a',
            ],
            [
                '',
                "\x01\x02\x03\x04\x05"
                    . 'ABCDE'
                    . "\x01\x02\x03\x04\x05"
                    . 'abcde'
                    . "\x01\x02\x03\x04\x05"
                    . '012345'
                    . "\x01\x02\x03\x04\x05"
                    . '?[]{}',
                'f31e14be0b2c1f903e77af11e6c901b0',
            ],
            [
                '',
                'Lorem ipsum dolor sit amet, consectetur adipiscing elit,'
                    . ' sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.'
                    . ' Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris'
                    . ' nisi ut aliquip ex ea commodo consequat.'
                    . ' Duis aute irure dolor in reprehenderit in voluptate velit esse'
                    . ' cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat'
                    . ' cupidatat non proident,'
                    . ' sunt in culpa qui officia deserunt mollit anim id est laborum.',
                'bb2b103d59e035a581fed0619090f89c',
            ],
            [
                '',
                "\x80\x81\x82\x83\x84",
                'da92b009c1f4430e2f62c76c5f708121',
            ],
            [
                '',
                "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89"
                    . "\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93"
                    . "\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                    . "\x9E\x9F\xA0",
                'f3dfdda6d6fdbd747c86f042fc649193',
            ],
            [
                '',
                "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89"
                    . "\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93"
                    . "\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                    . "\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7"
                    . "\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF\xB0\xB1"
                    . "\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB"
                    . "\xBC\xBD\xBE",
                'ee473dc76e160671f3d1991777894323',
            ],
            [
                '',
                '012345: : : : : : : : ',
                'b7ae80e65d754dc17fe116aaddd33c24',
            ],
            [
                '',
                '012345. , ',
                '92b442e5f1b33be91c576eddc12bcca7',
            ],
            [
                '',
                '012345. , . , . , . , ',
                '598fd97d8a28b1514cd0bf88369c68e9',
            ],
            [
                '',
                '~~~~~~. , ',
                'c40fc61717a7e802d7458897197227b1',
            ],
            [
                '',
                '******. , ',
                'abbe0bdfdc10ad059ad2c415e79dab31',
            ],
            [
                '',
                "\xBC\xBD\xBE" . '. , ',
                'c9ae209e0c6d03014753363affffee8b',
            ],
        ];
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getStringDataProvider')]
    public function testStrings(string $code): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('AZTEC,50,B,F', $code);
        $this->assertNotNull($type); // @phpstan-ignore method.alreadyNarrowedType
    }

    /**
     * @return array<array{string}>
     */
    public static function getStringDataProvider(): array
    {
        return \Test\TestStrings::$data;
    }
}
