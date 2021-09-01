<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Middleware;

use Facile\OpenIDClient\Client\ClientInterface;
use Facile\OpenIDClient\Exception\LogicException;
use Facile\OpenIDClient\Service\AuthorizationService;
use Facile\OpenIDClient\Session\AuthSessionInterface;
use Facile\OpenIDClient\Token\TokenSetInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Message\ServerRequestInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;

class CallbackMiddleware implements MiddlewareInterface
{
    /** @var AuthorizationService */
    private $authorizationService;

    /** @var string|null */
    private $redirectUri;

    /** @var null|ClientInterface */
    private $client;

    /** @var null|int */
    private $maxAge;

    public function __construct(
        AuthorizationService $authorizationService,
        ?ClientInterface $client = null,
        ?string $redirectUri = null,
        ?int $maxAge = null
    ) {
        $this->authorizationService = $authorizationService;
        $this->client = $client;
        $this->redirectUri = $redirectUri;
        $this->maxAge = $maxAge;
    }

    public function process(ServerRequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        $client = $this->client ?? $request->getAttribute(ClientInterface::class);
        $authSession = $request->getAttribute(AuthSessionInterface::class);

        if (! $client instanceof ClientInterface) {
            throw new LogicException('No OpenID client provided');
        }

        if (null !== $authSession && ! $authSession instanceof AuthSessionInterface) {
            throw new LogicException('Invalid auth session provided in attribute ' . AuthSessionInterface::class);
        }

        $params = $this->authorizationService->getCallbackParams($request, $client);
        $tokenSet = $this->authorizationService->callback(
            $client,
            $params,
            $this->redirectUri,
            $authSession,
            $this->maxAge
        );

        return $handler->handle($request->withAttribute(TokenSetInterface::class, $tokenSet));
    }
}
