#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct tar_t data;

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

int check_crash(char* extractor)
{
    int rv = 0;
    char cmd[72];
    strncpy(cmd, "./extractor_x86_64 archive2.tar", 70);
    cmd[71] = '\0';
    //strncat(cmd, " archive2.tar", 25);
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

    if ((fp = fopen("archive2.tar","wb")) == NULL)
    {
        printf("Error! opening file");
        exit(1);
    }
    fwrite(&data, sizeof(struct tar_t), 1, fp);
    
    fclose(fp);
    //save data as SUCCESS tar.
    int success = check_crash("./extractor");
}


void test_field(char *field, char payloads[][100], size_t len)
{
    // Store the original value of the field to restore later
    char original_value[100];
    strcpy(original_value, field);

    for (size_t i = 0; i < len; i++)
    {
        strcpy(field, payloads[i]);
        calculate_checksum(&data);
        write();
    }

    // Restore the original value of the field
    strcpy(field, original_value);
    calculate_checksum(&data);
    write();
}

// void name()
// {
//     char name_payloads[][100] = {
//         "a\x00\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
//     };
//     size_t len_name_payloads = sizeof name_payloads / sizeof (char[100]);
//     test_field(data.name, name_payloads, len_name_payloads);

// }

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
    strcpy(data.mtime, "time");
    calculate_checksum(&data);
    write();
    
    //reset to correct value
    snprintf(time, 12, "%o", 1239291); 
    strcpy(data.mtime, time);
    calculate_checksum(&data);
}

void name()
{   //"Ƿrojetѭچ", "test\n", "test\t", "test\a", "test\r", "\0", "      ","", 
    char names[][105] = {"Ƿrojetѭچ", "test\n", "test\t", "test\a", "test\r", "\0", "      ","",
    "test\x00name",
    "test\x80name",
    "test\\",
    "test/",
    "test\x1B[31m", // ANSI escape sequence
    "test\x7F",     // DEL character
    "test\x1A",     // SUB character
    "test\x1C",     // FS character
    "test\x1D",     // GS character
    "test\x1E",     // RS character
    "test\x1F",      // US character 
    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    '\0'
    };
    size_t len = sizeof names / sizeof (char[100]);

    for (size_t i = 0; i < len; i++)
    {
        printf("trying %s\n", names[i]);
        strcpy(data.name, names[i]);
        calculate_checksum(&data);
        write();
    }

    char namereset[] = "normalName";
    strcpy(data.name, namereset);
    calculate_checksum(&data);

    for (size_t i = 0; i < len; i++)
    {
        strcpy(data.linkname, names[i]);
        calculate_checksum(&data);
        write();
    }

    char lnamereset[] = "\0";
    strcpy(data.linkname, lnamereset);
    calculate_checksum(&data);
}

void uidgid()
{
    char uidB[8];
    strcpy(uidB, data.uid);
    printf("uid test (%s)\n", uidB);
    char size[12];
    long int sizes[] = {-100001, -1, 0, 500000, 999999};

    size_t len = sizeof sizes / sizeof (long int);

    for (size_t i = 0; i < len; i++)
    {
        snprintf(size, 12, "%o", sizes[i]); 
        //size[0] = *"-";
        printf("trying %s\n", size);
        strcpy(data.uid, size);
        calculate_checksum(&data);
        write();
    }
    printf("trying %s\n", "fakeid");
    strcpy(data.uid, "\0");
    calculate_checksum(&data);
    write();
    
    //reset to correct value
    strcpy(data.uid, uidB);
    calculate_checksum(&data);
    write();
}

void size()
{
    char sizeB[12];
    strcpy(sizeB, data.size);
    printf("size test (%s)\n", sizeB);
    char size[12];
    long int sizes[] = {-00000000001, -1, 0, 500000, 999999999999};
    size_t len = sizeof sizes / sizeof (long int);
    for (size_t i = 0; i < len; i++)
    {
        snprintf(size, 12, "%o", sizes[i]); 
        strcpy(data.size, size);
        calculate_checksum(&data);
        write();
    }
    // strcpy(data.size, "fakeSize");
    // calculate_checksum(&data);
    // write();
    
    //reset to correct value
    strcpy(data.size, sizeB);
    calculate_checksum(&data);
    write();
}

void typeflag()
{
    char uidB;
    uidB= data.typeflag;
    printf("typeflag test (%c)\n", uidB);
    char sizes[] = {" \n\r\x00\x7F\x1F\0"};

    size_t len = sizeof sizes / sizeof (char[1]);
    printf("%d\n", len);
    for (size_t i = 0; i < len; i++)
    {
        printf("trying %c\n", sizes[i]);
        data.typeflag = sizes[i];
        calculate_checksum(&data);
        write();
    }
    
    //reset to correct value
    data.typeflag = uidB;
    calculate_checksum(&data);
    write();
}

void read()
{
    FILE *fp;
    
    if ((fp = fopen("archive2.tar","rb")) == NULL){
        printf("Error! opening file");
        exit(1);
    }
    fread(&data, sizeof(struct tar_t), 1, fp); 
    fclose(fp); 
}

char toChar(int i){
    return (char)(48+i);
}

int main(int argc, char const *argv[])
{
    read();

    // printf("Checking mtime : \n");
    // mtime();
    // printf("Checking name : \n");
    // name();

    // size();

    // uidgid();
    typeflag();

    // printf("%s, %s, %s, %s, %s\n", data.name, data.mode, data.uid, data.gid,data.size);
    // printf("%s, %s, %s\n", data.version, data.uname, data.gname);
    // printf("%s, %s, %c, %s, %s\n", data.mtime,data.chksum, data.typeflag, data.linkname, data.magic);

    return 0;
}
