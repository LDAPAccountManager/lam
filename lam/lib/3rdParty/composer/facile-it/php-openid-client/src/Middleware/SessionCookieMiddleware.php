<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Middleware;

use function bin2hex;
use function class_exists;
use Dflydev\FigCookies\Cookies;
use Dflydev\FigCookies\FigResponseCookies;
use Dflydev\FigCookies\Modifier\SameSite;
use Dflydev\FigCookies\SetCookie;
use Facile\OpenIDClient\Exception\LogicException;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Session\AuthSession;
use Facile\OpenIDClient\Session\AuthSessionInterface;
use function is_array;
use function json_decode;
use function json_encode;
use Psr\Http\Message\ResponseInterface;
use Psr\Http\Message\ServerRequestInterface;
use Psr\Http\Server\MiddlewareInterface;
use Psr\Http\Server\RequestHandlerInterface;
use Psr\SimpleCache\CacheInterface;
use function random_bytes;

/**
 * @psalm-import-type AuthSessionType from AuthSessionInterface
 */
class SessionCookieMiddleware implements MiddlewareInterface
{
    public const SESSION_ATTRIBUTE = AuthSessionInterface::class;

    /** @var string */
    private $cookieName;

    /** @var int */
    private $ttl;

    /** @var CacheInterface */
    private $cache;

    public function __construct(CacheInterface $cache, string $cookieName = 'openid', int $ttl = 300)
    {
        $this->cache = $cache;
        $this->cookieName = $cookieName;
        $this->ttl = $ttl;
    }

    public function process(ServerRequestInterface $request, RequestHandlerInterface $handler): ResponseInterface
    {
        if (! class_exists(Cookies::class)) {
            throw new LogicException('To use the SessionCookieMiddleware you should install dflydev/fig-cookies package');
        }

        $cookies = Cookies::fromRequest($request);
        $sessionCookie = $cookies->get($this->cookieName);

        $sessionId = null !== $sessionCookie ? $sessionCookie->getValue() : null;
        /** @var string|null $sessionValue */
        $sessionValue = null !== $sessionId ? $this->cache->get($sessionId) : null;
        /** @var false|AuthSessionType $data */
        $data = null !== $sessionValue ? json_decode($sessionValue, true) : [];

        if (! is_array($data)) {
            $data = [];
        }

        $authSession = AuthSession::fromArray($data);

        $response = $handler->handle($request->withAttribute(self::SESSION_ATTRIBUTE, $authSession));

        $sessionId = $sessionId ?? bin2hex(random_bytes(32));

        /** @var string $sessionValue */
        $sessionValue = json_encode($authSession);

        if (false === $this->cache->set($sessionId, $sessionValue, $this->ttl)) {
            throw new RuntimeException('Unable to save session');
        }

        $sessionCookie = SetCookie::create($this->cookieName)
            ->withValue($sessionId)
            ->withMaxAge($this->ttl)
            ->withHttpOnly()
            ->withPath('/')
            ->withSameSite(SameSite::strict());

        $response = FigResponseCookies::set($response, $sessionCookie);

        return $response;
    }
}
