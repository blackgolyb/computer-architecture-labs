#include <stdio.h>
#include <dos.h>
#include <conio.h>

#define MBR_CODE_SIZE 0x1BE
#define PARTS_COUNT 4
#define VENDOR_LENGTH 8
#define NAME_LENGTH 11
#define FS_NAME_LENGTH 8
#define FSI_IN_SECTOR 16
#define FAT_ITEMS 128

typedef unsigned char byte;
typedef unsigned short int16_t;
typedef unsigned long int32_t;

struct BootRecord {
	byte bootOffset[3];
	char vendor[VENDOR_LENGTH];
	int16_t sectorSize;
	byte clusterSize;
	int16_t reserveSectors;
	byte fatCopies;
	int16_t itemsInRoot;
	int16_t totalSectorsInDir;
	byte deviceType;
	int16_t fatSize16;
	int16_t sectorsPerCylinder;
	int16_t headsAmount;
	int32_t sectorsBeforePart;
	int32_t totalSectorsInPart;
	int32_t fatSize32;
	int16_t flagsA, version;
	int32_t offsetToRootDir;
	int16_t fsInformationSector;
	int16_t reserveFsInfo;
	byte unused[12];
	byte diskIndex, flagsB;
	byte bootRecordType;
	int32_t serialPartNumber;
	char name[NAME_LENGTH];
	char fsName[FS_NAME_LENGTH];
	byte bootCode[420];
	int16_t sign;
};

struct PART {
	byte isBoot, startHead, startSector, startCylinder;
	byte fileSystem, endHead, endSector, endCylinder;
	int32_t offsetSectors, amountSectors;
};

struct MBR {
	byte bootProgram[MBR_CODE_SIZE];
	struct PART parts[PARTS_COUNT];
	int16_t sign;
};

struct DiskPosition {
	byte sector;
	byte cylinder;
	byte head;
};

struct FSItem {
	char name[8];
	char extension[3];
	byte attributes;
	byte registers;
	byte msCreationTime;
	int16_t creationTime;
	int16_t creationDate;
	int16_t dateLastRW;
	int16_t highCluster;
	int16_t editTime;
	int16_t editDate;
	int16_t lowCluster;
	int32_t size;
};

struct DiskPosition toDiskPosition(int32_t address,
														struct BootRecord* pbr) {
	struct DiskPosition dp;
	int32_t sectorNumber = address / pbr->sectorSize;
	int16_t spc = pbr->sectorsPerCylinder;
	int32_t spch = spc * pbr->headsAmount;

	dp.cylinder = sectorNumber / spch;
	dp.head = (sectorNumber % spch) / spc;
	dp.sector = (sectorNumber % spc) + 1;

	return dp;
}

int32_t fromDiskPosition(struct DiskPosition dp, struct BootRecord* pbr) {
	int32_t spc = pbr->sectorsPerCylinder;
	int32_t sectorNumber = (dp.cylinder * pbr->headsAmount * spc)
											 + (dp.head * spc) + (dp.sector - 1);

	return sectorNumber * pbr->sectorSize;
}

printChars(char* buff, int length) {
	int i;
	for (i = 0; i < length; i++) {
		putchar(buff[i]);
	}
}

int read(struct DiskPosition dp, void* buff) {
	union REGS in;
	struct SREGS sr;

	in.h.ah = 2;            // cmd read
	in.h.dl = 0x80;         // by hard-disk
	in.h.cl = dp.sector;    // sector
	in.h.ch = dp.cylinder;  // track (cylinder)
	in.h.al = 1;            // count sector
	in.h.dh = dp.head;      // head

	sr.es   = FP_SEG(buff); // copy buff adress
	in.x.bx = FP_OFF(buff);

	int86x(0x13, &in, &in, &sr);

	if (in.x.cflag) {
		return in.h.ah;
	}

	return 0;
}

void countParts(struct PART* parts) {
	int i, fsCount = 0;
	byte fileSystem;

	printf("--- Amount of used parts and their types ---\n");
	for (i = 0; i < PARTS_COUNT; i++) {
		fileSystem = parts[i].fileSystem;
		if (fileSystem) {
			fsCount++;
			printf("Found part type %Xh ", fileSystem);
			if (fileSystem == 0xB) printf("(Win95 FAT32)");
			putchar('\n');
		 }

	}
	printf("Parts amount: %d\n", fsCount);
}

void printStructPart(struct PART p) {
	printf("--- Part Info  ---\n");
	printf("IsActive: %X\n", p.isBoot);
	printf("Start - head: %d cylider: %d sector: %d\n",
				p.startHead, p.startCylinder, p.startSector);
	printf("End - head: %d cylider: %d sector: %d\n",
				p.endHead, p.endCylinder, p.endSector);
	printf("Sectors - Offset: %ld Amount: %ld \n",
				p.offsetSectors, p.amountSectors);
}

void printStructBootRecord(struct BootRecord* pbr) {
	printf("--- Boot Record Info  ---\n");
	printf("Sector size: %d\n", pbr->sectorSize);
	printf("Sectors per cylinder: %d\n", pbr->sectorsPerCylinder);
	printf("Heads amount: %d\n", pbr->headsAmount);
	printf("Cluster size: %d\n", pbr->clusterSize);
	printf("Reserve sectors: %d\n", pbr->reserveSectors);
	printf("Fat copies: %d\n", pbr->fatCopies);
	printf("Fat size: %lu\n", pbr->fatSize32);
	printf("Root size: %u\n", pbr->itemsInRoot);

	printf("Vendor: ");
	printChars(&(pbr->vendor[0]), VENDOR_LENGTH);
	putchar('\n');

	printf("Name: ");
	printChars(&(pbr->name[0]), NAME_LENGTH);
	putchar('\n');

	printf("File System: ");
	printChars(&(pbr->fsName[0]), FS_NAME_LENGTH);
	putchar('\n');
}

