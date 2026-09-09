<?php

declare(strict_types=1);

/**
 * RawObject.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Com\Tecnick\Pdf\Parser\Process;

use Com\Tecnick\Pdf\Parser\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Parser\Process\RawObject
 *
 * Process Raw Objects
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * @phpstan-type RawObjectArray array{
 *                 0: string,
 *                 1: string|array<int, array{
 *                     0: string,
 *                     1: string|array<int, array{
 *                         0: string,
 *                         1: string|array<int, array{
 *                             0: string,
 *                             1: string|array<int, array{
 *                                 0: string,
 *                                 1: string|array<int, array{
 *                                     0: string,
 *                                     1: string,
 *                                     2: int,
 *                                     3?: array{string, array<string>},
 *                                 }>,
 *                                 2: int,
 *                                 3?: array{string, array<string>},
 *                             }>,
 *                             2: int,
 *                             3?: array{string, array<string>},
 *                         }>,
 *                         2: int,
 *                         3?: array{string, array<string>},
 *                     }>,
 *                     2: int,
 *                     3?: array{string, array<string>},
 *                   }>,
 *                 2: int,
 *                 3?: array{string, array<string>},
 *             }
 *
 * @phpstan-type RawObjectValue string|array<int, array{
 *                 0: string,
 *                 1: string|array<int, array{
 *                     0: string,
 *                     1: string,
 *                     2: int,
 *                 }>,
 *                 2: int,
 *             }>
 */
abstract class RawObject
{
    /**
     * Maximum nesting depth allowed for array and dictionary objects.
     */
    protected const MAX_NESTING_DEPTH = 256;

    /**
     * Characters allowed inside a hexadecimal string object.
     */
    protected const HEXCHARS = "0123456789ABCDEFabcdef\x09\x0a\x0c\x0d\x20";

    /**
     * White-space and delimiter characters that terminate a name object
     * (PDF 32000-1 Table 1 and Table 2).
     */
    protected const NAME_DELIMITERS = "\x00\x09\x0a\x0c\x0d\x20\x25\x28\x29\x2f\x3c\x3e\x5b\x5d\x7b\x7d";

    /**
     * Raw content of the PDF document.
     */
    protected string $pdfdata = '';

    /**
     * Current nesting depth of array and dictionary objects.
     */
    protected int $nesting = 0;

    /**
     * Offset where the data of the most recently tokenized stream begins
     * (i.e. just after the "stream" keyword and its end-of-line marker),
     * or -1 when no stream payload has been located.
     */
    protected int $streamDataStart = -1;

    /**
     * Array of PDF objects.
     *
     * @var array<string, array<int, RawObjectArray>>
     */
    protected array $objects = [];

    /**
     * Map symbols with corresponding processing methods.
     *
     * @var array<string, string>
     */
    protected const SYMBOLMETHOD = [
        // \x2F SOLIDUS
        '/' => 'Solidus',
        // \x28 LEFT PARENTHESIS
        '(' => 'Parenthesis',
        // \x29 RIGHT PARENTHESIS
        ')' => 'Parenthesis',
        // \x5B LEFT SQUARE BRACKET
        '[' => 'Bracket',
        // \x5D RIGHT SQUARE BRACKET
        ']' => 'Bracket',
        // \x3C LESS-THAN SIGN
        '<' => 'Angular',
        // \x3E GREATER-THAN SIGN
        '>' => 'Angular',
    ];

