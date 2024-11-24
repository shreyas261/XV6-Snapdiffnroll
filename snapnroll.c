#include "types.h"
#include "fcntl.h"
#include "fs.h"
#include "stat.h"
#include "user.h"
#include "stdarg.h"

//Preprocessing_Directives 
#define SNAP         "snapshots"
#define LOG          "log.txt"
#define LOGLOCATION  "snapshots/log.txt"
#define MAX_VERSIONS  3
#define WORKING      "working\0"
#define MAX_FILE_NUMS 5
#define BUF_SIZE 512
#define LINE_SIZE 128

// Structure to store file information
struct file_info {
    char name[DIRSIZ]; 
    uint size;
    short type;
};

//Snapshot,RollBack and Diff Functions
int sshot( );
int rback(int );
int diff(int );

//Logging Functions
int read_log(char *,char * );
void write_log(char *,char *);
int create_log( );
int get_log( );
int numLogs( );

//Helper Functions
int isAvailable(char * ,char* );
char* strcat(char * ,char * ); 
int strlength(char *);
void printHelp();
int snprintf(char * , int , const char * , ...);
static void itoa(int , char * , int );

//Functions for removing whole Directory
int remove_recursive(char * );
int tryremove(int ,char * argv[] );


//Functions to copy one Directory  to another
void copy(char * , char * );
void copy_files_with_extension(char * , int , char * );
void copy_recursive(char * , char * );
int trycopy(int ,char *argv[] );

//Function to Extract Directory Information in an Array
int get_dir_contents(char * , struct file_info * ,int );
void traverseDir(char * );
int compareFILES(char * ,char * ); 
int readline(int , char *, int );
void dir_diff(char * , char * );
int trydiff(int ,char *argv[] );

//Main
int main(int argc,char * argv[]){
    if(argc == 2){
        if(strcmp(argv[1],"snapshot") == 0){
            sshot();
        }
        else if(strcmp(argv[1],"rollback") == 0){
            rback(-1);
        }
        else if(strcmp(argv[1],"diff") == 0){
            diff(-1);
            exit();
        }
        else{
            printHelp();
        }
        exit();
    }

    if(argc == 3){
        if(argv[2][0] < '0' || argv[2][1] > '9'){
            printHelp();
            exit();
        }
        if(strcmp(argv[1],"rollback") == 0){
            rback(atoi(argv[2]));
        }
        if(strcmp(argv[1],"diff") == 0){
            diff(atoi(argv[2]));
        }
        exit(); 
    }

    printHelp();
    exit();
}

//Function to create Snapshot
int sshot(){

    int ret = isAvailable(".",SNAP);
    
    if(ret == -1){
        printf(1,"snapshot taking failed No space\n");
        return 1;
    }
    if(ret == 0){
        if(mkdir(SNAP) < 0){
            printf(1,"some issue\n");
            return 1;
        }
    }
    int num = create_log();
    if(num < 0){
        printf(1,"snapshot taking failed\n");
        return 1;
    }
    char buf[20] = "snapshots/working";
    buf[17] = num + '0';
    buf[18] = '\0';
    char * arg[] = {
        "",
        "-R",
        WORKING,
        buf,
    };
    if(isAvailable(SNAP,&buf[10]) == 1){
        if(num == 0){
            printf(1,"Snapshots Full.. Resetting version to 0\n");
        }
        char * rem[] = {"","-R",buf};
        tryremove(3,rem);
    }
    trycopy(4,arg);
    return 0;
}

