<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Checker\AtHashChecker;
use Facile\JoseVerifier\Checker\CHashChecker;
use Facile\JoseVerifier\Checker\SHashChecker;
use Facile\JoseVerifier\Exception\InvalidTokenException;
use InvalidArgumentException;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Throwable;

/**
 * @psalm-import-type JWTHeaderObject from Psalm\PsalmTypes
 */
final class IdTokenVerifier extends AbstractTokenVerifier implements IdTokenVerifierInterface
{
    /** @var string|null */
    protected $accessToken;

    /** @var string|null */
    protected $code;

    /** @var string|null */
    protected $state;

    /**
     * @return $this
     */
    public function withAccessToken(?string $accessToken): self
    {
        $new = clone $this;
        $new->accessToken = $accessToken;

        return $new;
    }

    /**
     * @return $this
     */
    public function withCode(?string $code): self
    {
        $new = clone $this;
        $new->code = $code;

        return $new;
    }

    /**
     * @return $this
     */
    public function withState(?string $state): self
    {
        $new = clone $this;
        $new->state = $state;

        return $new;
    }

    /**
     * @inheritDoc
     *
     * @psalm-suppress MixedReturnTypeCoercion
     */
    public function verify(string $jwt): array
    {
        $jwt = $this->decrypt($jwt);

        try {
            $jws = (new CompactSerializer())->unserialize($jwt);
        } catch (InvalidArgumentException $e) {
            throw new InvalidTokenException('Invalid JWT provided', 0, $e);
        }

        $header = $jws->getSignature(0)->getProtectedHeader();

        $validator = $this->create($jwt);

        $requiredClaims = ['iss', 'sub', 'aud', 'exp', 'iat'];
        $alg = $header['alg'] ?? null;

        if (null !== $this->accessToken) {
            $validator = $validator->claim(new AtHashChecker($this->accessToken, $alg ?: ''));
        }

        if (null !== $this->code) {
            $validator = $validator->claim(new CHashChecker($this->code, $alg ?: ''));
        }

        if (null !== $this->state) {
            $validator = $validator->claim(new SHashChecker($this->state, $alg ?: ''));
        }

        $validator = $validator->mandatory($requiredClaims);

        try {
            return $validator->run();
        } catch (Throwable $e) {
            throw $this->processException($e);
        }
    }
}
