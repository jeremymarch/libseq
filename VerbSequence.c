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
#include "utilities.h"
#include "VerbSequence.h"
#include "sqlite3.h"

#define MAX_RECENT_VF 30
#define HC_VERBS_PER_SET 4

#define SQLITEPREPQUERYLEN 1024

/*
 
 create table forms (
 pai1s
 pai2s
 pai3s
 pai1p
 pai2p
 pai3p
 
 */

//these are assigned to globalGameID.
//its practice, insipient, or 1-n = a real saved game

int nextVerbSeqCustom(VerbFormD *vf1, VerbFormD *vf2, VerbSeqOptions *vso);

//GLOBAL VARIABLES
DataFormat *hcdata = NULL;
size_t sizeInBytes = 0;

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

int recentCheckCount = 0;
bool inRecentVFArray(VerbFormC *vf)
{
    recentCheckCount++;
    if (recentCheckCount > 1000) //safety
    {
        numRecentVFArray = 0;
        recentCheckCount = 0;
    }
    for (int i = 0; i < numRecentVFArray; i++)
    {
        if (compareVF(vf, &recentVFArray[i]))
        {
            printf("Recent VF Array hit\n");
            return true;
        }
    }
    
    return false;
}

void addToRecentVFArray(VerbFormC *vf)
{
    if (numRecentVFArray < MAX_RECENT_VF)
        numRecentVFArray++;
    
    if (recentVFArrayHead == MAX_RECENT_VF - 1)
        recentVFArrayHead = 0;
    else
        recentVFArrayHead++;
    
    recentVFArray[recentVFArrayHead].person = vf->person;
    recentVFArray[recentVFArrayHead].number = vf->number;
    recentVFArray[recentVFArrayHead].tense = vf->tense;
    recentVFArray[recentVFArrayHead].voice = vf->voice;
    recentVFArray[recentVFArrayHead].mood = vf->mood;
    recentVFArray[recentVFArrayHead].verb = vf->verb;
}

VerbFormC lastVF;

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


void dataFileInit(const char* path)
{
    sizeInBytes = sizeof(DataFormat);//10*1024*1024; //10 mb
    
    printf("Data file in verbseq: %s\n", path);
    
    int fd = open(path, O_RDWR);
    
    struct stat st;
    
    fstat(fd, &st);
    size_t size = st.st_size;
    //printf("size: %zu\n", size);
    
    if (size < sizeInBytes) {
        off_t result = lseek(fd, sizeInBytes - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            printf("Error calling lseek() to 'stretch' the file");
            return;
        }
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            printf("Error writing last byte of the file");
            return;
        }
        
    }
    fstat(fd, &st);
    size = st.st_size;
    //printf("size: %zu\n", size);
    
    hcdata = mmap(NULL, sizeInBytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (hcdata == MAP_FAILED)
    {
        printf("MMap failed\n");
    } else {
        printf("MMap success\n");
    }
    
    close(fd); // we can close file now
}
/*
void VerbSeqInit(const char *path)
{
    resetVerbSeq();
    
    //dataFileInit(path);
    //dbInit(path); //moved to appDelegate
}
*/
void syncDataFile()
{
    if (hcdata)
    {
        msync(hcdata,sizeInBytes,MS_SYNC);
        printf("sync file\n");
    }
}

void closeDataFile()
{
    if (hcdata)
    {
        msync(hcdata,sizeInBytes,MS_SYNC);
        munmap(hcdata, sizeInBytes);
        printf("close file\n");
        hcdata = NULL;
    }
    if (db)
    {
        sqlite3_close(db);
    }
}

VerbFormRecord *getNextRecord()
{
    //VerbFormRecord *a = NULL;
    
    return &hcdata->vr[1];
}

VerbFormRecord *prevNextRecord()
{
    //VerbFormRecord *a = NULL;
    
    return &hcdata->vr[1];
}