    /**
     * Get object type, raw value and offset to next object
     *
     * @param int $offset Object offset.
     *
     * @return RawObjectArray Array containing: object type, raw value and offset to next object
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getRawObject(int $offset = 0): array
    {
        $pdflen = \strlen($this->pdfdata);
        // skip white space chars and comments:
        // \x00 null (NUL)
        // \x09 horizontal tab (HT)
        // \x0A line feed (LF)
        // \x0C form feed (FF)
        // \x0D carriage return (CR)
        // \x20 space (SP)
        // \x25 percent sign (comment up to the end of the line)
        while (true) {
            $offset += \strspn($this->pdfdata, "\x00\x09\x0a\x0c\x0d\x20", $offset);
            // stop if we reached the end of the (possibly truncated/malformed) data
            if ($offset >= $pdflen) {
                return ['', '', $offset];
            }

            if ($this->pdfdata[$offset] !== '%') {
                break;
            }

            // the '%' itself is never an EOL char, so this always advances
            $offset += \strcspn($this->pdfdata, "\r\n", $offset);
        }

        // get first char
        $char = $this->pdfdata[$offset];
        $objtype = '';
        $objval = '';
        // map symbols with corresponding processing methods
        $methodSuffix = self::SYMBOLMETHOD[$char] ?? null;
        if (\is_string($methodSuffix)) {
            $method = 'process' . $methodSuffix;
            $this->$method($char, $offset, $objtype, $objval);
        } elseif (!$this->processDefaultName($offset, $objtype, $objval)) {
            $this->processDefault($offset, $objtype, $objval);
        }

        return [$objtype, $objval, $offset];
    }

    /**
     * Process name object
     * \x2F SOLIDUS
     *
     * @param string     $char    Symbol to process
     * @param-out int    $offset  Offset after processing
     * @param-out string $objtype Object type after processing
     * @param-out string $objval  Object content after processing
     */
    protected function processSolidus(string $char, int &$offset, string &$objtype, string|array &$objval): void
    {
        $objtype = $char;
        ++$offset;
        // a name runs up to the first white space or delimiter character
        $namelen = \strcspn($this->pdfdata, self::NAME_DELIMITERS, $offset);
        if ($namelen > 0) {
            $raw = \substr($this->pdfdata, $offset, $namelen);
            // the offset advances over the raw bytes, the value keeps the decoded form
            $offset += $namelen;
            // decode the #xx hex escapes allowed in name objects (PDF 32000-1 7.3.5)
            $objval = \str_contains($raw, '#') ? $this->decodeNameEscapes($raw) : $raw;
        }
    }

    /**
     * Decode the #xx hexadecimal escapes of a name object.
     *
     * @param string $name Raw name bytes.
     *
     * @return string Decoded name.
     */
    protected function decodeNameEscapes(string $name): string
    {
        $decoded = \preg_replace_callback(
            '/#([0-9A-Fa-f]{2})/',
            static fn(array $match): string => \chr((int) \hexdec($match[1] ?? '0')),
            $name,
        );

        return \is_string($decoded) ? $decoded : $name;
    }

    /**
     * Process literal string object
     * \x28 LEFT PARENTHESIS and \x29 RIGHT PARENTHESIS
     *
     * @param string     $char    Symbol to process
     * @param-out int    $offset  Offset after processing
     * @param-out string $objtype Object type after processing
     * @param-out string $objval  Object content after processing
     */
    protected function processParenthesis(string $char, int &$offset, string &$objtype, string|array &$objval): void
    {
        $objtype = $char;
        ++$offset;
        if ($char !== '(') {
            return;
        }

        $open_bracket = 1;
        $pdflen = \strlen($this->pdfdata);
        $strpos = $offset;
        while ($open_bracket > 0 && $strpos < $pdflen) {
            $chr = $this->pdfdata[$strpos];
            switch ($chr) {
                case '\\':
                    // REVERSE SOLIDUS (5Ch) (Backslash)
                    // skip next character
                    ++$strpos;
                    break;
                case '(':
                    // LEFT PARENTHESIS (28h)
                    ++$open_bracket;
                    break;
                case ')':
                    // RIGHT PARENTHESIS (29h)
                    --$open_bracket;
                    break;
            }

            ++$strpos;
        }

        // an escape on the last byte pushes the cursor past the end of the data
        $strpos = \min($strpos, $pdflen);
        // the closing delimiter is consumed but is not part of the value;
        // an unterminated string has no delimiter to strip
        $end = $open_bracket === 0 ? $strpos - 1 : $strpos;
        $objval = \substr($this->pdfdata, $offset, $end - $offset);
        $offset = $strpos;
    }

