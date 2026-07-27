<?php

/**
 * index.php
 *
 * @since       2011-05-23
 * @category    Library
 * @package     Unicode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

// NOTE: run "make deps" in the project root to install the dependencies before running this example.

// autoloader when using Composer
require '../vendor/autoload.php';

use \Com\Tecnick\Unicode\Bidi as Bidi;

$bidi = new Bidi(str: 'hello ', chrarr: null, ordarr: null, forcedir: 'R', shaping: false);

echo $bidi->getString() . "\n";
