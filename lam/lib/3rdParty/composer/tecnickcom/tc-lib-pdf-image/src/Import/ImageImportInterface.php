<?php

declare(strict_types=1);

/**
 * ImageImportInterface.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    jmleroux <jmleroux.pro@gmail.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Com\Tecnick\Pdf\Image\Import;

/**
 * Com\Tecnick\Pdf\Image\Import\Jpeg
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    jmleroux <jmleroux.pro@gmail.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 */
interface ImageImportInterface
{
    /**
     * Extract data from an image.
     *
     * @param ImageBaseData $data Image raw data.
     *
     * @return ImageBaseData Image raw data array.
     */
    public function getData(array $data): array;
}