    /**
     * Process array content
     * \x5B LEFT SQUARE BRACKET and \x5D RIGHT SQUARE BRACKET
     *
     * @param string           $char    Symbol to process
     * @param-out int          $offset  Offset after processing
     * @param-out string       $objtype Object type after processing
     * @param-out string|array $objval  Object content after processing
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function processBracket(string $char, int &$offset, string &$objtype, string|array &$objval): void
    {
        // array object
        $objtype = $char;
        ++$offset;
        if ($char === '[') {
            // get array content
            $elements = [];
            $this->enterNesting();
            try {
                $this->collectElements($offset, $elements, ']');
            } finally {
                --$this->nesting;
            }

            $objval = $elements;
        }
    }

    /**
     * Collect the elements of an array or dictionary up to its closing delimiter.
     *
     * Neither the closing delimiter nor an unparsable token is stored. An array or a
     * dictionary cannot span an indirect object, so "endobj" also ends the collection
     * and is left unconsumed for the caller.
     *
     * @param int                        $offset  Offset to read from
     * @param array<int, RawObjectArray> $objval  Collected elements
     * @param string                     $closing Closing delimiter type
     *
     * @param-out int                        $offset Offset after processing
     * @param-out array<int, RawObjectArray> $objval Collected elements
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function collectElements(int &$offset, array &$objval, string $closing): void
    {
        while (true) {
            $oldoffset = $offset;
            $element = $this->getRawObject($offset);
            $offset = $element[2];
            if ($element[0] === $closing) {
                // closing delimiter reached: consumed but not stored
                return;
            }

            if ($element[0] === 'endobj') {
                // the container is unterminated: give the keyword back to the caller
                $offset = $oldoffset;
                return;
            }

            if ($element[0] === '' || $offset === $oldoffset) {
                // end of data or a byte that cannot be tokenized: stop without storing it
                return;
            }

            $objval[] = $element;
        }
    }

    /**
     * Process \x3C LESS-THAN SIGN and \x3E GREATER-THAN SIGN
     *
     * @param string           $char    Symbol to process
     * @param-out int          $offset  Offset after processing
     * @param-out string       $objtype Object type after processing
     * @param-out string|array $objval  Object content after processing
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function processAngular(string $char, int &$offset, string &$objtype, string|array &$objval): void
    {
        if (($this->pdfdata[$offset + 1] ?? null) === $char) {
            // dictionary object
            $objtype = $char . $char;
            $offset += 2;
            if ($char === '<') {
                // get array content
                $elements = [];
                $this->enterNesting();
                try {
                    $this->collectElements($offset, $elements, '>>');
                } finally {
                    --$this->nesting;
                }

                $objval = $elements;
            }
        } else {
            $objtype = $char;
            ++$offset;
            if ($char !== '<') {
                // an unbalanced '>' is a single byte: it must not swallow the '>>' that
                // terminates the enclosing dictionary
                return;
            }

            // hexadecimal string object
            $endpos = $offset < \strlen($this->pdfdata) ? \strpos($this->pdfdata, '>', $offset) : false;
            if ($endpos === false) {
                return;
            }

            // the payload is valid only when every byte up to '>' is a hex digit or white space
            $raw = \substr($this->pdfdata, $offset, $endpos - $offset);
            if ($raw !== '' && \strspn($raw, self::HEXCHARS) === \strlen($raw)) {
                // remove white space characters
                $objval = \str_replace(["\x09", "\x0a", "\x0c", "\x0d", "\x20"], '', $raw);
                if ((\strlen($objval) % 2) === 1) {
                    // a missing final digit is a zero (PDF 32000-1 7.3.4.3)
                    $objval .= '0';
                }
            }

            $offset = $endpos + 1;
        }
    }

    /**
     * Enter a nested array or dictionary, enforcing the maximum nesting depth.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function enterNesting(): void
    {
        if ($this->nesting >= static::MAX_NESTING_DEPTH) {
            throw new PPException('Maximum object nesting depth exceeded: ' . static::MAX_NESTING_DEPTH);
        }

        ++$this->nesting;
    }

    /**
     * Process the keyword objects: endobj, null, true, false, stream and endstream
     *
     * @param-out int            $offset  Offset after processing
     * @param-out string         $objtype Object type after processing
     * @param-out RawObjectValue $objval  Object content after processing
     *
     * @return bool True in case of match, false otherwise
     */
    protected function processDefaultName(int &$offset, string &$objtype, string|array &$objval): bool
    {
        $status = false;
        if (\substr($this->pdfdata, $offset, 6) === 'endobj') {
            // indirect object
            $objtype = 'endobj';
            $offset += 6;
            $status = true;
        } elseif (\substr($this->pdfdata, $offset, 4) === 'null') {
            // null object
            $objtype = 'null';
            $offset += 4;
            $objval = 'null';
            $status = true;
        } elseif (\substr($this->pdfdata, $offset, 4) === 'true') {
            // boolean true object
            $objtype = 'boolean';
            $offset += 4;
            $objval = 'true';
            $status = true;
        } elseif (\substr($this->pdfdata, $offset, 5) === 'false') {
            // boolean false object
            $objtype = 'boolean';
            $offset += 5;
            $objval = 'false';
            $status = true;
        } elseif (\substr($this->pdfdata, $offset, 6) === 'stream') {
            // start stream object
            $objtype = 'stream';
            $offset += 6;
            // no payload located yet: drop the offset of the previous stream
            $this->streamDataStart = -1;
            $matches = [];
            // PDF 32000-1 7.3.8.1 requires CRLF or LF right after the keyword, never a bare CR;
            // horizontal white space before the marker is tolerated
            if (
                \preg_match('/\G[\x20\x09]*([\r]?[\n])/', $this->pdfdata, $matches, 0, $offset) === 1
                && ($matches[0] ?? null) !== null
            ) {
                $offset += \strlen($matches[0]);
                // record where the stream payload starts (used for length-aware extraction)
                $this->streamDataStart = $offset;
                if (
                    \preg_match(
                        '/(endstream)[\x09\x0a\x0c\x0d\x20]/isU',
                        $this->pdfdata,
                        $matches,
                        PREG_OFFSET_CAPTURE,
                        $offset,
                    ) === 1
                    && ($matches[0] ?? null) !== null
                    && ($matches[1] ?? null) !== null
                ) {
                    $objval = \substr($this->pdfdata, $offset, (int) $matches[0][1] - $offset);
                    $offset = (int) $matches[1][1];
                }
            }

            $status = true;
        } elseif (\substr($this->pdfdata, $offset, 9) === 'endstream') {
            // end stream object
            $objtype = 'endstream';
            $offset += 9;
            $status = true;
        }

        return $status;
    }

