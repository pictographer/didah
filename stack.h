// -*- mode: C++; -*-
#pragma once

// Fixed-sized stack for plain old data. No memory allocation or
// deallocation is performed. 

template <typename T, size_t N>
class Stack {
public:
  Stack() : n(0) {}
  // Return the number of elements currently in the stack. 
  size_t Size() { return n; }
  // Return the element i places from the top of the stack.
  T S(size_t i) {
    if (i < n) {
      return s[n - 1 - i];
    } else {
      return T();
    }
  }
  // Push t onto the stack, if there is space for it.
  void Push(T t) {
    if (n + 1 < N) {
      s[n++] = t;
    }
  }
  // Discard the top element, if the stack is not empty.
  void Pop() {
    if (n) {
      --n;
    }
  }
  // Swap the top two elements of the stack.
  void Exchange() {
    if (0 < n) {
      T t(s[n - 1]);
      s[n - 1] = s[n - 2];
      s[n - 2] = t;
    }
  }
  // Discard the contents of the stack.
  void Clear() {
    n = 0;
  }
private:
  size_t n;  		// number of elements
  T s[N];			// element storage
};
