# SPEC file

%global c_vendor    %{_vendor}
%global gh_owner    %{_owner}
%global gh_project  %{_project}

Name:      %{_package}
Version:   %{_version}
Release:   %{_release}%{?dist}
Summary:   PHP library to generate linear and bidimensional barcodes

License:   LGPLv3+
URL:       https://github.com/%{gh_owner}/%{gh_project}

BuildArch: noarch

Requires:  php(language) >= 8.2.0
Requires:  php-composer(%{c_vendor}/tc-lib-color) < 3.0.0
Requires:  php-composer(%{c_vendor}/tc-lib-color) >= 2.13.1
Requires:  php-bcmath
Requires:  php-date
Requires:  php-gd
Requires:  php-pcre

Provides:  php-composer(%{c_vendor}/%{gh_project}) = %{version}
Provides:  php-%{gh_project} = %{version}

%description
PHP classes to generate linear and bidimensional barcodes:
CODE 39, ANSI MH10.8M-1983, USD-3, 3 of 9, CODE 93, USS-93,
Standard 2 of 5, Interleaved 2 of 5, CODE 128 A/B/C,
2 and 5 Digits UPC-Based Extension, EAN 8, EAN 13, UPC-A,
UPC-E, MSI, POSTNET, PLANET, RMS4CC (Royal Mail 4-state Customer Code),
CBC (Customer Bar Code), KIX (Klant index - Customer index),
Intelligent Mail Barcode, Onecode, USPS-B-3200, CODABAR, CODE 11,
PHARMACODE, PHARMACODE TWO-TRACKS, AZTEC, Datamatrix ECC200, QR-Code, PDF417.

Optional dependency: php-pecl-imagick

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
* Thu Jul 02 2026 Nicola Asuni <info@tecnick.com> 1.2.0-1
- Changed package name, add provides section
* Tue Feb 24 2026 Nicola Asuni <info@tecnick.com> 1.0.0-1
- Initial Commit