void printFatDate(int16_t date) {
	int16_t day = date & 0x1F;
	int16_t month = (date >> 5) & 0xF ;
	int16_t year = ((date >> 9) & 0x7F) + 1980;
	if (date == 0) printf("Unknown");
	else printf("%02u.%02u.%04u", day, month, year);
}

void printFatTime(int16_t time) {
	int16_t seconds = (time & 0x1F) << 1;
	int16_t minutes = (time >> 5) & 0x3F;
	int16_t hours = (time >> 11) & 0x1F;
	if (time == 0) return;
	printf("%02u:%02u:%02u", hours, minutes, seconds);
}

void printFatAttributes(byte flags) {
	putchar((flags & 0x01) ? 'R' : '-');
	putchar((flags & 0x02) ? 'H' : '-');
	putchar((flags & 0x04) ? 'S' : '-');
	putchar((flags & 0x08) ? 'V' : '-');
	putchar((flags & 0x10) ? 'D' : '-');
	putchar((flags & 0x20) ? 'A' : '-');
}

void printFatSize(int32_t size) {
	int32_t gb = size >> 30;
	int32_t mb = (size - (gb << 30)) >> 20;
	int32_t kb = (size - (gb << 30) - (mb << 20)) >> 10;
	int32_t b  = (size - (gb << 30) - (mb << 20) - (kb << 10));

	if (gb) printf("%lu Gib ", gb);
	if (mb) printf("%lu Mib ", mb);
	if (kb) printf("%lu Kib ", kb);
	if  (b) printf("%lu b");
	if (size == 0) putchar('0');
}

int32_t getFatCluster(struct FSItem item) {
	int32_t high = item.highCluster;
	int32_t low = item.lowCluster;
	return (high << 16) | low;
}

void printFileSystemItem(struct FSItem item) {
	printChars(item.name, 8);
	putchar('.');
	printChars(item.extension, 3);
	putchar('\n');

	printFatAttributes(item.attributes);
	putchar('\n');

	printf("Creation: ");
	printFatDate(item.creationDate);
	putchar(' ');
	printFatTime(item.creationTime);
	putchar('\n');

	printf("Edit: ");
	printFatDate(item.editDate);
	putchar(' ');
	printFatTime(item.editTime);
	putchar('\n');

	printf("Size: ");
	printFatSize(item.size);
	putchar('\n');

	printf("Cluster: %lX\n", getFatCluster(item));
}

int main() {
	struct MBR mbr;
	struct BootRecord br;
	struct PART mainPart;
	struct DiskPosition dp, offsetDp;
	int16_t error;
	int32_t fatAddress, rootAddress, offset;
	int32_t fs[FAT_ITEMS];
	struct FSItem fsItems[FSI_IN_SECTOR];
	int16_t i;

	clrscr();

	printf("--- Sizes of structs in Bytes  ---\n");
	printf("struct PART: %d\n", sizeof(struct PART));
	printf("struct MBR: %d\n", sizeof(struct MBR));
	printf("struct BootRecord: %d\n", sizeof(struct BootRecord));
	printf("struct FSItem: %d\n", sizeof(struct FSItem));

	dp.head = 0;
	dp.cylinder = 0;
	dp.sector = 1;
	error = read(dp, &mbr);
	if (error) {
		printf("Read error: %d\n", error);
		return error;
	}

	countParts(&mbr.parts[0]);
	mainPart = mbr.parts[0];

	printStructPart(mainPart);


	dp.head = mainPart.startHead;
	dp.cylinder = mainPart.startCylinder;
	dp.sector = mainPart.startSector;
	offsetDp = dp;
	error = read(dp, &br);

	if (error) {
		printf("Read error: %d\n", error);
		return error;
	}

	printStructBootRecord(&br);

	printf("--- Addresses ---\n");

	offset = fromDiskPosition(offsetDp, &br);
	printf("Offset partition: %X\n", offset);

	printf("--- FAT Table  ---\n");

	fatAddress = br.reserveSectors * br.sectorSize + offset;
	printf("FAT: 0x%X\n", fatAddress);
	dp = toDiskPosition(fatAddress, &br);
	printf("Head: %d\n", dp.head);
	printf("Cylinder: %d\n", dp.cylinder);
	printf("Sector: %d\n", dp.sector);

	printf("--- ROOT Dirictory ---\n");

	rootAddress = ((br.reserveSectors + br.fatSize32 * br.fatCopies)
							+ (br.offsetToRootDir - 2) * br.clusterSize) * br.sectorSize;
	rootAddress += offset;
	printf("ROOT: 0x%lX\n", rootAddress);
	dp = toDiskPosition(rootAddress, &br);
	printf("Head: %d\n", dp.head);
	printf("Cylinder: %d\n", dp.cylinder);
	printf("Sector: %d\n", dp.sector);

	dp = toDiskPosition(fatAddress, &br);

	error = read(dp, fs);
	if (error) {
		printf("Read error: %d\n", error);
		return error;
	}


	if (fs[0] != 0x0FFFFFF8 ||
			fs[1] != 0x0FFFFFFF) {
			printf("Error can't found FAT table\n");
			printf("%lX\n", fs[0]);
			printf("%lX\n", fs[1]);
			return 1;
	} else {
		printf("Ok FAT table is valid\n");
	}

	printf("--- Files ---\n");

	dp = toDiskPosition(rootAddress, &br);
	error = read(dp, fsItems);
	if (error) {
		printf("Read error: %d\n", error);
		return error;
	}

	for (i = 0; i < FSI_IN_SECTOR; i++) {
		putchar('\n');
		printFileSystemItem(fsItems[i]);
	}

	return 0;
}