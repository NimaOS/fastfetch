#include "fastfetch.h"

#include "X11/extensions/Xrandr.h"
#include "dlfcn.h"

static int getCurrentRate(FFinstance* instance, Display* display)
{
    void* xrandr;
    if(instance->config.libXrandr.length == 0)
        xrandr = dlopen("libXrandr.so", RTLD_LAZY);
    else
        xrandr = dlopen(instance->config.libXrandr.chars, RTLD_LAZY);
    if(xrandr == NULL)
        return 0;

    Window root = RootWindow(display, 0);

    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display*, Window) = dlsym(xrandr, "XRRGetScreenInfo");
    if(ffXRRGetScreenInfo == NULL)
        return 0;

    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRConfigCurrentRate");
    if(ffXRRConfigCurrentRate == NULL)
        return 0;

    XRRScreenConfiguration* xrrscreenconf = ffXRRGetScreenInfo(display, root);
    if(xrrscreenconf == NULL)
        return 0;

    short currentRate = ffXRRConfigCurrentRate(xrrscreenconf);

    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRFreeScreenConfigInfo");
    if(ffXRRFreeScreenConfigInfo != NULL)
        ffXRRFreeScreenConfigInfo(xrrscreenconf);

    dlclose(xrandr);

    return (int) currentRate;
}

void ffPrintResolution(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Resolution"))
        return;

    void* x11;
    if(instance->config.libX11.length == 0)
        x11 = dlopen("libX11.so", RTLD_LAZY);
    else
        x11 = dlopen(instance->config.libX11.chars, RTLD_LAZY);
    if(x11 == NULL)
    {
        ffPrintError(instance, "Resolution", "dlopen(\"libX11.so\", RTLD_LAZY) == NULL");
        return;
    }

    Display*(*ffXOpenDisplay)(const char*) = dlsym(x11, "XOpenDisplay");
    if(ffXOpenDisplay == NULL)
    {
        ffPrintError(instance, "Resolution", "dlsym(x11, \"XOpenDisplay\") == NULL");
        return;
    }

    Display* display = ffXOpenDisplay(NULL);
    if(display == NULL)
    {
        ffPrintError(instance, "Resolution", "ffXOpenDisplay(NULL) == NULL");
        return;
    }

    Screen*  screen  = DefaultScreenOfDisplay(display);

    int currentRate = getCurrentRate(instance, display);

    dlclose(x11);

    FF_STRBUF_CREATE(resolution);

    if(instance->config.resolutionFormat.length == 0)
    {
        ffStrbufAppendF(&resolution, "%ix%i", screen->width, screen->height);

        if(currentRate > 0)
            ffStrbufAppendF(&resolution, " @ %iHz", currentRate);
    }
    else
    {
        ffParseFormatString(&resolution, &instance->config.resolutionFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &screen->width},
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &screen->height},
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &currentRate}
        );
    }

    ffPrintAndSaveCachedValue(instance, "Resolution", &resolution);
    ffStrbufDestroy(&resolution);
}