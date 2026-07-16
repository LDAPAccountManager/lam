<?php

/**
 * RecursiveReadFile.php
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Test;

class RecursiveReadFile extends \Com\Tecnick\File\File
{
    protected function hasUnreadBytes(mixed $resource): bool
    {
        return \is_resource($resource) && !\feof($resource);
    }
}
