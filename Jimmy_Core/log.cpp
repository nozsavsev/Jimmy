#define _CRT_SECURE_NO_WARNINGS
#include "Jimmy_Core.h"

void log(const char* log_str, ...)
{
    static std::mutex log_mutex;
    static FILE* flog = NULL;

    static char buffer[80];
    static time_t rawtime;
    static struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y | %H:%M:%S", timeinfo);

    log_mutex.lock();

    if (flog == nullptr)
    {
#ifdef DEBUG
        flog = stderr;
#else
        fopen_s(&flog, "jimmy_log.txt", "a+");

        if (!flog)
            flog = stderr;

        fprintf(flog, "\n\n\n%s\tLOG START\n\n\n", buffer);
#endif // DEBUG
    }

    else if (log_str == nullptr)
        if (flog != stderr)
        {
            fprintf(flog, "\n\n\n%s\tLOG END --\n\n\n", buffer);
            fclose(flog);
            return;
        }


    fprintf(flog, "%s: ", buffer);
    va_list argptr;
    va_start(argptr, log_str);
    vfprintf(flog, log_str, argptr);
    va_end(argptr);

    log_mutex.unlock();
}