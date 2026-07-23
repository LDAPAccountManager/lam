<?php

declare(strict_types=1);

/**
 * SpecRs.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\SpecRs
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @SuppressWarnings("PHPMD.TooManyPublicMethods")
 */
abstract class SpecRs
{
    /**
     * @param array<int, int> $spec
     */
    protected function getSpecValue(array $spec, int $index): int
    {
        return match ($index) {
            0 => $spec[0] ?? 0,
            1 => $spec[1] ?? 0,
            2 => $spec[2] ?? 0,
            3 => $spec[3] ?? 0,
            4 => $spec[4] ?? 0,
            default => 0,
        };
    }

    protected function getCapacityWidth(int $version): int
    {
        $capacity = Data::CAPACITY[$version] ?? [0, 0, 0, [0, 0, 0, 0]];

        return $capacity[0] ?? 0;
    }

    /**
     * @param array<int, string> $frame
     */
    protected function getFrameRow(array $frame, int $index): string
    {
        return $frame[$index] ?? '';
    }

    /**
     * @param array<int, string> $frame
     */
    protected function replaceFrameRow(
        array &$frame,
        int $index,
        string $replacement,
        int $offset,
        ?int $length = null,
    ): void {
        $replaceLength = $length === null ? null : \max(0, $length);
        $frame[$index] = \substr_replace($this->getFrameRow($frame, $index), $replacement, $offset, $replaceLength);
    }

    protected function getVersionPatternValue(int $version): int
    {
        return Data::VERSION_PATTERN[$version - 7] ?? 0;
    }

    protected function getAlignmentStart(int $version): int
    {
        $pattern = Data::ALIGN_PATTERN[$version] ?? [0, 0];

        return $pattern[0] ?? 0;
    }

    protected function getAlignmentEnd(int $version): int
    {
        $pattern = Data::ALIGN_PATTERN[$version] ?? [0, 0];

        return $pattern[1] ?? 0;
    }

    /**
     * Return block number 0
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsBlockNum(array $spec): int
    {
        return $this->getSpecValue($spec, 0) + $this->getSpecValue($spec, 3);
    }

    /**
     * Return block number 1
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsBlockNum1(array $spec): int
    {
        return $this->getSpecValue($spec, 0);
    }

    /**
     * Return data codes 1
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsDataCodes1(array $spec): int
    {
        return $this->getSpecValue($spec, 1);
    }

    /**
     * Return ecc codes 1
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsEccCodes1(array $spec): int
    {
        return $this->getSpecValue($spec, 2);
    }

    /**
     * Return block number 2
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsBlockNum2(array $spec): int
    {
        return $this->getSpecValue($spec, 3);
    }

    /**
     * Return data codes 2
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsDataCodes2(array $spec): int
    {
        return $this->getSpecValue($spec, 4);
    }

    /**
     * Return ecc codes 2
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsEccCodes2(array $spec): int
    {
        return $this->getSpecValue($spec, 2);
    }

    /**
     * Return data length
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsDataLength(array $spec): int
    {
        return (
            ($this->getSpecValue($spec, 0) * $this->getSpecValue($spec, 1))
            + ($this->getSpecValue($spec, 3) * $this->getSpecValue($spec, 4))
        );
    }

    /**
     * Return ecc length
     *
     * @param array<int, int> $spec Spec
     *
     * @return int value
     */
    public function rsEccLength(array $spec): int
    {
        return ($this->getSpecValue($spec, 0) + $this->getSpecValue($spec, 3)) * $this->getSpecValue($spec, 2);
    }

