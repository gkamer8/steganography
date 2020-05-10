# Steganography Tool

This is a command line application written in C to hide messages in images. Presently, the program works on Windows Bitmap images. It works by hiding the text in the least two significant bits of each byte in the image, without encryption.

## Usage

The program takes one mandatory argument, the image file. Users are then prompted to specify whether they would like to read the image or encode a message in the image. The optional argument [-r length] specifies that the user would like to read a message and takes the length of the message as an argument. The optional argument [-w outfile] specifies that the user would like to encode a message where outfile is the name of the image produced by the program.

Here is an example call of the program that reads a message of at most 500 characters: `./steg pic.bmp -r 500`.

## Organization

The code is completely contained in the steg.c file, where two functions (createSecret and readSecret) hand writing and reading encrypted messages, respectively.

