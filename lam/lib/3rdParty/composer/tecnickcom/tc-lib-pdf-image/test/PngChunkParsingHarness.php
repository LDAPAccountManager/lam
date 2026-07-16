<?php

declare(strict_types=1);

namespace Test;

/**
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 */
class PngChunkParsingHarness extends \Com\Tecnick\Pdf\Image\Import\Png
{
    /**
     * @param ImageBaseData $data
     *
     * @return ImageBaseData
     */
    public function callGetTrnsChunk(array $data, int &$offset, int $len): array
    {
        return $this->getTrnsChunk($data, $offset, $len);
    }

    /**
     * @param ImageBaseData $data
     *
     * @return ImageBaseData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \RangeException
     */
    public function callGetIccpChunk(\Com\Tecnick\File\Byte $byte, array $data, int &$offset, int $len): array
    {
        return $this->getIccpChunk($byte, $data, $offset, $len);
    }
}
