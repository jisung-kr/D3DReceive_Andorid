/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Apache ���̼��� 2.0 ����(���� "���̼���")�� ���� ����� �㰡��
 * ���̼����� �ؼ����� ������ �� ������ ����� �� �����ϴ�.
 * ���̼����� �纻�� �������� ���� �� �ֽ��ϴ�.
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * ���� ������ ������ ���� �ʿ��ϰų� �������� �������� �ʴ� �̻�
 * ���̼����� ���� �����Ǵ� ����Ʈ����� "�ִ� �״��",
 *��� �����, �������� ������ �����̳� ���� ���� �����˴ϴ�.
 * ���̼����� ���� Ư�� ����� ���� ���� �� ���ѿ� ���� ������
 * ���̼����� �����ϼ���.
 *
*/

#ifndef _ANDROID_NATIVE_APP_GLUE_H
#define _ANDROID_NATIVE_APP_GLUE_H

#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * <android/native_activity.h>���� �����ϴ� Native Activity �������̽���
 * Ư�� �̺�Ʈ�� �߻��� �� Ȱ���� �� �����尡 ȣ���ϴ�
 * ���ø����̼� ���� �ݹ� ������ ������� �մϴ�.
 *
 * �̴� ������ �ݹ� �� �ϳ��� �����ؼ��� �� �ǰ� 
 * �ý����� ���ø����̼��� ������ �����ϴ� ������ ������ �ǹ��մϴ�.
 * �� ���α׷��� ���� �������̰� �������� �������Դϴ�.
 *
 * 'threaded_native_app' ���� ���̺귯���� ���ø����̼��� �ٸ� �����忡��
 * ��� �� �̺�Ʈ ������ ������ �� �ִ�
 * �ٸ� ���� ���� �����ϴ� �� ���˴ϴ�. ��� ����� ������ �����ϴ�.
 *
 * 1/ ���ø����̼��� "android_main()"�̶�� �Լ��� �����ؾ� �մϴ�.
 * �� �Լ��� Ȱ���� �� ������κ���
 *  ������ �� �����忡�� Ȱ���� ������� �� ȣ��˴ϴ�.
 *
 * 2/ android_main()�� ���ø����̼��� ����Ǵ�
 * ANativeActivity ��ü �ν��Ͻ��� ���� �ٸ� �߿��� ��ü���� ������ �����ϴ�
 * ��ȿ�� "android_app" ������ ���� �����͸� �޽��ϴ�.
 *
 * 3/ "android_app" ��ü�� �̹� �߿��� �� ������ �����ϴ�
 * ALooper �ν��Ͻ��� �����մϴ�.
 *
 * - Ȱ�� ���� �ֱ� �̺�Ʈ(��: "�Ͻ� ����", "�ٽ� ����"). �Ʒ� APP_CMD_XXX
 * ������ �����ϼ���.
 *
 * - Ȱ���� ÷�ε� AInputQueue���� ���� �̺�Ʈ�� �Է��մϴ�.
 *
 * ���� LOOPER_ID_MAIN �� LOOPER_ID_INPUT ���� ����
 * ALooper_pollOnce�� ��ȯ�ϴ� ALooper �ĺ��ڿ�
 * �ش��մϴ�.
 *
 * ���ø����̼��� �߰� ���� �����ڸ� �����ϴ� �� ������ ALooper��
 * ����� �� �ֽ��ϴ�. �ݹ��� ������� �ϰų� LOOPER_ID_USER�� �����ϴ�
 * ��ȯ �ĺ��ڸ� ����� �� �ֽ��ϴ�.
 *
 * 4/ LOOPER_ID_MAIN �Ǵ� LOOPER_ID_INPUT �̺�Ʈ�� ���� ������
 * ��ȯ�� ���� android_poll_source ������ ����ŵ�ϴ�.
 * process() �Լ��� ȣ���Ͽ� �̺�Ʈ�� ��ü������ ó���ϱ� ���� ȣ���ϴ�
 * android_app->onAppCmd �� android_app->onInputEvent��
 * ä�� �� �ֽ��ϴ�.
 *
 * ���� �����͸� ���� �а� ó���ϱ� ����
 * ���� ������ �Լ��� ȣ���� �� �ֽ��ϴ�. ȣ���ϴ� ����� ���� �˾ƺ�����
 * process_cmd() �� process_input() ������ Ȯ���ϼ���.
 *
 * ��ü ��� ������ NDK�� ���ԵǾ� �ִ� "native-activity"
 * ������ �����ϼ���. ���� NativeActivity�� JavaDoc�� �����ϼ���.
 */

