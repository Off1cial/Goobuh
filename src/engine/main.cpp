#include <stdio.h>

#include "core/util/utlvector.h"
#include "math/vector.h"
#include "core/parser.h"
#include "core/util/rc.h"
#include "engine/vk/vk.h"
#include "core/assert.h"

#include "core/util/utlfile.h"
#include "core/util/utlmemory.h"

int main(int argc, char *argv[])
{
    #ifdef _DEBUG
    printf("Hello, world!\n");
    #else
    printf("No debug!\n");
    #endif

    Vector my_vec(0.2, 1.0f, 2.0f);
   
    //CVulkanSystem vksys( "Vulkan Renderer", 640, 480);
    
    Rc<RcObject> my_p = new RcObject;
    Rc<RcObject> second(my_p);

    Parser my_parser("test.txt", false);
    
    int test = 0;
    while(my_parser.ReadInt(&test))
    {
        printf("Read: %d\n", test);
    }

    char my_string[64];
    if (my_parser.ReadString (my_string, 64))
    {
        printf("Read string: %s\n", my_string);
    }

    if (my_parser.ReadVector(my_vec))
    {
        printf(Vec2String(my_vec));
    }


    CUtlFile test_file("test.txt", CUtlFileMode::rb);
    test_file.DebugPrint();

#if  0
    
    int my_numbers[] = { 1, 2, 3, 4, 5 };
    CUtlMemory<int> my_block( my_numbers, 5, 1.5f );

    my_block[3] = 54;
    for (int i = 0; i < my_block.Size(); i++)
    {
        printf("Alloc %d = %d\n", i, my_block[i]);
    }

    my_block.Grow(); 

    for (int i = 0; i < my_block.Size(); i++)
    {
        printf("Alloc %d = %d\n", i, my_block[i]);
    }
#endif
    
    CUtlVector<int> utlvector( 4, 2.0f );
    
    utlvector.AddToHead(33);
    utlvector.AddToHead(12);
    utlvector.AddToTail(0x32);
    utlvector.AddToHead(12);
    utlvector.AddToHead(12);
    utlvector.AddToHead(12);
    utlvector.AddToTail(0x32);
    utlvector.AddToTail(0x32);
    utlvector.AddToTail(0x32);

    for ( int i = 0; i < utlvector.Count(); i++)
    {
        printf("Element %d: %d\n", i, utlvector[i]);
    }



    return 0;
}
