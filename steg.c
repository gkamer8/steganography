#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createSecret(unsigned char *message, int messageLength, char *inFilename,
					char *outFilename, int builtInOffset) {

    // Image to read from
    FILE *image = fopen(inFilename, "rb");

    if (image == NULL) {
        printf("Could not open source image %s.\n", inFilename);
        exit(1);
    }

    unsigned char header[14];
    fread(header, sizeof(unsigned char), 14, image);  // read in the header

    // Get image parameters
    int size = *(int*)&header[0x0002];
    int offset = *(int*)&header[0x000A];

    // Store info header and color table
    unsigned char info[offset - 14];
    fread(info, sizeof(unsigned char), offset - 14, image);

    // Store image data
    unsigned char imageData[size - offset];
    fread(imageData, sizeof(unsigned char), size - offset, image);

    fclose(image);

    // Encode message
    for (int i = 0; i < (messageLength * 4); i++) {

		int character = i / 4;
        int bitPairNum = i % 4;

    	unsigned char messageByte;

		switch (bitPairNum) {
	        case 0:
	            messageByte = (message[character] & 0b11000000) >> 6;
	            break;
	        case 1:
	            messageByte = (message[character] & 0b00110000) >> 4;
	            break;
	        case 2:
	            messageByte = (message[character] & 0b00001100) >> 2;
	            break;
	        case 3:
	            messageByte = (message[character] & 0b00000011) >> 0;
	            break;
		}

		int insert = i + (builtInOffset * 4);

		if (insert >= size - offset) {
			printf("Max file size reached. Encoding incomplete.\n");
			exit(1);
		}

		imageData[insert] = (imageData[insert] & 0b11111100) | messageByte;
    }

    // Write out file
    FILE *outImage = fopen(outFilename, "wb");

    fwrite(header, 1, 14, outImage);
    fwrite(info, 1, offset - 14, outImage);
    fwrite(imageData, 1, size - offset, outImage);

    fclose(outImage);

	printf("Encoding successful.\n");

}

void readSecret(int messageLength, char* inFilename) {

    // Read message
    FILE *readImage = fopen(inFilename, "rb");

    if (readImage == NULL) {
        printf("Could not open source image %s.\n", inFilename);
        exit(1);
    }

    // Get offset
    unsigned char headerRead[14];
    fread(headerRead, sizeof(unsigned char), 14, readImage);
    int offsetRead = *(int*)&headerRead[0x000A];

    fseek(readImage, offsetRead - 14, SEEK_CUR);  // Skip over offset

    // Read in image data
    unsigned char mess[messageLength * 4];
    fread(mess, sizeof(unsigned char), messageLength * 4, readImage);

    fclose(readImage);

    unsigned char unencoded[messageLength + 1];  // for storing message

    unsigned char nextChar;  // for storing each character during construction
    for (int i = 0; i < (messageLength * 4); i++) {
        // each character is contructed over four bytes

		int character = i / 4;
        int bitPairNum = i % 4;

        unsigned char lastTwo = mess[i] & 0b00000011;

        if (bitPairNum == 0) {
            nextChar = lastTwo;
        }
        else {
            nextChar = (nextChar << 2) | lastTwo;
        }

        if (bitPairNum == 3) {
			// on last two bits, add to unencoded
            unencoded[character] = nextChar;
        }

    }

	// Add a \0 at the end of the array in case there is none

	unencoded[messageLength] = '\0';

	printf("Secret message: \n%s\n", unencoded);

}

