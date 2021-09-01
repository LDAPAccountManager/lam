<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

use function array_key_exists;
use Facile\JoseVerifier\Exception\RuntimeException;
use function is_array;
use function json_decode;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;

/**
 * @psalm-import-type JWKSetObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
class RemoteJwksProvider implements JwksProviderInterface
{
    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    /** @var string */
    private $uri;

    /** @var array<string, string|string[]> */
    private $headers;

    /**
     * @param array<string, string|string[]> $headers
     */
    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory,
        string $uri,
        array $headers = []
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
        $this->uri = $uri;
        $this->headers = $headers;
    }

    /**
     * @param array<string, string|string[]> $headers
     *
     * @return RemoteJwksProvider
     */
    public function withHeaders(array $headers): self
    {
        $new = clone $this;
        $new->headers = $headers;

        return $new;
    }

    /**
     * @inheritDoc
     *
     * @psalm-return JWKSetObject
     */
    public function getJwks(): array
    {
        $request = $this->requestFactory->createRequest('GET', $this->uri);

        foreach ($this->headers as $k => $v) {
            $request = $request->withHeader($k, $v);
        }

        $response = $this->client->sendRequest($request);

        if ($response->getStatusCode() >= 400) {
            throw new RuntimeException('Unable to get the key set.', $response->getStatusCode());
        }

        /** @var mixed $data */
        $data = json_decode((string) $response->getBody(), true);

        if ($this->isJWKSet($data)) {
            /** @var JWKSetObject $data */
            return $data;
        }

        throw new RuntimeException('Invalid key set content');
    }

    /**
     * @param mixed $data
     * @psalm-assert-if-true JWKSetObject $data
     */
    private function isJWKSet($data): bool
    {
        return is_array($data) && array_key_exists('keys', $data) && is_array($data['keys']);
    }

    /**
     * @inheritDoc
     */
    public function reload(): JwksProviderInterface
    {
        return $this;
    }
}
