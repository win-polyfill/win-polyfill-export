#pragma once

#include "base.h"

#define PHNT_NO_INLINE_INIT_STRING
#include <phnt.h>

// These headers should after phnt

#include <lm.h>
#include <lmdfs.h>
#include <lmat.h>
#include <davclnt.h>
// pdh should after lm
#include <pdh.h>

#define LM_STYPE_DISKTREE STYPE_DISKTREE
#define LM_STYPE_PRINTQ STYPE_PRINTQ
#define LM_STYPE_DEVICE STYPE_DEVICE
#define LM_STYPE_IPC STYPE_IPC

#undef STYPE_DISKTREE
#undef STYPE_PRINTQ
#undef STYPE_DEVICE
#undef STYPE_IPC

#include "crt-macros.h"
#include "entry-api.h"
