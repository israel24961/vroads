#include "1_CringeMiniDataStructs.h"
struct LiteQueue {
        // Internal
        u32_4 *_back;
        u32_4 *_front;
        u32 _curSize;
        u32 _capacity;
        u32_4 _queue[];
};

__malloc struct LiteQueue *LQueueInit(u32 capacity)
{

        struct LiteQueue *queue = (struct LiteQueue *)malloc(sizeof(struct LiteQueue) + capacity * sizeof(u32_4));
        queue->_back = queue->_queue;
        queue->_front = queue->_queue;
        queue->_curSize = 0;
        queue->_capacity = capacity;
        return queue;
}

bool LQueuePush(struct LiteQueue *queue, u32_4 val)
{
        bool isFull = queue->_curSize >= queue->_capacity;
        if (isFull)
                return false;

        *queue->_front = val;
        queue->_curSize++;

        var newFront = queue->_front + 1;
        var isOverflowBuffer = newFront == (queue->_queue + queue->_capacity);
        if (isOverflowBuffer)
                queue->_front = queue->_queue;
        else
                queue->_front = newFront;
        return true;
}
u32_4 LQueuePop(struct LiteQueue *queue)
{
        if (queue->_curSize == 0)
                return (u32_4){};
        u32_4 val = *queue->_back;
        queue->_curSize--;
        var newBack = queue->_back + 1;
        var isOverflowBuffer = newBack == (queue->_queue + queue->_capacity);
        if (isOverflowBuffer)
                queue->_back = queue->_queue;
        else
                queue->_back = newBack;

        return val;
}
