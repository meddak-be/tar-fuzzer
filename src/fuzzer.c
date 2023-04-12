#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct tar_t data;
char EXTRACTOR[25];

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

#define XHDTYPE  'x'            /* Extended header referring to the
                                   next file in the archive */
#define XGLTYPE  'g'            /* Global extended header */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

int SUCCESS, CURRENT;

struct tar_t
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char padding[12];             /* 500 */
};

void init_tar_header(struct tar_t *header, const char *filename, const char *mode) {
    memset(header, 0, sizeof(struct tar_t));
    strncpy(header->name, filename, 100);
    strncpy(header->mode, mode, 8);
    strncpy(header->uid, "0000000", 8);
    strncpy(header->gid, "0000000", 8);
    strncpy(header->magic, "ustar", 6);
    strncpy(header->version, "00", 2);
    strncpy(header->uname, "unknown", 32);
    strncpy(header->gname, "unknown", 32);
    strncpy(header->prefix, "", 155);
    header->typeflag = '0';
    time_t mtime = time(NULL);
    snprintf(header->mtime, 12, "%011lo", (unsigned long) mtime);
}

int check_crash(char* archiveName)
{
    int rv = 0;
    char cmd[51];
    strncpy(cmd, EXTRACTOR, 25);
    cmd[27] = '\0';
    char spaced[20];
    sprintf(spaced, " %s", archiveName);
    strncat(cmd, spaced, 23);

    char buf[33];

    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    if(fgets(buf, 33, fp) == NULL) {
        printf("No output\n");
        goto finally;
    }
    if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
        printf("Not the crash message\n");
        goto finally;
    } else {
        printf("Crash message\n");
        rv = 1;
        goto finally;
    }
    finally:
    if(pclose(fp) == -1) {
        printf("Command not found\n");
        rv = -1;
    }
    
    return rv;
}

/**
 * Computes the checksum for a tar header and encode it on the header
 * @param entry: The tar header
 * @return the value of the checksum
 */
