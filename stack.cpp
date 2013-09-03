///\file stack.cpp
///\brief Implementation of Stack-related convenience functions
#include "actions.h"
#include "stack.h"

/// The symbol stack is a buffer for user input which does not
/// translate directly into single-symbol command.
Stack<MorseToken, 256> tokenStack;

/// Report the stack size. This function provides a little insulation
/// so that we can change the implementation.
size_t symbolStackSize() {
  return tokenStack.size();
}

/// Access the stack using an index where 0 is the top of the stack.
MorseToken s(size_t i) {
  MorseToken result(0);
  if (i < symbolStackSize()) {
    result = tokenStack[i];
  }
  return result;
}

/// Push a symbol on to the symbol stack. If the stack is already full,
/// simply overwrite the top of the stack.
void pushSymbolStack(MorseToken code) {
  tokenStack.push(code);
}

/// Discard the top element of the stack unless the stack is empty in
/// which case do nothing.
void popSymbolStack(MorseToken) {
  tokenStack.pop();
}

/// Swap s0 and s1.
void exchangeStack(MorseToken) {
  if (1 < tokenStack.size()) {
    tokenStack.exchange();
  } else {
    txError();
  }
}

/// Discard all elements of the stack.
void clearSymbolStack(MorseToken) {
  tokenStack.clear();
}

void popN(uint8_t n) {
  uint8_t i(n);
  while (i-- && tokenStack.size()) {
    tokenStack.pop();
  }
}

uint32_t stackDigitsToUnsigned() {
   uint32_t n(0);
   uint32_t place(1);
   // Gobble up the digits on the stack, converting to a number.
   // Note, the cast is to insure that the range check can be 
   // done in a single compare.
   while (symbolStackSize() && (uint8_t(s(0).toChar()) - '0' < 10)) {
      uint32_t d(s(0).m2i());
      n += place * d;
      place *= 10;
      popN(1);
   }
   return n;
}
