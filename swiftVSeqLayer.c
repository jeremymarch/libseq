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

//global options
VerbSeqOptions opt;

void swSetVerbSeqOptions(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, const int *units, const int numUnits, bool shuffle, int repsPerVerb, int topUnit, bool isGame)
{
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

    if (topUnit > 20)
    {
        topUnit = 20;
    }
    else if (topUnit < 2)
    {
        topUnit = 2;
    }

    if (numVerbs < 1) //add verbs by unit
    {
        opt.numVerbs = 0;
        if (isGame)
        {
            printf("top unit: %d", topUnit);
            for (int i = 0; i < topUnit; i++)
            {
                vsAddVerbsForUnit(&opt, i+1, opt.verbs, &opt.numVerbs, NUM_VERBS);
            }
        }
        else
        {
            for (int i = 0; i < numUnits; i++)
            {
                vsAddVerbsForUnit(&opt, units[i], opt.verbs, &opt.numVerbs, NUM_VERBS);
            }
        }
        memmove(opt.units, units, numUnits*(sizeof(opt.units[0])));
        opt.numUnits = numUnits;
    }
    else //specify verbs specifically rather than by unit
    {
        memmove(opt.verbs, verbs, numVerbs*(sizeof(opt.verbs[0])));
        opt.numVerbs = numVerbs;
    }
    
    opt.isHCGame = isGame;
    opt.topUnit = topUnit;
    opt.shuffle = shuffle;
    opt.repsPerVerb = (unsigned char) repsPerVerb;
    opt.repNum = -1;
    if (isGame)
    {
        opt.lives = 3;
    }
    if (topUnit < 3)
    {
        opt.degreesToChange = 1;
    }
    else
    {
        opt.degreesToChange = 2;
    }
    opt.gameId = GAME_INCIPIENT; //this starts a new game
    
    fprintf(stderr, "here set options. is game: %d\n", isGame);
}

int swvsInit(const char *path)
{
    return vsInit(&opt, path);
}

//void vsAddVerbsForUnit(int unit, int *verbArray, int *verbArrayLen, int verbArrayCapacity);

int swvsNext(VerbFormD *vf1, VerbFormD *vf2)
{
    return vsNext(&opt, vf1, vf2);
}

bool swvsCompareFormsRecordResult(UCS2 *expected, int expectedLen, UCS2 *given, int givenLen, bool MFPressed, const char *elapsedTime, int *score, int *lives)
{
    return vsCompareFormsRecordResult(&opt, expected, expectedLen, given, givenLen, MFPressed, elapsedTime, score, lives);
}

void swvsReset(bool isGame)
{
    vsReset(&opt, isGame);
}
