<?php

declare(strict_types=1);

namespace Facile\OpenIDClient;

use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Encryption\Algorithm\ContentEncryption;
use Jose\Component\Encryption\Algorithm\KeyEncryption;
use Jose\Component\Signature\Algorithm;
use Jose\Easy\AlgorithmProvider;

class AlgorithmManagerBuilder
{
    /**
     * @var string[]
     * @psalm-var list<class-string<\Jose\Component\Core\Algorithm>>
     */
    private $algorithmClasses;

    /**
     * @param string[]|null $algorithmClasses
     * @psalm-param null|list<class-string<\Jose\Component\Core\Algorithm>> $algorithmClasses
     */
    public function __construct(?array $algorithmClasses = null)
    {
        $this->algorithmClasses = $algorithmClasses ?? $this->getAlgorithms();
    }

    public function build(): AlgorithmManager
    {
        return new AlgorithmManager((new AlgorithmProvider($this->algorithmClasses))->getAvailableAlgorithms());
    }

    /**
     * @return string[]
     * @psalm-return list<class-string<\Jose\Component\Core\Algorithm>>
     */
    private function getAlgorithms(): array
    {
        return [
            Algorithm\None::class,
            Algorithm\HS256::class,
            Algorithm\HS384::class,
            Algorithm\HS512::class,
            Algorithm\RS256::class,
            Algorithm\RS384::class,
            Algorithm\RS512::class,
            Algorithm\PS256::class,
            Algorithm\PS384::class,
            Algorithm\PS512::class,
            Algorithm\ES256::class,
            Algorithm\ES384::class,
            Algorithm\ES512::class,
            Algorithm\EdDSA::class,
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
