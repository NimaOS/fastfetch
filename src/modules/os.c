#include "fastfetch.h"

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "OS"))
        return;

    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    FF_STRBUF_CREATE(os);

    if(instance->config.osFormat.length == 0)
    {
        if(name[0] == '\0')
        {
            ffPrintError(instance, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
            return;
        }

        ffStrbufSetF(&os, "%s %s", name, instance->state.utsname.machine);
    }
    else
    {
        ffParseFormatString(&os, &instance->config.osFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.machine}
        );
    }

    ffPrintAndSaveCachedValue(instance, "OS", &os);
    ffStrbufDestroy(&os);
}