#!/usr/bin/env php
<?php
/**
 * bulk_convert.php
 *
 * @since       2015-11-30
 * @category    Library
 * @package     PdfFont
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 *
 * Command-line tool to convert fonts data for the tc-lib-pdf-font library in bulk.
 * Each family is written with a LICENSE file built from the NOTICE and LICENSE files of its source.
 * A family whose licence terms are missing or incomplete is not converted.
 *
 * Exit codes:
 *     0     success
 *     1     not run from the command line
 *     2     output directory not writable
 *     3     input directory missing
 *     4     font conversion errors
 *     5     Composer autoloader not found
 *     6     input directory not readable
 *     9     licence terms missing or incomplete for at least one family
 */

if (\php_sapi_name() != 'cli') {
    \fwrite(STDERR, 'You need to run this command from console.'."\n");
    exit(1);
}

/**
 * Display help guide for this command.
 */
function showHelp()
{
    $help = <<<EOD

bulk_convert - Command-line tool to convert fonts data for the tc-lib-pdf-font library.

Usage:
    bulk_convert.php [ options ]

Options:

    -i, --indir
        Input directory containing one sub directory per font family.
        Default: the tc-font-mirror package installed in util/vendor.

    -o, --outpath
        Output path for generated font files (must be writeable by the
        web server). Leave empty for default font folder.

    -h, --help
        Display this help and exit.

EOD;
    \fwrite(STDOUT, $help);
    exit(0);
}

// initialize the array of options
$options = array(
    'indir' => __DIR__.'/vendor/tecnickcom/tc-font-mirror/',
    'outpath' => \dirname(__DIR__).'/target/fonts/',
);

// short input options
$sopt = 'i:o:';

// long input options
$lopt = array('indir:', 'outpath:');

// parse input options
$inopt = \getopt($sopt, $lopt);

// import options (with some sanitization)
foreach ($inopt as $opt => $val) {
    // getopt() hands over an array when the same option is repeated: the last occurrence wins
    if (\is_array($val)) {
        $val = \end($val);
    }

    switch ($opt) {
        case 'i':
        case 'indir':
            $options['indir'] = \rtrim($val, '/').'/';
            break;
        case 'o':
        case 'outpath':
            // a missing directory keeps its raw value and is normalized below, once created
            $resolved = \realpath($val);
            $options['outpath'] = ($resolved !== false) ? $resolved : $val;
            if (\substr($options['outpath'], -1) != '/') {
                $options['outpath'] .= '/';
            }
            break;
        case 'h':
        case 'help':
        default:
            showHelp();
            break;
    }
}

// check input values
if (!is_dir($options['outpath'])) {
    \mkdir($options['outpath'], 0755, true);
}
if (!is_dir($options['outpath']) || !\is_writable($options['outpath'])) {
    \fwrite(STDERR, 'ERROR: Can\'t write to '.$options['outpath']."\n\n");
    exit(2);
}

$ttfdir = $options['indir'];
if (!is_dir($ttfdir)) {
    \fwrite(STDERR, 'ERROR: The '.$ttfdir.' directory is missing, please execute \'php util/build_fonts.php\' before this command.'."\n\n");
    exit(3);
}

\fwrite(STDOUT, "\n".'>>> Converting fonts:'."\n".'*** Output directory set to '.$options['outpath']."\n");

// count conversions
$convert_errors = 0;
$convert_success = 0;
$license_errors = 0;

// locate the Composer autoloader of a standalone checkout or of a host project
$autoloadFound = false;
foreach (
    array(
        \dirname(__DIR__) . '/vendor', // standalone repository checkout
        \dirname(__DIR__, 3),          // installed under <vendor-dir>/tecnickcom/tc-lib-pdf-font
    ) as $vendorDir
) {
    if (\is_file($vendorDir . '/autoload.php') && \is_file($vendorDir . '/composer/installed.json')) {
        require_once $vendorDir . '/autoload.php';
        $autoloadFound = true;
        break;
    }
}

if (!$autoloadFound) {
    \fwrite(STDERR, 'ERROR: Composer autoloader not found. Run "composer install" first.'."\n\n");
    exit(5);
}

// scandir() returns false for a directory it cannot read
$entries = \scandir($ttfdir);
if ($entries === false) {
    \fwrite(STDERR, 'ERROR: Unable to read the '.$ttfdir.' directory.'."\n\n");
    exit(6);
}

