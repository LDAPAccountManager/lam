
Some notes on managing Kolab accounts with LAM: 


1. LDAP Suffix

  You must create your accounts directly in the LDAP root (e.g. dc=company,dc=com).
  If you use other suffixes like ou=people,dc=company,dc=com the Kolab daemon will
  not recognize them (e.g. create no mailbox).


2. Deleting accounts

  If you want to cleanly delete accounts use the "Mark for deletion" button on the
  Kolab subpage of an account. This will also remove the user's mailbox.
  If you delete the account from the account list (which is standard for LAM accounts)
  then no cleanup actions are made.


3. Managing accounts with both LAM and Kolab Admin GUI

  The Kolab GUI has some restrictions that LAM does not have.
  Please pay attention to the following restrictions:

  - Common name in LAM
    The common name must have the format "<first name> <last name>".
    You can leave the field empty in LAM and it will automatically
    fill in the correct value.

  - Changing first/last name in Kolab GUI
    Do not change the first/last name of your users in the Kolab GUI!
    The GUI will change the common name which leads to an LDAP object class
    violation. This is caused by a bug in the Kolab GUI.