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
#include "VerbSequence.h"
#include "sqlite3.h"

#define MAX_RECENT_VF 30
#define HC_VERBS_PER_SET 4

#define SQLITEPREPQUERYLEN 1024

//these are assigned to globalGameID.
//its practice, insipient, or 1-n = a real saved game

void copyVFD(VerbFormD *fromVF, VerbFormD *toVF);
void copyVFC(VerbFormC *fromVF, VerbFormC *toVF);
bool setupVerbFormsTable(void);
void insertDB(int formid, VerbFormD *vf, sqlite3_stmt *stmt);
bool sqliteTableExists(char *tbl_name);
int sqliteTableCount(char *tbl_name);

//GLOBAL VARIABLES
DataFormat *hcdata = NULL;
size_t sizeInBytes = 0;

VerbSeqOptionsNew opt; //global options

char sqlitePrepquery[SQLITEPREPQUERYLEN];
sqlite3_stmt *statement;
sqlite3_stmt *statement2;
sqlite3 *db;

int pointsPerForm = 1; //this is set in getRandomVerbFromUnit
int bonusPointsMultiple = 2;
int highestUnit = 1;

VerbFormC recentVFArray[MAX_RECENT_VF];
int numRecentVFArray = 0;
int recentVFArrayHead = -1;

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

VerbFormD lastVF;
int lastFormID = -1;

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
void addNewGameToDB(int topUnit, long *gameid);
void updateGameScore(long gameid, int score, int lives);
bool setHeadAnswer(bool correct, char *givenAnswer, const char *elapsedTime, VerbSeqOptions *vso);

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
        
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    printf("\n");
    
    return 0;
}


bool compareFormsCheckMFRecordResult(UCS2 *expected, int expectedLen, UCS2 *entered, int enteredLen, bool MFPressed, const char *elapsedTime, VerbSeqOptions *opt)
{
    char buffer[200];
    bool isCorrect = compareFormsCheckMF(expected, expectedLen, entered, enteredLen, MFPressed);
    
    ucs2_to_utf8_string(entered, enteredLen, (unsigned char*)buffer);
    
    if(opt->gameId == GAME_INSIPIENT)
    {
        long localGameId = GAME_INSIPIENT;
        addNewGameToDB(highestUnit, &localGameId);
        opt->gameId = localGameId;
    }
    
    if (opt->gameId > 1) //is a real game, not practice
    {
        if (opt->score < 0) //this should always be false
            opt->score = 0;
        
        if (isCorrect)
        {
            if (opt->verbSeq >= HC_VERBS_PER_SET)
                opt->score += (pointsPerForm * bonusPointsMultiple); //add bonus here
            else
                opt->score += pointsPerForm;
            fprintf(stderr, "SScore: %i\n", opt->score);
        }
        else
        {
            if (opt->gameId > -1)
                opt->lives -= 1;
            else
                opt->lives = -1;
        }
        
        updateGameScore(opt->gameId, opt->score, opt->lives);
    }

    opt->lastAnswerCorrect = isCorrect; //keep track of last answer here, so we don't need to rely on the db
    
    setHeadAnswer(isCorrect, buffer, elapsedTime, opt);

    return isCorrect;
}

int currentVerb = 0;
void resetVerbSeq(VerbSeqOptions *opt)
{
    opt->gameId = GAME_INVALID;
    opt->score = -1;
    opt->lives = 3;
    opt->lastAnswerCorrect = false;
    opt->verbSeq = 99999;
    opt->firstVerbSeq = true;
    //buildSequence(opt);
    currentVerb = 0;
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
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        vf1->person = 0;
        vf1->number = 0;
        vf1->tense = 0;
        vf1->voice = 0;
        vf1->mood = 0;
        //sqlite3_finalize(res); //not needed if prepare fails, though no harm
        return;
    }
    
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

