# SPEC file

%global c_vendor    %{_vendor}
%global gh_owner    %{_owner}
%global gh_project  %{_project}

Name:      %{_package}
Version:   %{_version}
Release:   %{_release}%{?dist}
Summary:   PHP PDF Fonts Library

License:   LGPL-3.0+
URL:       https://github.com/%{gh_owner}/%{gh_project}

BuildArch: noarch

Requires:  php(language) >= 8.2.0
Requires:  php-json
Requires:  php-pcre
Requires:  php-zlib
Requires:  php-composer(%{c_vendor}/tc-lib-file) < 3.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-file) >= 3.7.1
Requires:  php-composer(%{c_vendor}/tc-lib-unicode-data) < 3.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-unicode-data) >= 2.7.1
Requires:  php-composer(%{c_vendor}/tc-lib-pdf-encrypt) < 3.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-pdf-encrypt) >= 2.9.1
Requires:  php-composer(%{c_vendor}/tc-lib-pdf-font-data-core) < 2.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-pdf-font-data-core) >= 1.8.7

Provides:  php-composer(%{c_vendor}/%{gh_project}) = %{version}
Provides:  php-%{gh_project} = %{version}

%description
PHP library containing PDF font methods and utilities

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
* Mon Aug 10 2026 Nicola Asuni <info@tecnick.com> 1.0.0-1
- Initial Commit
