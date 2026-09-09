<?php

declare(strict_types=1);

/**
 * ItfOneFour.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\ItfOneFour;
 *
 * ItfOneFour Barcode type class
 * ITF-14 (GS1 General Specifications)
 *
 * Interleaved 2 of 5 symbol encoding a 14-digit GTIN, enclosed by the quiet
 * zones and the bearer bar frame defined by the GS1 General Specifications.
 * One module is half an X-dimension, so the 2.5:1 bar width ratio and the
 * symbol proportions are expressed as whole modules.
 *
 * GS1 is a registered trademark of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class ItfOneFour extends \Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFiveCheck
{
    use FixedLengthCheckDigit;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'ITF14';

    /**
     * Fixed code length (GTIN-14)
     */
    protected const CODE_LENGTH = 14;

    /**
     * Element width in modules, keyed by the narrow (1) and wide (2) markers used by CHBAR
     *
     * @var array<int|string, int>
     */
    protected const ELEMENT = [
        '1' => 2,
        '2' => 5,
    ];

    /**
     * Quiet zone width in modules (10X)
     */
    protected const QUIET_ZONE = 20;

    /**
     * Bearer bar thickness in modules (5X, 5.08 mm at the 1.016 mm target X-dimension)
     */
    protected const BEARER = 10;

    /**
     * Bar height in modules (32.00 mm at the 1.016 mm target X-dimension)
     */
    protected const BAR_HEIGHT = 63;

    /**
     * Get the width in modules of a narrow (1) or wide (2) element
     */
    protected function getElementWidth(string $element): int
    {
        return $this::ELEMENT[$element] ?? 0;
    }

    /**
     * Check that the input code is a digit string that fits the fixed GTIN-14 length.
     * Shorter codes are left-padded with zeros by formatCode().
     *
     * @throws BarcodeException if the code is not numeric or is too long
     */
    protected function validateCode(): void
    {
        if (!\ctype_digit($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        if (\strlen($this->code) > $this::CODE_LENGTH) {
            throw new BarcodeException(
                'The code is too long: '
                . \strlen($this->code)
                . ' digits (maximum '
                . $this::CODE_LENGTH
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }
    }

    /**
     * Calculate the GTIN check digit, which Interleaved 2 of 5 names differently.
     *
     * @param string $code Data digits without the check digit
     */
    protected function getCheckDigit(string $code): int
    {
        return $this->getChecksum($code);
    }

    /**
     * Format code.
     * A code of the full length must carry a valid check digit, a shorter one is
     * left-padded with zeros to the data length and the check digit is appended.
     *
     * @throws BarcodeException if the supplied check digit is wrong
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->getCheckedCode($this->code, $this::CODE_LENGTH);
    }

    /**
     * Get the width in modules of each symbol element, alternating bars and spaces,
     * from the first bar of the start pattern to the last bar of the stop pattern.
     *
     * @return array<int, int>
     */
    protected function getElements(): array
    {
        $pairs = 'AA' . $this->extcode . 'ZA';
        $elements = [];
        $clen = \strlen($pairs);
        for ($idx = 0; $idx < $clen; $idx += 2) {
            $bar_pattern = $this->getPattern($pairs[$idx]);
            $space_pattern = $this->getPattern($pairs[$idx + 1]);
            $chrlen = \strlen($bar_pattern);
            for ($pos = 0; $pos < $chrlen; ++$pos) {
                $elements[] = $this->getElementWidth($bar_pattern[$pos]);
                $elements[] = $this->getElementWidth($space_pattern[$pos] ?? '');
            }
        }

        // the trailing space of the stop pattern is part of the quiet zone
        \array_pop($elements);

        return $elements;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->validateCode();
        $this->formatCode();

        $elements = $this->getElements();
        $this->ncols = \array_sum($elements) + (2 * ($this::QUIET_ZONE + $this::BEARER));
        $this->nrows = $this::BAR_HEIGHT + (2 * $this::BEARER);

        // bearer bar frame, butting against the top and bottom of the symbol bars
        $this->bars = [
            [0, 0, $this->ncols, $this::BEARER],
            [0, $this->nrows - $this::BEARER, $this->ncols, $this::BEARER],
            [0, 0, $this::BEARER, $this->nrows],
            [$this->ncols - $this::BEARER, 0, $this::BEARER, $this->nrows],
        ];

        $posx = $this::BEARER + $this::QUIET_ZONE;
        foreach ($elements as $idx => $width) {
            if (($idx % 2) === 0) {
                $this->bars[] = [$posx, $this::BEARER, $width, $this::BAR_HEIGHT];
            }

            $posx += $width;
        }
    }
}