$fontdir = \array_diff($entries, array('.', '..', '.git'));

// phrase shared by every font family distributed with the GNU font embedding exception
$font_exception = 'As a special exception, if you create a document which uses this font';

/*
 * Font family metadata:
 *   url     source of the font files;
 *   spdx    SPDX licence expression;
 *   markers phrases the licence terms must contain (whitespace-insensitive);
 *   note    optional README line.
 */
$families = array(
    'cid0' => array(
        'url' => 'http://unifoundry.com/unifont.html',
        'spdx' => 'GPL-2.0-or-later WITH Font-exception-2.0',
        'markers' => array($font_exception),
    ),
    'core' => array(
        'url' => 'https://github.com/tecnickcom/tc-font-core14-afms',
        'spdx' => 'APAFML',
        'markers' => array('Adobe Systems', 'the AFM files are not distributed without this file'),
        'note' => 'The files contain font metrics converted from the Adobe AFM files, not the AFM files themselves.',
    ),
    'dejavu' => array(
        'url' => 'http://sourceforge.net/projects/dejavu/files/dejavu/2.35/dejavu-fonts-ttf-2.35.zip',
        'spdx' => 'Bitstream-Vera AND LicenseRef-Arev',
        'markers' => array('Bitstream Vera Fonts Copyright', 'Arev Fonts Copyright'),
    ),
    'freefont' => array(
        'url' => 'https://ftp.gnu.org/gnu/freefont/freefont-ttf-20120503.zip',
        'spdx' => 'GPL-3.0-or-later WITH Font-exception-2.0',
        'markers' => array($font_exception, 'GNU GENERAL PUBLIC LICENSE Version 3'),
    ),
    'pdfa' => array(
        'url' => 'https://github.com/tecnickcom/tc-font-pdfa',
        'spdx' => 'GPL-3.0-or-later WITH Font-exception-2.0',
        'markers' => array(
            'The GNU FreeFont font embedding exception is extended to these fonts',
            $font_exception,
            'GNU GENERAL PUBLIC LICENSE Version 3',
        ),
    ),
    'unifont' => array(
        'url' => 'http://unifoundry.com/unifont.html',
        'spdx' => 'GPL-2.0-or-later WITH Font-exception-2.0 OR OFL-1.1',
        'markers' => array($font_exception, 'SIL OPEN FONT LICENSE Version 1.1'),
    ),
);

/**
 * Read a licence file and return its content, or null when the file is missing.
 *
 * @throws \RuntimeException when the file cannot be read or contains only whitespace.
 */
function readLicenseFile(string $file): ?string
{
    if (!\file_exists($file)) {
        return null;
    }
    $data = \is_file($file) && \is_readable($file) ? \file_get_contents($file) : false;
    if ($data === false) {
        throw new \RuntimeException('unable to read '.$file);
    }
    if (\trim($data) === '') {
        throw new \RuntimeException('empty '.$file);
    }
    return $data;
}

/**
 * Build the licence terms of a font family: NOTICE (when present) followed by LICENSE.
 *
 * @param string        $indir   Font family source directory.
 * @param array<string> $markers Phrases the terms must contain.
 *
 * @throws \RuntimeException when LICENSE is missing, a file is empty or unreadable, or a phrase is missing.
 */
function buildLicenseTerms(string $indir, array $markers): string
{
    $license = readLicenseFile($indir.'/LICENSE');
    if ($license === null) {
        throw new \RuntimeException('missing '.$indir.'/LICENSE');
    }
    $terms = $license;
    $notice = readLicenseFile($indir.'/NOTICE');
    if ($notice !== null) {
        $terms = \rtrim($notice)."\n\n".\str_repeat('-', 78)."\n\n".$license;
    }

    $flat = (string) \preg_replace('/\s+/', ' ', $terms);
    foreach ($markers as $marker) {
        if (\strpos($flat, (string) \preg_replace('/\s+/', ' ', $marker)) === false) {
            throw new \RuntimeException('the licence terms in '.$indir.' do not contain: '.$marker);
        }
    }

    return $terms;
}

