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

// TODO: Make the header 512 bytes large and compatible with booting disks.

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

int main(int argc, char **argv)
{

    if(argc < 3)
    {
        printf("Usage: %s image file1 file2 ...\n", argv[0]);
        return EXIT_SUCCESS;
    }

    // Pointer to image file
    FILE *image_fp;

    // Open the file
    image_fp = fopen(argv[1], "wb");
    
    // Check the return value of the function to make sure that the file pointer is valid.
    if(!image_fp)
    {
        fprintf(stderr, "Could not open the image file.\n");

        return EXIT_FAILURE;
    }

    // Create the image header block.

    char *signature = "JRFS";
    char *name = "JurkurOS RamFS";

    header_t header;

    strncpy(header.signature, signature, strlen(signature));
    strcpy(header.name, name);
    
    header.major_version = 0;
    header.minor_version = 1;

    header.n_files = argc - 2;

    // Write the header to the file.

    fwrite(&header, sizeof(header_t), 1, image_fp);

    // Open the files and place them in the image.
    
    unsigned int current_offset = sizeof(header_t) + header.n_files * sizeof(file_entry_t);

    // Allocate memory for the file entries

    file_entry_t *file_entries = malloc(sizeof(file_entry_t) * header.n_files);

    // Fill out all file entries
    
    for(unsigned int i = 0; i < header.n_files; ++i)
    {

        char *file_name = argv[i + 2];
        FILE *file;
        unsigned int length = 0;

        file = fopen(file_name, "rb");

        if(!file)
        {
            fprintf(stderr, "Error opening file %s\n", file_name);
            exit(EXIT_FAILURE);
        }

        // Extract the length of the file

        fseek(file, 0L, SEEK_END);
        length = ftell(file);
        fseek(file, 0L, SEEK_SET);

        strcpy(file_entries[i].name, file_name);
        file_entries[i].size = length;
        file_entries[i].offset = current_offset;      

        printf("Wrote %s (%i bytes) at %#x\n", file_name, length, current_offset);

        // Update the file offset with the length of the current file.

        current_offset += length;  

        fclose(file);

    }

    // Write the file headers to file

    fwrite(file_entries, sizeof(file_entry_t), header.n_files, image_fp);

    for(unsigned int i = 0; i < header.n_files; ++i)
    {
        FILE *file;
        unsigned int length = 0;
        void *buffer;

        // Open the file again

        file = fopen(file_entries[i].name, "rb");

        length = file_entries[i].size;

        // Allocate a buffer with the size of the file
        
        buffer = malloc(length);

        // Read the file to the buffer

        fread(buffer, 1, length, file);

        // Write the buffer to the image

        fwrite(buffer, 1, length, image_fp);
        
        // Remember to free the buffer and close the file

        free(buffer);

        fclose(file);
    }

    free(file_entries);

    // Close the file.
    fclose(image_fp);

    return EXIT_SUCCESS;
}


/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
