<?php

declare(strict_types=1);

/**
 * FontSubsetCacheInterface.php
 *
 * @since     2026-06-16
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

/**
 * Com\Tecnick\Pdf\Font\FontSubsetCacheInterface
 *
 * Optional cache contract for reusing TrueType font subset programs.
 *
 * Implementations are injected into Output and consulted before the
 * (computational and memory intensive) subsetting is performed. The cached
 * value is the raw subset font program string, i.e. the output of
 * Subset::getSubsetFont() before any compression or encryption.
 *
 * The library never evicts entries; backends own their own expiration and
 * size limits.
 *
 * @since     2026-06-16
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
interface FontSubsetCacheInterface
{
    /**
     * Return the cached subset font program for the given key, or null on miss.
     *
     * @param string $key Cache key identifying the font subset.
     */
    public function get(string $key): ?string;

    /**
     * Store the subset font program for the given key.
     *
     * @param string $key        Cache key identifying the font subset.
     * @param string $subsetFont Raw subset font program (uncompressed).
     */
    public function set(string $key, string $subsetFont): void;
}
