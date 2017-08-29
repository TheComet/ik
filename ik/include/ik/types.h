#ifndef IK_TYPES_H
#define IK_TYPES_H

/* containers */
typedef struct vector_t vector_t;
typedef struct bstv_t bstv_t;

/* IK specific types */
typedef struct chain_t         chain_t;
typedef struct base_chain_t    base_chain_t;
typedef struct ik_constraint_t ik_constraint_t;
typedef struct ik_effector_t   ik_effector_t;
typedef struct ik_node_t       ik_node_t;
typedef struct ik_solver_t     ik_solver_t;

/* solver implementations */
typedef struct fabrik_t        fabrik_t;
typedef struct two_bone_t      two_bone_t;
typedef struct one_bone_t      one_bone_t;
typedef struct msd_t           msd_t;

#endif /* IK_TYPES_H */