struct android_app;

/**
 * �ҽ��� �غ�� �����Ͱ� ���� �� ALooper fd�� ����� �����ʹ�
 * "outData"�� ��ȯ�˴ϴ�.
 */
struct android_poll_source {
    // �� �ҽ��� �ĺ����Դϴ�. LOOPER_ID_MAIN �Ǵ�
    // LOOPER_ID_INPUT.
    int32_t id;

    // �� android_app ID�� ����Ǿ� �ֽ��ϴ�.
    struct android_app* app;

    // �� �ҽ����� �������� ǥ�� ó���� �����ϱ� ���� ȣ��Ǵ�
    // �Լ��Դϴ�.
    void (*process)(struct android_app* app, struct android_poll_source* source);
};

/**
 * ������ ���ø����̼��� ǥ�� ���̱� �ڵ忡 ����
 * �������̽��Դϴ�. �� �𵨿��� ���ø����̼��� �ڵ��
 * ���μ����� �� �����忡�� �и��� ��ü �����忡�� ����˴ϴ�.
 * �����尡 Java VM�� ����� �ʿ�� ������
 * JNI���� Java ��ü�� ȣ���ϰ� �Ϸ��� �����ؾ� �� ��
 * �ֽ��ϴ�.
 */
struct android_app {
    // ���ø����̼��� �ʿ��� ��� ���⿡ ���� ��ü�� ����Ű�� �����͸�
    // ��ġ�� �� �ֽ��ϴ�.
    void* userData;

    // �� �� ���(APP_CMD_*)�� ó���ϴ� �Լ��� ä��ϴ�.
    void (*onAppCmd)(struct android_app* app, int32_t cmd);

    // �Է� �̺�Ʈ�� ó���ϴ� �Լ��� ä��ϴ�. ���⼭��
    // �̺�Ʈ�� �̹� ������ ����ġ�Ǿ�����, ��ȯ �� ����˴ϴ�.
    // �̺�Ʈ�� ó���ϸ� 1�� ��ȯ�ϰ� �⺻ ����ġ�� ��� 0��
    // ��ȯ�մϴ�.
    int32_t (*onInputEvent)(struct android_app* app, AInputEvent* event);

    // �� ���� ANativeActivity ��ü �ν��Ͻ����� ����˴ϴ�.
    ANativeActivity* activity;

    // ���� ���� �������� ����˴ϴ�.
    AConfiguration* config;

    // ���� �� ������ �ν��Ͻ��� ������ ���� �����Դϴ�.
    // ���°� ���� ��� NULL�� �Ǹ�, �ʿ��ϸ� ����� �� �ֽ��ϴ�.
    // �޸𸮴� APP_CMD_RESUME�� ���� android_app_exec_cmd()�� ȣ���� ������ �����ֽ��ϴ�.
    // ȣ��Ǹ� �޸𸮰� �����ǰ� savedState�� NULL�� �����˴ϴ�.
    // �� ������ APP_CMD_SAVE_STATE�� ó���� ���� �����ؾ� �մϴ�.
    // �̶� NULL�� �ʱ�ȭ�ǹǷ� ���¸� malloc�Ͽ�
    // ������ ���⿡ ��ġ�� �� �ֽ��ϴ�. �̷� ��� �޸𸮴�
    // ���߿� �����˴ϴ�.
    void* savedState;
    size_t savedStateSize;

