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


  Configuration wizard - server settings second part
*/

include_once('../../lib/config.inc');
include_once('../../lib/ldap.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check master password
$cfg = new CfgMain();
if ($cfg->password != $_SESSION['confwiz_masterpwd']) {
	require("../config/conflogin.php");
	exit;
}


// check if user clicked on buttons
if ($_POST['submit'] || $_POST['cancel'] || $_POST['back']) {
	unset($error);
	unset($ret);
	if ($_POST['cancel']) {
		@unlink("../../config/" . $_SESSION['confwiz_config']->file . ".conf");
		metarefresh('../config/conflogin.php');
	}
	elseif ($_POST['back']) {
		metarefresh('server.php?back=true');
	}
	else {
		// set input values
		$errors = array();
		if (!$_SESSION['confwiz_config']->set_UserSuffix($_POST['usersuffix'])) {
			$error = _("UserSuffix is invalid!");
		}
		if (!$_SESSION['confwiz_config']->set_GroupSuffix($_POST['groupsuffix'])) {
			$error = _("GroupSuffix is invalid!");
		}
		if (!$_SESSION['confwiz_config']->set_HostSuffix($_POST['hostsuffix'])) {
			$error = _("HostSuffix is invalid!");
		}
		if ($_SESSION['confwiz_config']->is_samba3() && !$_SESSION['confwiz_config']->set_DomainSuffix($_POST['domainsuffix'])) {
			$error = _("DomainSuffix is invalid!");
		}
		if (!$_SESSION['confwiz_config']->set_pwdhash($_POST['pwdhash'])) {
			$error = _("Password hash is invalid!");
		}
		if (!$_SESSION['confwiz_config']->set_cacheTimeout($_POST['cachetimeout'])) {
			$error = _("Cache timeout is invalid!");
		}
		$_SESSION['confwiz_config']->save();
		// print error message if needed
		if (sizeof($errors) > 0) {
			echo $_SESSION['header'];
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "<title>\n";
				echo _("Configuration wizard");
			echo "</title>\n";
			echo "</head><body>\n";
			for ($i = 0; $i < sizeof($errors); $i++) {
				StatusMessage("ERROR", $errors[$i], "");
			}
			echo "<p><br><br><a href=\"server2.php\">" . _("Back to last page") . "</a></p>\n";
			echo "</body></html>\n";
		}
		// if all ok, go to next page
		else {
			$_SESSION['confwiz_optional'] = array();
			if ($_POST['ranges']) $_SESSION['confwiz_optional']['ranges'] = 'yes';
			if ($_POST['lists']) $_SESSION['confwiz_optional']['lists'] = 'yes';
			if ($_POST['lang']) $_SESSION['confwiz_optional']['lang'] = 'yes';
			if ($_POST['daemon']) $_SESSION['confwiz_optional']['daemon'] = 'yes';
			metarefresh('optional.php');
		}
	}
	exit;
}

echo $_SESSION['header'];

	echo "<title>\n";
		echo _("Configuration wizard");
	echo "</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
echo "<body>\n";
	echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
	echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
	echo "</p>\n";
	echo "<hr>\n";

// formular
echo "<form action=\"server2.php\" method=\"post\">\n";
	echo "<br><br>\n";
	echo "<table border=0>\n";

	// suffixes
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Please enter the suffixes of your LDAP tree where LAM should store the accounts.");
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("UserSuffix") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"text\" size=50 name=\"usersuffix\" value=\"" . $_SESSION['confwiz_config']->get_userSuffix() . "\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("GroupSuffix") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"text\" size=50 name=\"groupsuffix\" value=\"" . $_SESSION['confwiz_config']->get_groupSuffix() . "\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("HostSuffix") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"text\" size=50 name=\"hostsuffix\" value=\"" . $_SESSION['confwiz_config']->get_hostSuffix() . "\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	if ($_SESSION['confwiz_config']->is_samba3()) {
		echo "<tr>\n";
			echo "<td>\n";
				echo "<b>" . _("DomainSuffix") . ":</b>\n";
			echo "</td>\n";
			echo "<td>\n";
				echo "<input type=\"text\" size=50 name=\"domainsuffix\" value=\"" . $_SESSION['confwiz_config']->get_domainSuffix() . "\">\n";
			echo "</td>\n";
		echo "</tr>\n";
	}

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// password hash
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("LAM supports CRYPT, SHA, SSHA, MD5 and SMD5 to generate the hash value of an user password. SSHA and CRYPT are the most common but CRYPT does not support passwords greater than 8 letters. We do not recommend to use plain text passwords.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Password hash type") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<select name=\"pwdhash\">\n<option selected>" . $_SESSION['confwiz_config']->get_pwdhash() . "</option>\n";
			if ($_SESSION['confwiz_config']->get_pwdhash() != "CRYPT") echo("<option>CRYPT</option>\n");
			if ($_SESSION['confwiz_config']->get_pwdhash() != "SHA") echo("<option>SHA</option>\n");
			if ($_SESSION['confwiz_config']->get_pwdhash() != "SSHA") echo("<option>SSHA</option>\n");
			if ($_SESSION['confwiz_config']->get_pwdhash() != "MD5") echo("<option>MD5</option>\n");
			if ($_SESSION['confwiz_config']->get_pwdhash() != "SMD5") echo("<option>SMD5</option>\n");
			if ($_SESSION['confwiz_config']->get_pwdhash() != "PLAIN") echo("<option>PLAIN</option>\n");
			echo ("</select>\n");
		echo "</td>\n";
	echo "</tr>\n";

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// cache timeout
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("LAM caches its LDAP searches, you can set the cache time here. Shorter times will stress LDAP more but decrease the possibility that changes are not identified.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Cache timeout") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<select name=\"cachetimeout\">\n<option selected>".$_SESSION['confwiz_config']->get_cacheTimeout()."</option>\n";
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 0) echo("<option>0</option>\n");
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 1) echo("<option>1</option>\n");
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 2) echo("<option>2</option>\n");
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 5) echo("<option>5</option>\n");
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 10) echo("<option>10</option>\n");
			if ($_SESSION['confwiz_config']->get_cacheTimeout() != 15) echo("<option>15</option>\n");
			echo ("</select>\n");
		echo "</td>\n";
	echo "</tr>\n";

	echo "</table>\n";

	echo "<p><br></p>\n";

// optional pages
	echo "<fieldset><legend><b>" . _("Optional settings") . "</b></legend>\n";
	echo "<p>" . _("Please select here if you want to make additional changes to your configuration profile or if LAM should use default values.") .
			"<br></p>\n";
	echo "<input type=\"checkbox\" name=\"ranges\">" . _("Ranges for UID and GID numbers") . "<br>\n";
	echo "<input type=\"checkbox\" name=\"lists\">" . _("Attributes in list views") . "<br>\n";
	echo "<input type=\"checkbox\" name=\"lang\">" . _("Language and additional admin users") . "<br>\n";
	echo "<input type=\"checkbox\" name=\"daemon\">" . _("Lamdaemon settings and PDF text") . "<br>\n";
	echo "</fieldset>\n";

	echo "<p><br></p>\n";

// next/cancel button
	echo "<p>\n";
		echo "<input type=\"submit\" name=\"submit\" value=\"" . _("Next") . "\">\n";
		echo "<input type=\"submit\" name=\"back\" value=\"" . _("Back") . "\">\n";
		echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">\n";
	echo "</p>\n";

echo "</form>\n";

echo "</body>\n</html>\n";

?>
