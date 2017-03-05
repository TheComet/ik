Inverse Kinematics Library
==========================

Very much a work-in-progress. In its current state, this library  is  not  yet
usable. Please come back later :)

The  goal  of  this  library is to provide a fast,  lightweight  and  flexible
solution to solving the inverse kinematics problem.

Overview
--------

One of the challenges was to design an interface  which  could  work  with any
scene graph/skeleton/animation system.  The  library  provides  a leightweight
interface for building a tree  and specifying positions and rotations for each
node.  The tree also holds information on effector targets and  chain  length.
The  tree is then preprocessed into a more optimal form for the  solver.  This
preprocessing step is necessary  whenever the tree is altered or effectors are
added or removed,  which,  in  practice,  should only occur once. Invoking the
solver  will  cause  a  series  of  computations  to  occur  on  the  internal
structures, the results of which will then be  mapped  back  onto the original
tree structure specified by the user. The user can then iterate  the  tree and
apply   the   solved   positions   and   rotations   back   to    his    scene
graph/skeleton/animation structure.

All  of the code was written in C89 and has no dependencies other than  the  C
standard  library.  Memory  debugging  facilities are in place to track memory
leaks.  On  linux,  backtraces can be generated to the respective malloc() and
free() calls.

