<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Session;

use function array_filter;

final class AuthSession implements AuthSessionInterface
{
    /** @var null|string */
    private $state;

    /** @var null|string */
    private $nonce;

    /** @var null|string */
    private $codeVerifier;

    /** @var array<string, mixed> */
    private $customs = [];

    public function getState(): ?string
    {
        return $this->state;
    }

    public function getNonce(): ?string
    {
        return $this->nonce;
    }

    public function getCodeVerifier(): ?string
    {
        return $this->codeVerifier;
    }

    /**
     * @return array<string, mixed>
     */
    public function getCustoms(): array
    {
        return $this->customs;
    }

    public function setState(?string $state): void
    {
        $this->state = $state;
    }

    public function setNonce(?string $nonce): void
    {
        $this->nonce = $nonce;
    }

    public function setCodeVerifier(?string $codeVerifier): void
    {
        $this->codeVerifier = $codeVerifier;
    }

    /**
     * @param array<string, mixed> $customs
     */
    public function setCustoms(array $customs): void
    {
        $this->customs = $customs;
    }

    public static function fromArray(array $array): AuthSessionInterface
    {
        $session = new static();
        $session->setState($array['state'] ?? null);
        $session->setNonce($array['nonce'] ?? null);
        $session->setCodeVerifier($array['code_verifier'] ?? null);
        $session->setCustoms($array['customs'] ?? []);

        return $session;
    }

    public function jsonSerialize(): array
    {
        return array_filter([
            'state' => $this->getState(),
            'nonce' => $this->getNonce(),
            'code_verifier' => $this->getCodeVerifier(),
            'customs' => $this->getCustoms(),
        ]);
    }
}