//Function To Rollback
int rback(int ver){
    int ret = isAvailable(".",SNAP);
    if(ret == -1){
        printf(1,"Rollback failed\n");
        return 1;
    }

    if(ret == 0){
        printf(1,"No Snapshot Version Found\n");
        return 1;
    }

    if(ver > MAX_VERSIONS || ver < -1){
        printf(1,"Invlaid Version\n");
        return 1;
    }
    if(ver > numLogs()){
        printf(1,"Invalid Version,Either Version Does Not exist or Log is corrupted.\n");
    }

    int fd = open(WORKING,O_RDONLY);
    if(fd < 0){
        mkdir(WORKING);
    }
    close(fd);
    int log_no = get_log();
    if(log_no < 0){
        printf(1,"No Versions Found\n");
        return 1;
    }
    if(ver > - 1){
        log_no = ver;
    }
    char * arg[] = {
        "",
        "-R",
        WORKING,
    };
    char buf[20] ="snapshots/working";
    buf[17] = log_no + '0';
    buf[18] = '\0';
    tryremove(3,arg);
    
    char * cpy[] = {
        "",
        "-R",
        buf,
        WORKING,
    };

    trycopy(4,cpy);
    
    return log_no;

}

//Function For Difference
int diff(int ver){
    if(ver == -1){
        ver = get_log();
        if(ver == -1){
            printf(1,"No Log to Diff.\n");
            return 0;
        }
    }
    if(ver > MAX_VERSIONS || ver < -1){
        printf(1,"Invlaid Version\n");
        return 0;
    }
    if(ver > numLogs()){
        printf(1,"Invalid Version,Either Version Does Not exist or Log is corrupted.\n");
        return 0;
    }
    char buf[20] ="snapshots/working";
    buf[17] = ver + '0';
    buf[18] = '\0';
    char * arg[] = {
        "",
        buf,
        WORKING,
    };
    trydiff(3,arg);
    return 1;
}

/*xxxxxxxxxxxxx LOGGING  FUNCTIONS STARTS HERE xxxxxxxxxxxxx*/
int read_log(char *filename, char * result) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf(1, "Error: Could not open file %s\n", filename);
        return -1;
    }
    int bytes_read = 0;
    char c;
    while((read(fd,&c, sizeof(char))) > 0){
        result[bytes_read] = c;
        bytes_read++;
        if(c == '\0'){
            break;
        }

    }
    bytes_read--;
    close(fd);
    
    if (bytes_read < 0) {
        printf(1, "Error: Could not read file %s or file is empty\n", filename);
        return -1;
    }

    return bytes_read;
}

void write_log(char * filename,char *str) {
    int fd = open(filename,O_CREATE|O_RDWR);
    if (fd < 0) {
        printf(1, "Error: Could not open or create log.txt\n");
        return;
    }
    
    // Write the string followed by a nextline
    int j = 0;
    while(str[j] != '\0')
        j++;

    str[j++] = '\n';
    str[j] = '\0';
    write(fd,str,j);
    close(fd);
}

int create_log(){
    int ret = isAvailable(SNAP,LOG);
    int fd = 0;
    if(ret == 0){
        if((fd = open(LOGLOCATION,O_CREATE|O_RDWR)) < 0)
        {
            printf(1,"touch: error where creating %s\n",LOG);
            exit();
        }
        char buf[15] = "working0\n0\0";
        write_log(LOGLOCATION,buf);
        return 0;
    }
    if(ret == -1){
        return -1;
    }
    char * buf = (char *)malloc(256*sizeof(char));
    int num = read_log(LOGLOCATION,buf);
    ret = num;
    num = 0;

    for(int i = 0;i < ret;i++){
        if(buf[i] == '\n'){
            num++;
        }
    }

    num--;
    if(num < MAX_VERSIONS - 1){
        int i = ret - 1;
        ret = buf[i] - '0';
        ret = (ret + 1) % MAX_VERSIONS;
        char * put = "working";
        int j = 0;
        while(put[j] != '\0'){
            buf[i++] = put[j++];
        }
        buf[i++] = ret + '0';
        buf[i++] = '\n';
        buf[i++] = ret + '0';
        buf[i] = '\0';
        write_log(LOGLOCATION,buf);
    }
    else{
        int i = ret - 1;
        ret = buf[i] - '0';
        ret = (ret + 1) % MAX_VERSIONS;
        buf[i] = ret + '0';
        fd = open(LOGLOCATION,O_CREATE|O_RDWR);
        write(fd,buf,i + 2);
        close(fd);
    }
    free(buf);
    return ret;
}

