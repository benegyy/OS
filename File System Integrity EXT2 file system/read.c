#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include "ext2fs.h"
#define EXT2_BOOT_BLOCK_SIZE 1024
#define EXT2_SUPER_BLOCK_SIZE 1024
#define EXT2_SUPER_BLOCK_POSITION EXT2_BOOT_BLOCK_SIZE
#define EXT2_ROOT_INODE 2
#define EXT2_INODE_SIZE 256
#define EXT2_NUM_DIRECT_BLOCKS 12
#define EXT2_MAX_NAME_LENGTH 255
#define EXT2_SUPER_MAGIC 0xEF53
#define EXT2_UNLOG(v) (1UL << (10UL + (v)))

char* get_time_format(const uint32_t t)
{
    const time_t t2 = (const long)t;
    return strtok(asctime(localtime(&t2)), "\n");

}
int block_per_group=256;
unsigned char signature[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void print_inode(const struct ext2_inode* inode)
{
    char mode[50] = { '\0' };
    switch (inode->mode & 0xf000)
    {
        case 0xC000: strcpy(mode, "socket"); break;
        case 0xA000: strcpy(mode, "symbolic link"); break;
        case EXT2_I_FTYPE: strcpy(mode, "regular file"); break;
        case 0x6000: strcpy(mode, "block device"); break;
        case EXT2_I_DTYPE: strcpy(mode, "directory"); break;
        case 0x2000: strcpy(mode, "character device"); break;
        case 0x1000: strcpy(mode, "fifo"); break;
        default:     sprintf(mode, "0x%x", inode->mode); break;
    }

    printf("###############################\n");

    printf("# mode: 0x%3x %-15s #\n", 0xfff & inode->mode, mode);
    printf("# uid: %-22hu #\n", inode->uid);
    printf("# size: %-21u #\n", inode->size);
    printf("# access_time: %-14u #\n", inode->access_time);
    if (inode->access_time != 0u)
        printf("# %-27s #\n", get_time_format(inode->access_time));
    printf("# creation_time: %-12u #\n", inode->creation_time);
    if (inode->creation_time != 0u)
        printf("# %-27s #\n", get_time_format(inode->creation_time));
    printf("# modification_time: %-8u #\n", inode->modification_time);
    if (inode->modification_time != 0u)
        printf("# %-27s #\n", get_time_format(inode->modification_time));
    printf("# deletion_time: %-12u #\n", inode->deletion_time);
    if (inode->deletion_time != 0u)
        printf("# %-27s #\n", get_time_format(inode->deletion_time));
    printf("# gid: %-22hu #\n", inode->gid);
    printf("# link_count: %-15hu #\n", inode->link_count);
    printf("# block_count_512: %-10u #\n", inode->block_count_512);
    printf("# flags: %-20u #\n", inode->flags);
    printf("# reserved: %-17u #\n", inode->reserved);

    printf("# direct_blocks:              #\n");
    for (int i = 0; i < EXT2_NUM_DIRECT_BLOCKS; i++) {
        printf("#   direct_blocks[%2d]: %-6u #\n", i + 1, inode->direct_blocks[i]);
    }

    printf("# single_indirect: %-10u #\n", inode->single_indirect);
    printf("# double_indirect: %-10u #\n", inode->double_indirect);
    printf("# triple_indirect: %-10u #\n", inode->triple_indirect);

    printf("###############################\n");
    printf("\n");
}
void print_super_block(const struct ext2_super_block* super_block)
{
    printf("###############################\n");
    printf("#   EXT2 SUPER BLOCK DETAILS  #\n");
    printf("# inode_count: %-14u #\n", super_block->inode_count);
    printf("# block_count: %-14u #\n", super_block->block_count);
    printf("# reserved_block_count: %-5u #\n", super_block->reserved_block_count);
    printf("# free_block_count: %-9u #\n", super_block->free_block_count);
    printf("# free_inode_count: %-9u #\n", super_block->free_inode_count);
    printf("# first_data_block: %-9u #\n", super_block->first_data_block);
    printf("# log_block_size: %-11u #\n", super_block->log_block_size);
    printf("# blocks_per_group: %-9u #\n", super_block->blocks_per_group);
    printf("# inodes_per_group: %-9u #\n", super_block->inodes_per_group);
    printf("# first_inode: %-14u #\n", super_block->first_inode);
    printf("# inode_size: %-15hu #\n", super_block->inode_size);
    printf("# block_group_nr: %-11hu #\n", super_block->block_group_nr);
    printf("\n");
}
void print_group_descriptor(const struct ext2_block_group_descriptor* group_descriptor)
{
    printf("###############################\n");
    printf("# EXT2 BLOCK GROUP DESCRIPTOR #\n");
    printf("# block_bitmap : %-12u #\n", group_descriptor->block_bitmap);
    printf("# inode_bitmap : %-12u #\n", group_descriptor->inode_bitmap);
    printf("# inode_table: %-14u #\n", group_descriptor->inode_table);
    printf("# free_block_count : %-8hu #\n", group_descriptor->free_block_count);
    printf("# free_inode_count : %-8hu #\n", group_descriptor->free_inode_count);
    printf("# used_dirs_count: %-10hu #\n", group_descriptor->used_dirs_count);
    printf("# pad: %-22hu #\n", group_descriptor->pad);
    printf("###############################\n");
    printf("\n");
}
void print_bitmap(uint8_t *block_bitmap, uint32_t m) {
    printf("Block Bitmap:\n");
    for (uint32_t i = 0; i < (m + 7) / 8; i++) {
        for (int bit = 0; bit < 8; bit++) {
            if (i * 8 + bit < m) {
                printf("%d", (block_bitmap[i] & (1 << bit)) ? 1 : 0);
            }
        }
        printf(" ");  // separate bytes for better readability
    }
    printf("\n");
}

void read_signature(char *argv[], uint8_t signature[32]) {
    for (int i = 0; i < 32; i++) {
        signature[i] = strtoul(argv[2 + i], NULL, 16);
    }
}

void mark_inode_used(uint8_t *inode_bitmap, int block) {
    int byte_index = block / 8;

    // Calculate the bit position within the byte
    int bit_position = block % 8;

    // Set the bit in the byte
    inode_bitmap[byte_index] |= (1 << bit_position);}

void mark_block_used(uint8_t* block_bitmap, uint32_t block_num) {

    int byte_offset = block_num / 8;
    int bit_offset = block_num % 8;
    block_bitmap[byte_offset] |= (1 << bit_offset);
}
int is_inode_active(uint8_t* inode_bitmap, uint32_t inode_num) {
    int byte_offset = inode_num / 8;
    int bit_offset = inode_num % 8;
    return (inode_bitmap[byte_offset] & (1 << bit_offset)) != 0;
}
int is_block_empty(uint8_t *block) {
    for (int i = 0; i < 245; i++) {
        if (block[i] != 0) {
            return 0;
        }

    }
    return 1;
}

int is_user_data_block(int fd, uint32_t block, uint32_t block_size, uint8_t signature[32]) {
    uint8_t block_data[32];
    pread(fd, block_data, 32, block * block_size);
    return memcmp(block_data, signature, 32) == 0;
}


void process_indirect_blocks(int fd, uint32_t block, uint8_t *block_bitmap, uint32_t block_size, uint8_t signature[32]) {
    if (block == 0) return;

    uint32_t *indirect_block = (uint32_t *)malloc(block_size);
    pread(fd, indirect_block, block_size, block * block_size);
    printf("bb");
    for (uint32_t i = 0; i < block_size / sizeof(uint32_t); i++) {
        if (indirect_block[i] != 0) {
            if (/*is_user_data_block(fd, indirect_block[i], block_size, signature)*/1) {
                if (!(block_bitmap[indirect_block[i] / 8] & (1 << (indirect_block[i] % 8)))) {
                    printf("Indirect Block %u marked as used but not set in bitmap\n", indirect_block[i]);
                }
              //  mark_block_used(block_bitmap, indirect_block[i]);
            }
        }
    }

    free(indirect_block);
}

void process_double_indirect_blocks(int fd, uint32_t block, uint8_t *block_bitmap, uint32_t block_size, uint8_t signature[32]) {
    if (block == 0) return;

printf("cc");
    uint32_t num_pointers = block_size / sizeof(uint32_t);

    uint32_t *double_indirect_block = (uint32_t *)malloc(block_size);
    if (double_indirect_block == NULL) {
        // Handle memory allocation failure (e.g., log an error and return)
        return;
    }
    pread(fd, double_indirect_block, block_size, block * block_size);

    for (uint32_t i = 0; i < block_size / sizeof(uint32_t); i++) {
        if (double_indirect_block[i] != 0) {
            process_indirect_blocks(fd, double_indirect_block[i], block_bitmap, block_size, signature);
        }
    }

    free(double_indirect_block);
}

void process_triple_indirect_blocks(int fd, uint32_t block, uint8_t *block_bitmap, uint32_t block_size, uint8_t signature[32]) {
    if (block == 0) return;
printf("dd");
    uint32_t *triple_indirect_block = (uint32_t *)malloc(block_size);
    pread(fd, triple_indirect_block, block_size, block * block_size);

    for (uint32_t i = 0; i < block_size / sizeof(uint32_t); i++) {
        if (triple_indirect_block[i] != 0) {
            process_double_indirect_blocks(fd, triple_indirect_block[i], block_bitmap, block_size, signature);
        }
    }

    free(triple_indirect_block);
}

void recover_block_bitmap(int fd, struct ext2_super_block *sb, struct ext2_block_group_descriptor *bgdt, uint32_t block_groups, uint32_t block_size, uint8_t signature[32]) {
    for (uint32_t i = 0; i < block_groups-1; i++) {
        uint32_t block_bitmap_block = bgdt[i].block_bitmap;
        uint32_t inode_table_block = bgdt[i].inode_table;

        uint8_t *block_bitmap = (uint8_t *)malloc(256);
        if (block_bitmap == NULL) { perror("Failed to allocate memory"); close(fd);}

        ssize_t bytes_read = pread(fd, block_bitmap, 256, block_bitmap_block * block_size);
        if (bytes_read < 0) {perror("Failed to read from file");free(block_bitmap);close(fd);}
        if (bytes_read != 256) {fprintf(stderr, "Incomplete readd\n");free(block_bitmap);close(fd);}

        int data_offset = bgdt[i].inode_table + (sb->inodes_per_group*sb->inode_size)/block_size;
        printf("table offset is %u data offset is %d \n", bgdt[i].inode_table, data_offset);

        print_bitmap(block_bitmap, 256);
        if(bgdt[i].free_block_count==0) {
            for (int f = 0; f < 256; f++) {
                mark_inode_used(block_bitmap, f);
            }
            uint8_t *blockbitmapxx = (uint8_t *)malloc(256);
            pwrite(fd, block_bitmap, 256, block_bitmap_block * block_size);
            bytes_read = pread(fd, blockbitmapxx, 256, block_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(blockbitmapxx, 256);
            free(blockbitmapxx);
            continue;
        }
        else{
            for (int k=0;k<256;k++){
                uint8_t *block = (uint8_t *)malloc(block_size);
                if (block == NULL) {
                    perror("Failed to allocate memory");
                    close(fd);
                }

                ssize_t bytes_readx = pread(fd, block, block_size, inode_table_block * block_size  + k*block_size);
                if (bytes_readx < 0) {
                    perror("Failed to read from file");
                    free(block);
                    close(fd);
                }
                if (bytes_readx != block_size) {
                    fprintf(stderr, "Incomplete readd\n");
                    free(block);
                    close(fd);
                }
                int is_empty= is_block_empty(block);
              //  printf("is empty is %d \n",is_empty);
                if(!is_empty){
                    mark_inode_used(block_bitmap,k);
                }

             //   print_bitmap(block,block_size);
                free(block);
            }
            uint8_t *blockbitmapxxx = (uint8_t *)malloc(256);
            pwrite(fd, block_bitmap, 256, block_bitmap_block * block_size);
            bytes_read = pread(fd, blockbitmapxxx, 245, block_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(blockbitmapxxx, 256);
            free(blockbitmapxxx);
}
}
}
// Function to recover the inode bitmap

void recover_block_bitmap2(int fd, struct ext2_super_block *sb, struct ext2_block_group_descriptor *bgdt, uint32_t block_groups, uint32_t block_size, uint8_t signature[32]) {
    for (uint32_t i = 0; i < block_groups; i++) {
        uint32_t block_bitmap_block = bgdt[i].block_bitmap;
        uint32_t inode_table_block = bgdt[i].inode_table;

        uint8_t *block_bitmap = (uint8_t *)malloc(256);
        if (block_bitmap == NULL) { perror("Failed to allocate memory"); close(fd);}

        ssize_t bytes_read = pread(fd, block_bitmap, 256, block_bitmap_block * block_size);
        if (bytes_read < 0) {perror("Failed to read from file");free(block_bitmap);close(fd);}
        if (bytes_read != 256) {fprintf(stderr, "Incomplete readd\n");free(block_bitmap);close(fd);}

        int data_offset = bgdt[i].inode_table + (sb->inodes_per_group*sb->inode_size)/block_size;
        printf("table offset is %u data offset is %d \n", bgdt[i].inode_table, data_offset);

        print_bitmap(block_bitmap, 256);
        if(bgdt[i].free_block_count==0) {
            for (int f = 0; f < 256; f++) {
                mark_inode_used(block_bitmap, f);
            }
            uint8_t *blockbitmapxx = (uint8_t *)malloc(256);
            pwrite(fd, block_bitmap, 256, block_bitmap_block * block_size);
            bytes_read = pread(fd, blockbitmapxx, 256, block_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(blockbitmapxx, 256);
            free(blockbitmapxx);
            continue;
        }
        else{

        uint32_t inode_bitmap_block = bgdt[i].inode_bitmap;
        uint32_t inode_table_block = bgdt[i].inode_table;
        uint32_t num_inodes = sb->inodes_per_group;
        uint8_t *inode_bitmap = (uint8_t *)malloc(block_size);
        if (inode_bitmap == NULL) {perror("Failed to allocate memory");close(fd);}
        ssize_t bytes_read = pread(fd, inode_bitmap, block_size, inode_bitmap_block * block_size);
        if (bytes_read < 0) {perror("Failed to read from file");free(inode_bitmap);close(fd);}
        if (bytes_read != block_size) {fprintf(stderr, "Incomplete readd\n");free(inode_bitmap);close(fd);}


            struct ext2_inode *inode_table = (struct ext2_inode *)malloc(num_inodes* sb->inode_size);
            if (inode_table == NULL) {perror("Failed to allocate memory for inode table");close(fd);}
            off_t inode_table_offset = inode_table_block * block_size;
            ssize_t bytes_readd = pread(fd, inode_table, num_inodes* sb->inode_size, inode_table_offset);
            if (bytes_readd < 0) {perror("Failed to read inode table from file");free(inode_table);close(fd);}
            else if (bytes_readd != num_inodes * sb->inode_size) {fprintf(stderr, "Incomplete read of inode table\n");free(inode_table);close(fd);}
        
            for (uint32_t b = 0; b < sb->inodes_per_group; b++) {
    
            struct ext2_inode *inode = (struct ext2_inode *)((char *)inode_table + b * sb->inode_size);
               // print_inode(inode);
                if( inode->link_count>0){
                    printf("bddd\n");
                for (uint32_t k = 0; k < EXT2_NUM_DIRECT_BLOCKS; k++) {
                    if (inode->direct_blocks[k] > 0) {
                        if (is_user_data_block(fd, inode->direct_blocks[k], block_size, signature)) {
                            printf("ss");
                            mark_block_used(block_bitmap, inode->direct_blocks[k]);
                        }
                    }
                }


                // Handle indirect blocks
                process_indirect_blocks(fd, inode->single_indirect, block_bitmap, block_size, signature);
                process_double_indirect_blocks(fd, inode->double_indirect, block_bitmap, block_size, signature);
                process_triple_indirect_blocks(fd, inode->triple_indirect, block_bitmap, block_size, signature);
            }
        }
        print_bitmap(block_bitmap, 256);
        // Write back the updated block bitmap
        if (pwrite(fd, block_bitmap, block_size, block_bitmap_block * block_size) != block_size) {
            perror("Failed to write block bitmap");
            return;
        }
        
        uint8_t *block_bitmapx = (uint8_t *)malloc(block_size);
            pwrite(fd, block_bitmap, block_size, block_bitmap_block * block_size);
            bytes_read = pread(fd, block_bitmapx, block_size, block_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(block_bitmapx, 256);
            free(block_bitmapx);
            free(block_bitmap);
            free(inode_table);
    }

}
}

void recover_inode_bitmap(int fd, struct ext2_super_block *sb, struct ext2_block_group_descriptor *bgdt, uint32_t block_groups, uint32_t block_size) {
    for (int i = 0; i < block_groups; i++) {

        uint32_t inode_bitmap_block = bgdt[i].inode_bitmap;
        uint32_t inode_table_block = bgdt[i].inode_table;
        printf("inode bitmap is in %u inode table is in %d \n",inode_bitmap_block,inode_table_block);
        uint32_t num_inodes = sb->inodes_per_group;
        uint8_t *inode_bitmap = (uint8_t *)malloc(block_size);
        if (inode_bitmap == NULL) {
            perror("Failed to allocate memory");
            close(fd);

        }
        ssize_t bytes_read = pread(fd, inode_bitmap, block_size, inode_bitmap_block * block_size);
        if (bytes_read < 0) {
            perror("Failed to read from file");
            free(inode_bitmap);
            close(fd);
        }
        if (bytes_read != block_size) {
            fprintf(stderr, "Incomplete readd\n");
            free(inode_bitmap);
            close(fd);
        }
        print_bitmap(inode_bitmap, 32);
        if(bgdt[i].free_inode_count==0){
            for (int f=0;f<32;f++){
                mark_inode_used(inode_bitmap,f);
            }
            uint8_t *inode_bitmapx = (uint8_t *)malloc(block_size);
            pwrite(fd, inode_bitmap, block_size, inode_bitmap_block * block_size);
            pread(fd, inode_bitmapx, block_size, inode_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(inode_bitmapx, 32);
            continue;
        }
        else{
            struct ext2_inode *inode_table = (struct ext2_inode *)malloc(num_inodes * sb->inode_size);
            if (inode_table == NULL) {
                perror("Failed to allocate memory for inode table");
                free(inode_bitmap);
                close(fd);
            }

            off_t inode_table_offset = inode_table_block * block_size;
            ssize_t bytes_readd = pread(fd, inode_table, num_inodes * sb->inode_size, inode_table_offset);
            if (bytes_readd < 0) {
                perror("Failed to read inode table from file");
                free(inode_table);
                close(fd);
            }
            else if (bytes_readd != num_inodes * sb->inode_size) {
                fprintf(stderr, "Incomplete read of inode table\n");
                free(inode_table);
                close(fd);
            }

            // Iterate over each inode in the table and print it
            for (int  b = 0; b < num_inodes; b++) {
                struct ext2_inode *inode = (struct ext2_inode *)((char *)inode_table + b * sb->inode_size);
               // print_inode(inode);
                if(inode->deletion_time == 0 && inode->link_count>0){
                    mark_inode_used(inode_bitmap,b);
                }

               // print_inode(&inode_table[b]);
                //check_and_print_found(fd, &inode_table[i]);
            }
            uint8_t *inode_bitmapx = (uint8_t *)malloc(num_inodes);
            pwrite(fd, inode_bitmap, sb->inodes_per_group, inode_bitmap_block * block_size);
            bytes_read = pread(fd, inode_bitmapx, sb->inodes_per_group, inode_bitmap_block * block_size);
            printf("new one is %d\n", i);
            print_bitmap(inode_bitmapx, 32);
            free(inode_bitmapx);
            free(inode_bitmap);
            free(inode_table);

        }
        }



}

void read_inode(int fd, struct ext2_super_block *sb, struct ext2_block_group_descriptor *bgdt, uint32_t inode_num, struct ext2_inode *inode) {
    uint32_t block_group = (inode_num - 1) / sb->inodes_per_group;
    uint32_t index = (inode_num - 1) % sb->inodes_per_group;
    uint32_t inode_table_block = bgdt[block_group].inode_table;
    uint32_t inode_offset = inode_table_block * (1024 << sb->log_block_size) + index * sb->inode_size;
    if (pread(fd, inode, sb->inode_size, inode_offset) != sb->inode_size) {
        perror("Failed to read inode");
        exit(1);
    }
}



void print_directory_tree(int fd, struct ext2_super_block *sb, struct ext2_block_group_descriptor *bgdt, struct ext2_inode *inode, const char *path, int depth) {
    if (depth > 100) {
        fprintf(stderr, "Too deep recursion. Potential loop detected.\n");
        return;
    }
    
    char new_path[1024];
    struct ext2_dir_entry *entry;
    int block_size = 1024 << sb->log_block_size;
    char *block = (char *)malloc(block_size);
    int block_num;

    for (int i = 0; i < EXT2_NUM_DIRECT_BLOCKS; i++) {
        block_num = inode->direct_blocks[i];
        if (block_num == 0) break;

        if (pread(fd, block, block_size, block_num * block_size) != block_size) {
            perror("Failed to read directory block");
            free(block);
            exit(1);
        }

        int offset = 0;
        while (offset < block_size) {
            entry = (struct ext2_dir_entry *)(block + offset);
            if (entry->inode) {
                if (strncmp(entry->name, ".", entry->name_length) == 0 ||
                    strncmp(entry->name, "..", entry->name_length) == 0) {
                    // Skip printing "." and ".." entries explicitly
                    entry->name_length = 0;
                } else {
                    strncpy(new_path, path, sizeof(new_path) - 1);
                    new_path[sizeof(new_path) - 1] = '\0';

                    strncat(new_path, "/", sizeof(new_path) - strlen(new_path) - 1);
                    strncat(new_path, entry->name, entry->name_length);
                    new_path[sizeof(new_path) - 1] = '\0';

                    for (int j = 0; j < depth; j++) printf("--");
                    printf(" %.*s", entry->name_length, entry->name);

                    // Check if it's a directory
                    struct ext2_inode child_inode;
                    read_inode(fd, sb, bgdt, entry->inode, &child_inode);
                    if (S_ISDIR(child_inode.mode) && entry->name_length > 0 &&
                        strncmp(entry->name, ".", entry->name_length) != 0 &&
                        strncmp(entry->name, "..", entry->name_length) != 0) {
                        printf("/\n");
                        print_directory_tree(fd, sb, bgdt, &child_inode, new_path, depth + 1);
                    } else {
                        printf("\n");
                    }
                }
            }
            offset += entry->length;
        }
    }
    free(block);
}






// Main function to combine the recovery processes
int main(int argc, char *argv[]) {
    if (argc < 34) { // 1 for program name, 1 for image file, 32 for the signature bytes
        fprintf(stderr, "Usage: %s <image file> <32-byte signature in hex>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t signature[32];
    memcpy(signature, argv[2], 32);


    struct ext2_super_block superblock;
    pread(fd, &superblock, sizeof(superblock), EXT2_SUPER_BLOCK_POSITION);
    uint32_t block_size = 1024 << superblock.log_block_size;
    uint32_t block_groups = (superblock.block_count + superblock.blocks_per_group - 1) / superblock.blocks_per_group;


    print_super_block(&superblock);

    struct ext2_block_group_descriptor *bgdt = (struct ext2_block_group_descriptor *)malloc(block_groups * sizeof(struct ext2_block_group_descriptor));
    pread(fd, bgdt, block_groups * sizeof(struct ext2_block_group_descriptor), 2 * block_size);

    print_group_descriptor(&bgdt[0]);
    print_group_descriptor(&bgdt[1]);
    print_group_descriptor(&bgdt[2]);
    print_group_descriptor(&bgdt[3]);
    print_group_descriptor(&bgdt[4]);
    print_group_descriptor(&bgdt[5]);
    print_group_descriptor(&bgdt[6]);
    print_group_descriptor(&bgdt[7]);

   /* uint32_t inode_table = bgdt[2].inode_table;

    for (uint32_t j = 1; j < superblock.inodes_per_group; j++) {
        struct ext2_inode inode;
        pread(fd, &inode, sizeof(inode), (inode_table * block_size) + (j * sizeof(struct ext2_inode)));
        print_inode(&inode);
    }*/

   recover_inode_bitmap(fd, &superblock, bgdt, block_groups, block_size);
    recover_block_bitmap(fd, &superblock, bgdt, block_groups, block_size, signature);
    struct ext2_inode root_inode;
    read_inode(fd, &superblock, bgdt, EXT2_ROOT_INODE, &root_inode);

    printf("- root/\n");
    char root_path[1024] = "/";
    print_directory_tree(fd, &superblock, bgdt, &root_inode, root_path, 1);

   /*printf("- /root\n");
    print_directory_tree(fd, &superblock, &bgdt, &root_inode, "/root", 1);*/


    free(bgdt);

    close(fd);
    return 0;
}
