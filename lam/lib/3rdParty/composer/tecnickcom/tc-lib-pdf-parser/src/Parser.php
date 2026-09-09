<?php

declare(strict_types=1);

/**
 * Parser.php
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

namespace Com\Tecnick\Pdf\Parser;

use Com\Tecnick\Pdf\Filter\Filter;
use Com\Tecnick\Pdf\Parser\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Parser\Parser
 *
 * PHP class for parsing PDF documents.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 *
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 * @phpstan-import-type XrefData from \Com\Tecnick\Pdf\Parser\Process\XrefStream
 */
class Parser extends \Com\Tecnick\Pdf\Parser\Process\Xref
{
    /**
     * Default maximum size in bytes of a single decoded stream (32MB).
     */
    public const DEFAULT_MAX_STREAM_SIZE = 33_554_432;

    /**
     * Maximum number of indirect object resolutions that may be in flight at once.
     */
    protected const MAX_RESOLUTION_DEPTH = 64;

    /**
     * Maximum size in bytes of a single decoded stream, 0 means unlimited.
     */
    private int $maxStreamSize = self::DEFAULT_MAX_STREAM_SIZE;

    /**
     * Cache of decoded object streams keyed by object stream reference.
     *
     * @var array<string, array<string, array<int, RawObjectArray>>>
     */
    private array $objstmCache = [];

    /**
     * References of the objects that are currently being resolved, used to break
     * reference cycles (e.g. two streams whose /Length point at each other).
     *
     * @var array<string, bool>
     */
    private array $resolving = [];

    /**
     * Array of configuration parameters.
     *
     * @var array<string, bool>
     */
    private array $cfg = [
        'ignore_filter_errors' => false,
        'decode_streams' => true,
    ];

    /**
     * Initialize the PDF parser
     *
     * @param array<string, bool|int> $cfg Configuration parameters:
     *                                     'ignore_filter_errors': if true, ignore filter decoding errors;
     *                                     'decode_streams': if true, decode stream payloads while parsing
     *                                     regular indirect objects;
     *                                     'max_stream_size': maximum size in bytes of a single decoded
     *                                     stream, 0 means unlimited.
     */
    public function __construct(array $cfg = [])
    {
        if (\array_key_exists('max_stream_size', $cfg)) {
            $this->maxStreamSize = \max(0, (int) $cfg['max_stream_size']);
        }

        if (\array_key_exists('ignore_filter_errors', $cfg)) {
            $this->cfg['ignore_filter_errors'] = (bool) $cfg['ignore_filter_errors'];
        }

        if (\array_key_exists('decode_streams', $cfg)) {
            $this->cfg['decode_streams'] = (bool) $cfg['decode_streams'];
        }
    }

