#include <0_GlobalIncludes.h>
#include <1_CringeMiniDataStructs.h>
#include <easycringelib.h>

int correct_push_and_pop()
{
        __clean var q = LQueueInit(3);
        LQueuePush(q, (u32_4){1, 2, 3, 4});
        LQueuePush(q, (u32_4){5, 6, 7, 8});
        LQueuePush(q, (u32_4){9, 10, 11, 12});

        u32_4 a = LQueuePop(q);
        u32_4 b = LQueuePop(q);
        u32_4 c = LQueuePop(q);

        if (a.a != 1 || a.b != 2 || a.c != 3 || a.d != 4)
                return 1;
        if (b.a != 5 || b.b != 6 || b.c != 7 || b.d != 8)
                return 2;
        if (c.a != 9 || c.b != 10 || c.c != 11 || c.d != 12)
                return 3;
        return 0;
}
int more_push_than_capacity()
{
        __clean var q = LQueueInit(2);
        // struct LiteQueue* q=NULL;
        LQueuePush(q, (u32_4){1, 2, 3, 4});
        LQueuePush(q, (u32_4){5, 6, 7, 8});
        LQueuePush(q, (u32_4){9, 10, 11, 12});

        u32_4 a = LQueuePop(q);
        u32_4 b = LQueuePop(q);
        u32_4 c = LQueuePop(q);
        u32_4 d = LQueuePop(q);

        if (a.a != 1 || a.b != 2 || a.c != 3 || a.d != 4)
                return 1;
        if (b.a != 5 || b.b != 6 || b.c != 7 || b.d != 8)
                return 2;
        if (c.a != 0 || c.b != 0 || c.c != 0 || c.d != 0)
                return 3;
        if (d.a != 0 || d.b != 0 || d.c != 0 || d.d != 0)
                return 4;

        return 0;
}

int push_queue_notSequential()
{
        __clean var q = LQueueInit(3);
        LQueuePush(q, (u32_4){1, 2, 3, 4});
        var re = LQueuePop(q);
        if (re.a != 1 || re.b != 2 || re.c != 3 || re.d != 4)
                return 1;

        LQueuePush(q, (u32_4){5, 6, 7, 8});
        LQueuePush(q, (u32_4){9, 10, 11, 12});
        var rf0 = LQueuePop(q);
        var rf1 = LQueuePop(q);

        if (rf0.a != 5 || rf0.b != 6 || rf0.c != 7 || rf0.d != 8)
                return 2;
        if (rf1.a != 9 || rf1.b != 10 || rf1.c != 11 || rf1.d != 12)
                return 3;

        LQueuePush(q, (u32_4){13, 14, 15, 16});
        LQueuePush(q, (u32_4){17, 18, 19, 20});
        LQueuePush(q, (u32_4){26, 26, 27, 28});
        var rg0 = LQueuePop(q);
        var rg1 = LQueuePop(q);
        var rg2 = LQueuePop(q);

        if (rg0.a != 13 || rg0.b != 14 || rg0.c != 15 || rg0.d != 16)
                return 4;
        if (rg1.a != 17 || rg1.b != 18 || rg1.c != 19 || rg1.d != 20)
                return 5;
        if (rg2.a != 26 || rg2.b != 26 || rg2.c != 27 || rg2.d != 28)
                return 6;

        LQueuePush(q, (u32_4){29, 30, 31, 32});
        LQueuePush(q, (u32_4){33, 34, 35, 36});
        LQueuePush(q, (u32_4){37, 38, 39, 40});
        var rh0 = LQueuePop(q);
        var rh1 = LQueuePop(q);
        LQueuePush(q, (u32_4){41, 42, 43, 44});
        var rh2 = LQueuePop(q);
        var rh3 = LQueuePop(q);

        if (rh0.a != 29 || rh0.b != 30 || rh0.c != 31 || rh0.d != 32)
                return 7;
        if (rh1.a != 33 || rh1.b != 34 || rh1.c != 35 || rh1.d != 36)
                return 8;
        if (rh2.a != 37 || rh2.b != 38 || rh2.c != 39 || rh2.d != 40)
                return 9;
        if (rh3.a != 41 || rh3.b != 42 || rh3.c != 43 || rh3.d != 44)
                return 10;
        return 0;
}

int push_1000()
{
        var q = LQueueInit(10000000);
        for (int i = 0; i < 5000000; i++) {
                LQueuePush(q, (u32_4){i, i, i, i});
        }
        // Remove 250
        for (int i = 0; i < 250; i++) {
                LQueuePop(q);
        }
        // Add 500

        for (int i = 500; i < 1000; i++) {
                LQueuePush(q, (u32_4){i, i, i, i});
        }
        // Pop the rest

        for (int i = 0; i < 750; i++) {
                var e = LQueuePop(q);
                if (e.a != i + 250u || e.b != i + 250u || e.c != i + 250u || e.d != i + 250u)
                        return 1;
        }

        return 0;
}
#define nameof(x) #x
#define fNTuple(x)                                                                                                                                   \
        {                                                                                                                                            \
                nameof(x), x                                                                                                                         \
        }
struct funcNameTuple {
        char *name;
        int (*func)();
};

int main(int argc, char *argv[])
{
        // Delegate
        struct funcNameTuple tests[] = {
            fNTuple(correct_push_and_pop),
            fNTuple(more_push_than_capacity),
            fNTuple(push_queue_notSequential),
            fNTuple(push_1000),
        };
        const int nTests = sizeof(tests) / sizeof(tests[0]);

        var opt = 0;
        L("Args %d", argc);
        opt = argc > 1 ? atoi(argv[1]) : nTests;
        if (opt==nTests) {
                bool AnyFailed = false;
                for (int i = 0; i < nTests; i++) {
                        var res = tests[i].func();
                        if (res != 0) {
                                printf("Test %s failed with code %d\n", tests[i].name, res);
                                AnyFailed = true;
                        } else {
                                printf("Test %s passed\n", tests[i].name);
                        }
                }
                return AnyFailed;
        }
        else if (opt < nTests) {
                var res = tests[opt].func();
                if (res != 0) {
                        printf("Test %s failed with code %d\n", tests[opt].name, res);
                        return 1;
                } else {
                        printf("Test %s passed\n", tests[opt].name);
                        return 0;
                }
        }
}