    // Alooper�� ���� ������� ����˴ϴ�.
    ALooper* looper;

    // NULL�� �ƴ� �� ����� �Է� �̺�Ʈ�� �޴� ����
    // �Է� ť�� �˴ϴ�.
    AInputQueue* inputQueue;

    // NULL�� �ƴ� �� ���� �׸� �� �ִ� â ǥ���� �˴ϴ�.
    ANativeWindow* window;

    // â�� ���� ������ �簢���Դϴ�. �̰��� ����ڿ��� ǥ�õǴ�
    // â �������� ��ġ�ؾ� �ϴ� �����Դϴ�.
    ARect contentRect;

    // �� Ȱ���� ���� �����Դϴ�. APP_CMD_START�� �� �ֽ��ϴ�.
    // APP_CMD_RESUME, APP_CMD_PAUSE �Ǵ� APP_CMD_STOP�� �� �� ������ �ڼ��� ������ �Ʒ��� �����ϼ���.
    int activityState;

    // ���ø����̼��� NativeActivity�� ���ŵǾ��ų�
    // �� �����尡 �Ϸ�Ǳ⸦ ��ٸ��� ������ 0�� �ƴմϴ�.
    int destroyRequested;

    // -------------------------------------------------
    // �Ʒ��� ���̱� �ڵ��� "private" �����Դϴ�.

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int msgread;
    int msgwrite;

    pthread_t thread;

    struct android_poll_source cmdPollSource;
    struct android_poll_source inputPollSource;

    int running;
    int stateSaved;
    int destroyed;
    int redrawNeeded;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
    ARect pendingContentRect;
};

enum {
    /**
     * ���� �� �����忡�� ���� ����� Looper ������ ID��
     * ALooper_pollOnce()�� �ĺ��ڷ� ��ȯ�˴ϴ�. �� �ĺ��ڿ�
     * ���� �����ʹ� android_poll_source ������ ���� �������Դϴ�.
     * android_app_read_cmd()
     * �� android_app_exec_cmd()�� �˻��ϰ� ó���� �� �ֽ��ϴ�.
     */
    LOOPER_ID_MAIN = 1,

    /**
     * ���ø����̼� â�� AInputQueue���� ���� �̺�Ʈ�� Looper ������ ID��
     * ALooper_pollOnce()���� �ĺ��ڷ� ��ȯ�˴ϴ�.
     * �� �ĺ��ڿ� ���� �����ʹ�
     * android_poll_source ������ �������Դϴ�. android_app�� inputQueue
     * ��ü�� ���� ���� �� �ֽ��ϴ�.
     */
    LOOPER_ID_INPUT = 2,

    /**
     * ����� ���ǵ� ALooper �ĺ����� �����Դϴ�.
     */
    LOOPER_ID_USER = 3,
};

enum {
    /**
     * �� �������� ���: AInputQueue�� ����Ǿ����ϴ�. �� �����
     * ó�� �� android_app->inputQueue�� �� ť
     * (�Ǵ� NULL)�� ������Ʈ�˴ϴ�.
     */
    APP_CMD_INPUT_CHANGED,

    /**
     * �� �������� ���: �� ANativeWindow�� ����� �� �ֽ��ϴ�.
     * �� ����� ���� �� android_app->window�� �� â
     * ǥ���� �����մϴ�.
     */
    APP_CMD_INIT_WINDOW,

    /**
     * �� �������� ���: ���� ANativeWindow�� �����ؾ�
     * �մϴ�. �� ����� ���� �� android_app->window��
     * ���� â�� ��� �����մϴ�.
     * android_app_exec_cmd�� ȣ���� �� NULL�� �����˴ϴ�.
     */
    APP_CMD_TERM_WINDOW,

    /**
     * �� �������� ���: ���� ANativeWindow ũ�Ⱑ ����Ǿ����ϴ�.
     * �� ũ��� �ٽ� �׸�����.
     */
    APP_CMD_WINDOW_RESIZED,

