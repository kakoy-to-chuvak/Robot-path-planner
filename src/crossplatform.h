#ifndef CROSSPLATFORM_H
#define CROSSPLATFORM_H


#if defined( _WIN32 ) || defined( _WIN64 ) // system check
#       include <windows.h>
#       include <direct.h>
        
#       define CRP_sleep Sleep
#       define CRP_chdir _chdir


#else // system check
#       include <unistd.h>


#       define CRP_sleep sleep
#       define CRP_chdir chdir

#endif // system check


#ifndef MAX_PATH
#       define MAX_PATH 260
#endif














#endif // CROSSPLATFORM_H