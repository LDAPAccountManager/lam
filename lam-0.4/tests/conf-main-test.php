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

	This test reads all preferences from lam.conf. Then it writes new values and verifies
	if they were written. At last the old values are restored.
*/

include ("../lib/config.inc");
$conf = new CfgMain();
echo "<html><head><title></title><link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\"></head><body>";
echo ("<b> Current Values</b><br><br>");
echo "<b>Password: </b>" . $conf->password . "<br>\n";
echo "<b>Default: </b>" . $conf->default . "<br>\n";
echo ("<br><br><big><b> Starting Test...</b></big><br><br>");
// now all prferences are loaded
echo ("Loading preferences...");
$password = $conf->password;
$default = $conf->default;
echo ("done<br>");
// next we modify them and save config.cfg
echo ("Changing preferences...");
$conf->password = "123456";
$conf->default = "lam";
$conf->save();
echo ("done<br>");
// at last all preferences are read from lam.conf and compared
echo ("Loading and comparing...");
$conf = new CfgMain();
if ($conf->password != "123456") echo ("<br><font color=\"#FF0000\">Saving password failed!</font><br>");
if ($conf->default != "lam") echo ("<br><font color=\"#FF0000\">Saving Default failed!</font><br>");
echo ("done<br>");
// restore old values
echo ("Restoring old preferences...");
$conf->password = $password;
$conf->default = $default;
$conf->save();
echo ("done<br>");
// finished
echo ("<br><b><font color=\"#00C000\">Test is complete.</font></b>");
echo ("<br><br><b> Current Config</b><br><br>");
echo "<b>Password: </b>" . $conf->password . "<br>\n";
echo "<b>Default: </b>" . $conf->default . "<br>\n";

?>
