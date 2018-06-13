/* "'(file-name-nondirectory (buffer-file-name))'" --- 
 * 
 * Filename: "'(file-name-nondirectory (buffer-file-name))'"
 * Description: 
 * Author: Joakim Bertils
 * Maintainer: 
 * Created: "'(current-time-string)'"
 * Version: 
 * Package-Requires: ()
 * Last-Updated: 
 *          
 *     
 * URL: 
 * Doc URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change Log:
 * 
 * 
 */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.
 */

/* Code: */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// File header for the image placed at the beginning of the file.
typedef struct
{

    // Signature to detect the JurkurOS RamFS
    char signature[4];

    // Name of the image
    char name[32];

    // Minor version of the file system
    unsigned char major_version;
    
    // Major version of the file system
    unsigned char minor_version;
    
    // Number of files.
    unsigned short n_files;

} __attribute__((packed)) header_t;

// File entry for each file, placed in an array following the header.
typedef struct 
{
    // Name of the file.
    char name[32];

    // Size of the file.
    unsigned int size;

    // Offset into the image file to the start of the file.
    unsigned int offset;
    
} __attribute__((packed)) file_entry_t;


//==============================================================================
// Device file
//==============================================================================

typedef struct 
{
    FILE *fp;
    
} device_t;

device_t *device_initialize(const char *path)
{

    device_t *device = malloc(sizeof(device_t));

    device->fp = fopen(path, "rb");

    if(!device->fp)
    {
        fprintf(stderr, "Could not open the image file.\n");
        return NULL;
    }

    return device;
}

void device_seek(device_t *device, unsigned int offset)
{
    fseek(device->fp, offset, SEEK_SET);
}

void device_read(device_t *device, void *buffer, unsigned int length)
{
    fread(buffer, 1, length, device->fp);
}

void device_write(device_t *device, void *buffer, unsigned int length)
{
    fwrite(buffer, 1, length, device->fp);
}

void device_close(device_t *device)
{
    fclose(device->fp);
    free(device);
}

int main(int argc, char **argv)
{

    if(argc < 3)
    {
        printf("Usage: %s image file\n", argv[0]);
        return EXIT_SUCCESS;
    }

    device_t *device = device_initialize(argv[1]);

    char *name = argv[2];

    header_t header;

    device_read(device, &header, sizeof(header_t));
    
    // TODO: Verify the signature and extract name of volume

    unsigned int n_files = header.n_files;

    file_entry_t *file_entries = malloc(sizeof(file_entry_t) * n_files);

    device_read(device, file_entries, sizeof(file_entry_t) * n_files);

    unsigned int size = 0;
    unsigned int offset = 0;

    for(unsigned int i = 0; i < n_files; ++i)
    {
        if(strcmp(name, file_entries[i].name) == 0)
        {
            size = file_entries[i].size;
            offset = file_entries[i].offset;

            break;
        }
    }
    
    if(size == 0 || offset == 0)
    {
        fprintf(stderr, "Could not find the specified file.\n");

        device_close(device);
        return EXIT_SUCCESS;
    }

    unsigned char *buffer = malloc(size);
    
    device_seek(device, offset);

    device_read(device, buffer, size);

    free(file_entries);

    device_close(device);
    

    printf("Size: %i\nOffset: %#x\n", size, offset);

    unsigned int counter = 0;

    for(unsigned int i = 0; i < (size / 16 + 1); ++i)
    {
        int remaining = 16;

        if(counter + 16 >= size)
        {
            remaining = size - counter;
        }

        printf("%#08x  ", i * 16);

        for(unsigned int j = 0; j < remaining; ++j)
        {
            
            unsigned char c =  buffer[i * 16 + j];
            
            printf("%02x ", c);

            if(j == 7)
            {
                printf(" ");
            }
        }      

        for(unsigned int j = remaining; j < 16; ++j)
        {

            printf("   ");

            if(j == 7)
            {
                printf(" ");
            }            
        }


        printf(" |");

        for(unsigned int j = 0; j < remaining; ++j)
        {
            
            char c =  buffer[i * 16 + j];

            if(c < 32 || c >= 127)
            {
                c = '.';
            }
            
            printf("%c", c);
        }      

        for(unsigned int j = remaining; j < 16; ++j)
        {

            printf("%c", ' ');
        }

        printf("|");

        printf("\n");

        counter += remaining;
    }

    free(buffer);
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
