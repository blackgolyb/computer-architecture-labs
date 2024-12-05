#include <stdio.h>
#include <dos.h>

#define MBR_CODE_SIZE 0x1BE
#define PARTS_COUNT 4
#define VENDOR_LENGTH 8
#define NAME_LENGTH 11
#define FS_NAME_LENGTH 8

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
	printf("Fat size: %d\n", pbr->fatSize32);

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

int main() {
	struct MBR mbr;
	struct BootRecord br;
	struct PART mainPart;
	struct DiskPosition dp;
	int16_t error;
	int32_t fatAddress, rootAddress;

	printf("--- Sizes of structs in Bytes  ---\n");
	printf("struct PART: %d\n", sizeof(struct PART));
	printf("struct MBR: %d\n", sizeof(struct MBR));
	printf("struct BootRecord: %d\n", sizeof(struct BootRecord));

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
	error = read(dp, &br);

	if (error) {
		printf("Read error: %d\n", error);
		return error;
	}

	printStructBootRecord(&br);

	printf("--- Addresses ---\n");

	fatAddress = br.reserveSectors * br.sectorSize;
	printf("FAT: 0x%X\n", fatAddress);
	dp = toDiskPosition(fatAddress, &br);
	printf("Head: %d\n", dp.head);
	printf("Cylinder: %d\n", dp.cylinder);
	printf("Sector: %d\n", dp.sector);

	rootAddress = ((br.reserveSectors + br.fatSize32 * br.fatCopies)
							+ (br.offsetToRootDir - 2) * br.clusterSize) * br.sectorSize;
	printf("ROOT: 0x%lX\n", rootAddress);
	dp = toDiskPosition(rootAddress, &br);
	printf("Head: %d\n", dp.head);
	printf("Cylinder: %d\n", dp.cylinder);
	printf("Sector: %d\n", dp.sector);

	return 0;
}