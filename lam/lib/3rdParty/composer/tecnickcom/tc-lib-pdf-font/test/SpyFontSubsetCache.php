<?php

/**
 * SpyFontSubsetCache.php
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

namespace Test;

use Com\Tecnick\Pdf\Font\FontSubsetCacheInterface;

/**
 * In-memory FontSubsetCacheInterface implementation that records access for
 * assertions in the test-suite.
 */
class SpyFontSubsetCache implements FontSubsetCacheInterface
{
    /**
     * Stored subset font programs keyed by cache key.
     *
     * @var array<string, string>
     */
    public array $store = [];

    /**
     * Keys requested via get(), in call order.
     *
     * @var array<int, string>
     */
    public array $getCalls = [];

    /**
     * Keys written via set(), in call order.
     *
     * @var array<int, string>
     */
    public array $setCalls = [];

    public function get(string $key): ?string
    {
        $this->getCalls[] = $key;

        return $this->store[$key] ?? null;
    }

    public function set(string $key, string $subsetFont): void
    {
        $this->setCalls[] = $key;
        $this->store[$key] = $subsetFont;
    }
}
