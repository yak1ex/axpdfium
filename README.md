axpdfium
========

This is a susie plugin for PDF files, using [PDFium](https://code.google.com/p/pdfium/) library. PDFium is distributed under the non-copyleft permissive license. Thus, there is no license issue like other PDF susie plugins using GPL-licensed libraries.

`axpdfium.txt` is written for Japanese end-users.


Build procedure
---------------

- Note that the latest PDFium requires 64bit env and MSVS 2017. Community edition suffices.
- Prepare prerequisites for building PDFium library
  - Open MSVS command prompt
  - [Install depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
    - Download https://storage.googleapis.com/chrome-infra/depot_tools.zip and extract somewhere like c:\depot_tools.
    - Add the extracted path to the *beginning* of the `PATH` environment variable.
    - Run `gclient`
      - Note that not only lock issues but also network issues (e.g. denied by access to the URL) may produce the messages like the followings:
      ```
CIPD lock is held, trying again after delay...
Bootstrapping cipd client for windows-386 from https://chrome-infra-packages.appspot.com/client?platform=windows-386&version=git_revision:4d19637ec2c3d1efd8c6a1b05285118b786919e2...
```
  - Install Python, Subversion and Git if necessary. If you use Chocolatey, you can install them by the followings:
    ```
choco install python2 subversion git.install -y
```
  - If not yet, set Git config vars.
  - Check visual studio setup: https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md#visual-studio
    - If you use Chocolatey, you can install them except for `Debugging Tools for Windows` by the followings. You need to change `Windows Software Development Kit` in `Programs and Features` in `Control Panel` to enable `Debugging Tools for Windows`
      ```
choco install visualstudio2017community -y
choco install visualstudio2017-workload-nativedesktop -y
```
    - If you changed the install location of MSVS, you may need to set envvar like `set "GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Application\Microsoft Visual Studio\2017"`
  - `set DEPOT_TOOLS_WIN_TOOLCHAIN=0`
- Get PDFium sources
  ```
mkdir pdfium
cd pdfium
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
```
  - `gclient sync` may fail inside proxy without config as the followings:
    ```
[Boto]
proxy=ProxyHost
proxy_port=ProxyPort
```
    `set NO_AUTH_BOTO_CONFIG=<path_to_above_file>`
  - gclient sync may fail if any of subkeys in HKEY_CLASSES_ROOT have non-ascii characters
- Make configuration and build PDFium library. You can check available options by `gn args --list out/Debug`
  ```
gn args out/Debug
ninja -C out/Debug pdfium_all
gn args out/Release
ninja -C out/Release pdfium_all
```
  ```
# Configuration example for 'gn args out/Debug'
pdf_enable_xfa = false
pdf_enable_v8 = false
pdf_is_standalone = true
is_component_build = false
clang_use_chrome_plugins = false
target_cpu = "x86"
```
  ```
# Configuration example for 'gn args out/Release
is_debug = false
pdf_enable_xfa = false
pdf_enable_v8 = false
pdf_is_standalone = true
is_component_build = false
clang_use_chrome_plugins = false
target_cpu = "x86"
```
- Set enviroment variables or MSVS variables: PDIFUM_ROOTPATH and PDFIUM_OBJPATH. PDFIUM_ROOTPATH points to where PDFium is checked out. PDFIUM_OBJPATH points to `obj` folder under the folder given to `gn args`.
  ```
PDFIUM_ROOTPATH=c:\pdfium\pdfium
PDFIUM_OBJPATH=c:\pdfium\pdfium\out\Debug\obj
```
- Build axpdfium

Used PDFium revision
--------------------

It is confirmed that this souce can be built against [ddfa5177021f76b4d84c6fbdce964ab3ca17e046](https://pdfium.googlesource.com/pdfium.git/+/ddfa5177021f76b4d84c6fbdce964ab3ca17e046)

License
-------

zlib/libpng License

Author
------

Yak! <yak1ex@users.noreply.github.com>
