<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Decrypter;

use function class_exists;
use Facile\JoseVerifier\Exception\InvalidTokenException;
use Facile\JoseVerifier\Exception\LogicException;
use function Facile\JoseVerifier\jose_secret_key;
use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;
use Jose\Component\Checker\AlgorithmChecker;
use Jose\Component\Checker\HeaderCheckerManager;
use Jose\Component\Core\Algorithm;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\JWKSet;
use Jose\Component\Encryption\Algorithm\ContentEncryption;
use Jose\Component\Encryption\Algorithm\KeyEncryption;
use Jose\Component\Encryption\Compression\CompressionMethodManager;
use Jose\Component\Encryption\Compression\Deflate;
use Jose\Component\Encryption\JWEDecrypter;
use Jose\Component\Encryption\JWELoader;
use Jose\Component\Encryption\JWETokenSupport;
use Jose\Component\Encryption\Serializer\CompactSerializer;
use Jose\Component\Encryption\Serializer\JWESerializerManager;
use Jose\Easy\AlgorithmProvider;
use Jose\Easy\ContentEncryptionAlgorithmChecker;
use function preg_match;
use Throwable;

class TokenDecrypter implements TokenDecrypterInterface
{
    /** @var string|null */
    private $expectedAlg;

    /** @var string|null */
    private $expectedEnc;

    /** @var JwksProviderInterface */
    private $jwksProvider;

    /** @var string|null */
    private $clientSecret;

    /** @var Algorithm[] */
    private $algorithms;

    public function withExpectedAlg(?string $expectedAlg): self
    {
        $new = clone $this;
        $new->expectedAlg = $expectedAlg;

        return $new;
    }

    public function withExpectedEnc(?string $expectedEnc): self
    {
        $new = clone $this;
        $new->expectedEnc = $expectedEnc;

        return $new;
    }

    public function withJwksProvider(JwksProviderInterface $jwksProvider): self
    {
        $new = clone $this;
        $new->jwksProvider = $jwksProvider;

        return $new;
    }

    public function withClientSecret(?string $clientSecret): self
    {
        $new = clone $this;
        $new->clientSecret = $clientSecret;

        return $new;
    }

    public function __construct()
    {
        $this->jwksProvider = new MemoryJwksProvider();
        $this->algorithms = (new AlgorithmProvider($this->getAlgorithmMap()))
            ->getAvailableAlgorithms()
        ;
    }

    private function buildJwks(string $jwt): JWKSet
    {
        $jwe = (new CompactSerializer())->unserialize($jwt);
        $header = $jwe->getSharedProtectedHeader();

        $alg = $header['alg'] ?? '';
        $enc = $header['enc'] ?? '';

        if ((bool) preg_match('/^(?:RSA|ECDH)/', $alg)) {
            $jwks = JWKSet::createFromKeyData($this->jwksProvider->getJwks());
        } else {
            $jwk = jose_secret_key($this->clientSecret ?? '', $alg === 'dir' ? $enc : $alg);
            $jwks = new JWKSet([$jwk]);
        }

        return $jwks;
    }

    public function decrypt(string $jwt): ?string
    {
        if (! class_exists(JWELoader::class)) {
            throw new LogicException('In order to decrypt JWT you should install web-token/jwt-encryption package');
        }

        $headerCheckers = [];

        if (null !== $this->expectedAlg) {
            $headerCheckers[] = new AlgorithmChecker([$this->expectedAlg], true);
        }

        if (null !== $this->expectedEnc) {
            $headerCheckers[] = new ContentEncryptionAlgorithmChecker([$this->expectedEnc], true);
        }

        $headerChecker = new HeaderCheckerManager($headerCheckers, [new JWETokenSupport()]);

        $jweLoader = new JWELoader(
            new JWESerializerManager([new CompactSerializer()]),
            new JWEDecrypter(
                new AlgorithmManager($this->algorithms),
                new AlgorithmManager($this->algorithms),
                new CompressionMethodManager([new Deflate()])
            ),
            $headerChecker
        );

        try {
            return $jweLoader->loadAndDecryptWithKeySet(
                $jwt,
                $this->buildJwks($jwt),
                $recipient
            )->getPayload();
        } catch (Throwable $e) {
            throw new InvalidTokenException('Unable to decrypt JWE', 0, $e);
        }
    }

    /**
     * @return string[]
     * @psalm-return list<class-string<Algorithm>>
     */
    protected function getAlgorithmMap(): array
    {
        return [
            KeyEncryption\A128GCMKW::class,
            KeyEncryption\A192GCMKW::class,
            KeyEncryption\A256GCMKW::class,
            KeyEncryption\A128KW::class,
            KeyEncryption\A192KW::class,
            KeyEncryption\A256KW::class,
            KeyEncryption\Dir::class,
            KeyEncryption\ECDHES::class,
            KeyEncryption\ECDHESA128KW::class,
            KeyEncryption\ECDHESA192KW::class,
            KeyEncryption\ECDHESA256KW::class,
            KeyEncryption\PBES2HS256A128KW::class,
            KeyEncryption\PBES2HS384A192KW::class,
            KeyEncryption\PBES2HS512A256KW::class,
            KeyEncryption\RSA15::class,
            KeyEncryption\RSAOAEP::class,
            KeyEncryption\RSAOAEP256::class,
            ContentEncryption\A128GCM::class,
            ContentEncryption\A192GCM::class,
            ContentEncryption\A256GCM::class,
            ContentEncryption\A128CBCHS256::class,
            ContentEncryption\A192CBCHS384::class,
            ContentEncryption\A256CBCHS512::class,
        ];
    }
}
