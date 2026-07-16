<?php

/**
 * EmptyReadStreamWrapper.php
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
 * A stream wrapper that returns initial data on the first read then returns
 * empty strings on all subsequent reads while never signalling EOF via
 * stream_eof().  This exercises the inner `break` in File::rfRead() that
 * fires when fread() yields an empty string before the while-loop condition
 * can detect EOF.
 */
class EmptyReadStreamWrapper
{
    public mixed $context;

    private string $initialData = 'ab';

    private bool $initialRead = false;

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
        if (!$this->initialRead) {
            $this->initialRead = true;
            return $this->initialData;
        }

        // Return empty string while still claiming not at EOF so that the
        // while-loop in rfRead() re-enters and hits the inner break.
        return '';
    }

    // phpcs:ignore PSR1.Methods.CamelCapsMethodName.NotCamelCaps
    public function stream_eof(): bool
    {
        // Never signal EOF — stream_read() will return '' instead.
        return false;
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
