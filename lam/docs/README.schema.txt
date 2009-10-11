
  Here is a list of needed LDAP schema files for the different LAM modules.
  For OpenLDAP we also provide a source where you can get the files.


  1. Unix accounts (modules posixAccount/shadowAccount/posixGroup)

  Schema: nis.schema
  Source: Part of OpenLDAP installation

  The rfc2307bis.schema is only supported by LAM Pro. Use the nis.schema
  if you do not want to upgrade to LAM Pro.


  2. Address book entries (module inetOrgPerson)
 
  Schema: inetorgperson.schema
  Source: Part of OpenLDAP installation

  
  3. Samba 3 accounts (modules sambaSamAccount)

  Schema: samba.schema
  Source: Part of Samba tarball (examples/LDAP/samba.schema)


  4. Quota (module quota)

  Schema: none


  5. Kolab 2 users (module kolabUser)
 
  Schema: kolab2.schema, rfc2739.schema
  Source: Part of Kolab 2 installation

  
  6. Mail routing (module inetLocalMailRecipient)

  Schema: misc.schema
  Source: Part of OpenLDAP installation


  7. Mail aliases (module nisMailAlias)

  Schema: misc.schema
  Source: Part of OpenLDAP installation


  8. MAC addresses (module ieee802device)

  Schema: nis.schema
  Source: Part of OpenLDAP installation

  
  9. Simple Accounts (module account)

  Schema: cosine.schema
  Source: Part of OpenLDAP installation


  10. SSH public keys (module ldapPublicKey)

  Schema: openssh-lpk.schema
  Source: Included in patch from http://www.opendarwin.org/en/projects/openssh-lpk/


  11. Group of (unique) names (modules groupOfNames/groupOfUniqueNames)

  These modules are only available in LAM Pro.
  Schema: core.schema
  Source: Part of OpenLDAP installation


  12. phpGroupWare (modules phpGroupwareUser, phpGroupwareGroup)

  Schema: phpgroupware.schema
  Source: http://www.phpgroupware.org/


  13. DHCP (modules dhcp_settings, ddns, fixed_ip, range)

  Schema: dhcp.schema
  Source: docs/schema/dhcp.schema
  The LDAP suffix should be set to your dhcpServer entry.

