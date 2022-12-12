#include "fileSystem.h"
#include "lib.h"
#include "cp3_syscall.h"


boot_block_t* bootblock_ptr;
dentry_t* dentry_ptr;
inode_t* inode_ptr;
dataBlock_t* datablock_ptr;

dentry_t tmp_dentry[8];
uint32_t* tmp_offset[8];
int32_t last_fd;

/*
 *   fs_init
 *   DESCRIPTION: initialize pointers to the different parts of the file system
 *   INPUTS: fs_address: the boot_block of the file image to copy
 *   RETURN VALUE: no return
 * 			
 */

void fs_init(boot_block_t* fs_address){
    uint32_t num_nodes;
    bootblock_ptr = fs_address;
    dentry_ptr = bootblock_ptr->dir_entries;
    inode_ptr  = (inode_t*)(bootblock_ptr + 1); //skip the first 4096Bytes (bootblk)
    num_nodes = bootblock_ptr -> inode_num;
    datablock_ptr = (dataBlock_t*)(bootblock_ptr + num_nodes +1 ); // offset 4kB*num_node for inode entries 
    last_fd = 0;
}

/*
 *   read_dentry_by_name
 *   DESCRIPTION: given a name, copy the file (if it exists) to another given dentry
 *   INPUTS: fname: name of the file to be copied
 *           dentry: place to copy the file to
 *   RETURN VALUE: 0: if sucessful
 *                 -1: if unable to file the file
 * 			
 */

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    uint32_t i,n;
    int8_t* tmp;
    n = strlen((int8_t*)fname);
    if(n>32) {
        printf("invalid file name\n");
        return -1;
    }     //oob check
    // for all 63 dentries in bootblock 
    for(i=0; i<63; i++){
        tmp = (int8_t*)dentry_ptr[i].fileName;
        //tlen = strlen(tmp);
        if(1){
            if(!strncmp(tmp,(int8_t*)fname,32)){
                //fill dentry
                strncpy((int8_t*)dentry->fileName, (int8_t*)fname, n);
                dentry->fileType = dentry_ptr[i].fileType;
                dentry->inode_num = dentry_ptr[i].inode_num;
                return 0;
            }
        }
    }

    return -1;
}


/*
 *   read_dentry_by_index
 *   DESCRIPTION: Copy the given file at a specific index to dentry object
 *   INPUTS: index: index of the file to be copied
 *           dentry: place to copy the file to
 *   RETURN VALUE: 0: if success
 *                 -1: if index isn't valid
 * 			
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    uint32_t n;
    int8_t* fname;
    if(index < 0 || index >= bootblock_ptr->dir_entries_num) return -1;  //oob check
    //fetch fname at idx
    fname = (int8_t*)dentry_ptr[index].fileName;
    n = strlen(fname);
    //fill dentry
    strncpy((int8_t*)dentry->fileName, fname, n);
    dentry->fileType = dentry_ptr[index].fileType;
    dentry->inode_num = dentry_ptr[index].inode_num;
    return 0;
}


/*
 *   read_data
 *   DESCRIPTION: copy data from a given inode and offset of a given length to a buffer
 *   INPUTS: inode: inode num to be copied from
 *           offset:  datablock offset from a given inode
 *           buf:   buffer to copy the data to
 *           length:   length of the data to copy in bytes
 *   RETURN VALUE: bytes_placed: number of bytes placed
 *                 -1: if inode index is out of bounds
 * 			
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t i;
    uint32_t num_inodes;
    inode_t* curr_inode;
    uint32_t curr_file_length;     //file's length in bytes
    uint32_t bytes_placed,blk_index,byte_index, byte_num, dbn;
    uint8_t* curr_copybyte;        //ptr to 1 byte to be copied

    if(buf == NULL){
        printf("\ninvalid null buffer pointer\n");
        return -1;
    } 

    //init count
    bytes_placed = 0;
    
    num_inodes = bootblock_ptr->inode_num;
    if(inode < 0 || inode >= num_inodes) return -1; //if valid inode number
    
    curr_inode = (inode_t*)(inode_ptr + inode);            //ptr to current inode
    // printf("bootblk_ptr is at : %x \n", bootblock_ptr);
    // printf("dentry_ptr is at : %x \n", dentry_ptr);
    // printf("inode_ptr is at : %x \n", inode_ptr);
    // printf("datablk_ptr is at : %x \n", datablock_ptr);
    // printf("inode num: %d \n", bootblock_ptr->inode_num);
    // printf("curr_inode byte num: %d \n", curr_inode->length);
    
    // printf("curr_inode datablk0 idx: %d \n", curr_inode->data[0]);
    curr_file_length = curr_inode->length;
   
    dbn = bootblock_ptr->dataBlock_num;
    for (i = 0; i < length; i++){
        byte_num = i+offset;
        if(byte_num > curr_file_length){
            return bytes_placed;    // check if eof
        }
        //calcutae blk and byte index, 4096 is for 4kb blocks in data.
        blk_index = byte_num / 4096;
        byte_index = byte_num % 4096;
        if(blk_index <0 || blk_index>=dbn){
            return -1;
        }
        //copy length bytes from blk+offset 
        curr_copybyte = &(datablock_ptr[curr_inode->data[blk_index]].data[byte_index]);
        // printf("blk index: %d    byte index: %d \n", blk_index, byte_index);
        // printf("curr_copybyte when i = %d : %x \n", i, curr_copybyte);
        memcpy(buf+bytes_placed,curr_copybyte,1);

        bytes_placed++;
    }

    return bytes_placed;
}

/*
 *   cpyFileDescriptor
 *   DESCRIPTION: store the dentry and offest within the file (given fd array idx) to global var
 *   INPUTS: offset: location (offset) in file; fd: fd idx; dentry: dentry to be stored
 *   RETURN VALUE: 0: if success
 *                		
 */
