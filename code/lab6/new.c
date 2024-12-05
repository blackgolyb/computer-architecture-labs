#include <lab.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>

#define SECTOR_SIZE 512
#define PARTITIONS 4
#define FS_TYPE_FAT32 0xB


typedef struct {
    u8 bootable;        // Boot indicator
    u8 start_head;
    u8 start_sector;
    u8 start_cylinder;
    u8 fs_type;
    u8 end_head;
    u8 end_sector;
    u8 end_cylinder;
    u32 start_sector_abs; // Relative start sector
    u32 total_sectors;    // Total sectors
} part_t;

typedef struct {
    u8 vendor[8];
    u16 sector_size;
    u8 cluster_size;
    u16 reserved_sectors;
    u8 fat_copies;
    u16 root_entries;
    u16 total_sectors_short;
    u8 media_type;
    u16 fat_size_16;
    u16 sectors_per_track;
    u16 number_of_heads;
    u32 hidden_sectors;
    u32 total_sectors_long;
    u32 fat_size_32;
    u16 flags;
    u16 version;
    u32 root_cluster;
    u16 fs_info_sector;
    u16 backup_boot_sector;
    u8 reserved[12];
    u8 drive_number;
    u8 flags_nt;
    u8 signature;
    u32 volume_serial;
    char volume_label[11];
    char fs_type[8];
} boot_record_t;

typedef struct {
    u8 boot_code[446];
    part_t partitions[PARTITIONS];
    u16 signature;
} mbr_t;

typedef struct {
    u8 sector;
    u8 cylinder;
    u8 head;
} disk_pos_t;

disk_pos_t
to_disk_pos(u32 addr, boot_record_t* br) {
    disk_pos_t pos;
    u32 sectors_per_cylinder = br->sectors_per_track * br->number_of_heads;
    u32 sector_num = addr / br->sector_size;

    pos.cylinder = sector_num / sectors_per_cylinder;
    pos.head = (sector_num % sectors_per_cylinder) / br->sectors_per_track;
    pos.sector = (sector_num % br->sectors_per_track) + 1;

    return pos;
}

void
print_partition_info(part_t* part) {
    printf("Partition Info:\n");
    printf("  Bootable: %02X\n", part->bootable);
    printf("  Start - Head: %d, Sector: %d, Cylinder: %d\n",
           part->start_head, part->start_sector, part->start_cylinder);
    printf("  End   - Head: %d, Sector: %d, Cylinder: %d\n",
           part->end_head, part->end_sector, part->end_cylinder);
    printf("  Start Sector (ABS): %lu\n", part->start_sector_abs);
    printf("  Total Sectors: %lu\n", part->total_sectors);
    printf("  File System: %02X\n", part->fs_type);
}

void
print_boot_record(boot_record_t* br) {
    i16 i = 0;
    printf("Boot Record Info:\n");
    printf("  Sector Size: %d\n", br->sector_size);
    printf("  Cluster Size: %d\n", br->cluster_size);
    printf("  Reserved Sectors: %d\n", br->reserved_sectors);
    printf("  FAT Copies: %d\n", br->fat_copies);
    printf("  Volume Label: ");
    for (i = 0; i < 11; i++) putchar(br->volume_label[i]);
    putchar('\n');
    printf("  File System: ");
    for (i = 0; i < 8; i++) putchar(br->fs_type[i]);
    putchar('\n');
}

u16
read_sector(disk_pos_t* pos, void* buffer) {
    union REGS regs;
    struct SREGS sregs;

    regs.h.ah = 0x02;             // Read sector command
    regs.h.al = 0x01;             // Number of sectors to read
    regs.h.dl = 0x80;             // Hard disk
    regs.h.ch = pos->cylinder;    // Cylinder
    regs.h.cl = pos->sector;      // Sector
    regs.h.dh = pos->head;        // Head

    sregs.es = FP_SEG(buffer);    // Segment of the buffer
    regs.x.bx = FP_OFF(buffer);   // Offset of the buffer

    int86x(0x13, &regs, &regs, &sregs);

    return regs.x.cflag ? regs.h.ah : 0;
}

int
main() {
    i16 i = 0;
    mbr_t mbr;
    boot_record_t br;
    disk_pos_t pos;
    part_t* main_part;
    u16 err;

    // Read MBR
    pos.head = 0;
    pos.cylinder = 0;
    pos.sector = 1;
    err = read_sector(&pos, &mbr);
    if (err) {
        printf("Error reading MBR: %02X\n", err);
        return err;
    }

    printf("Partitions in MBR:\n");
    for (i = 0; i < PARTITIONS; i++) {
        if (mbr.partitions[i].fs_type) {
            print_partition_info(&mbr.partitions[i]);
        }
    }

    main_part = &mbr.partitions[0];

    // Read Boot Record
    pos.head = main_part->start_head;
    pos.cylinder = main_part->start_cylinder;
    pos.sector = main_part->start_sector;
    err = read_sector(&pos, &br);
    if (err) {
        printf("Error reading Boot Record: %02X\n", err);
        return err;
    }

    print_boot_record(&br);

    return 0;
}