    /**
     * Return a copy of initialized frame.
     *
     * @param int $version Version
     *
     * @return array<int, string> of unsigned char.
     *
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     */
    public function createFrame(int $version): array
    {
        $width = \max(0, $this->getCapacityWidth($version));
        $frameLine = \str_repeat("\0", $width);
        $frame = \array_fill(0, $width, $frameLine);
        // Finder pattern
        $frame = $this->putFinderPattern($frame, 0, 0);
        $frame = $this->putFinderPattern($frame, $width - 7, 0);
        $frame = $this->putFinderPattern($frame, 0, $width - 7);
        // Separator
        $yOffset = $width - 7;
        for ($ypos = 0; $ypos < 7; ++$ypos) {
            $this->replaceFrameRow($frame, $ypos, "\xc0", 7, 1);
            $this->replaceFrameRow($frame, $ypos, "\xc0", $width - 8, 1);
            $this->replaceFrameRow($frame, $yOffset, "\xc0", 7, 1);
            ++$yOffset;
        }

        $setPattern = \str_repeat("\xc0", 8);
        $frame = $this->qrstrset($frame, 0, 7, $setPattern);
        $frame = $this->qrstrset($frame, $width - 8, 7, $setPattern);
        $frame = $this->qrstrset($frame, 0, $width - 8, $setPattern);
        // Format info
        $setPattern = \str_repeat("\x84", 9);
        $frame = $this->qrstrset($frame, 0, 8, $setPattern);
        $frame = $this->qrstrset($frame, $width - 8, 8, $setPattern, 8);

        $yOffset = $width - 8;
        for ($ypos = 0; $ypos < 8; ++$ypos, ++$yOffset) {
            $this->replaceFrameRow($frame, $ypos, "\x84", 8, 1);
            $this->replaceFrameRow($frame, $yOffset, "\x84", 8, 1);
        }

        // Timing pattern
        $wdo = $width - 15;
        for ($idx = 1; $idx < $wdo; ++$idx) {
            $this->replaceFrameRow($frame, 6, \chr((0x90 | ($idx & 1)) & 0xFF), 7 + $idx, 1);
            $this->replaceFrameRow($frame, 7 + $idx, \chr((0x90 | ($idx & 1)) & 0xFF), 6, 1);
        }

        // Alignment pattern
        $frame = $this->putAlignmentPattern($version, $frame, $width);
        // Version information
        if ($version >= 7) {
            $vinf = $this->getVersionPattern($version);
            $val = $vinf;
            for ($xpos = 0; $xpos < 6; ++$xpos) {
                for ($ypos = 0; $ypos < 3; ++$ypos) {
                    $this->replaceFrameRow($frame, $width - 11 + $ypos, \chr((0x88 | ($val & 1)) & 0xFF), $xpos, 1);
                    $val >>= 1;
                }
            }

            $val = $vinf;
            for ($ypos = 0; $ypos < 6; ++$ypos) {
                for ($xpos = 0; $xpos < 3; ++$xpos) {
                    $this->replaceFrameRow($frame, $ypos, \chr(0x88 | ($val & 1 & 0xFF)), $xpos + ($width - 11), 1);
                    $val >>= 1;
                }
            }
        }

        // and a little bit...
        $this->replaceFrameRow($frame, $width - 8, "\x81", 8, 1);
        return $frame;
    }

    /**
     * Replace a value on the array at the specified position
     *
     * @param array<int, string>  $srctab     Source table
     * @param int    $xpos       X position
     * @param int    $ypos       Y position
     * @param string $repl    Value to replace
     * @param int|null    $replLen Length of the repl string
     *
     * @return array<int, string> srctab
     */
    public function qrstrset(array $srctab, int $xpos, int $ypos, string $repl, ?int $replLen = null): array
    {
        $replaceLength = \max(0, $replLen ?? \strlen($repl));
        $srctab[$ypos] = \substr_replace(
            $this->getFrameRow($srctab, $ypos),
            $replLen !== null ? \substr($repl, 0, $replLen) : $repl,
            $xpos,
            $replaceLength,
        );

        return $srctab;
    }

