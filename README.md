Inverse Kinematics Library
==========================

An implementation of the FABRIK solver. Specialized 2-bone and 1-bone solvers are also included.

[See the wiki page for details on how to use it](https://github.com/TheComet93/ik/wiki)

Building
--------

Use the ```v1.0``` branch. The master branch has some unstable features.

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
  + Calculation of final rotations.
  + Specifying chain length for each effector.
  + Conversion between local and global space.
  + Target rotations with weighted decay.
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

static void results_callback(struct ik_node_t* ikNode)
{
    /* Extract our scene graph node again */
    Node* node = (Node)ikNode->user_data;

    /* Apply results back to our engine's tree */
    node->SetWorldPosition(ikNode->position);
    node->SetWorldRotation(ikNode->rotation);
}

int main()
{
    /* Create a simple 3-bone structure */
    struct ik_node_t* root = ik_node_create(0);
    struct ik_node_t* child1 = ik_node_create(1);
    struct ik_node_t* child2 = ik_node_create(2);
    struct ik_node_t* child3 = ik_node_create(3);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);

    /* Attach an effector at the end */
    struct ik_effector_t* eff = ik_effector_create();
    ik_node_attach_effector(child3, eff);

    /* Create a solver using the FABRIK algorithm */
    struct ik_solver_t* solver = ik_solver_create(SOLVER_FABRIK);

    /* We want to calculate rotations as well as positions */
    solver->flags |= SOLVER_CALCULATE_FINAL_ROTATIONS;

    /* Assign our tree to the solver and rebuild the data */
    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_chain_tree(solver);
    ik_solver_solve(solver);
}

[See the wiki page for details on how to use it](https://github.com/TheComet93/ik/wiki)

```
