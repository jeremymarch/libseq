//
//  swiftVSeqLayer.h
//  HopliteChallenge
//
//  Created by Jeremy March on 11/4/17.
//  Copyright © 2017 Jeremy March. All rights reserved.
//

#ifndef swiftVSeqLayer_h
#define swiftVSeqLayer_h

#include <stdio.h>
#include "VerbSequence.h"

int nextVS(int *seq, VerbFormD *vf1, VerbFormD *vf2);

bool checkVFResult(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed, const char *elapsedTime, int *score, int *lives);

bool checkVFResultNoSave(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed);

void swiftResetVerbSeq(void);

void setOptionsxx(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, bool shuffle, int repsPerVerb, int topUnit, bool isGame);


#endif /* swiftVSeqLayer_h */
