<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006 Michael Duergner
                2003 - 2011 Roland Gruber

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
 * @author Michael Duergner
 * @package Help
 */

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
				// 200 - 299
				// configuration wizard
				// configuration login
				// config profile management
				"200" => array ("Headline" => _("Login"),
					"Text" => _("Please enter the configuration password. This is NOT your LDAP password. It is stored in your .conf-file. If this is the first time you log in, enter \"lam\".")),
				"201" => array ("Headline" => _("Server address"),
					"Text" => _("This is the server address of your LDAP server. Use ldap:// for unencrypted LDAP connections or TLS encrypted connections. LDAP+SSL (LDAPS) encrypted connections are specified with ldaps://. The port value is optional.") .
						" " . _("TLS cannot be combined with ldaps://.") .
						"<br><br><b> " .
						_("Examples") .
						":</b><br><br> " .
						_("ldap://localhost:389 connects to localhost using a standard LDAP connection on port 389") .
						"<br> " .
						_("ldaps://ldap.domain.com connects to ldap.domain.com using an encrypted LDAP connection.") .
						"<br><br><br><b> " .
						_("Note") .
						":</b><br><br>" .
						_("When using ldaps:// or TLS be sure to use exactly the same IP/domain name as in your certificate!")),
				"202" => array ("Headline" => _("LDAP suffix"),
					"Text" => _("This is the suffix of the LDAP tree from where to search for LDAP entries. Only entries in this subtree will be displayed in the account list. When creating a new accont this will be the DN where it is saved.") .
						"<br><br><b>".
						_("Example").
						":</b><br><br>".
						_("ou=People,dc=yourcompany,dc=com will read and store all accounts in this subtree.")),
				"203" => array ("Headline" => _("Tree suffix"),
					"Text" => _("This is the suffix for the LDAP tree viewer.") .
						"<br><br><b>".
						_("Example").
						":</b><br><br>".
						_("dc=yourcompany,dc=com")),
				"206" => array ("Headline" => _("List attributes"),
					"Text" => _("This is the list of attributes to show in the account list. The entries can either be predefined values, \"#attribute\", or individual ones, \"attribute:description\". Several entries are separated by semicolons.") .
						"<br><br><br><big><b>" .
						_("Example") . ": </b></big>#homeDirectory;#uid;#uidNumber;#gidNumber;mail:Mail address<br><br>" .
						"<br><big><b>" . _("Predefined values") . ":</b></big><br><br><br>" . $entry206Example),
				"207" => array ("Headline" => _("Valid users"),
					"Text" => _("This is a list of valid DN entries of all users that are allowed to login to LDAP Account Manager. Please enter one DN per line.") .
						"<br><br><b>" .
						_("Example") .
						": </b><br>cn=admin,dc=yourdomain,dc=org<br>cn=manager,dc=yourdomain,dc=org"),
				"208" => array ("Headline" => _("Maximum list entries"),
					"Text" => _("This is the number of rows to show in the account list. If more entries are found the list will be split into several pages.")),
				"209" => array ("Headline" => _("Default language"),
					"Text" => _("This defines the language of the login window and sets this language as the default language. Users can change the language at login.")),
				"210" => array ("Headline" => _("Script path"),
					"Text" => _("This is the absolute path to an external script for setting quotas and creating home directories.")),
				"212" => array ("Headline" => _("Change password"),
					"Text" => _("If you want to change the current preferences password, please enter it here.")),
				"214" => array ("Headline" => _("Cache timeout"),
					"Text" => _("This is the time in minutes which LAM caches its LDAP searches. Shorter times will stress LDAP more but decrease the possibility that changes are not identified.")),
				"215" => array ("Headline" => _("Access level"),
					"Text" => _("You can specify if LAM allows full write access, password changes or only read access.")),
				"217" => array ("Headline" => _("Account types and modules"),
					"Text" => _("Here you can select which plugins you want to use for account management.") . "<br><br>"
						. _("Account types define which sorts of LDAP entries (e.g. users and groups) should be managed. The account modules define which properties (e.g. Unix and Samba) can be edited.")),
				"218" => array ("Headline" => _("Script servers"),
					"Text" => _("This is a list of the servers where the lamdaemon scripts are stored. LDAP Account Manager will make a SSH connection to the servers with the user name and password provided at login. Multiple servers are separated by semicolons. You can append a descriptive name after a colon.") . "<br>"
						. _("If your server runs on another port then add a comma and the port number after the server.") . "<br><br>"
						. _("Example") . ": <b>127.0.0.1:LOCAL;192.168.0.2,12345:Servername;192.168.0.5</b>"),
				"219" => array ("Headline" => _("Rights for the home directory"),
					"Text" => _("This defines the rights for the home directories which are created by lamdaemon.")),
				"220" => array ("Headline" => _("Login method"),
					"Text" => _("The number of users who may login to LAM is restricted. This can be either a fixed list of DNs or LAM can search LDAP to find a DN which matches the given user name.")),
				"221" => array ("Headline" => _("LDAP search"),
					"Text" => _("Please enter the LDAP suffix where LAM should start to search for users. The LDAP filter needs to match the given user name to exactly one DN. The value \"%USER%\" will be replaced by the user name from the login page.")),
				"222" => array ("Headline" => _("LDAP search limit"),
					"Text" => _("Here you can set a limit for LDAP searches. This will restrict the number of results for LDAP searches. Please use this if LAM's LDAP queries produce too much load.")),
				"230" => array ("Headline" => _("Profile management") . " - " . _("Add profile"),
					"Text" => _("Please enter the name of the new profile and the password to change its settings. Profile names may contain letters, numbers and -/_.")),
				"231" => array ("Headline" => _("Profile management") . " - " . _("Rename profile"),
					"Text" => _("Please enter the new name of the profile. The name may contain letters, numbers and -/_.")),
				"232" => array ("Headline" => _("Profile management") . " - " . _("Delete profile"),
					"Text" => _("This will delete the selected profile.")),
				"233" => array ("Headline" => _("Profile management") . " - " . _("Set profile password"),
					"Text" => _("This changes the password of the selected profile.")),
				"234" => array ("Headline" => _("Profile management") . " - " . _("Change default profile"),
					"Text" => _("This changes the profile which is selected by default at login.")),
				"235" => array ("Headline" => _("Change master password"),
					"Text" => _("If you want to change your master configuration password, please enter it here.")),
				"236" => array ("Headline" => _("Master password"),
					"Text" => _("Please enter the master configuration password. This is NOT your LDAP password. It is stored in your config.cfg file. If this is the first time you log in, enter \"lam\".")),
				"237" => array ("Headline" => _("Base module"),
					"Text" => _("Every account type needs exactly one base module. This module provides a structural object class.")),
				"238" => array ("Headline" => _("Session timeout"),
					"Text" => _("This is the time (in minutes) of inactivity after which a user is automatically logged off.")),
				"239" => array ("Headline" => _("Log level"),
					"Text" => _("Please select your prefered log level. Messages with a lower level will not be logged.")),
				"240" => array ("Headline" => _("Log destination"),
					"Text" => _("Here you can select where LAM should save its log messages. System logging will go to Syslog on Unix systems and event log on Windows. You can also select an extra file.")),
				"241" => array ("Headline" => _("Allowed hosts"),
					"Text" => _("This is a list of IP addresses from hosts who may access LAM. You can use \"*\" as wildcard (e.g. 192.168.0.*).")),
				"242" => array ("Headline" => _("Password policy"),
					"Text" => _("Here you can specify minimum requirements for passwords. The character classes are: lowercase, uppercase, numeric and symbols.")),
				"250" => array ("Headline" => _("Filter"),
					"Text" => _("Here you can input simple filter expressions (e.g. 'value' or 'v*'). The filter is case-sensitive.")),
				// 300 - 399
				// profile editor, file upload
				"301" => array ("Headline" => _("RDN identifier"),
					"Text" => _("This is the identifier for the relative DN value. It must be one of the given allowed LDAP attributes (e.g. user accounts usually use \"uid\" while groups use \"cn\").")),
				"360" => array ("Headline" => _("Profile name"),
					"Text" => _("Name under which the profile will be saved. If a profile with the same name exists, it will be overwritten.")),
				"361" => array ("Headline" => _("DN suffix"),
					"Text" => _("The account will be saved under this LDAP suffix.")),
				// 400 - 499
				// account pages
				"401" => array ("Headline" => _("Load profile"),
					"Text" => _("Here you can load an account profile to set default settings for your account. The \"default\" profile is automatically loaded for new accounts.")),
				"403" => array ("Headline" => _("Create PDF file"),
					"Text" => _("Here you can select a PDF structure and export the account to a PDF file.")),
				"404" => array ("Headline" => _("Password"),
					"Text" => _("Please enter the password which you want to set for this account. You may also generate a random password (12 characters) which will be displayed on your screen.")),
				"405" => array ("Headline" => _("PDF structure"),
					"Text" => _("The PDF structure defines what information is exported as PDF file and how the pages are structured. You can manage the PDF structures in the PDF editor (under \"Tools\").")),
				// 500 - 599
				// LAM Pro
				"501" => array ("Headline" => _("LDAP suffix"),
					"Text" => _("LAM will search for accounts in this part of the LDAP tree.")),				
				"502" => array ("Headline" => _("LDAP user and password"),
					"Text" => _("LAM will use this LDAP DN and password to search for accounts. It is sufficient to specify an account with read rights. If nothing is inserted then LAM will try to connect anonymously.")),				
				"503" => array ("Headline" => _("LDAP search attribute"),
					"Text" => _("This is needed to find the LDAP DNs of your user accounts. E.g. if you use \"uid\" and your user inputs \"miller\" then LAM will search for an account with uid=miller.")),
				"504" => array ("Headline" => _("Login attribute label"),
					"Text" => _("This text should shortly describe your selected LDAP search attribute (e.g. email or user name).")),				
				"505" => array ("Headline" => _("Login caption"),
					"Text" => _("This text is displayed on top of the self service login page. You can also input HTML code here.")),
				"506" => array ("Headline" => _("Main page caption"),
					"Text" => _("This text is displayed on top of the self service main page. You can also input HTML code here.")),
				"507" => array ("Headline" => _("Add new group"),
					"Text" => _("Adds a new group element to the list of self service options. Use this to structure the input fields.")),
				"508" => array ("Headline" => _("Add input field"),
					"Text" => _("Adds a new self service option to the selected group element.")),
				"509" => array ("Headline" => _("Page header"),
					"Text" => _("This HTML code will be placed on top of all self service pages. E.g. you can use this to place your custom logo. Any HTML code is permitted.")),
				"510" => array ("Headline" => _("Additional CSS links"),
					"Text" => _("Here you can specify additional CSS links to change the layout of the self service pages. This is useful to adapt them to your corporate design. Please enter one link per line.")),
				"520" => array ("Headline" => _("Generate random password"),
					"Text" => _("This will set a random password and display it on the screen or send it the user via mail.")),
				"550" => array ("Headline" => _("From address"),
					"Text" => _("This email address will be set as sender address of all password mails. If empty the system default (php.ini) will be used.")),
				"551" => array ("Headline" => _("Subject"),
					"Text" => _('The subject of all password mails. If empty the first line of "config/passwordMailTemplate.txt" will be used.')),
				"552" => array ("Headline" => _("Text"),
					"Text" => _('The mail text of all password mails. If empty the content of "config/passwordMailTemplate.txt" starting at the second line will be used.') .
						'<br><br><b>' . _('Format') . ': </b><ul>' .
						'<li>' . _('You can use wildcards for LDAP attributes in the form @@attribute@@ (e.g. @@uid@@ for the user name).') . '</li>' .
						'<li>' . _('The wildcard for the new password is @@newPassword@@.') . '</li>'),
				// 600 - 699
				// OU-editor, domain page
				"601" => array ("Headline" => _("OU-Editor") . " - " . _("New organizational unit"),
					"Text" => _("This will create a new organizational unit under the selected one.")),
				"602" => array ("Headline" => _("OU-Editor") . " - " . _("Delete organizational unit"),
					"Text" => _("This will delete the selected organizational unit. The OU has to be empty."))

);

/* This is a sample help entry. Just copy this line an modify the values between the [] brackets.

	"[HelpNumber]" => array ("Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => array('link' => "[SeeAlso link]", 'text' => '[link text]')),
	"999" => array ("Headline" => _("This is a sample entry"), "Text" => _("Sample text"), "SeeAlso" => array('link' => "http://www.ldap-account-manager.org/", 'text' => 'Project page')),



	Syntax:
	=======

		HelpNumber: numeric value
		Headline: topic of help entry
		Text: help text (ext = FALSE)
		SeeAlso: URL to additional information (ext = FALSE, optional)
		Link: relative URL to external help page (ext = TRUE)
*/
?>
