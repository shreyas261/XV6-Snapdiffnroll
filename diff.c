#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUF_SIZE 512
#define LINE_SIZE 128

// Function to read a line from a file
int readline(int fd, char *line, int max) {
    int i = 0;
    char c;
    while (i < max - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) {
            break; // End of file or error
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
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf(2, "Usage: diff file1 file2\n");
        exit();
    }

    int fd1 = open(argv[1], O_RDONLY);
    int fd2 = open(argv[2], O_RDONLY);

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
            // Both files ended, they are identical if no differences were found
            break;
        } else if (len1 == 0) {
            // file1 ended, but file2 has more lines
            printf(1, "File %s has extra lines starting at line %d\n", argv[2], line_num);
            break;
        } else if (len2 == 0) {
            // file2 ended, but file1 has more lines
            printf(1, "File %s has extra lines starting at line %d\n", argv[1], line_num);
            break;
        }

        // Compare lines
        if (strcmp(line1, line2) != 0) {
            printf(1, "Line %d differs:\n< %s\n> %s\n", line_num, line1, line2);
        }

        line_num++;
    }

    close(fd1);
    close(fd2);
    exit();
}
