# SPEC file

%global c_vendor    %{_vendor}
%global gh_owner    %{_owner}
%global gh_project  %{_project}

Name:      %{_package}
Version:   %{_version}
Release:   %{_release}%{?dist}
Summary:   PHP library containing Unicode methods

License:   LGPLv3+
URL:       https://github.com/%{gh_owner}/%{gh_project}

BuildArch: noarch

Requires:  php(language) >= 8.2.0
Requires:  php-composer(%{c_vendor}/tc-lib-unicode-data) < 3.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-unicode-data) >= 2.7.1
Requires:  php-pcre
Requires:  php-mbstring

Provides:  php-composer(%{c_vendor}/%{gh_project}) = %{version}
Provides:  php-%{gh_project} = %{version}

%description
PHP library containing Unicode methods

%build
#(cd %{_current_directory} && make build)

%install
rm -rf "%{buildroot}"
(cd "%{_current_directory}" && make install DESTDIR="%{buildroot}")

%files
%attr(-,root,root) %{_libpath}
%attr(-,root,root) %{_docpath}
%docdir %{_docpath}
# Optional config files can be listed here when used by a project.

%changelog
* Tue Jul 21 2026 Nicola Asuni <info@tecnick.com> 1.0.0-1
- Initial Commit
