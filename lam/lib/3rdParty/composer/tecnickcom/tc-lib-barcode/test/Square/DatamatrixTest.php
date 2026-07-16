<?php

/**
 * DatamatrixTest.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test\Square;

use PHPUnit\Framework\Attributes\DataProvider;
use Test\TestUtil;

/**
 * Datamatrix Barcode class test
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class DatamatrixTest extends TestUtil
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
        $barcode->getBarcodeObj('DATAMATRIX', '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testCapacityException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $code = \str_pad('', 3000, 'X');
        $barcode->getBarcodeObj('DATAMATRIX', $code);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testEncodeTXTC40shiftException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $encode = new \Com\Tecnick\Barcode\Type\Square\Datamatrix\Encode();
        $chr = -1;
        $enc = -1;
        $temp_cw = [];
        $ptr = 0;
        $encode->encodeTXTC40shift($chr, $enc, $temp_cw, $ptr);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testEncodeTXTC40Exception(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $encode = new \Com\Tecnick\Barcode\Type\Square\Datamatrix\Encode();
        $data = "\x80";
        $enc = \Com\Tecnick\Barcode\Type\Square\Datamatrix\Data::ENC_X12;
        $temp_cw = [];
        $ptr = 0;
        $epos = 0;
        $charset = [];
        $encode->encodeTXTC40($data, $enc, $temp_cw, $ptr, $epos, $charset);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getGridDataProvider')]
    public function testGetGrid(string $mode, string $code, mixed $expected): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj($mode, $code);
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
                'DATAMATRIX',
                '0&0&0&0&0&0&_',
                'fffdfdaec33af0788d24cdfa8cba5ac6',
            ],
            [
                'DATAMATRIX',
                '0&0&0&0&0&0&0',
                '10d0faf5a6e7b71829f268218df7e6af',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3',
                '75c6038d90476cec641ad07690989b36',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3x',
                'f020e44d0926d17af7eb21febdb38d53',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3xyz',
                '17420fbffefddb5f1b8abd0d05de724d',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3-',
                'a63372ce839b51294964f0da0ae0f9f9',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3-xy',
                'f65ab07c374c53e2a93016776041de42',
            ],
            [
                'DATAMATRIX',
                '-=-1-=-2-=-3-=x',
                '7a30efdf7616397a1ea2fd5fd95fed2c',
            ],
            [
                'DATAMATRIX',
                '(400)BS2WZ64PA(00)0',
                '9cb7f1c2aa5989909229ef8e4252d61d',
            ],
            [
                'DATAMATRIX',
                '(400)BS2WZ64QA(00)0',
                '0494f709138a1feef5a1c9f14852dbe5',
            ],
            [
                'DATAMATRIX',
                'LD2B 1 CLNGP',
                'f806889d1dbe0908dcfb530f86098041',
            ],
            [
                'DATAMATRIX',
                'XXXXXXXXXNGP',
                'c6f2b7b293a2943bae74f2a191ec4aea',
            ],
            [
                'DATAMATRIX',
                'XXXXXXXXXXXXNGP',
                'f7679d5a7ab4a8edf12571a6866d92bc',
            ],
            [
                'DATAMATRIX',
                'ABCDABCDAB' . "\x80" . 'DABCD',
                '39aca5ed58b922bee369e5ab8e3add8c',
            ],
            [
                'DATAMATRIX',
                '123aabcdefghijklmnopqrstuvwxyzc',
                'b2d1e957af10655d7a8c3bae86696314',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopq',
                'c45bd372694ad7a20fca7d45f3d459ab',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnop',
                '4fc7940fe3d19fca12454340c38e3421',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnopq',
                'a452e658e3096d8187969cbdc930909c',
            ],
            [
                'DATAMATRIX',
                'abcdefghij',
                '8ec27153e5d173aa2cb907845334e68c',
            ],
            [
                'DATAMATRIX',
                '30Q324343430794<OQQ',
                'e67808f91114fb021851098c4cc65b88',
            ],
            [
                'DATAMATRIX',
                '0123456789',
                'cc1fd942bc919b2d09b3c7cf508c6ae4',
            ],
            [
                'DATAMATRIX',
                'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX',
                'c61d8ced313e2a2e79ab56eded67f11a',
            ],
            [
                'DATAMATRIX',
                '10f27ce-acb7-4e4e-a7ae-a0b98da6ed4a',
                '1a56c44e3977f1ac68057230181e49a8',
            ],
            [
                'DATAMATRIX',
                'Hello World',
                'e72650689027fe75d1f9377ec759c710',
            ],
            [
                'DATAMATRIX',
                'https://github.com/tecnickcom/tc-lib-barcode',
                'efed64acfa2ca29024446fa9816be696',
            ],
            [
                'DATAMATRIX',
                'abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdab'
                    . 'cdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd'
                    . 'abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdab'
                    . 'cdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd',
                '4dc0efb6248b3802c2ab7cf123b884d0',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\',
                '1d41ee32691ff75637224e4fbe68a626',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\'
                    . 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\'
                    . 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\',
                '0b2921466e097ff9cc1ad63719430540',
            ],
            [
                'DATAMATRIX',
                "\x80\x8A\x94\x9E",
                '9300000cee5a5f7b3b48145d44aa7fff',
            ],
            [
                'DATAMATRIX',
                '!"£$%^&*()-+_={}[]\'#@~;:/?,.<>|',
                '4993e149fd20569c8a4f0d758b6dfa76',
            ],
            [
                'DATAMATRIX',
                '!"£$',
                '792181edb48c6722217dc7e2e4cd4095',
            ],
            [
                'DATAMATRIX',
                'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\1234567890',
                '7360a5a6c25476711139ae1244f56c29',
            ],
            [
                'DATAMATRIX',
                "\xFE\xFD"
                    . 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*(),./\\'
                    . "\xFC\xFB",
                '0f078e5e5735396312245740484fa6d1',
            ],
            [
                'DATAMATRIX',
                'aABCDEFG',
                'f074dee3f0f386d9b2f30b1ce4ad08a8',
            ],
            [
                'DATAMATRIX',
                '123 45678',
                '6c2e6503625e408fe9a4e392743f31a8',
            ],
            [
                'DATAMATRIX',
                'DATA MATRIX',
                '3ba4f4ef8449d795813b353ddcce4d23',
            ],
            [
                'DATAMATRIX',
                '123ABCD89',
                '7ce2f8433b82c16e80f4a4c59cad5d10',
            ],
            [
                'DATAMATRIX',
                'AB/C123-X',
                '703318e1964c63d5d500d14a821827cd',
            ],
            [
                'DATAMATRIX',
                \str_pad('', 300, "\xFE\xFD\xFC\xFB"),
                'e524bb17821d0461f3db6f313d35018f',
            ],
            [
                'DATAMATRIX',
                'ec:b47'
                    . "\x7F"
                    . '4#P d*b}gI2#DB|hl{!~[EYH*=cmR{lf'
                    . "\x7F"
                    . '=gcGIa.st286. #*"!eG[.Ryr?Kn,1mIyQqC3 6\'3N>',
                '57fbb9bfb7d542e2e5eadb615e6be549',
            ],
            [
                'DATAMATRIX',
                'eA211101A2raJTGL/r9o93CVk4gtpEvWd2A2Qz8jvPc7l8ybD3m'
                    . 'Wel91ih727kldinPeHJCjhr7fIBX1KQQfsN7BFMX00nlS8FlZG+',
                'b2f0d45920c7da5b298bbab5cff5d402',
            ],
            // Square
            [
                'DATAMATRIX,S',
                "\xFF\xFE\xFD\xFC\xFB\xFA\xF9\xF8\xF7\xF6"
                    . "\xF5\xF4\xF3\xF2\xF1\xF0\xEF\xEE\xED\xEC"
                    . "\xEB\xEA\xE9\xE8\xE7\xE6\xE5\xE4\xE3\xE2"
                    . "\xE1\xE0\xDF\xDE\xDD\xDC\xDB\xDA\xD9\xD8"
                    . "\xD7\xD6\xD5\xD4\xD3\xD2\xD1\xD0\xCF\xCE"
                    . "\xCD\xCC\xCB\xCA\xC9\xC8\xC7\xC6\xC5\xC4"
                    . "\xC3\xC2\xC1\xC0\xBF\xBE\xBD\xBC\xBB\xBA"
                    . "\xB9\xB8\xB7\xB6\xB5\xB4\xB3\xB2\xB1\xB0"
                    . "\xAF\xAE\xAD\xAC\xAB\xAA\xA9\xA8\xA7\xA6"
                    . "\xA5\xA4\xA3\xA2\xA1\xA0\x9F\x9E\x9D\x9C"
                    . "\x9B\x9A\x99\x98\x97\x96\x95\x94\x93\x92"
                    . "\x91\x90\x8F\x8E\x8D\x8C\x8B\x8A\x89\x88"
                    . "\x87\x86\x85\x84\x83\x82\x81\x80\x7F\x7E"
                    . "\x7D\x7C\x7B\x7A\x79\x78\x77\x76\x75\x74"
                    . "\x73\x72\x71\x70\x6F\x6E\x6D\x6C\x6B\x6A"
                    . "\x69\x68\x67\x66\x65\x64\x63\x62\x61\x60"
                    . "\x5F\x5E\x5D\x5C\x5B\x5A\x59\x58\x57\x56"
                    . "\x55\x54\x53\x52\x51\x50\x4F\x4E\x4D\x4C"
                    . "\x4B\x4A\x49\x48\x47\x46\x45\x44\x43\x42"
                    . "\x41\x40\x3F\x3E\x3D\x3C\x3B\x3A\x39\x38"
                    . "\x37\x36\x35\x34\x33\x32\x31\x30\x2F\x2E"
                    . "\x2D\x2C\x2B\x2A\x29\x28\x27\x26\x25\x24"
                    . "\x23\x22\x21\x20\x1F\x1E\x1D\x1C\x1B\x1A"
                    . "\x19\x18\x17\x16\x15\x14\x13\x12\x11\x10"
                    . "\x0F\x0E\x0D\x0C\x0B\x0A\x09\x08\x07\x06"
                    . "\x05\x04\x03\x02\x01",
                '514963c4fde0cee7ff91f76dd56015cc',
            ],
            // Rectangular shape
            [
                'DATAMATRIX,R',
                '01234567890',
                'd3811e018f960beed6d3fa5e675e290e',
            ],
            [
                'DATAMATRIX,R',
                '01234567890123456789',
                'fe3ecb042dabc4b40c5017e204df105b',
            ],
            [
                'DATAMATRIX,R',
                '012345678901234567890123456789',
                '3f8e9aa4413b90f7e1c2e85b4471fd20',
            ],
            [
                'DATAMATRIX,R',
                '0123456789012345678901234567890123456789',
                'b748b02c1c4cae621a84c8dbba97c710',
            ],
            // Rectangular GS1
            [
                'DATAMATRIX,R,GS1',
                "\xE8" . '01034531200000111719112510ABCD1234',
                'f55524d239fc95072d99eafe5363cfeb',
            ],
            [
                'DATAMATRIX,R,GS1',
                "\xE8" . '01095011010209171719050810ABCD1234' . "\xE8" . '2110',
                'e17f2a052271a18cdc00b161908eccb9',
            ],
            [
                'DATAMATRIX,R,GS1',
                "\xE8" . '01034531200000111712050810ABCD1234' . "\xE8" . '4109501101020917',
                '31759950f3253805b100fedf3e536575',
            ],
            // Square GS1
            [
                'DATAMATRIX,S,GS1',
                "\xE8" . '01034531200000111719112510ABCD1234',
                'c9efb69a62114fb6a3d2b52f139a372a',
            ],
            [
                'DATAMATRIX,S,GS1',
                "\xE8" . '01095011010209171719050810ABCD1234' . "\xE8" . '2110',
                '9630bdba9fc79b4a4911fc465aa08951',
            ],
            [
                'DATAMATRIX,S,GS1',
                "\xE8" . '01034531200000111712050810ABCD1234' . "\xE8" . '4109501101020917',
                'a29a330a01cce34a346cf7049e2259ee',
            ],
            // Different encoding datamatrix
            [
                'DATAMATRIX,S,N,ASCII',
                '01234567890',
                'ac7dd9e1ebdb42d07fe928fb33cd307b',
            ],
            [
                'DATAMATRIX,S,N,C40',
                '01234567890',
                '958a7a3bcd036d7135489eb703a25633',
            ],
            [
                'DATAMATRIX,S,N,TXT',
                '01234567890',
                '057981dfbf527b029ae59d65fb55f61d',
            ],
            [
                'DATAMATRIX,S,N,X12',
                '01234567890',
                '8d75b0fcfb2d0977abd95004a6ba98dd',
            ],
            [
                'DATAMATRIX,S,N,EDF',
                '01234567890',
                '989eab3ca16c97e05dd2307bef32f64b',
            ],
            [
                'DATAMATRIX,S,N,BASE256',
                '01234567890',
                '8b4f688a774130bc654e39dfcfadb482',
            ],
            [
                'DATAMATRIX,S,GS1',
                // \xE8 is the control character FNC1 (ASCII 232)
                // Expected read:
                //     (01)03453120000011
                //     (17)191125
                //     (10)ABCD1234
                //     (21)10
                "\xE8" . '01034531200000111719112510ABCD1234' . "\xE8" . '2110',
                '3c66c6c7355e7dea071501216e894eac',
            ],
            [
                'DATAMATRIX,S,GS1',
                // \xE8 is the control character FNC1 (ASCII 232)
                // \x1D is the control character <GS> (ASCII 29)
                // Expected read:
                //     (01)03453120000011
                //     (17)191125
                //     (10)ABCD1234
                //     (21)10
                "\xE8" . '01034531200000111719112510ABCD1234' . "\x1D" . '2110',
                '3c66c6c7355e7dea071501216e894eac',
            ],
            [
                'DATAMATRIX,S,GS1,C40',
                // \xE8 is the control character FNC1 (ASCII 232)
                "\xE8" . '01095011010209171719050810ABCD1234' . "\xE8" . '2110',
                'ba117111dfa40a40e1bb968c719d2eef',
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
        $type = $barcode->getBarcodeObj('DATAMATRIX', $code);
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