void incrementHead()
{
    if (hcdata->head > 1000)
        hcdata->head = 0;
    else
        hcdata->head++;
}

void setHead(VerbFormC *vf)
{
    printf("sethead\n");
    //VerbFormRecord *a = NULL;
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

int nextVerbSeq2old(VerbFormD *vf1, VerbFormD *vf2, VerbSeqOptions *vso1)
{
    /*
    VerbSeqOptions vso;
    vso.degreesToChange = 2;
    vso.isHCGame = true;
    vso.numUnits = 1;
    vso.repsPerVerb = 4;
    vso.practiceVerbID = -1;
    vso.units[0] = 1;// = {1,2, 3, 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
    //vso.units[1] = 7;
    */
    VerbFormC vfc1;
    VerbFormC vfc2;
    
    vfc1.person = vf1->person;
    vfc1.number = vf1->number;
    vfc1.tense = vf1->tense;
    vfc1.voice = vf1->voice;
    vfc1.mood = vf1->mood;
    vfc1.verb = &verbs[vf1->verbid];
    
    vfc2.person = vf2->person;
    vfc2.number = vf2->number;
    vfc2.tense = vf2->tense;
    vfc2.voice = vf2->voice;
    vfc2.mood = vf2->mood;
    vfc2.verb = &verbs[vf2->verbid];
    
    fprintf(stderr, "HERE1: %d", vfc1.verb->verbid);
    
    int ret = nextVerbSeq(&vfc1, &vfc2, vso1);
    //int ret = nextVerbSeqCustom(&vfc1, &vfc2, vso1);
    
    fprintf(stderr, "HERE2: %d", vfc1.verb->verbid);
    
    vf1->person = vfc1.person;
    vf1->number = vfc1.number;
    vf1->tense = vfc1.tense;
    vf1->voice = vfc1.voice;
    vf1->mood = vfc1.mood;
    vf1->verbid = vfc1.verb->verbid;
    
    vf2->person = vfc2.person;
    vf2->number = vfc2.number;
    vf2->tense = vfc2.tense;
    vf2->voice = vfc2.voice;
    vf2->mood = vfc2.mood;
    vf2->verbid = vfc2.verb->verbid;
    
    return ret;
}

int nextVerbSeq2(VerbFormD *vf1, VerbFormD *vf2, VerbSeqOptions *vso1)
{
    //int ret = nextVerbSeqCustom(vf1, vf2, vso1);
    int ret = nextVerbSeq2old(vf1, vf2, vso1);

    return ret;
}

VerbFormD vseq[1000024];

#define NELEMS(x)  (sizeof(x) / sizeof(x[0])) //this doesn't work because not using whole array
/* arrange the N elements of ARRAY in random order.
 * Only effective if N is much smaller than RAND_MAX;
 * if this may not be the case, use a better random
 * number generator. */
/*
static void shuffle2(void *array, size_t n, size_t size) {
    char tmp[size];
    char *arr = array;
    size_t stride = size * sizeof(char);
    time_t t;
    srand((unsigned) time(&t));
    
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; ++i) {
            size_t rnd = (size_t) rand();
            size_t j = i + rnd / (RAND_MAX / (n - i) + 1);
            
            memmove(tmp, arr + j * stride, size);
            memmove(arr + j * stride, arr + i * stride, size);
            memmove(arr + i * stride, tmp, size);
        }
    }
}
*/
//https://stackoverflow.com/questions/6127503/shuffle-array-in-c
void shuffle4(VerbFormD *array, size_t n, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand48(usec);
    
    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (drand48()*(i+1));
            VerbFormD temp;
            memmove(&temp, &array[j], size);
            memmove(&array[j], &array[i], size);
            memmove(&array[i], &temp, size);
            //int t = array[j];
            //array[j] = array[i];
            //array[i] = t;
        }
    }
}
/*
void shuffle3(int *array, size_t n) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand48(usec);
    
    
    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (drand48()*(i+1));
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}
*/
/*
static void shuffle1(void *array, size_t n, size_t size) {
    // This if() is not needed functionally, but left per OP's style
    if (n > 1) {
        char *carray = array;
        void * aux;
        aux = malloc(size);
        size_t i;
        for (i = 1; i < n; ++i) {
            size_t j = rand() % (i + 1);
            j *= size;
            memcpy(aux, &carray[j], size);
            memcpy(&carray[j], &carray[i*size], size);
            memcpy(&carray[i*size], aux, size);
        }
        free(aux);
    }
}
*/
/*
#define Shuffle(A, B, C) shuffle(A, B, 0, C, sizeof(*A))
void * shuffle(void * array, int seed, int index, int len, size_t size);
void swap(void * num1, void * num2, size_t size);

void * shuffle(void * array, int seed, int index, int len, size_t size)
{
    int result;
    if(index == len)
        return array;
    srand(seed);
    result = rand();
    swap((array + index * size), (array + result%len * size), size);
    return shuffle(array, result, index + 1, len, size);
}

void swap(void * a, void * b, size_t size)
{
    void * temp = malloc(size);
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
    free(temp);
}
*/

