%define httpd_rootdir @@HTTP_DIR@@
%define lam_dir lam
%define lam_uid @@USER@@
%define lam_gid @@GROUP@@
%define lam_distribution @@DISTRIBUTION@@
%define is_suse %(test suse = %lam_distribution && echo 1 || echo 0)
%define is_fedora %(test fedora = %lam_distribution && echo 1 || echo 0)
%define _binary_payload w9.bzdio
%define _source_payload w9.bzdio

Name:         ldap-account-manager
License:      GPL
Group:        Productivity/Networking/Web/Frontends
Version:      @@VERSION@@
Release:      0.%lam_distribution.1
Source0:      ldap-account-manager-%{version}.tar.gz
URL:          http://www.ldap-account-manager.org/
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}
Summary:      Administration of LDAP users, groups and hosts via Web GUI
Summary(de):  Administration von Benutzern, Gruppen und Hosts für LDAP-Server
Vendor:       Roland Gruber
Packager:     Roland Gruber <post@rolandgruber.de>
BuildArchitectures: noarch
AutoReqProv:  no
%if %is_suse
Requires:      php5
Requires:      php5-ldap
Requires:      php5-hash
Requires:      php5-gd
Requires:      perl
%endif
%if %is_fedora
Requires:      php
Requires:      perl
%endif


%description
LDAP Account Manager (LAM) runs on an existing webserver.
It manages user, group and host accounts. Currently LAM supports
these account types: Samba 3, Unix, Kolab 2, address book
entries, NIS mail aliases and MAC addresses. There is an integrated LDAP browser
to allow access to the raw LDAP attributes. You
can use templates for account creation and use multiple configuration
profiles. Account information can be exported as PDF file. There is also
a script included which manages quota and homedirectories.

%description -l de
LDAP Account Manager (LAM) läuft auf einem exisierenden Webserver.
LAM verwaltet Benutzer, Gruppen und Hosts. Zur Zeit werden folgende Account-Typen
unterstützt: Samba 3, Unix, Kolab 2, Addressbuch Einträge, NIS
mail Aliase und MAC-Addressen. Es gibt einen integrierten LDAP-Browser mit dem
man die LDAP-Einträge direkt bearbeiten kann. Zum Anlegen von Accounts können
Vorlagen definiert werden. Es können mehrere Konfigurations-Profile
definiert werden. Account-Informationen können als PDF exportiert
werden. Außerdem exisitiert ein Script mit dem man Quotas und
Home-Verzeichnisse verwalten kann.

%prep
%setup -n ldap-account-manager-%{version}

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}
cp -dR * $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT

%post
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/config
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/tmp
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/sess

%files
%defattr(-, root, root)
%doc COPYING HISTORY README docs/*
%config(noreplace) %{httpd_rootdir}/%{lam_dir}/config/profiles/default.*
%config(noreplace) %{httpd_rootdir}/%{lam_dir}/config/pdf/default.*
%config(noreplace) %{httpd_rootdir}/%{lam_dir}/config/selfService/default.*
%{httpd_rootdir}/%{lam_dir}

%changelog
* Sat Nov 07 2009 - Roland Gruber post@rolandgruber.de
- Added LAM manuals

* Sat Jul 26 2008 - Roland Gruber post@rolandgruber.de
- Added subpackage for lamdaemon

* Wed Jan 11 2006 - Iain Lea iain@bricbrac.de
- Updated for 1.0 series on Fedora Core 

* Mon Dec 12 2005 - Iain Lea iain@bricbrac.de
- Updated for 0.5.x series on Fedora Core 

* Sun Mar 21 2004 - TiloLutz@gmx.de
- Initial release 0.1.0 - 0.4.5


%package lamdaemon

Summary:      Quota and home directory management for LDAP Account Manager
Summary(de):  Verwaltung von Quotas und Heimatverzeichnissen für LDAP Account Manager
Group:        Productivity/Networking/Web/Frontends
AutoReqProv:  no
%if %is_suse
Requires:      perl
Requires:      sudo
%endif
%if %is_fedora
Requires:      perl
Requires:      sudo
%endif

%description lamdaemon
Lamdaemon is part of LDAP Account Manager. This package
needs to be installed on the server where the home directories
reside and/or quotas should be managed.

%description lamdaemon -l de
Lamdaemon ist Teil von LDAP Account Manager. Dieses Paket
wird auf dem Server installiert, auf dem Quotas und
Heimatverzeichnisse verwaltet werden sollen.

%files lamdaemon
%{httpd_rootdir}/%{lam_dir}/lib/lamdaemon.pl
%doc COPYING HISTORY README VERSION docs/*

