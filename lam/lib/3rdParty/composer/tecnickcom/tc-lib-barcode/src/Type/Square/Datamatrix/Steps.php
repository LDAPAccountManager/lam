<?php

declare(strict_types=1);

/**
 * Steps.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix\Steps
 *
 * Steps methods for Datamatrix Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Steps extends \Com\Tecnick\Barcode\Type\Square\Datamatrix\Modes
{
    /**
     * The look-ahead test scans the data to be encoded to find the best mode (Annex P - steps from J to S).
     *
     * @param string $data Data to encode
     * @param int    $pos  Current position
     * @param int    $mode Current encoding mode
     *
     * @return int encoding mode
     */
    public function lookAheadTest(string $data, int $pos, int $mode): int
    {
        $data_length = \strlen($data);
        if ($pos >= $data_length) {
            return $mode;
        }

        $charscount = 0; // count processed chars
        // STEP J
        $numch = match ($mode) {
            Data::ENC_C40 => [1.0, 0.0, 2.0, 2.0, 2.0, 2.25],
            Data::ENC_TXT => [1.0, 2.0, 0.0, 2.0, 2.0, 2.25],
            Data::ENC_X12 => [1.0, 2.0, 2.0, 0.0, 2.0, 2.25],
            Data::ENC_EDF => [1.0, 2.0, 2.0, 2.0, 0.0, 2.25],
            Data::ENC_BASE256 => [1.0, 2.0, 2.0, 2.0, 2.0, 0.0],
            default => [0.0, 1.0, 1.0, 1.0, 1.0, 1.25],
        };

        while (true) {
            if (($pos + $charscount) === $data_length) {
                return $this->stepK($numch);
            }

            $chr = \ord($data[$pos + $charscount]);
            ++$charscount;
            $this->stepL($chr, $numch);
            $this->stepM($chr, $numch);
            $this->stepN($chr, $numch);
            $this->stepO($chr, $numch);
            $this->stepP($chr, $numch);
            $this->stepQ($chr, $numch);
            if ($charscount >= 4) {
                $ret = $this->stepR($numch, $pos, $data_length, $charscount, $data);
                if ($ret >= 0) {
                    return $ret;
                }
            }
        }
    }

    /**
     * Step K
     *
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     *
     * @return int encoding mode
     */
    protected function stepK(array $numch): int
    {
        if (
            ($numch[Data::ENC_ASCII] ?? 0.0) <= \ceil(\min(
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            ))
        ) {
            return Data::ENC_ASCII;
        }

        if (
            ($numch[Data::ENC_BASE256] ?? 0.0) < \ceil(\min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
            ))
        ) {
            return Data::ENC_BASE256;
        }

        if (
            ($numch[Data::ENC_EDF] ?? 0.0) < \ceil(\min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            ))
        ) {
            return Data::ENC_EDF;
        }

        if (
            ($numch[Data::ENC_TXT] ?? 0.0) < \ceil(\min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            ))
        ) {
            return Data::ENC_TXT;
        }

        if (
            ($numch[Data::ENC_X12] ?? 0.0) < \ceil(\min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            ))
        ) {
            return Data::ENC_X12;
        }

        return Data::ENC_C40;
    }

    /**
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch
     */
    protected function getNumch(array $numch, int $mode): float
    {
        return match ($mode) {
            Data::ENC_ASCII => $numch[0],
            Data::ENC_C40 => $numch[1],
            Data::ENC_TXT => $numch[2],
            Data::ENC_X12 => $numch[3],
            Data::ENC_EDF => $numch[4],
            Data::ENC_BASE256 => $numch[5],
            default => 0.0,
        };
    }

    /**
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch
     */
    protected function setNumch(array &$numch, int $mode, float $value): void
    {
        switch ($mode) {
            case Data::ENC_ASCII:
                $numch[0] = $value;
                return;

            case Data::ENC_C40:
                $numch[1] = $value;
                return;

            case Data::ENC_TXT:
                $numch[2] = $value;
                return;

            case Data::ENC_X12:
                $numch[3] = $value;
                return;

            case Data::ENC_EDF:
                $numch[4] = $value;
                return;

            case Data::ENC_BASE256:
                $numch[5] = $value;
                return;
        }
    }

    /**
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch
     */
    protected function addNumch(array &$numch, int $mode, float $value): void
    {
        $this->setNumch($numch, $mode, $this->getNumch($numch, $mode) + $value);
    }

    /**
     * Step L
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepL(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_ASCII_NUM)) {
            $this->addNumch($numch, Data::ENC_ASCII, 0.5);
            return;
        }

        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            $this->setNumch($numch, Data::ENC_ASCII, \ceil($this->getNumch($numch, Data::ENC_ASCII)));
            $this->addNumch($numch, Data::ENC_ASCII, 2.0);
            return;
        }

        $this->setNumch($numch, Data::ENC_ASCII, \ceil($this->getNumch($numch, Data::ENC_ASCII)));
        $this->addNumch($numch, Data::ENC_ASCII, 1.0);
    }

    /**
     * Step M
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepM(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_C40)) {
            $this->addNumch($numch, Data::ENC_C40, 2.0 / 3.0);
            return;
        }

        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            $this->addNumch($numch, Data::ENC_C40, 8.0 / 3.0);
            return;
        }

        $this->addNumch($numch, Data::ENC_C40, 4.0 / 3.0);
    }

    /**
     * Step N
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepN(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_TXT)) {
            $this->addNumch($numch, Data::ENC_TXT, 2.0 / 3.0);
            return;
        }

        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            $this->addNumch($numch, Data::ENC_TXT, 8.0 / 3.0);
            return;
        }

        $this->addNumch($numch, Data::ENC_TXT, 4.0 / 3.0);
    }

    /**
     * Step O
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepO(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_X12) || $this->isCharMode($chr, Data::ENC_C40)) {
            $this->addNumch($numch, Data::ENC_X12, 2.0 / 3.0);
            return;
        }

        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            $this->addNumch($numch, Data::ENC_X12, 13.0 / 3.0);
            return;
        }

        $this->addNumch($numch, Data::ENC_X12, 10.0 / 3.0);
    }

    /**
     * Step P
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepP(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_EDF)) {
            $this->addNumch($numch, Data::ENC_EDF, 3.0 / 4.0);
            return;
        }

        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            $this->addNumch($numch, Data::ENC_EDF, 17.0 / 4.0);
            return;
        }

        $this->addNumch($numch, Data::ENC_EDF, 13.0 / 4.0);
    }

    /**
     * Step Q
     *
     * @param int $chr    Character code
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     */
    protected function stepQ(int $chr, array &$numch): void
    {
        if ($this->isCharMode($chr, Data::ENC_BASE256)) {
            $this->addNumch($numch, Data::ENC_BASE256, 4.0);
            return;
        }

        $this->addNumch($numch, Data::ENC_BASE256, 1.0);
    }

    /**
     * Step R-f
     *
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     * @param int    $pos  Current position
     * @param int    $data_length  Data length
     * @param int    $charscount   Number of processed characters
     * @param string $data Data to encode
     *
     * @return int   Encoding mode
     */
    protected function stepRf(array $numch, int $pos, int $data_length, int $charscount, string $data): int
    {
        if (
            (($numch[Data::ENC_C40] ?? 0.0) + 1) < \min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            )
        ) {
            if (($numch[Data::ENC_C40] ?? 0.0) < ($numch[Data::ENC_X12] ?? 0.0)) {
                return Data::ENC_C40;
            }

            if (($numch[Data::ENC_C40] ?? 0.0) === ($numch[Data::ENC_X12] ?? 0.0)) {
                $ker = $pos + $charscount + 1;
                while ($ker < $data_length) {
                    $tmpchr = \ord($data[$ker]);
                    if ($this->isCharMode($tmpchr, Data::ENC_X12)) {
                        return Data::ENC_X12;
                    }

                    if ($this->isCharMode($tmpchr, Data::ENC_C40)) {
                        break;
                    }

                    ++$ker;
                }

                return Data::ENC_C40;
            }
        }

        return -1;
    }

    /**
     * Step R
     *
     * @param array{0: float, 1: float, 2: float, 3: float, 4: float, 5: float} $numch Number of characters
     * @param int    $pos  Current position
     * @param int    $data_length  Data length
     * @param int    $charscount   Number of processed characters
     * @param string $data Data to encode
     *
     * @return int   Encoding mode
     */
    protected function stepR(array $numch, int $pos, int $data_length, int $charscount, string $data): int
    {
        if (
            (($numch[Data::ENC_ASCII] ?? 0.0) + 1) <= \min(
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            )
        ) {
            return Data::ENC_ASCII;
        }

        if (
            (($numch[Data::ENC_BASE256] ?? 0.0) + 1) <= ($numch[Data::ENC_ASCII] ?? 0.0)
            || (($numch[Data::ENC_BASE256] ?? 0.0) + 1) < \min(
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
            )
        ) {
            return Data::ENC_BASE256;
        }

        if (
            (($numch[Data::ENC_EDF] ?? 0.0) + 1) < \min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            )
        ) {
            return Data::ENC_EDF;
        }

        if (
            (($numch[Data::ENC_TXT] ?? 0.0) + 1) < \min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_X12] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            )
        ) {
            return Data::ENC_TXT;
        }

        if (
            (($numch[Data::ENC_X12] ?? 0.0) + 1) < \min(
                $numch[Data::ENC_ASCII] ?? 0.0,
                $numch[Data::ENC_C40] ?? 0.0,
                $numch[Data::ENC_TXT] ?? 0.0,
                $numch[Data::ENC_EDF] ?? 0.0,
                $numch[Data::ENC_BASE256] ?? 0.0,
            )
        ) {
            return Data::ENC_X12;
        }

        return $this->stepRf($numch, $pos, $data_length, $charscount, $data);
    }
}
