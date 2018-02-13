# File Path

`FilePath` is a container for pathnames stored in a platform's native string type, providing containers for manipulation in according with the platform's conventions for pathnames.
It supports the following path types:

|                  | POSIX              | Windows                                |
|------------------|--------------------|----------------------------------------|
| Fundamental type | `char[]`           | `wchar_t[]`                            |
| Encoding         | unspecified*       | UTF-16**                               |
| Separator        | `/`                | `\`, tolerant of `/`                   |
| Drive letters    | no                 | case-insensitive A-Z followed by `:`   |
| Alternate root   | `//` (surprise!)   | `\\`, for UNC paths                    |

* The encoding need not be specified on POSIX systems, although some
  POSIX-compliant systems do specify an encoding. Mac OS X uses UTF-8.
  Linux does not specify an encoding, but in practice, the locale's
  character set may be used.

* The characters are not validated by driver.  
  Any of `0..0xFFFF` character can be used (excluding reserved characters).
  Invalid surrogate pairs are possible.

For more arcane bits of path trivia, see :ref:`below <stp-base-file-path-arcane>`.

On most platforms, native pathnames are char arrays, and the encoding may or may not be specified. On Mac OS X, native pathnames are encoded in UTF-8.
On Windows, for Unicode-aware applications, native pathnames are `wchar_t` arrays encoded in UTF-16.

## Operations

`FilePath` objects are intended to be used anywhere paths are.
An application may pass `FilePath` objects around internally, masking the underlying differences between systems, only differing in implementation where interfacing directly with the system. For example, a single `OpenFile()` function may be made available, allowing all callers to operate without regard to the underlying implementation.

On POSIX-like platforms, `OpenFile` might wrap `fopen`, and on Windows, it might wrap `_wfopen_s`, perhaps both by calling `ToNullTerminated(file_path)`.
This allows each platform to pass paths around without requiring conversions between encodings, which has an impact on performance, but more importantly, has an impact on correctness on platforms that do not have well-defined encodings for paths.

Several methods are available to perform common operations on a `FilePath` object, such as determining the parent directory (`GetDirName()`), isolating the final path component (`GetBaseName()`), and appending a relative pathname string to an existing `FilePath` object (`Append()`).
These methods are highly recommended over attempting to split and concatenate strings directly. These methods are based purely on string manipulation and knowledge of platform-specific pathname conventions, and do not consult the filesystem at all, making them safe to use without fear of blocking on I/O operations.

To aid in initialization of `FilePath` objects from string literals, a `FILE_PATH_LITERAL` macro is provided, which accounts for the difference between platforms.

As a precaution against premature truncation, paths can't contain nulls.

.. _stp-base-file-path-arcane:

## Arcane Bits of Path Trivia

### Alternate Root on POSIX

A double leading slash is actually part of the POSIX standard.
Systems are allowed to treat `//` as an alternate root, as Windows does for UNC (network share) paths.
Most POSIX systems don't do anything special with two leading slashes, but `FilePath` handles this case properly in case it ever comes across such a system. `FilePath` needs this support for Windows UNC paths, anyway.

#### References

The Open Group Base Specifications Issue 7, sections 3.267 ("Pathname") and 4.12 ("Pathname Resolution"), available at:

* http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_267
* http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_12

### Alternate Root on Windows

Windows treats `c:\\` the same way it treats `\\`.  This was intended to allow older applications that require drive letters to support UNC paths like `\\server\share\path`, by permitting `c:\\server\share\path` as an equivalent.

Since the OS treats these paths specially, `FilePath` needs to do the same.

Since Windows can use either `/` or `\` as the separator,
`FilePath` treats `c://`, `c:\\`, `//`, and `\\` all equivalently.

#### References

The Old New Thing, "Why is a drive letter permitted in front of UNC
paths (sometimes)?", available at:
http://blogs.msdn.com/oldnewthing/archive/2005/11/22/495740.aspx

## (Not)ToStringConvertible

`FilePath` is **not** convertible to `String` on purpose.
Such conversion would be **lossy**.