void removeFromList(VerbFormD *list, int *listCount, int itemToRemove)
{
    for (int i = itemToRemove; i < *listCount - 1; i++)
    {
        list[i] = list[i+1];
        
    }
    *listCount -= 1;
}

void prepareSeq(VerbFormD *vseq, int *seqNum)
{
    int bufferCapacity = 1024;
    char buffer[bufferCapacity];
    int mpcount = 0;
    //weed out mid/passive forms that are the same
    for (int i = 0; i < *seqNum; i++)
    {
        if (getForm2(&vseq[i], buffer, bufferCapacity, true, false) && strlen(buffer) == 3 && !memcmp(&buffer[0], "—", 3))
        {
            fprintf(stderr, "remove: %s\n", buffer);
            removeFromList(vseq, seqNum, i);
            fprintf(stderr, "seqNum %d\n", *seqNum);
        }
        else if (vseq[i].tense != AORIST && vseq[i].tense != FUTURE && vseq[i].voice != ACTIVE)
        {
            VerbFormD temp;
            temp.person = vseq[i].person;
            temp.number = vseq[i].number;
            temp.tense = vseq[i].tense;
            temp.voice = vseq[i].voice;
            temp.mood = vseq[i].mood;
            temp.verbid = vseq[i].verbid;
            
            for (int j = i + 1; j < *seqNum - 1; j++)
            {
                if (temp.person == vseq[j].person && temp.number == vseq[j].number && temp.tense == vseq[j].tense && temp.mood == vseq[j].mood && vseq[j].voice != ACTIVE)
                {
                    getForm2(&temp, buffer, bufferCapacity, true, false);
                    fprintf(stderr, "%d form %d: %s, ", mpcount++, temp.voice, buffer);
                    
                    getForm2(&vseq[j], buffer, bufferCapacity, true, false);
                    fprintf(stderr, "remove %d: %s\n", vseq[j].voice, buffer);
                    removeFromList(vseq, seqNum, j);
                    fprintf(stderr, "seqNum %d\n", *seqNum);
                    break;
                }
            }
        }
        
    }
}
int seqNum = 0;
int removeAlreadySeen(int verbid)
{
    //search db to remove forms already seen and correct.
    //how far to look back?
    if (!db)
    {
        fprintf(stderr, "sqlite error0 : no db\n\n");
        return -1;
    }
    int bufferLen = 1024;
    char buffer[bufferLen];
    VerbFormD temp;
    temp.verbid = verbid;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT person,number,tense,voice,mood FROM verbseq WHERE verbid=? AND correct=0 ORDER BY id DESC LIMIT 100000";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite error2 : %s", sqlite3_errmsg(db));
        return -1;
    }
    rc = sqlite3_bind_int(stmt, 1, verbid);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite error2 : %s", sqlite3_errmsg(db));
        return -1;
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        temp.person = sqlite3_column_int (stmt, 0);
        temp.number = sqlite3_column_int (stmt, 1);
        temp.tense = sqlite3_column_int (stmt, 2);
        temp.voice = sqlite3_column_int (stmt, 3);
        temp.mood = sqlite3_column_int (stmt, 4);
        
        for (int i = 0; i < seqNum; i++)
        {
            if (stepsAway(&temp, &vseq[i]) == 0)
            {
                getForm2(&vseq[i], buffer, bufferLen, true, false);
                fprintf(stderr, "already seen: %d, %d, %d, %d, %d, %d, %d, %s\n", i, vseq[i].person, vseq[i].number, vseq[i].tense, vseq[i].voice, vseq[i].mood, vseq[i].verbid, buffer);
                removeFromList(vseq, &seqNum, i);
            }
        }
    }
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "sqlite error: %s", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return 0;
}