    /**
     * Parse a PDF document into an array of objects
     *
     * @param string $data PDF data to parse.
     *
     * @return array{
     *             0: XrefData,
     *             1: array<string, array<int, RawObjectArray>>,
     *         }
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function parse(string $data): array
    {
        if ($data === '') {
            throw new PPException('Empty PDF data.');
        }

        // find the pdf header starting position
        if (($trimpos = \strpos($data, '%PDF-')) === false) {
            throw new PPException('Invalid PDF data: missing %PDF header.');
        }

        // get PDF content string
        $this->pdfdata = \substr($data, $trimpos);
        // reset per-document state so the same instance can be reused for multiple parses
        $this->mrkoff = [];
        $this->xrefdone = [];
        $this->objstmCache = [];
        $this->resolving = [];
        $this->nesting = 0;
        $this->streamDataStart = -1;
        $this->objects = [];
        $this->xref = self::XREF_EMPTY;

        try {
            // get xref and trailer data
            $this->xref = $this->getXrefData();
            // parse all document objects
            $decodeStreams = $this->cfg['decode_streams'] ?? true;
            foreach ($this->xref['xref'] as $obj => $offset) {
                // an object resolved as a side effect of another one is cached without
                // decoding its stream: keep it only when nothing is left to decode
                if (
                    \array_key_exists($obj, $this->objects)
                    && !($decodeStreams && $this->hasUndecodedStream($this->objects[$obj]))
                ) {
                    continue;
                }

                if (\is_int($offset)) {
                    if ($offset <= 0) {
                        continue;
                    }

                    // decode objects with positive offset
                    $this->objects[$obj] = $this->getIndirectObject($obj, $offset, $decodeStreams);
                    continue;
                }

                if (\preg_match('/^\d+_\d+_\d+$/', $offset) === 1) {
                    $compressedObj = $this->getCompressedObject($obj, $offset);
                    if ($compressedObj !== null) {
                        $this->objects[$obj] = $compressedObj;
                    }
                }
            }
        } finally {
            // never keep the document data on the instance
            $this->pdfdata = '';
        }

        return [$this->xref, $this->objects];
    }

    /**
     * Get content of indirect object.
     *
     * @param string $obj_ref  Object number and generation number separated by underscore character.
     * @param int    $offset   Object offset.
     * @param bool   $decoding If true decode streams.
     *
     * @return array<int, RawObjectArray> Object data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
    {
        $obj = \explode('_', $obj_ref);
        if (\count($obj) !== 2) {
            throw new PPException('Invalid object reference: ' . \serialize($obj));
        }

        // an indirect reference to an undefined object shall be considered a reference to the null object
        $nullobj = [['null', 'null', $offset]];

        // an out-of-range offset cannot point at any object
        if ($offset < 0 || $offset >= \strlen($this->pdfdata)) {
            return $nullobj;
        }

        // break reference cycles (e.g. two streams whose /Length point at each other)
        // and bound the depth of an acyclic chain of references
        if (isset($this->resolving[$obj_ref]) || $this->resolutionDepthReached()) {
            return $nullobj;
        }

        /** @var array{0: string, 1: string} $obj */
        $objref = $obj[0] . ' ' . $obj[1] . ' obj';
        // ignore leading zeros
        $offset += \strspn($this->pdfdata, '0', $offset);
        $objPos = \strpos($this->pdfdata, $objref, $offset);
        if ($objPos !== $offset) {
            ++$offset;
            if ($offset >= \strlen($this->pdfdata)) {
                return $nullobj;
            }

            $objPos = \strpos($this->pdfdata, $objref, $offset);
            if ($objPos !== $offset) {
                return [['null', 'null', $offset]];
            }
        }

        // starting position of object content
        $offset += \strlen($objref);

