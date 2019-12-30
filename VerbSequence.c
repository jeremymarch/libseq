//
//  VerbSequence.c
//  Hoplite Challenge
//
//  Created by Jeremy on 2/1/16.
//  Copyright © 2016 Jeremy March. All rights reserved.
//

#include <stdlib.h> // For random(), RAND_MAX
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <utilities.h>
#include <assert.h>
#include "VerbSequence.h"
#include "sqlite3.h"

//or we could pic a random verb from first half of array.
//then move last used verb to end of array


//what if we had db of verbforms like this:
//ω verb forms
//μι verb forms where different
//any interesting alternates, otherwise need to be asked freq.
//then we don't ask same verb form for a different verb often.
//we would get more variation
//at max difficulty we would use different principal part for every change.

//https://stackoverflow.com/questions/1941307/debug-print-macro-in-c

#if defined(HCDEBUG) && ( defined(__ANDROID__) || defined(ANDROID) )
#include <android/log.h>
#define  DEBUG_PRINT(...)  ((void)__android_log_print(ANDROID_LOG_ERROR, "Hoplite", __VA_ARGS__))
#elif defined(HCDEBUG)
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#define MAX_RECENT_VF 30
#define SQLITEPREPQUERYLEN 1024
#define BONUS_POINTS_MULTIPLE 2

bool isValidFormForUnit(VerbFormC *vf, int unit);
bool isValidFormForUnitD(VerbFormD *vf, int unit);

void copyVFD(VerbFormD *fromVF, VerbFormD *toVF);
//void copyVFC(VerbFormC *fromVF, VerbFormC *toVF);
bool setupVerbFormsTable(void);
void insertDB(int formid, VerbFormD *vf, sqlite3_stmt *stmt);
bool sqliteTableExists(char *tbl_name);
int sqliteTableCount(char *tbl_name);

//GLOBAL VARIABLES
sqlite3 *db = NULL;
char sqlitePrepquery[SQLITEPREPQUERYLEN];

/*
VerbFormC recentVFArray[MAX_RECENT_VF];
int numRecentVFArray = 0;
int recentVFArrayHead = -1;
*/