int main(int argc, char const *argv[]) {

    char usageStatement[200];
    sprintf(usageStatement,
        "usage: %s infile [-r length] [-w outfile] [-f outfile textfile]\n",
        argv[0]);

    // Parse arguments
    char *fname;  // For in-filename
    int gotFilename = 0;

    int read = 0;
    int readLength;

    int write = 0;
    char *outname;
    int writeFromFile = 0;
    char *text;  // For optional text file to read from
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            read = 1;
            if (i + 1 <= argc - 1) {  // Check if there are enough args
                readLength = atoi(argv[i + 1]);
            }
            else {
                printf("%s", usageStatement);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-w") == 0) {
            write = 1;
            if (i + 1 <= argc - 1) {  // Check if there are enough args
                outname = malloc(sizeof(argv[i + 1]));
                strcpy(outname, argv[i + 1]);
            }
            else {
                printf("%s", usageStatement);
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i], "-f") == 0) {
          writeFromFile = 1;
          if (i + 2 <= argc - 1) {
              outname = malloc(sizeof(argv[i + 1]));
              strcpy(outname, argv[i + 1]);

              text = malloc(sizeof(argv[i + 2]));
              strcpy(text, argv[i + 2]);
          }
          else {
              printf("%s", usageStatement);
              exit(1);
          }
          i += 2;
        }
        else {
            fname = malloc(sizeof(argv[i]));
            strcpy(fname, argv[i]);
            gotFilename = 1;
        }
    }

    if (gotFilename == 0) {
        printf("%s", usageStatement);
        exit(1);
    }

    // If no option was selected in the command line, ask
    if (read == 0 && write == 0 && writeFromFile == 0) {
        char choice = '_';  // Doesn't work if empty in certain compilers
        printf("Read message (r) or encode message (e)?\n");
        while (choice != 'r' && choice != 'e') {
            choice = getchar();
            getchar();  // To remove extra \n
        }

        if (choice == 'r') {
            read = 1;
            printf("Enter message length: ");
            scanf("%i", &readLength);
        }
        if (choice == 'e')  {
            write = 1;
            printf("Enter outfile: ");
            // Allows for filename up to 200 chars
            outname = malloc(sizeof(char) * 200);
            scanf("%s", outname);
            getchar();  // Remove extra \n
        }
    }

    if (write) {

        // Allow user to write message

        unsigned char *message, c;
        int i = 0, j = 1;

        // Memory allocated to message dynamically based on message length
        message = (unsigned char*)malloc(sizeof(unsigned char));

        printf("Enter Secret Message:\n");

        while (1) {
            c = getchar();

            if (c == '\n'){
                break;
            }

            // Re-allocate memory every one hundred chars
            if ((j - 1) % 100 == 0) {
                int newSize = (j * 100) * sizeof(unsigned char);
                message = (unsigned char*)realloc(message, newSize);
            }
            message[i] = c;

            i++;
            j++;
        }

        int messageLength = j;

        createSecret(message, messageLength, fname, outname, 0);

        free(message);
        free(outname);
    }
    else if (read) {
        readSecret(readLength, fname);
    }
    else if (writeFromFile) {

		FILE *textFile = fopen(text, "r");

		if (textFile == NULL){
			printf("Could not open file %s", text);
			return 1;
		}

		// Data is dumped into the new file every MAX_LOAD characters
		const int MAX_LOAD = 10000;

		unsigned char *readText =
			(unsigned char*)malloc(sizeof(unsigned char) * MAX_LOAD);

		unsigned char inChar;
		int j = 0;
		int finalOffset = 0;
		while (1) {
			inChar = getc(textFile);
			if ((signed char)inChar == EOF) {
				break;
			}

			readText[j % MAX_LOAD] = inChar;

			// Dump data into file every ten thousand chars (offset by 1)
			if ((j + 1) % MAX_LOAD == 0) {
				createSecret(readText, MAX_LOAD, fname, outname, finalOffset);
				fname = outname;

				// Clear memory
                memset(readText, 0, MAX_LOAD);

				finalOffset += MAX_LOAD;
			}

            j++;
		}

		createSecret(readText, j - finalOffset, fname, outname, finalOffset);

		free(readText);
        free(text);
        free(outname);

    }
    else {
        printf("Error\n");
        exit(1);
    }

    free(fname);

    return 0;
}
