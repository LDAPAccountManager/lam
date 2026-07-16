<?php

declare(strict_types=1);

/**
 * Spec.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\Spec
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @phpstan-type EccSpec array{
 *            0: int,
 *            1: int,
 *            2: int,
 *            3: int,
 *            4: int,
 *        }
 */
class Spec extends \Com\Tecnick\Barcode\Type\Square\QrCode\SpecRs
{
    /**
     * @return array{0: int, 1: int, 2: int, 3: array{0: int, 1: int, 2: int, 3: int}}
     */
    protected function getCapacityRow(int $version): array
    {
        return Data::CAPACITY[$version] ?? [0, 0, 0, [0, 0, 0, 0]];
    }

    protected function getCapacityWords(int $version): int
    {
        return $this->getCapacityRow($version)[1];
    }

    protected function getCapacityEcc(int $version, int $level): int
    {
        return $this->getCapacityRow($version)[3][$level] ?? 0;
    }

    protected function getCapacityWidthValue(int $version): int
    {
        return $this->getCapacityRow($version)[0];
    }

    protected function getCapacityRemainderValue(int $version): int
    {
        return $this->getCapacityRow($version)[2];
    }

    protected function getLenTableBitsValue(int $mode, int $index): int
    {
        $modeTable = Data::LEN_TABLE_BITS[$mode] ?? [0, 0, 0];

        return $modeTable[$index] ?? 0;
    }

    protected function getEccTableValue(int $version, int $level, int $index): int
    {
        $versionTable = Data::ECC_TABLE[$version] ?? [[0, 0], [0, 0], [0, 0], [0, 0]];
        $levelTable = $versionTable[$level] ?? [0, 0];

        return $levelTable[$index] ?? 0;
    }

    protected function getFormatInfoValue(int $level, int $maskNo): int
    {
        $levelTable = Data::FORMAT_INFO[$level] ?? [0, 0, 0, 0, 0, 0, 0, 0];

        return $levelTable[$maskNo] ?? 0;
    }

    /**
     * Return maximum data code length (bytes) for the version.
     *
     * @param int $version Version
     * @param int $level   Error correction level
     *
     * @return int maximum size (bytes)
     */
    public function getDataLength(int $version, int $level): int
    {
        return $this->getCapacityWords($version) - $this->getCapacityEcc($version, $level);
    }

    /**
     * Return maximum error correction code length (bytes) for the version.
     *
     * @param int $version Version
     * @param int $level   Error correction level
     *
     * @return int ECC size (bytes)
     */
    public function getECCLength(int $version, int $level): int
    {
        return $this->getCapacityEcc($version, $level);
    }

    /**
     * Return the width of the symbol for the version.
     *
     * @param int $version Version
     *
     * @return int width
     */
    public function getWidth(int $version): int
    {
        return $this->getCapacityWidthValue($version);
    }

    /**
     * Return the numer of remainder bits.
     *
     * @param int $version Version
     *
     * @return int number of remainder bits
     */
    public function getRemainder(int $version): int
    {
        return $this->getCapacityRemainderValue($version);
    }

    /**
     * Return the maximum length for the mode and version.
     *
     * @param int $mode    Encoding mode
     * @param int $version Version
     *
     * @return int the maximum length (bytes)
     */
    public function maximumWords(int $mode, int $version): int
    {
        if ($mode === Data::MODE_ST || $mode < Data::MODE_NL || $mode > Data::MODE_ST) {
            return 3;
        }

        $lval = match (true) {
            $version <= 9 => 0,
            $version <= 26 => 1,
            default => 2,
        };

        $bits = $this->getLenTableBitsValue($mode, $lval);
        $words = (1 << $bits) - 1;
        if ($mode === Data::MODE_KJ) {
            $words *= 2; // the number of bytes is required
        }

        return $words;
    }

    /**
     * Return an array of ECC specification.
     *
     * @param int   $version Version
     * @param int   $level   Error correction level
     * @param EccSpec $spec Array of ECC specification
     *
     * @return EccSpec spec:
     *            0 = # of type1 blocks
     *            1 = # of data code
     *            2 = # of ecc code
     *            3 = # of type2 blocks
     *            4 = # of data code
     */
    public function getEccSpec(int $version, int $level, array $spec): array
    {
        if (\count($spec) < 5) {
            $spec = [0, 0, 0, 0, 0];
        }

        $bv1 = $this->getEccTableValue($version, $level, 0);
        $bv2 = $this->getEccTableValue($version, $level, 1);
        $data = $this->getDataLength($version, $level);
        $ecc = $this->getECCLength($version, $level);
        if ($bv2 === 0) {
            $spec[0] = $bv1;
            $spec[1] = (int) ($data / $bv1); /* @phpstan-ignore-line */
            $spec[2] = (int) ($ecc / $bv1); /* @phpstan-ignore-line */
            $spec[3] = 0;
            $spec[4] = 0;
            return $spec;
        }

        $spec[0] = $bv1;
        $spec[1] = (int) ($data / ($bv1 + $bv2));
        $spec[2] = (int) ($ecc / ($bv1 + $bv2));
        $spec[3] = $bv2;
        $spec[4] = $spec[1] + 1;

        return $spec;
    }

    /**
     * Return BCH encoded format information pattern.
     *
     * @param int $maskNo Mask number
     * @param int $level  Error correction level
     */
    public function getFormatInfo(int $maskNo, int $level): int
    {
        if ($maskNo < 0 || $maskNo > 7 || $level < 0 || $level > 3) {
            return 0;
        }

        return $this->getFormatInfoValue($level, $maskNo);
    }
}
