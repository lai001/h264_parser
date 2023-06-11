#include "application.h"

Application *createApplication();

int main(int argc, char **argv)
{
    Application *application = createApplication();
    printf("application->number: %d\n", application->number);
    return 0;
}