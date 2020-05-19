# Steganography Tool

This is a command line application I've written to help me learn C. It hide messages in images using least significant (two) bit stegangography. Presently, the program works on Windows Bitmap images and does not automatically encrypt messages.

## Usage

`usage: steg infile [-r length] [-w outfile] [-f outfile textfile]`

The program takes one mandatory argument, the image file. Users are then prompted to specify whether they would like to read the image or encode a message in the image. The optional argument [-r length] specifies that the user would like to read a message and takes the length of the message as an argument. The optional argument [-w outfile] specifies that the user would like to encode a message where outfile is the name of the image produced by the program. The optional argument [-f outfile textfile] allows for messages to be written from a text file.

Input messages are limited to 1/4th the size of the input image (for every one bit inserted, three are taken from the input image). When entering text into the terminal, messages are limited by the operating system's allocation of memory for the text input. When using a text file as input, memory is allocated and dumped into the output file 10,000 bytes at a time.


## Organization

The code is completely contained in the steg.c file, where two functions (createSecret and readSecret) handle writing and reading encrypted messages, respectively. The main function first interprets the arguments and then goes to one of three processes, corresponding to reading, writing, and writing from a file.

## Examples

Example files are in the examples folder. The file alice.txt contains the entire text of Alice in Wonderland, which is encoded in the least significant bits of alice.bmp. The alice.bmp file is nearly visually identical to pic.bmp, which is a picture of Leverett House at Harvard College.

Here is an example call of the program that reads Alice in Wonderland from alice.bmp: `./steg alice.bmp -r 148573`. Nothing terrible happens if the length is off, but it is cleaner to use the exact number.

