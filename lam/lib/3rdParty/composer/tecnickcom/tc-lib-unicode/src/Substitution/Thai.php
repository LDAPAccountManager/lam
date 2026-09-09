<?php

declare(strict_types=1);

/**
 * Thai.php
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode\Substitution;

/**
 * Com\Tecnick\Unicode\Substitution\Thai
 *
 * Thai codepoints are returned unchanged.
 *
 * Thai preposed vowels (sara E, sara AE, sara O, sara AI) are stored before the
 * consonant they follow in pronunciation and are displayed in that same position, so a
 * left-to-right glyph stream needs no reordering.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Thai
{
    /**
     * Transformed codepoint array.
     *
     * @var list<int>
     */
    private array $ordarr;

    /**
     * @param array<int, int> $ordarr Array of Unicode codepoints.
     */
    public function __construct(array $ordarr)
    {
        $this->ordarr = array_values($ordarr);
    }

    /**
     * Returns the codepoint array.
     *
     * @return list<int>
     */
    public function getOrdarr(): array
    {
        return $this->ordarr;
    }
}
