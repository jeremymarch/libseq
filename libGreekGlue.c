//Useful link: http://developer.android.com/training/articles/perf-jni.html

#ifdef __ANDROID__

#include <string.h>
#include <jni.h>
#include <android/log.h>
#include "utilities.h"
#include "libmorph.h"
#include "GreekForms.h"
#include "GreekUnicode.h"
#include "VerbSequence.h"
#include "accent.h"

#include <android/log.h>
#define  LOG_TAG    "your-log-tag"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
// If you want you can add other log definition for info, warning etc

extern VerbSeqOptionsNew opt;

JNIEXPORT jboolean JNICALL
          Java_com_philolog_hc_VerbSequence_VerbSeqInit( JNIEnv* env, jobject thiz, jstring path)
{
    const char *cpath = (*env)->GetStringUTFChars(env, path, 0);
    //VerbSeqInit(cpath);
    dbInit(cpath);
    LOGE("INIT dbpath: %s", cpath);
    (*env)->ReleaseStringUTFChars(env, path, cpath);
    return true;
}

JNIEXPORT int JNICALL
Java_com_philolog_hc_VerbSequence_setupUnits( JNIEnv* env, jobject thiz, jbooleanArray arr, jboolean isHCGame) {
    jboolean *ba;
    ba = (*env)->GetBooleanArrayElements(env, arr, NULL);
    int baLength = (*env)->GetArrayLength(env, arr);

    opt.numUnits = 0;
    int verbsLen = 0;
    bool atLeastOne = false;
    for (int i = 1; i < baLength; i++) { //i = 1 to skip unit 1
        if (ba[i] == JNI_TRUE) {
            opt.units[opt.numUnits] = i + 1;
            opt.numUnits++;
            addVerbsForUnit(i + 1, opt.verbs, &verbsLen, NUM_VERBS);
            atLeastOne = true;
        }
    }
    if (!atLeastOne) {
        opt.verbs[0] = 0;
        opt.verbs[1] = 1;
        opt.verbs[2] = 2;
        opt.verbs[3] = 3;
        opt.numVerbs = 4;
    } else {
        opt.numVerbs = verbsLen;
    }

    opt.repsPerVerb = 4;
/*
    for (int i = 0; i < opt.numVerbs; i++) {
        LOGE("%d, ", opt.verbs[i]);
    }
    LOGE("num: %d ", opt.numVerbs);
*/
    opt.isHCGame = isHCGame;
    resetVerbSeq(isHCGame);
    //opt.practiceVerbID = -1;

    (*env)->ReleaseBooleanArrayElements(env, arr, ba, 0);
    return 1;
}

JNIEXPORT int JNICALL
Java_com_philolog_hc_VerbSequence_nextVerbSeq( JNIEnv* env, jobject thiz, jobject gv1, jobject gv2 )
{
    VerbFormD vf1, vf2;
    int ret = 0;
    int seq = 0;
    jfieldID fid;
    jclass gvcls, verbcls;
    jobject verbObj;

    //get vf1
    gvcls = (*env)->GetObjectClass(env, gv1);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    vf1.person = (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    vf1.number = (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    vf1.tense = (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    vf1.voice = (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    vf1.mood = (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    vf1.verbid = (*env)->GetIntField(env, gv1 ,fid);

    //get vf2
    gvcls = (*env)->GetObjectClass(env, gv2);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    vf2.person = (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    vf2.number = (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    vf2.tense = (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    vf2.voice = (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    vf2.mood = (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    vf2.verbid = (*env)->GetIntField(env, gv2 ,fid);

    //ret = nextVerbSeq(&seq, &vf1, &vf2, &opt);
    ret = nextVerbSeqCustomDB( &vf1, &vf2 );

    //vf1
    gvcls = (*env)->GetObjectClass(env, gv1);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.person);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.number);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.tense);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.voice);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.mood);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    (*env)->SetIntField(env, gv1 ,fid, vf1.verbid);


    //vf2
    gvcls = (*env)->GetObjectClass(env, gv2);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.person);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.number);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.tense);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.voice);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.mood);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    (*env)->SetIntField(env, gv2 ,fid, vf2.verbid);

    //set state on vs
    gvcls = (*env)->GetObjectClass(env, thiz);
    fid = (*env)->GetFieldID(env,gvcls,"state","I");
    (*env)->SetIntField(env, thiz ,fid, ret);

    //set seq
    fid = (*env)->GetFieldID(env,gvcls,"seq","I");
    (*env)->SetIntField(env, thiz ,fid, 1);
    return ret;
}

JNIEXPORT void JNICALL
Java_com_philolog_hc_VerbSequence_resetVerbSeq( JNIEnv* env, jobject thiz )
{
    resetVerbSeq(opt.isHCGame);
}

//bool compareFormsCheckMFRecordResult(UCS2 *expected, int expectedLen, UCS2 *given, int givenLen, bool MFPressed, char *elapsedTime, int *score)

