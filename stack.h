// -*- mode: C++; -*-
#pragma once
/// Fixed-sized stack for plain old data.

/// No memory allocation or deallocation is performed.

///\file stack.h

///\template Stack
///\brief Fixed-sized stack for plain old data
///\tparam T Element type
///\tparam N Maximum number of elements
template <typename T, size_t N>
class Stack {
public:
   Stack() : n(0) {}
   /// Return the number of elements currently in the stack. 
   size_t size() { return n; }
   /// Return the element i places from the top of the stack.
   /// If i is out of range, return the default object.
   T operator[] (size_t i) {
      if (i < n) {
         return s[n - 1 - i];
      } else {
         return T();
      }
   }
   /// Push t onto the stack, if there is space for it.
   void push(T t) {
      if (n + 1 < N) {
         s[n++] = t;
      }
   }
   /// Discard the top element, if the stack is not empty.
   void pop() {
      if (n) {
         --n;
      }
   }
   /// Swap the top two elements of the stack.
   void exchange() {
      if (0 < n) {
         T t(s[n - 1]);
         s[n - 1] = s[n - 2];
         s[n - 2] = t;
      }
   }
   /// Discard the contents of the stack.
   void clear() {
      n = 0;
   }
private:
   /// Current element count.
   size_t n;			
   /// Element storage.
   T s[N];
};

/// Pop the stack n times or until it is empty.
void popN(uint8_t n);

/// Interpret the digits on the stack as an unsigned 32-bit integer.
///\note There is no overflow check. 0 to 2147483647 should work.
uint32_t stackDigitsToUnsigned();

///\name Stack Wrapper Functions
//{@

/// Fetch element i of the stack. The top element is s(0). No error checking
/// is done.
MorseToken s(size_t i);
size_t symbolStackSize();
void pushSymbolStack(MorseToken code);
void popSymbolStack(MorseToken);
void exchangeStack(MorseToken);
void clearSymbolStack(MorseToken);

//@}
