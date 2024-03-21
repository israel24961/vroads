#include <0_GlobalIncludes.h>
#include <easycringelib.h>

struct LiteQueue;

__attr(malloc) struct LiteQueue  * LQueueInit(u32 size);

bool LQueuePush(struct LiteQueue *q, u32_4 val);
u32_4 LQueuePop(struct LiteQueue *q);
