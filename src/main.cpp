#include <iostream>

#include "window.h"

int main(void)
{
    try
    {
        Window w{800, 600, "FBO test"};
        w.createWindow();
        w.initScene();
        w.run();
    }
    catch(std::exception const & e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}
