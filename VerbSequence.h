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
#include <libmorph.h>
#include <GreekForms.h>

enum {
    GAME_INVALID = -1,
    GAME_INSIPIENT = 0, //A game which is started, but not yet in the db.  We add it to the db when first item is answered
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

typedef struct so {
    int numPerson;
    int numNumbers;
    int numTense;
    int numVoice;
    int numMood;
    int numVerbs;
    int persons[3];
    int numbers[2];
    int tenses[6];
    int voices[3];
    int moods[4];
    int verbs[125];
} SeqOptions;

typedef struct vsoNew {
    
    bool startOnFirstSing;
    bool askEndings;
    bool askPrincipalParts;
    bool isHCGame; //else is practice
    bool shuffle;
    int repsPerVerb;
    int degreesToChange;

    //int practiceVerbID; //to just practice on one verb
    long gameId;
    int score;
    int lives;
    int verbSeq;
    bool firstVerbSeq;
    bool lastAnswerCorrect;

    int numUnits;
    int units[20];
    
    //SeqOptions
    int numPerson;
    int numNumbers;
    int numTense;
    int numVoice;
    int numMood;
    int numVerbs;
    int persons[3];
    int numbers[2];
    int tenses[6];
    int voices[3];
    int moods[4];
    int *verbs;
    int topUnit;
} VerbSeqOptionsNew;

typedef struct vso {
    bool startOnFirstSing;
    unsigned char repsPerVerb;
    unsigned char degreesToChange;
    unsigned char numUnits;
    bool askEndings;
    bool askPrincipalParts;
    bool isHCGame; //else is practice
    int practiceVerbID; //to just practice on one verb
    long gameId;
    int score;
    int lives;
    int verbSeq;
    bool firstVerbSeq;
    bool lastAnswerCorrect;
    bool shuffle;
    int units[20];
    SeqOptions seqOptions;
} VerbSeqOptions;

void externalSetUnits(const char *units);
bool compareFormsCheckMFRecordResult(UCS2 *expected, int expectedLen, UCS2 *given, int givenLen, bool MFPressed, const char *elapsedTime, VerbSeqOptions *opt);
//void closeDataFile();
//void syncDataFile();

bool buildSequence(VerbSeqOptions *vso);

bool dbInit(const char *path);
//void VerbSeqInit(const char *path);
int nextVerbSeq(VerbFormC *vf1, VerbFormC *vf2, VerbSeqOptions *vso);
int nextVerbSeq2(VerbFormD *vf1, VerbFormD *vf2, VerbSeqOptions *vso1);
int nextVerbSeqCustom(VerbFormD *vf1, VerbFormD *vf2);
int nextVerbSeqCustomDB(VerbFormD *vf1, VerbFormD *vf2);

void resetVerbSeq(VerbSeqOptions *opt);
void changeFormByDegrees(VerbFormC *verbform, int degrees);
void generateForm(VerbFormC *verbform);
void getDistractorsForChange(VerbFormC *orig, VerbFormC *new, int numDistractors, char *buffer);
bool isValidFormForUnit(VerbFormC *vf, int unit);
bool isValidFormForUnitD(VerbFormD *vf, int unit);


Verb *getRandomVerb(int *units, int numUnits);
Verb *getRandomVerbFromUnit(int *units, int numUnits);

Ending *getRandomEnding(int *units, int numUnits);
void getRandomEndingAsString(int *units, int numUnits, char *buffer, int bufferLen);

#endif /* VerbSequence_h */


