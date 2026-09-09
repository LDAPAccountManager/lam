<?php

declare(strict_types=1);

/**
 * PostCode.php
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

namespace Com\Tecnick\Barcode\Type\Linear\Mailmark;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\Mailmark\PostCode
 *
 * Destination Post Code plus DPS field of a 4-state Mailmark barcode
 *
 * The field is either the fixed international string or one of six patterns of
 * character types. Each pattern has its own block of values, and the blocks
 * follow one another in the order the patterns are listed, starting at one.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class PostCode
{
    /**
     * Number of characters of the field
     */
    public const LENGTH = 9;

    /**
     * Field value that denotes an international destination
     *
     * @var string
     */
    public const INTERNATIONAL = 'XY11     ';

    /**
     * Allowed character values of each character type
     *
     * @var array<string, string>
     */
    protected const CHARACTER_TYPE = [
        'F' => 'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
        'L' => 'ABDEFGHJLNPQRSTUWXYZ',
        'N' => '0123456789',
        'S' => '',
    ];

    /**
     * Allowed patterns of character types, in the order of their value blocks
     *
     * @var array<int, string>
     */
    protected const PATTERN = [
        'FNFNLLNLS',
        'FFNNLLNLS',
        'FFNNNLLNL',
        'FFNFNLLNL',
        'FNNLLNLSS',
        'FNNNLLNLS',
    ];

    /**
     * Get the number of values a pattern can take
     */
    protected function getPatternSize(string $pattern): int
    {
        $size = 1;
        for ($pos = 0; $pos < \strlen($pattern); ++$pos) {
            $size *= \max(1, \strlen($this::CHARACTER_TYPE[$pattern[$pos]] ?? ''));
        }

        return $size;
    }

    /**
     * Get the pattern of character types the field matches, or an empty string
     */
    protected function getPattern(string $code): string
    {
        foreach ($this::PATTERN as $pattern) {
            $match = true;
            for ($pos = 0; $pos < $this::LENGTH; ++$pos) {
                $set = $this::CHARACTER_TYPE[$pattern[$pos]] ?? '';
                $char = $code[$pos] ?? '';
                $ok = $pattern[$pos] === 'S' ? $char === ' ' : \str_contains($set, $char) && $char !== '';
                if (!$ok) {
                    $match = false;
                    break;
                }
            }

            if ($match) {
                return $pattern;
            }
        }

        return '';
    }

    /**
     * Get the offset of the value block of a pattern
     */
    protected function getPatternOffset(string $pattern): int
    {
        $offset = 1;
        foreach ($this::PATTERN as $candidate) {
            if ($candidate === $pattern) {
                return $offset;
            }

            $offset += $this->getPatternSize($candidate);
        }

        return $offset;
    }

    /**
     * Get the internal value of the field
     *
     * @throws BarcodeException if the field matches no pattern
     */
    public function getValue(string $code): int
    {
        if ($code === $this::INTERNATIONAL) {
            return 0;
        }

        $pattern = $this->getPattern($code);
        if ($pattern === '') {
            throw new BarcodeException('Invalid destination post code plus DPS: ' . $code);
        }

        $value = 0;
        for ($pos = 0; $pos < $this::LENGTH; ++$pos) {
            $set = $this::CHARACTER_TYPE[$pattern[$pos]] ?? '';
            if ($set === '') {
                continue;
            }

            $value = ($value * \strlen($set)) + (int) \strpos($set, $code[$pos] ?? '');
        }

        return $this->getPatternOffset($pattern) + $value;
    }
}
