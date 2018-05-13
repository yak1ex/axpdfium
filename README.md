axpdfium
========

This is a susie plugin for PDF files, using [PDFium](https://code.google.com/p/pdfium/) library. PDFium is distributed under the non-copyleft permissive license. Thus, there is no license issue like other PDF susie plugins using GPL-licensed libraries.

`axpdfium.txt` is written for Japanese end-users.


Build procedure
---------------

- Build PDFium library
  - Open MSVS command prompt
  - [Install depot_tools](https://dev.chromium.org/developers/how-tos/install-depot-tools)
    - Download and extract `depot_tools.zip`
    - Set `PATH` environment variable appropriately
    - Run `gclient`
  - `set DEPOT_TOOLS_WIN_TOOLCHAIN=0`
  - `set GYP_MSVS_VERSION=2017` if VS2017 is used
  - If you changed VS2017 install location from default, you should set `vs2017_install` environment variable as the actual install location.
  - Get PDFium sources
    ```
    mkdir pdfium
    cd pdfium
    gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
    gclient sync
    cd pdfium
    ```
  - Make configuration and build PDFium library. You can check available options by `gn args --list out/Debug`
    ```
    gn args out/Debug
    ninja -C out/Debug pdfium_all
    gn args out/Release
    ninja -C out/Release pdfium_all
    ```
    ```
    # Configuration example for 'gn args out/Debug'
    is_component_build = false
    pdf_enable_v8 = false
    ```
    ```
    # Configuration example for 'gn args out/Release
    is_component_build = false
    pdf_enable_v8 = false
    is_debug = false
    ```

License
-------

zlib/libpng License

Author
------

Yak! <yak_ex@mx.scn.tv>