int nextVerbSeqCustomDB(VerbFormD *vf1, VerbFormD *vf2)
{
    //char *err_msg = 0;
    sqlite3_stmt *res;
    
    char *sql = "SELECT person,number,tense,voice,mood,verbid,formid FROM verbforms WHERE verbid= ?1 ORDER BY lastSeen ASC, RANDOM();";
    int rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, vf1->verbid);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    
    if (vf1->person == 3)//unset
    {
        getLastSeen(vf1); //set first form to last seen
    }

    while ( sqlite3_step(res) == SQLITE_ROW )
    {
        vf2->person = sqlite3_column_int(res, 0);
        vf2->number = sqlite3_column_int(res, 1);
        vf2->tense = sqlite3_column_int(res, 2);
        vf2->voice = sqlite3_column_int(res, 3);
        vf2->mood = sqlite3_column_int(res, 4);
        vf2->verbid = sqlite3_column_int(res, 5);
        lastFormID = sqlite3_column_int(res, 6);
        
        if (stepsAway(vf1, vf2) == 2/*fix me: change to variable*/ && !mpToMp(vf1, vf2) && isValidFormForUnitD(vf2, opt.topUnit))
        {
            break;
        }
        fprintf(stderr, "top unit: %d\n", opt.topUnit);
        
    }
    sqlite3_finalize(res);
    return 1;
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
        printf("Hit mid to pass or pass to mid\n");
    
        return true;
    }
    else
    {
        return false;
    }
}
/*
void changeFormByDegrees(VerbFormC *vf, int degrees)
{
    unsigned char tempPerson;
    unsigned char tempNumber;
    unsigned char tempTense;
    unsigned char tempVoice;
    unsigned char tempMood;
    
    int components[degrees];
    
    do
    {
        tempPerson = vf->person;
        tempNumber = vf->number;
        tempTense = vf->tense;
        tempVoice = vf->voice;
        tempMood = vf->mood;
        
        //re-initialize components array to invalid values
        for (int i = 0; i < degrees; i++)
            components[i] = -1;
        
        for (int i = 0; i < degrees; i++)
        {
            int n = (int)randWithMax(5);
            bool notAlreadyIn = true;
            for (int j = 0; j < degrees; j++)
            {
                if (n == components[j])
                {
                    notAlreadyIn = false;
                    --i;
                    break;
                }
            }
            if (notAlreadyIn)
                components[i] = n;
        }
        
        for (int i = 0; i < degrees; i++)
        {
            int v = components[i];
            
            if (v == PERSON)
            {
                tempPerson = incrementValue(NUM_PERSONS, vf->person);
            }
            else if (v == NUMBER)
            {
                tempNumber = incrementValue(NUM_NUMBERS, vf->number);
            }
            else if (v == TENSE)
            {
                tempTense = incrementValue(NUM_TENSES, vf->tense);
            }
            else if (v == VOICE)
            {
                tempVoice = incrementValue(NUM_VOICES, vf->voice);
            }
            else if (v == MOOD)
            {
                tempMood = incrementValue(NUM_MOODS, vf->mood);
            }
        }
    } //make sure form is valid and this verb has the required principal part,
    //and make sure we're not changing from mid to pass or vice versa unless the tense is aorist or future
    while (!formIsValidReal(tempPerson, tempNumber, tempTense, tempVoice, tempMood) || getPrincipalPartForTense(vf->verb, tempTense, tempVoice)[0] == '\0' || isMidToPassOrPassToMid(vf, tempTense, tempVoice));
    
    vf->person = tempPerson;
    vf->number = tempNumber;
    vf->tense = tempTense;
    vf->voice = tempVoice;
    vf->mood = tempMood;
}
*/

