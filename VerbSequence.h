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

enum {
    VERB_SEQ_CHANGE = 1,
    VERB_SEQ_CHANGE_NEW,
    VERB_SEQ_PP,
    VERB_SEQ_ENDING
};

typedef struct vfr VerbFormRecord;
struct vfr {
    time_t time;
    int elapsedTime;
    int verb;
    unsigned char person;
    unsigned char number;
    unsigned char tense;
    unsigned char voice;
    unsigned char mood;
    bool correct;
    char answer[130]; //needs to be more than longest answer: Longest Form: 2,1,3,0,2, v: καταστήσαιεν, καταστήσειαν, κατασταῖεν, κατασταίησαν, l: 102
};



//for mmap on ios see: https://github.com/mekjaer/ios-mmap/blob/master/ios-mmap-example/ios-mmap-example/AppDelegate.m
typedef struct da {
    unsigned int numRecords;
    unsigned int head;
    VerbFormRecord vr[2000];
} DataFormat;

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

bool compareFormsRecordResult(UCS2 *expected, int expectedLen, UCS2 *given, int givenLen, bool MFPressed, const char *elapsedTime, int *score, int *lives);
bool dbInit(const char *path);
int nextVerbSeq(VerbFormD *vf1, VerbFormD *vf2);
void resetVerbSeq(bool isGame);
void addVerbsForUnit(int unit, int *verbArray, int *verbArrayLen, int verbArrayCapacity);

#endif /* VerbSequence_h */


