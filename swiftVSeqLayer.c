//
//  swiftVSeqLayer.c
//  HopliteChallenge
//
//  Created by Jeremy March on 11/4/17.
//  Copyright © 2017 Jeremy March. All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include "swiftVSeqLayer.h"
#include "VerbSequence.h"

VerbSeqOptions swiftLayerOptions; //this is the global options for the app.

void setOptionsxx(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, bool shuffle, int repsPerVerb, int topUnit)
{
    VerbSeqOptions opt;
    memmove(opt.seqOptions.persons, persons, numPersons*(sizeof(opt.seqOptions.persons[0])));
    opt.seqOptions.numPerson = numPersons;
    memmove(opt.seqOptions.numbers, numbers, numNumbers*(sizeof(opt.seqOptions.numbers[0])));
    opt.seqOptions.numNumbers = numNumbers;
    memmove(opt.seqOptions.tenses, tenses, numTenses*(sizeof(opt.seqOptions.tenses[0])));
    opt.seqOptions.numTense = numTenses;
    memmove(opt.seqOptions.voices, voices, numVoices*(sizeof(opt.seqOptions.voices[0])));
    opt.seqOptions.numVoice = numVoices;
    memmove(opt.seqOptions.moods, moods, numMoods*(sizeof(opt.seqOptions.moods[0])));
    opt.seqOptions.numMood = numMoods;
    memmove(opt.seqOptions.verbs, verbs, numVerbs*(sizeof(opt.seqOptions.verbs[0])));
    opt.seqOptions.numVerbs = numVerbs;
    
    opt.shuffle = shuffle;
    opt.repsPerVerb = (unsigned char) repsPerVerb;
    
    printf("here set options");
}
/*
void setOptions()
{
    swiftLayerOptions.startOnFirstSing = false;
}

void externalSetUnits(const char *unitStr)
{
    int i = 0;
    char *end = (char *) unitStr;
    while (*end && i < 20)
    {
        long unitNum = strtol(unitStr, &end, 10);
        
        swiftLayerOptions.units[i] = (int) unitNum;
        //printf("%ld\n", iUnits[i]);
        i++;
        while (*end == ',')
        {
            end++;
        }
        unitStr = end;
    }
    swiftLayerOptions.numUnits = i;
}
*/

int nextVS(int *seq, VerbFormD *vf1, VerbFormD *vf2)
{
    fprintf(stdout, "SWIFT LAYER1\n\n");
    //int a = nextVerbSeq2(vf1, vf2, &swiftLayerOptions);
    int a = nextVerbSeqCustomDB(vf1, vf2);
    //int a = nextVerbSeqCustom(vf1, vf2);
    *seq = swiftLayerOptions.verbSeq;
    fprintf(stdout, "SWIFT LAYER2\n\n");
    return a;
}

bool checkVFResult(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed, const char *elapsedTime, int *score, int *lives)
{
    bool a = compareFormsCheckMFRecordResult(expected, expectedLen, entered, enteredLen, MFPressed, elapsedTime, &swiftLayerOptions);
    
    *lives = swiftLayerOptions.lives;
    *score = swiftLayerOptions.score;
    
    return a;
}

bool checkVFResultNoSave(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed)
{
    bool a = compareFormsCheckMF(expected, expectedLen, entered, enteredLen, MFPressed);
    
    return a;
}
/*
void swiftResetVerbSeq()
{
    swiftLayerOptions.isHCGame = true;
    swiftLayerOptions.gameId = GAME_INSIPIENT;
    swiftLayerOptions.startOnFirstSing = false;
    swiftLayerOptions.degreesToChange = 2;
    swiftLayerOptions.practiceVerbID = -1;//4;
    
    swiftLayerOptions.seqOptions.persons[0] = 0;
    swiftLayerOptions.seqOptions.persons[1] = 1;
    swiftLayerOptions.seqOptions.persons[2] = 2;
    swiftLayerOptions.seqOptions.numbers[0] = 0;
    swiftLayerOptions.seqOptions.numbers[1] = 1;
    swiftLayerOptions.seqOptions.tenses[0] = 0;
    swiftLayerOptions.seqOptions.tenses[1] = 1;
    swiftLayerOptions.seqOptions.tenses[2] = 2;
    swiftLayerOptions.seqOptions.tenses[3] = 3;
    swiftLayerOptions.seqOptions.tenses[4] = 4;
    swiftLayerOptions.seqOptions.voices[0] = 0;
    swiftLayerOptions.seqOptions.voices[1] = 1;
    swiftLayerOptions.seqOptions.voices[2] = 2;
    swiftLayerOptions.seqOptions.moods[0] = 3;
    swiftLayerOptions.seqOptions.moods[1] = 1;
    swiftLayerOptions.seqOptions.moods[2] = 2;
    swiftLayerOptions.seqOptions.moods[3] = 3;
    swiftLayerOptions.seqOptions.verbs[0] = 3;
    
    swiftLayerOptions.seqOptions.numPerson = 3;
    swiftLayerOptions.seqOptions.numNumbers = 2;
    swiftLayerOptions.seqOptions.numTense = 5;
    swiftLayerOptions.seqOptions.numVoice = 3;
    swiftLayerOptions.seqOptions.numMood = 1;
    swiftLayerOptions.seqOptions.numVerbs = 1;
    
    externalSetUnits("2");
    
    //resetVerbSeq(&swiftLayerOptions);
}
*/
