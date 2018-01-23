
## MoveConstructible

Move construction/assignment must be `noexcept` always.

## Swappable

```c++
void Swap(T& x, T& y);
TIsTriviallyRelocatable
```

## Relocatable

```c++
TIsTriviallyRelocatable
```

## ZeroConstructible

```c++
TIsZeroConstructible
```

## Comparable

```c++
int Compare(const T& lhs, const T& rhs);
TIsTriviallyComparable
```
## EqualityComparable

```c++
bool operator==(const T& lhs, const T& rhs);
bool operator!=(const T& lhs, const T& rhs);
TIsTriviallyEqualityComparable
```

## Hashable

```c++
HashCode Hash(const T& value);
```

## Formattable

```c++
void Format(TextWriter& out, const T& value, const StringSpan& opts);
TextWriter& operator<<(TextWriter& out, const T& value);
```

# Containers

## ConiguousContainer

```c++
TIsContiguousContainer
```
