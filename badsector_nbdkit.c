/* badsector_nbdkit.c
 * See https://github.com/pepaslabs/nbdkit-badsector-plugin
 * Copyright (C) 2015 Jason Pepas.
 * Released under the terms of the MIT license.  See https://opensource.org/licenses/MIT
 */

// see also https://github.com/libguestfs/nbdkit/blob/master/docs/nbdkit-plugin.pod

#include <config.h>
#include <nbdkit-plugin.h> // nbdkit_error(), etc.
#include <stdbool.h> // bool, etc.
#include <string.h> // memcpy(), etc.
#include <stdlib.h> // malloc(), etc.

#include "parsing.h"


#define THREAD_MODEL NBDKIT_THREAD_MODEL_SERIALIZE_ALL_REQUESTS


// per-connection state (not used):
int handle;
uint64_t bad_sector = 1024;


unsigned char *disk;
uint64_t dev_size=32*1024*1024;


#define badsector_config_help \
    "size: size of the block device (in bytes) (default 32M).\n" \
    "badsector: the sector number to treat as a bad sector (default 1024)."


int badsector_config(const char *key, const char *value)
{
    int retval;

    if (strcmp(key, "size") == 0)
    {
        retval = parse_size_str(value, strlen(value)+1, &dev_size);
        if (retval != 0)
        {
            nbdkit_error("Unable to parse 'size' value '%s'.  Error: %i", value, retval);
            return -1;
        }
    }
    else if (strcmp(key, "badsector") == 0)
    {
        retval = parse_uint64t_str_strict(value, strlen(value)+1, &bad_sector);
        if (retval != 0)
        {
            nbdkit_error("Unable to parse 'bad_sector' value '%s'.  Error: %i", value, retval);
            return -1;
        }
    }
    else
    {
        nbdkit_error("Unrecognized parameter: '%s'", key);
        return -1;
    }
}


int badsector_config_complete()
{
    nbdkit_debug("Creating in-memory block device of %llu bytes", dev_size);

    disk = malloc(dev_size);
    if (disk == NULL)
    {
        nbdkit_error("malloc() failed: %m");
        return -1;
    }

    nbdkit_debug("Simulating sector number %llu as a bad sector", bad_sector);

    return 0;
}


void* badsector_open(int readonly)
{
    return (void*)(&handle);
}


int64_t badsector_get_size(void *passed_handle)
{
    return (int64_t)sizeof(disk);
}


#define SHIFTS_FOR_512_BYTE_SECTOR 9


bool extent_crosses_bad_sector(uint64_t offset, uint32_t size)
{
    uint64_t first_sector = offset >> SHIFTS_FOR_512_BYTE_SECTOR;
    uint64_t last_sector = (offset + size) >> SHIFTS_FOR_512_BYTE_SECTOR; 
    return first_sector <= bad_sector && bad_sector <= last_sector;
}


int badsector_pread(void *passed_handle, void *buf, uint32_t count, uint64_t offset)
{
    if (extent_crosses_bad_sector(offset, count))
    {
        return -1;
    }
    else
    {
        memcpy(buf, disk+offset, count);
    }
}


int badsector_pwrite(void *passed_handle, const void *buf, uint32_t count, uint64_t offset)
{
    if (extent_crosses_bad_sector(offset, count))
    {
        return -1;
    }
    else
    {
        memcpy(disk+offset, buf, count);
    }
}


static struct nbdkit_plugin plugin = {
    .name              = "badsector",
    .longname          = "nbdkit badsector plugin",
    .description       = "An nbdkit plugin which simulates a disk with a bad sector.",
    .version           = "0.0",

    .config_help       = badsector_config_help,
    .config            = badsector_config,
    .config_complete   = badsector_config_complete,

    .open              = badsector_open,
    .get_size          = badsector_get_size,

    .pread             = badsector_pread,
    .pwrite            = badsector_pwrite
};


NBDKIT_REGISTER_PLUGIN(plugin)
