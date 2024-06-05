// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
#include <sys/stat.h>


const char devdir[] = "/dev/";
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    for (int i = 0; ttyusb_prefixes[i]; i++)
        if (strncmp(ttyusb_prefixes[i], d->d_name, strlen(ttyusb_prefixes[i])) == 0)
            return 1;
    return 0;
}

static int sortFirst(const struct dirent **a, const struct dirent **b) {
    char *pathA = malloc(sizeof(devdir) + strlen((*a)->d_name));
    strcpy(pathA, devdir);
    strcat(pathA, (*a)->d_name);
    char *pathB = malloc(sizeof(devdir) + strlen((*b)->d_name));
    strcpy(pathB, devdir);
    strcat(pathB, (*b)->d_name);
    struct stat stA, stB;
    stat(pathA, &stA);
    free(pathA);
    stat(pathB, &stB);
    free(pathB);
    return stA.st_mtime - stB.st_mtime;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, NULL);
    if (n != 1)
        panic("expected 1 ttyusb device, found %d", n);
    char *path = malloc(sizeof(devdir) + strlen(namelist[0]->d_name));
    strcpy(path, devdir);
    strcat(path, namelist[0]->d_name);
    return path;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by stat()
char *find_ttyusb_last(void) {
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, sortFirst);
    if (n <= 0)
        panic("expected 1+ ttyusb device, found %d", n);
    char *path = malloc(sizeof(devdir) + strlen(namelist[n-1]->d_name));
    strcpy(path, devdir);
    strcat(path, namelist[0]->d_name);
    return path;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, sortFirst);
    if (n <= 0)
        panic("expected 1+ ttyusb device, found %d", n);
    char *path = malloc(sizeof(devdir) + strlen(namelist[0]->d_name));
    strcpy(path, devdir);
    strcat(path, namelist[0]->d_name);
    return path;
}
