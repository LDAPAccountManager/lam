<?php

/**
 * SingleByteStreamWrapper.php
 *
 * @since     2026-04-30
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Test;

/**
 * A stream wrapper that delivers exactly one byte per stream_read() call.
 * Used to exercise the iterative while-loop in File::rfRead().
 */
class SingleByteStreamWrapper
{
    public mixed $context;

    private string $data = 'abcdefgh';

    private int $position = 0;

    // phpcs:ignore PSR1.Methods.CamelCapsMethodName.NotCamelCaps
    public function stream_open(string $path, string $mode, int $options, ?string &$opened_path): bool
    {
        unset($path, $mode, $options, $opened_path);
        return true;
    }

    // phpcs:ignore PSR1.Methods.CamelCapsMethodName.NotCamelCaps
    public function stream_read(int $count): string
    {
        unset($count);
        // Always return one byte at a time regardless of requested count.
        if ($this->position >= \strlen($this->data)) {
            return '';
        }

        $byte = $this->data[$this->position];
        ++$this->position;
        return $byte;
    }

    // phpcs:ignore PSR1.Methods.CamelCapsMethodName.NotCamelCaps
    public function stream_eof(): bool
    {
        return $this->position >= \strlen($this->data);
    }

    /**
     * @return array<string, mixed>
     */
    // phpcs:ignore PSR1.Methods.CamelCapsMethodName.NotCamelCaps
    public function stream_stat(): array
    {
        return [];
    }
}
