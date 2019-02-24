#include "ik/attachment.h"

static const char* g_table[IK_ATTACHMENT_COUNT] = {
#define X(upper, lower) #upper,
    IK_ATTACHMENT_LIST
#undef X
};

/* ------------------------------------------------------------------------- */
const char*
ik_attachment_to_str(enum ik_attachment_type_e type)
{
    return g_table[type];
}
