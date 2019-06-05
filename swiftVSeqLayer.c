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

extern VerbSeqOptions opt; //global options

void setVerbSeqOptions(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, const int *units, const int numUnits, bool shuffle, int repsPerVerb, int topUnit, bool isGame)
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

    if (numVerbs < 1) //add verbs by unit
    {
        opt.numVerbs = 0;
        for (int i = 0; i < numUnits; i++)
        {
            addVerbsForUnit(units[i], opt.verbs, &opt.numVerbs, NUM_VERBS);
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
    if (opt.isHCGame)
    {
        opt.lives = 3;
    }
    opt.gameId = GAME_INCIPIENT; //this starts a new game
    
    fprintf(stderr, "here set options. is game: %d\n", isGame);
}