int u2[4] = {0,1,2,3};
int u3[4] = {4,5,6,7};
int u4[4] = {8,9,10,11};
int u5[4] = {12,13,14,15};
int u6[4] = {16,17,18,19};
int u7[3] = {20,21,22};
int u8[2] = {23,24};
int u9[4] = {25,26,27,28};
int u10[6] = {29,30,31,32,33,34};
int u11[7] = {35,36,37,38,39,40,41};
int u12[10] = {42,43,44,45,46,47,48,49,50,51};
int u13[8] = {52,53,54,55,56,57,58,59};
int u14[13] = {60,61,62,63,64,65,66,67,68,69,70,71,72};
int u15[8] = {73,74,75,76,/*77,78,*/79,80,81,82};
int u16[9] = {83,84,85,86,87,88,89,90,91};
int u17[7] = {92,93,94,95,96,97,98};
int u18[11] = {99,100,101,102,103,104,105,106,107,108,109};
int u19[10] = {110,111,112,113,114,115,116,117,118,119};
int u20[5] = {120,/*121,*/122,123,124,125/*,126*/}; //exclude dei, xrh
void vsAddVerbsForUnit(VerbSeqOptions *vs, int unit, int *verbArray, int *verbArrayLen, int verbArrayCapacity)
{
    int i = 0;
    switch(unit)
    {
        case 1:
            break;
        case 2:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u2[i];
            break;
        case 3:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u3[i];
            break;
        case 4:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u4[i];
            break;
        case 5:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u5[i];
            break;
        case 6:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u6[i];
            break;
        case 7:
            for ( ; i < 3 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u7[i];
        case 8:
            for ( ; i < 2 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u8[i];
            break;
        case 9:
            for ( ; i < 4 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u9[i];
            break;
        case 10:
            for ( ; i < 6 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u10[i];
            break;
        case 11:
            for ( ; i < 7 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u11[i];
            break;
        case 12:
            for ( ; i < 10 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u12[i];
            break;
        case 13:
            for ( ; i < 8 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u13[i];
            break;
        case 14:
            for ( ; i < 13 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u14[i];
            break;
        case 15:
            for ( ; i < 8 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u15[i];
            break;
        case 16:
            for ( ; i < 9 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u16[i];
            break;
        case 17:
            for ( ; i < 7 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u17[i];
            break;
        case 18:
            for ( ; i < 11 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u18[i];
            break;
        case 19:
            for ( ; i < 10 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u19[i];
            break;
        case 20:
            for ( ; i < 5 && i + *verbArrayLen < verbArrayCapacity; i++)
                verbArray[*verbArrayLen + i] = u20[i];
            break;
    }
    *verbArrayLen += i;
}

//true for same, false for different
bool compareVF(VerbFormC *vf1, VerbFormC *vf2)
{
    if (vf1->person != vf2->person)
        return false;
    if (vf1->number != vf2->number)
        return false;
    if (vf1->tense != vf2->tense)
        return false;
    if (vf1->voice != vf2->voice)
        return false;
    if (vf1->mood != vf2->mood)
        return false;
    if (vf1->verb != vf2->verb)
        return false;

    return true;
}

//how many parameters are different
int stepsAway(VerbFormD *vf1, VerbFormD *vf2)
{
    int steps = 0;
    if (vf1->person != vf2->person)
        steps++;
    if (vf1->number != vf2->number)
        steps++;
    if (vf1->tense != vf2->tense)
        steps++;
    if (vf1->voice != vf2->voice)
        steps++;
    if (vf1->mood != vf2->mood)
        steps++;

    return steps;
}

int findVerbIndexByPointer(Verb *v)
{
    for (int i = 0; i < NUM_VERBS; i++)
    {
        if (v == &verbs[i])
            return i;
    }
    return -1;
}

void randomAlternative(char *s, int *offset);
void addNewGameToDB(VerbSeqOptions *vs, int topUnit, long *gameid, bool isGame);
void updateGameScore(long gameid, int score, int lives);
bool setHeadAnswer(VerbFormD *requestedForm, bool correct, char *givenAnswer, const char *elapsedTime, VerbSeqOptions *vso);

int getVerbSeqCallback(void *NotUsed, int argc, char **argv,
             char **azColName) {

    NotUsed = 0;

    for (int i = 0; i < argc; i++) {

        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    //printf("\n");

    return 0;
}

void getVerbSeq()
{
    char *err_msg = 0;
    sqlite3_exec(db, "SELECT COUNT(*) FROM verbseq;", getVerbSeqCallback, 0, &err_msg);
}

int callback(void *NotUsed, int argc, char **argv,
             char **azColName) {

    NotUsed = 0;

    for (int i = 0; i < argc; i++) {

        fprintf(stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    fprintf(stderr, "\n");

    return 0;
}

bool vsCompareFormsRecordResult(VerbSeqOptions *vs, UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed, const char *elapsedTime, int *score, int *lives)
{
    char buffer[200];
    bool isCorrect = compareFormsCheckMF(expected, expectedLen, entered, enteredLen, MFPressed);

    ucs2_to_utf8_string(entered, enteredLen, (unsigned char*)buffer);

    if(vs->gameId == GAME_INCIPIENT)
    {
        DEBUG_PRINT("is new gameid: %ld, %d\n", vs->gameId,vs->isHCGame);
        long localGameId = GAME_INCIPIENT;
        addNewGameToDB(vs, vs->topUnit, &localGameId, vs->isHCGame);
        vs->gameId = localGameId;
    }

    if (vs->isHCGame) //is a real game, not practice
    {
        if (vs->score < 0)
        {
            vs->score = 0;
        }

        if (isCorrect)
        {
            int pointsPerForm = vs->topUnit;
            if ( vs->repNum >= vs->repsPerVerb ) //should never be greater than
                vs->score += (pointsPerForm * BONUS_POINTS_MULTIPLE); //add bonus here
            else
                vs->score += pointsPerForm;
        }
        else
        {
            vs->lives -= 1;
            vs->repNum = -1; //to start with new verb
            if ( vs->lives < 1 )
            {
                vs->state = STATE_GAMEOVER;
            }
        }
        DEBUG_PRINT("Score: %i, Lives: %i\n", vs->score, vs->lives);

        updateGameScore(vs->gameId, vs->score, vs->lives);
    }

    vs->lastAnswerCorrect = isCorrect; //keep track of last answer here, so we don't need to rely on the db

    setHeadAnswer(&vs->requestedForm, isCorrect, buffer, elapsedTime, vs);
    
    *lives = vs->lives;
    *score = vs->score;

    return isCorrect;
}

int currentVerb = 0;
void vsReset(VerbSeqOptions *vs, bool isGame)
{
    vs->gameId = GAME_INCIPIENT;
    if (isGame)
    {
        DEBUG_PRINT("reset game\n");
        vs->score = -1;
        vs->lives = 3;
        vs->isHCGame = true;
    }
    else
    {
        DEBUG_PRINT("reset practice\n");
        vs->score = -1;
        vs->lives = -1;
        vs->isHCGame = false;
    }
    vs->state = STATE_NEW;
    vs->repNum = -1;
    vs->lastAnswerCorrect = false;
    vs->currentVerbIdx = 0;
}

void copyVFD(VerbFormD *fromVF, VerbFormD *toVF)
{
    toVF->person = fromVF->person;
    toVF->number = fromVF->number;
    toVF->tense = fromVF->tense;
    toVF->voice = fromVF->voice;
    toVF->mood = fromVF->mood;
    toVF->verbid = fromVF->verbid;
}

void copyVFC(VerbFormC *fromVF, VerbFormC *toVF)
{
    toVF->person = fromVF->person;
    toVF->number = fromVF->number;
    toVF->tense = fromVF->tense;
    toVF->voice = fromVF->voice;
    toVF->mood = fromVF->mood;
    toVF->verb = fromVF->verb;
}

//true if middle/passive form is being changed from middle to passive or vice versa
//i.e. if both are middle/passive forms and their voices are not the same
bool mpToMp(VerbFormD *vf1, VerbFormD *vf2)
{
    if ( getVoiceDescription2(vf1) == MIDDLEPASSIVE && getVoiceDescription2(vf2) == MIDDLEPASSIVE && vf1->voice != vf2->voice )
    {
        return true;
    }
    return false;
}

void getLastSeen(VerbFormD *vf1)
{
    sqlite3_stmt *res;
    char *sql = "SELECT person,number,tense,voice,mood,verbid FROM verbforms WHERE verbid= ?1 ORDER BY lastSeen DESC LIMIT 1;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, vf1->verbid);
    } else {
        DEBUG_PRINT("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        vf1->person = 0;
        vf1->number = 0;
        vf1->tense = 0;
        vf1->voice = 0;
        vf1->mood = 0;
        //sqlite3_finalize(res); //not needed if prepare fails, though no harm
        return;
    }

    //this should be sure starting form is valid for topunit
    if ( sqlite3_step(res) == SQLITE_ROW )
    {
        vf1->person = sqlite3_column_int(res, 0);
        vf1->number = sqlite3_column_int(res, 1);
        vf1->tense = sqlite3_column_int(res, 2);
        vf1->voice = sqlite3_column_int(res, 3);
        vf1->mood = sqlite3_column_int(res, 4);
    }
    else
    {
        vf1->person = 0;
        vf1->number = 0;
        vf1->tense = 0;
        vf1->voice = 0;
        vf1->mood = 0;
    }
    sqlite3_finalize(res);
}


//https://www.geeksforgeeks.org/shuffle-a-given-array-using-fisher-yates-shuffle-algorithm/
void swap (int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

//we want the last verbid to be last.
void randomize ( int arr[], int arrayLen, int lastVerbID)
{
    // Use a different seed value so that we don't get same
    // result each time we run this program
    srand ( (unsigned int)time(NULL) );

    // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (int i = arrayLen-1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i+1);

        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}

bool isBlankOrDashOrFails(VerbFormD *vf) {
    int bufferlen = 1024;
    UCS2 buffer[bufferlen];
    int len = 0;
    if(vf->verbid < 0 || vf->verbid >= NUM_VERBS)
    {
        return true;
    }
    VerbFormC vfc;
    vfc.person = vf->person;
    vfc.number = vf->number;
    vfc.tense = vf->tense;
    vfc.voice = vf->voice;
    vfc.mood = vf->mood;
    vfc.verb = (Verb *) &verbs[vf->verbid];
    
    if (getFormUCS2(&vfc, buffer, &len, bufferlen, true, false)) {
        for (int i = 0; i < len; i++)
        {
            //check whole word in case it has alternate forms
            if (buffer[i] == 0x2014 || buffer[i] == 0x002D || buffer[i] == 0x2010) //emdash or hyphen-minus or hyphen
            {
                //DEBUG_PRINT("found hyphen or blank: %d, %d, %d\n", buffer[i] == 0x2014, buffer[i] == 0x2010, buffer[i] == 0x002D);

                return true;
            }
        }
        return false; //ok to ask
    }
    else {
        return true; //if fails return true to block form
    }
}

int vsNext(VerbSeqOptions *vs, VerbFormD *vf1, VerbFormD *vf2)
{
    if (vs->isHCGame && vs->lives < 1)
    {
        vs->state = STATE_GAMEOVER;
        return vs->state;
    }

    vs->state = STATE_NEW;

    if (vs->numVerbs < 1)
    {
        vs->verbs[0] = 0;
        vs->numVerbs = 1;
    }
    //printf("repnum: \(repNum), \(verbIDs.count)")

    //are we at end and need to reshuffle?
    if (vs->repNum >= vs->repsPerVerb && vs->currentVerbIdx >= vs->numVerbs - 1)
    {
        vs->repNum = -1; //reshuffle and restart
    }

    //brand new or time to reshuffle
    if (vs->repNum < 0)
    {
        //no need to shuffle if there are only two
        if (vs->numVerbs == 1)
        {
            vs->repNum = 1;
            vs->repsPerVerb = 9999999;
            vs->currentVerbIdx = 0;
        }
        else if (vs->numVerbs == 2)
        {
            //flip them, if two, since we are setting vs->currentVerbIdx to 0 below
            int temp = vs->verbs[0];
            vs->verbs[0] = vs->verbs[1];
            vs->verbs[1] = temp;
        }
        else if (vs->numVerbs > 2)
        {
            //we don't want to randomly get same verb twice in a row
            //do {
            randomize(vs->verbs, vs->numVerbs, vf1->verbid);
            //} while (vs->verbs[0] == vf1->verbid);
        }
        vs->repNum = 1;
        vs->currentVerbIdx = 0;
        //givenForm.person = .unset; //reset
        vs->state = STATE_NEW;
    }
    else if (vs->repNum >= vs->repsPerVerb) // maxRepsPerVerb
    {
        vs->currentVerbIdx += 1;
        vs->repNum = 1;
        //givenForm.person = .unset; //reset
        vs->state = STATE_NEW;
    }
    else
    {
        //state = .rep????
        vs->repNum += 1;
        vs->state = STATE_REP;
    }

    vf1->verbid = vs->verbs[vs->currentVerbIdx];

    DEBUG_PRINT("verb rep: %d, reps per verb: %d, state: %d\n", vs->repNum, vs->repsPerVerb, vs->state);

    if (vs->state != STATE_NEW)
    {
        copyVFD(vf2, vf1);
    }
    
    if (vs->state == STATE_NEW)//unset
    {
        getLastSeen(vf1); //set first form to last seen
        DEBUG_PRINT("new verb\n");
    }

    //char *err_msg = 0;
    sqlite3_stmt *res;
    
    //add hits to results array, then choose a random one.
    int numResults = 10;
    int resultIdx = 0;
    VerbFormD results[numResults];
    int lastFormIDs[numResults];

    char *sql = "SELECT person,number,tense,voice,mood,verbid,formid,datetime(lastSeen, 'unixepoch', 'localtime') FROM verbforms WHERE verbid = ?1 ORDER BY lastSeen ASC, RANDOM();";
    assert(vf1->verbid > -1 && vf1->verbid < NUM_VERBS);

    int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, vf1->verbid);
    } else {
        DEBUG_PRINT("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    DEBUG_PRINT("sql: %d, %s\n", vf1->verbid, sql);
    
    //int desiredStepsAway = 2;
    //long long lastseen = 0;
    const unsigned char *lastlast;
    int formid = 0;
    while ( sqlite3_step(res) == SQLITE_ROW )
    {
        vf2->person = (unsigned char)sqlite3_column_int(res, 0);
        vf2->number = (unsigned char)sqlite3_column_int(res, 1);
        vf2->tense = (unsigned char)sqlite3_column_int(res, 2);
        vf2->voice = (unsigned char)sqlite3_column_int(res, 3);
        vf2->mood = (unsigned char)sqlite3_column_int(res, 4);
        vf2->verbid = sqlite3_column_int(res, 5);
        formid = sqlite3_column_int(res, 6);
        lastlast = sqlite3_column_text(res, 7);
        if (stepsAway(vf1, vf2) == vs->degreesToChange && !isBlankOrDashOrFails(vf2) && !mpToMp(vf1, vf2) && isValidFormForUnitD(vf2, vs->topUnit))
        {
            if (resultIdx < numResults)
            {
                copyVFD(vf2, &results[resultIdx]);
                lastFormIDs[resultIdx] = formid;
                resultIdx++;
                
                char buffer[1024];
                int bufferLen = 0;
                getForm2(vf2, buffer, bufferLen, true, false);
                DEBUG_PRINT("verb queue: %s, %s\n", lastlast, buffer);
            }
            else
            {
                break;
            }
        }
    }

    sqlite3_finalize(res);
    
    //pick random result from vf2
    long rand = randWithMax(resultIdx); //max is set by last index in array
    DEBUG_PRINT("RAND %d, MAX %d", rand, resultIdx);
    assert(resultIdx > 0);
    copyVFD(&results[rand], vf2);
    vs->lastFormID = lastFormIDs[rand];
    
    copyVFD(vf1, &vs->givenForm);
    copyVFD(vf2, &vs->requestedForm);
    

    if (vs->gameId != GAME_INCIPIENT && vs->state == STATE_NEW)
    {
        setHeadAnswer(&vs->givenForm, true, "START", NULL, vs);
    }

    return vs->state;
}

/*
 This is because we don't want to ask to change mid to pass/pass to mid unless its aorist or future
 return true if the tense is not aorist or future
 and changing from mid to pass or pass to mid.
 false means it's ok to ask for this change, true means don't
 */
bool isMidToPassOrPassToMid(VerbFormC *vf, int tempTense, int tempVoice)
{
    //if the new tense is aorist or future then ok to change.
    if (tempTense == AORIST || tempTense == FUTURE)
        return false;

    if ((vf->voice == MIDDLE && tempVoice == PASSIVE) || (vf->voice == PASSIVE && tempVoice == MIDDLE))
    {
        DEBUG_PRINT("Hit mid to pass or pass to mid\n");

        return true;
    }
    else
    {
        return false;
    }
}

//unit is the highest unit we're up to
bool isValidFormForUnitD(VerbFormD *vf, int unit)
{
    VerbFormC vfc;
    vfc.person = vf->person;
    vfc.number = vf->number;
    vfc.tense = vf->tense;
    vfc.voice = vf->voice;
    vfc.mood = vf->mood;
    vfc.verb = (Verb *) &verbs[vf->verbid];

    return isValidFormForUnit(&vfc, unit);
}

//unit is the highest unit we're up to
bool isValidFormForUnit(VerbFormC *vf, int unit)
{
    if (unit <= 2)
    {
        //2 and under active indicative and not perfect or pluperfect
        if (vf->tense == PERFECT || vf->tense == PLUPERFECT || vf->voice != ACTIVE || vf->mood != INDICATIVE)
            return false;
    }
    else if (unit <= 4)
    {
        //4 and under must be active, no imperatives
        if (vf->voice != ACTIVE || vf->mood == IMPERATIVE || (vf->tense == FUTURE && vf->mood == OPTATIVE))
            return false;
    }
    else if (unit <= 5)
    {
        //5 and under can't be middle, no imperatives
        if (vf->voice == MIDDLE || vf->mood == IMPERATIVE || (vf->tense == FUTURE && vf->mood == OPTATIVE))
            return false;
    }
    else if (unit <= 10)
    {
        //10 and under no imperatives
        if (vf->mood == IMPERATIVE || (vf->tense == FUTURE && vf->mood == OPTATIVE))
            return false;
    } /* I don't think we need this
    else if (unit <= 11)
    {
        return true;
    } */
    else if (unit <= 12)
    {
        //12 and under no aorists of mi verbs or perf/plup of isthmi
        if ((utf8HasSuffix(vf->verb->present, "μι") && vf->tense == AORIST) || (utf8HasSuffix(vf->verb->present, "στημι") && (vf->tense == AORIST || vf->tense == PERFECT || vf->tense == PLUPERFECT)) || (vf->tense == FUTURE && vf->mood == OPTATIVE))
            return false;
    }
    else if (unit <= 15)
    {
        //15 and under no future optative
        if (vf->tense == FUTURE && vf->mood == OPTATIVE)
            return false;
    }
    return true;
}

//sort with weights going from smallest to largest, they will be ints who add up to 100
//http://stackoverflow.com/questions/8529665/changing-probability-of-getting-a-random-number
int chooseRandomFromArrayWithWeighting(int *values, int len, int *weights)
{
    //or make weights a double? and out of 1?
    long rand = randWithMax(100);
    for (int i = 0; i < len; i++)
    {
        if (rand < weights[i])
            return values[i];
    }
    return 0; //shouldn't need this
}

void generateForm(VerbFormC *verbform)
{
    unsigned char iTense, iMood, iVoice, iPerson, iNumber;

    do
    {
        iTense = (unsigned char)randWithMax(NUM_TENSES);
        iMood = (unsigned char)randWithMax(NUM_MOODS);
        while ( iTense != PRESENT && iTense != AORIST && iMood != INDICATIVE )
            iMood = (unsigned char)randWithMax(NUM_MOODS);

        iVoice = (unsigned char)randWithMax(NUM_VOICES);
        /*
         if (iMood == 1)
         {
         iPerson = -1;
         iNumber = -1;
         }
         else if (iMood == 4)
         {
         iNumber = randWithMax([[self numbers] count]);
         iPerson = randWithMax([[self persons] count]);
         while (iPerson == 0)
         iPerson = randWithMax([[self persons] count]);
         }
         else
         {
         */
        iPerson = (unsigned char)randWithMax(NUM_PERSONS);
        iNumber = (unsigned char)randWithMax(NUM_NUMBERS);
        //}

        //NSArray conj = [NSArray arrayWithObjects: [NSNumber  v], nil];

        //NSUInteger randomIndex = randWithMax([theArray count]);
        //NSString *form = [NSString stringWithFormat:@"%@ %@ %@ %@ %@", (iPerson > -1) ? [[self persons] objectAtIndex: iPerson] : @"", (iNumber > -1) ? [[self numbers] objectAtIndex: iNumber] : @"", [[self tenses] objectAtIndex: iTense], [[self voices] objectAtIndex: iVoice], [[self moods] objectAtIndex: iMood]];
    }
    while (!formIsValidReal(iPerson, iNumber, iTense, iVoice, iMood) || getPrincipalPartForTense(verbform->verb, iTense, iVoice)[0] == '\0');

    verbform->person = iPerson;
    verbform->number = iNumber;
    verbform->tense = iTense;
    verbform->voice = iVoice;
    verbform->mood = iMood;
}

int incrementValue(int theArrayCount, int start)
{
    long n = randWithMax((theArrayCount - 1));
    for (int i = 0; i < n + 1; i++)
    {
        if (start < theArrayCount - 1)
            start++;
        else
            start = 0;
    }
    return start;
}

//From: http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
// Assumes 0 <= max <= RAND_MAX
// Returns in the half-open interval [0, max]
long randWithMax(unsigned int max)
{
    //return arc4random() % max;
    return arc4random_uniform(max);
}

/***********************DB**********************/
void vsClose(void)
{
    if (db)
    {
        sqlite3_close(db);
    }
}

int sqliteCheckFutSubj(void);

//returns 0 on success, error code otherwise
int vsInit(VerbSeqOptions *vs, const char *path)
{
    unsigned long dbpathLen = strlen(path) + 12;
    char dbpath[dbpathLen];
    snprintf(dbpath, dbpathLen - 1, "%s", path);

    long long int size = 0;
#ifdef HCDEBUG
    //char *dbp2 = "/data/user/0/com.philolog.hc/databases";
    struct stat64 st;
    int32_t res = stat64(dbpath, &st);
    if (res == 0) //0 for success
    {
        size = st.st_size;
    }
    /*
    if (0 == res && (st.st_mode & S_IFDIR)){
        DEBUG_PRINT("Database directory already exists in path:%s", dbp2);
    }else{
        DEBUG_PRINT("Creating database path:%s", dbpath);
        int status = mkdir(dbpath, S_IRWXU | S_IRWXG | S_IWOTH | S_IXOTH);
        if(status != 0){
            DEBUG_PRINT("Error occurred while creating database path : %s", dbpath);
            return 9;
        }
    }
    */
#endif

    if (db)
    {
        sqlite3_close(db);
    }

    char *zErrMsg = 0;
    int rc = sqlite3_open(dbpath, &db);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("Can't open database: %s. %s\n", sqlite3_errmsg(db), dbpath);
        sqlite3_close(db);
        db = NULL;
        return 1;
    }
    else
    {
        DEBUG_PRINT("SQLite db open, path: %s, size: %lld\n", dbpath, size);
    }

    //char *check = "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name'";
    //"DROP TABLE IF EXISTS games; DROP TABLE IF EXISTS verbseq;
    //DROP TABLE IF EXISTS verbforms;
    char *sql = "VACUUM; CREATE TABLE IF NOT EXISTS games (" \
    "gameid INTEGER PRIMARY KEY NOT NULL, " \
    "timest INT NOT NULL, " \
    "score INT NOT NULL, " \
    "topUnit INT NOT NULL, " \
    "lives INT NOT NULL " \
    "); " \
    
    "CREATE TABLE IF NOT EXISTS hqvocab (" \
    "hqid INTEGER PRIMARY KEY UNIQUE NOT NULL, " \
    "unit INTEGER NOT NULL, " \
    "lemma CHAR, " \
    "present CHAR, " \
    "future CHAR, " \
    "aorist CHAR, " \
    "perfect CHAR, " \
    "perfectmid CHAR, " \
    "aoristpass CHAR, " \
    "def CHAR, " \
    "pos CHAR, " \
    "note CHAR, " \
    "seq INTEGER, " \
    "sortkey CHAR); " \

    "CREATE TABLE IF NOT EXISTS verbseq (" \
    "id INTEGER PRIMARY KEY NOT NULL, " \
    "timest INT NOT NULL, " \
    "gameid INT NOT NULL, " \
    "person INT1 NOT NULL, " \
    "number INT1 NOT NULL, " \
    "tense INT1 NOT NULL, " \
    "voice INT1 NOT NULL, " \
    "mood INT1 NOT NULL, " \
    "verbid INT NOT NULL, " \
    "correct INT1 NOT NULL, " \
    "elapsedtime VARCHAR(255), " \
    "incorrectAns VARCHAR(255), " \
    "FOREIGN KEY (gameid) REFERENCES games(gameid) " \
    "); ";

    //"INSERT OR IGNORE INTO games VALUES (1,0,-1,0,0);"; //This is the Practice Game

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 2;
    }

    char *verbForms = "verbforms";
    bool exists = sqliteTableExists(verbForms);
    int vfcount = sqliteTableCount(verbForms);

    DEBUG_PRINT("exists %d, count %d\n", exists, vfcount);
    assert(vfcount == 21117);
    assert(sqliteCheckFutSubj() == 0);
    //DEBUG_PRINT("COUNT FUT SUBJ: %d\n", sqliteCheckFutSubj());

    if ( !exists || vfcount < 1 )
    {
        setupVerbFormsTable();
        DEBUG_PRINT("created verb forms table\n");
    }
    
    assert(sqliteCheckFutSubj() == 0);
    //DEBUG_PRINT("COUNT FUT SUBJ: %d\n", sqliteCheckFutSubj());

    DEBUG_PRINT("sqlite success, version: %s\n", SQLITE_VERSION);
    return 0;
}

//note the table name is case-sensitive here
bool sqliteTableExists(char *tbl_name)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM sqlite_master where type='table' and name=?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, tbl_name, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    bool found;
    if (rc == SQLITE_ROW)
        found = true;
    else if (rc == SQLITE_DONE)
        found = false;
    else {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return found;
}


int sqliteCheckFutSubj(void)
{
    sqlite3_stmt *stmt;
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "SELECT count(*) FROM verbforms WHERE tense=2 AND mood=1;");
    
    int rc = sqlite3_prepare_v2(db, sqlitePrepquery, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int (stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    else
    {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
}

int sqliteTableCount(char *tbl_name)
{
    sqlite3_stmt *stmt;
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "SELECT count(*) FROM %s;", tbl_name);

    int rc = sqlite3_prepare_v2(db, sqlitePrepquery, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int (stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    else
    {
        DEBUG_PRINT("error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
}

bool setupVerbFormsTable(void)
{
    int seqNum = 0;
    int bufferLen = 1024;
    char buffer[bufferLen];
    VerbFormD vf;
    int formid = 1;
    char *zErrMsg = 0;

    char *create = "BEGIN; DROP TABLE IF EXISTS verbforms; " \
    "CREATE TABLE IF NOT EXISTS verbforms (" \
    "formid INTEGER PRIMARY KEY NOT NULL, " \
    "lastSeen INTEGER NOT NULL, " \
    "difficulty INT1 NOT NULL, " \
    "defaultDifficulty INT1 NOT NULL, " \
    "person INT1 NOT NULL, " \
    "number INT1 NOT NULL, " \
    "tense INT1 NOT NULL, " \
    "voice INT1 NOT NULL, " \
    "mood INT1 NOT NULL, " \
    "verbid INT NOT NULL " \
    "); " \
    "CREATE INDEX verbididx ON verbforms(verbid); " \
    "CREATE INDEX lastSeenidx ON verbforms(lastSeen); ";
    //"UPDATE games SET score = -1, lives = -1 WHERE gameid == 1;";

    int rc = sqlite3_exec(db, create, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("Create verbforms table SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "INSERT INTO verbforms  (lastSeen,difficulty,defaultDifficulty,person,number,tense,voice,mood,verbid) VALUES (cast(strftime('%s','now') as int), ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);", -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        DEBUG_PRINT("ERROR preparing query: %s\n", sqlite3_errmsg(db));
        return false;
    }

    for (int vrb = 0; vrb < NUM_VERBS; vrb++)
    {
        vf.verbid = vrb;
        for (int t = 0; t < NUM_TENSES; t++)
        {
            vf.tense = t;
            for (int v = 0; v < NUM_VOICES; v++)
            {
                vf.voice = v;
                for (int m = 0; m < NUM_MOODS; m++)
                {
                    vf.mood = m;
                    for (int n = 0; n < NUM_NUMBERS; n++)
                    {
                        vf.number = n;
                        for (int p = 0; p < NUM_PERSONS; p++)
                        {
                            vf.person = p;

                            //fprintf(stderr, "here: Building seq #: %d, %d, %d, %d, %d, %d, %d\n", c++, vf.person, vf.number, vf.tense, vf.voice, vf.mood, vf.verbid);
                            if (getForm2(&vf, buffer, bufferLen, true, false) && strlen(buffer) > 0)
                            {
                                insertDB(formid, &vf, stmt);
                                formid = formid + 1;
                                /*
                                 copyVFD(&vf, &vseq[seqNum);
                                 */
                                //fprintf(stderr, "Building seq #: %d, p%d, n%d, t%d, v%d, m%d, verbid%d, %s\n", seqNum, vseq[seqNum].person, vseq[seqNum].number, vseq[seqNum].tense, vseq[seqNum].voice, vseq[seqNum].mood, vseq[seqNum].verbid, buffer);
                                seqNum++;
                            }
                        }
                    }
                }
            }
        }
    }
    rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("COMMIT verbforms table SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        DEBUG_PRINT("done creating verbforms table\n");
    }
    sqlite3_finalize(stmt);

    return true;
}

bool setHeadAnswer(VerbFormD *vf, bool correct, char *givenAnswer, const char *elapsedTime, VerbSeqOptions *vso)
{
    if (db) {
        if (vf->verbid < 0) {
            return false;
        }
        //changd this to only use lastformid
        snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN,
                 "INSERT INTO verbseq (id,answerTimestamp,gameid,person,number,tense,voice,mood,verbid,correct,elapsedTime,answerGiven) VALUES (NULL,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,'%s','%s');",
                 time(NULL), vso->gameId, vf->person, vf->number, vf->tense, vf->voice, vf->mood,
                 vf->verbid, correct, elapsedTime, givenAnswer);
        char *zErrMsg = 0;
        int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            DEBUG_PRINT("SQL1 error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        DEBUG_PRINT("Insert into gameid: %ld\n", vso->gameId);

        if (correct)
        {
            //https://stackoverflow.com/questions/11556546/sqlite-storing-default-timestamp-as-unixepoch
            snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN,
                     " UPDATE verbforms SET lastSeen=cast(strftime('%%s','now') as int) WHERE formid=%d;", vso->lastFormID);
            //char *zErrMsg = 0;
            //DEBUG_PRINT("SQL2 aaa: %s\n", sqlitePrepquery);
            rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
            if (rc != SQLITE_OK) {
                DEBUG_PRINT("SQL2 error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else
            {
                DEBUG_PRINT("HEAD UPDATED, %d", vso->lastFormID);
            }
        }
    }
    return true;
}

void addNewGameToDB(VerbSeqOptions *vs, int topUnit, long *gameid, bool isGame)
{
    char *zErrMsg = 0;

    int initialScore = 0;
    if (!isGame)
    {
        initialScore = -1; //we identify practice by -1 score
    }

    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "INSERT INTO games (timest,score,topUnit,lives) VALUES (%li,%d,%d,3);", time(NULL), initialScore, topUnit);
    int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        *gameid = sqlite3_last_insert_rowid(db);
        DEBUG_PRINT("new id: %ld", *gameid);
        vs->gameId = *gameid;
        //add first starting form
        setHeadAnswer(&vs->givenForm, true, "START", NULL, vs);
    }
}

void updateGameScore(long gameid, int score, int lives)
{
    DEBUG_PRINT("sqlite: gameid: %ld, score: %d, lives: %d\n", gameid, score, lives);
    char *zErrMsg = 0;
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "UPDATE games SET score=%d,lives=%d WHERE gameid=%ld;", score, lives, gameid);
    int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void insertDB(int formid, VerbFormD *vf, sqlite3_stmt *stmt)
{
    //char *zErrMsg = 0;

    if (db)
    {
        //sqlite3_bind_int(stmt, 1, formid); /* 1 */
        //sqlite3_bind_int(stmt, 1, 0); /* 2 lastSeen */
        sqlite3_bind_int(stmt, 1, 0); /* 3 difficulty */
        sqlite3_bind_int(stmt, 2, 0); /* 3 default difficulty */
        sqlite3_bind_int(stmt, 3, vf->person); /* 4 */
        sqlite3_bind_int(stmt, 4, vf->number); /* 5 */
        sqlite3_bind_int(stmt, 5, vf->tense); /* 6 */
        sqlite3_bind_int(stmt, 6, vf->voice); /* 7 */
        sqlite3_bind_int(stmt, 7, vf->mood); /* 8 */
        sqlite3_bind_int(stmt, 8, vf->verbid); /* 9 */

        int rc = sqlite3_step(stmt);

        DEBUG_PRINT("insert %d, %d, %d, %d, %d, %d\n", vf->person, vf->number, vf->tense, vf->voice, vf->mood, vf->verbid);

        if (rc != SQLITE_DONE) {
            DEBUG_PRINT("ERROR inserting data: %d, %s\n", formid, sqlite3_errmsg(db));
            return;
        }
        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);
    }
}

//copy new db to documents directory with temp name
//copy info from old db into new db
//delete old db, rename new one to name of old db
int upgradedb(const char *fromPath, const char *toPath)
{
    sqlite3 *fromDB = NULL;
    sqlite3 *toDB = NULL;
    sqlite3_stmt *res1;
    sqlite3_stmt *res2;
    int rc = sqlite3_open(fromPath, &fromDB);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("Can't open previous database version: %s, %s\n", sqlite3_errmsg(fromDB), fromPath);
        sqlite3_close(fromDB);
        fromDB = NULL;
        return 91;
    }
    rc = sqlite3_open(toPath, &toDB);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINT("Can't open new database version: %s, %s\n", sqlite3_errmsg(toDB), toPath);
        sqlite3_close(toDB);
        toDB = NULL;
        return 92;
    }

    char *u1 = "INSERT INTO games (gameid,timest,score,topUnit,lives,gameState) VALUES (?1,?2,?3,?4,?5,-1);";
    rc = sqlite3_prepare_v2(toDB, u1, -1, &res2, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("Failed to prepare statement1: %s\n", sqlite3_errmsg(toDB));
        return 93;
    }
    
    char *q1 = "SELECT gameid,timest,score,topUnit,lives FROM games ORDER BY gameid;";
    rc = sqlite3_prepare_v2(fromDB, q1, -1, &res1, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("Failed to prepare statement2: %s\n", sqlite3_errmsg(fromDB));
        return 94;
    }
    
    while ( sqlite3_step(res1) == SQLITE_ROW )
    {
        int gameid = sqlite3_column_int(res1, 0);
        long long timest = sqlite3_column_int64(res1, 1);
        int score = sqlite3_column_int(res1, 2);
        int stopUnit = sqlite3_column_int(res1, 3);
        int lives = sqlite3_column_int(res1, 4);
        
        //set practice game to be identified as practice
        //by score and lives.
        //in first version practice game always had id of 1
        if (gameid == 1)
        {
            score = -1;
            lives = -1;
            timest = time(NULL);
        }
        
        sqlite3_bind_int(res2, 1, gameid);
        sqlite3_bind_int64(res2, 2, timest);
        sqlite3_bind_int(res2, 3, score);
        sqlite3_bind_int(res2, 4, stopUnit);
        sqlite3_bind_int(res2, 5, lives);
        
        if ( sqlite3_step(res2) != SQLITE_DONE )
        {
            DEBUG_PRINT("Couldn't insert game, %s\n", sqlite3_errmsg(toDB));
            break;
        }
        DEBUG_PRINT("Copy gameid: %d\n", gameid);
        sqlite3_clear_bindings(res2);
        sqlite3_reset(res2);

    }
    sqlite3_finalize(res1);
    sqlite3_finalize(res2);

    //timest has been renamed to answerTimestamp
    //incorrectAns has been renamed to answerGiven
    char *u2 = "INSERT INTO verbseq (id,answerTimestamp,gameid,person,number,tense,voice,mood,verbid,correct,elapsedtime,answerGiven) VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12);";
    rc = sqlite3_prepare_v2(toDB, u2, -1, &res2, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("Failed to prepare statement1: %s\n", sqlite3_errmsg(toDB));
        return 95;
    }
    
    char *q2 = "SELECT id,timest,gameid,person,number,tense,voice,mood,verbid,correct,elapsedtime,incorrectAns FROM verbseq ORDER BY id;";
    rc = sqlite3_prepare_v2(fromDB, q2, -1, &res1, NULL);
    if (rc != SQLITE_OK) {
        DEBUG_PRINT("Failed to prepare statement2: %s\n", sqlite3_errmsg(fromDB));
        return 96;
    }

    while ( sqlite3_step(res1) == SQLITE_ROW )
    {
        int id = sqlite3_column_int(res1, 0);
        int timest = sqlite3_column_int(res1, 1);
        int gameid = sqlite3_column_int(res1, 2);
        int person = sqlite3_column_int(res1, 3);
        int number = sqlite3_column_int(res1, 4);
        int tense = sqlite3_column_int(res1, 5);
        int voice = sqlite3_column_int(res1, 6);
        int mood = sqlite3_column_int(res1, 7);
        int verbid = sqlite3_column_int(res1, 8);
        int correct = sqlite3_column_int(res1, 9);
        const unsigned char *elapsedtime = sqlite3_column_text(res1, 10);
        const unsigned char *incorrectAns = sqlite3_column_text(res1, 11);
        
        sqlite3_bind_int(res2, 1, id);
        sqlite3_bind_int(res2, 2, timest);
        sqlite3_bind_int(res2, 3, gameid);
        sqlite3_bind_int(res2, 4, person);
        sqlite3_bind_int(res2, 5, number);
        sqlite3_bind_int(res2, 6, tense);
        sqlite3_bind_int(res2, 7, voice);
        sqlite3_bind_int(res2, 8, mood);
        sqlite3_bind_int(res2, 9, verbid);
        sqlite3_bind_int(res2, 10, correct);
        sqlite3_bind_text(res2, 11, elapsedtime, -1, SQLITE_STATIC);
        sqlite3_bind_text(res2, 12, incorrectAns, -1, SQLITE_STATIC);

        if ( sqlite3_step(res2) != SQLITE_DONE )
        {
            DEBUG_PRINT("Couldn't insert row, %s\n", sqlite3_errmsg(toDB));
            break;
        }
        DEBUG_PRINT("Copy move: %d\n", gameid);
        sqlite3_clear_bindings(res2);
        sqlite3_reset(res2);
        
    }
    sqlite3_finalize(res1);
    sqlite3_finalize(res2);
    
    sqlite3_close(toDB);
    sqlite3_close(fromDB);
    return 0; //0 for success
}