unsigned int calculate_checksum(struct tar_t* entry){
    // use spaces for the checksum bytes while calculating the checksum
    memset(entry->chksum, ' ', 8);

    // sum of entire metadata
    unsigned int check = 0;
    unsigned char* raw = (unsigned char*) entry;
    for(int i = 0; i < 512; i++){
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
}

void write()
{
    FILE *fp;
    char nameStr[40];
    sprintf(nameStr, "archive%d.tar", CURRENT);
    CURRENT++;
    fp = fopen(nameStr,"wb");

    fwrite(&data, sizeof(struct tar_t), 1, fp);
    // write the padding
    char padding[512] = {0};
    fwrite(padding, 512 - (sizeof(struct tar_t) % 512), 1, fp);
    
    fclose(fp);
    //save data as SUCCESS tar.
    int success = check_crash(nameStr);
    if (success)
    {
        FILE *fpsucces;
        char successStr[40];
        sprintf(successStr, "success_archive%d.tar", SUCCESS);
        rename(nameStr, successStr);
        SUCCESS++;
    }
}

void writeTestPadding(char padding[], size_t size_padding)
{
    FILE *fp;
    char nameStr[40];
    sprintf(nameStr, "archive%d.tar", CURRENT);
    CURRENT++;
    fp = fopen(nameStr,"wb");

    fwrite(&data, sizeof(struct tar_t), 1, fp);
    // write the padding
    fwrite(padding, size_padding, 1, fp);
    
    fclose(fp);
    //save data as SUCCESS tar.
    int success = check_crash(nameStr);
    if (success)
    {
        FILE *fpsucces;
        char successStr[40];
        sprintf(successStr, "success_archive%d.tar", SUCCESS);
        rename(nameStr, successStr);
        SUCCESS++;
    }
}

void padding(){
    calculate_checksum(&data);
    size_t sizes_test[5] = {0, 1, 512, (512 - (sizeof(struct tar_t) % 512)), 512*2};
    char padding[512] = {0};
    for (size_t i = 0; i < 5; i++)
    {
        writeTestPadding(padding,  sizes_test[i]);
    }
    
}


void test_field(char *field, size_t size)
{
    //Test for weird ascii and non ascii
    char names[][100] = {"testǷrojetѭچ", "test\n", "test\t", "test\a", "test\r", "\0", "      ","",
    "test\x00name",
    "test\x80name",
    "test\\",
    "test/",
    "test\x1B", // ANSI escape sequence
    "test\x7F",     // DEL character
    "test\x1A",     // SUB character
    "test\x1C",     // FS character
    "test\x1D",     // GS character
    "test\x1E",     // RS character
    "test\x1F",      // US character 
    '\0'
    };

    size_t len = sizeof names / sizeof (char[100]);
    for (size_t i = 0; i < len; i++)
    {
        strcpy(field, names[i]);
        calculate_checksum(&data);
        write();
    }

    // no terminate
    printf("size : %d\n", size);
    memset(field, 'a', size);
    calculate_checksum(&data);
    write();

    //Test for non octal - decimal
    strcpy(field, "9");
    calculate_checksum(&data);
    write();

    // Test for non octal - hex
    char hex[size];
    snprintf(hex, size, "%x", 26); 
    strcpy(field, hex);
    calculate_checksum(&data);
    write();

    //Test max value
    memset(field, 1, size-1);
    field[size] = "\0";
    calculate_checksum(&data);
    write();
    

}

void checksum()
{
    strcpy(data.name, "000");
    write();
}


void mtime()
{
    char time[12];
    long int timestamps[] = {-2200883485000, -1, 0, 500000, 253378870115};
    size_t len = sizeof timestamps / sizeof (long int);
    for (size_t i = 0; i < len; i++)
    {
        snprintf(time, 12, "%o", timestamps[i]); 
        strcpy(data.mtime, time);
        calculate_checksum(&data);
        write();
    }
    
    //reset to correct value
    init_tar_header(&data, "example.txt", "0644");
}



void size()
{
    //TODO improve (play with size)
    test_field(data.size, sizeof data.size);
    char size[12];
    int sizes[] = {0, 500, 99999};
    size_t len = sizeof sizes / sizeof (int);
    for (size_t i = 0; i < len; i++)
    {
        snprintf(size, 12, "%o", sizes[i]); 
        strcpy(data.size, size);
        calculate_checksum(&data);
        write();
    }

    //reset to correct value
    init_tar_header(&data, "example.txt", "0644");
}

void typeflag()
{
    //test special chars
    char chars[] = {'\n','\r','\x7F','\x1F', 'Ƿ', 'چ','\0'}; 

    size_t len = sizeof chars / sizeof (char);
    for (size_t i = 0; i < len; i++)
    {
        data.typeflag = chars[i];
        calculate_checksum(&data);
        write();
    }

    //test everything (The "real" values used in typeflag field are also tested)
    for (int i = 0; i < 256; i++)
    {
        data.typeflag = (char) i;
        calculate_checksum(&data);
        write();
    }
    
    //reset to correct value
    init_tar_header(&data, "example.txt", "0644");
}

void mode()
{
    //all modes in the tar def
    int modes[] = {TSUID, TSGID, TSVTX, TUREAD, TUWRITE, TUEXEC, TGREAD, TGWRITE, TGEXEC, TOREAD, TOWRITE, TOEXEC};

    char Mode[8];
    for (int i = 0; i < 12; i++)
    {
        snprintf(Mode, 8, "%o", modes[i]); 
        strcpy(data.mode, Mode);
        calculate_checksum(&data);
        write();
    }

    test_field(data.mode, sizeof data.mode);

}



int main(int argc, char const *argv[])
{
    if (argc < 2)
        return -1;
    strncpy(EXTRACTOR, argv[1], 25);

    SUCCESS = 0;
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking name : \n");
    test_field(data.name, sizeof data.name);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking linkname : \n");
    test_field(data.linkname, sizeof data.linkname);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking mode : \n");
    mode();
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking magic : \n");
    test_field(data.magic, sizeof data.magic);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking uid : \n");
    test_field(data.uid, sizeof data.uid);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking gid : \n");
    test_field(data.gid, sizeof data.gid);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking mtime : \n");
    mtime();
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking uname : \n");
    test_field(data.uname, sizeof data.gname);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking gname : \n");
    test_field(data.gname, sizeof data.name);
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking checksum : \n");
    checksum();
    init_tar_header(&data, "example.txt", "0644");

    printf("Checking size : \n");
    size();
    init_tar_header(&data, "example.txt", "0644");
  
    printf(">>> Checking padding : \n");
    padding();

    system("rm -f test*");
    system("rm -f archive*");

    return 0;
}

//TODO 
//     end bytes
