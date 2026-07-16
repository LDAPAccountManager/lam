<?php

declare(strict_types=1);

namespace Test;

/**
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 * @phpstan-import-type ImageRawData from \Com\Tecnick\Pdf\Image\Import
 * @phpstan-type ImageRef array{'iid': int, 'key': string, 'width': int, 'height': int, 'defprint': bool, 'altimgs'?: array<int, int>}
 */
class ImportProtectedMethodsHarness extends \Com\Tecnick\Pdf\Image\Import
{
    /**
     * @param ImageRawData $data
     *
     * @return ImageRawData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function callGetData(array $data, int $width, int $height, int $quality): array
    {
        return $this->getData($data, $width, $height, $quality);
    }

    /**
     * @param ImageBaseData $data
     *
     * @return ImageRawData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function callGetResizedRawData(
        array $data,
        int $width,
        int $height,
        bool $alpha = true,
        int $quality = 100,
    ): array {
        return $this->getResizedRawData($data, $width, $height, $alpha, $quality);
    }

    /**
     * @param ImageBaseData $data
     *
     * @return ImageRawData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function callGetAlphaChannelRawData(array $data): array
    {
        return $this->getAlphaChannelRawData($data);
    }

    /**
     * @param ImageRef $img
     * @param ImageRawData $data
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function callGetOutImage(array &$img, array &$data, string $sub = ''): string
    {
        return $this->getOutImage($img, $data, $sub);
    }

    /**
     * @param ImageRef $img
     * @param ImageRawData $data
     */
    public function callGetOutAltImages(array $img, array &$data, string $sub = ''): string
    {
        return $this->getOutAltImages($img, $data, $sub);
    }

    /**
     * @param ImageRawData $data
     */
    public function callGetOutTransparency(array $data): string
    {
        return $this->getOutTransparency($data);
    }

    public function setPon(int $pon): void
    {
        $this->pon = $pon;
    }

    /**
     * @param ImageRawData $data
     */
    public function setCacheEntry(string $key, array $data): void
    {
        $this->cache[$key] = $data;
    }

    /**
     * @param array<string, int> $xobjdict
     */
    public function setXobjdict(array $xobjdict): void
    {
        $this->xobjdict = $xobjdict;
    }

    /**
     * @param ImageRef $img
     */
    public function setImageEntry(int $iid, array $img): void
    {
        $this->image[$iid] = $img;
    }
}
