# What are vtables

If you've been browsing the source code you may have already stumbled upon some strange macros such as IK_INTERFACE, IK_IMPLEMENT, IK_OVERRIDE. Or maybe you opened some header files and realized that there are no function declarations anywhere, only structs with function pointers.

Different types of solvers required different types of data -- for example if we call ```ik_solver_FABRIK_create(IK_FABRIK)``` we also want to use the ```ik_node_FABRIK_create()``` functions and **NOT** the ```ik_node_base_create()``` functions because base nodes don't have the required fields that FABRIK nodes have. To help address this, it was necessary to implement a kind of "inheritance" mechanism using interface structs and "virtual function tables" (vtables).

# The problem

For example, let's imagine we had the following structure

```
        ______________
       | solver_iface |
       |--------------|
       | solve()      |
       |______________|
         /          \
    ____/_____     __\_______
   | solver1  |   | solver2  |
   |----------|   |----------|
   | solve1() |   | solve2() |
   |__________|   |__________|
```

How do we allocate a "solver" object which, when passed to solve(solver), calls either solve1() or solve2() depending on which implementation we want? In C, one might write:

```c
struct solver_t {
    void (*solve)(void);
};

void solve1(struct solver_t* solver) {
    /* this is the solver_1 implementation */
}

void solve2(struct solver_t* solver) {
    /* this is the solver2 implementation */
}

struct solver_t* solver1_create(void) {
    struct solver_t* solver = malloc(sizeof *solver);
    solver->solve = solve1;
    return solver;
}

struct solver_t* solver2_create(void) {
    struct solver_t* solver = malloc(sizeof *solver);
    solver->solve = solve2;
    return solver;
}
```

With this setup, we can now implement the solve() function to be:

```c
void solve(struct solver_t* solver) {
    solver->solve(solver);
}
```

# More than 1 function

What you saw was a simple example and it works, but it comes with a few problems: As more and more functions are added, the size of the struct starts to grow larger, and you have to remember to update every derived implementation to write the correct function pointers to each field. If you forget to do so, the compiler won't even generate a warning because this happens at runtime!

As it turns out, these function pointers actually never change after the object is allocated. In fact, they never change **period**. And better yet, every instance of a derived object always has the same list of function pointers! So a far better approach would be something along the lines of this:

```c
struct solver_iface_t {
    void (*solve)(void);
    /* other functions */
};

void solve1(struct solver_t* solver) {
    /* this is the solver_1 implementation */
}
void solve2(struct solver_t* solver) {
    /* this is the solver2 implementation */
}

/* statically initialize two "interfaces" that contain a list of functions for each solver implementation */
static const struct solver_iface_t solver1 = {
    solve1,
    /* other functions */
};
static const struct solver_iface_t solver2 = {
    solve2,
    /* other functions */
};
```

Now it's possible to allocate each solver and simply store a pointer to the correct interface rather than having to maticulously write to each function pointer individually. Not only that, but every time the interface changes, the compiler will actually generate an error if you forgot to update any of the ```solver1``` or ```solver2``` structs!

```c
struct solver_t {
    const struct solver_iface_t* vtable;
};

struct solver_t* solver1_create(void) {
    struct solver_t* solver = malloc(sizeof *solver);
    solver->vtable = &solver1;
    return solver;
}

struct solver_t* solver2_create(void) {
    struct solver_t* solver = malloc(sizeof *solver);
    solver->vtable = solver2;
    return solver;
}
```

With this new approach, the solve() function is implemented as:

```cpp
void solve(struct solver_t* solver) {
    solver->vtable->solve(solver);
}
```

# Automating vtable generation

So now that you understand the mechanism this library uses for dispatching to the correct function call, you might still complain that this requires a lot of manual book keeping -- and you'd be right. That's why there's a script called ik_gen_vtables.py in the root directory which helps automate this process.

In order to understand how this works, let's look at a real example. Consider the following inheritance diagram:

```
             ______________
            | solver_iface |
            |______________|
                   |
             ______|_______
            | solver_base  |
            |______________|
              /         \
    _________/_____    __\_______________
   | solver_FABRIK |  | solver_ONE_BONE  |
   |_______________|  |__________________|
```

First, the interface. If you look at the file ik/solver.h you will find the following code:

```c
IK_INTERFACE(solver_interface)
{
    uintptr_t (*type_size)(void);
    struct ik_solver_t* (*create)(enum ik_algorithm_e algorithm);
    void (*destroy)(struct ik_solver_t* solver);
    ik_ret (*construct)(struct ik_solver_t* solver);
    void (*destruct)(struct ik_solver_t* solver);
    /* etc. */
};
```

IK_INTERFACE(x) actually just expands to ```struct ik_##x##_t```, that is, ```struct ik_solver_interface_t```. Why not just write it as a struct? Because IK_INTERFACE is an identifier that will be picked up by the script and it tells it that this is an interface that needs to be implemented.

