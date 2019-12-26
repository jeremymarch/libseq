//Useful link: http://developer.android.com/training/articles/perf-jni.html

#if defined(__ANDROID__) || defined(ANDROID)

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
#define  LOG_TAG    "hoplite"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
// If you want you can add other log definition for info, warning etc

//global options
VerbSeqOptions opt;

JNIEXPORT jint JNICALL
          Java_com_philolog_hc_VerbSequence_vsInit( JNIEnv* env, jobject thiz, jstring path)
{
    const char *cpath = (*env)->GetStringUTFChars(env, path, 0);

    jint res = vsInit(&opt, cpath);
    LOGE("jnivsInit result: %d, db path: %s", res, cpath);
    (*env)->ReleaseStringUTFChars(env, path, cpath);
    return res;
}

JNIEXPORT jint JNICALL
Java_com_philolog_hc_VerbSequence_setupUnits( JNIEnv* env, jobject thiz, jbooleanArray arr, jboolean isHCGame) {
    jboolean *ba;
    ba = (*env)->GetBooleanArrayElements(env, arr, NULL);
    int baLength = (*env)->GetArrayLength(env, arr);

    opt.numUnits = 0;
    opt.topUnit = 2;
    for (int i = 1; i < baLength; i++) { //i = 1 to skip unit 1
        if (ba[i] == JNI_TRUE) {
            if( i + 1 > opt.topUnit )
            {
                opt.topUnit = i + 1;
            }
            opt.units[opt.numUnits] = i + 1;
            opt.numUnits++;
        }
    }

    if (opt.numUnits < 1)
    {
        opt.units[0] = 2;
        opt.numUnits = 1;
    }

    opt.numVerbs = 0;
    if (isHCGame)
    {
        printf("top unit: %d", opt.topUnit);
        for (int i = 0; i < opt.topUnit; i++)
        {
            vsAddVerbsForUnit(&opt, i+1, opt.verbs, &opt.numVerbs, NUM_VERBS);
        }
    }
    else
    {
        for (int i = 0; i < opt.numUnits; i++) {
            vsAddVerbsForUnit(&opt, opt.units[i], opt.verbs, &opt.numVerbs, NUM_VERBS);
        }
    }
    if (opt.topUnit < 3)
    {
        opt.degreesToChange = 1;
    }
    else {
        opt.degreesToChange = 2;
    }

    opt.repsPerVerb = 4;
/*
    for (int i = 0; i < opt.numVerbs; i++) {
        LOGE("%d, ", opt.verbs[i]);
    }
    LOGE("num: %d ", opt.numVerbs);
*/
    opt.isHCGame = (isHCGame == JNI_TRUE);
    vsReset(&opt, (isHCGame == JNI_TRUE) );
    //opt.practiceVerbID = -1;

    (*env)->ReleaseBooleanArrayElements(env, arr, ba, 0);
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_philolog_hc_VerbSequence_upgradedb( JNIEnv* env, jobject thiz, jstring joldpath, jstring jnewpath )
{
    const char *oldpath = (*env)->GetStringUTFChars(env, joldpath, 0);
    const char *newpath = (*env)->GetStringUTFChars(env, jnewpath, 0);
    int ret = upgradedb(oldpath, newpath);
    (*env)->ReleaseStringUTFChars(env, joldpath, oldpath);
    (*env)->ReleaseStringUTFChars(env, jnewpath, newpath);
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_philolog_hc_VerbSequence_vsNext( JNIEnv* env, jobject thiz, jobject gv1, jobject gv2 )
{
    VerbFormD vf1;
    VerbFormD vf2;
    jint ret = 0;
    jfieldID fid;
    jclass gvcls;

    //get vf1
    gvcls = (*env)->GetObjectClass(env, gv1);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    vf1.person = (unsigned char) (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    vf1.number = (unsigned char) (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    vf1.tense = (unsigned char) (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    vf1.voice = (unsigned char) (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    vf1.mood = (unsigned char) (*env)->GetIntField(env, gv1 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    vf1.verbid = (*env)->GetIntField(env, gv1 ,fid);

    //get vf2
    gvcls = (*env)->GetObjectClass(env, gv2);

    fid = (*env)->GetFieldID(env,gvcls,"person","I");
    vf2.person = (unsigned char) (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"number","I");
    vf2.number = (unsigned char) (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"tense","I");
    vf2.tense = (unsigned char) (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"voice","I");
    vf2.voice = (unsigned char) (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"mood","I");
    vf2.mood = (unsigned char) (*env)->GetIntField(env, gv2 ,fid);

    fid = (*env)->GetFieldID(env,gvcls,"verbid","I");
    vf2.verbid = (*env)->GetIntField(env, gv2 ,fid);

    ret = vsNext( &opt, &vf1, &vf2 );

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
Java_com_philolog_hc_VerbSequence_vsReset( JNIEnv* env, jobject thiz )
{
    vsReset(&opt, opt.isHCGame);
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

    bool ret = vsCompareFormsRecordResult(&opt, expecteducs2, expecteducs2Len, givenucs2, givenucs2Len, MFP, elapsedTime, &score, &lives);//, &score, &lives);

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
    vf.person = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    vf.number = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    vf.tense = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    vf.voice = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    vf.mood = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"verbid","I");
    vf.verbid = (*env)->GetIntField(env, thiz ,fid);

/*
 * //To force a form:
    vf.verb = &verbs[45];
    vf.person = FIRST;
    vf.number = SINGULAR;
    vf.tense = IMPERFECT;
    vf.voice = ACTIVE;
    vf.mood = INDICATIVE;
*/
    int bufferLen = 2048;
    char buffer[bufferLen];

    LOGE("JNI getForm: %d %d %d %d %d %d", vf.person, vf.number, vf.tense, vf.voice, vf.mood, vf.verbid);

    if (!getForm2(&vf, buffer, bufferLen, mf, decompose)) //don't look here because will get stuck for deponents e.g.
    {
        LOGE("JNI getForm returned 0, bufferLen %d", bufferLen);
        return (*env)->NewStringUTF(env, buffer);
    }

    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT jstring JNICALL
Java_com_philolog_hc_GreekVerb_getAbbrevDescription( JNIEnv* env, jobject thiz )
{
    VerbFormC vf;
    jfieldID fid;
    jclass cls;
    jclass cls2;
    cls = (*env)->GetObjectClass(env, thiz);

    fid = (*env)->GetFieldID(env,cls,"person","I");
    vf.person = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"number","I");
    vf.number = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"tense","I");
    vf.tense = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"voice","I");
    vf.voice = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    fid = (*env)->GetFieldID(env,cls,"mood","I");
    vf.mood = (unsigned char) (*env)->GetIntField(env, thiz ,fid);

    jobject verbObj;
    fid = (*env)->GetFieldID(env,cls,"verb","Lcom/philolog/hc/Verb;");
    verbObj = (*env)->GetObjectField(env, thiz ,fid);
    cls2 = (*env)->GetObjectClass(env, verbObj);
    fid = (*env)->GetFieldID(env,cls2,"verbId","I");
    jint verbid = (*env)->GetIntField(env, verbObj ,fid);
    vf.verb = (Verb *) &verbs[verbid];

    int bufferLen = 2048;
    char buffer[bufferLen];

    LOGE("TESTING: %d, %d, %d, %d, %d, %d", vf.person, vf.number, vf.tense, vf.voice, vf.mood, vf.verb->verbid);

    getAbbrevDescription(&vf, buffer, bufferLen);

    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT void JNICALL
Java_com_philolog_hc_Verb_getVerb( JNIEnv* env, jobject thiz, jint verbId )
{
    Verb *v = (Verb *) &verbs[verbId];

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

    Verb *v = (Verb *) &verbs[verbId];
    return deponentType(v);
}

JNIEXPORT jstring JNICALL
Java_com_philolog_hc_GreekVerb_addAccent( JNIEnv* env, jobject thiz, jint accent, jstring str ) {
    int bufferSize = 1024;
    char buffer[bufferSize];
    UCS2 ucs2[bufferSize];
    int ucs2Len = 0;

    const char *letters = (*env)->GetStringUTFChars(env, str, NULL);
    utf8_to_ucs2_string((const unsigned char *)letters, ucs2, &ucs2Len);

    if (ucs2[0] != COMBINING_ACUTE && ucs2[0] != COMBINING_MACRON && ucs2[0] != COMBINING_ROUGH_BREATHING && ucs2[0] != COMBINING_SMOOTH_BREATHING) {
        accentSyllable(ucs2, 0, &ucs2Len, accent, true, PRECOMPOSED_HC_MODE);
        ucs2_to_utf8_string(ucs2, ucs2Len, (unsigned char*)buffer);
    }
    else
    {
        buffer[0] = '\0';
    }

    return (*env)->NewStringUTF(env, buffer);
}

#endif //end #if ANDROID
