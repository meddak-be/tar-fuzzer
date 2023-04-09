#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct tar_t data;


// Function to write a tar file
void write_tar_file(tar_header *header, const char *content, size_t content_length, const char *end_content, size_t end_length, int should_update_checksum) {
    FILE *archive;

    if (should_update_checksum) {
        compute_checksum(header);
    }

    archive = fopen(TAR_ARCHIVE_NAME, "wb");
    fwrite(header, sizeof(tar_header), 1, archive);
    fwrite(content, content_length, 1, archive);
    fwrite(end_content, end_length, 1, archive);
    fclose(archive);
}

// Function to create an empty tar file
void create_empty_tar_file(tar_header *header, int should_update_checksum) {
    char zero_bytes[END_BYTES_COUNT] = {0};
    write_tar_file(header, "", 0, zero_bytes, END_BYTES_COUNT, should_update_checksum);
}

// Function to set up a simple tar header
void setup_simple_header(tar_header *header) {
    memset(header, 0, sizeof(tar_header));

    char file_name[100];
    snprintf(file_name, 7, "%d.txt", (rand() % 100));

    strcpy(header->name, file_name);
    strcpy(header->mode, FULL_PERMISSIONS);
    strcpy(header->uid, STANDARD_UID);
    strcpy(header->gid, STANDARD_GID);
    set_size(header, 0);
    strcpy(header->mtime, "14220157140");
    header->typeflag = REGTYPE;
    header->linkname[0] = 0;
    strcpy(header->magic, TMAGIC);
    strcpy(header->version, TVERSION);

    strcpy(header->uname, "root");
    strcpy(header->gname, "root");
    strcpy(header->devmajor, "0000000");
    strcpy(header->devminor, "0000000");

    compute_checksum(header);
}

// Tar header utility functions
void set_size(tar_header *header, int size) {
    snprintf(header->size, 12, "%011o", size);
}

void modify_header_field(char *field, size_t field_length, const char *new_value) {
    strncpy(field, new_value, field_length);
}

unsigned int compute_checksum(tar_header *entry) {
    memset(entry->chksum, ' ', 8);

    unsigned int checksum_value = 0;
    unsigned char *byte_data = (unsigned char *)entry;
    for (int i = 0; i < 512; i++) {
        checksum_value += byte_data[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", checksum_value);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return checksum_value;
}

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

void end_bytes()
{

}



void test_field(char *field)
{
    //Test for weird ascii and non ascii
    size_t size = sizeof field;
    char names[][105] = {"Ƿrojetѭچ", "test\n", "test\t", "test\a", "test\r", "\0", "      ","",
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
    // Store the original value of the field to restore later
    char original_value[100];
    strcpy(original_value, field);

    size_t len = sizeof names / sizeof (char[105]);
    for (size_t i = 0; i < len; i++)
    {
        strcpy(field, names[i]);
        calculate_checksum(&data);
        write();
    }



    strcpy(field, "9");
    calculate_checksum(&data);
    write();

    char hex[size];
    snprintf(hex, size, "%x", 26); 
    strcpy(field, hex);
    calculate_checksum(&data);
    write();



    // Restore the original value of the field
    strcpy(field, original_value);
    calculate_checksum(&data);
    write();

}

// void name()
// {
//     test_field(data.name);

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
    test_field(data.name);
    test_field(data.linkname);
    printf("size : \n");
    test_field(data.mtime);
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
    printf("Checking name : \n");
    name();

    // size();

    // uidgid();
    // typeflag();

    // printf("%s, %s, %s, %s, %s\n", data.name, data.mode, data.uid, data.gid,data.size);
    // printf("%s, %s, %s\n", data.version, data.uname, data.gname);
    // printf("%s, %s, %c, %s, %s\n", data.mtime,data.chksum, data.typeflag, data.linkname, data.magic);

    return 0;
}