int get_log(){
    int ret = isAvailable(SNAP,LOG);
    if(ret == 0 || ret == -1){
        return -1;
    }
    char * buf = (char *)malloc(512*sizeof(char));
    int i = read_log(LOGLOCATION,buf);   
    ret = buf[i - 1] - '0';
    free(buf);
    return ret;  
}

int numLogs(){
    int ret = isAvailable(SNAP,LOG);
        if(ret == 0 || ret == -1){
        return -1;
    }
    char * buf = (char *)malloc(512*sizeof(char));
    int i = read_log(LOGLOCATION,buf);
    int n = 0;
    for(int j = 0;j < i;j++){
        if(buf[j] == '\n'){
            n++;
        }
    }
    free(buf);
    return n - 1;
}


/*xxxxxxxxxxxxx HELPER  FUNCTIONS STARTS HERE xxxxxxxxxxxxx*/


//Check if the directory is already present or not
int isAvailable(char *path,char * check){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        return -1;
    }

    if(fstat(fd, &st) < 0){
        close(fd);
        return -1;
    }

    switch(st.type){
        case T_FILE:
            if(strcmp(path,check) == 0){
                return 1;
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf(1, "ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;

                if(strcmp(de.name,check) == 0){
                    return 1;
                }

                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    continue;
                }
            }
            break;
        default:
            break;
    }
    close(fd);
    return 0;
}

//Helper Function to concatinate String
char* strcat(char *dest, char *src){
    char *temp = dest;
    while (*dest) ++dest;
    while (*src) *dest++ = *src++;
    *dest = 0;
    return temp;
}

int strlength(char * str){
    int i = 0;
    while(str[i++] != '\0');
    return i;
}


static void itoa(int n, char *buffer, int base) {
    static char digits[] = "0123456789abcdef";
    int i = 0;
    int is_negative = 0;

    // Handle negative numbers for base 10
    if (n < 0 && base == 10) {
        is_negative = 1;
        n = -n;
    }

    // Convert number to string in reverse order
    do {
        buffer[i++] = digits[n % base];
        n /= base;
    } while (n > 0);

    // Add minus sign for negative numbers
    if (is_negative) {
        buffer[i++] = '-';
    }

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start++] = buffer[end];
        buffer[end--] = temp;
    }

    buffer[i] = '\0'; // Null-terminate the string
}

// Simplified snprintf function [Not Written By Me][Taken from kernal/sprintf.c from xv6-riscv]
int snprintf(char *str, int size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int written = 0;
    char *s;
    char num_buffer[32];

    for (; *fmt != '\0'; fmt++) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                int n = va_arg(args, int);
                itoa(n, num_buffer, 10);
                for (s = num_buffer; *s != '\0' && written < size - 1; s++) {
                    str[written++] = *s;
                }
            } else if (*fmt == 'x') {
                int n = va_arg(args, int);
                itoa(n, num_buffer, 16);
                for (s = num_buffer; *s != '\0' && written < size - 1; s++) {
                    str[written++] = *s;
                }
            } else if (*fmt == 's') {
                s = va_arg(args, char *);
                while (*s != '\0' && written < size - 1) {
                    str[written++] = *s++;
                }
            } else {
                // Handle unsupported format specifier by printing it as-is
                if (written < size - 1) {
                    str[written++] = '%';
                }
                if (written < size - 1) {
                    str[written++] = *fmt;
                }
            }
        } else {
            // Copy regular characters
            if (written < size - 1) {
                str[written++] = *fmt;
            }
        }
    }

    // Null-terminate the result
    if (size > 0) {
        str[written] = '\0';
    }

    va_end(args);
    return written;
}

