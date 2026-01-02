<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Session;

use Override;

use function array_filter;

final class AuthSession implements AuthSessionInterface
{
    private ?string $state = null;

    private ?string $nonce = null;

    private ?string $codeVerifier = null;

    /** @var array<string, mixed> */
    private array $customs = [];

    #[Override]
    public function getState(): ?string
    {
        return $this->state;
    }

    #[Override]
    public function getNonce(): ?string
    {
        return $this->nonce;
    }

    #[Override]
    public function getCodeVerifier(): ?string
    {
        return $this->codeVerifier;
    }

    /**
     * @return array<string, mixed>
     */
    #[Override]
    public function getCustoms(): array
    {
        return $this->customs;
    }

    #[Override]
    public function setState(?string $state): void
    {
        $this->state = $state;
    }

    #[Override]
    public function setNonce(?string $nonce): void
    {
        $this->nonce = $nonce;
    }

    #[Override]
    public function setCodeVerifier(?string $codeVerifier): void
    {
        $this->codeVerifier = $codeVerifier;
    }

    /**
     * @param array<string, mixed> $customs
     */
    #[Override]
    public function setCustoms(array $customs): void
    {
        $this->customs = $customs;
    }

    #[Override]
    public static function fromArray(array $array): AuthSessionInterface
    {
        $session = new static();
        $session->setState($array['state'] ?? null);
        $session->setNonce($array['nonce'] ?? null);
        $session->setCodeVerifier($array['code_verifier'] ?? null);
        $session->setCustoms($array['customs'] ?? []);

        return $session;
    }

    #[Override]
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
