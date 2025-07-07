// C Libraries
#include <stdio.h>
#include <string.h>
#include <zip.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

int read_mmrs(char in_filename[], char dir[])
{

    char mmrs_path[strlen(in_filename) + 1 + strlen(dir)];
    sprintf(mmrs_path, "%s/%s", dir, in_filename);

    zip_t *mmrs_archive;
    zip_file_t *file;
    int err;

    mmrs_archive = zip_open(mmrs_path, 0, &err);

    // if error, return error
    if (mmrs_archive == NULL) 
    {
        return err;
    }
    // otherwise, extract the files to their respective folders
    else
    {
        char title[strlen(in_filename)];
        strcpy(title, in_filename);
        title[strlen(title) - 5] = '\0';
        
        char song_path[8 + strlen(title)];
        sprintf(song_path, "mus_tmp/%s", title);

        mkdir(song_path, 0777);

        for (int i = 0; i < zip_get_num_entries(mmrs_archive, 0); i++)
        {
            file = zip_fopen_index(mmrs_archive, i, 0);
            zip_stat_t sb;
            zip_stat_index(mmrs_archive, i, 0, &sb);
            char file_contents[sb.size];
            zip_fread(file, file_contents, sb.size);
            
            FILE *out_file;
            char out_filename[256];
            sprintf(out_filename, "%s/%s",song_path, sb.name);

            // Write audiobank name
            if (sb.name[strlen(sb.name) - 1] == 'q' && sb.name[strlen(sb.name) - 2] == 'e' && sb.name[strlen(sb.name) - 3] == 's' && sb.name[strlen(sb.name) - 4] == 'z' && sb.name[strlen(sb.name) - 5] == '.')
            {
                puts("");
                
            }
            else
            {

            }
        }

        return 0;
    }
}

int read_seq_directory() 
{
    char cwd[256];

    DIR *dir;
    struct dirent *file;
    char dir_path[] = "mmrs";
    dir = opendir(dir_path);

    mkdir("mus_tmp", 0777);

    int status = 0;

    if(dir) 
    {
        while((file = readdir(dir)) != NULL)
        {
            char *dot = strrchr(file->d_name, '.');
            // If directory entry is "." or "..", print that to the console and continue
            if(file->d_name[0] == '.')
            {
                // puts("Skipping /. and /.. (I hate C)");
            }
            // If file has an extension other than .mmrs, print that to the console and continue
            else if (dot && strcmp(dot, ".mmrs"))
            {
                printf("File %s is not a .mmrs file!\n", file->d_name);
            }
            // Otherwise, read it as an mmrs
            else
            {
                int mmrs_read_status = read_mmrs(file->d_name, dir_path);

                if (mmrs_read_status != 0)
                {
                    zip_error_t error;
                    zip_error_init_with_code(&error, mmrs_read_status);
                    fprintf(stderr, "Cannot open zip archive '%s': %s\n",
                        file->d_name, zip_error_strerror(&error));
                    zip_error_fini(&error);
                }
                else
                {
                    printf("Successfully read file %s!\n", file->d_name);
                }
            }
        }
    }
    else
    {
        status = -1;
    }

    rmdir("mus_tmp");
    return status;
}

int main()
{
    int result = read_seq_directory();

    return 0;
}