#ifndef _ELEVENMPV_FIOS2_H_
#define _ELEVENMPV_FIOS2_H_

#include <psp2/fios2.h>
#include <psp2/types.h>

int fios2Init(void);
int fios2CommonOpDeleteCB(void *pContext, SceFiosOp op, SceFiosOpEvent event, int err);

#endif