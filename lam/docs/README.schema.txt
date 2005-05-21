
  Here is a list of needed LDAP schema files for the different LAM modules.
  For OpenLDAP we also provide a source where you can get the files.


  1. Unix accounts (modules posixAccount/shadowAccount/posixGroup)

  Schema: nis.schema
  Source: part of OpenLDAP installation

  Suse Linux: Do not use the rfc2307bis.schema but nis.schema instead.


  2. Address book entries (module inetOrgPerson)
 
  Schema: inetorgperson.schema
  Source: part of OpenLDAP installation

  
  3. Samba 2/3 accounts (modules sambaAccount/sambaSamAccount)

  Schema: samba.schema
  Source: part of Samba tarball (examples/LDAP/samba.schema)


  4. Quota (module quota)

  Schema: none


  5. Mail routing (module inetLocalMailRecipient)

  Schema: misc.schema
  Source: part of OpenLDAP installation


  6. Mail aliases (module nisMailAlias)

  Schema: misc.schema
  Source: part of OpenLDAP installation


  7. MAC addresses (module ieee802device)

  Schema: nis.schema
  Source: part of OpenLDAP installation

  8. Simple Accounts (module account)

  Schema: cosine.schema
  Source: part of OpenLDAP installation
