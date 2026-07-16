<?php

/**
 * QrCodeTest.php
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
 * QR-Code Barcode class test
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class QrCodeTest extends TestUtil
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
        $barcode->getBarcodeObj('QRCODE', '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testCapacityException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $code = \str_pad('', 4000, 'iVoo{[O17n~>(FXC9{*t1P532}l7E{7/R\' ObO`y?`9G(qjBmu7 GM3ZK!qp|)!P1" sRanqC(:Ky');
        $barcode->getBarcodeObj('QRCODE', $code);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getGridDataProvider')]
    public function testGetGrid(string $options, string $code, mixed $expected): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('QRCODE' . $options, $code);
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
                '',
                '0123456789',
                '89e599523008751db7eef3b5befc37ed',
            ],
            [
                ',L',
                '0123456789',
                '89e599523008751db7eef3b5befc37ed',
            ],
            [
                ',H,NM',
                '0123456789',
                '07c7cc526ed4efdc303ff133f3912619',
            ],
            [
                ',L,8B,0,0',
                '123aeiouàèìòù',
                '1622068066c77d3e6ea0a3ad420d105c',
            ],
            [
                ',H,KJ,0,0',
                'ぎポ亊',
                '00cb1bd983bfe7b3d942c3f89f7643f5',
            ],
            [
                ',H,ST,0,0',
                'ABCdef0123',
                'a09df8f57bfd39faaf08acaa23eca45c',
            ],
            [
                '',
                \str_pad('', 350, '0123456789'),
                '1a4753c428f644794b9ee1515138ecb3',
            ],
            [
                '',
                'abcdefghijklmnopqrstuvwxyz01234567890123456789',
                'b41c13e6bba528cc0390d1797bbac38e',
            ],
            [
                ',H,AN,40,1,0,1,2',
                'abcdefghijklmnopqrstuvwxyz01234567890123456789',
                '5ba221be81b269ab1f105b07bf49b372',
            ],
            [
                '',
                "\x9E\x13\xC0\x08\x47\x71\x6B\xFC\xAB\xA9"
                    . "\x72\x72\xCC\x97\xB7\x14\xB4\x1A\x49\x4C"
                    . "\xC1\x10\x45\xD4\xE8\x5A\xF8\x73\x09\x68"
                    . "\x95\xA7\x7B\x56\xAF\xC1\xC7\x1B\xBE\x73"
                    . "\xC4\x32\xE4\x92\xC9\x9C\xA5\x7E\xB6\xED"
                    . "\xC9\x79\xFD\x0F\x4E\xE7\x69\x48\x5C\x72"
                    . "\xAF\xF0\x1A\x2B\x47\xC8\xEC\x0F\xE3\xAC"
                    . "\x81\xA9\xDD\x67\x3C\xA7\x05\xE1\x27\xBA"
                    . "\xD0\xF0\x34\xCE\xFE\x82\xB7\x69\xC9\x14"
                    . "\xDA\x7A\x05\xF4\xA5\x4C\xBD\x92\x5B\xA2"
                    . "\x3F\xDC\x4C\x1E\x44\x87\xC4\x49\x6A\xEB"
                    . "\x05\x3B\xDC\x38\x0B\xDC\xBA\xC2\x46\x84"
                    . "\xD5\x22\xFE\xDA\x17\xA4\x28\xD4\x38\x82"
                    . "\x77\x76\x5F\xC2\x94\xA3\x4B\x5A\xEC\xB4"
                    . "\x46\xF0\xEF\x23\x2A\xFA\xFE\xE3\xBD\x46"
                    . "\x69\x94\x67\x68\x70\x7E\x0D\x97\x53\x44"
                    . "\x1B\xC9\xBA\x79\x8D\x50\x1E\xD7\xA9\x0C"
                    . "\x8D\xEE\xFB\x7E\x12\x27\x79\x12\x0C\x38"
                    . "\x58\x74\xCB\xBE\xDC\x3C\x3D\xE9\xD3\x90"
                    . "\x2F\xED\x5A\xE8\x68\xE6\x39\x86\xBF\xE2"
                    . "\x91\x4D\xD1\x8E\xCA\xE3\xB4\x45\xF5\xBF"
                    . "\x7C\x4E\x35\x49\x0D\x12\x85\x4A\xFA\x59"
                    . "\xD9\x2A\x47\x35\x14\xAF\x1D\x4D\x36\xDB"
                    . "\x30\xC6\x29\x03\x55\xF3\xE5\x0B\x39\xDB"
                    . "\xC9\xB4\x2B\xFD\xFC\x38\x11\x83\x81\x0C"
                    . "\xDB\x5C\x36\x24\x91\x4A\xD2\xAD\x97\x09"
                    . "\x89\xC6\xCF\xB2\xC9\x26\xA6\xAF\x30\xDF"
                    . "\x8C\xF9\x95\xB6\xF8\x93\xED\x0A\x17\x70"
                    . "\x16\xF1\xCC\x4C\x17\x5E\x96\xE8\x0D\x2E"
                    . "\xF1\x95\xF3\xC1\x49\xBE\xE6\xEF\x6E\x18",
                '6a418df382be3ad883e29c92ff7c4a6b',
            ],
            [
                '',
                "\x9E\xC6\x2D\x5E\xE7\x53\x3C\x2D\x68\x3E"
                    . "\x7A\x58\xF5\x08\x15\x4C\xAA\xFA\x57\x06"
                    . "\xA2\x20\x2B\xD0\x11\x60\x8B\x2B\xC0\x0F"
                    . "\x39\x5F\xD4\x66\xBD\xBC\xB8\xF9\xE9\x22"
                    . "\x38\x65\x7A\x2F\x6C\x8E\x7A\x18\x89\xD1"
                    . "\x1E\x2D\xF0\x48\xFD\x02\xA7\x8A\x2C\x69"
                    . "\x98\x65\xC8\x6D\xCA\x87\x2B\x84\x81\x15"
                    . "\xA5\xB9\x79\x20\xE7\xE5\xAE\x63\xFC\x39"
                    . "\x35\x1B\x65\x26\x63\x64\x27\x0C\xED\x53"
                    . "\x74\x86\xB7\x3E\xF3\x83\xC4\x1F\x08\x46"
                    . "\x34\xAC\xFE\xAD\xCC\xE7\x93\x7C\x4B\x91"
                    . "\xB4\x80\xAC\x1B\xA5\x10\x7E\xCC\x1B\x6D"
                    . "\x21\x8F\xF2\xD8\xCC\xE7\x5C\x91\x07\x63"
                    . "\xD6\x3B\x11\xD6\xE7\xDD\xBE\x7C\x5A\x0B"
                    . "\x0E\x10\x8A\xBA\x2B\x31\xC9\xA8\xFC\xE3"
                    . "\x16\x1F\x74\x0A\xF6\x41\xF0\x53\xD1\xF7"
                    . "\xB6\xA9\x34\xC7\x81\x1D\xA5\x41\x98\x01"
                    . "\x4B\xA6\x11\xD4\x61\x3B\x07\x2C\xE3\x04"
                    . "\x11\xF9\x23\x84\x05\x1A\xC4\xF4\x6C\x97"
                    . "\xED\x24\x42\x22\xEA\xC2\x3F\x91\x04\xD6"
                    . "\x92\x4F\x7E\xA2\x24\xDE\xDC\x2A\x0C\xC1"
                    . "\x2E\x1C\xBB\x50\x9F\xBF\x6A\x64\xB5\xD5"
                    . "\xFB\xA4\xF9\x3E\xC6\xE4\x02\x06\x77\x05"
                    . "\xDC\x0A\x53\x5B\xAB\x77\x3B\x89\xA1\x46"
                    . "\x4B\xCF\x61\x08\x21\x01\xC6\x8A\x65\x7D"
                    . "\x61\x61\x22\x5B\x9F\xE7\x41\xA0\xED\xB7"
                    . "\xA5\xCA\xC0\xF8\x26\x6C\x70\x60\xF5\x13"
                    . "\xA6\x41\xE1\x08\x48\x04\x09\x10\x8D\x6D"
                    . "\x8D\xED\xCE\xAE\x49\x6F\x97\x8A\x10\x85"
                    . "\x42\xB4\x51\x03\xAD\x76\x6F\x1F\xD6\x65"
                    . "\x31\x7D\xA6\x14\x85\xEE\x17\x8D\xFE\xA3"
                    . "\xFA\x8C\x92\xCA\x3C\xDB\x3A\xD3\x66\x49"
                    . "\x5A\xA7\xFD\xAA\xAA\xAC\x22\x1B\xCA\xF7"
                    . "\x80\xFB\x75\x27\x11\xF9\x17\x27\x88\x16"
                    . "\xCA\x83\xA2\x5E\x4E\xDE\x3A\x88\xB2\x9F"
                    . "\xD1\x0E\x48\xCF\xB7\xF2\x7C\xD9\x0E\x48"
                    . "\xD1\x8D\x45\x48\xB4\x55\x43\xCA\x7C\xCA"
                    . "\xE0\x48\x4F\x83\xA5\x9D\x63\xDF\x26\x17"
                    . "\x80\xF6\x24\xC7\xC7\xDB\xBB\x45\xB5\xC8"
                    . "\x8C\x88\x56\xD0\xCF\x0B\x27\x13\xD5\xA2"
                    . "\xDD\xB6\xE9\x2E\x3B\x90\xCA\x9D\x70\xF0"
                    . "\xB3\xF0\xE7\xD7\xB9\xB0\xB4\x75\xF4\x6A"
                    . "\x3E\x81\xF2\x94\x53\xC2\x9F\x79\xD5\x75"
                    . "\x1D\xB3\x2C\x08\xE0\x67\x97\xAC\x05\x09"
                    . "\x9D\xB8\xF8\x86\x91\xB3\x37\x46\x29\x2C"
                    . "\xB0\x66\xAD\xA3\xFA\x02\x67\x9A\x7B\x3D"
                    . "\x10\x97\xF0\x3C\x9F\xD2\xA2\x37\x7F\xA7"
                    . "\x40\x1E\x61\x3A\xA3\xF1\xEC\xDA\x39\x16"
                    . "\x08\xE9\x7C\xB4\x8D\x77\xB6\xF3\x12\x33"
                    . "\x32\x22\xC9\x23\x5D\x6A\xF4\x01\xA1\x74"
                    . "\xA7\xE0\x92\x09\x1C\x36\xFA\x0A\x11\x35"
                    . "\x20\x18\x1F\x9B\xCC\xAC\x14\x84\xA0\x26"
                    . "\xB6\xD1\x48\x81\xF3\xA4\xEA\xE9\xA4\x8C"
                    . "\x5F\x4D\x6E\xF0\x56\x89\x27\x51\x92\x38"
                    . "\x86\xB1\x50\xA4\x4E\x1E\x51\x62\xA1\xF1"
                    . "\x87\x58\xC4\xCE\xD8\xB9\x74\xC3\xA3\x1A"
                    . "\x51\x03\x66\xBE\xF3\xBB\x48\x1C\x0E\xDA"
                    . "\x53\x93\x8D\xA3\x39\xDA\xC0\x89\x3D\x62"
                    . "\x7C\xC4\xBA\x41\x94\x93\xF9\x09\x58\x9D"
                    . "\x22\xA8\xA0\x87\x67\x94\x44\xAF\xB0\x51"
                    . "\x8A\x04\xE4\x19\xA7\x1E\xF2\x68\xA7\x31"
                    . "\xCA\x24\xF4\x85\x64\x8A\x1A\x5E\x92\x71"
                    . "\xFB\xB4\x1B\x9D\x3D\x81\x33\x80\x32\xE2"
                    . "\xD1\xBC\xE6\xB6\xD4\x8E\xD4\xC8\xF6\x7C"
                    . "\xF8\xC1\x9F\xEE\x47\x04\x79\x60\x62\x0D"
                    . "\xD1\x5F\xC1\xEB\xFB\xFD\x6E\x2F\x7F\x9F"
                    . "\x12\x51\x5D\xF7\x09\x32\x87\xDC\xF9\x7E"
                    . "\x5A\xF3\x40\xF8\xE3\x87\xFC\x5E\xE7\x60"
                    . "\x6A\xBA\xBE\x2C\xA6\xBB\x2B\x15\xE9\xA9"
                    . "\xB4\xFB\xFA\x12\xF4\x05\x44\x7C\xE1\x3F"
                    . "\xFA\x3C\x34\x3B\x35\x18\xC2\x33\x75\xAB"
                    . "\x92\xDF\x66\x52\x0C\x0D\x0E\x36\x22\xF6"
                    . "\xDF\xD6\xF3\xDA\xE8\xE8\xDE\x2D\x66\xC0"
                    . "\x6C\x61\xFC\x9F\x9C\x32\xB7\x60\x65\x2D"
                    . "\x0C\xF6\x0D\x71\x49\x19\x7E\x57\x4F\xA0"
                    . "\x4E\x2F\x77\x43\x0B\x60\x2D\xE9\x8D\x92"
                    . "\xAB\xF9\xF2\xA8\x99\x8F\xDA\x51\xEE\x40"
                    . "\x7E\xFA\x37\x8B\x6D\x80\xA3\xEA\xD6\xF2"
                    . "\x8B\x26\x22\x04\x68\x2D\x64\x94\x17\xF1"
                    . "\x28\xC2\xEB\x1B\x6B\x85\xAA\x46\xD6\x9A"
                    . "\x85\x56\x95\xBC\xE0\x03\x3D\x85\xED\x15"
                    . "\x79\x7A\x3B\x9A\x7D\xA3\xC7\xE1\x38\xDE"
                    . "\xD3\x60\xA1\xBF\x7A\x0D\x46\x26\x52\x1E"
                    . "\xBF\xD7\x73\x56\x94\x55\x59\xD1\xDA\x47"
                    . "\xE6\x54\xC1\x22\xEE\x3F\xC4\xB6\x22\xFC"
                    . "\x95\xF4\x5D\x37\xB4\xD7\x44\xF9\xFC\x96"
                    . "\x18\xBD\x6E\x8B\x15\x04\xDF\x6D\xD5\xBA"
                    . "\xB4\xBC\x10\x76\xDD\xFD\xB5\xA3\xB4\xD6"
                    . "\xA0\x4B\xCB\xFC\x82\x81\xD5\xC5\x7C\xD2"
                    . "\x5C\x94\x91\xCA\x20\xA6\xCD\x01\x15\xA4"
                    . "\xBB\xC8\x61\xCA\x40\x40\xC8\xF5\xE2\x7E"
                    . "\xCD\x84\xC9\x9A\x82\x4C\x1C\x58\x12\x98"
                    . "\x2C\x6E\x2D\xBC\x39\x4C\x64\x08\x4C\x78"
                    . "\xAC\x09\x41\x0E\xD2\x81\x4E\x9C\x78\x32"
                    . "\x1C\x46\xB5\xE4\xDF\x38\x31\xFB\x8F\x43"
                    . "\x94\xBB\xB0\xC0\x78\xE9\x0E\xDB\xF1\x5A"
                    . "\x55\x9E\x62\x96\xAC\x36\x18\xF9\xD1\x8F"
                    . "\x2D\xEC\xD5\xE1\xD2\xB6\x1B\x04\xB3\xA9"
                    . "\x46\x48\x65\xF6\x0A\xDD\xE1\x18\xBA\xD4"
                    . "\x71\x10\x73\xD3\xA5\x21\x0A\xBD\x1C\xDB",
                'ffed7d96ae53a9c63c88d426680833dd',
            ],
            [
                ',H,NM',

                "\x1A\x97\xE0\xC1\x47\x20\xA2\xBF\x1E\x62"
                    . "\x52\x0D\x99\x91\x45\x03\x7B\xE3\x32\xC8"
                    . "\x18\xEA\x80\x51\xE8\x70\x93\x2D\x19\x08"
                    . "\x26\x33\x9E\x07\xF3\xE5\x27\x96\xA5\x45"
                    . "\xF7\xF6\x51\x92\x89\x95\x94\x06\x7A\xC5",

                '4f6fd3799489b48fa07e1a7aef0561fc',
            ],
            [
                ',H,AN',

                "\xCD\x92\xB0\x4F\xE2\x9A\xBF\x76\xC6\xD7"
                    . "\x7E\xEC\x0C\x1D\xF3\xFE\x04\x1B\x96\xA8"
                    . "\x60\x8E\xA0\xB0\x22\x2A\x47\xB6\x30\xC0"
                    . "\x7D\xFC\x54\x2E\x4D\x37\xC8\x0D\xAD\x90"
                    . "\xE3\x2C\x7D\xEE\x49\x71\xEE\x4C\x8C\x85",

                '55cd590ed76d12591c6df3b673904530',
            ],
            [
                ',H,KJ',
                "\xF4\xEB\x15\x95\x9D\x36\xBF\xE3\xEB\xEE"
                    . "\xA5\x69\xEC\xF8\x97\x3A\x31\x61\x46\xDD"
                    . "\xF0\x2B\x0B\x6F\x1B\x53\xDF\x0A\x9F\x6D"
                    . "\x8E\x94\x59\xA3\x2A\xF6\xD8\xE9\xDA\xC5"
                    . "\xD8\x81\x30\xC5\x7A\xC7\x01\xAA\x29\x46",

                '92e82c296965d97d35ab7168ece11dd0',
            ],
            [
                ',H,8B',
                "\x89\x1B\x70\x93\x89\x8A\xE6\x6A\x94\x86"
                    . "\xD6\x24\x1B\x31\xC6\x45\x28\xA0\x2F\x04"
                    . "\x67\x09\x85\x96\xCE\xFE\x5F\xCE\xAA\x88"
                    . "\x16\x35\xA2\x86\xC7\x2D\x12\xAE\x96\xA5"
                    . "\x36\x6D\xC9\x51\x9E\x90\x96\xC6\x32\xC4",
                '68799fdb9685b5e2f258245833006425',
            ],
            [
                ',H,ST',
                "\xC9\x98\xCD\x4F\x2F\x9D\x4F\x8E\x6C\xF9"
                    . "\x17\x82\x2F\xB9\x09\xF6\xE5\x1A\xA6\x7C"
                    . "\xBF\xDB\xE9\x89\x2D\x89\x1B\xC2\x50\x4C"
                    . "\x88\x1B\xE3\x57\x6A\x14\xF3\xB8\xA1\x61"
                    . "\xB3\xB8\xE2\xE2\x72\xEB\xD9\x58\x06\x81",
                '6fb328c418ea40c6c94277f420ba9357',
            ],
            [
                '',
                'w(fa`nC]e=}OY(K^ 3xN1Vz1g<F=P%!H-h*nWNL>kKnFS;&TN '
                    . '$`W~r?;9\\l?]5MF@<~oh>\\4-#hH*=w*AYaAL!]f^J&<`Tc!'
                    . 'pcpZ"Nn0RWY\\uQf8+HZXJ8?*bFGDz+Eln7Gqe6"8n[te.\\}'
                    . ':&YrQq3[UY#yU.@B}Xio>!rWoNMV]*Uw0/kb!~>WYAR0PrROK'
                    . '=?j>3B/boe@z;8,K$nM$-%]OWm KAOv^oa}#%-ets&p/?|[Dk'
                    . ')Hy.\'IfuI27y*viktmq#Tfv[X\'zUb?Bkh=zofbe1t|+~tuk'
                    . 'id]l9Edt}kpTO0w<x57h|yO.oM:oB1[-u:z/`_%_Lu4{\',9 '
                    . 'BPi?K:M;gh,+yh8p#3!ds&D@|X=$eV%((oGS*uor^{}Ye6JhJ'
                    . 'LM>Sr^PK|T2SZ:[Jb0UX!I8}Grc^>L)jzG>n\'n:%DMX g5KK'
                    . 'F!$GJ=Er0*QOVZ:R#YA+H\\0m*inr :>G;Cof`5Yq@,Avg\\J'
                    . 'j6lv_J(MUq<IrWg:s,*Zl@5_`B"X*^$utqlT<t#rg@<[w%uk1'
                    . '!G~A]^# `\\*?` 5RmiocmcL. (&&~r7 :6BwuFwW##wc#-7q'
                    . 'w(Uek#sl+zr*m*+)AN!8tyow|h\\!vssn|IUiMJVXEJc4To>v'
                    . '>?03:!+8ig9`\\-PZW\'D%Qz^wEC,z3JrQj#d&$p>nYXP6f!p'
                    . '?)5EZ1$RWH_S[+F-vIr|Nc<==tmT\\oF{x\\ASnF\'FnfyKr@'
                    . 'YijLg91$VOyD%V4KS-(tav;h>+P8VCY0.D]u^nz6?tZDLoo }'
                    . 'xt_p1I0=zIs?#%MHbD(R?>q4y7ai(ah"WIrir$\'nM{.P![yd'
                    . ' "7@@*T:A7%IxmKP7?:+CAyp>)B?e<$e><\'_F\\yhs~2ll^>'
                    . '%~X1Bz+494VYys^`2zhrBEl+9l>&Y}D_|}p@y|T32,m- Ln3H'
                    . 'W&j|sv6`6=;5bz2alS[i(o{5]*6*xIRPP>NE6d&L#Abe=tG+H'
                    . '$tQsqgQ{}\\tH0FyNt?eZE2]gtD&jl14p\'fdxO7uMskv$2pS'
                    . '(19bWLA@BeF-RXBD_*)YY@O\'5;~9NolV!\':YS[yuA@$tyF9'
                    . 'YPC3*cc2y~13N"!%$(@][{WR>xV4r|MMNp`YUDkPD|cr~ex#m'
                    . '9`J69}T2Th&R7S")4[_YG0~EtqDZkHI&*t<CrpVHZ\\zr0|{E'
                    . 'X[ !l~Faqk<]4fd?[!bNr:vMvr@(p=MJyMKfMrH?^e}sLb3)c'
                    . 'uWV0O%(CF04c]_],,EY~ny^TwR"[e+@[cwl3|uWD&l(dLfqY,'
                    . 'LXzF7P?iRzO^<,B4yV2o.Vvz^[HFM"Ry[NAr~`]R\\.1x;S.5'
                    . '*@%9v|4VX]|\'_P";~C/~%mQa{c[77iMB+R["PpY)NV1/(3K('
                    . 'W[\'\\IY?E={]Uf+wq^Ts/EM9t%J$-]P65,=rUw2{6.Zktedg'
                    . 'E:\\U`+nU09Z>w+T.8r!mk4j"CEn9+S!Qn]\'Ohu%y0`9)lm7'
                    . '%a9sMN^Oq$?,0r.ablh2U_8PoxixeX1k;K_hy>9lBXxRL\'5/'
                    . 's~BJ^Z{OSfI:?[&[\'eD!$^mG8gzen1uc08/or+@Fria2FgnM'
                    . 'N3NRr=z+%uqt\'gY8 h(rtI:g4{zZdi(3}Wfpta|zXoo`WIxX'
                    . '3"L)Kgp_cl:IVB\\UyCGo&Ej^5[?m&8F::rCfZ4F?"`hX$F/~'
                    . 'iVoo{[O17n~>(FXC9{*t1P532}l7E{7/R\' ObO`y?`9G(qjB'
                    . 'mu7 GM3ZK!qp|)!P1" sRanqC(:Ky&mh{&hDS|~ }qqzzrL,u'
                    . 'L!H/o:RwU}r[l\\XrE|FB{FAm9=i-iv#7wKFgfx`<wTxd1QWV'
                    . 'N~yKF<9_Y$lDzo[r<#[${=Atq:Y#k2Z;1UfXq!8K%&p vMs3P'
                    . 'O7MlYB^s{b`/=|@rcxde21j9#k0P`C!0[N}5p]*m@k|^h>RM8'
                    . '83KI~dMkt}L9 ]uN[,@:6/[",:jKl8c%L/OKs}7i{c#{BxK}%'
                    . 'k9<zt>(0*S}C7#oGS;<QS&N8)KZ"vY(crD_hchxm<v1Tz!{N='
                    . '9!p?P*H{dKs>TW2x8z]!sK=k]rf',
                '83747986cf0df320b915587609232076',
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
        $type = $barcode->getBarcodeObj('QRCODE,H,NL,0,1,3,1', $code);
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
