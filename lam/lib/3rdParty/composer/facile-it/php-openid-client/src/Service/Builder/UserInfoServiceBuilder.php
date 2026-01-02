<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Facile\OpenIDClient\Service\UserInfoService;
use Facile\OpenIDClient\Token\TokenVerifierBuilderInterface;
use Facile\OpenIDClient\Token\UserInfoVerifierBuilder;

/**
 * @psalm-api
 */
final class UserInfoServiceBuilder extends AbstractServiceBuilder
{
    private ?TokenVerifierBuilderInterface $userInfoVerifierBuilder = null;

    protected function getUserInfoVerifierBuilder(): TokenVerifierBuilderInterface
    {
        return $this->userInfoVerifierBuilder ??= new UserInfoVerifierBuilder();
    }

    public function build(): UserInfoService
    {
        return new UserInfoService(
            $this->getUserInfoVerifierBuilder(),
            $this->getHttpClient(),
            $this->getRequestFactory()
        );
    }
}
