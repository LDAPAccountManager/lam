<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Psalm;

use function array_merge;
use function array_values;
use function iterator_to_array;
use Psalm\Plugin\PluginEntryPointInterface;
use Psalm\Plugin\RegistrationInterface;
use RecursiveDirectoryIterator;
use RecursiveIteratorIterator;
use RegexIterator;
use SimpleXMLElement;

/**
 * @internal
 */
final class Plugin implements PluginEntryPointInterface
{
    public function __invoke(RegistrationInterface $registration, ?SimpleXMLElement $config = null): void
    {
        foreach ($this->getStubFiles() as $file) {
            $registration->addStubFile($file);
        }
    }

    /**
     * @return array<string>
     */
    private function getStubFiles(): array
    {
        return $this->rsearch(__DIR__ . '/../../stubs/', '/^.*\.phpstub$/');
    }

    /**
     * @return array<string>
     */
    private function rsearch(string $folder, string $pattern): array
    {
        $dir = new RecursiveDirectoryIterator($folder);
        $ite = new RecursiveIteratorIterator($dir);
        /** @psalm-var \FilterIterator<array-key, string[]> $files */
        $files = new RegexIterator($ite, $pattern, RegexIterator::GET_MATCH);

        return array_merge([], ...array_values(iterator_to_array($files)));
    }
}
