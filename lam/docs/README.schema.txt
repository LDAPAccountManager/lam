
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


  5. Kolab 2 users (module kolabUser)
 
  Schema: kolab2.schema, rfc2739.schema
  Source: part of Kolab 2 installation

  
  6. Mail routing (module inetLocalMailRecipient)

  Schema: misc.schema
  Source: part of OpenLDAP installation


  7. Mail aliases (module nisMailAlias)

  Schema: misc.schema
  Source: part of OpenLDAP installation


  8. MAC addresses (module ieee802device)

  Schema: nis.schema
  Source: part of OpenLDAP installation

  
  9. Simple Accounts (module account)

  Schema: cosine.schema
  Source: part of OpenLDAP installation


  10. SSH public keys (module ldapPublicKey)

  Schema: openssh-lpk.schema
  Source: Included in patch from http://www.opendarwin.org/en/projects/openssh-lpk/
