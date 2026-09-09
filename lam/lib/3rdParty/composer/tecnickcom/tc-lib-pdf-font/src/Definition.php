<?php

declare(strict_types=1);

/**
 * Definition.php
 *
 * @since     2026-08-27
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

/**
 * Com\Tecnick\Pdf\Font\Definition
 *
 * Reads the decoded font definition file at the types the font data declares.
 *
 * Each member is read at the shape of its entry in Load::DEFAULT_DATA. A member that cannot
 * be read that way, or that Load::DEFAULT_DATA does not declare, is dropped.
 *
 * @since     2026-08-27
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
final class Definition
{
    /**
     * Normalize the decoded members of a font definition file.
     *
     * @param array<array-key, mixed> $raw Decoded definition file.
     *
     * @return array<string, mixed> Members to merge into the font data.
     */
    public static function normalize(array $raw): array
    {
        $data = [];
        /** @var mixed $value */
        foreach ($raw as $key => $value) {
            $name = (string) $key;
            if (!\array_key_exists($name, Load::DEFAULT_DATA)) {
                continue;
            }

            /** @var mixed $member */
            $member = self::member($name, Load::DEFAULT_DATA[$name], $value);
            if ($member !== null) {
                $data[$name] = $member;
            }
        }

        return $data;
    }

    /**
     * Returns a member read at the shape of its default, or null when it cannot be read.
     *
     * @param string $name    Name of the member.
     * @param mixed  $default Default value of the member, which states its shape.
     * @param mixed  $value   Raw value read from the definition file.
     */
    private static function member(string $name, mixed $default, mixed $value): mixed
    {
        if (\is_bool($default)) {
            return \is_scalar($value) ? (bool) $value : null;
        }

        if (\is_int($default) || \is_float($default)) {
            return self::number($value);
        }

        if (\is_string($default)) {
            return self::text($value);
        }

        if (!\is_array($value)) {
            return null;
        }

        return match ($name) {
            'desc' => self::descriptor($value),
            'cidinfo' => self::cidInfo($value),
            'mode' => self::mode($value),
            'FontBBox' => self::numberList($value),
            'cbbox', 'cbboxu' => self::bboxMap($value),
            'cw', 'cwu', 'ctgu', 'ctgdata', 'indexToLoc', 'usedgid' => self::numberMap($value),
            'subsetchars' => self::flagMap($value),
            'enc_map' => self::textMap($value),
            // 'table' and 'encodingTables' belong to the import stage, not to a definition file
            default => null,
        };
    }

    /**
     * Returns a value read as a number, or null when it is not one.
     *
     * @param mixed $value Raw value read from the definition file.
     */
    private static function number(mixed $value): int|float|null
    {
        if (\is_int($value) || \is_float($value)) {
            return $value;
        }

        if (!\is_string($value) || !\is_numeric($value)) {
            return null;
        }

        // a numeric string is returned as an integer when it spells one
        $float = (float) $value;
        $int = (int) $float;
        return (float) $int === $float ? $int : $float;
    }

    /**
     * Returns a value read as a string, or null when it is not one.
     *
     * @param mixed $value Raw value read from the definition file.
     */
    private static function text(mixed $value): ?string
    {
        if (\is_string($value)) {
            return $value;
        }

        // a number is spelled out
        return \is_int($value) || \is_float($value) ? (string) $value : null;
    }

    /**
     * Returns a key read as a character code, or null when it is not one.
     *
     * @param mixed $key Raw key read from the definition file.
     */
    private static function code(mixed $key): ?int
    {
        if (\is_int($key)) {
            return $key;
        }

        return \is_string($key) && \is_numeric($key) ? (int) $key : null;
    }

    /**
     * Returns the numbers of a list, dropping everything else.
     *
     * @param array<array-key, mixed> $value Raw list read from the definition file.
     *
     * @return array<int, int|float>
     */
    private static function numberList(array $value): array
    {
        $list = [];
        /** @var mixed $item */
        foreach ($value as $item) {
            $number = self::number($item);
            if ($number !== null) {
                $list[] = $number;
            }
        }

        return $list;
    }

    /**
     * Returns a map of character code to number, dropping everything else.
     *
     * @param array<array-key, mixed> $value Raw map read from the definition file.
     *
     * @return array<int, int|float>
     */
    private static function numberMap(array $value): array
    {
        $map = [];
        /** @var mixed $item */
        foreach ($value as $key => $item) {
            $code = self::code($key);
            $number = self::number($item);
            if ($code !== null && $number !== null) {
                $map[$code] = $number;
            }
        }

        return $map;
    }

    /**
     * Returns a map of character code to flag, dropping everything else.
     *
     * @param array<array-key, mixed> $value Raw map read from the definition file.
     *
     * @return array<int, bool>
     */
    private static function flagMap(array $value): array
    {
        $map = [];
        /** @var mixed $item */
        foreach ($value as $key => $item) {
            $code = self::code($key);
            if ($code !== null && \is_scalar($item)) {
                $map[$code] = (bool) $item;
            }
        }

        return $map;
    }

    /**
     * Returns a map of character code to string, dropping everything else.
     *
     * @param array<array-key, mixed> $value Raw map read from the definition file.
     *
     * @return array<int, string>
     */
    private static function textMap(array $value): array
    {
        $map = [];
        /** @var mixed $item */
        foreach ($value as $key => $item) {
            $code = self::code($key);
            $text = self::text($item);
            if ($code !== null && $text !== null) {
                $map[$code] = $text;
            }
        }

        return $map;
    }

    /**
     * Returns a map of character code to glyph bounding box, dropping everything else.
     *
     * @param array<array-key, mixed> $value Raw map read from the definition file.
     *
     * @return array<int, array<int, int|float>>
     */
    private static function bboxMap(array $value): array
    {
        $map = [];
        /** @var mixed $box */
        foreach ($value as $key => $box) {
            $code = self::code($key);
            if ($code === null || !\is_array($box)) {
                continue;
            }

            $corners = self::numberList($box);
            if (\count($corners) === 4) {
                $map[$code] = $corners;
            }
        }

        return $map;
    }

    /**
     * Returns the font descriptor entries.
     *
     * A value is kept when it is a number or a string, since OutFont::getKeyValOut() writes
     * it as the raw syntax following a PDF name.
     *
     * @param array<array-key, mixed> $value Raw descriptor read from the definition file.
     *
     * @return array<array-key, int|float|string>
     */
    private static function descriptor(array $value): array
    {
        $desc = [];
        /** @var array<string, int|string> $default */
        $default = Load::DEFAULT_DATA['desc'];
        /** @var mixed $item */
        foreach ($value as $key => $item) {
            $name = (string) $key;
            if ($name === '') {
                continue;
            }

            $entry = self::descriptorEntry($default[$name] ?? null, $item);
            if ($entry !== null) {
                $desc[$name] = $entry;
            }
        }

        return $desc;
    }

    /**
     * Returns a descriptor entry read at the shape of its default, or null when it cannot
     * be read.
     *
     * An entry the descriptor does not declare is kept as a number or as a string.
     *
     * @param mixed $default Default value of the entry, or null when it declares none.
     * @param mixed $value   Raw value read from the definition file.
     */
    private static function descriptorEntry(mixed $default, mixed $value): int|float|string|null
    {
        if (\is_string($default)) {
            // the bounding box is the only entry of the descriptor written as a string
            return self::boundingBox($value);
        }

        if (\is_int($default) || \is_float($default)) {
            return self::number($value);
        }

        return \is_string($value) ? $value : self::number($value);
    }

    /**
     * Returns the bounding box of the descriptor as an array literal, or null when the
     * value is neither a string nor an array of numbers.
     *
     * @param mixed $value Raw bounding box read from the definition file.
     */
    private static function boundingBox(mixed $value): ?string
    {
        if (\is_string($value)) {
            return $value;
        }

        return \is_array($value) ? '[' . \implode(' ', self::numberList($value)) . ']' : null;
    }

    /**
     * Returns the CID system info entries.
     *
     * @param array<array-key, mixed> $value Raw entries read from the definition file.
     *
     * @return array<string, mixed>
     */
    private static function cidInfo(array $value): array
    {
        $info = [];
        foreach (['Registry', 'Ordering'] as $name) {
            $text = self::text($value[$name] ?? null);
            if ($text !== null) {
                $info[$name] = $text;
            }
        }

        $supplement = self::number($value['Supplement'] ?? null);
        if ($supplement !== null) {
            $info['Supplement'] = (int) $supplement;
        }

        /** @var mixed $uni2cid */
        $uni2cid = $value['uni2cid'] ?? null;
        if (\is_array($uni2cid)) {
            $info['uni2cid'] = self::numberMap($uni2cid);
        }

        return $info;
    }

    /**
     * Returns the style mode flags.
     *
     * @param array<array-key, mixed> $value Raw flags read from the definition file.
     *
     * @return array<string, bool>
     */
    private static function mode(array $value): array
    {
        $mode = [];
        /** @var array<string, bool> $default */
        $default = Load::DEFAULT_DATA['mode'];
        foreach (\array_keys($default) as $name) {
            /** @var mixed $flag */
            $flag = $value[$name] ?? null;
            if (\is_scalar($flag)) {
                $mode[$name] = (bool) $flag;
            }
        }

        return $mode;
    }
}