The IK_INTERFACE() structure should contain all of the function pointers that either the user of the library should have access to or inheriting implementations should implement.

Next, the base solver implementation. Here is where the fun starts. If you take a look at the file ik/vtables/solver_base.v, you will see it contains just two lines of code:

```c
#include "ik/solver.h"

IK_IMPLEMENT(solver_base, solver_interface)
```

When writing a .v file it is important to #include the header(s) that contain(s) the IK_INTERFACE() structure(s) you wish to implement.

What does IK_IMPLEMENT(solver_base, solver_interface) do? It tells the script that there exist a set of functions that directly map onto the functions listed in IK_INTERFACE(solver_interface), and these functions shall have the prefix ```ik_solver_base_XXX```. When the script runs on this .v file, it will generate the following header file:

```c
IK_PRIVATE_API uintptr_t ik_solver_base_type_size(void);
IK_PRIVATE_API struct ik_solver_t* ik_solver_base_create(enum ik_algorithm_e algorithm);
IK_PRIVATE_API void ik_solver_base_destroy(struct ik_solver_t* solver);
IK_PRIVATE_API ik_ret ik_solver_base_construct(struct ik_solver_t* solver);
IK_PRIVATE_API void ik_solver_base_destruct(struct ik_solver_t* solver);
/* etc... */
#define IK_SOLVER_BASE_IMPL \
    ik_solver_base_type_size, \
    ik_solver_base_create, \
    ik_solver_base_destroy, \
    ik_solver_base_construct, \
    ik_solver_base_destruct
/* etc... */
```

This header file is included by src/solver/BASE/solver_base.c and that's also where you will find all implementations of those forward declarations.

Let's take a look at ik/vtables/solver_FABRIK.v, which contains the following:

```c
#include "ik/solver_base.h"

IK_IMPLEMENT(solver_FABRIK, solver_base)
{
    IK_OVERRIDE(type_size)
    IK_OVERRIDE(construct)
    IK_OVERRIDE(destruct)
    IK_OVERRIDE(solve)
}
```

Here we see the third and last directive, IK_OVERRIDE, being used. We are telling the script that there exist a **subset** of functions that directly map onto the functions listed in IK_INTERFACE(solver_interface), and that those **not** listed should simply be filled in by the base implementation ```solver_base```. Indeed, if we take a look at the output header file, we see:

```c
IK_PRIVATE_API uintptr_t ik_solver_FABRIK_type_size(void);
IK_PRIVATE_API ik_ret ik_solver_FABRIK_construct(struct ik_solver_t* solver);
IK_PRIVATE_API void ik_solver_FABRIK_destruct(struct ik_solver_t* solver);
IK_PRIVATE_API ik_ret ik_solver_FABRIK_solve(struct ik_solver_t* solver);
#define IK_SOLVER_FABRIK_IMPL \
    ik_solver_FABRIK_type_size, \  <--- here
    ik_solver_base_create, \
    ik_solver_base_destroy, \
    ik_solver_FABRIK_construct, \  <--- here
    ik_solver_FABRIK_destruct, \  <--- here
    ik_solver_base_rebuild_data, \
    ik_solver_base_recalculate_segment_lengths, \
    ik_solver_FABRIK_solve, \  <--- here
    ik_solver_base_set_tree, \
    ik_solver_base_unlink_tree, \
    ik_solver_base_destroy_tree, \
    ik_solver_base_iterate_nodes, \
    ik_solver_base_iterate_affected_nodes, \
    ik_solver_base_iterate_base_nodes
```

Notice how only the function names that were listed using IK_OVERRIDE() are forward-declared, and notice in particular how IK_SOLVER_FABRIK_IMPL lists all of the functions from ```solver_base``` **except** for the ones we marked with IK_OVERRIDE().

# Putting it all together

The last thing to discuss is what these generated macros IK_SOLVER_BASE_IMPL and IK_SOLVER_FABRIK_IMPL are used for. They list a set of functions that map onto the interface struct. So, if we wanted to create an interface that called the ```solver_base``` implementation, we would write:

```c
struct ik_solver_interface_t solver_base = { IK_SOLVER_BASE_IMPL };
```

This would now allow us to call ```solver_base.solve()``` or ```solver_base.set_tree()``` and it would correctly call the functions ```ik_solver_base_solve()``` and ```ik_solver_base_set_tree()```, respectively.

Similarly, if we wanted to create an interface that called the ```solver_FABRIK``` implementation, we would write:

```c
struct ik_solver_interface_t solver_FABRIK = { IK_SOLVER_FABRIK_IMPL };
```

This would allow us to call ```solver_FABRIK.solve()``` or ```solver_FABRIK.set_tree()``` and it would correctly call the functions ```ik_solver_FABRIK_solve()``` and -- **notice** -- ```ik_solver_base_set_tree()```.
