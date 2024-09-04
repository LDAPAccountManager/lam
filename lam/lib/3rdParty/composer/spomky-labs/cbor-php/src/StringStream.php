<?php

declare(strict_types=1);

namespace CBOR;

use InvalidArgumentException;
use RuntimeException;
use function strlen;

final class StringStream implements Stream
{
    /**
     * @var resource
     */
    private $resource;

    public function __construct(string $data)
    {
        $resource = fopen('php://memory', 'rb+');
        if ($resource === false) {
            throw new RuntimeException('Unable to open the memory');
        }
        $result = fwrite($resource, $data);
        if ($result === false) {
            throw new RuntimeException('Unable to write the memory');
        }
        $result = rewind($resource);
        if ($result === false) {
            throw new RuntimeException('Unable to rewind the memory');
        }
        $this->resource = $resource;
    }

    public static function create(string $data): self
    {
        return new self($data);
    }

    public function read(int $length): string
    {
        if ($length === 0) {
            return '';
        }

        $alreadyRead = 0;
        $data = '';
        while ($alreadyRead < $length) {
            $left = $length - $alreadyRead;
            $sizeToRead = $left < 1024 && $left > 0 ? $left : 1024;
            $newData = fread($this->resource, $sizeToRead);
            $alreadyRead += $sizeToRead;

            if ($newData === false) {
                throw new RuntimeException('Unable to read the memory');
            }
            if (strlen($newData) < $sizeToRead) {
                throw new InvalidArgumentException(sprintf(
                    'Out of range. Expected: %d, read: %d.',
                    $length,
                    strlen($data)
                ));
            }
            $data .= $newData;
        }

        if (strlen($data) !== $length) {
            throw new InvalidArgumentException(sprintf(
                'Out of range. Expected: %d, read: %d.',
                $length,
                strlen($data)
            ));
        }

        return $data;
    }
}
