/*Copyright 2020 Nozdrachev Ilia
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
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