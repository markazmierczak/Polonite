// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

/**
 * \fn NativeThread::setName(const char* name)
 *
 * You probably don't want to set it for main thread. That will rename
 * the entire process on Linux, causing tools like killall to stop working.
 */
