<?php

declare(strict_types=1);

namespace Webauthn;

class PublicKeyCredentialDescriptor
{
    final public const CREDENTIAL_TYPE_PUBLIC_KEY = 'public-key';

    final public const AUTHENTICATOR_TRANSPORT_USB = 'usb';

    final public const AUTHENTICATOR_TRANSPORT_NFC = 'nfc';

    final public const AUTHENTICATOR_TRANSPORT_BLE = 'ble';

    final public const AUTHENTICATOR_TRANSPORT_CABLE = 'cable';

    final public const AUTHENTICATOR_TRANSPORT_INTERNAL = 'internal';

    final public const AUTHENTICATOR_TRANSPORTS = [
        self::AUTHENTICATOR_TRANSPORT_USB,
        self::AUTHENTICATOR_TRANSPORT_NFC,
        self::AUTHENTICATOR_TRANSPORT_BLE,
        self::AUTHENTICATOR_TRANSPORT_CABLE,
        self::AUTHENTICATOR_TRANSPORT_INTERNAL,
    ];

    /**
     * @param string[] $transports
     */
    public function __construct(
        public readonly string $type,
        public readonly string $id,
        public readonly array $transports = []
    ) {
    }

    /**
     * @param string[] $transports
     */
    public static function create(string $type, string $id, array $transports = []): self
    {
        return new self($type, $id, $transports);
    }
}
