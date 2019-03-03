/*
Use:
gcc -std=c99 ../testSeq.c ../libmorph.c ../GreekForms.c ../accent.c ../utilities.c ../augment.c ../ending.c ../VerbSequence.c -I.. -o testSeq
./checkVerbForms2
diff -u 2017-03-23_19-23-18.txt 2017-03-23_19-25-58.txt
*/
#include <time.h>
#include <stdlib.h> // For random(), RAND_MAX
#include <string.h>  //for strlen
#include <stdbool.h> //for bool type
#include "libmorph.h"
#include "utilities.h"
#include "VerbSequence.h"
#include "sqlite3.h"

int numVerbs = 125;

int main(int argc, char **argv)
{
    int rowCount = 0;
    int bufferLen = 50;
    char buffer[bufferLen];
    int bufferLen2 = 137;
    char buffer2[bufferLen2];

    VerbSeqOptions opt;
    opt.practiceVerbID = 1;

    opt.seqOptions.numPerson = 3;
    opt.seqOptions.numNumbers = 2;
    opt.seqOptions.numTense = 6;
    opt.seqOptions.numVoice = 3;
    opt.seqOptions.numMood = 4;
    opt.seqOptions.numVerbs = 125;

    memmove(opt.seqOptions.persons, (int[]){0,1,2}, opt.seqOptions.numPerson*sizeof(int));
    memmove(opt.seqOptions.numbers, (int[]){0,1}, opt.seqOptions.numNumbers*sizeof(int));
    memmove(opt.seqOptions.tenses, (int[]){0,1,2,3,4,5}, opt.seqOptions.numTense*sizeof(int));
    memmove(opt.seqOptions.voices, (int[]){0,1,2}, opt.seqOptions.numVoice*sizeof(int));
    memmove(opt.seqOptions.moods, (int[]){0,1,2,3}, opt.seqOptions.numMood*sizeof(int));
    //memmove(opt.seqOptions.verbs, (int[]){3}, opt.seqOptions.numVerbs*sizeof(int));

    for (int i = 0; i < numVerbs; i++)
    {
        opt.seqOptions.verbs[i] = i;
    }

    buildSequence(&opt);
/*
    char buff[] = "1,2,3,5";
    char *aux;
    int arr[50];
    int length = 0;
    aux = strtok(buff, ",");
    while (aux)
    {
        arr[length] = atoi(aux);
        length++;
        aux = strtok(NULL, ",");
    }
    for (int i = 0; i < length; i++)
    {
        printf("%d,",arr[i]);
    }
    printf("\n");
*/
    //fprintf(stdout, "\nTotal rows including -: %d\n", rowCount);
    VerbFormD v;
    v.person = 0;
    v.number = 0;
    v.tense = 2;
    v.voice = 1;
    v.mood = 0;
    v.verbid = 76;
    char buf[1024];
    getForm2(&v, buffer, 1024, true, false);
    printf("buffer: %s\n", buffer);

    /*
    jmarch-itsvm

    Domain: corporate.testsys.com

    erxomai
    epomai -- check all deponents
    prodidwmi
    thnhskw

    mi verb +present / aorist
      subj.
      opt.
      pres.
        indic.
        imper.
      aor.
        indic.
        imper.
      imperf.
    isthmi + perf/plup.

    root aorist?

    edidoun ending
    apothnskw
    */
}
