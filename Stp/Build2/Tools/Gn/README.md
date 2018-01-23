# Update Clang

GN binaries are downloaded from Google storage.
First update sha1 sums from https://cs.chromium.org/chromium/src/buildtools .
Do it for all platforms.
Then run following command for each `\*.sha1` file:
```bash
download_from_google_storage --no_auth --bucket=chromium-gn -s Stp/Build/Tools/Gn/GnWin.exe.sha1
```
`download_from_google_storage` script is available in Google's depot tools.

GN binaries are stored in Git LFS.