int32_t cpyFileDescriptor(uint32_t* offset, int32_t fd, uint8_t* filename){
    read_dentry_by_name(filename,&tmp_dentry[fd]);
    tmp_offset[fd] = offset;
    last_fd = fd;
    return 0;
}

/*
 *   file_open
 *   DESCRIPTION: create a file descriptor of a given file
 *   INPUTS: fileName: the filename of the file to open 
 *   RETURN VALUE: 0: if success
 *                		
 */
int32_t file_open(const uint8_t* fileName){
    int ret;
    dentry_t tmp_dentry;
    ret = read_dentry_by_name(fileName, &tmp_dentry);
    return ret;
}


/*
 *   file_close
 *   DESCRIPTION: close a given file descriptor 
 *   INPUTS: fd: the number of the file descriptor to be removed 
 *   RETURN VALUE: 0: sucess
 *                		
 */
int32_t file_close(int32_t fd){
    return 0;
}


/*
 *   file_read
 *   DESCRIPTION: copy the data of a given file 
 *   INPUTS: fileName: the filename of the file to open 
 *   RETURN VALUE: 0: if success
 *                		
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active;
    uint32_t inode = curPCB ->file_descriptor[fd].inode;
    int32_t retval;
    //check if buf is null
    // if(buf == NULL){
    //     printf("\ninvalid null buffer ptr\n");
    //     return -1;
    // }
    retval = read_data(inode, curPCB ->file_descriptor[fd].file_position, buf, nbytes);
    if((retval) == -1){
        return -1;
    }
    curPCB ->file_descriptor[fd].file_position += nbytes;
    return retval;
}


/*
 *   file_write
 *   DESCRIPTION: **file system is read only, this func has no use**** 
 *   INPUTS: fd: the file to be written to
 *   RETURN VALUE: -1: always return -1
 *                		
 */
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}


/*
 *   dir_open
 *   DESCRIPTION: set up the file descriptor of a directory to be opened
 *   INPUTS: dirName: name of directory to be opened
 *   RETURN VALUE: fd: number of file descriptor in the array
 *                		
 */
int32_t dir_open(const uint8_t* dirName){
   
    // read_dentry_by_name(dirName, &tmp_dentry);
    return 0;
}

/*
 *   dir_close
 *   DESCRIPTION: close the directory given by the fd
 *   INPUTS: fd: the directory to be closed
 *   RETURN VALUE: 0: always return true
 *                		
 */
int32_t dir_close(int32_t fd){
    return 0;
}

/*
 *   dir_read
 *   DESCRIPTION:  read from a specified fd number
 *   INPUTS: fd: file descriptor number to read from
 *           buf: destination to copy the file name to
 *           nbytes: number of bytes to copy
 *   RETURN VALUE: 0: if copy was success
 *                 -1: if copy was unsuccessful
 *                		
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    int8_t* fileName_ptr;
    dentry_t* cur_dentry;
    //check null ptr
    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active;
     if(buf == NULL){
        printf("\ninvalid null buffer ptr\n");
        return -1;
    }

    if (curPCB ->file_descriptor[fd].file_position < 0 || curPCB ->file_descriptor[fd].file_position >= bootblock_ptr->dir_entries_num) return 0;  //check oob.

    //fetch current dentry
    cur_dentry = (dentry_t*)(dentry_ptr+curPCB ->file_descriptor[fd].file_position);
    fileName_ptr = (int8_t*)cur_dentry->fileName;
    //copy filename and increment in directory
    memcpy((int8_t*)buf,fileName_ptr,nbytes);
    (curPCB ->file_descriptor[fd].file_position)++; 

    return nbytes;
}

/*
 *   file_write
 *   DESCRIPTION: **file system is read only, this func has no use**** 
 *   INPUTS: fd: the file to be written to
 *   RETURN VALUE: -1: always return -1
 *                		
 */
int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