        $this->resolving[$obj_ref] = true;
        try {
            // return raw object content
            return $this->getRawIndirectObject($offset, $decoding);
        } finally {
            unset($this->resolving[$obj_ref]);
        }
    }

    /**
     * Get content of indirect object.
     *
     * @param int  $offset   Object offset.
     * @param bool $decoding If true decode streams.
     *
     * @return array<int, RawObjectArray> Object data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getRawIndirectObject(int $offset, bool $decoding): array
    {
        // get array of object content
        $objdata = [];
        $idx = 0; // object main index
        /** @var RawObjectArray|null $prevElement previously parsed element (null on first iteration) */
        $prevElement = null;
        do {
            $oldoffset = $offset;

            $element = $this->getRawObject($offset);
            $offset = $element[2];
            // capture the stream payload start before any nested tokenization can clobber it
            $streamDataStart = $this->streamDataStart;
            // decode stream using stream's dictionary information
            if (
                $element[0] === 'stream'
                && \is_array($prevElement)
                && $prevElement[0] === '<<'
                && \is_array($prevElement[1])
                && \is_string($element[1])
            ) {
                /** @var array<int, RawObjectArray> $sdic */
                $sdic = $prevElement[1];
                $length = $this->getDeclaredLength($sdic);
                // re-slice the payload using the declared /Length when the first "endstream"
                // marker is a false positive inside the binary data
                $reslice = $this->resliceStreamByLength($length, $streamDataStart, $element[1]);
                if ($reslice !== null) {
                    $element[1] = $reslice['stream'];
                    $offset = $reslice['offset'];
                    // index 2 is the offset to the next object: keep it in step with the cursor
                    $element[2] = $offset;
                } elseif ($length === null) {
                    // no length to slice by: the end-of-line before "endstream" is the only
                    // part of the extracted bytes known not to belong to the payload
                    $element[1] = $this->stripStreamEol($element[1]);
                }

                if ($decoding) {
                    $element[3] = $this->decodeStream($sdic, $element[1]);
                }
            }

            if ($element[0] === 'endobj') {
                // closing delimiter reached: consumed but not stored
                break;
            }

            if ($element[0] === '' || $offset === $oldoffset) {
                // end of data or a byte that cannot be tokenized: stop without storing it
                break;
            }

            $objdata[$idx] = $element;
            $prevElement = $element;
            ++$idx;
        } while (true);

        // return raw object content
        return $objdata;
    }

    /**
     * Re-slice a stream payload using the declared /Length when it disagrees with the
     * payload the tokenizer extracted up to the first "endstream" marker.
     *
     * @param int|null $length    Declared /Length, or null when it cannot be determined.
     * @param int      $dataStart Offset where the stream payload starts.
     * @param string   $extracted Payload extracted up to the first "endstream".
     *
     * @return array{stream: string, offset: int}|null Corrected payload and next offset, or null.
     */
    private function resliceStreamByLength(?int $length, int $dataStart, string $extracted): ?array
    {
        if ($length === null || $length <= 0) {
            return null;
        }

        // nothing to correct when the declaration matches what was extracted
        if ($length === \strlen($extracted)) {
            return null;
        }

        $pdfLen = \strlen($this->pdfdata);
        $end = $dataStart + $length;
        if ($dataStart < 0 || $end > $pdfLen) {
            return null;
        }

        // the declared length must be followed (after optional EOL) by the real "endstream"
        $tailStart = $end + \strspn($this->pdfdata, "\x00\x09\x0a\x0c\x0d\x20", $end);
        if (\substr($this->pdfdata, $tailStart, 9) !== 'endstream') {
            return null;
        }

        return [
            'stream' => \substr($this->pdfdata, $dataStart, $length),
            'offset' => $tailStart,
        ];
    }

    /**
     * Remove the end-of-line marker that precedes "endstream" and that
     * PDF 32000-1 7.3.8.1 keeps out of the stream data.
     *
     * @param string $stream Payload extracted up to the first "endstream".
     *
     * @return string Payload without its trailing end-of-line.
     */
    private function stripStreamEol(string $stream): string
    {
        if (\str_ends_with($stream, "\r\n")) {
            return \substr($stream, 0, -2);
        }

        if (\str_ends_with($stream, "\n") || \str_ends_with($stream, "\r")) {
            return \substr($stream, 0, -1);
        }

        return $stream;
    }

    /**
     * Resolve the declared /Length of a stream dictionary, following indirect references.
     *
     * @param array<int, RawObjectArray> $sdic Stream's dictionary array.
     *
     * @return int|null The declared length, or null when it cannot be determined.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function getDeclaredLength(array $sdic): ?int
    {
        $count = \count($sdic);
        // only even positions of the flat key/value list are dictionary keys
        for ($i = 0; $i < $count; $i += 2) {
            $key = $sdic[$i] ?? null;
            if (!\is_array($key) || $key[0] !== '/' || $key[1] !== 'Length') {
                continue;
            }

            return $this->resolveLengthValue($sdic[$i + 1] ?? null);
        }

        return null;
    }

    /**
     * Resolve a /Length value to its number, following an indirect reference.
     *
     * @param RawObjectArray|null $value Raw object found in the /Length value position.
     *
     * @return int|null The declared length, or null when it cannot be determined.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function resolveLengthValue(?array $value): ?int
    {
        if ($value === null) {
            return null;
        }

        if ($value[0] === 'objref') {
            $value = $this->getObjectVal($value);
        }

        if ($value[0] === 'numeric' && \is_scalar($value[1])) {
            return (int) $value[1];
        }

        return null;
    }

    /**
     * Get the content of object, resolving indirect object reference if necessary.
     *
     * @param RawObjectArray $obj Object value.
     *
     * @return RawObjectArray Object data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getObjectVal(array $obj): array
    {
        if ($obj[0] === 'objref' && \is_string($obj[1])) {
            // reference to indirect object
            if (($this->objects[$obj[1]][0] ?? null) !== null) {
                // this object has been already parsed
                return $this->objects[$obj[1]][0];
            }

            if (isset($this->resolving[$obj[1]]) || $this->resolutionDepthReached()) {
                // cycle or depth limit: leave the reference unresolved
                return $obj;
            }

            if (isset($this->xref['xref'][$obj[1]])) {
                $xrefOffset = $this->xref['xref'][$obj[1]];
                if (\is_int($xrefOffset)) {
                    if ($xrefOffset <= 0) {
                        return $obj;
                    }

                    // parse new object
                    $this->objects[$obj[1]] = $this->getIndirectObject($obj[1], $xrefOffset, false);
                    if (($this->objects[$obj[1]][0] ?? null) !== null) {
                        return $this->objects[$obj[1]][0];
                    }

                    return $obj;
                }

                if (\preg_match('/^\d+_\d+_\d+$/', $xrefOffset) === 1) {
                    $compressedObj = $this->getCompressedObject($obj[1], $xrefOffset);
                    if ($compressedObj !== null) {
                        $this->objects[$obj[1]] = $compressedObj;
                        return $this->objects[$obj[1]][0] ?? $obj;
                    }

                    return $obj;
                }

                return $obj;
            }
        }

        return $obj;
    }

    /**
     * Resolve one compressed object by object-stream locator.
     *
     * @param string $objRef  Target object reference (e.g. "14_0").
     * @param string $locator Object-stream locator "streamObj_streamGen_index".
     *
     * @return array<int, RawObjectArray>|null
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function getCompressedObject(string $objRef, string $locator): ?array
    {
        $parts = \explode('_', $locator);
        $streamRef = $parts[0] . '_' . ($parts[1] ?? '');
        $cache = $this->objstmCache[$streamRef] ?? null;
        if (!\is_array($cache)) {
            // seed the cache before parsing: an object stream whose own dictionary
            // references an object stored inside itself must not re-enter this method
            $this->objstmCache[$streamRef] = [];
            $this->objstmCache[$streamRef] = $this->parseObjectStream($streamRef);
            $cache = $this->objstmCache[$streamRef];
        }

        $obj = $cache[$objRef] ?? null;
        return \is_array($obj) ? $obj : null;
    }

    /**
     * Parse a PDF object stream and return extracted objects keyed as "objNum_0".
     *
     * @param string $streamRef Object stream reference.
     *
     * @return array<string, array<int, RawObjectArray>>
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function parseObjectStream(string $streamRef): array
    {
        if (($this->objects[$streamRef][0] ?? null) === null) {
            $streamOffset = $this->xref['xref'][$streamRef] ?? null;
            if (!\is_int($streamOffset) || $streamOffset <= 0) {
                return [];
            }

            $this->objects[$streamRef] = $this->getIndirectObject($streamRef, $streamOffset, true);
        }

        $streamObj = $this->objects[$streamRef];
        if ($this->hasUndecodedStream($streamObj)) {
            $streamOffset = $this->xref['xref'][$streamRef] ?? null;
            if (!\is_int($streamOffset) || $streamOffset <= 0) {
                return [];
            }

            $streamObj = $this->getIndirectObject($streamRef, $streamOffset, true);
            $this->objects[$streamRef] = $streamObj;
        }

        [$dict, $decodedData] = $this->extractObjectStreamEnvelope($streamObj);
        if ($dict === null || $decodedData === null) {
            return [];
        }

        [$n, $first] = $this->readObjectStreamConfig($dict);
        if ($n <= 0 || $first < 0) {
            return [];
        }

        $index = $this->readObjectStreamIndex($decodedData, $n, $first);
        if ($index === null) {
            return [];
        }

        return $this->extractObjectsFromStream($decodedData, $first, $index['objNums'], $index['objOffsets']);
    }

    /**
     * Extract the dictionary and the decoded payload of an object stream.
     *
     * @param array<int, RawObjectArray> $streamObj Parsed object stream.
     *
     * @return array{0: array<int, RawObjectArray>|null, 1: string|null} Dictionary and decoded payload.
     */
    private function extractObjectStreamEnvelope(array $streamObj): array
    {
        $dict = null;
        $decodedData = null;
        foreach ($streamObj as $element) {
            if ($element[0] === '<<' && \is_array($element[1])) {
                $dict = $element[1];
                continue;
            }

            if ($element[0] === 'stream' && \is_array($element[3] ?? null)) {
                $decodedData = $element[3][0];
            }
        }

        return [
            \is_array($dict) ? $dict : null,
            \is_string($decodedData) ? $decodedData : null,
        ];
    }

    /**
     * Read the /N and /First entries of an object stream dictionary.
     *
     * @param array<int, RawObjectArray> $dict Object stream dictionary.
     *
     * @return array{0: int, 1: int} Number of embedded objects and offset of the first body.
     */
    private function readObjectStreamConfig(array $dict): array
    {
        $n = 0;
        $first = 0;
        $dictCount = \count($dict);
        // a dictionary is a flat list of key/value pairs: only even positions are keys,
        // otherwise a value that happens to be the name /N or /First is taken for a key
        for ($idx = 0; $idx < $dictCount; $idx += 2) {
            $key = $dict[$idx] ?? null;
            $val = $dict[$idx + 1] ?? null;
            if (!\is_array($key) || !\is_array($val)) {
                continue;
            }

            if ($key[0] !== '/' || !\is_string($key[1])) {
                continue;
            }

            if ($key[1] === 'N' && $val[0] === 'numeric' && \is_scalar($val[1])) {
                $n = (int) $val[1];
                continue;
            }

            if ($key[1] === 'First' && $val[0] === 'numeric' && \is_scalar($val[1])) {
                $first = (int) $val[1];
            }
        }

        return [$n, $first];
    }

    /**
     * Read the object number and offset pairs from the header of an object stream.
     *
     * @param string $decodedData Decoded object stream payload.
     * @param int    $n           Number of embedded objects.
     * @param int    $first       Byte offset where object bodies begin.
     *
     * @return array{objNums: array<int, int>, objOffsets: array<int, int>}|null Index, or null when invalid.
     */
    private function readObjectStreamIndex(string $decodedData, int $n, int $first): ?array
    {
        $header = \substr($decodedData, 0, $first);
        $meta = \preg_split('/\s+/', \trim($header));
        if (!\is_array($meta) || \count($meta) < (2 * $n)) {
            return null;
        }

        $objNums = [];
        $objOffsets = [];
        for ($idx = 0; $idx < $n; ++$idx) {
            $numTok = $meta[2 * $idx] ?? null;
            $offTok = $meta[(2 * $idx) + 1] ?? null;
            if (!\is_string($numTok) || !\is_string($offTok) || !\is_numeric($numTok) || !\is_numeric($offTok)) {
                return null;
            }

            $objNums[$idx] = (int) $numTok;
            $objOffsets[$idx] = (int) $offTok;
        }

        return [
            'objNums' => $objNums,
            'objOffsets' => $objOffsets,
        ];
    }

    /**
     * Parse the bodies of the objects embedded in an object stream.
     *
     * @param string          $decodedData Decoded object stream payload.
     * @param int             $first       Byte offset where object bodies begin.
     * @param array<int, int> $objNums     Embedded object numbers.
     * @param array<int, int> $objOffsets  Embedded object offsets from $first.
     *
     * @return array<string, array<int, RawObjectArray>> Objects keyed as "objNum_0".
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function extractObjectsFromStream(string $decodedData, int $first, array $objNums, array $objOffsets): array
    {
        $result = [];
        $n = \count($objNums);
        $streamLen = \strlen($decodedData);
        for ($idx = 0; $idx < $n; ++$idx) {
            $start = $first + ($objOffsets[$idx] ?? 0);
            $nextStart = $idx < ($n - 1) ? $first + ($objOffsets[$idx + 1] ?? 0) : $streamLen;
            if ($start < 0 || $nextStart < $start || $start > $streamLen) {
                continue;
            }

            $body = \trim(\substr($decodedData, $start, $nextStart - $start));
            if ($body === '') {
                continue;
            }

            $miniObj = $this->parseObjectBody($body);
            $key = ($objNums[$idx] ?? 0) . '_0';
            if ($key !== '0_0' && $miniObj !== null) {
                $result[$key] = $miniObj;
            }
        }

        return $result;
    }

    /**
     * Check whether a parsed object holds a stream whose payload was not decoded.
     *
     * @param array<int, RawObjectArray> $streamObj Parsed object.
     */
    private function hasUndecodedStream(array $streamObj): bool
    {
        foreach ($streamObj as $element) {
            if ($element[0] !== 'stream') {
                continue;
            }

            if (\is_array($element[3] ?? null)) {
                continue;
            }

            return true;
        }

        return false;
    }

    /**
     * Check whether the maximum number of in-flight object resolutions has been reached.
     */
    private function resolutionDepthReached(): bool
    {
        return \count($this->resolving) >= static::MAX_RESOLUTION_DEPTH;
    }

    /**
     * Parse a raw object body into a parser token array.
     *
     * The body is parsed in an isolated parser instance, so the indirect references it
     * may contain are not resolved against the outer document.
     *
     * @param string $body Raw object body from an object stream.
     *
     * @return array<int, RawObjectArray>|null
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function parseObjectBody(string $body): ?array
    {
        $parser = new self($this->cfg + ['max_stream_size' => $this->maxStreamSize]);
        $obj = $parser->parseStandaloneObject($body);
        return $obj === [] ? null : $obj;
    }

    /**
     * Decode a single indirect object body in isolation.
     *
     * @param string $body Raw object body (the content between "obj" and "endobj").
     *
     * @return array<int, RawObjectArray> Object data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function parseStandaloneObject(string $body): array
    {
        $this->mrkoff = [];
        $this->xrefdone = [];
        $this->objstmCache = [];
        $this->resolving = [];
        $this->nesting = 0;
        $this->streamDataStart = -1;
        $this->objects = [];
        $this->xref = self::XREF_EMPTY;
        $this->pdfdata = "1 0 obj\n" . $body . "\nendobj\n";

        try {
            return $this->getIndirectObject('1_0', 0, $this->cfg['decode_streams'] ?? true);
        } finally {
            $this->pdfdata = '';
        }
    }

    /**
     * Decode the specified stream.
     *
     * @param array<int, RawObjectArray> $sdic   Stream's dictionary array.
     * @param string                     $stream Stream to decode.
     *
     * @return array{
     *             0: string,
     *             1: array<string>,
     *         } Decoded stream data and remaining filters.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function decodeStream(array $sdic, string $stream): array
    {
        // get stream length and filters
        $slength = \strlen($stream);
        if ($slength <= 0) {
            return ['', []];
        }

        $filters = [];
        $params = [];
        // a dictionary is a flat list of key/value pairs: only even positions are keys,
        // otherwise a value that happens to be the name /Length is taken for a key
        $count = \count($sdic);
        for ($key = 0; $key < $count; $key += 2) {
            $val = $sdic[$key] ?? null;
            if (!\is_array($val) || $val[0] !== '/' || !\is_string($val[1])) {
                continue;
            }

            $nextSdic = $sdic[$key + 1] ?? null;
            if ($val[1] === 'Length' && \is_array($nextSdic)) {
                // get declared stream length
                $this->getDeclaredStreamLength($stream, $slength, $sdic, $key);
            } elseif ($val[1] === 'Filter' && $nextSdic !== null) {
                $filters = $this->getFilters($filters, $sdic, $key);
            } elseif ($val[1] === 'DecodeParms' && $nextSdic !== null) {
                $params = $this->getDecodeParms($sdic, $key);
            }
        }

        return $this->getDecodedStream($filters, $stream, $params);
    }

    /**
     * Truncate the stream to the declared /Length when it is shorter than the payload.
     *
     * @param string                     $stream  Stream to truncate.
     * @param int                        $slength Stream length.
     * @param array<int, RawObjectArray> $sdic    Stream's dictionary array.
     * @param int                        $key     Index of the /Length key.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getDeclaredStreamLength(string &$stream, int &$slength, array $sdic, int $key): void
    {
        // get declared stream length, following an indirect reference
        $declength = $this->resolveLengthValue($sdic[$key + 1] ?? null);
        // ignore a non-positive declaration
        if ($declength !== null && $declength > 0 && $declength < $slength) {
            $stream = \substr($stream, 0, $declength);
            $slength = $declength;
        }
    }

    /**
     * Add the filters declared in the /Filter entry to the filter chain.
     *
     * @param array<string>              $filters Filters collected so far.
     * @param array<int, RawObjectArray> $sdic    Stream's dictionary array.
     * @param int                        $key     Index of the /Filter key.
     *
     * @return array<string> Filter chain.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getFilters(array $filters, array $sdic, int $key): array
    {
        // resolve indirect object
        $nextElem = $sdic[$key + 1] ?? null;
        if (!\is_array($nextElem)) {
            return $filters;
        }

        $elem = $nextElem;
        $objval = $this->getObjectVal($elem);

        switch ($objval[0]) {
            case '/':
                // single filter
                if (\is_string($objval[1])) {
                    $filters[] = $objval[1];
                }

                break;
            case '[':
                if (!\is_array($objval[1])) {
                    break;
                }

                foreach ($objval[1] as $flt) {
                    if ($flt[0] !== '/') {
                        continue;
                    }

                    if (!\is_string($flt[1])) {
                        continue;
                    }

                    $filters[] = $flt[1];
                }

                break;
        }

        return $filters;
    }

    /**
     * Read the /DecodeParms entry of a stream dictionary.
     *
     * @param array<int, RawObjectArray> $sdic Stream's dictionary array.
     * @param int                        $key  Index of the /DecodeParms key.
     *
     * @return array<array-key, mixed> A single decode parameters dictionary,
     *                                 or a list holding one dictionary per filter.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getDecodeParms(array $sdic, int $key): array
    {
        // resolve indirect object
        $nextElem = $sdic[$key + 1] ?? null;
        if (!\is_array($nextElem)) {
            return [];
        }

        $objval = $this->getObjectVal($nextElem);
        if (!\is_array($objval[1])) {
            return [];
        }

        return match ($objval[0]) {
            // single DecodeParms dictionary
            '<<' => $this->buildDecodeParms($objval[1]),
            // array of DecodeParms, one per filter
            '[' => $this->buildDecodeParmsList($objval[1]),
            default => [],
        };
    }

    /**
     * Build the list of DecodeParms dictionaries parallel to the filter chain.
     *
     * @param array<int, RawObjectArray> $parmarr Raw DecodeParms array.
     *
     * @return array<int, array<string, mixed>> One dictionary per filter, empty where none applies.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function buildDecodeParmsList(array $parmarr): array
    {
        $params = [];
        foreach ($parmarr as $parm) {
            // resolve indirect references
            $parmval = $this->getObjectVal($parm);
            $params[] = $parmval[0] === '<<' && \is_array($parmval[1]) ? $this->buildDecodeParms($parmval[1]) : [];
        }

        return $params;
    }

    /**
     * Build a DecodeParms associative array from a raw dictionary.
     *
     * @param array<int, RawObjectArray> $parmdict Raw parameter dictionary.
     *
     * @return array<string, mixed> Decode parameters.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function buildDecodeParms(array $parmdict): array
    {
        $params = [];
        $count = \count($parmdict);
        for ($i = 0; $i < $count; $i += 2) {
            if (!\is_array($parmdict[$i] ?? null)) {
                continue;
            }

            if ($parmdict[$i][0] !== '/' || !\is_string($parmdict[$i][1])) {
                continue;
            }

            $key = $parmdict[$i][1];
            $nextVal = $parmdict[$i + 1] ?? null;
            if (!\is_array($nextVal)) {
                continue;
            }

            $val = $nextVal;
            // resolve indirect references
            $val = $this->getObjectVal($val);

            // extract the value based on type
            $paramVal = $this->extractParamValue($val);
            if ($paramVal !== null) {
                $params[$key] = $paramVal;
            }
        }

        return $params;
    }

    /**
     * Extract a parameter value from a raw object.
     *
     * @param RawObjectArray $val Raw object value.
     *
     * @return int|string|bool|null The extracted value, or null when the type is not supported.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    private function extractParamValue(array $val): int|string|bool|null
    {
        $val1 = $val[1];

        return match ($val[0]) {
            'numeric' => \is_string($val1) ? (int) $val1 : null,
            '/' => \is_string($val1) ? $val1 : null,
            'string' => \is_string($val1) ? $val1 : null,
            // the tokenizer emits booleans as type 'boolean' with 'true'/'false' as value
            'boolean' => $val1 === 'true',
            default => null,
        };
    }

    /**
     * Apply the decoding filters to a stream.
     *
     * @param array<string>           $filters Decoding filters to apply.
     * @param string                  $stream  Stream to decode.
     * @param array<array-key, mixed> $params  DecodeParms: a single dictionary applied to
     *                                         every filter, or one dictionary per filter.
     *
     * @return array{
     *             0: string,
     *             1: array<string>,
     *         } Decoded stream data and remaining filters.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception If filter decoding fails and ignore_filter_errors is false.
     */
    protected function getDecodedStream(array $filters, string $stream, array $params = []): array
    {
        // decode the stream: the filter layer also reverses the predictor declared in DecodeParms
        try {
            return [(new Filter())->decodeAll($filters, $stream, $this->capDecodeParms($params, \count($filters))), []];
        } catch (\Com\Tecnick\Pdf\Filter\Exception $exception) {
            if (!($this->cfg['ignore_filter_errors'] ?? false)) {
                throw new PPException($exception->getMessage());
            }
        }

        return [$stream, $filters];
    }

    /**
     * Force the decoded-size cap into the DecodeParms passed to the filter chain.
     *
     * The cap takes precedence over any MaxOutputSize declared by the document.
     *
     * @param array<array-key, mixed> $params      DecodeParms: a single dictionary applied to
     *                                             every filter, or one dictionary per filter.
     * @param int                     $filterCount Number of filters in the chain.
     *
     * @return array<array-key, mixed> DecodeParms with the cap applied.
     */
    private function capDecodeParms(array $params, int $filterCount): array
    {
        if ($this->maxStreamSize <= 0) {
            return $params;
        }

        $cap = ['MaxOutputSize' => $this->maxStreamSize];
        // a non-empty list is the per-filter form: every position needs its own cap,
        // including the positions the document left out
        if ($params !== [] && \array_is_list($params)) {
            $capped = [];
            $count = \max($filterCount, \count($params));
            for ($idx = 0; $idx < $count; ++$idx) {
                $capped[$idx] = \is_array($params[$idx] ?? null) ? $cap + $params[$idx] : $cap;
            }

            return $capped;
        }

        return $cap + $params;
    }
}
