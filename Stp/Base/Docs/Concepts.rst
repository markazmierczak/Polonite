
**MoveConstructible**

   Move construction/assignment must be ``noexcept`` always.

**Swappable**

   ``void swap(T& x, T& y)``

**Relocatable**

   ``TIsTriviallyRelocatableTmpl``

**ZeroConstructible**

   ``TIsZeroConstructibleTmpl``

**Comparable**

   .. code::

      int compare(const T& lhs, const T& rhs);
      TIsTriviallyComparableTmpl<T>

**EqualityComparable**

   .. code::

      bool operator==(const T& lhs, const T& rhs);
      bool operator!=(const T& lhs, const T& rhs);
      TIsTriviallyEqualityComparableTmpl<T>

**Hashable**

   .. code::

      HashCode Hash(const T& value);

**Formattable**

   .. code::

      void Format(TextWriter& out, const T& value, const StringSpan& opts);
      TextWriter& operator<<(TextWriter& out, const T& value);
