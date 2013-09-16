<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010         Cedric Dugas and Olivier Refalo
                2011 - 2013  Roland Gruber

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

/** access to configuration options */
include_once("../../lib/config.inc"); // Include config.inc which provides Config class

// set session save path
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../../sess');
}

// start empty session and change ID for security reasons
@session_start();
setlanguage();

?>

(function($){
    $.fn.validationEngineLanguage = function(){
    };
    $.validationEngineLanguage = {
        newLang: function(){
            $.validationEngineLanguage.allRules = {
                "required": {
                    "regex": "none",
                    "alertText": "<?php echo _('This field is required.'); ?>"
                },
                "numeric": {
                    "regex": /^[0-9]+$/,
                    "alertText": "<?php echo _('Please enter a number.') ?>"
                },
                "numericWithNegative": {
                    "regex": /^[-]?[0-9]+$/,
                    "alertText": "<?php echo _('Please enter a number.') ?>"
                }
            };
        }
    };
    $.validationEngineLanguage.newLang();
})(jQuery);
