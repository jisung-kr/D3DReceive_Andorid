//
// pch.h
// ǥ�� �ý��� ���� ������ ����Դϴ�.
//
// ���� �ý��ۿ��� �̸� �����ϵ� ����� ������ �� ����մϴ�.
// pch.cpp�� �ʿ����� ������, pch.h�� ������Ʈ�� �Ϻ��� ��� cpp ���Ͽ�
// �ڵ����� ���Ե˴ϴ�.
//

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <math.h>

#include <thread>
#include <atomic>

#include "Receiver.h"
#include "lz4.h"
#include "QuickLZ.h"
