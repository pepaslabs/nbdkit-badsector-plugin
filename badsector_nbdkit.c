/* badsector_nbdkit.c
 * See https://github.com/pepaslabs/nbdkit-badsector-plugin
 * Copyright (C) 2015 Jason Pepas.
 * Released under the terms of the MIT license.  See https://opensource.org/licenses/MIT
 */

// see also https://github.com/libguestfs/nbdkit/blob/master/docs/nbdkit-plugin.pod

#include <config.h>
#include <nbdkit-plugin.h> // nbdkit_error(), etc.

#define THREAD_MODEL NBDKIT_THREAD_MODEL_SERIALIZE_ALL_REQUESTS

// per-connection state (not used):
int handle;
uint64_t bad_sector = 1024;

unsigned char disk[32 * 1024 * 1024];

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
    if extent_crosses_bad_sector(offset, size)
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
    if extent_crosses_bad_sector(offset, size)
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

    .open              = badsector_open,
    .get_size          = badsector_get_size,

    .pread             = badsector_pread,
    .pwrite            = badsector_pwrite
};

NBDKIT_REGISTER_PLUGIN(plugin)