foreach ($fontdir as $dir) {
	$indir = $ttfdir.$dir;
    if (!is_dir($indir)) {
        continue;
    }
    // search font files in sub directories
    $all_files  = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($indir));
    $fonts = \iterator_to_array(new RegexIterator($all_files, '/\.ttf$/'));
    $fonts = \array_merge($fonts, \iterator_to_array(new RegexIterator($all_files, '/\.pfb$/')));
    $fonts = \array_merge($fonts, \iterator_to_array(new RegexIterator($all_files, '/\.otf$/')));
    if (empty($fonts)) {
        $fonts = \iterator_to_array(new RegexIterator($all_files, '/\.afm$/'));
    }
    if (empty($fonts)) {
        continue;
    }

    $family = $families[$dir] ?? null;
    if ($family === null) {
        \fwrite(STDERR, "\033[33m".'!!! WARN : no licence metadata for the '.$dir.' family'."\033[m\n");
    }

    // a family is not converted without complete licence terms
    try {
        $terms = buildLicenseTerms($indir, $family['markers'] ?? array());
    } catch (\RuntimeException $exc) {
        ++$license_errors;
        \fwrite(STDERR, "\033[31m".'--- ERROR: skipped the '.$dir.' family: '.$exc->getMessage()."\033[m\n");
        continue;
    }

    // build output path directory
    $outdir = $options['outpath'].$dir.'/';
    if (!is_dir($outdir)) {
        \mkdir($outdir, 0755, true);
    }
    \file_put_contents($outdir.'LICENSE', $terms);

    // generate a README file
    $spdx = $family['spdx'] ?? 'NOASSERTION';
    $readme = '# '.$dir.' font files for tc-lib-pdf-font'."\n\n"
        .'This folder contains font files and/or font data extracted from:'."\n"
        .($family['url'] ?? 'an unlisted source')."\n"
        .'using the "bulk_convert.php" utility in https://github.com/tecnickcom/tc-lib-pdf-font'."\n\n"
        .'The original files (if present) have been renamed and compressed using the ZLIB data format (.z files).'."\n"
        .(isset($family['note']) ? $family['note']."\n" : '')
        .'The font files and data are distributed under the terms in the LICENSE file,'
        .' including any notice and exception stated there.'."\n"
        .((\strpos($spdx, 'Font-exception-2.0') !== false)
            ? 'The LICENSE file includes the GNU font embedding exception.'."\n"
            : '')
        .'For further information please consult the original documentation at the link above.'."\n\n"
        .'SPDX-License-Identifier: '.$spdx."\n";
    \file_put_contents($outdir.'README', $readme);

    foreach ($fonts as $font) {
        if (\substr($font, -4) == '.otf') {
            // OTF fonts are unsupported: convert them to TTF using FontForge
            \system('fontforge -script otf2ttf.ff '.\escapeshellarg((string) $font), $err);
            if ($err != 0) {
                \fwrite(STDERR, "\033[31m".'Unable to convert: '.$font."\033[m");
                continue;
            }
            $font = \substr($font, 0, -4).'.ttf';
        }
        
        $type = '';
        $encoding = '';
        if ($dir == 'cid0') {
            $type = \strtoupper(\basename($font, '.ttf'));
        } elseif (($dir == 'core') || ($dir == 'pdfa')) {
            if (\strpos($font, 'Symbol') !== false) {
                $encoding = 'symbol';
            } elseif (\strpos($font, 'ZapfDingbats') === false) {
                $encoding = 'cp1252';
            }
        }
        try {
            $import = new \Com\Tecnick\Pdf\Font\Import(
                \realpath($font),
                $outdir,
                $type,
                $encoding
            );
            $fontname = $import->getFontName();
            \fwrite(STDOUT, "\033[32m".'+++ OK   : '.$font.' added as '.$fontname."\033[m\n");
            ++$convert_success;
        } catch (\Exception $exc) {
            ++$convert_errors;
            \fwrite(STDERR, "\033[31m".'--- ERROR: can\'t add '.$font."\n           ".$exc->getMessage()."\033[m\n");
        }
    }
}

$endmsg = '>>> PROCESS COMPLETED: '.$convert_success.' CONVERTED FONT(S), '.$convert_errors.' ERROR(S), '
    .$license_errors.' SKIPPED FAMILY(IES)!'."\n\n";

if ($license_errors > 0) {
    \fwrite(STDERR, "\033[31m".$endmsg.'ERROR'."\033[m");
    exit(9);
}

if ($convert_errors > 0) {
    \fwrite(STDERR, "\033[31m".$endmsg.'ERROR'."\033[m");
    exit(4);
}

\fwrite(STDOUT, "\033[32m".$endmsg."\033[m");
exit(0);