    /**
     * �� �������� ���: �ý��ۿ��� ���� ANativeWindow��
     * �ٽ� �׷��� �մϴ�. �Ͻ����� �׸��� ������ �����Ϸ��� 
     * android_app_exec_cmd()�� ������ ���� â�� �ٽ� �׷��� �մϴ�.
     */
    APP_CMD_WINDOW_REDRAW_NEEDED,

    /**
     * �� �������� ���: ����Ʈ �Է� â�� ǥ�õǰų� ������ �Ͱ� ����
     * â�� ������ ������ ����Ǿ����ϴ�. android_app::contentRect����
     * �� ������ �簢���� ã�� �� �ֽ��ϴ�.
     */
    APP_CMD_CONTENT_RECT_CHANGED,

    /**
     * �� �������� ���: ���� Ȱ�� â�� �Է� ��Ŀ����
     * ��ġ�߽��ϴ�.
     */
    APP_CMD_GAINED_FOCUS,

    /**
     * �� �������� ���: ���� Ȱ�� â���� �Է� ��Ŀ����
     * ��������ϴ�.
     */
    APP_CMD_LOST_FOCUS,

    /**
     * �� �������� ���: ���� ��ġ ������ ����Ǿ����ϴ�.
     */
    APP_CMD_CONFIG_CHANGED,

    /**
     * �� �������� ���: �ý��ۿ��� ����� �� �ִ� �޸𸮰� �����մϴ�.
     * �޸� ����� ���̼���.
     */
    APP_CMD_LOW_MEMORY,

    /**
     * �� �������� ���: ���� Ȱ���� ���۵Ǿ����ϴ�.
     */
    APP_CMD_START,

    /**
     * �� �������� ���: ���� Ȱ���� �ٽ� ���۵Ǿ����ϴ�.
     */
    APP_CMD_RESUME,

    /**
     * �� �������� ���: ���� ���߿� �ʿ��� �� ������ �� �ֵ��� ��ü������
     * ���ο� ���� ���¸� �����ؾ� �մϴ�. ����� ���°� ������
     * malloc���� �Ҵ��ϰ� android_app.savedStateSize�� ũ���
     * android_app.savedState�� ��ġ�ϼ���. ���߿�
     * �����˴ϴ�.
     */
    APP_CMD_SAVE_STATE,

    /**
     * �� �������� ���: ���� Ȱ���� �Ͻ� �����Ǿ����ϴ�.
     */
    APP_CMD_PAUSE,

    /**
     * �� �������� ���: ���� Ȱ���� �����Ǿ����ϴ�.
     */
    APP_CMD_STOP,

    /**
     * �� �������� ���: ���� Ȱ���� ���ŵǾ�,
     * ��� �����ϱ� ���� �� �����尡 �����Ǳ⸦ ��ٸ��� �ֽ��ϴ�.
     */
    APP_CMD_DESTROY,
};

/**
 * ALooper_pollAll()�� LOOPER_ID_MAIN�� ��ȯ�� �� ȣ���ϸ� ����
 * �� ��� �޽����� �н��ϴ�.
 */
int8_t android_app_read_cmd(struct android_app* android_app);

/**
 * ������ ����� �ʱ⿡ ���� ó���ϱ� ���� android_app_read_cmd()��
 * ��ȯ�ϴ� ������� ȣ���մϴ�. �� �Լ��� ȣ���� ��
 * ��ɿ� ���� �۾��� ������ �� �ֽ��ϴ�.
 */
void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * ������ ����� ���������� ���� ó���ϱ� ���� android_app_read_cmd()��
 * ��ȯ�ϴ� ����� ȣ���մϴ�. �� �Լ��� ȣ���ϱ� ����
 * ��ɿ� ���� �۾��� ���ľ� �մϴ�.
 */
void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * ���ø����̼� �ڵ尡 �����ؾ� �ϴ� �Լ��̸�
 * �ۿ� ���� �� �Է��� ��Ÿ���ϴ�.
 */
extern void android_main(struct android_app* app);

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_NATIVE_APP_GLUE_H */
