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

session_start();

// check if password was entered
// if not: load login page
if (! $passwd) {
	require('conflogin.php');
	exit;
}

// check if password is valid
// if not: load login page
include_once ('config.php');
$conf = new Config();
if (!(($conf->get_Passwd()) == $passwd)) {
	require('conflogin.php');
	exit;
}

echo ("<html>\n");
echo ("<head>\n");
echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo ("</head>\n");
echo ("<body>\n");
echo ("<p align=\"center\"><img src=\"../graphics/banner.jpg\" border=1></p><hr><br><br>\n");
echo ("<form action=\"confsave.php\" method=\"post\">\n");
echo ("<p align=\"center\"><table border=\"0\">");
echo ("<tr><th><p align=\"right\"><b>" . _("Hostname") . ": </b></th> <th><p align=\"left\"><input type=\"text\" name=\"host\" value=\"" . $conf->get_Host() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("Hostname of LDAP server") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Portnumber") . ": </b></th> <th><p align=\"left\"><input type=\"text\" size=5 name=\"port\" value=\"" . $conf->get_Port() . "\"></th>\n");
echo _("<th><p align=\"left\">Default is 389, use 636 for SSL connections</th></tr>\n");
if ($conf->get_SSL() == "True") echo ("<tr><th><p align=\"right\"><b>" . _("Use SSL") . ": </b></th> <th><p align=\"left\"><input type=\"checkbox\" name=\"ssl\" checked></th>\n");
else echo ("<tr><th><p align=\"right\"><b>" . _("Use SSL") . ": </b></th> <th><p align=\"left\"><input type=\"checkbox\" name=\"ssl\"></th>\n");
echo ("<th><p align=\"left\">" . _("Check if your server supports secure connections.") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("List of valid users") . ": </b></th> <th><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("Usernames must be seperated by semicolons<br>(e.g. cn=admin,dc=yourcompany,dc=com ; uid=root,ou=people,dc=yourcompany,dc=com)") . "</th></tr>\n");
echo ("<tr><th>&nbsp</th></tr>");
echo ("<tr><th><p align=\"right\"><b>" . _("UserSuffix") . ": </b></th> <th><input size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_UserSuffix() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("This is the suffix from where to search for users.<br>(e.g. ou=People,dc=yourcompany,dc=com)=") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("GroupSuffix") . ": </b></th> <th><input size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_GroupSuffix() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("This is the suffix from where to search for groups.<br>(e.g. ou=group,dc=yourcompany,dc=com)") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("HostSuffix") . ": </b></th> <th><input size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_HostSuffix() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("This is the suffix from where to search for Samba hosts.<br>(e.g. ou=machines,dc=yourcompany,dc=com)") . "</th></tr>\n");
echo ("<tr><th>&nbsp</th></tr>");
echo ("<tr><th><p align=\"right\"><b>" . _("Minimum UID number") . ": </b></th> <th><p align=\"left\"><input size=6 type=\"text\" name=\"minUID\" value=\"" . $conf->get_minUID() . "\">\n");
echo ("&nbsp <b>" . _("Maximum UID number") . ": </b><input size=6 type=\"text\" name=\"maxUID\" value=\"" . $conf->get_maxUID() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("These are the minimum and maximum numbers to use for user IDs") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Minimum GID number") . ": </b></th> <th><p align=\"left\"><input size=6 type=\"text\" name=\"minGID\" value=\"" . $conf->get_minGID() . "\">\n");
echo ("&nbsp <b>" . _("Maximum GID number") . ": </b><input size=6 type=\"text\" name=\"maxGID\" value=\"" . $conf->get_maxGID() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("These are the minimum and maximum numbers to use for group IDs") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Minimum Machine number") . ": </b></th> <th><p align=\"left\"><input size=6 type=\"text\" name=\"minMach\" value=\"" . $conf->get_minMachine() . "\">\n");
echo ("&nbsp <b>" . _("Maximum Machine number") . ": </b><input size=6 type=\"text\" name=\"maxMach\" value=\"" . $conf->get_maxMachine() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("These are the minimum and maximum numbers to use for Samba hosts. <br> Do not use the same range as for user IDs.") . "</th></tr>\n");
echo ("<tr><th>&nbsp</th></tr>");
echo ("<tr><th><p align=\"right\"><b>" . _("Default shell") . ": </b></th> <th><p align=\"left\"><input type=\"text\" name=\"defShell\" value=\"" . $conf->get_defaultShell() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("Default shell when creating new users.") . "</th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Shell list") . ": </b></th> <th><p align=\"left\"><input type=\"text\" size=50 name=\"shellList\" value=\"" . $conf->get_shellList() . "\"></th>\n");
echo ("<th><p align=\"left\">" . _("List of possible shells when creating new users. <br> The entries have to be separated by semicolons.") . "</th></tr>\n");
echo ("<tr><th>&nbsp</th></tr>");
echo ("<tr><th><p align=\"right\"><b>" . _("New Password") . ": </b></th> <th><p align=\"left\"><input type=\"password\" name=\"pass1\"></th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Reenter Password") . ": </b></th> <th><p align=\"left\"><input type=\"password\" name=\"pass2\"></th></tr>\n");
echo ("<input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"><br>\n");
echo ("<tr><th>&nbsp</th></tr>\n");
echo ("<tr><th>&nbsp</th></tr>\n");
echo ("<tr><th></th><th><p align=\"left\"><input type=\"submit\" name=\"submitconf\" value=\"" . _("Submit") . "></th></tr>\n");
echo ("</table>\n");
echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");

?>

