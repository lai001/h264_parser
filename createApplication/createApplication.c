#include "application.h"
#include <stdio.h>
#include <stdlib.h>

Application *createApplication()
{
    Application *application = (Application *)malloc(sizeof(Application));
    application->number = 100;
    return application;
}
