<?php

/**
 * index.php
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

// autoloader when using Composer
require __DIR__ . '/../vendor/autoload.php';

// autoloader when using RPM or DEB package installation
//require ('/usr/share/php/Com/Tecnick/Barcode/autoload.php');

// data to generate for each barcode type
$linear = [
    'C128A' => ['0123456789', 'CODE 128 A'],
    'C128B' => ['0123456789', 'CODE 128 B'],
    'C128C' => ['0123456789', 'CODE 128 C'],
    'C128' => ['0123456789', 'CODE 128'],
    'C39E+' => ['0123456789', 'CODE 39 EXTENDED + CHECKSUM'],
    'C39E' => ['0123456789', 'CODE 39 EXTENDED'],
    'C39+' => ['0123456789', 'CODE 39 + CHECKSUM'],
    'C39' => ['0123456789', 'CODE 39 - ANSI MH10.8M-1983 - USD-3 - 3 of 9'],
    'C93' => ['0123456789', 'CODE 93 - USS-93'],
    'CODABAR' => ['0123456789', 'CODABAR'],
    'CODE11' => ['0123456789', 'CODE 11'],
    'EAN13' => ['0123456789', 'EAN 13'],
    'EAN2' => ['12', 'EAN 2-Digits UPC-Based Extension'],
    'EAN5' => ['12345', 'EAN 5-Digits UPC-Based Extension'],
    'EAN8' => ['1234567', 'EAN 8'],
    'I25+' => ['0123456789', 'Interleaved 2 of 5 + CHECKSUM'],
    'I25' => ['0123456789', 'Interleaved 2 of 5'],
    'IMB' => ['01234567094987654321-01234567891', 'IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200'],
    'IMBPRE' => ['AADTFFDFTDADTAADAATFDTDDAAADDTDTTDAFADADDDTFFFDDTTTADFAAADFTDAADA', 'IMB pre-processed'],
    'KIX' => ['0123456789', 'KIX (Klant index - Customer index)'],
    'MSI+' => ['0123456789', 'MSI + CHECKSUM (modulo 11)'],
    'MSI' => ['0123456789', 'MSI (Variation of Plessey code)'],
    'PHARMA2T' => ['0123456789', 'PHARMACODE TWO-TRACKS'],
    'PHARMA' => ['0123456789', 'PHARMACODE'],
    'PLANET' => ['0123456789', 'PLANET'],
    'POSTNET' => ['0123456789', 'POSTNET'],
    'RMS4CC' => ['0123456789', 'RMS4CC (Royal Mail 4-state Customer Bar Code)'],
    'S25+' => ['0123456789', 'Standard 2 of 5 + CHECKSUM'],
    'S25' => ['0123456789', 'Standard 2 of 5'],
    'UPCA' => ['72527273070', 'UPC-A'],
    'UPCE' => ['725277', 'UPC-E'],
];

$square = [
    'LRAW' => ['0101010101', '1D RAW MODE (comma-separated rows of 01 strings)'],
    'SRAW' => ['0101,1010', '2D RAW MODE (comma-separated rows of 01 strings)'],
    'AZTEC' => ['ABCDabcd01234', 'AZTEC (ISO/IEC 24778:2008)'],
    'AZTEC,50,A,A' => ['ABCDabcd01234', 'AZTEC (ISO/IEC 24778:2008)'],
    'PDF417' => ['0123456789', 'PDF417 (ISO/IEC 15438:2006)'],
    'QRCODE' => ['0123456789', 'QR-CODE'],
    'QRCODE,H,ST,0,0' => ['abcdefghijklmnopqrstuvwxy0123456789', 'QR-CODE WITH PARAMETERS'],
    'DATAMATRIX' => ['0123456789', 'DATAMATRIX (ISO/IEC 16022) SQUARE'],
    'DATAMATRIX,R' => [
        '0123456789012345678901234567890123456789',
        'DATAMATRIX Rectangular (ISO/IEC 16022) RECTANGULAR',
    ],
    'DATAMATRIX,S,GS1' => [
        \chr(232) . '01095011010209171719050810ABCD1234' . \chr(232) . '2110',
        'GS1 DATAMATRIX (ISO/IEC 16022) SQUARE GS1',
    ],
    'DATAMATRIX,R,GS1' => [
        \chr(232) . '01095011010209171719050810ABCD1234' . \chr(232) . '2110',
        'GS1 DATAMATRIX (ISO/IEC 16022) RECTANGULAR GS1',
    ],
];

$barcode = new \Com\Tecnick\Barcode\Barcode();

$examples = '<h3>Linear</h3>' . "\n";
foreach ($linear as $type => $code) {
    $bobj = $barcode->getBarcodeObj($type, $code[0], -3, -30, 'black', [0, 0, 0, 0]);
    $examples .=
        '<h4>[<span>'
        . $type
        . '</span>] '
        . $code[1]
        . '</h4><p style="font-family:monospace;">'
        . $bobj->getHtmlDiv()
        . '</p>'
        . "\n";
}

$examples .= '<h3>Square</h3>' . "\n";
foreach ($square as $type => $code) {
    $bobj = $barcode->getBarcodeObj($type, $code[0], -4, -4, 'black', [0, 0, 0, 0]);
    $examples .=
        '<h4>[<span>'
        . $type
        . '</span>] '
        . $code[1]
        . '</h4><p style="font-family:monospace;">'
        . $bobj->getHtmlDiv()
        . '</p>'
        . "\n";
}

$bobj = $barcode->getBarcodeObj('QRCODE,H', 'https://tecnick.com', -4, -4, 'black', [
    -2,
    -2,
    -2,
    -2,
])->setBackgroundColor('#f0f0f0');

echo
    '
<!DOCTYPE html>
<html>
    <head>
        <title>Usage example of tc-lib-barcode library</title>
        <meta charset="utf-8">
        <style>
            body {font-family:Arial, Helvetica, sans-serif;margin:30px;}
            table {border: 1px solid black;}
            th {border: 1px solid black;padding:4px;background-barcode:cornsilk;}
            td {border: 1px solid black;padding:4px;}
            h3 {color:darkblue;}
            h4 {color:darkgreen;}
            h4 span  {color:firebrick;}
        </style>
    </head>
    <body>
        <h1>Usage example of tc-lib-barcode library</h1>
        <p>This is an usage example of <a href="https://github.com/tecnickcom/tc-lib-barcode" title="tc-lib-barcode: PHP library to generate linear and bidimensional barcodes">tc-lib-barcode</a> library.</p>
        <h2>Output Formats</h2>
        <h3>PNG Image</h3>
        <p><img alt="Embedded Image" src="data:image/png;base64,'
        . \base64_encode($bobj->getPngData())
        . '" /></p>
        <h3>SVG Image</h3>
        <p style="font-family:monospace;">'
        . $bobj->getSvgCode()
        . '</p>
        <h3>HTML DIV</h3>
        <p style="font-family:monospace;">'
        . $bobj->getHtmlDiv()
        . '</p>
        <h3>Unicode String</h3>
        <pre style="font-family:monospace;line-height:0.61em;font-size:6px;">'
        . $bobj->getGrid(\json_decode('"\u00A0"'), \json_decode('"\u2584"'))
        . '</pre>
        <h3>Binary String</h3>
        <pre style="font-family:monospace;">'
        . $bobj->getGrid()
        . '</pre>
        <h2>Barcode Types</h2>
        '
        . $examples
        . '
    </body>
</html>
'
;
