Upgrade instructions:
=====================

1.1.0 -> 2.2.0

No changes.


1.0.4 -> 1.1.0:
===============

If you use the lamdaemon.pl script to manage quotas and home directories please
read docs/README.lamdaemon.txt.



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
