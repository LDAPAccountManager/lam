<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006 Michael Duergner

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

/**
 * LDAP Account Manager help entries.
 * 
 * @author Michael D�rgner
 * @package Help
 */

setlanguage();

// generate help entry for translated list attributes
$types = getTypes();
$entry206Example = "";
for ($i = 0; $i < sizeof($types); $i++) {
	$entry206Example .= "<b>" . getTypeAlias($types[$i]) . ":</b><br><br>\n";
	$descriptions = getListAttributeDescriptions($types[$i]);
	$attributes = array_keys($descriptions);
	for ($a = 0; $a < sizeof($attributes); $a++) {
		$entry206Example .= "#" . $attributes[$a] . ": " . $descriptions[$attributes[$a]] . "<br>";
	}
	$entry206Example .= "<br><br>";
}

$helpArray = array (
				// 0 - 99
				// any developer
				// 200 - 299
				// Roland Gruber
				// configuration wizard
				// configuration login
				// config profile management
				"200" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Login"),
					"Text" => _("Please enter the configuration password. This is NOT your LDAP password. It is stored in your .conf-file. If this is the first time you log in, enter \"lam\".")),
				"201" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Server address"),
					"Text" => _("This is the server address of your LDAP server. Use ldap:// for standard LDAP connections and ldaps:// for encrypted (require server certificates) connections. The port value is optional.") .
						"<br><br><b> " .
						_("Examples") .
						":</b><br><br> " .
						_("ldap://localhost:389 connects to localhost using a standard LDAP connection on port 389") .
						"<br> " .
						_("ldaps://141.40.146.133 connects to 141.40.146.133 using an encrypted LDAP connection.") .
						"<br><br><br><b> " .
						_("Note") .
						":</b><br><br>" .
						_("When using ldaps:// be sure to use exactly the same IP/domain name as in your certificate!")),
				"202" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("LDAP suffix"),
					"Text" => _("This is the suffix of the LDAP tree from where to search for LDAP entries. Only entries in this subtree will be displayed in the account list. When creating a new accont this will be the DN where it is saved.") .
						"<br><br><b>".
						_("Example").
						":</b><br><br>".
						_("ou=People,dc=yourcompany,dc=com will read and store all accounts in this subtree.")),
				"203" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Tree suffix"),
					"Text" => _("This is the suffix for the LDAP tree viewer.") .
						"<br><br><b>".
						_("Example").
						":</b><br><br>".
						_("dc=yourcompany,dc=com")),
				"206" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("List attributes"),
					"Text" => _("This is the list of attributes to show in the account list. The entries can either be predefined values, \"#attribute\", or individual ones, \"attribute:description\". Several entries are seperated by semicolons.") .
						"<br><br><br><big><b>" .
						_("Example") . ": </b></big>#homeDirectory;#uid;#uidNumber;#gidNumber;mail:Mail address<br><br>" .
						"<br><big><b>" . _("Predefined values") . ":</b></big><br><br><br>" . $entry206Example),
				"207" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Valid users"),
					"Text" => _("This is a list of valid DN entries of all users that are allowed to login to LDAP Account Manager. The user names have to be separated by semicolons.") .
						"<br><br><b>" .
						_("Example") .
						": </b>cn=admin,dc=yourdomain,dc=org;cn=manager,dc=yourdomain,dc=org"),
				"208" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Maximum list entries"),
					"Text" => _("This is the number of rows to show in the user/group/host list. If more entries are found the list will be split into several pages.")),
				"209" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Default language"),
					"Text" => _("This defines the language of the login window and sets this language as the default language. Users can change the language at login.")),
				"210" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Script path"),
					"Text" => _("This is the absolute path to an external script for setting quotas and creating home directories.").
							"<br><br><b>".
							_("Use it at your own risk and read the documentation for lamdaemon before you use it!").
							"</b>"),
				"211" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Script server"),
					"Text" => _("This is the server where the lamdaemon script is stored. LDAP Account Manager will make a SSH connection to this server with user name and password provided at login.").
						"<br><br><b>".
						_("Use it at your own risk and read the documentation for lamdaemon before you use it!").
						"</b>"),
				"212" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Change password"),
					"Text" => _("If you want to change the current preferences password, please enter it here.")),
				"214" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Cache timeout"),
					"Text" => _("This is the time in minutes which LAM caches its LDAP searches. Shorter times will stress LDAP more but decrease the possibility that changes are not identified.")),
				"216" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Text for user PDF"),
					"Text" => _("This text will appear on top of every user PDF file.")),
				"217" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Account types and modules"),
					"Text" => _("Here you can select which plugins you want to use for account management.") . "<br><br>"
						. _("Account types define which sorts of LDAP entries (e.g. users and groups) should be managed. The account modules define which properties (e.g. Unix and Samba) can be edited.")),
				"230" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Add profile"),
					"Text" => _("Please enter the name of the new profile and the password to change its settings. Profile names may contain letters, numbers and -/_.")),
				"231" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Rename profile"),
					"Text" => _("Please enter the new name of the profile. The name may contain letters, numbers and -/_.")),
				"232" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Delete profile"),
					"Text" => _("This will delete the selected profile.")),
				"233" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Set profile password"),
					"Text" => _("This changes the password of the selected profile.")),
				"234" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Change default profile"),
					"Text" => _("This changes the profile which is selected by default at login.")),
				"235" => array ("ext" => "FALSE", "Headline" => _("Change master password"),
					"Text" => _("If you want to change your master configuration password, please enter it here.")),
				"236" => array ("ext" => "FALSE", "Headline" => _("Master password"),
					"Text" => _("Please enter the master configuration password. This is NOT your LDAP password. It is stored in your config.cfg file. If this is the first time you log in, enter \"lam\".")),
				"237" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Base module"),
					"Text" => _("Every account type needs exactly one base module. This module provides a structural object class.")),
				"238" => array ("ext" => "FALSE", "Headline" => _("Session timeout"),
					"Text" => _("This is the time (in minutes) of inactivity after which a user is automatically logged off.")),
				"239" => array ("ext" => "FALSE", "Headline" => _("Log level"),
					"Text" => _("Please select your prefered log level. Messages with a lower level will not be logged.")),
				"240" => array ("ext" => "FALSE", "Headline" => _("Log destination"),
					"Text" => _("Here you can select where LAM should save its log messages. System logging will go to Syslog on Unix systems and event log on Windows. You can also select an extra file.")),
				"241" => array ("ext" => "FALSE", "Headline" => _("Allowed hosts"),
					"Text" => _("This is a list of IP addresses from hosts who may access LAM. You can use \"*\" as wildcard (e.g. 192.168.0.*).")),
				"250" => array ("ext" => "FALSE", "Headline" => _("Account lists - Filters"),
					"Text" => _("Here you can input small filter expressions (e.g. 'value' or 'v*'). LAM will filter case-insensitive.")),
				// 300 - 399
				// Roland Gruber
				// profile editor, file upload
				"301" => array ("ext" => "FALSE", "Headline" => _("RDN identifier"),
					"Text" => _("This is the identifier for the relative DN value. It must be one of the given allowed LDAP attributes (e.g. user accounts usually use \"uid\" while groups use \"cn\").")),
				"360" => array ("ext" => "FALSE", "Headline" => _("Profile name"),
					"Text" => _("Name under which the profile will be saved. If a profile with the same name exists, it will be overwritten.")),
				"361" => array ("ext" => "FALSE", "Headline" => _("DN suffix"),
					"Text" => _("The new account will be saved under this LDAP suffix.")),
				// 400 - 499
				// account pages
				"401" => array ("ext" => "FALSE", "Headline" => _("Load profile"),
					"Text" => _("Here you can load an account profile to set default settings for your account. The \"default\" profile is automatically loaded for new accounts.")),
				"402" => array ("ext" => "FALSE", "Headline" => _("Save account"),
					"Text" => _("This saves your account to the LDAP database.")),
				"403" => array ("ext" => "FALSE", "Headline" => _("Create PDF file"),
					"Text" => _("Here you can select a PDF structure and export the account to a PDF file.")),
				// 500 - 599
				// Roland Gruber
				// self service
				"501" => array ("ext" => "FALSE", "Headline" => _("LDAP suffix"),
					"Text" => _("LAM will search for accounts in this part of the LDAP tree.")),				
				"502" => array ("ext" => "FALSE", "Headline" => _("LDAP user and password"),
					"Text" => _("LAM will use this LDAP DN and password to search for accounts. It is sufficient to specify an account with read rights. If nothing is inserted then LAM will try to connect anonymously.")),				
				"503" => array ("ext" => "FALSE", "Headline" => _("LDAP search attribute"),
					"Text" => _("This is needed to find the LDAP DNs of your user accounts. E.g. if you use \"uid\" and your user inputs \"miller\" then LAM will search for an account with uid=miller.")),
				"504" => array ("ext" => "FALSE", "Headline" => _("Login attribute label"),
					"Text" => _("This text should shortly describe your selected LDAP search attribute (e.g. email or user name).")),				
				"505" => array ("ext" => "FALSE", "Headline" => _("Login caption"),
					"Text" => _("This text is displayed on top of the self service login page. You can also input HTML code here.")),
				"506" => array ("ext" => "FALSE", "Headline" => _("Main page caption"),
					"Text" => _("This text is displayed on top of the self service main page. You can also input HTML code here.")),
				"507" => array ("ext" => "FALSE", "Headline" => _("Add new group"),
					"Text" => _("Adds a new group element to the list of self service options. Use this to structure the input fields.")),
				"508" => array ("ext" => "FALSE", "Headline" => _("Add input field"),
					"Text" => _("Adds a new self service option to the selected group element.")),
				// 600 - 699
				// Roland Gruber
				// OU-editor, domain page
				"601" => array ("ext" => "FALSE", "Headline" => _("OU-Editor") . " - " . _("New organizational unit"),
					"Text" => _("This will create a new organizational unit under the selected one.")),
				"602" => array ("ext" => "FALSE", "Headline" => _("OU-Editor") . " - " . _("Delete organizational unit"),
					"Text" => _("This will delete the selected organizational unit. The OU has to be empty."))

);

/* This is a sample help entry. Just copy this line an modify the values between the [] brackets.

	"[HelpNumber]" => array ("ext" => "FALSE", "Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => "[SeeAlso link]"),
	"999" => array ("ext" => "FALSE", "Headline" => _("This is a sample entry"), "Text" => _("Sample text"), "SeeAlso" => "http://lam.sf.net"),


	If help text is located in an extra file then the file must be stored in help directory.
	The link has to be the path to the file if it is stored in a subdirectory and the filename with a leading slash.

	"[HelpNumber]" => array ("ext" => "TRUE", "Link" => "[path/FileName]),
	"999" => array ("ext" => "TRUE", "Link" => "extra.htm),


	Syntax:
	=======

		HelpNumber: numeric value
		ext: FALSE if help entry is located in this file, TRUE if external help page
		Headline: topic of help entry
		Text: help text (ext = FALSE)
		SeeAlso: URL to additional information (ext = FALSE, optional)
		Link: relative URL to external help page (ext = TRUE)
*/
?>
