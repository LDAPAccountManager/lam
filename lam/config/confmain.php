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

// start session
session_save_path("../sess");
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
echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\"><img src=\"../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>\n");
echo ("<form action=\"confsave.php\" method=\"post\">\n");
echo ("<table align=\"center\" border=\"0\">");
echo ("<tr><td width=\"20%\"><p align=\"right\"><b>" . _("Hostname") . ": </b></p></td> <td width=\"30%\"><p align=\"left\"><input type=\"text\" name=\"host\" value=\"" . $conf->get_Host() . "\"></td>\n");
echo ("<td width=\"50%\"><p align=\"left\">" . _("Hostname of LDAP server") . "</p></td></tr>\n");
echo ("<tr><td><p align=\"right\"><b>" . _("Portnumber") . ": </b></p></td> <td><p align=\"left\"><input type=\"text\" size=5 name=\"port\" value=\"" . $conf->get_Port() . "\"></td>\n");
echo _("<td><p align=\"left\">Default is 389, use 636 for SSL connections</p></td></tr>\n");
if ($conf->get_SSL() == "True") echo ("<tr><td><p align=\"right\"><b>" . _("Use SSL") . ": </b></p></td> <td><p align=\"left\"><input type=\"checkbox\" name=\"ssl\" checked></td>\n");
else echo ("<tr><td><p align=\"right\"><b>" . _("Use SSL") . ": </b></p></td> <td><p align=\"left\"><input type=\"checkbox\" name=\"ssl\"></td>\n");
echo ("<td><p align=\"left\">" . _("Check if your server supports secure connections.") . "</p></td></tr>\n");
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("List of valid users") . ": </b></p></td> <td><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("Usernames must be seperated by semicolons<br>(e.g. cn=admin,dc=yourcompany,dc=com;uid=root,ou=people,dc=yourcompany,dc=com)") . "</p></td></tr>\n");
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("UserSuffix") . ": </b></p></td> <td><input size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_UserSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("This is the suffix from where to search for users.<br>(e.g. ou=People,dc=yourcompany,dc=com)") . "</p></td></tr>\n");
echo ("<tr><td><p align=\"right\"><b>" . _("GroupSuffix") . ": </b></p></td> <td><input size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_GroupSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("This is the suffix from where to search for groups.<br>(e.g. ou=group,dc=yourcompany,dc=com)") . "</p></td></tr>\n");
echo ("<tr><td><p align=\"right\"><b>" . _("HostSuffix") . ": </b></p></td> <td><input size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_HostSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("This is the suffix from where to search for Samba hosts.<br>(e.g. ou=machines,dc=yourcompany,dc=com)") . "</p></td></tr>\n");
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td align=\"right\"><b>" . _("Minimum UID number") . ": </b></td> <td align=\"left\"><input size=6 type=\"text\" name=\"minUID\" value=\"" . $conf->get_minUID() . "\">\n");
echo ("&nbsp <b>" . _("Maximum UID number") . ": </b><input size=6 type=\"text\" name=\"maxUID\" value=\"" . $conf->get_maxUID() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("These are the minimum and maximum numbers to use for user IDs") . "</p></td></tr>\n");
echo ("<tr><td align=\"right\"><b>" . _("Minimum GID number") . ": </b></td> <td align=\"left\"><input size=6 type=\"text\" name=\"minGID\" value=\"" . $conf->get_minGID() . "\">\n");
echo ("&nbsp <b>" . _("Maximum GID number") . ": </b><input size=6 type=\"text\" name=\"maxGID\" value=\"" . $conf->get_maxGID() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("These are the minimum and maximum numbers to use for group IDs") . "</p></td></tr>\n");
echo ("<tr><td align=\"right\"><b>" . _("Minimum Machine number") . ": </b></td> <td align=\"left\"><input size=6 type=\"text\" name=\"minMach\" value=\"" . $conf->get_minMachine() . "\">\n");
echo ("&nbsp <b>" . _("Maximum Machine number") . ": </b><input size=6 type=\"text\" name=\"maxMach\" value=\"" . $conf->get_maxMachine() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("These are the minimum and maximum numbers to use for Samba hosts. <br> Do not use the same range as for user IDs.") . "</p></td></tr>\n");
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("Attributes in User List:") . "</b></p></td><td><input size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $conf->get_userlistAttributes() . "\"></td>");
echo ("<td rowspan=3><p>" . _("This is the list of attributes to show in the lists. The entries can either be predefined values (e.g. '#cn' or '#gidNumber') or individual ones (e.g. 'cn:Group Name'). The entries are seperated by semicolons.")
	. "</p></td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("Attributes in Group List:") . "</b></p></td><td><input size=50 type=\"text\" name=\"grplstattr\" value=\"" . $conf->get_grouplistAttributes() . "\"></td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("Attributes in Host List:") . "</b></p></td><td><input size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $conf->get_hostlistAttributes() . "\"></td></tr>");
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td><p align=\"right\"><b>" . _("Default shell") . ": </b></p></td> <td><p align=\"left\"><input type=\"text\" name=\"defShell\" value=\"" . $conf->get_defaultShell() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("Default shell when creating new users.") . "</p></td></tr>\n");
echo ("<tr><td><p align=\"right\"><b>" . _("Shell list") . ": </b></p></td> <td><p align=\"left\"><input type=\"text\" size=50 name=\"shellList\" value=\"" . $conf->get_shellList() . "\"></td>\n");
echo ("<td><p align=\"left\">" . _("List of possible shells when creating new users. <br> The entries have to be separated by semicolons.") . "</p></td></tr>\n");
echo ("</table>\n");
echo ("<p>&nbsp</p>\n");
echo ("<table align=\"left\" border=\"0\">");
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>" . _("New Password") . ": </b></td> <td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass1\"></td>");
echo ("<td rowspan=2 width=10></td><td rowspan=2>" . _("Optional") . "</td></tr>\n");
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>" . _("Reenter Password") . ": </b></td> <td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass2\"></td></tr>\n");
echo ("</table>\n");
echo ("<p>&nbsp</p>\n");
echo ("<p>&nbsp</p>\n");
echo ("<table align=\"left\" border=\"0\">");
echo ("<tr><td align=\"left\"><pre><input type=\"submit\" name=\"submitconf\" value=\"" . _("Submit") . "\">     <input type=\"reset\" name=\"resetconf\" value=\"" . _("Reset") . "\">");
echo ("<input type=\"button\" name=\"back\" value=\"" . _("Abort") . "\" onClick=\"self.location.href='../templates/login.php'\"></pre></td></tr>\n");
echo ("</table>\n");
echo ("<input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"><br>\n");
echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");

?>