//Print if help is need
void printHelp(){
    printf(1,"Usage[Max Limit : %d] : \na] ./snapnroll snapshot\n",MAX_VERSIONS);
    printf(1,"b] ./snapnroll rollback <version_no> [version_no = {0..%d}]\n",MAX_VERSIONS - 1);
    printf(1,"c] ./snapnroll rollback\n"); 
    printf(1,"d] ./snapnroll diff  [Difference in last snapshot and working Directory]\n");
    printf(1,"e] ./snapnroll diff  <version_no> [version_no = {0..%d}]\n",MAX_VERSIONS - 1);
}



/*xxxxxxxxxxxxx DIRECTORY REMOVAL FUNCTIONS STARTS HERE xxxxxxxxxxxxx*/


//Recursively Empty files in directory then remove directory
int remove_recursive(char *source)
{
    char *buffer;
    buffer = (char*)malloc(512 * sizeof(char));
    int fdSource;
    struct dirent de;
    struct stat st;

    if (source[strlen(source) - 1] == '/') source[strlen(source) - 1] = 0;
    if ((fdSource = open(source, 0)) < 0)
    {
        printf(2, "cp: cannot open '%s' No such file or directory\n", source);
        exit();
    }
    if (fstat(fdSource, &st) < 0)
    {
        printf(2, "cp: cannot stat '%s' No such file or directory\n", source);
        exit();
    }

    char *tempSource;
    tempSource = (char*)malloc(512 * sizeof(char));
    switch (st.type)
    {
        case T_FILE:
        {
            return unlink(source);
        }
        case T_DIR:
        {
            strcat(buffer, source);
            strcat(buffer, "/");

            while (read(fdSource, &de, sizeof(de)) == sizeof(de))
            {
                if (de.inum == 0 || de.name[0] == '.') 
                    continue;
                strcpy(tempSource, source);
                strcat(tempSource, "/");
                strcat(tempSource, de.name);
                remove_recursive(tempSource);
            }
            break;
        }
        default : 
            break;
    }
    close(fdSource);
    free(tempSource);
    free(buffer);
    return unlink(source);
}

//Try to Remove the Directory [May Fail unlink]
int tryremove(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(2, "Usage : [OPTIONS] [source]\n");
        printf(2, "Options:\n");
        printf(2, "  -R : Remove recursively\n");
    }
    if (argv[1][0] == '*')
    {
        
        return unlink(argv[1]);
        
    }
    else if (strcmp(argv[1], "-R") == 0)
    {
        return remove_recursive(argv[2]);
    }
    else
    {
        return unlink(argv[1]);
    }
    return -1;
}	

/*xxxxxxxxxxxxx DIRECTORY COPY FUNCTIONS STARTS HERE xxxxxxxxxxxxx*/


//Copy one File from Source to Destination
void copy(char *source, char *destination) 
{
    struct stat st;
    char *buf;
    buf = (char*)malloc(512 * sizeof(char));
    int fdSource;

    // Open source file
    if ((fdSource = open(source, O_RDONLY)) < 0)
    {
        printf(2, "cp: cannot open '%s' No such file or directory\n", source);
        exit();
    }

    // If source is a directory
    if (fstat(fdSource, &st) >= 0)
    {
        if (st.type == T_DIR)
        {
            printf(2, "cp: cannot copy directory '%s'\n", source);
            exit();
        }
    }

    int fdDestination;
    char *temp;
    temp = (char*)malloc(512 * sizeof(char));
    if (destination[strlen(destination) - 1] == '/') destination[strlen(destination) - 1] = 0;

    // Open destination file
    fdDestination = open(destination, 0);
    if (1)
    {
        // If destination is a directory
        if (fstat(fdDestination, &st) >= 0 && st.type == T_DIR)
        {
            mkdir(destination);
            strcat(temp, destination);
            strcat(temp, "/");
            strcat(temp, source);
            close(fdDestination);
            if ((fdDestination = open(temp, O_CREATE | O_RDWR)) < 0)
            {
                printf(2, "cp: error while creating '%s'\n", temp);
                exit();
            }
        }
        else
        {
            close(fdDestination);
            if ((fdDestination = open(destination, O_CREATE | O_RDWR)) < 0)
            {
                printf(2, "cp: error while creating '%s'\n", destination);
                exit();
            }
        }
    }
    int n;
    while ((n = read(fdSource, buf, sizeof(buf))) > 0)
    {
        printf(fdDestination, "%s", buf);
    }
    close(fdDestination);
    free(temp);
    free(buf);
}

