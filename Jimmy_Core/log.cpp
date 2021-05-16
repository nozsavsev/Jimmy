#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"

void Log(const char* Log_str, ...)
{
    static std::mutex Log_mutex;
    static FILE* fLog = NULL;

    static char buffer[80];
    static time_t rawtime;
    static struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y | %H:%M:%S", timeinfo);

    Log_mutex.lock();

    if (fLog == nullptr)
    {
#ifdef DEBUG
        fLog = stderr;
#else
        fopen_s(&fLog, "jimmy_Log.txt", "a+");

        if (!fLog)
            fLog = stderr;

        fprintf(fLog, "\n\n\n%s\tLog START\n\n\n", buffer);
#endif // DEBUG
    }

    else if (Log_str == nullptr)
        if (fLog != stderr)
        {
            fprintf(fLog, "\n\n\n%s\tLog END --\n\n\n", buffer);
            fclose(fLog);
            return;
        }


    fprintf(fLog, "%s: ", buffer);
    va_list argptr;
    va_start(argptr, Log_str);
    vfprintf(fLog, Log_str, argptr);
    va_end(argptr);

    Log_mutex.unlock();
}