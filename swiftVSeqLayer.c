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
extern VerbSeqOptionsNew opt; //global options

void setOptionsxx(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, const int *units, const int numUnits, bool shuffle, int repsPerVerb, int topUnit, bool isGame)
{
    //VerbSeqOptions opt;
    memmove(opt.persons, persons, numPersons*(sizeof(opt.persons[0])));
    opt.numPerson = numPersons;
    memmove(opt.numbers, numbers, numNumbers*(sizeof(opt.numbers[0])));
    opt.numNumbers = numNumbers;
    memmove(opt.tenses, tenses, numTenses*(sizeof(opt.tenses[0])));
    opt.numTense = numTenses;
    memmove(opt.voices, voices, numVoices*(sizeof(opt.voices[0])));
    opt.numVoice = numVoices;
    memmove(opt.moods, moods, numMoods*(sizeof(opt.moods[0])));
    opt.numMood = numMoods;

    memmove(opt.verbs, verbs, numVerbs*(sizeof(opt.verbs[0])));
    opt.numVerbs = numVerbs;
    memmove(opt.units, units, numUnits*(sizeof(opt.units[0])));
    opt.numUnits = numUnits;
    
    opt.isHCGame = isGame;
    opt.topUnit = topUnit;
    opt.shuffle = shuffle;
    opt.repsPerVerb = (unsigned char) repsPerVerb;
    opt.repNum = -1;
    if (opt.isHCGame)
    {
        opt.lives = 3;
    }
    opt.gameId = GAME_INCIPIENT; //this starts a new game
    
    printf("here set options. is game: %d\n", isGame);
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
    //fprintf(stdout, "SWIFT LAYER1\n\n");
    //int a = nextVerbSeq2(vf1, vf2, &swiftLayerOptions);
    int a = nextVerbSeqCustomDB(vf1, vf2);
    //int a = nextVerbSeqCustom(vf1, vf2);
    //*seq = swiftLayerOptions.verbSeq;
    //fprintf(stdout, "SWIFT LAYER2\n\n");
    return a;
}

bool checkVFResult(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed, const char *elapsedTime, int *score, int *lives)
{
    bool isCorrect = compareFormsCheckMFRecordResult(expected, expectedLen, entered, enteredLen, MFPressed, elapsedTime, &opt);
    
    *lives = opt.lives;
    *score = opt.score;
    
    printf("swiftc lives: %d\n", *lives);
    
    return isCorrect;
}

bool checkVFResultNoSave(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed)
{
    bool a = compareFormsCheckMF(expected, expectedLen, entered, enteredLen, MFPressed);
    
    return a;
}
