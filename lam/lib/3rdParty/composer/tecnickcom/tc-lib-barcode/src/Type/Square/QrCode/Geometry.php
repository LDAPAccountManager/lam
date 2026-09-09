<?php

declare(strict_types=1);

/**
 * Geometry.php
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Geometry
 *
 * Function patterns of the QrCode Barcode type class, section 7.3 of
 * ISO/IEC 18004: the finder patterns and their separators, the timing patterns,
 * the alignment patterns, and the modules reserved for the format and the
 * version information.
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Geometry
{
    /**
     * Symbol size in modules.
     */
    protected int $size;

    /**
     * Row and column coordinates of the centre module of the alignment
     * patterns of the version.
     *
     * @var array<int, array{int, int}>
     */
    protected array $alignments = [];

    /**
     * @param int $version Symbol version, from 1 to 40.
     */
    public function __construct(
        protected int $version,
    ) {
        $this->size = (4 * $version) + 17;
        $this->alignments = $this->getAlignmentCentres();
    }

    /**
     * Returns the symbol size in modules.
     */
    public function getSize(): int
    {
        return $this->size;
    }

    /**
     * Returns the centre of every alignment pattern of the version: every pair
     * of the coordinates of Table E.1 of ISO/IEC 18004, less the three pairs the
     * finder patterns cover.
     *
     * @return array<int, array{int, int}>
     */
    public function getAlignmentCentres(): array
    {
        $centres = Data::ALIGN_CENTRES[$this->version] ?? [];
        $last = $this->size - 7;
        $result = [];
        foreach ($centres as $row) {
            foreach ($centres as $col) {
                if ($row === 6 && $col === 6 || $row === 6 && $col === $last || $row === $last && $col === 6) {
                    continue;
                }

                $result[] = [$row, $col];
            }
        }

        return $result;
    }

    /**
     * Returns whether the module belongs to a function pattern, to the format
     * information or to the version information, and so cannot carry data.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    public function isFunctionModule(int $row, int $col): bool
    {
        if ($row === 6 || $col === 6) {
            return true;
        }

        // the finder patterns, their separators and the format information take
        // the whole of each of the three corners
        if ($row <= 8 && ($col <= 8 || $col >= ($this->size - 8))) {
            return true;
        }

        if ($col <= 8 && $row >= ($this->size - 8)) {
            return true;
        }

        if ($this->isVersionInfoModule($row, $col)) {
            return true;
        }

        return $this->isAlignmentModule($row, $col);
    }

    /**
     * Returns whether the module is a dark module of a function pattern. Every
     * module of the separators, of the format information and of the version
     * information is light at this stage.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    public function isDarkModule(int $row, int $col): bool
    {
        $finder = $this->getFinderValue($row, $col);
        if ($finder >= 0) {
            return $finder === 1;
        }

        if ($this->isAlignmentModule($row, $col)) {
            return $this->isAlignmentDark($row, $col);
        }

        if ($row === 6) {
            return ($col % 2) === 0;
        }

        if ($col === 6) {
            return ($row % 2) === 0;
        }

        // the module below the format information of the bottom left corner is
        // always dark, section 7.9.1
        return $row === ($this->size - 8) && $col === 8;
    }

    /**
     * Returns 1 for a dark module of a finder pattern, 0 for a light module of a
     * finder pattern or of its separator, and a negative value for a module
     * outside all three of them.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    protected function getFinderValue(int $row, int $col): int
    {
        foreach ([[0, 0], [0, $this->size - 7], [$this->size - 7, 0]] as $origin) {
            $dry = $row - $origin[0];
            $dcx = $col - $origin[1];
            if ($dry < -1 || $dry > 7 || $dcx < -1 || $dcx > 7) {
                continue;
            }

            if ($dry < 0 || $dry > 6 || $dcx < 0 || $dcx > 6) {
                return 0;
            }

            $ring = \max(\abs($dry - 3), \abs($dcx - 3));

            return $ring === 3 || $ring <= 1 ? 1 : 0;
        }

        return -1;
    }

    /**
     * Returns whether the module belongs to an alignment pattern.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    protected function isAlignmentModule(int $row, int $col): bool
    {
        foreach ($this->alignments as $centre) {
            if (\abs($row - $centre[0]) <= 2 && \abs($col - $centre[1]) <= 2) {
                return true;
            }
        }

        return false;
    }

    /**
     * Returns whether the module of an alignment pattern is dark: the outer ring
     * and the centre module are dark, the ring between them is light.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    protected function isAlignmentDark(int $row, int $col): bool
    {
        foreach ($this->alignments as $centre) {
            $ring = \max(\abs($row - $centre[0]), \abs($col - $centre[1]));
            if ($ring > 2) {
                continue;
            }

            return $ring !== 1;
        }

        return false;
    }

    /**
     * Returns whether the module is reserved for the version information, that
     * is the block of six by three modules beside each of the top right and the
     * bottom left finder patterns of a symbol of version 7 or above.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    protected function isVersionInfoModule(int $row, int $col): bool
    {
        if ($this->version < Data::VERSION_INFO_MIN) {
            return false;
        }

        $first = $this->size - 11;
        if ($row <= 5 && $col >= $first && $col <= ($first + 2)) {
            return true;
        }

        return $col <= 5 && $row >= $first && $row <= ($first + 2);
    }
}
