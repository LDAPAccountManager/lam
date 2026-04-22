<?php

declare(strict_types=1);

namespace Webauthn;

use InvalidArgumentException;
use function in_array;

class AuthenticatorSelectionCriteria
{
    final public const AUTHENTICATOR_ATTACHMENT_NO_PREFERENCE = null;

    final public const AUTHENTICATOR_ATTACHMENT_PLATFORM = 'platform';

    final public const AUTHENTICATOR_ATTACHMENT_CROSS_PLATFORM = 'cross-platform';

    final public const AUTHENTICATOR_ATTACHMENTS = [
        self::AUTHENTICATOR_ATTACHMENT_NO_PREFERENCE,
        self::AUTHENTICATOR_ATTACHMENT_PLATFORM,
        self::AUTHENTICATOR_ATTACHMENT_CROSS_PLATFORM,
    ];

    final public const USER_VERIFICATION_REQUIREMENT_REQUIRED = 'required';

    final public const USER_VERIFICATION_REQUIREMENT_PREFERRED = 'preferred';

    final public const USER_VERIFICATION_REQUIREMENT_DISCOURAGED = 'discouraged';

    final public const USER_VERIFICATION_REQUIREMENTS = [
        self::USER_VERIFICATION_REQUIREMENT_REQUIRED,
        self::USER_VERIFICATION_REQUIREMENT_PREFERRED,
        self::USER_VERIFICATION_REQUIREMENT_DISCOURAGED,
    ];

    final public const RESIDENT_KEY_REQUIREMENT_NO_PREFERENCE = null;

    final public const RESIDENT_KEY_REQUIREMENT_REQUIRED = 'required';

    final public const RESIDENT_KEY_REQUIREMENT_PREFERRED = 'preferred';

    final public const RESIDENT_KEY_REQUIREMENT_DISCOURAGED = 'discouraged';

    final public const RESIDENT_KEY_REQUIREMENTS = [
        self::RESIDENT_KEY_REQUIREMENT_NO_PREFERENCE,
        self::RESIDENT_KEY_REQUIREMENT_REQUIRED,
        self::RESIDENT_KEY_REQUIREMENT_PREFERRED,
        self::RESIDENT_KEY_REQUIREMENT_DISCOURAGED,
    ];

    public function __construct(
        public null|string $authenticatorAttachment = null,
        public string $userVerification = self::USER_VERIFICATION_REQUIREMENT_PREFERRED,
        public null|string $residentKey = self::RESIDENT_KEY_REQUIREMENT_NO_PREFERENCE,
    ) {
        in_array($authenticatorAttachment, self::AUTHENTICATOR_ATTACHMENTS, true) || throw new InvalidArgumentException(
            'Invalid authenticator attachment'
        );
        in_array($userVerification, self::USER_VERIFICATION_REQUIREMENTS, true) || throw new InvalidArgumentException(
            'Invalid user verification'
        );
        in_array($residentKey, self::RESIDENT_KEY_REQUIREMENTS, true) || throw new InvalidArgumentException(
            'Invalid resident key'
        );
    }

    public static function create(
        ?string $authenticatorAttachment = null,
        string $userVerification = self::USER_VERIFICATION_REQUIREMENT_PREFERRED,
        null|string $residentKey = self::RESIDENT_KEY_REQUIREMENT_NO_PREFERENCE,
    ): self {
        return new self($authenticatorAttachment, $userVerification, $residentKey);
    }
}
