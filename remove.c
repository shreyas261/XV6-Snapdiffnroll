int 
remove_recursive(char *source)
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

int tryremove(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(2, "Usage : cp [OPTIONS] [source] [destination]\n");
        printf(2, "Options:\n");
        printf(2, "  -R : copy files recursively\n");
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
