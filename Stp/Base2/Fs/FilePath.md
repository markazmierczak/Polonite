# File Path

On most platforms, native pathnames are char arrays, and the encoding may or may not be specified. On Mac OS X, native pathnames are encoded in UTF-8.
On Windows, for Unicode-aware applications, native pathnames are `wchar_t` arrays encoded in UTF-16.
