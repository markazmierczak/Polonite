// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

/**
 * @class ReadWriteLock
 *
 * An OS-independent wrapper around reader-writer locks. There's no magic here.
 *
 * @note
 * You are strongly encouraged to use Lock instead of this, unless you
 * can demonstrate contention and show that this would lead to an improvement.
 * This lock does not make any guarantees of fairness, which can lead to writer
 * starvation under certain access patterns. You should carefully consider your
 * writer access patterns before using this lock.
 */
