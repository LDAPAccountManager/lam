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

?>

<html>
<head>
<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\" />

</head>
<body>
<table border=0 width="100%">
  <tr>
    <td width="100"></td>
    <td><p align="center"><a href="http://lam.sf.net" target="new_window"><img src="../graphics/banner.jpg" border=1></a></p></td>
    <td width="100"><p align="right"><a href="./logout.php" target="_top"><? echo _("Logout") ?></a></p><br></td>
  </tr>
</table>
<br>
<table border="0" align="center" width="600">
  <tr>
    <td width="200"><p align="center"><a href="../lib/listusers.php" target="mainpart"> <? echo _("Users");?> </a></p></td>
    <td width="200"><p align="center"><a href="../lib/listgroups.php" target="mainpart"> <? echo _("Groups");?> </a></p></td>
    <td width="200"><p align="center"><a href="../lib/listhosts.php" target="mainpart"> <? echo _("Hosts");?> </a></p></td>
  </tr>
</table>
</body>
