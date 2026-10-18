#pragma once
#include "ae_base.hpp"

typedef int (*aeCallbackFunc_t)(void *, const char *, const char *,
                                const char *, const char *, int32_t, int32_t,
                                int32_t, int32_t);
typedef int (*aeEntryFunc_t)(void *, aeCallbackFunc_t);
