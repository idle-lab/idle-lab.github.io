
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

- Update Struct Syntax: 