//For Files Having Extensions
void copy_files_with_extension(char *path, int extLength, char *extension)
{
    char *buffer;
    buffer = (char*)malloc(512 * sizeof(char));
    int fdCurrentDir, fdPath;
    struct dirent de;
    struct stat st;

    if (path[strlen(path) - 1] == '/') path[strlen(path) - 1] = 0;
    if ((fdCurrentDir = open(".", 0)) < 0)
    {
        printf(2, "cp: cannot open '\".\"' No such file or directory\n");
        exit();
    }

    if ((fdPath = open(path, O_RDONLY)) < 0)
    {
        printf(2, "cp: cannot open '%s' No such file or directory\n", path);
        exit();
    }
    if (fstat(fdPath, &st) < 0)
    {
        printf(2, "cp: cannot stat '%s' No such file or directory\n", path);
        exit();
    }
    else
    {
        if (st.type != T_DIR)
        {
            printf(2, "cp: '%s' is not a directory\n", path);
            exit();
        }
    }

    strcat(buffer, path);
    strcat(buffer, "/");
    int len = strlen(buffer);

    while (read(fdCurrentDir, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0) 
            continue;
        if (de.name[0] == '.')
            continue;
        if (stat(de.name, &st) >= 0 && st.type == T_DIR) continue;

        memmove(buffer + len, de.name, strlen(de.name));
        if (strcmp(de.name + (strlen(de.name) - extLength + 1), extension) == 0) 
            copy(de.name, buffer);
        memset(buffer + len, '\0', sizeof(buffer) + len);
    }
    free(buffer);
    close(fdCurrentDir);
}

//Recursively Traverse Directory to copy Directory Structure and Then File
void copy_recursive(char *source, char *destination)
{
    char *buffer;
    buffer = (char*)malloc(512 * sizeof(char));
    int fdSource;
    struct dirent de;
    struct stat st;

    if (source[strlen(source) - 1] == '/') source[strlen(source) - 1] = 0;
    if (destination[strlen(destination) - 1] == '/') destination[strlen(destination) - 1] = 0;
    
    if ((fdSource = open(source, 0)) < 0)
    {
        printf(2, "cp: cannot open '%s' No such file or directory\n", source);
        exit();
    }
    if (fstat(fdSource, &st) < 0)
    {
        printf(2, "cp: cannot stat '%s' No such file or directory\n", source);
        exit();
    }

    char *tempSource, *tempDest;
    tempSource = (char*)malloc(512 * sizeof(char));
    tempDest = (char*)malloc(512 * sizeof(char));
    switch (st.type)
    {
        case T_FILE:
        {
            copy(source, destination);
            break;
        }
        case T_DIR:
        {
            strcpy(buffer, destination);
            strcat(buffer, "/");
            strcat(buffer, source);
            if (mkdir(destination) >= 0)
            {
                while (read(fdSource, &de, sizeof(de)) == sizeof(de))
                {
                    if (de.inum == 0 || de.name[0] == '.') 
                        continue;
                    strcpy(tempSource, source);
                    strcat(tempSource, "/");
                    strcat(tempSource, de.name);
                    strcpy(tempDest, destination);
                    strcat(tempDest, "/");
                    strcat(tempDest, de.name);
                    copy_recursive(tempSource, tempDest);
                }
            }
            else
            {
                while (read(fdSource, &de, sizeof(de)) == sizeof(de))
                {
                    if (de.inum == 0 || de.name[0] == '.') 
                        continue;
                    strcpy(tempSource, source);
                    strcat(tempSource, "/");
                    strcat(tempSource, de.name);
                    strcpy(tempDest, buffer);
                    strcat(tempDest, "/");
                    strcat(tempDest, de.name);
                    copy_recursive(tempSource, tempDest);
                }
            }
            break;
        }
        close(fdSource);
    }
    free(tempSource);
    free(tempDest);
    free(buffer);
}

