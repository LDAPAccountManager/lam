<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Middleware;

use Facile\OpenIDClient\Authorization\AuthRequestInterface;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Message\ServerRequestInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;

final class AuthRequestProviderMiddleware implements MiddlewareInterface
{
    /** @var AuthRequestInterface */
    private $authRequest;

    public function __construct(AuthRequestInterface $authRequest)
    {
        $this->authRequest = $authRequest;
    }

    public function process(ServerRequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        return $handler->handle($request->withAttribute(AuthRequestInterface::class, $this->authRequest));
    }
}
