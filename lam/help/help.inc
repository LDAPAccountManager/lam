<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Duergner

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


  LDAP Account Manager help topics.
*/

$helpArray = array (
				"101" => array ("ext" => "FALSE", "Headline" => _("Hilfetest 1"), "Text" => _("blablabla 1"), "SeeAlso" => "<a href=\"./help.php?HelpNumber=102\" target=\"_self\">102</a>"),
				"102" => array ("ext" => "FALSE", "Headline" => _("Hilfetest 2"), "Text" => _("blablabla 2"), "SeeAlso" => ""),
				"103" => array ("ext" => "TRUE", "Link" => "help/extFileExample.php")
/* This is a sample help entry. Just copy this line an modify the vakues between the [] brackets.
	Help text is located in the array:
		"[Helpnumber]" => array ("ext" => "FALSE", "Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => "[SeeAlso link]"),
	Help text is located in an extra file; path must be relative to lam_root; File should be stored in help directory
		"[Helpnumber]" => array ("ext" => "TRUE", "Link" => "[help/pathToFile]),
*/
				);
?>
