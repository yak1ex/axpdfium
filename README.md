odstream
========

C++ iostream using `OutputDebugString()` Win32 API.

Usage
-----

### preparation ###

    make

builds `libodstream.a`. Include `odstream.hpp` and link `libodstream.a`

### code ###

1. Direct style

        yak::debug::ods << "Trace messages" << std::endl;

    If `DEBUG` macro is defined, the message is output by `OutputDebugString()`. Otherwise, ignored.  However, `std::endl` requires including `iostream` header  reagardless whether `DEBUG` is defined or not and it might cause a burden of executable size.

2. Macro style

        DEBUG_LOG(<< "Trace messages" << std::endl);

    This is simliar as direct style but need not including `iostream` when `DEBUG` is not defined.

To tell the truth, including `iostream` when `DEBUG` is defined is done by `odstream.hpp` header, so you need not it by yourself. To define `ODSTREAM_NO_INCLUDE_IOSTREAM` suppresses this behavior.

License
-------

This software is distributed under the terms of a zlib/libpng License.

> Copyright (c) 2013 Yak! / Yasutaka ATARASHI
>
> This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
>
> Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
>
> 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
>
> 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
>
> 3. This notice may not be removed or altered from any source distribution.
