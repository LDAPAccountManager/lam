<?php

/**
 * index.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     PdfParser
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

// autoloader when using Composer
require __DIR__ . '/../vendor/autoload.php';

// autoloader when using RPM or DEB package installation
//require ('/usr/share/php/Com/Tecnick/Pdf/Parser/autoload.php');

$filename = '../resources/test/example_036.pdf';
$rawdata = \file_get_contents($filename);
if ($rawdata === false) {
    \die('Unable to get the content of the file: ' . $filename);
}

// configuration parameters for parser
$cfg = [
    'ignore_filter_errors' => true,
];

// parse PDF data
$pdf = new \Com\Tecnick\Pdf\Parser\Parser($cfg);
$data = $pdf->parse($rawdata);

// display data
\var_dump($data);
