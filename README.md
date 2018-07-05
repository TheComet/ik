Inverse Kinematics Library
==========================

An implementation of the FABRIK solver. Specialized 2-bone and 1-bone solvers are also included.

[See the wiki page for details on how to use it](https://github.com/TheComet/ik/wiki)

Building
--------

You can build the project as follows using the default settings:
```sh
mkdir build && cd build
cmake ../
make -j8
make install
```

For a detailed list of all of the build options, [see the wiki page](https://github.com/TheComet/ik/wiki)
On POSIX systems, you can enable malloc()/free() wrappers with ```-DIK_MEMORY_DEBUGGING=ON``` and you can further enable memory backtraces with ```-DIK_MEMORY_BACKTRACE=ON```.

Unit tests and benchmarks are also included, those can be enabled with ```-DIK_TESTS=ON``` and ```-DIK_BENCHMARKS=ON```, respectively.

Overview
--------

IK  (Inverse kinematics)  can  be  useful  in  many  situations  ranging  from
procedural animation to small adjustments of animation. Simply put, IK is used
when you want to position the tips of a  hierarchichal  structure  at  a known
location and need to calculate all  of  the  rotations of the parent joints to
achieve this.

Here is an example of foot placement being adjusted according to inclination.

[Click to see image](https://i.imgur.com/OswlUDa.gif)

Here is another example of the paw of a dog  being  placed at a location using
IK.

![](https://cdn-standard2.discourse.org/uploads/urho3d/original/1X/a498777dc3a834d3aefd19aea937dffd27edf33c.gif)

Supported features are
  + Solving arbitrary trees (including disjoint trees) with any number of end effectors.
  + Matching target rotations as well as target positions.
  + Calculation of joint rotations, useful for skinned characters.
  + Specifying chain length for each effector.
  + Conversion between local and global space.
  + Weighted end effectors to facilitate transitioning between the solved and initial transforms.
  + Nlerp of weighted end effectors to make transitioning look more natural.
  + Logging.
  + Dumping trees to DOT format.

Features being worked on are
  + Weighted segments.
  + Joint constraints and constraint callbacks.
  + Bone skipping.
  + Mass/Spring/Damper solver.
  
All  of the code was written in C89 and has no dependencies other than  the  C
standard  library.  Memory  debugging  facilities are in place to track memory
leaks.  On  linux,  backtraces can be generated to the respective malloc() and
free() calls.

Example usage
-------------

Here is a minimal working example that probably satisfies your needs.

```cpp
#include <ik/ik.h>

int main()
{
    /* Create a solver using the FABRIK algorithm */
    struct ik_solver_t* solver = ik.solver.create(IK_FABRIK);

    /* Create a simple 3-bone structure */
    struct ik_node_t* root = solver->node->create(0);
    struct ik_node_t* child1 = solver->node->create_child(1, root);
    struct ik_node_t* child2 = solver->node->create_child(2, child1);
    struct ik_node_t* child3 = solver->node->create_child(3, child2);

    /* Set node positions in local space so they form a straight line in the Y direction*/
    child1->position = ik.vec3.vec3(0, 10, 0);
    child2->position = ik.vec3.vec3(0, 10, 0);
    child3->position = ik.vec3.vec3(0, 10, 0);

    /* Attach an effector at the end */
    struct ik_effector_t* eff = solver->effector->create();
    solver->effector->attach(eff, child3);

    /* set the target position of the effector to be somewhere within range */
    eff->target_position = ik.vec3.vec3(2, -3, 5);

    /* We want to calculate rotations as well as positions */
    solver->flags |= IK_ENABLE_TARGET_ROTATIONS;

    /* Assign our tree to the solver, rebuild data and calculate solution */
    ik.solver.set_tree(solver, root);
    ik.solver.rebuild_data(solver);
    ik.solver.solve(solver);
}
```

[See the wiki page for details on how to use it](https://github.com/TheComet/ik/wiki)

