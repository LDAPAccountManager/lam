<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Exception;

use function array_key_exists;
use function is_array;
use function json_decode;
use JsonException;
use JsonSerializable;
use Psr\Http\Message\ResponseInterface;
use function sprintf;
use Throwable;

/**
 * @psalm-type OAuth2ErrorType = array{}&array{
 *     error: string,
 *     error_description?: string,
 *     error_uri?: string,
 *     state?: string,
 * }
 */
class OAuth2Exception extends RuntimeException implements JsonSerializable
{
    /** @var string */
    private $error;

    /** @var null|string */
    private $description;

    /** @var null|string */
    private $errorUri;

    /**
     * @psalm-param array<string, mixed> $data
     *
     * @psalm-assert-if-true OAuth2ErrorType $data
     */
    public static function isOAuth2Error(array $data): bool
    {
        return array_key_exists('error', $data);
    }

    /**
     * @throws RemoteException
     */
    public static function fromResponse(ResponseInterface $response, Throwable $previous = null): self
    {
        try {
            /** @psalm-var false|array{error?: string, error_description?: string, error_uri?: string}  $data */
            $data = json_decode((string) $response->getBody(), true, 512, JSON_THROW_ON_ERROR);
        } catch (JsonException $e) {
            throw new RemoteException($response, $response->getReasonPhrase(), $previous);
        }

        if (! is_array($data) || ! isset($data['error'])) {
            throw new RemoteException($response, $response->getReasonPhrase(), $previous);
        }

        return self::fromParameters($data);
    }

    /**
     * @param array<string, mixed> $params
     *
     * @psalm-param OAuth2ErrorType $params
     */
    public static function fromParameters(array $params, Throwable $previous = null): self
    {
        if (! static::isOAuth2Error($params)) {
            throw new InvalidArgumentException('Invalid OAuth2 exception', 0, $previous);
        }

        return new self(
            $params['error'],
            $params['error_description'] ?? null,
            $params['error_uri'] ?? null,
            0,
            $previous
        );
    }

    public function __construct(
        string $error,
        ?string $description = null,
        ?string $errorUri = null,
        int $code = 0,
        Throwable $previous = null
    ) {
        $message = $error;
        if (null !== $description) {
            $message = sprintf('%s (%s)', $description, $error);
        }

        parent::__construct($message, $code, $previous);
        $this->error = $error;
        $this->description = $description;
        $this->errorUri = $errorUri;
    }

    public function getError(): string
    {
        return $this->error;
    }

    public function getDescription(): ?string
    {
        return $this->description;
    }

    public function getErrorUri(): ?string
    {
        return $this->errorUri;
    }

    /**
     * @return array<string, mixed>
     *
     * @psalm-return array{error: string, error_description?: string, error_uri?: string}
     */
    public function jsonSerialize(): array
    {
        $data = [
            'error' => $this->getError(),
        ];

        $description = $this->getDescription();
        if (null !== $description) {
            $data['error_description'] = $description;
        }

        $errorUri = $this->getErrorUri();
        if (null !== $errorUri) {
            $data['error_uri'] = $errorUri;
        }

        return $data;
    }
}
