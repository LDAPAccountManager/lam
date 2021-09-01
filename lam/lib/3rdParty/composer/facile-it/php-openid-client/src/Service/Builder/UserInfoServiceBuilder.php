<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Facile\OpenIDClient\Service\UserInfoService;
use Facile\OpenIDClient\Token\TokenVerifierBuilderInterface;
use Facile\OpenIDClient\Token\UserInfoVerifierBuilder;

final class UserInfoServiceBuilder extends AbstractServiceBuilder
{
    /** @var TokenVerifierBuilderInterface|null */
    private $userInfoVerifierBuilder;

    protected function getUserInfoVerifierBuilder(): TokenVerifierBuilderInterface
    {
        return $this->userInfoVerifierBuilder = $this->userInfoVerifierBuilder ?? new UserInfoVerifierBuilder();
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
