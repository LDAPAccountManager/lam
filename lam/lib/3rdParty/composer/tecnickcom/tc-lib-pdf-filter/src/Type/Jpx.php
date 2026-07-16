<?php

declare(strict_types=1);

/**
 * Jpx.php
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

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Type\Jpx
 *
 * JPXDecode filter (PDF 32000-2008 §7.4.10).
 * Decompresses JPEG 2000 (JP2/JPX) encoded image data using the Imagick
 * extension (ext-imagick). If Imagick is not available a PPException is thrown.
 *
 * Suggested PHP extension: ext-imagick
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Jpx implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data.
     *
     * Requires the Imagick PHP extension.
     *
     * @param string              $data   Data to decode.
     * @param array<string, mixed> $params Optional filter parameters.
     *
     * @return string Decoded data string.
     *
     * @throws PPException if the Imagick extension is not loaded or decoding fails.
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        if (!extension_loaded('imagick')) {
            throw new PPException('JPXDecode requires the Imagick PHP extension (ext-imagick)');
        }

        try {
            $imagick = $this->newImagick();
            $imagick->readImageBlob($data);
            $imagick->setImageFormat('png');

            return $imagick->getImageBlob();
        } catch (\ImagickException $e) {
            throw new PPException('JPXDecode: Imagick failed to decode the stream: ' . $e->getMessage());
        }
    }

    /**
     * Instantiate Imagick (overridable in tests).
     */
    protected function newImagick(): \Imagick
    {
        return new \Imagick();
    }
}
