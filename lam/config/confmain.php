<?
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

// check if password was entered
if (! $passwd) {
	require('conflogin.php');
	exit;
}

// check if password is valid
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
echo ("<br><b><big><big><p align=\"center\"> LDAP Account Manager</b></big></big></p><br><br>\n");
echo ("<form action=\"confsave.php\" method=\"post\">\n");
echo ("<p align=\"center\"><table border=\"0\">");
echo ("<tr><th><p align=\"right\"><b>" . _("Hostname") . ": </b></th> <th><p align=\"left\"><input type=\"text\" name=\"host\" value=\"" . $conf->get_Host() . "\"></th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("Portnumber") . ": </b></th> <th><p align=\"left\"><input type=\"text\" size=5 name=\"port\" value=\"" . $conf->get_Port() . "\"></th>\n");
echo _("<th><p align=\"left\">Default is 389, use 636 for SSL connections</th></tr>\n");
if ($conf->get_SSL() == "True") echo ("<tr><th><p align=\"right\"><b>" . _("Use SSL") . ": </b></th> <th><p align=\"left\"><input type=\"checkbox\" name=\"ssl\" checked></th></tr>\n");
else echo ("<tr><th><p align=\"right\"><b>" . _("Use SSL") . ": </b></th> <th><p align=\"left\"><input type=\"checkbox\" name=\"ssl\"></th></tr>\n");
echo ("<tr><th><p align=\"right\"><b>" . _("List of valid users") . ": </b></th> <th><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></th>\n");
echo _("<th><p align=\"left\">Usernames must be seperated by semicolons<br>(e.g. cn=admin,dc=yourcompany,dc=com ; uid=root,ou=people,dc=yourcompany,cd=com)</th></tr>\n");
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

