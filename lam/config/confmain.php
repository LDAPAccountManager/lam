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
echo ("<form action=\"confsave.php\" method=\"post\">\n");
echo ("<b>" . _("Hostname") . ": </b> <input type=\"text\" name=\"host\" value=\"" . $conf->get_Host() . "\"><br>\n");
echo ("<b>" . _("Portnumber") . ": </b> <input type=\"text\" name=\"port\" value=\"" . $conf->get_Port() . "\"><br>\n");
if ($conf->get_SSL() == "True") echo ("<b>" . _("Use SSL") . ": </b> <input type=\"checkbox\" name=\"ssl\" checked><br><br>\n");
else echo ("<b>" . _("Use SSL") . ": </b> <input type=\"checkbox\" name=\"ssl\"><br><br>\n");
echo ("<b>" . _("List of valid users") . ": </b> <input type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"><br>\n");
echo _("Usernames must be seperated by semicolons (e.g. cn=admin,dc=yourcompany,dc=com ; uid=root,ou=people,dc=yourcompany,cd=com)\n");
echo ("<br><br><br>\n");
echo ("<b>" . _("New Password") . ": </b> <input type=\"password\" name=\"pass1\"><br>\n");
echo ("<b>" . _("Reenter Password") . ": </b> <input type=\"password\" name=\"pass2\"><br>\n");
echo ("<input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"><br>\n");
echo ("<br><br><br>\n");
echo ("<input type=\"submit\" name=\"submitconf\" value=\"" . _("Ok") . ">\n");
echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");

?>

