#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "lib.h"



typedef struct dentry {
    char fileName[32];       //max file name length
    uint32_t fileType;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;


typedef struct inode {
    uint32_t length;
    uint32_t data[4096/4-1];    //max file size 
} inode_t;


typedef struct dataBlock {
    uint8_t data[4096];
} dataBlock_t;


typedef struct boot_block {
    uint32_t dir_entries_num;
    uint32_t inode_num;
    uint32_t dataBlock_num;
    uint8_t reserved[52];       
    dentry_t dir_entries[63];   //max number of files
} boot_block_t;



/* given a name, copy the dentry if it exists */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
/* given an index, copy the dentry if it exists */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);



/* initiliaze all the pointers to a given file system memory */
void fs_init(boot_block_t* fs_address);


extern int32_t file_open(const uint8_t* fileName);
extern int32_t file_close(int32_t fd);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, void* buf, int32_t nbytes);

int32_t cpyFileDescriptor(uint32_t* offset, int32_t fd, uint8_t* filename);


int32_t dir_open(const uint8_t* dirName);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, void* buf, int32_t nbytes);



#endif

