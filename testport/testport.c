#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <errno.h>
#include <time.h>

#include <peek.h>

HFILE          hFile;
ULONG          data;

int main (int argc, char *argv[])
    {
    long port;
    int  value, pos, i;
    char binary[9];

    if (argc != 2)
        {
        printf ("%s <portnumber>\n  Example: %s 0x201\n", argv[0], argv[0]);
        return 1;
        }

    port = strtol (argv[1], NULL, 0);
    if (errno == ERANGE)
        {
        printf ("invalid number %s\n", argv[1]);
        return 1;
        }

    printf ("working on port 0x%X\n", port);

    while (TRUE)
        {
        clock_t t1, t2;

        value = PeekReadByte (port);
        for (pos=0, i = 0x80; i > 0; i >>= 1, pos++)
            {
            if (value & i)
                binary[pos] = '1';
            else
                binary[pos] = '0';
            }
        binary[pos] = '\0';
        printf ("0x%2x (%sB)\n", value, binary);
        DosSleep (50);
        }
    }

