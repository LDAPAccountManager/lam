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

  Creates main suffixes if they are missing.

*/

include_once ("../lib/config.inc");
include_once ("../lib/ldap.inc");
include_once ("../lib/status.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

// check if user already pressed button
if ($_POST['new_suff'] || $_POST['cancel']) {
	if ($_POST['new_suff']) {
	$new_suff = $_POST['new_suff'];
	$new_suff = str_replace("\\'", "", $new_suff);
	$new_suff = explode(";", $new_suff);
		// add entry
		for ($i = 0; $i < sizeof($new_suff); $i++) {
			$suff = $new_suff[$i];
			// generate DN and attributes
			$tmp = explode(",", $suff);
			$name = explode("=", $tmp[0]);
			array_shift($tmp);
			$end = implode(",", $tmp);
			if ($name[0] != "ou") {
				continue;
			}
			else {
				$name = $name[1];
				$attr = array();
				$attr['objectClass'] = "organizationalunit";
				$attr['ou'] = $name;
				$dn = "ou=" . $name . "," . $end;
				@ldap_add($_SESSION['ldap']->server(), $dn, $attr);
			}
		}
	}
	echo "<meta http-equiv=\"refresh\" content=\"0; lists/listusers.php\">";
	exit;
}

// first show of page
$new_suff = $_GET['suffs'];
$new_suff = str_replace("\\'", "", $new_suff);
$new_suff = explode(";", $new_suff);

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
echo "<html><head><title>initsuff</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head><body>\n";
	echo "<p>&nbsp;</p>\n";
	echo "<p><font color=\"red\"><b>" . _("The following suffix(es) are missing in LDAP. LAM can create them for you.") . "</b></font></p>\n";
	echo "<p>&nbsp;</p>\n";
	// print missing suffixes
	for ($i = 0; $i < sizeof($new_suff); $i++) {
		echo "<p><b>" . $new_suff[$i] . "</b></p>\n";
	}
	echo "<p>&nbsp;</p>\n";
	echo "<form action=\"initsuff.php\" method=\"post\">\n";
	echo "<input type=\"hidden\" name=\"new_suff\" value=\"" . implode(";", $new_suff) . "\">\n";
	echo "<input type=\"submit\" name=\"add_suff\" value=\"" . _("Create") . "\">";
	echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">";
	echo "</form>\n";
echo "</body></html>\n";
?>
