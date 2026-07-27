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
 * Emits the Document Security Store (DSS) PDF objects for a single signature:
 * the certificate, OCSP, and CRL streams, a VRI entry, and the DSS dictionary.
 * The object number is passed by reference and advanced, and the concatenated
 * object bytes are returned. Stream encryption is delegated to an optional
 * encryptor callable so the emitter does not depend on the host encryption object.
 *
 * The VRI key is the uppercase base-16 SHA-1 digest of the signature Contents
 * bytes, per ISO 32000-2 clause 12.8.4.3.
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
     *
     * @return array{objects: array<int, string>, object_id: int} The emitted object
     *         bodies keyed by object number, and the DSS dictionary object number
     *         (an empty map and 0 when there is no material to emit). The keyed shape
     *         feeds an incremental-update writer directly, one xref entry per object.
     *
     * @throws Exception If the encryptor returns a non-string value.
     */
    public function emit(array $material, string $signatureContents, int &$pon, ?callable $encryptor = null): array
    {
        if ($material['certs'] === [] && $material['ocsp'] === [] && $material['crls'] === []) {
            return ['objects' => [], 'object_id' => 0];
        }

        $objects = [];
        $certIds = $this->emitStreams($material['certs'], $pon, $objects, $encryptor);
        $ocspIds = $this->emitStreams($material['ocsp'], $pon, $objects, $encryptor);
        $crlIds = $this->emitStreams($material['crls'], $pon, $objects, $encryptor);

        $vriKey = \strtoupper(\sha1($signatureContents));
        $vriObjectId = ++$pon;
        $objects[$vriObjectId] = $this->vriObject($vriObjectId, $certIds, $ocspIds, $crlIds);

        $dssObjectId = ++$pon;
        $objects[$dssObjectId] = $this->dssObject($dssObjectId, $vriKey, $vriObjectId, $certIds, $ocspIds, $crlIds);

        return ['objects' => $objects, 'object_id' => $dssObjectId];
    }

    /**
     * Emit one stream object per payload and return the assigned object numbers.
     *
     * @param list<string>       $items
     * @param array<int, string> $objects Emitted object bodies keyed by number; appended to.
     *
     * @return list<int>
     *
     * @throws Exception If the encryptor returns a non-string value.
     */
    private function emitStreams(array $items, int &$pon, array &$objects, ?callable $encryptor): array
    {
        $ids = [];
        foreach ($items as $item) {
            $objectId = ++$pon;
            $ids[] = $objectId;
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
     * @param list<int> $certIds
     * @param list<int> $ocspIds
     * @param list<int> $crlIds
     */
    private function dssObject(
        int $objectId,
        string $vriKey,
        int $vriObjectId,
        array $certIds,
        array $ocspIds,
        array $crlIds,
    ): string {
        $out = $objectId . " 0 obj\n" . '<< /Type /DSS';
        $out .= ' /VRI << /' . $vriKey . ' ' . $vriObjectId . ' 0 R >>';
        $out .= $this->referenceArray('Certs', $certIds);
        $out .= $this->referenceArray('OCSPs', $ocspIds);
        $out .= $this->referenceArray('CRLs', $crlIds);

        return $out . " >>\nendobj\n";
    }

    /**
     * Render a named array of indirect references, or the empty string.
     *
     * @param list<int> $ids
     */
    private function referenceArray(string $name, array $ids): string
    {
        if ($ids === []) {
            return '';
        }

        $refs = '';
        foreach ($ids as $id) {
            $refs .= ' ' . $id . ' 0 R';
        }

        return ' /' . $name . ' [' . $refs . ' ]';
    }
}
