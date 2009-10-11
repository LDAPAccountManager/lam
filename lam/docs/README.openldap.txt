Some basic hints to configure the OpenLDAP server:

SIZELIMIT:  OpenLDAP allows by default 500 return values per search, if you have more users/groups/hosts
            change this in slapd.conf: e.g. "sizelimit 10000" or "sizelimit -1" for unlimited return values.

INDICES:  Indices will improve the performance when searching for entries in the LDAP directory.
          The following indices are recommended:

          index objectClass eq
          index default sub
          index uidNumber eq
          index gidNumber eq
          index memberUid eq
          index cn,sn,uid,displayName pres,sub,eq
          # Samba 3.x
          index sambaSID eq
          index sambaPrimaryGroupSID eq
          index sambaDomainName eq
