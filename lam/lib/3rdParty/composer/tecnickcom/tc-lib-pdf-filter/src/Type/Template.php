<?php

declare(strict_types=1);

/**
 * Template.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

/**
 * Com\Tecnick\Pdf\Filter\Type\Template.php
 *
 * Template.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
interface Template
{
    /**
     * Decode the data.
     *
     * @param string              $data   Data to decode.
     * @param array<string, mixed> $params Optional filter parameters (e.g., DecodeParms from PDF).
     *
     * @return string Decoded data string.
     */
    public function decode(string $data, array $params = []): string;
}
