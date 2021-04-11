# LLANG

Basic LLVM based compiler with javascript-like syntax

 * Static type checking
 * Compiles to x86_64 object files
 * C/asm/etc-interop (no varargs)
 * Structures
 * Functions
 * Variables
 * Pointers

## Building

Requires `llvm`, `clang`, `boost` and `gtest`

### Ubuntu/Debian

```shell
sudo apt-get install llvm clang libboost-all-dev libgtest-dev
cmake .
make run # build, compile example, link and run example
make llang # build binary file
make test # run tests
make lint # lint (style check)
make format # format all files
```

## Example

```typescript
function puts(string: str): void;
function malloc(size: i64): void*;
function free(ptr: str): void;
function sprintf(ptr: str, val: str, t: i64): i64;

struct MyStruct {
  a: i64;
  b: i64;
  n: i64;
}

function fib(ms: MyStruct*): void {
  let out: str = malloc(10)
  ms->a = 1
  ms->b = 1
  for (let i: i64 = 0; i < ms->n; i += 1) {
    let temp: i64 = (*ms).b
    ms->b += ms->a
    ms->a = temp
    sprintf(out, "%lld", ms->b)
    puts(out)
  }
  free(out)
  return;
}

function main(argc: i64, argv: str*): i64 {
  let init: MyStruct = MyStruct {
    a: 1,
    b: 1,
    n: 10,
  };
  fib(&init)
  return 0
}
```