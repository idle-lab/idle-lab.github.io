---
comments: true
---

## **What Is Ownership?**

Ownership is a set of rules that govern how a Rust program manages memory. As everyone knows, memory is divided into serveral partions, Heap, Stack, Code, .

Simple value (i.e. i32, float32, char...) are allocated on the stack space.And some data structure like `Vec` or smart pointer `Box` are allocated on the heap. All of this values follow the following rules:

- Each value in Rust has an owner.

- There can only be one owner at a time.

- When the owner goes out of scope, the value will be dropped.

A variable can own a value or borrow value from anther variable. 

## **Ownership Move**

Like `unique_ptr` in C++, variable's ownership can only move, not copy.

```rust
let s1 = String::from("cyb");
let s2 = s1; // ownership is move to s2.
```

If you want copy, need to implement `Copy()` or `Clone()`, the former performs shallow copying while the latter performs deep copying. Note that only your struct doesn't contain memory in heap, you can implement `Copy()`.

```rust
#[derive(Copy)] // derive Copy
struct Point {
    x: i32,
    y: i32,
}

// You can't implement Copy in this struct.
struct MyStruct {
    name: String, // String has memeory in heap.
}
```

## **References and Borrowing**

We can use `&` to borrow a value from anther variable. We don't have the ownership of the value, so we don't drop the value when we go out of scope.Look example follow:

```rust
let mut s = String::from("hello");

{
    let r1 = &mut s;
} // r1 goes out of scope here, so we can make a new reference with no problems.

let r2 = &mut s;
```

If you want modify the value, you can use `&mut` to get a mutable references. But in the same scope, a variable can only have one mutable reference. With the lifetimes concept that we will discuss in the following chapters, we can check the *Dangling References* during compilation period. 

```rust
fn main() {
    let reference_to_nothing = dangle();
}

fn dangle() -> &String {
    let s = String::from("hello");

    &s
}
```

Compiler report error message like follow:

```
$ cargo run
   Compiling ownership v0.1.0 (file:///projects/ownership)
error[E0106]: missing lifetime specifier
 --> src/main.rs:5:16
  |
5 | fn dangle() -> &String {
  |                ^ expected named lifetime parameter
  |
  = help: this function's return type contains a borrowed value, but there is no value for it to be borrowed from
help: consider using the `'static` lifetime, but this is uncommon unless you're returning a borrowed value from a `const` or a `static`
  |
5 | fn dangle() -> &'static String {
  |                 +++++++
help: instead, you are more likely to want to return an owned value
  |
5 - fn dangle() -> &String {
5 + fn dangle() -> String {
  |

error[E0515]: cannot return reference to local variable `s`
 --> src/main.rs:8:5
  |
8 |     &s
  |     ^^ returns a reference to data owned by the current function

Some errors have detailed explanations: E0106, E0515.
For more information about an error, try `rustc --explain E0106`.
error: could not compile `ownership` (bin "ownership") due to 2 previous errors
```

## **Slice**

Same as go, you can get a mutable(`&mut`) or immutable(`&`) slice from a sequence values.

```rust
let a = [1, 2, 3, 4, 5];

let slice = &a[1..3];

let s = String::from("hello");

let len = s.len();

let slice = &s[3..len];
let slice = &s[3..];
```

There's nothing to say.

## **Summarize**

The ownership concept provides a different way to manage memory that safer than C++ and more efficient than Java or golang.I think this design is very genius.

---
