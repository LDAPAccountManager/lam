<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Exception\InvalidTokenException;
use Facile\JoseVerifier\Internal\Checker\AtHashChecker;
use Facile\JoseVerifier\Internal\Checker\CHashChecker;
use Facile\JoseVerifier\Internal\Checker\SHashChecker;
use InvalidArgumentException;
use Jose\Component\Signature\Serializer\CompactSerializer;

final class IdTokenVerifier extends AbstractTokenVerifier implements IdTokenVerifierInterface
{
    protected ?string $accessToken = null;

    protected ?string $code = null;

    protected ?string $state = null;

    public function withAccessToken(?string $accessToken): static
    {
        $new = clone $this;
        $new->accessToken = $accessToken;

        return $new;
    }

    public function withCode(?string $code): static
    {
        $new = clone $this;
        $new->code = $code;

        return $new;
    }

    public function withState(?string $state): static
    {
        $new = clone $this;
        $new->state = $state;

        return $new;
    }

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
            $validator = $validator->withClaim(new AtHashChecker($this->accessToken, $alg ?? ''));
        }

        if (null !== $this->code) {
            $validator = $validator->withClaim(new CHashChecker($this->code, $alg ?? ''));
        }

        if (null !== $this->state) {
            $validator = $validator->withClaim(new SHashChecker($this->state, $alg ?? ''));
        }

        $validator = $validator->withMandatory($requiredClaims);

        return $validator->run();
    }
}
