<?php

declare(strict_types=1);

/**
 * Byte.php
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Com\Tecnick\File;

use RangeException;

/**
 * Com\Tecnick\File\Byte
 *
 * Function to read byte-level data
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
readonly class Byte
{
    /**
     * Length of the binary string in bytes.
     */
    private int $length;

    /**
     * Initialize a new string to be processed.
     *
     * Values are read in place via \ord() + bitwise math: no per-read unpack()
     * call and no byte-table copy, so memory stays at the size of the string.
     *
     * The read methods are deliberately inlined (each repeats the bounds check
     * and the \ord()/shift sequence) rather than sharing private helpers: every
     * userland call in PHP carries real frame-setup overhead and is not inlined
     * by the engine in the interpreter, so the duplication is a measurable speed
     * win on the hot read path.
     *
     * @param string $str String (binary) from where to extract values
     */
    public function __construct(
        /**
         * Binary string to process
         */
        protected string $str,
    ) {
        $this->length = \strlen($str);
    }

    /**
     * Throw for a read that failed its bounds check.
     *
     * Kept as a separate method so the cold error path does not bloat the
     * inlined read methods; it is never reached on a successful read.
     *
     * @param int $offset Read start position.
     * @param int $length Number of bytes requested.
     *
     * @throws RangeException always.
     */
    private function throwOutOfBounds(int $offset, int $length): never
    {
        throw new RangeException(
            "Out-of-bounds read at offset {$offset} (length {$length}, string length {$this->length})",
        );
    }

    /**
     * Get BYTE from string (8-bit unsigned integer).
     *
     * @param int $offset Point from where to read the data.
     *
     * @return int 8 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getByte(int $offset): int
    {
        if ($offset < 0 || $offset >= $this->length) {
            $this->throwOutOfBounds($offset, 1);
        }

        return \ord($this->str[$offset]);
    }

    /**
     * Get USHORT from string (Big Endian 16-bit unsigned integer).
     *
     * @param int $offset Point from where to read the data
     *
     * @return int 16 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getUShort(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 2)) {
            $this->throwOutOfBounds($offset, 2);
        }

        return (\ord($this->str[$offset]) << 8) | \ord($this->str[$offset + 1]);
    }

    /**
     * Get SHORT from string (Big Endian 16-bit signed integer).
     *
     * @param int $offset Point from where to read the data.
     *
     * @return int 16 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getShort(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 2)) {
            $this->throwOutOfBounds($offset, 2);
        }

        // The uint16 value
        $val = (\ord($this->str[$offset]) << 8) | \ord($this->str[$offset + 1]);

        // Convert unsigned 16-bit to signed (two's complement)
        return ($val ^ 0x8000) - 0x8000;
    }

    /**
     * Get UFWORD from string (Big Endian 16-bit unsigned integer).
     * Alias for getUShort().
     *
     * @param int $offset Point from where to read the data.
     *
     * @return int 16 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getUFWord(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 2)) {
            $this->throwOutOfBounds($offset, 2);
        }

        return (\ord($this->str[$offset]) << 8) | \ord($this->str[$offset + 1]);
    }

    /**
     * Get FWORD from string (Big Endian 16-bit signed integer).
     * Alias for getShort().
     *
     * @param int $offset Point from where to read the data.
     *
     * @return int 16 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getFWord(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 2)) {
            $this->throwOutOfBounds($offset, 2);
        }

        // The uint16 value
        $val = (\ord($this->str[$offset]) << 8) | \ord($this->str[$offset + 1]);

        // Convert unsigned 16-bit to signed (two's complement)
        return ($val ^ 0x8000) - 0x8000;
    }

    /**
     * Get ULONG from string (Big Endian 32-bit unsigned integer).
     *
     * @param int $offset Point from where to read the data
     *
     * @return int 32 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getULong(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 4)) {
            $this->throwOutOfBounds($offset, 4);
        }

        return (
            (\ord($this->str[$offset]) << 24)
            | (\ord($this->str[$offset + 1]) << 16)
            | (\ord($this->str[$offset + 2]) << 8)
            | \ord($this->str[$offset + 3])
        );
    }

    /**
     * Get LONG from string (Big Endian 32-bit signed integer).
     *
     * @param int $offset Point from where to read the data
     *
     * @return int 32 bit value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getLong(int $offset): int
    {
        if ($offset < 0 || $offset > ($this->length - 4)) {
            $this->throwOutOfBounds($offset, 4);
        }

        // The uint32 value
        $val =
            (\ord($this->str[$offset]) << 24)
            | (\ord($this->str[$offset + 1]) << 16)
            | (\ord($this->str[$offset + 2]) << 8)
            | \ord($this->str[$offset + 3]);

        // Convert unsigned 32-bit to signed (two's complement)
        return ($val ^ 0x8000_0000) - 0x8000_0000;
    }

    /**
     * Get FIXED from string (Big Endian 32-bit signed fixed-point number (16.16)).
     *
     * A fixed-point 16.16 number is the signed int16 integer part plus the
     * unsigned 16-bit fractional part divided by 65536 = (1 << 16).
     *
     * @param int $offset Point from where to read the data.
     *
     * @return float Fixed-point value
     *
     * @throws RangeException if the requested read is out of bounds.
     */
    public function getFixed(int $offset): float
    {
        if ($offset < 0 || $offset > ($this->length - 4)) {
            $this->throwOutOfBounds($offset, 4);
        }

        // The int16 integer part
        $int16 = (((\ord($this->str[$offset]) << 8) | \ord($this->str[$offset + 1])) ^ 0x8000) - 0x8000;

        // Add the uint16 fractional part
        return $int16 + (((\ord($this->str[$offset + 2]) << 8) | \ord($this->str[$offset + 3])) / 65_536.0);
    }
}
