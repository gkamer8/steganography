#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createSecret(unsigned char *message, int messageLength,
                  char *inFilename, char *outFilename) {

  FILE *image = fopen(inFilename, "rb");

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

  //int messageLength = sizeof(message)/sizeof(unsigned char);

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

    imageData[i] = (imageData[i] & 0b11111100) | messageByte;

  }

  // Write out file
  FILE *outImage = fopen(outFilename, "wb");

  fwrite(header, 1, 14, outImage);
  fwrite(info, 1, offset - 14, outImage);
  fwrite(imageData, 1, size - offset, outImage);

  fclose(outImage);
}

void readSecret(int messageLength, char* inFilename) {

  // Read message

  FILE *readImage = fopen(inFilename, "rb");

  // Get offset
  unsigned char headerRead[14];
  fread(headerRead, sizeof(unsigned char), 14, readImage);
  int offsetRead = *(int*)&headerRead[0x000A];

  fseek(readImage, offsetRead - 14, SEEK_CUR);  // Skip over offset

  // Read in image data
  unsigned char mess[messageLength * 4];
  fread(mess, sizeof(unsigned char), messageLength * 4, readImage);

  fclose(readImage);

  unsigned char unencoded[messageLength];  // for storing message

  unsigned char nextChar;  // for storing each character during construction
  for (int i = 0; i < (messageLength * 4); i++) {
    int character = i / 4;  // each character is contructed over four bytes
    int bitPairNum = i % 4;

    unsigned char lastTwo = mess[i] & 0b00000011;

    if (bitPairNum == 0) {
      nextChar = lastTwo;
    }
    else {
      nextChar = (nextChar << 2) | lastTwo;
    }

    if (bitPairNum == 3) {
      unencoded[character] = nextChar;  // on last two bits, add to unencoded
    }
  }

  printf("Secret message: \n%s\n", unencoded);

}

int main(int argc, char const *argv[]) {

  char usageStatement[200];
  sprintf(usageStatement,
    "usage: %s infile [-r length] [-w outfile] [-wf outfile textfile]\n",
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
    else if (strcmp(argv[i], "-wf") == 0) {
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
    char choice;
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
    }
  }

  if (write == 1) {

    // Allow user to write message

    unsigned char *message, c;
    int i = 0, j = 1;

    // Memory allocated to message dynamically based on message length
    message = (unsigned char*)malloc(sizeof(unsigned char));

    printf("Enter Secret Message:\n");

    // ? add: && c != EOF
    while (c != '\n') {
      c = getc(stdin);

      if (c == '\n'){
        break;
      }

      // Re-allocate memory
      message = (unsigned char*)realloc(message, j * sizeof(unsigned char));
      message[i] = c;

      i++;
      j++;
    }

    int messageLength = j;

    createSecret(message, messageLength, fname, outname);
  }
  else if (read == 1) {
    readSecret(readLength, fname);
  }
  else if (writeFromFile == 1) {
    printf("Write from file not implemented.\n");
    exit(0);
  }
  else {
    printf("Error\n");
    exit(1);
  }

  return 0;
}
