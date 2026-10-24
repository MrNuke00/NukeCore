#include <Core/Core.h>
#include <Core/Assert.h>



int main() {
    // 1. Test passing assert (Execution continues normally)
    int x = 42;
    assert(x > 0);

    // 2. Test verify with a side effect
    // Even if NDEBUG is defined (Release build), verify still executes 
    // the assignment expression so 'y' gets properly initialized.
    int y = 0;
    assert(y == 100);

    // 3. Test ensure (Evaluates expression, returns bool, and triggers 
    // DEBUG_BREAK() if false)
    bool is_valid = ensure(x != 42);

    // Note: Uncommenting the line below will intentionally trigger 
    // your custom DEBUG_BREAK() (e.g., int3 / __debugbreak)
    // assert(x == 0); 

    return 0;
}