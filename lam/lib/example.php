<?php
// $Horde: nmslib/base.php,v 1.6 2002/06/19 02:31:26 chuck Exp $
// modified 2002/07/13 Tilo Lutz
/*
 * NMS base inclusion file.
 *
 * This file brings in all of the dependencies that every NMS script
 * will need, and sets up objects that all scripts use.
 */

// Find the base file path of Horde
@define('HORDE_BASE', dirname(__FILE__) . '/../..');

// Find the base file path of VACATION
@define('NMS_BASE', dirname(__FILE__) . '/..');

// Registry
require_once HORDE_BASE . '/lib/Registry.php';
$registry = &Registry::singleton();
$registry->pushApp('nms');
$conf = &$GLOBALS['conf'];
@define('NMS_TEMPLATES', $registry->getParam('templates'));

// Horde base libraries
require_once HORDE_BASE . '/lib/Horde.php';
require_once HORDE_BASE . '/lib/Auth.php';
require_once HORDE_BASE . '/lib/Secret.php';
require_once HORDE_BASE . '/lib/Text.php';
require_once HORDE_BASE . '/lib/Help.php';

// Browser detection library
require_once HORDE_BASE . '/lib/Browser.php';
$browser = new Browser();
if (isset($session_control)) {
    switch ($session_control) {
    case 'netscape':
        if ($browser->isBrowser('mozilla')) {
            session_cache_limiter('private, must-revalidate');
        }
        break;

    case 'cache_ssl_downloads':
        header('Vary: User-Agent');
        if ($browser->hasQuirk('cache_ssl_downloads')) {
            session_cache_limiter('private, must-revalidate');
        }
        break;
    }
}

// Notification system
require_once HORDE_BASE . '/lib/Notification.php';
$notification = &Notification::singleton();
$notification->attach('status');

// NMS base library
#require_once NMS_BASE . '/lib/version.php';
define('NMS_NAME', 'Horde nms module');

// Don't allow access unless there is a Horde login
// NOTE:  We explicitely do not honor the guests flag here!!!
if (!Auth::getAuth()) {
    header('Location: ' . Horde::url($registry->getWebRoot("horde") . '/login.php?url=' . urlencode(Horde::selfUrl()), true));
    echo "\n";
    exit;
}

?>