//Try To copy Source file to destination [May Fail because of SEGSERV or Disc Size is FULL]
int trycopy(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(2, "Usage : cp [OPTIONS] [source] [destination]\n");
        printf(2, "Options:\n");
        printf(2, "  -R : copy files recursively\n");
    }
    if (argv[1][0] == '*')
    {
        int extLength = strlen(argv[1]);
        char ext[512];
        strcpy(ext, argv[1] + 1);
        copy_files_with_extension(argv[2], extLength, ext);
        exit();
    }
    else if (strcmp(argv[1], "-R") == 0)
    {
        char *temp;
        temp = (char*)malloc(512 * sizeof(char));
        strcat(temp, argv[3]);
        strcat(temp, "/");
        strcat(temp, argv[2]);
        mkdir(temp);
        copy_recursive(argv[2], argv[3]);
        free(temp);
        exit();
    }
    else
    {
        copy(argv[1], argv[2]);
        exit();
    }
    exit();
}	

//Get Contents of a directory
int get_dir_contents(char *path, struct file_info *files, int max_files) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    int file_count = 0;

    // Open the directory
    if((fd = open(path, 0)) < 0) {
        printf(2, "get_dir_contents: cannot open %s\n", path);
        return -1;
    }

    // Get directory's stat information
    if(fstat(fd, &st) < 0) {
        printf(2, "get_dir_contents: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    // Verify it's a directory
    if(st.type != T_DIR) {
        printf(2, "get_dir_contents: %s is not a directory\n", path);
        close(fd);
        return -1;
    }

    // Read directory entries
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
        // Directory entry not in use
        if(de.inum == 0)
            continue;

        // Skip "." and ".." entries
        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        // Create full path
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        strcpy(p, de.name);

        // Get file's stat information
        if(stat(buf, &st) < 0) {
            printf(2, "get_dir_contents: cannot stat %s\n", buf);
            continue;
        }

        // Check if we have space in our array
        if(file_count >= max_files) {
            printf(2, "get_dir_contents: too many files, stopping at %d\n", max_files);
            break;
        }

        // Store file information
        int i = 0;
        for(i = 0;de.name[i] != '\0';i++){
            files[file_count].name[i] = de.name[i];
        }
        files[file_count].name[i] = '\0';
        files[file_count].size = st.size;
        files[file_count].type = st.type;
        file_count++;
    }

    close(fd);
    return file_count;
}

void traverseDir(char * source){
    struct file_info * file1 = (struct file_info *)malloc(MAX_FILE_NUMS*sizeof(struct file_info));
    int j = get_dir_contents(source,file1,MAX_FILE_NUMS);
    printf(1,"Name    : Size in Bytes  : Type\n");
    for(int i = 0;i < j;i++){
        if(file1[i].type == (short)2){
            printf(1,"%s  : %d : File\n",file1[i].name,file1[i].size);
        }
        else{
            printf(1,"%s  : %d : Directory\n",file1[i].name,file1[i].size);
        }
    }
    free(file1);
}

//XXXXXXXXXXXXXXXXXXXXXXXX FUNCTIONS FOR DIFF XXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Function to read a line from a file
int readline(int fd, char *line, int max) {
    int i = 0;
    char c;
    while (i < max - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) {
            break; 
        }
        if (c == '\n') {
            line[i] = '\0';
            return i + 1;
        }
        line[i++] = c;
    }
    line[i] = '\0';
    return i;
}

