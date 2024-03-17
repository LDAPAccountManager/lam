<?php

/**
 * League.Uri (https://uri.thephpleague.com)
 *
 * (c) Ignace Nyamagana Butera <nyamsprod@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

declare(strict_types=1);

namespace League\Uri;

use League\Uri\Contracts\UriInterface;
use Psr\Http\Message\UriInterface as Psr7UriInterface;
use function array_pop;
use function array_reduce;
use function count;
use function end;
use function explode;
use function implode;
use function in_array;
use function str_repeat;
use function strpos;
use function substr;

final class UriResolver
{
    /**
     * @var array<string,int>
     */
    const DOT_SEGMENTS = ['.' => 1, '..' => 1];

    /**
     * @codeCoverageIgnore
     */
    private function __construct()
    {
    }

    /**
     * Resolves an URI against a base URI using RFC3986 rules.
     *
     * If the first argument is a UriInterface the method returns a UriInterface object
     * If the first argument is a Psr7UriInterface the method returns a Psr7UriInterface object
     */
    public static function resolve(Psr7UriInterface|UriInterface $uri, Psr7UriInterface|UriInterface $base_uri): Psr7UriInterface|UriInterface
    {
        $null = $uri instanceof Psr7UriInterface ? '' : null;

        if ($null !== $uri->getScheme()) {
            return $uri
                ->withPath(self::removeDotSegments($uri->getPath()));
        }

        if ($null !== $uri->getAuthority()) {
            return $uri
                ->withScheme($base_uri->getScheme())
                ->withPath(self::removeDotSegments($uri->getPath()));
        }

        $user = $null;
        $pass = null;
        $userInfo = $base_uri->getUserInfo();
        if (null !== $userInfo) {
            [$user, $pass] = explode(':', $userInfo, 2) + [1 => null];
        }

        [$uri_path, $uri_query] = self::resolvePathAndQuery($uri, $base_uri);

        return $uri
            ->withPath(self::removeDotSegments($uri_path))
            ->withQuery($uri_query)
            ->withHost($base_uri->getHost())
            ->withPort($base_uri->getPort())
            ->withUserInfo((string) $user, $pass)
            ->withScheme($base_uri->getScheme())
        ;
    }

    /**
     * Remove dot segments from the URI path.
     */
    private static function removeDotSegments(string $path): string
    {
        if (!str_contains($path, '.')) {
            return $path;
        }

        $old_segments = explode('/', $path);
        $new_path = implode('/', array_reduce($old_segments, UriResolver::reducer(...), []));
        if (isset(self::DOT_SEGMENTS[end($old_segments)])) {
            $new_path .= '/';
        }

        // @codeCoverageIgnoreStart
        // added because some PSR-7 implementations do not respect RFC3986
        if (str_starts_with($path, '/') && !str_starts_with($new_path, '/')) {
            return '/'.$new_path;
        }
        // @codeCoverageIgnoreEnd

        return $new_path;
    }

    /**
     * Remove dot segments.
     *
     * @return array<int, string>
     */
    private static function reducer(array $carry, string $segment): array
    {
        if ('..' === $segment) {
            array_pop($carry);

            return $carry;
        }

        if (!isset(self::DOT_SEGMENTS[$segment])) {
            $carry[] = $segment;
        }

        return $carry;
    }

    /**
     * Resolves an URI path and query component.
     *
     * @return array{0:string, 1:string|null}
     */
    private static function resolvePathAndQuery(
        Psr7UriInterface|UriInterface $uri,
        Psr7UriInterface|UriInterface $base_uri
    ): array {
        $target_path = $uri->getPath();
        $target_query = $uri->getQuery();
        $null = $uri instanceof Psr7UriInterface ? '' : null;
        $baseNull = $base_uri instanceof Psr7UriInterface ? '' : null;

        if (str_starts_with($target_path, '/')) {
            return [$target_path, $target_query];
        }

        if ('' === $target_path) {
            if ($null === $target_query) {
                $target_query = $base_uri->getQuery();
            }

            $target_path = $base_uri->getPath();
            //@codeCoverageIgnoreStart
            //because some PSR-7 Uri implementations allow this RFC3986 forbidden construction
            if ($baseNull !== $base_uri->getAuthority() && !str_starts_with($target_path, '/')) {
                $target_path = '/'.$target_path;
            }
            //@codeCoverageIgnoreEnd

            return [$target_path, $target_query];
        }

        $base_path = $base_uri->getPath();
        if ($baseNull !== $base_uri->getAuthority() && '' === $base_path) {
            $target_path = '/'.$target_path;
        }

        if ('' !== $base_path) {
            $segments = explode('/', $base_path);
            array_pop($segments);
            if ([] !== $segments) {
                $target_path = implode('/', $segments).'/'.$target_path;
            }
        }

        return [$target_path, $target_query];
    }

    /**
     * Relativizes an URI according to a base URI.
     *
     * This method MUST retain the state of the submitted URI instance, and return
     * an URI instance of the same type that contains the applied modifications.
     *
     * This method MUST be transparent when dealing with error and exceptions.
     * It MUST not alter of silence them apart from validating its own parameters.
     */
    public static function relativize(
        Psr7UriInterface|UriInterface $uri,
        Psr7UriInterface|UriInterface $base_uri
    ): Psr7UriInterface|UriInterface {
        $uri = self::formatHost($uri);
        $base_uri = self::formatHost($base_uri);
        if (!self::isRelativizable($uri, $base_uri)) {
            return $uri;
        }

        $null = $uri instanceof Psr7UriInterface ? '' : null;
        $uri = $uri->withScheme($null)->withPort(null)->withUserInfo($null)->withHost($null);
        $target_path = $uri->getPath();
        if ($target_path !== $base_uri->getPath()) {
            return $uri->withPath(self::relativizePath($target_path, $base_uri->getPath()));
        }

        if (self::componentEquals('query', $uri, $base_uri)) {
            return $uri->withPath('')->withQuery($null);
        }

        if ($null === $uri->getQuery()) {
            return $uri->withPath(self::formatPathWithEmptyBaseQuery($target_path));
        }

        return $uri->withPath('');
    }

    /**
     * Tells whether the component value from both URI object equals.
     */
    private static function componentEquals(
        string                        $property,
        Psr7UriInterface|UriInterface $uri,
        Psr7UriInterface|UriInterface $base_uri
    ): bool {
        return self::getComponent($property, $uri) === self::getComponent($property, $base_uri);
    }

    /**
     * Returns the component value from the submitted URI object.
     */
    private static function getComponent(string $property, Psr7UriInterface|UriInterface $uri): ?string
    {
        $component = match ($property) {
            'query' => $uri->getQuery(),
            'authority' => $uri->getAuthority(),
            default => $uri->getScheme(), //scheme
        };

        if ($uri instanceof Psr7UriInterface && '' === $component) {
            return null;
        }

        return $component;
    }

    /**
     * Filter the URI object.
     */
    private static function formatHost(Psr7UriInterface|UriInterface $uri): Psr7UriInterface|UriInterface
    {
        if (!$uri instanceof Psr7UriInterface) {
            return $uri;
        }

        $host = $uri->getHost();
        if ('' === $host) {
            return $uri;
        }

        return $uri->withHost((string) Uri::createFromComponents(['host' => $host])->getHost());
    }

    /**
     * Tells whether the submitted URI object can be relativize.
     */
    private static function isRelativizable(
        Psr7UriInterface|UriInterface $uri,
        Psr7UriInterface|UriInterface $base_uri
    ): bool {
        return !UriInfo::isRelativePath($uri)
            && self::componentEquals('scheme', $uri, $base_uri)
            && self::componentEquals('authority', $uri, $base_uri);
    }

    /**
     * Relatives the URI for an authority-less target URI.
     */
    private static function relativizePath(string $path, string $basepath): string
    {
        $base_segments = self::getSegments($basepath);
        $target_segments = self::getSegments($path);
        $target_basename = array_pop($target_segments);
        array_pop($base_segments);
        foreach ($base_segments as $offset => $segment) {
            if (!isset($target_segments[$offset]) || $segment !== $target_segments[$offset]) {
                break;
            }
            unset($base_segments[$offset], $target_segments[$offset]);
        }
        $target_segments[] = $target_basename;

        return self::formatPath(
            str_repeat('../', count($base_segments)).implode('/', $target_segments),
            $basepath
        );
    }

    /**
     * returns the path segments.
     *
     * @return string[]
     */
    private static function getSegments(string $path): array
    {
        if ('' !== $path && '/' === $path[0]) {
            $path = substr($path, 1);
        }

        return explode('/', $path);
    }

    /**
     * Formatting the path to keep a valid URI.
     */
    private static function formatPath(string $path, string $basepath): string
    {
        if ('' === $path) {
            return in_array($basepath, ['', '/'], true) ? $basepath : './';
        }

        if (false === ($colon_pos = strpos($path, ':'))) {
            return $path;
        }

        $slash_pos = strpos($path, '/');
        if (false === $slash_pos || $colon_pos < $slash_pos) {
            return "./$path";
        }

        return $path;
    }

    /**
     * Formatting the path to keep a resolvable URI.
     */
    private static function formatPathWithEmptyBaseQuery(string $path): string
    {
        $target_segments = self::getSegments($path);
        /** @var string $basename */
        $basename = end($target_segments);

        return '' === $basename ? './' : $basename;
    }
}