//unit is the highest unit we're up to
bool isValidFormForUnitD(VerbFormD *vf, int unit)
{
    VerbFormC vfc;
    vfc.person = vf->person;
    vfc.number = vf->number;
    vfc.tense = vf->tense;
    vfc.voice = vf->voice;
    vfc.mood = vf->mood;
    vfc.verb = &verbs[vf->verbid];
    
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
    /*
     unsigned long
     // max <= RAND_MAX < ULONG_MAX, so this is okay.
     num_bins = (unsigned long) max + 1,
     num_rand = (unsigned long) RAND_MAX + 1,
     bin_size = num_rand / num_bins,
     defect   = num_rand % num_bins;
     
     long x;
     do {
     x = random();
     }
     // This is carefully written not to overflow
     while (num_rand - defect <= (unsigned long)x);
     
     // Truncated division is intentional
     return x/bin_size;
     */
}
/*
//problem if match and distractor have are alternate forms of each other.
void getDistractorsForChange(VerbFormC *orig, VerbFormC *new, int numDistractors, char *buffer)
{
    VerbFormC vf;
    int i = 0;
    int n = 0;
    int starts[numDistractors + 1];
    int numStarts = 0;
    for (i = 0; i < numDistractors + 1; i++)
    {
        starts[i] = 0;
    }
    
    i = 0;
    char tempBuffer[2048];
    int offset = 0;
    
    getForm(new, tempBuffer, 2048, false, false); //put the changed form on the buffer so no duplicates
    randomAlternative(tempBuffer, &offset);
    strncpy(&buffer[n], &tempBuffer[offset], strlen(&tempBuffer[offset]));
    n += strlen(&tempBuffer[offset]);
    strncpy(&buffer[n], "; ", 2);
    n += 2;
    
    numStarts++;
    
    do
    {
        vf.verb = new->verb;
        vf.person = new->person;
        vf.number = new->number;
        vf.tense = new->tense;
        vf.voice = new->voice;
        vf.mood = new->mood;
        
        changeFormByDegrees(&vf, 1);
        
        getForm(&vf, tempBuffer, 2048, false, false);
        offset = 0;
        randomAlternative(tempBuffer, &offset);
        
        int j = 0;
        int noMatches = 1;
        for (j = 0; j < numStarts; j++)
        {
            if (memcmp(&tempBuffer[offset], &buffer[ starts[j] ], strlen(&tempBuffer[offset])) == 0 || memcmp(&tempBuffer[offset], "—", 1) == 0)
            {
                //printf("HEREREREREEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
                noMatches = 0;
                break;
            }
        }
        
        if (noMatches == 1)
        {
            //reduce alternate forms to just one
            strncpy(&buffer[n], &tempBuffer[offset], strlen(&tempBuffer[offset]));
            starts[numStarts] = n;
            numStarts++;
            
            //printf("%s\n", tempBuffer);
            n += strlen(&tempBuffer[offset]);
            strncpy(&buffer[n], "; ", 2);
            n += 2;
            
            i++;
        }
    } while (i < numDistractors);
    buffer[n - 2] = '\0';
}
*/
void randomAlternative(char *s, int *offset)
{
    int starts[5] = { 0,0,0,0,0 };
    int numStarts = 1;
    unsigned long lenS = strlen(s);
    
    for (int i = 0; i < lenS; i++)
    {
        if (s[i] == ',')
        {
            starts[numStarts] = i + 2;
            numStarts++;
        }
    }
    long random = randWithMax(numStarts);
    *offset = starts[random];
    
    if (random < numStarts - 1)
        s[starts[random + 1] - 2] = '\0';
}

Verb *getRandomVerb(int *units, int numUnits)
{
    int u, v;
    int verbsToChooseFrom[NUM_VERBS];
    int numVerbsToChooseFrom = 0;
    for (v = 0; v < NUM_VERBS; v++)
    {
        for (u = 0; u < numUnits; u++)
        {
            if (verbs[v].hq == units[u])
            {
                verbsToChooseFrom[numVerbsToChooseFrom] = v;
                numVerbsToChooseFrom++;
                break;
            }
        }
    }
    int verb = (int)randWithMax(numVerbsToChooseFrom);
    return &verbs[ verbsToChooseFrom[verb] ];
}

Verb *getRandomVerbFromUnit(int *units, int numUnits)
{
    int u;
    highestUnit = 1;
    for (u = 0; u < numUnits; u++)
    {
        if (units[u] > highestUnit)
            highestUnit = units[u];
    }
    pointsPerForm = highestUnit;
    int v;
    int verbsToChooseFrom[NUM_VERBS];
    int numVerbsToChooseFrom = 0;
    for (v = 0; v < NUM_VERBS; v++)
    {
        if (verbs[v].hq <= highestUnit)
        {
            verbsToChooseFrom[numVerbsToChooseFrom] = v;
            numVerbsToChooseFrom++;
        }
    }
    int verb = (int)randWithMax(numVerbsToChooseFrom);
    return &verbs[ verbsToChooseFrom[verb] ];
}
/*
Ending *getRandomEnding(int *units, int numUnits)
{
    int u, e;
    int endingsToChooseFrom[NUM_ENDINGS];
    int numEndingsToChooseFrom = 0;
    for (e = 0; e < NUM_ENDINGS; e++)
    {
        for (u = 0; u < numUnits; u++)
        {
            if (endings[e].hq == units[u])
            {
                endingsToChooseFrom[numEndingsToChooseFrom] = e;
                numEndingsToChooseFrom++;
                break;
            }
        }
    }
    int ending = (int)randWithMax(numEndingsToChooseFrom);
    return &endings[ endingsToChooseFrom[ending] ];
}

void getRandomEndingAsString(int *units, int numUnits, char *buffer, int bufferLen)
{
    //char description[512];
    Ending *e = getRandomEnding(units, numUnits);
    
    //endingGetDescription(1, description, 512);
    
    snprintf(buffer, bufferLen, "%s; %s; %s; %s; %s; %s; %s", e->description, e->fs, e->ss, e->ts, e->fp, e->sp, e->tp);
}
*/
/***********************DB**********************/

