Upgrade instructions:
=====================

2.0.0 -> 2.1.0

Developers:

Style changes:
  - "fieldset.<type>edit fieldset" and "fieldset.<type>edit fieldset fieldset" were removed.
  - "table.<type>list input" changed to "table.<type>list input,select"

baseModule:
  - The class variable $base is no longer visible in child classes. Please use
    $this->getAccountContainer() to access the accountContainer object.



1.3.0 -> 2.0.0

Developers:

LAM is now PHP5 only. Several variables are now private and need to be accessed via functions.



1.2.0 -> 1.3.0:
===============

Users:

No changes.


Developers:

New lamList function:

 - listPrintTableCellContent(): This function allows you to control how the LDAP
   attributes are displayed in the table. This can be used to display links
   or binary data.

 - listPrintAdditionalOptions(): If you want to display additional conrols for a list
   please use this function. The controls will be placed under the account table.

No more lamdaemon commands via delete_attributes() and save_attributes() in account modules.
Please use these new functions to call lamdaemon directly:

 - preModifyActions()
 - postModifyActions()
 - preDeleteActions()
 - postDeleteActions()



1.1.x -> 1.2.0:
===============


Users:

No changes.


Developers:

API changes:
 - removed get_configDescription() from module interface



1.0.4 -> 1.1.0:
===============

Users:

If you use the lamdaemon.pl script to manage quotas and home directories please
read docs/README.lamdaemon.txt.


Developers:

API changes:
 - removed $post parameters from module functions (delete_attributes(),
   process_...(), display_html_...()). Use $_POST instead.
 - process_...() functions: returned messages are no longer grouped
   (e.g. return: array(array('INFO', 'headline', 'text'), array('INFO', 'headline2', 'text2')))



1.0.0 -> 1.0.2:
===============

Users:

No changes.


Developers:

New module functions:
  - getRequiredExtensions: Allows to define required PHP extensions
  - getManagedObjectClasses: Definition of managed object classes for this module
  - getLDAPAliases: list of LDAP alias names which are replaced by LAM
  - getManagedAttributes: list of LDAP attributes which are managed by this module

The LDAP attributes are no longer loaded by reading the LDAP schema. If your
module does not implement the load_attributes() function then you have to use
getManagedAttributes() or the meta data to specify them.

The class variable "triggered_messages" in baseModule was removed.



0.5.x -> 1.0.0:
===============

The architecture of LAM changed again.

Please enter the LAM configuration editor and edit your existing profiles.
You can now select which account lists should be displayed by selecting
the active account types ("Edit account types"). The settings for the LDAP
suffixes and the list attributes also moved on this page.

After saving all configuration profiles you can login to LAM. The Samba domain
editor under "Tools" no longer exists. This is now an account type just like
users or groups. The NIS mail aliases have their own account list, too.



0.4.x -> 0.5.0:
===============

There were some major changes since 0.4.x.

First enter the LAM configuration editor and check if all settings are correct. Since
LAM now supports a plugin architecture for all accounts you can select the needed
modules. Click on "Edit modules" and select which account types you want to manage.
Depending on which modules you selected there might be more configuration options.

Now save your settings and login to LAM. You will have to recreate all your account
profiles because the format changed. The profile editor can be found on the tools
page ("Tools" in the upper left corner).

The tools page also includes the new flexible file upload and the PDF editor.
You can specify yourself which attributes should show up in the PDF files. There
are also different PDF profiles possible.
