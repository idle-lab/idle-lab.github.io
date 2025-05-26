
## **Syntax**

Struct in Rust is similar with it in C++ is a custom data type that lets you package together and name multiple related values that make up a meaningful group.

You can use the following syntax to define structures:

```rust
struct User {
    active: bool,
    username: String,
    email: String,
    sign_in_count: u64, // The last field should also have a comma 
}
```

When we init a struct instance first, we need to specify values for **all** fields:

```rust
fn main() {
    let u = User{
        active: true,
        username: String::from("user"),
        email: String::from("123456@example.com"),
        sign_in_count: 1,
    };
}
```

Rust provides some syntactic sugar to help us initialize structures:

- Using the Field Init Shorthand: If a variable has the same name as a field in a structure, we can initialize that field directly with the variable

```rust
fn build_user(email: String, username: String) -> User {
    User {
        active: true,
        username,
        email,
        sign_in_count: 1,
    }
}
```

- Update Struct Syntax: If you want to initialize a struct from another struct and update some field, you can use the following syntax.

```rust
fn build_user(u: User) -> User {
    let u2 = User{
        active: false,
        ..u
    };```
}
```


- Tuple struct: A tuple-like struct.

```rust
struct Color(i32, i32, i32);
struct Point(i32, i32, i32);

fn main() {
    let black = Color(0, 0, 0);
    let origin = Point(0, 0, 0);
}
```


- Unit-Like Structs: You can define structs that don’t have any fields. Unit-like structs can be useful when you need to implement a trait on some type but don’t have any data that you want to store in the type itself.

```rust
struct AlwaysEqual;

fn main() {
    let subject = AlwaysEqual;
}
```

??? tip "Format output"
    



## **Method**

We can define methods for a struct as following syntax:

```rust
#[derive(Debug)]
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    fn area(&self) -> u32 {
        self.width * self.height
    }
}

fn main() {
    let rect1 = Rectangle {
        width: 30,
        height: 50,
    };

    println!(
        "The area of the rectangle is {} square pixels.",
        rect1.area()
    );
}
```