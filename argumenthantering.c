#include <stdio.h>
#include <math.h>
#include <string.h>

int main(int argc, char *argv[])
{

    float avg      = 0;
    int total      = 0;
    int maxlength  = 14; //Length of "Average length"

    for(int i = 0; i < argc; i++)
    {
      if(strlen(argv[i]) > maxlength)
      {
        maxlength = strlen(argv[i]);
      }
      total += strlen(argv[i]);
    }

    avg = ((double)total / (double)argc);

    for(int i = 0; i < argc; i++)
    {
        printf("%s", argv[i]);
        printf("%*.d\n", (int)(maxlength-strlen(argv[i]))+8, (int)strlen(argv[i]));
    }

    printf("Total length%*.d\n", (int)maxlength-4, (int)total);
    printf("Average length%*.2f\n", (int)maxlength-6, (double)avg);

    return 0;
}
