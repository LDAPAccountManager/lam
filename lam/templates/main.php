<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/**
* This is the main window. The user and group lists will be shown in this frameset.
*
* @package main
* @author Roland Gruber
*/

/** LDAP attibute cache */
include_once('../lib/cache.inc');

// create cache object
if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
}


// check if all suffixes in conf-file exist
$conf = $_SESSION['config'];
$new_suffs = array();
if ($conf->get_UserSuffix() && ($conf->get_UserSuffix() != "")) {
	$info = @ldap_search($_SESSION['ldap']->server, $conf->get_UserSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
	if (!$res && !in_array($conf->get_UserSuffix(), $new_suffs)) $new_suffs[] = $conf->get_UserSuffix();
}
if ($conf->get_GroupSuffix() && ($conf->get_GroupSuffix() != "")) {
	$info = @ldap_search($_SESSION['ldap']->server, $conf->get_GroupSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
	if (!$res && !in_array($conf->get_GroupSuffix(), $new_suffs)) $new_suffs[] = $conf->get_GroupSuffix();
}
if ($conf->get_HostSuffix() && ($conf->get_HostSuffix() != "")) {
	$info = @ldap_search($_SESSION['ldap']->server, $conf->get_HostSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
	if (!$res && !in_array($conf->get_HostSuffix(), $new_suffs)) $new_suffs[] = $conf->get_HostSuffix();
}
if ($conf->get_DomainSuffix() && ($conf->get_DomainSuffix() != "")) {
	$info = @ldap_search($_SESSION['ldap']->server, $conf->get_DomainSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
	if (!$res && !in_array($conf->get_DomainSuffix(), $new_suffs)) $new_suffs[] = $conf->get_DomainSuffix();
}

// get encoding
$lang = explode(":",$_SESSION['language']);
$lang = $lang[1];

echo "<?xml version=\"1.0\" encoding=\"$lang\"?>\n";
echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\" \"http://www.w3.org/TR/html4/frameset.dtd\">\n";
echo "<html>\n";
echo "<head>\n";
echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=$lang\">\n";
echo "<meta http-equiv=\"pragma\" content=\"no-cache\">\n";
echo "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
echo ("<title>LDAP Account Manager</title>\n");
echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">");
echo ("</head>\n");
echo ("<frameset rows=\"130,*\">\n");
echo ("<frame src=\"./main_header.php\" name=\"head\" frameborder=\"0\" scrolling=\"no\" noresize>\n");
// display page to add suffixes, if needed
if (sizeof($new_suffs) > 0) echo ("<frame src=\"initsuff.php?suffs='" . implode(";", $new_suffs) .
	"'\" name=\"mainpart\" frameborder=\"0\" scrolling=\"yes\">\n");
else echo ("<frame src=\"./lists/listusers.php\" name=\"mainpart\" frameborder=\"0\" scrolling=\"yes\">\n");
echo ("<noframes>\n");
echo ("This page requires a browser that can show frames!\n");
echo ("</noframes>\n");
echo ("</frameset>\n");
echo ("</html>\n");

?>
