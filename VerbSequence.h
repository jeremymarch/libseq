//
//  VerbSequence.h
//  Hoplite Challenge
//
//  Created by Jeremy on 2/1/16.
//  Copyright © 2016 Jeremy March. All rights reserved.
//

#ifndef VerbSequence_h
#define VerbSequence_h

#include <stdio.h>
//https://stackoverflow.com/questions/2093069/xcode-how-to-include-c-library-and-header-file-to-cocoa-project
#include <libmorph.h>
#include <GreekForms.h>

enum {
    STATE_ERROR = 0,
    STATE_NEW,
    STATE_REP,
    STATE_GAMEOVER
};

enum {
    GAME_INVALID = -1,
    GAME_INCIPIENT = 0, //A game which is started, but not yet in the db.  We add it to the db when first item is answered
    GAME_PRACTICE = 1 //the practice "game" has an id of 1.
};

typedef struct vso {
    
    bool startOnFirstSing;
    bool askEndings;
    bool askPrincipalParts;
    bool isHCGame; //else is practice
    bool shuffle;
    int repsPerVerb;
    int repNum;
    int degreesToChange;

    //int practiceVerbID; //to just practice on one verb
    long gameId;
    int score;
    int lives;
    int verbSeq;
    bool firstVerbSeq;
    bool lastAnswerCorrect;
    int lastFormID;
    int currentVerbIdx;
    int state;

    int numPerson;
    int numNumbers;
    int numTense;
    int numVoice;
    int numMood;
    int numVerbs;
    int numUnits;
    int persons[3];
    int numbers[2];
    int tenses[6];
    int voices[3];
    int moods[4];
    int verbs[NUM_VERBS];
    int units[20];
    int topUnit;
    VerbFormD givenForm;
    VerbFormD requestedForm;
} VerbSeqOptions;

int vsInit(VerbSeqOptions *vs, const char *path); //0 for success, else error code
void vsAddVerbsForUnit(VerbSeqOptions *vs, int unit, int *verbArray, int *verbArrayLen, int verbArrayCapacity);
int vsNext(VerbSeqOptions *vs, VerbFormD *vf1, VerbFormD *vf2);
bool vsCompareFormsRecordResult(VerbSeqOptions *vs, UCS2 *expected, int expectedLen, UCS2 *given, int givenLen, bool MFPressed, const char *elapsedTime, int *score, int *lives);
void vsReset(VerbSeqOptions *vs, bool isGame);

#endif /* VerbSequence_h */


