<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Authorization;

use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Override;

use function array_diff;
use function array_diff_key;
use function array_flip;
use function array_keys;
use function array_merge;
use function count;
use function implode;

/**
 * @psalm-type AuthRequestParams = array{client_id: string, redirect_uri: string, scope: string, response_type: string, response_mode: string, state?: string, nonce?: string, display?: string, prompt?: string, max_age?: int, ui_locales?: string, id_token_hint?: string, login_hint?: string, acr_values?: string, request?: string, code_challenge?: string, code_challenge_method?: string}
 */
final class AuthRequest implements AuthRequestInterface
{
    /**
     * @var array<string, mixed>
     *
     * @psalm-var AuthRequestParams
     */
    private array $params;

    /** @var string[] */
    private static array $requiredKeys = [
        'client_id',
        'redirect_uri',
    ];

    /**
     * @param array<string, mixed> $params
     */
    public function __construct(
        string $clientId,
        string $redirectUri,
        array $params = []
    ) {
        $defaults = [
            'scope' => 'openid',
            'response_type' => 'code',
            'response_mode' => 'query',
        ];
        /** @var AuthRequestParams $merged */
        $merged = array_merge($defaults, $params);

        $merged['client_id'] = $clientId;
        $merged['redirect_uri'] = $redirectUri;

        $this->params = $merged;
    }

    /**
     * @psalm-api
     *
     * @param array<string, mixed> $params
     *
     * @psalm-param array{client_id: string, redirect_uri: string} $params
     *
     * @return static
     */
    public static function fromParams(array $params): self
    {
        $missingKeys = array_diff(self::$requiredKeys, array_keys($params));
        if (0 !== count($missingKeys)) {
            throw new InvalidArgumentException(implode(', ', $missingKeys) . ' keys not provided');
        }

        return new static(
            $params['client_id'],
            $params['redirect_uri'],
            $params
        );
    }

    /**
     * OpenID Connect requests MUST contain the openid scope value.
     */
    #[Override]
    public function getScope(): string
    {
        return $this->params['scope'];
    }

    /**
     * OAuth 2.0 Response Type value that determines the authorization processing flow to be used,
     * including what parameters are returned from the endpoints used. When using the Authorization Code Flow,
     * this value is code.
     */
    #[Override]
    public function getResponseType(): string
    {
        return $this->params['response_type'];
    }

    /**
     * OAuth 2.0 Client Identifier valid at the Authorization Server.
     */
    #[Override]
    public function getClientId(): string
    {
        return $this->params['client_id'];
    }

    /**
     * Redirection URI to which the response will be sent.
     */
    #[Override]
    public function getRedirectUri(): string
    {
        return $this->params['redirect_uri'];
    }

    /**
     * Opaque value used to maintain state between the request and the callback.
     */
    #[Override]
    public function getState(): ?string
    {
        return $this->params['state'] ?? null;
    }

    /**
     * Informs the Authorization Server of the mechanism to be used for returning parameters from
     * the Authorization Endpoint.
     */
    #[Override]
    public function getResponseMode(): ?string
    {
        return $this->params['response_mode'] ?? null;
    }

    /**
     * String value used to associate a Client session with an ID Token, and to mitigate replay attacks.
     */
    #[Override]
    public function getNonce(): ?string
    {
        return $this->params['nonce'] ?? null;
    }

    /**
     * ASCII string value that specifies how the Authorization Server displays the authentication and consent
     * user interface pages to the End-User.
     *
     * The defined values are:
     * - page
     * - popup
     * - touch
     * - wrap
     */
    #[Override]
    public function getDisplay(): ?string
    {
        return $this->params['display'] ?? null;
    }

    /**
     * Case sensitive list of ASCII string values that specifies whether the Authorization Server prompts
     * the End-User for reauthentication and consent.
     *
     * The defined values are:
     * - none
     * - login
     * - consent
     * - select_account
     */
    #[Override]
    public function getPrompt(): ?string
    {
        return $this->params['prompt'] ?? null;
    }

    /**
     * Maximum Authentication Age. Specifies the allowable elapsed time in seconds since the last time the End-User
     * was actively authenticated by the OP.
     */
    #[Override]
    public function getMaxAge(): ?int
    {
        return $this->params['max_age'] ?? null;
    }

    /**
     * End-User's preferred languages and scripts for the user interface, represented as a space-separated list
     * of BCP47 [RFC5646] language tag values, ordered by preference.
     */
    #[Override]
    public function getUiLocales(): ?string
    {
        return $this->params['ui_locales'] ?? null;
    }

    /**
     * ID Token previously issued by the Authorization Server being passed as a hint about the End-User's current or
     * past authenticated session with the Client.
     */
    #[Override]
    public function getIdTokenHint(): ?string
    {
        return $this->params['id_token_hint'] ?? null;
    }

    /**
     * Hint to the Authorization Server about the login identifier the End-User might use to log in (if necessary).
     */
    #[Override]
    public function getLoginHint(): ?string
    {
        return $this->params['login_hint'] ?? null;
    }

    /**
     * Requested Authentication Context Class Reference values.
     */
    #[Override]
    public function getAcrValues(): ?string
    {
        return $this->params['acr_values'] ?? null;
    }

    #[Override]
    public function getRequest(): ?string
    {
        return $this->params['request'] ?? null;
    }

    #[Override]
    public function getCodeChallenge(): ?string
    {
        return $this->params['code_challenge'] ?? null;
    }

    #[Override]
    public function getCodeChallengeMethod(): ?string
    {
        return $this->params['code_challenge_method'] ?? null;
    }

    /**
     * Add other params and return a new instance.
     *
     * @param array<string, mixed> $params
     */
    #[Override]
    public function withParams(array $params): AuthRequestInterface
    {
        $instance = clone $this;
        /** @var AuthRequestParams $params */
        $params = array_merge($instance->params, $params);

        $instance->params = $params;

        if (0 === count(array_diff_key($instance->params, array_flip(self::$requiredKeys)))) {
            throw new InvalidArgumentException(implode(', ', self::$requiredKeys) . ' should be provided');
        }

        return $instance;
    }

    /**
     * Create params ready to use.
     *
     * @return array<string, mixed>
     */
    #[Override]
    public function createParams(): array
    {
        return $this->params;
    }

    /**
     * @return array<string, mixed>
     */
    #[Override]
    public function jsonSerialize(): array
    {
        return $this->createParams();
    }
}