bool dbInit(const char *path)
{
    unsigned long dbpathLen = strlen(path) + 12;
    char dbpath[dbpathLen];
    struct stat st;
    
    snprintf(dbpath, dbpathLen - 1, "%s", path);
    
    stat(dbpath, &st);
    off_t size = st.st_size;
    
    char *zErrMsg = 0;
    int rc = sqlite3_open(dbpath, &db);
    if( rc != SQLITE_OK )
    {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    else
    {
        printf("SQLite db open, path: %s, size: %lld\n", dbpath, size);
    }
    
    //char *check = "SELECT name FROM sqlite_master WHERE type='table' AND name='table_name'";
    //"DROP TABLE IF EXISTS games; DROP TABLE IF EXISTS verbseq;
    //DROP TABLE IF EXISTS verbforms;
    char *sql = "CREATE TABLE IF NOT EXISTS games (" \
    "gameid INTEGER PRIMARY KEY NOT NULL, " \
    "timest INT NOT NULL, " \
    "score INT NOT NULL, " \
    "topUnit INT NOT NULL, " \
    "lives INT NOT NULL " \
    "); " \
    
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
    "); " \
    
    "INSERT OR IGNORE INTO games VALUES (1,0,-1,0,0);"; //This is the Practice Game
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
 
    char *verbForms = "verbforms";
    bool exists = sqliteTableExists(verbForms);
    int vfcount = sqliteTableCount(verbForms);
    
    printf("exists %d, count %d\n", exists, vfcount);
    
    if ( !exists || vfcount < 1)
    {
        setupVerbFormsTable();
        printf("created verb forms table\n");
    }
    
    printf("sqlite success, version: %s\n", SQLITE_VERSION);
    
    return true;
}

//note the table name is case-sensitive here
bool sqliteTableExists(char *tbl_name)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM sqlite_master where type='table' and name=?";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("error: %s\n", sqlite3_errmsg(db));
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
        printf("error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

int sqliteTableCount(char *tbl_name)
{
    sqlite3_stmt *stmt;
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "SELECT count(*) FROM %s;", tbl_name);
    
    int rc = sqlite3_prepare_v2(db, sqlitePrepquery, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("error: %s\n", sqlite3_errmsg(db));
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
        printf("error: %s\n", sqlite3_errmsg(db));
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
    
    char *create = "DROP TABLE IF EXISTS verbforms; " \
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
    "); ";
    
    int rc = sqlite3_exec(db, create, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "Create verbforms table SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "INSERT INTO verbforms  (lastSeen,difficulty,defaultDifficulty,person,number,tense,voice,mood,verbid) VALUES (datetime('now'), ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);", -1, &stmt, NULL);
    
    if (rc != SQLITE_OK)
    {
        printf("ERROR preparing query: %s\n", sqlite3_errmsg(db));
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
    printf("done creating verbforms table\n");
    sqlite3_finalize(stmt);
    
    return true;
}

bool setHeadAnswer(bool correct, char *givenAnswer, const char *elapsedTime, VerbSeqOptions *vso)
{
    if (db)
    {
        int lastVerbIndex = lastVF.verbid;
        if (lastVerbIndex < 0)
        {
            return false;
        }
        snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "INSERT INTO verbseq VALUES (NULL,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,'%s','%s');", time(NULL), vso->gameId, lastVF.person, lastVF.number, lastVF.tense, lastVF.voice, lastVF.mood, lastVF.verbid, correct, elapsedTime, givenAnswer);
        char *zErrMsg = 0;
        int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL1 error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        
        snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, " UPDATE verbforms SET lastSeen=datetime('now') WHERE formid=%d;", lastFormID);
        //char *zErrMsg = 0;
        rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL2 error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        //printf("Insert: %d %s\n%s\n", rc, err,sqlitePrepquery);
    }
    return true;
}

void addNewGameToDB(int topUnit, long *gameid)
{
    char *zErrMsg = 0;
    
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "INSERT INTO games (timest,score,topUnit,lives) VALUES (%li,0, %d,3);", time(NULL), topUnit);
    int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        *gameid = sqlite3_last_insert_rowid(db);
    }
}

void updateGameScore(long gameid, int score, int lives)
{
    fprintf(stderr, "sqlite: gameid: %ld, score: %d, lives: %d\n", gameid, score, lives);
    char *zErrMsg = 0;
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "UPDATE games SET score=%d,lives=%d WHERE gameid=%ld;", score, lives, gameid);
    int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
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
        
        printf("insert %d, %d, %d, %d, %d, %d\n", vf->person, vf->number, vf->tense, vf->voice, vf->mood, vf->verbid);
        
        if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %d, %s\n", formid, sqlite3_errmsg(db));
            return;
        }
        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);
    }
}


