/* from: https://github.com/BSzili/aros-stuff */

#ifndef INCSTACK_H
#define INCSTACK_H

#ifndef __AROS__
#error You should only include this on AROS.
#endif

#include <proto/exec.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int real_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    struct Task *mytask = FindTask(NULL);
    ULONG stacksize = (UBYTE *)mytask->tc_SPUpper - (UBYTE *)mytask->tc_SPLower;
    int rc = 1;

    if (stacksize >= (ULONG)__stack)
    {
        rc = real_main(argc, argv);
    }
    else
    {
        struct StackSwapArgs swapargs;
        struct StackSwapStruct stack;

        swapargs.Args[0] = argc;
        swapargs.Args[1] = (IPTR)argv;

        if ((stack.stk_Lower = AllocVec(__stack, MEMF_PUBLIC)) != NULL)
        {
            stack.stk_Upper = (APTR)((IPTR)stack.stk_Lower + __stack);
            stack.stk_Pointer = stack.stk_Upper;

            rc = NewStackSwap(&stack, (void *)real_main, &swapargs);
        //  printf("BACK IN MAIN WRAPPER.\n");

            FreeVec(stack.stk_Lower);
        }
        else
        {
            printf("Couldn't allocate %d bytes for stack.\n", __stack);
        }
    }

    return rc;
}

#ifdef __cplusplus
}
#endif

#define main(x,y) real_main(x,y)

#endif /* INCSTACK_H */