    /**
     * Put an alignment marker.
     *
     * @param array<int, string> $frame Frame
     * @param int   $pox   X center coordinate of the pattern
     * @param int   $poy   Y center coordinate of the pattern
     *
     * @return array<int, string> frame
     */
    public function putAlignmentMarker(array $frame, int $pox, int $poy): array
    {
        $finder = [
            "\xa1\xa1\xa1\xa1\xa1",
            "\xa1\xa0\xa0\xa0\xa1",
            "\xa1\xa0\xa1\xa0\xa1",
            "\xa1\xa0\xa0\xa0\xa1",
            "\xa1\xa1\xa1\xa1\xa1",
        ];
        $yStart = $poy - 2;
        $xStart = $pox - 2;
        for ($ydx = 0; $ydx < 5; ++$ydx) {
            $frame = $this->qrstrset($frame, $xStart, $yStart + $ydx, $finder[$ydx] ?? '');
        }

        return $frame;
    }

    /**
     * Put a finder pattern.
     *
     * @param array<int, string> $frame Frame
     * @param int   $pox   X center coordinate of the pattern
     * @param int   $poy   Y center coordinate of the pattern
     *
     * @return array<int, string> frame
     */
    public function putFinderPattern(array $frame, int $pox, int $poy): array
    {
        $finder = [
            "\xc1\xc1\xc1\xc1\xc1\xc1\xc1",
            "\xc1\xc0\xc0\xc0\xc0\xc0\xc1",
            "\xc1\xc0\xc1\xc1\xc1\xc0\xc1",
            "\xc1\xc0\xc1\xc1\xc1\xc0\xc1",
            "\xc1\xc0\xc1\xc1\xc1\xc0\xc1",
            "\xc1\xc0\xc0\xc0\xc0\xc0\xc1",
            "\xc1\xc1\xc1\xc1\xc1\xc1\xc1",
        ];
        for ($ypos = 0; $ypos < 7; ++$ypos) {
            $frame = $this->qrstrset($frame, $pox, $poy + $ypos, $finder[$ypos] ?? '');
        }

        return $frame;
    }

    /**
     * Return BCH encoded version information pattern that is used for the symbol of version 7 or greater.
     * Use lower 18 bits.
     *
     * @param int $version Version
     */
    public function getVersionPattern(int $version): int
    {
        if ($version < 7 || $version > Data::QRSPEC_VERSION_MAX) {
            return 0;
        }

        return $this->getVersionPatternValue($version);
    }

    /**
     * Put an alignment pattern.
     *
     * @param int   $version Version
     * @param array<int, string> $frame   Frame
     * @param int   $width   Width
     *
     * @return array<int, string> frame
     */
    public function putAlignmentPattern(int $version, array $frame, int $width): array
    {
        if ($version < 2) {
            return $frame;
        }

        $alignStart = $this->getAlignmentStart($version);
        $alignEnd = $this->getAlignmentEnd($version);
        $dval = $alignEnd - $alignStart;
        $wdt = 2;
        if ($dval >= 0) {
            $wdt = (int) ((($width - $alignStart) / $dval) + 2);
        }

        if ((($wdt * $wdt) - 3) === 1) {
            $psx = $alignStart;
            $psy = $alignStart;
            return $this->putAlignmentMarker($frame, $psx, $psy);
        }

        $cpx = $alignStart;
        $wdo = $wdt - 1;
        for ($xpos = 1; $xpos < $wdo; ++$xpos) {
            $frame = $this->putAlignmentMarker($frame, 6, $cpx);
            $frame = $this->putAlignmentMarker($frame, $cpx, 6);
            $cpx += $dval;
        }

        $cpy = $alignStart;
        for ($y = 0; $y < $wdo; ++$y) {
            $cpx = $alignStart;
            for ($xpos = 0; $xpos < $wdo; ++$xpos) {
                $frame = $this->putAlignmentMarker($frame, $cpx, $cpy);
                $cpx += $dval;
            }

            $cpy += $dval;
        }

        return $frame;
    }
}