JNIEXPORT jboolean JNICALL
Java_com_philolog_hc_GreekVerb_compareFormsCheckMFRecordResult( JNIEnv* env, jobject thiz, jstring given, jstring expected, jstring elapsed, jboolean MFPressed)
{
    //http://stackoverflow.com/questions/4181934/jni-converting-jstring-to-char
    const char *givenChar = (*env)->GetStringUTFChars(env, given, 0);
    const char *expectedChar = (*env)->GetStringUTFChars(env, expected, 0);
    const char *elapsedChar = (*env)->GetStringUTFChars(env, elapsed, 0);
    bool MFP = (MFPressed) ? true : false;

    UCS2 givenucs2[1024];
    int givenucs2Len = 0;
    UCS2 expecteducs2[1024];
    int expecteducs2Len = 0;

    utf8_to_ucs2_string((const unsigned char *)givenChar, givenucs2, &givenucs2Len);
    utf8_to_ucs2_string((const unsigned char *)expectedChar, expecteducs2, &expecteducs2Len);

    char elapsedTime[100];
    strncpy(elapsedTime, elapsedChar, 99);
    int score = 0;
    int lives = 0;

    jfieldID fid;
    jclass cls;
    cls = (*env)->GetObjectClass(env, thiz);
    fid = (*env)->GetFieldID(env,cls,"score","I");
    score = (*env)->GetIntField(env, thiz ,fid);

    LOGE("Score before: %d", opt.score);

    bool ret = compareFormsCheckMFRecordResult(expecteducs2, expecteducs2Len, givenucs2, givenucs2Len, MFP, elapsedTime, &opt);//, &score, &lives);

    LOGE("Score after: %d", opt.score);

    (*env)->SetIntField(env, thiz ,fid, opt.score);

    (*env)->ReleaseStringUTFChars(env, elapsed, elapsedChar);
    (*env)->ReleaseStringUTFChars(env, given, givenChar);
    (*env)->ReleaseStringUTFChars(env, expected, expectedChar);
    return (ret) ? true : false;
}

JNIEXPORT jstring JNICALL
Java_com_philolog_hc_GreekVerb_getForm( JNIEnv* env, jobject thiz, jint mf, jint decompose )
{
    VerbFormD vf;
    jfieldID fid;
    jclass cls;
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"person","I");
    vf.person = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    vf.number = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    vf.tense = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    vf.voice = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    vf.mood = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"verbid","I");
    vf.verbid = (*env)->GetIntField(env, thiz ,fid);

    //__android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "Here 2");

/*
    vf.verb = &verbs[45];
    vf.person = FIRST;
    vf.number = SINGULAR;
    vf.tense = IMPERFECT;
    vf.voice = ACTIVE;
    vf.mood = INDICATIVE;
*/
    int bufferLen = 2048;
    char buffer[bufferLen];
    //long units[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    //Verb *v = getRandomVerb(units, 11);

    //vf.verb = v;//&verbs[15];
    //do {
        //generateForm(&vf); }
     //while (!
    //__android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "Here 3 %d %d %d %d %d %d", vf.person, vf.number, vf.tense, vf.voice, vf.mood, vf.verb->verbid);

    if (!getForm2(&vf, buffer, bufferLen, mf, decompose)) //don't look here because will get stuck for deponents e.g.
    {
        LOGE("AAA Returned 0 %d", bufferLen);
        return (*env)->NewStringUTF(env, buffer);
    }
//__android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "Here 4");
    //__android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "The value of n is %s", buffer);

    //stpcpy(buffer, "ψψᾱ́ψψ"); //this works, so not a problem with conversion to utf16

    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT jstring JNICALL
Java_com_philolog_hc_GreekVerb_getAbbrevDescription( JNIEnv* env, jobject thiz )
{
    VerbFormC vf;
    jfieldID fid;
    jclass cls, cls2;
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"person","I");
    vf.person = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    vf.number = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    vf.tense = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    vf.voice = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    vf.mood = (*env)->GetIntField(env, thiz ,fid);

    jobject verbObj;
    fid = (*env)->GetFieldID(env,cls,"verb","Lcom/philolog/hc/Verb;");
    verbObj = (*env)->GetObjectField(env, thiz ,fid);
    cls2 = (*env)->GetObjectClass(env, verbObj);
    fid = (*env)->GetFieldID(env,cls2,"verbId","I");
    jint verbid = (*env)->GetIntField(env, verbObj ,fid);
    vf.verb = &verbs[verbid];

    int bufferLen = 2048;
    char buffer[bufferLen];

    getAbbrevDescription(&vf, buffer, bufferLen);

    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT void JNICALL
Java_com_philolog_hc_GreekVerb_generateForm( JNIEnv* env, jobject thiz )
{
    jfieldID fid;
    jclass cls, cls2;
    VerbFormC vf;
    jobject verbObj;

    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"verb","Lcom/philolog/hc/Verb;");
    verbObj = (*env)->GetObjectField(env, thiz ,fid);

    cls2 = (*env)->GetObjectClass(env, verbObj);
    fid = (*env)->GetFieldID(env,cls2,"verbId","I");
    jint verbid = (*env)->GetIntField(env, verbObj ,fid);

    vf.verb = &verbs[verbid];

    generateForm(&vf);

    fid = (*env)->GetFieldID(env,cls,"person","I");
    (*env)->SetIntField(env, thiz ,fid, vf.person);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    (*env)->SetIntField(env, thiz ,fid, vf.number);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    (*env)->SetIntField(env, thiz ,fid, vf.tense);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    (*env)->SetIntField(env, thiz ,fid, vf.voice);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    (*env)->SetIntField(env, thiz ,fid, vf.mood);
}