void sortByDegreeDiff()
{
    
}

bool buildSequence(VerbSeqOptions *vso)
{
    seqNum = 0;
    int bufferLen = 1024;
    char buffer[bufferLen];
    VerbFormD vf;

    for (int vrb = 0; vrb < vso->seqOptions.numVerbs; vrb++)
    {
        vf.verbid = vso->seqOptions.verbs[vrb];
        for (int t = 0; t < vso->seqOptions.numTense; t++)
        {
            vf.tense = vso->seqOptions.tenses[t];
            for (int v = 0; v < vso->seqOptions.numVoice; v++)
            {
                vf.voice = vso->seqOptions.voices[v];
                for (int m = 0; m < vso->seqOptions.numMood; m++)
                {
                    vf.mood = vso->seqOptions.moods[m];
                    for (int n = 0; n < vso->seqOptions.numNumbers; n++)
                    {
                        vf.number = vso->seqOptions.numbers[n];
                        for (int p = 0; p < vso->seqOptions.numPerson; p++)
                        {
                            vf.person = vso->seqOptions.persons[p];
                            
                            //fprintf(stderr, "here: Building seq #: %d, %d, %d, %d, %d, %d, %d\n", c++, vf.person, vf.number, vf.tense, vf.voice, vf.mood, vf.verbid);
                            if (getForm2(&vf, buffer, bufferLen, true, false) && strlen(buffer) > 0)
                            {
                                memmove(&vseq[seqNum], &vf, sizeof(vf));
                                /*
                                vseq[seqNum].person = vf.person;
                                vseq[seqNum].number = vf.number;
                                vseq[seqNum].tense = vf.tense;
                                vseq[seqNum].voice = vf.voice;
                                vseq[seqNum].mood = vf.mood;
                                vseq[seqNum].verbid = vf.verbid;
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
    
    //fprintf(stderr, "\nshuffle\n\n");
    if (vso->shuffle)
    {
        for (int i = 0; i < 5; i++) //4 times
        {
            shuffle4(vseq, seqNum, sizeof(vseq[0]));
        }
    }
    
    prepareSeq(vseq, &seqNum); //after shuffle, so mid/pass pruning is random
    removeAlreadySeen(vseq[0].verbid);
    sortByDegreeDiff();
    for (int i = 0; i < seqNum; i++)
    {
        int steps = 0;
        if (i > 0)
        {
            steps = stepsAway(&vseq[i], &vseq[i - 1]);
        }
        getForm2(&vseq[i], buffer, bufferLen, true, false);
        fprintf(stderr, "After shuffle: %d, %d, %d, %d, %d, %d, %d, %s (%d)\n", i, vseq[i].person, vseq[i].number, vseq[i].tense, vseq[i].voice, vseq[i].mood, vseq[i].verbid, buffer, steps);
    }
    
    fprintf(stderr, "\n\n");
    /*
    for (int i = 0; i < seqNum; i++)
    {
        int steps = 0;
        if (i > 0)
        {
            steps = stepsAway(&vseq[i], &vseq[i - 1]);
        }
        if (steps == 2 || i == 0)
        {
            getForm2(&vseq[i], buffer, bufferLen, true, false);
            fprintf(stderr, "After shuffle: %d, p%d, n%d, t%d, v%d, m%d, vrb%d, %s (%d)\n", i, vseq[i].person, vseq[i].number, vseq[i].tense, vseq[i].voice, vseq[i].mood, vseq[i].verbid, buffer, steps);
        }
    }
    */
    return true;
}

int nextVerbSeqCustom(VerbFormD *vf1, VerbFormD *vf2, VerbSeqOptions *vso)
{
    //char buffer[1024];
    //int len = 1024;
    currentVerb = 0;
    if (vf1->verbid < 0) //start at -1
    {
        vf1->person = vseq[currentVerb].person;
        vf1->number = vseq[currentVerb].number;
        vf1->tense = vseq[currentVerb].tense;
        vf1->voice = vseq[currentVerb].voice;
        vf1->mood = vseq[currentVerb].mood;
        vf1->verbid = vseq[currentVerb].verbid;
        removeFromList(vseq, &seqNum, currentVerb);
    }

    //getAbbrevDescription(vf1, buffer, len);
    //fprintf(stderr, "current verb A: %d, person: %d, %s, %d\n", currentVerb, vf1->person,  buffer, vf1->verb->verbid);
    
    while (currentVerb < seqNum && stepsAway(vf1, &vseq[currentVerb]) != 2)
    {
        currentVerb++;
    }
    
    vf2->person = vseq[currentVerb].person;
    vf2->number = vseq[currentVerb].number;
    vf2->tense = vseq[currentVerb].tense;
    vf2->voice = vseq[currentVerb].voice;
    vf2->mood = vseq[currentVerb].mood;
    vf2->verbid = vseq[currentVerb].verbid;
    removeFromList(vseq, &seqNum, currentVerb);
    
    fprintf(stderr, "steps: %d, seq count: %d\n", stepsAway(vf1, vf2), seqNum);
    
    
    //vf2 = &vseq[currentVerb];
    //getAbbrevDescription(vf2, buffer, len);
    //fprintf(stderr, "current verb B: %d, person: %d, %s, %d\n", currentVerb, vf2->person,  buffer, vf2->verb->verbid);
    //fprintf(stderr, "current verb C\n");
    return 1;
}

bool once = true;
long lastInitialDegreesToChange = 0;
int nextVerbSeq(VerbFormC *vf1, VerbFormC *vf2, VerbSeqOptions *vso)
{
    fprintf(stderr, "GET FORM");
    static Verb *v;
    static Verb *lastV = NULL;
    
    int bufferLen = 2048;
    char buffer[bufferLen];
    int degreesToChange = vso->degreesToChange;
    
    if (!vso->isHCGame)
    {
        vso->gameId = GAME_PRACTICE;
        vso->score = -1;
    }
    else
    {
        //create new game or use current game
        if (vso->firstVerbSeq)
        {
            vso->gameId = GAME_INSIPIENT; //It will be save to db when first question is answered
            vso->firstVerbSeq = false;
        }
    }
    if (vso->practiceVerbID > -1)
    {
        v = &verbs[vso->practiceVerbID];
        fprintf(stderr, "verbid: %i, %i\n", vso->practiceVerbID, v->verbid);
    }
    else if (vso->isHCGame)
    {
        fprintf(stderr, "IS GAME");
        if (!vso->lastAnswerCorrect || vso->verbSeq >= HC_VERBS_PER_SET)
        {
            do //so we don't ask the same verb twice in a row
            {
                //for the game we can select any verb under and including highest unit selected.
                v = getRandomVerbFromUnit(vso->units, vso->numUnits);
                //v = getRandomVerb(vso->units, vso->numUnits);
            } while (v == lastV);
            lastV = v;
            
            vso->verbSeq = 1;
        }
        else
        {
            vso->verbSeq++;
        }
    }
    else
    {
        if (vso->verbSeq >= vso->repsPerVerb)
        {
            
            do //so we don't ask the same verb twice in a row
            {
                v = getRandomVerb(vso->units, vso->numUnits);
            } while (v == lastV);
            lastV = v;
            
            vso->verbSeq = 1;
        }
        else
        {
            vso->verbSeq++;
        }
    }

    /*
    if (v == NULL)
    {
        v = &verbs[1];
    }
    */
    vf1->verb = v; //THIS IS THE VERB WE'E USING
    //***************OVERRIDE for testing on specific verbs, set here*******************************
    //vf1->verb = &verbs[46];//46];//13]; //46 kathisthmi is longest
    //***************for testing on specific verbs*****************************************
    
    int highestUnit = 0;
    for (int i = 0; i < vso->numUnits; i++)
    {
        if (vso->units[i] > highestUnit)
            highestUnit = vso->units[i];
    }
    
    //only change 1 degree for units 1 and 2
    if (highestUnit <= 2)
        degreesToChange = 1;

    if (vso->startOnFirstSing && vso->verbSeq == 1)
    {
        vf1->person = FIRST;
        vf1->number = SINGULAR;
        vf1->tense = PRESENT;
        vf1->voice = ACTIVE;
        vf1->mood = INDICATIVE;
        
        //doesn't work if verb is deponent
        if (!getForm(vf1, buffer, bufferLen, false, false))
        {
            vf1->voice = MIDDLE;
            getForm(vf1, buffer, bufferLen, false, false); //do we need this?
        }
        addToRecentVFArray(vf1);
    }
    else if (vso->verbSeq == 1)
    {
         do
         {
             generateForm(vf1);
         
         } while (!getForm(vf1, buffer, bufferLen, false, false) || !isValidFormForUnit(vf1, highestUnit) || !strncmp(buffer, "—", 1)|| !strncmp(buffer, "-", 1)/*hyphen*/);
        
        addToRecentVFArray(vf1);
    }
    else
    {
        vf1->person = lastVF.person;
        vf1->number = lastVF.number;
        vf1->tense = lastVF.tense;
        vf1->voice = lastVF.voice;
        vf1->mood = lastVF.mood;
/*        if (lastVF.verb != NULL)
            vf1->verb = lastVF.verb;
        else
            vf1->verb = &verbs[1];
    */
        //we assume this is valid since it was the resulting form from last seq.
        //getForm(vf1, buffer, bufferLen, false, false);
    }

  /*
    if (vf1->verb == NULL)
    {
        vf1->verb = &verbs[1];
    }
*/
    do
    {
        if (vso->verbSeq == 1)
        {
            int limit = 1000;
            do
            {
                /*
                if (highestUnit <= 2)
                    degreesToChange = 1;
                else
                    degreesToChange = randWithMax(4) + 2; //2-5
                */
                limit--;
                
            } while (degreesToChange == lastInitialDegreesToChange && limit > 0); //for variety
            /*
            if (limit == 0)
            {
                degreesToChange = 2;
            }
            
            lastInitialDegreesToChange = degreesToChange;
             */
        }
        
        //these need to be in the loop, so we're always starting from the same place
        vf2->person = vf1->person;
        vf2->number = vf1->number;
        vf2->tense = vf1->tense;
        vf2->voice = vf1->voice;
        vf2->mood = vf1->mood;
        vf2->verb = vf1->verb;
        
        
        changeFormByDegrees(vf2, degreesToChange);
    } while (!getForm(vf2, buffer, bufferLen, true, false) || !isValidFormForUnit(vf2, highestUnit) || !strncmp(buffer, "—", 1)/*dash*/ || !strncmp(buffer, "-", 1)/*hyphen*/ || inRecentVFArray(vf2));

    /*
     // **************for testing to force form****************************
    vf1->person = THIRD;
    vf1->number = PLURAL;
    vf1->tense = AORIST;
    vf1->voice = ACTIVE;
    vf1->mood = OPTATIVE;
    
    vf2->person = SECOND;
     vf2->number = SINGULAR;
    vf2->tense = PRESENT;
     vf2->voice = MIDDLE;
     vf2->mood = INDICATIVE;
     vf2->verb = vf1->verb;
     // **************for testing to force form****************************
    */
    
    lastVF.person = vf2->person;
    lastVF.number = vf2->number;
    lastVF.tense = vf2->tense;
    lastVF.voice = vf2->voice;
    lastVF.mood = vf2->mood;
    lastVF.verb = vf2->verb;
    
    addToRecentVFArray(vf2);
    
    //temp
    if(vso->verbSeq == 2 && vso->askPrincipalParts && !vso->isHCGame)
    {
        return VERB_SEQ_PP;
    }
    
    //setHead(vf2); //we set it here, add whether it is correct later
    //fprintf(stderr, "verbid2: %i, %i\n", vf1->verb->verbid, vf2->verb->verbid);
    if (vso->verbSeq == 1)
        return VERB_SEQ_CHANGE_NEW;
    else
        return VERB_SEQ_CHANGE;
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

//unit is the highest unit we're up to
bool isValidFormForUnit(VerbFormC *vf, int unit)
{
    if (unit <= 2)
    {
        if (vf->tense == PERFECT || vf->tense == PLUPERFECT || vf->voice != ACTIVE || vf->mood != INDICATIVE)
            return false;
    }
    else if (unit <= 4)
    {
        if (vf->voice != ACTIVE || vf->mood == IMPERATIVE)
            return false;
    }
    else if (unit <= 5)
    {
        if (vf->voice == MIDDLE || vf->mood == IMPERATIVE)
            return false;
    }
    else if (unit <= 7)
    {
        if (vf->mood == IMPERATIVE)
            return false;
    }
    else if (unit <= 11)
    {
        return true;
    }
    else if (unit <= 12)
    {
        if ((utf8HasSuffix(vf->verb->present, "μι") && vf->tense == AORIST) || (utf8HasSuffix(vf->verb->present, "στημι") && (vf->tense == AORIST || vf->tense == PERFECT || vf->tense == PLUPERFECT)))
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
    }
    /*
     rc = sqlite3_exec(db, "INSERT INTO abc VALUES (1);", NULL, NULL, &zErrMsg);
     if( rc!=SQLITE_OK ){
     fprintf(stderr, "SQL error: %s\n", zErrMsg);
     sqlite3_free(zErrMsg);
     }
     */
    
    /*
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "INSERT INTO verbseq (timest,gameid,person,number,tense,voice,mood,verbid,correct,incorrectAns) VALUES (?,?,?,?,?,?,?,?,?,?)");
    if (sqlite3_prepare_v2(db, sqlitePrepquery, strlen(sqlitePrepquery), &statement, NULL) != SQLITE_OK)
    {
        printf("\nCould not prepare statement1.\n");
        return false;
    }
    snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "UPDATE verbseq SET correct=?, elapsedtime=?, incorrectAns=? WHERE id=?;");
    if (sqlite3_prepare_v2(db, sqlitePrepquery, strlen(sqlitePrepquery), &statement2, NULL) != SQLITE_OK)
    {
        printf("\nCould not prepare statement2.\n");
        return false;
    }
    */
    
    printf("sqlite success, version: %s\n", SQLITE_VERSION);
    
    return true;
}

bool setHeadAnswer(bool correct, char *givenAnswer, const char *elapsedTime, VerbSeqOptions *vso)
{
    /*
    if (0)//hcdata)
    {
        hcdata->vr[hcdata->head].person = lastVF.person;
        hcdata->vr[hcdata->head].number = lastVF.number;
        hcdata->vr[hcdata->head].tense = lastVF.tense;
        hcdata->vr[hcdata->head].voice = lastVF.voice;
        hcdata->vr[hcdata->head].mood = lastVF.mood;
        hcdata->vr[hcdata->head].correct = correct;
        hcdata->vr[hcdata->head].verb = findVerbIndexByPointer(lastVF.verb);
        hcdata->vr[hcdata->head].time = time(NULL);
        hcdata->vr[hcdata->head].correct = correct;
        unsigned long len = strlen(givenAnswer);
        strncpy(hcdata->vr[hcdata->head].answer, givenAnswer, (len > 199) ? 200 : len);
        
        //hcdata->vr[hcdata->head].answer = "222";
        incrementHead();

        
         //for (int i = 0; i < hcdata->head; i++)
         //{
         //printf("Rec: %s %s %s %s %s: %d %s, %s\n", persons[hcdata->vr[i].person], numbers[hcdata->vr[i].number], tenses[hcdata->vr[i].tense], voices[hcdata->vr[i].voice],moods[hcdata->vr[i].mood], hcdata->vr[i].correct, hcdata->vr[i].answer, asctime( localtime(&ltime) ));
         //}
        
    }
    */
    if (db)
    {
        snprintf(sqlitePrepquery, SQLITEPREPQUERYLEN, "INSERT INTO verbseq VALUES (NULL,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,'%s','%s');", time(NULL), vso->gameId, lastVF.person, lastVF.number, lastVF.tense, lastVF.voice, lastVF.mood, findVerbIndexByPointer(lastVF.verb), correct, elapsedTime, givenAnswer);
        char *zErrMsg = 0;
        int rc = sqlite3_exec(db, sqlitePrepquery, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
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

/*
 bool updateDB(VerbFormC *vf)
 {
 char *zErrMsg = 0;
 
 int verbIndex = findVerbIndexByPointer(vf->verb);
 if (verbIndex < 0)
 return false;
 
 //if (sqlite3_bind_int(statement, 1, wordid) != SQLITE_OK) //id
 //    return false;
 //long t = time(NULL);
 //printf("Time: %s", ctime(&t));
 if (sqlite3_bind_int64(statement, 1, time(NULL)) != SQLITE_OK) //time
 {
 printf("2\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 2, globalGameId) != SQLITE_OK) //gameid
 {
 printf("3\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 3, vf->person) != SQLITE_OK) //person
 {
 printf("4\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 4, vf->number) != SQLITE_OK) //num
 {
 printf("5\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 5, vf->tense) != SQLITE_OK) //tense
 {
 printf("6\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 6, vf->voice) != SQLITE_OK) //voice
 {
 printf("7\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 7, vf->mood) != SQLITE_OK) //mood
 {
 printf("8\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 8, verbIndex) != SQLITE_OK) //verbid
 {
 printf("9\n");
 return false;
 }
 if (sqlite3_bind_int(statement, 9, 0) != SQLITE_OK) //correct, set to incorrect for now
 {
 printf("10\n");
 return false;
 }
 
 //sqlite3_exec(db, "BEGIN", 0, 0, 0);
 
 if( sqlite3_step(statement) != SQLITE_DONE )
 {
 fprintf(stderr, "1SQL error: %s\nError: %s\n", zErrMsg, sqlite3_errmsg(db));
 sqlite3_free(zErrMsg);
 return false;
 }
 
 //sqlite3_exec(db, "COMMIT", 0, 0, 0);
 sqlite3_reset(statement);
 
 printf("updated db!\n");
 
 //char *err_msg = 0;
 //int rc = sqlite3_exec(db, "SELECT COUNT(*) FROM verbseq;", callback, 0, &err_msg);
 return true;
 }
 */


