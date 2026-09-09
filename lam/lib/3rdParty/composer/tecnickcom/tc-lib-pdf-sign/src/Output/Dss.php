<?php

declare(strict_types=1);

/**
 * Dss.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Output;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Output\Dss
 *
 * Emits the Document Security Store (DSS) PDF objects for a signature: the
 * certificate, OCSP, and CRL streams, a VRI entry, and the DSS dictionary. The
 * object number is passed by reference and advanced, and the emitted object
 * bodies are returned keyed by number. Stream encryption is delegated to an
 * optional encryptor callable so the emitter does not depend on the host
 * encryption object.
 *
 * The VRI key is the uppercase base-16 SHA-1 digest of the signature Contents
 * bytes, per ISO 32000-2 clause 12.8.4.3.
 *
 * A DSS written by an incremental update replaces the one before it. Pass the
 * previous state as $existing and its VRI entries and object references are merged
 * into the new dictionary instead of being dropped. Material an earlier revision
 * already wrote is referenced again rather than written a second time.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Dss
{
    /**
     * Emit the DSS objects for a signature's validation material.
     *
     * @param array{certs: list<string>, ocsp: list<string>, crls: list<string>} $material
     * @param string        $signatureContents Signature /Contents bytes (hex-decoded,
     *                            including any placeholder padding), hashed for the VRI key.
     * @param int           $pon               Current object number; advanced by reference.
     * @param callable|null $encryptor         Optional fn(string $data, int $objectId): string.
     * @param array{vri: array<string, int>, certs: list<int>, ocsp: list<int>, crls: list<int>,
     *           objects?: array<string, int>}|null $existing
     *                            State returned by a previous emit() for the same document,
     *                            carried forward into the new dictionary.
     *
     * @return array{objects: array<int, string>, object_id: int, state: array{vri: array<string, int>,
     *           certs: list<int>, ocsp: list<int>, crls: list<int>, objects: array<string, int>}}
     *         The emitted object bodies keyed by object number, the DSS dictionary
     *         object number (0 when there is nothing to emit), and the state to pass
     *         back as $existing on the next incremental update.
     *
     * @throws Exception If the object number, the signature contents, the material, or
     *                   the carried state is invalid, or the encryptor returns a
     *                   non-string value.
     */
    public function emit(
        array $material,
        string $signatureContents,
        int &$pon,
        ?callable $encryptor = null,
        ?array $existing = null,
    ): array {
        // Object numbers start at 1 (ISO 32000-1 section 7.5.4), so the first number
        // this emitter assigns is $pon + 1 and $pon itself may be 0 but not negative.
        if ($pon < 0) {
            throw new Exception('Invalid DSS object number: ' . $pon);
        }

        $certs = $this->materialField($material, 'certs');
        $ocspResponses = $this->materialField($material, 'ocsp');
        $crlLists = $this->materialField($material, 'crls');

        // Bounded above as well: the cursor below is stepped with ++, which past
        // PHP_INT_MAX yields a float. One number per material item, plus the VRI and
        // the DSS dictionary.
        $assigned = \count($certs) + \count($ocspResponses) + \count($crlLists) + 2;
        if ($pon > (PHP_INT_MAX - $assigned)) {
            throw new Exception('DSS object number overflow: ' . $pon);
        }

        // The VRI key is the digest of the signature it indexes, so the empty string
        // would yield a well-formed key no signature can match.
        if ($signatureContents === '') {
            throw new Exception('Empty signature contents for the DSS VRI key');
        }

        $carried = $this->checkedState($existing, $pon);

        if ($certs === [] && $ocspResponses === [] && $crlLists === []) {
            return ['objects' => [], 'object_id' => 0, 'state' => $carried];
        }

        $objects = [];

        // Numbers are assigned from a local cursor and written back to $pon only once
        // every object exists, so an encryptor that throws part way through leaves the
        // host's counter untouched.
        $next = $pon;

        // A payload an earlier revision already wrote is referenced again rather than
        // written a second time.
        $written = $carried['objects'];
        $certIds = $this->emitStreams($certs, $next, $objects, $encryptor, $written);
        $ocspIds = $this->emitStreams($ocspResponses, $next, $objects, $encryptor, $written);
        $crlIds = $this->emitStreams($crlLists, $next, $objects, $encryptor, $written);

        $vriKey = \strtoupper(\sha1($signatureContents));
        $vriObjectId = ++$next;
        $objects[$vriObjectId] = $this->vriObject($vriObjectId, $certIds, $ocspIds, $crlIds);

        // The DSS-level arrays are the union over every signature, so a validator
        // resolving an earlier VRI entry still finds the objects it points at.
        $state = [
            'vri' => [...$carried['vri'], $vriKey => $vriObjectId],
            'certs' => \array_values(\array_unique([...$carried['certs'], ...$certIds])),
            'ocsp' => \array_values(\array_unique([...$carried['ocsp'], ...$ocspIds])),
            'crls' => \array_values(\array_unique([...$carried['crls'], ...$crlIds])),
            'objects' => $written,
        ];

        $dssObjectId = ++$next;
        $objects[$dssObjectId] = $this->dssObject($dssObjectId, $state);

        $pon = $next;

        return ['objects' => $objects, 'object_id' => $dssObjectId, 'state' => $state];
    }

    /**
     * Check a carried state before any of it is written into the dictionary.
     *
     * The state crosses whatever the host stores it in between two incremental
     * updates, and its VRI keys and object numbers are interpolated into PDF syntax
     * without escaping.
     *
     * The numbers are bounded above as well as below: this update assigns from
     * $pon + 1 upwards, so a carried number at or beyond that would collide.
     *
     * @param array{vri: array<string, int>, certs: list<int>, ocsp: list<int>, crls: list<int>,
     *           objects?: array<string, int>}|null $existing
     * @param int $pon Current object number; every carried number must be at or below it.
     *
     * @return array{vri: array<string, int>, certs: list<int>, ocsp: list<int>, crls: list<int>,
     *           objects: array<string, int>}
     *
     * @throws Exception If a field is not an array, a VRI key is not a SHA-1 digest, or
     *                   an object number is not an integer, is not positive, or collides
     *                   with one this update assigns.
     */
    private function checkedState(?array $existing, int $pon): array
    {
        // The shape is checked before the values: a field the host's store lost is a
        // missing array rather than an empty one, and would reach a spread as null.
        $vri = [];
        /** @var mixed $objectId */
        foreach ($this->stateField($existing, 'vri') as $key => $objectId) {
            $this->checkVriKey($key);
            $vri[(string) $key] = $this->checkObjectNumber($objectId, $pon);
        }

        $objects = [];
        /** @var mixed $objectId */
        foreach ($this->stateField($existing, 'objects') as $key => $objectId) {
            $objects[(string) $key] = $this->checkObjectNumber($objectId, $pon);
        }

        return [
            'vri' => $vri,
            'certs' => $this->checkedNumbers($this->stateField($existing, 'certs'), $pon),
            'ocsp' => $this->checkedNumbers($this->stateField($existing, 'ocsp'), $pon),
            'crls' => $this->checkedNumbers($this->stateField($existing, 'crls'), $pon),
            'objects' => $objects,
        ];
    }

    /**
     * Check every member of a carried object-number list.
     *
     * @param array<array-key, mixed> $ids
     *
     * @return list<int>
     *
     * @throws Exception If a member is not an integer, is not positive, or collides
     *                   with a number this update assigns.
     */
    private function checkedNumbers(array $ids, int $pon): array
    {
        $checked = [];
        /** @var mixed $id */
        foreach ($ids as $id) {
            $checked[] = $this->checkObjectNumber($id, $pon);
        }

        return $checked;
    }

    /**
     * Read one field of the validation material.
     *
     * Held to the same reading checkedState() gives the carried state, both crossing
     * whatever the host keeps them in between the collection pass and the incremental
     * update that writes them. A missing field would reach count() as null and a
     * payload that is not a string would reach hash(), both as a TypeError.
     *
     * An empty payload is refused: a /Certs or /OCSPs array pointing at a zero-length
     * stream is validation material no validator can use.
     *
     * @param array<array-key, mixed> $material
     *
     * @return list<string>
     *
     * @throws Exception If the field is missing, is not an array, or holds a payload
     *                   that is not a non-empty string.
     */
    private function materialField(array $material, string $key): array
    {
        /** @var mixed $value */
        $value = $material[$key] ?? null;
        if (!\is_array($value)) {
            throw new Exception('Invalid DSS material field: ' . $key);
        }

        $items = [];
        /** @var mixed $item */
        foreach ($value as $item) {
            if (!\is_string($item)) {
                throw new Exception('Invalid Document Security Store payload: ' . \get_debug_type($item));
            }

            if ($item === '') {
                throw new Exception('Empty Document Security Store payload');
            }

            $items[] = $item;
        }

        return $items;
    }

    /**
     * Read one field of a carried state, defaulting an absent one to the empty array.
     *
     * @param array<array-key, mixed>|null $existing
     *
     * @return array<array-key, mixed>
     *
     * @throws Exception If the field is present and is not an array.
     */
    private function stateField(?array $existing, string $key): array
    {
        /** @var mixed $value */
        $value = $existing[$key] ?? [];
        if (!\is_array($value)) {
            throw new Exception('Invalid DSS state field: ' . $key);
        }

        return $value;
    }

    /**
     * Check a carried VRI key.
     *
     * The key is the uppercase base-16 SHA-1 of a signature's Contents, written
     * straight after a name slash. PHP narrows an all-digit array key to an int, so
     * it may not be a string by the time it gets here.
     *
     * @throws Exception If the key is not a SHA-1 digest.
     */
    private function checkVriKey(int|string $key): void
    {
        // \z rather than $, which PCRE matches before a final newline.
        if (\preg_match('/^[0-9A-F]{40}\z/', (string) $key) !== 1) {
            throw new Exception('Invalid DSS VRI key: ' . $key);
        }
    }

    /**
     * Check one carried object number.
     *
     * @param int $pon Current object number; the carried number must not exceed it.
     *
     * @return int The number, now known to be one.
     *
     * @throws Exception If the object number is not an integer, is not one a PDF can
     *                   reference, or is one this update is about to assign.
     */
    private function checkObjectNumber(mixed $objectId, int $pon): int
    {
        // A state that crossed a store which stringifies numbers arrives with the
        // declared shape but not the declared types, and the comparisons below would
        // raise a TypeError under strict types.
        if (!\is_int($objectId)) {
            throw new Exception('Invalid DSS object number in the carried state: ' . \get_debug_type($objectId));
        }

        if ($objectId < 1) {
            throw new Exception('Invalid DSS object number in the carried state: ' . $objectId);
        }

        if ($objectId > $pon) {
            throw new Exception(
                'The carried state holds object number '
                . $objectId
                . ', which this update would assign again from '
                . ($pon + 1),
            );
        }

        return $objectId;
    }

    /**
     * Emit one stream object per payload and return the assigned object numbers.
     *
     * @param list<string>        $items
     * @param array<int, string>  $objects Emitted object bodies keyed by number; appended to.
     * @param array<string, int>  $written Object number of each payload already written by an
     *                                     earlier revision, keyed by digest; appended to.
     *
     * @return list<int>
     *
     * @throws Exception If the encryptor returns a non-string value.
     */
    private function emitStreams(array $items, int &$pon, array &$objects, ?callable $encryptor, array &$written): array
    {
        $ids = [];
        foreach ($items as $item) {
            $fingerprint = \hash('sha256', $item);
            if (isset($written[$fingerprint])) {
                $ids[] = $written[$fingerprint];
                continue;
            }

            $objectId = ++$pon;
            $ids[] = $objectId;
            $written[$fingerprint] = $objectId;
            $stream = $encryptor !== null ? $this->encryptStream($encryptor, $item, $objectId) : $item;
            $objects[$objectId] =
                $objectId
                . " 0 obj\n"
                . '<< /Length '
                . \strlen($stream)
                . " >>\n"
                . "stream\n"
                . $stream
                . "\nendstream\n"
                . "endobj\n";
        }

        return $ids;
    }

    /**
     * Encrypt a stream payload through the caller's encryptor.
     *
     * @throws Exception If the encryptor returns a non-string value.
     */
    private function encryptStream(callable $encryptor, string $data, int $objectId): string
    {
        /** @var mixed $result */
        $result = $encryptor($data, $objectId);
        if (!\is_string($result)) {
            throw new Exception('Invalid stream encryptor result');
        }

        return $result;
    }

    /**
     * Emit the /VRI entry object for one signature.
     *
     * @param list<int> $certIds
     * @param list<int> $ocspIds
     * @param list<int> $crlIds
     */
    private function vriObject(int $objectId, array $certIds, array $ocspIds, array $crlIds): string
    {
        $out = $objectId . " 0 obj\n" . '<< /Type /VRI';
        $out .= $this->referenceArray('Cert', $certIds);
        $out .= $this->referenceArray('OCSP', $ocspIds);
        $out .= $this->referenceArray('CRL', $crlIds);

        return $out . " >>\nendobj\n";
    }

    /**
     * Emit the /DSS dictionary object.
     *
     * @param array{vri: array<string, int>, certs: list<int>, ocsp: list<int>, crls: list<int>,
     *           objects: array<string, int>} $state
     */
    private function dssObject(int $objectId, array $state): string
    {
        $vri = '';
        foreach ($state['vri'] as $key => $vriObjectId) {
            $vri .= ' /' . $key . ' ' . $vriObjectId . ' 0 R';
        }

        $out = $objectId . " 0 obj\n" . '<< /Type /DSS';
        $out .= ' /VRI <<' . $vri . ' >>';
        $out .= $this->referenceArray('Certs', $state['certs']);
        $out .= $this->referenceArray('OCSPs', $state['ocsp']);
        $out .= $this->referenceArray('CRLs', $state['crls']);

        return $out . " >>\nendobj\n";
    }

    /**
     * Render a named array of indirect references, or the empty string.
     *
     * Deduplicated here: emitStreams() collapses two equal payloads onto one object
     * but still answers one number per input item.
     *
     * @param list<int> $ids
     */
    private function referenceArray(string $name, array $ids): string
    {
        $refs = '';
        foreach (\array_unique($ids) as $id) {
            $refs .= ' ' . $id . ' 0 R';
        }

        if ($refs === '') {
            return '';
        }

        return ' /' . $name . ' [' . $refs . ' ]';
    }
}
