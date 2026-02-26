# Calling Convention

This document describes the calling convention of the Fluir VM.
Specifically, it describes the way in which functions are called and the way in which arguments and return values are
passed between functions.

> Note that the document set is still under construction and is subject to change until version 1.0.0.
>
> No guarantees of backwards compatibility are made at this time.

## Description

The caller first allocates space for the return value on the stack, either zero or one slot depending on whether the
callee has a return value. Next, the caller pushes the arguments onto the stack in order. Finally, the caller invokes
the `CALL` instruction with the callee as an argument.

The VM will store the return address and set up the stack frame for the callee. The callee's stack frame begins at the
return slot and contains the arguments.

If the callee has a return value, it will be written to the reserved return slot on the stack.

After the callee returns, it leaves the return value on the stack and pops its arguments off. The callee can use the
return slot as needed.

## Example

Below is an example of what the stack looks like when a function is called. The function `add` takes two
i32 arguments and returns an i64, as an example. The caller will invoke `add` with 13 and 14 as arguments.

```
# Initial stack
[ 1.4][ 23 ][    ][    ][    ][    ]
# Caller allocates return slot
[ 1.4][ 23 ][    ][    ][    ][    ]
             ^ empty
# Caller pushes the arguments
[ 1.4][ 23 ][    ][ 13 ][ 14 ][    ]
# `add` is called, it's stack frame is set up
[ 1.4][ 23 ][    ][ 13 ][ 14 ][    ]
            ^ frame start
# As part of its operation, `add` writes the return value to the return slot
[ 1.4][ 23 ][ 27 ][ 13 ][ 14 ][    ]
            ^ frame start
# `add` pops its arguments off the stack and returns, leaving its return value on the top of the stack
[ 1.4][ 23 ][ 27 ][    ][    ][    ]
```
