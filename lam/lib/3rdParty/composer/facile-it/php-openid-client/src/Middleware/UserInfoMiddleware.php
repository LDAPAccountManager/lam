<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Middleware;

use Facile\OpenIDClient\Client\ClientInterface;
use Facile\OpenIDClient\Exception\LogicException;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Service\UserInfoService;
use Facile\OpenIDClient\Token\TokenSetInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Message\ServerRequestInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;

class UserInfoMiddleware implements MiddlewareInterface
{
    public const USERINFO_ATTRIBUTE = self::class;

    /** @var UserInfoService */
    private $userInfoService;

    /** @var null|ClientInterface */
    private $client;

    public function __construct(
        UserInfoService $userInfoService,
        ?ClientInterface $client = null
    ) {
        $this->userInfoService = $userInfoService;
        $this->client = $client;
    }

    public function process(ServerRequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        $tokenSet = $request->getAttribute(TokenSetInterface::class);
        $client = $this->client ?? $request->getAttribute(ClientInterface::class);

        if (! $client instanceof ClientInterface) {
            throw new LogicException('No OpenID client provided');
        }

        if (! $tokenSet instanceof TokenSetInterface) {
            throw new RuntimeException('Unable to get token response attribute');
        }

        $claims = $this->userInfoService->getUserInfo($client, $tokenSet);

        return $handler->handle($request->withAttribute(self::USERINFO_ATTRIBUTE, $claims));
    }
}
