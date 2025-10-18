# Instructions

This document describes the complete set of bytecode instructions understood by the VM and briefly describes their use.

> Note that the instruction set is still under construction and is subject to change until version 1.0.0.
>
> No guarantees of backwards compatibility are made at this time.

## Instructions

| Name         | Operands | Description                                                                                                           |
|--------------|----------|-----------------------------------------------------------------------------------------------------------------------|
| `EXIT`       |          | Causes the VM to shut down gracefully.                                                                                |
| `PUSH`       | index    | Pushes the value at index in the constant table to the top of the stack.                                              |
| `POP`        |          | Pops the top value from the stack. As a temporary debug step, prints the value popped (this will be removed in v0.3). |
| `F64_ADD`    |          | Adds (binary+) the two F64 values on the top of the stack and pushes the result.                                      |
| `F64_SUB`    |          | Subtracts (binary-) the two F64 values on the top of the stack and pushes the result.                                 |
| `F64_MUL`    |          | Multiplies (binary*) the two F64 values on the top of the stack and pushes the result.                                |
| `F64_DIV`    |          | Divides (binary/) the two F64 values on the top of the stack and pushes the result.                                   |
| `F64_NEG`    |          | Negates (unary-) the F64 value on the top of the stack and pushes the result.                                         |
| `F64_AFF`    |          | Affirms (unary+) the F64 value on the top of the stack and pushes the result. This is a no op.                        |
| `I64_ADD`    |          | Adds (binary+) the two int values on the top of the stack and pushes the result.                                      |
| `I64_SUB`    |          | Subtracts (binary-) the two int values on the top of the stack and pushes the result.                                 |
| `I64_MUL`    |          | Multiplies (binary*) the two int values on the top of the stack and pushes the result.                                |
| `I64_DIV`    |          | Divides (binary/) the two int values on the top of the stack and pushes the result.                                   |
| `I64_NEG`    |          | Negates (unary-) the int value on the top of the stack and pushes the result.                                         |
| `I64_AFF`    |          | Affirms (unary+) the int value on the top of the stack and pushes the result. This is a no op.                        |
| `U64_ADD`    |          | Adds (binary+) the two uint values on the top of the stack and pushes the result.                                     |
| `U64_SUB`    |          | Subtracts (binary-) the two uint values on the top of the stack and pushes the result.                                |
| `U64_MUL`    |          | Multiplies (binary*) the two uint values on the top of the stack and pushes the result.                               |
| `U64_DIV`    |          | Divides (binary/) the two uint values on the top of the stack and pushes the result.                                  |
| `U64_AFF`    |          | Affirms (unary+) the uint value on the top of the stack and pushes the result.   This is a no op.                     |
| `CAST_IU`    | width    | Cast an int to an unsigned int of the given width, widening or narrowing if necessary.                                |
| `CAST_UI`    | width    | Cast an unsigned int to an int of the given width, widening or narrowing if necessary.                                |
| `CAST_IF`    |          | Cast an int to an F64.                                                                                                |
| `CAST_UF`    |          | Cast an unsigned int to an F64.                                                                                       |
| `CAST_FI`    | width    | Cast an F64 to an int of the given width, widening or narrowing if necessary.                                         |
| `CAST_FU`    | width    | Cast an F64 to an unsigned int of the given width, widening or narrowing if necessary.                                |
| `CAST_WIDTH` | width    | Widens or narrows the int or unsigned int on the top of the stack to the desired width.                               |

## Width

| Width    | Description        |
|----------|--------------------|
| WIDTH_8  | 8-bit width (byte) |
| WIDTH_16 | 16-bit width       |
| WIDTH_32 | 32-bit width       |
| WIDTH_64 | 64-bit width       |

## Detailed Descriptions

> Still under construction.
>
> TODO: This section
