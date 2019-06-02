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

void setVerbSeqOptions(const int *persons, const int numPersons, const int *numbers, const int numNumbers, const int *tenses, const int numTenses, const int *voices, const int numVoices, const int *moods, const int numMoods, const int *verbs, const int numVerbs, const int *units, const int numUnits, bool shuffle, int repsPerVerb, int topUnit, bool isGame);


#endif /* swiftVSeqLayer_h */