/*
JNIEXPORT void JNICALL
Java_com_philolog_hc_GreekVerb_changeFormByDegrees( JNIEnv* env, jobject thiz, jint degrees )
{
    VerbFormC vf;
    jfieldID fid;
    jclass cls, cls2;
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"person","I");
    vf.person = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    vf.number = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    vf.tense = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    vf.voice = (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    vf.mood = (*env)->GetIntField(env, thiz ,fid);

    //FIX ME, or is this right?
    jobject verbObj;
    fid = (*env)->GetFieldID(env,cls,"verb","Lcom/philolog/hc/Verb;");
    verbObj = (*env)->GetObjectField(env, thiz ,fid);
    cls2 = (*env)->GetObjectClass(env, verbObj);
    fid = (*env)->GetFieldID(env,cls2,"verbId","I");
    jint verbid = (*env)->GetIntField(env, verbObj ,fid);
    vf.verb = &verbs[verbid];

    changeFormByDegrees(&vf, degrees);

    fid = (*env)->GetFieldID(env, cls, "person", "I");
    (*env)->SetIntField(env, thiz, fid, vf.person);

    fid = (*env)->GetFieldID(env, cls, "number", "I");
    (*env)->SetIntField(env, thiz, fid, vf.number);

    fid = (*env)->GetFieldID(env, cls, "tense", "I");
    (*env)->SetIntField(env, thiz, fid, vf.tense);

    fid = (*env)->GetFieldID(env, cls, "voice", "I");
    (*env)->SetIntField(env, thiz, fid, vf.voice);
}
*/
JNIEXPORT void JNICALL
Java_com_philolog_hc_Verb_getVerb( JNIEnv* env, jobject thiz, jint verbId )
{
    Verb *v = &verbs[verbId];

    jfieldID fid;
    jclass cls;
    jstring str;

    //https://www3.ntu.edu.sg/home/ehchua/programming/java/JavaNativeInterface.html

    //http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/field.html
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"verbId","I");
    (*env)->SetIntField(env, thiz ,fid, verbId);

    fid = (*env)->GetFieldID(env,cls,"present","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->present);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"future","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->future);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"aorist","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->aorist);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"perfect","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->perf);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"perfmid","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->perfmid);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"aoristpass","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->aoristpass);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);
}

JNIEXPORT int JNICALL
Java_com_philolog_hc_Verb_deponentType( JNIEnv* env, jobject thiz ) {
    jfieldID fid;
    jclass cls;

    cls = (*env)->GetObjectClass(env, thiz);
    fid = (*env)->GetFieldID(env,cls,"verbId","I");
    int verbId = (*env)->GetIntField(env, thiz ,fid);

    Verb *v = &verbs[verbId];
    return deponentType(v);
}
/*
JNIEXPORT void JNICALL
Java_com_philolog_hc_Verb_getRandomVerb( JNIEnv* env, jobject thiz )
{
    long units[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    Verb *v = getRandomVerb(units, 13);

    jfieldID fid;
    jclass cls;
    jstring str;

    //http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/field.html
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"verbId","I");
    (*env)->SetIntField(env, thiz ,fid, v->verbid);

    fid = (*env)->GetFieldID(env,cls,"present","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->present);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"future","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->future);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"aorist","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->aorist);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"perfect","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->perf);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"perfmid","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->perfmid);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);

    fid = (*env)->GetFieldID(env,cls,"aoristpass","Ljava/lang/String;");
    str = (*env)->NewStringUTF(env, v->aoristpass);
    if (NULL == str)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "MyApp", "null string");
        return;
    }
    (*env)->SetObjectField(env, thiz ,fid, str);
}
*/
JNIEXPORT jstring JNICALL
Java_com_philolog_hc_GreekVerb_addAccent( JNIEnv* env, jobject thiz, jint accent, jstring *str) {
    char buffer[1024];
    UCS2 ucs2[1024];
    int ucs2Len = 0;

    const char *letters = (*env)->GetStringUTFChars(env, str, NULL);
    utf8_to_ucs2_string((const unsigned char *)letters, ucs2, &ucs2Len);

    if (ucs2[0] != COMBINING_ACUTE && ucs2[0] != COMBINING_MACRON && ucs2[0] != COMBINING_ROUGH_BREATHING && ucs2[0] != COMBINING_SMOOTH_BREATHING) {
        accentSyllable(ucs2, 0, &ucs2Len, accent, true, PRECOMPOSED_HC_MODE);
        ucs2_to_utf8_string(ucs2, ucs2Len, buffer);
    }
    else
    {
        buffer[0] = '\0';
    }

    return (*env)->NewStringUTF(env, buffer);
}

#endif
