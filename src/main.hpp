#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#define clen(s) (s), (sizeof(s) - 1)
#define lenc(s) (sizeof(s) - 1), (s)

#define blen(s) (s), sizeof(s)
#define lenb(s) sizeof(s), (s)

#define MTK_CONTAINER "\x88\x16\x88\x58"

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024

const char *prog_name = NULL;

enum {
	TYPE_KERNEL = 0, 
	TYPE_ROOTFS, 
	TYPE_NEXT_STEP
};

#pragma pack(push,1)
struct mtk_container_hdr {
	unsigned char magic[4];
	unsigned int data_length;
	char data_type[32];
	unsigned char reserved[472];
};

struct android_boot_img_hdr {
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    char name[BOOT_NAME_SIZE]; /* asciiz product name */

    char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */

    /* Supplemental command line data; kept here to maintain
     * binary compatibility with older versions of mkbootimg */
    unsigned char extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
};
#pragma pack(pop)

struct mtk_container {
	mtk_container_hdr *header;
	unsigned char *data;
	unsigned int length;
};

struct android_boot_img {
	android_boot_img_hdr *header;
	unsigned char *data;
	unsigned int length;
};


int help();
unsigned char *unpack_data(const unsigned char *data, unsigned int size, unsigned int *new_size, bool *need_free);
unsigned char *read_file(const char *path, unsigned int *length);
bool write_file(const char *path, const void *data, unsigned int length);
void dump_android_img_hdr(struct android_boot_img_hdr *header, unsigned int ps = 0);
void dump_mtk_container_hdr(struct mtk_container_hdr *header);
bool is_dir(const char *path);
