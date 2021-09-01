<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Exception;

use Psr\Http\Message\ResponseInterface;
use Throwable;

class RemoteException extends RuntimeException
{
    /** @var ResponseInterface */
    private $response;

    public function __construct(ResponseInterface $response, ?string $message = null, Throwable $previous = null)
    {
        parent::__construct(
            $message ?? $response->getReasonPhrase(),
            $response->getStatusCode(),
            $previous
        );
        $this->response = $response;
    }

    public function getResponse(): ResponseInterface
    {
        return $this->response;
    }
}