// Simple diff command to compare two files line by line
int compareFILES(char * file1 ,char * file2) {

    int fd1 = open(file1, O_RDONLY);
    int fd2 = open(file2, O_RDONLY);

    if (fd1 < 0 || fd2 < 0) {
        printf(2, "diff: Cannot open one of the files.\n");
        exit();
    }

    char line1[LINE_SIZE], line2[LINE_SIZE];
    int line_num = 1;
    int len1, len2;

    while (1) {
        len1 = readline(fd1, line1, LINE_SIZE);
        len2 = readline(fd2, line2, LINE_SIZE);
        if (len1 == 0 && len2 == 0) {
            break;
        } else if (len1 == 0) {
            printf(1, "File %s has extra lines starting at line %d\n", file2, line_num);
            break;
        } else if (len2 == 0) {

            printf(1, "File %s has extra lines starting at line %d\n", file1, line_num);
            break;
        }

        if (strcmp(line1, line2) != 0) {
            printf(1, "Line %d differs:\n< %s\n> %s\n", line_num, line1, line2);
        }

        line_num++;
    }

    close(fd1);
    close(fd2);
    exit();
}

void dir_diff(char *dir1, char *dir2) {
    int fd1 = open(dir1, O_RDONLY);
    int fd2 = open(dir2, O_RDONLY);
    if (fd1 < 0 || fd2 < 0) {
        printf(2, "Error opening directories %s or %s\n", dir1, dir2);
        return;
    }

    struct dirent de1, de2;
    struct stat st1, st2;
    char name[] = "snapshot ";
    name[8] = dir2[17];

    // Iterate over entries in the first directory
    while (read(fd1, &de1, sizeof(de1)) == sizeof(de1)) {
        if (de1.inum == 0) continue;
        
        if(strcmp(de1.name,".") == 0 || strcmp(de1.name,"..") == 0)
            continue;

        char path1[128], path2[128];
        snprintf(path1, sizeof(path1), "%s/%s", dir1, de1.name);
        snprintf(path2, sizeof(path2), "%s/%s", dir2, de1.name);

        if (stat(path1, &st1) < 0) continue;
        
        if (stat(path2, &st2) < 0) {
            printf(1, "File %s exists in %s but not in %s\n", de1.name, dir1, name);
            continue;
        }

        // Compare file sizes
        if (st1.size != st2.size) {
            printf(1, "File %s differs in size: %d bytes in %s, %d bytes in %s\n",
                   de1.name, st1.size, dir1, st2.size, name);
        } 
        if (st1.type == T_FILE && st2.type == T_FILE) {
            // Compare file contents if sizes match
            int diff_result = compareFILES(path1, path2);
            if (diff_result == 0) {
                printf(1, "Files %s/%s and %s/%s are identical\n", dir1, de1.name, name, de1.name);
            }
        }
    }

    // Check for files in dir2 not in dir1
    while (read(fd2, &de2, sizeof(de2)) == sizeof(de2)) {
        if (de2.inum == 0) continue;
        
        if(strcmp(de2.name,".") == 0 || strcmp(de2.name,"..") == 0)
            continue;

        char path2[128], path1[128];
        snprintf(path2, sizeof(path2), "%s/%s", name, de2.name);
        snprintf(path1, sizeof(path1), "%s/%s", dir1, de2.name);

        if (stat(path1, &st1) < 0 && stat(path2, &st2) >= 0) {
            printf(1, "File %s exists in %s but not in %s\n", de2.name, dir2, dir1);
        }
    }

    close(fd1);
    close(fd2);
}

int trydiff(int argc,char * argv[]){
    if (argc != 3) {
        exit();
    }

    struct stat st1, st2;
    if (stat(argv[1], &st1) < 0 || stat(argv[2], &st2) < 0) {
        printf(2, "Error accessing directories %s or %s\n", argv[1], argv[2]);
        exit();
    }

    if (st1.type == T_DIR && st2.type == T_DIR) {
        dir_diff(argv[1], argv[2]);
    } else {
        printf(2, "Both arguments must be directories\n");
    }
    exit();
}