    /**
     * Process indirect references, object start markers and numeric objects
     *
     * @param-out int            $offset  Offset after processing
     * @param-out string         $objtype Object type after processing
     * @param-out RawObjectValue $objval  Object content after processing
     */
    protected function processDefault(int &$offset, string &$objtype, string|array &$objval): void
    {
        $matches = [];
        if (
            \preg_match('/^([0-9]+)[\s]+([0-9]+)[\s]+R/iU', \substr($this->pdfdata, $offset, 33), $matches) === 1
            && ($matches[0] ?? null) !== null
            && ($matches[1] ?? null) !== null
            && ($matches[2] ?? null) !== null
        ) {
            // indirect object reference
            $objtype = 'objref';
            $offset += \strlen($matches[0]);
            $objval = (int) $matches[1] . '_' . (int) $matches[2];
        } elseif (
            \preg_match('/^([0-9]+)[\s]+([0-9]+)[\s]+obj/iU', \substr($this->pdfdata, $offset, 33), $matches) === 1
            && ($matches[0] ?? null) !== null
            && ($matches[1] ?? null) !== null
            && ($matches[2] ?? null) !== null
        ) {
            // object start
            $objtype = 'obj';
            $objval = (int) $matches[1] . '_' . (int) $matches[2];
            $offset += \strlen($matches[0]);
        } elseif (($numlen = \strspn($this->pdfdata, '+-.0123456789', $offset)) > 0) {
            // numeric object
            $objtype = 'numeric';
            $objval = \substr($this->pdfdata, $offset, $numlen);
            $offset += $numlen;
        }
    }
}
