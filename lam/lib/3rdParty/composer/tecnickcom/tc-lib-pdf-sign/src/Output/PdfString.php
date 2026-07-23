<?php

declare(strict_types=1);

/**
 * PdfString.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Output;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Output\PdfString
 *
 * Encodes a text value as a PDF string token, either through a host-supplied
 * encoder (which may apply UTF-16, escaping, and encryption) or, when none is
 * given, a minimal literal-string fallback for ASCII content.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class PdfString
{
    /**
     * @param callable|null $encoder fn(string $text, int $objectId): string
     *
     * @throws Exception If the encoder returns a non-string value.
     */
    public static function encode(string $text, int $objectId, ?callable $encoder = null): string
    {
        if ($encoder === null) {
            return '(' . \strtr($text, ['\\' => '\\\\', '(' => '\\(', ')' => '\\)']) . ')';
        }

        /** @var mixed $result */
        $result = $encoder($text, $objectId);
        if (!\is_string($result)) {
            throw new Exception('Invalid string encoder result');
        }

        return $result;
    }
}
