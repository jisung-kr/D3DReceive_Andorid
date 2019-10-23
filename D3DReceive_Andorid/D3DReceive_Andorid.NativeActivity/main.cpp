/*
* Copyright (C) 2010 The Android Open Source Project
*
* Apache 라이선스 2.0 버전(이하 "라이선스")에 따라 라이선스가 부여됩니다.
* 라이선스를 준수하지 않으면 이 파일을 사용할 수 없습니다.
* 라이선스의 사본은
*
*      http://www.apache.org/licenses/LICENSE-2.0에서 얻을 수 있습니다.
*
* 적용 가능한 법률에 따라 필요하거나 서면으로 동의하지 않는 이상
* 라이선스에 따라 배포되는 소프트웨어는 "있는 그대로",
* 명시적 또는 묵시적이든 어떠한 유형의 보증이나 조건 없이 배포됩니다.
* 라이선스에 따른 특정 언어의 권한 및 제한에 대한 내용은
* 라이선스를 참조하세요.
*
*/

#include <malloc.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGE(...) (printf("(E): " __VA_ARGS__))


using namespace std;

/* 서버와 연결하기 위한 클라이언트 객체및 스레드*/
Client gClient;
std::thread* NetworkRecvThread = nullptr;
std::thread* NetworkSendThread = nullptr;
bool canRunning = true;

/**
* 저장된 상태 데이터입니다.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t w;
};

/**
* 앱에 대한 공유 상태입니다.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};


float ConvertToRadian(float degree) { return degree * (M_PI / 180.0f); }
float ConvertToDegree(float radian) { return radian * (180.0f/ M_PI); }


/**
* 현재 디스플레이에 대한 EGL 컨텍스트를 초기화합니다.
*/
static int engine_init_display(struct engine* engine) {
	// OpenGL ES 및 EGL 초기화

	/*
	* 여기에서 원하는 구성의 특성을 지정합니다.
	* 아래에서 화면 창과 호환되는 
	* 색상 구성 요소당 최소 8비트가 포함된 EGLConfig를 선택했습니다.
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* 여기에서 애플리케이션이 원하는 구성을 선택합니다. 이 샘플에는
	* 기준에 일치하는 첫 번째 EGLConfig를 선택하는
	* 아주 간소화된 선택 과정이 포함되어 있습니다. */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID는 EGLConfig의 특성으로
	* ANativeWindow_setBuffersGeometry()에서 승인되도록 보장합니다.
	* EGLConfig를 선택하면 EGL_NATIVE_VISUAL_ID를 사용하여 
	* ANativeWindow 버퍼가 일치하도록 안전하게 다시 구성할 수 있습니다. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;

	// GL 상태를 초기화합니다.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return 0;
}

GLuint texture[1];

GLfloat square[] = {
	 -1.0f, -1.0f,  0.0f,
	 1.0f, -1.0f,  0.0f,
	 -1.0f,  1.0f,  0.0f,
	 1.0f,  1.0f,  0.0f,
};

GLfloat texCoords[] = {
	// FRONT
	 -1.0f, -1.0f,
	 1.0f, -1.0f,
	 -1.0f, 1.0f,
	 1.0f, 1.0f,
};

float lightAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };

float matAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const int mClientWidth = 1024;
const int mClinetHeight = 576;


bool loadTextures(char* bitmap)
{
	if (!bitmap)
		return false;

	glGenTextures(1, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mClientWidth,
		mClinetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		bitmap);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

bool init()
{
	//압축 해제 후 텍스쳐 생성
	int size = mClientWidth * mClinetHeight * 4;
	char* srcData = new char[size];

	LZ4_decompress_fast(gClient.GetData(), srcData, size);
	loadTextures(srcData);

	delete[] srcData;

	glEnable(GL_TEXTURE_2D);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glVertexPointer(3, GL_FLOAT, 0, square);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	return true;
}



void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//투영행렬 초기화
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrthof(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	
	//텍스쳐 행렬 초기화(텍스쳐 y축이 반대임)
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glPushMatrix();
	
	glRotatef(90, 0.0f, 0.0f, 1.0f);
	glRotatef(180, 1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	glFlush();
}


/**
* 디스플레이의 현재 프레임입니다.
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// 디스플레이가 없습니다.
		return;
	}

	if (gClient.SizeRQueue() > 0) {
		// 그리기는 화면 업데이트 속도의 제한을 받으므로
		// 여기에서는 타이밍을 계산할 필요가 없습니다.
		LOGI("Drawing...\n");

		//그리기 준비
		init();

		//그리기
		display();

		//버퍼 스와핑
		eglSwapBuffers(engine->display, engine->surface);


		gClient.PopPacketRQueue();
	}

}

/**
* 현재 디스플레이에 연결된 EGL 컨텍스트를 분해합니다.
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* 다음 입력 이벤트를 처리합니다.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;

	int32_t sourceType = AInputEvent_getSource(event);
	int32_t eventType = AInputEvent_getType(event);
	int32_t actionType = AKeyEvent_getAction(event);


	__android_log_print(ANDROID_LOG_DEBUG, "Debug", "SourceType = %d \nEventType = %d\nActionType = %d", sourceType, eventType, actionType);


	if (sourceType & AINPUT_SOURCE_JOYSTICK) {
		if (eventType == AINPUT_EVENT_TYPE_KEY) {

		}
		else if (eventType == AINPUT_EVENT_TYPE_MOTION) {
			/*		*/
			if (actionType == AMOTION_EVENT_ACTION_DOWN) {
				engine->state.x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);
				engine->state.y = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);
				engine->state.z = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0);
				engine->state.w = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0);
			}
			else if (actionType == AMOTION_EVENT_ACTION_MOVE) {
				// xy는 왼쪽 축
				// wz는 오른쪽 축
				float x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);
				float y = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);
				float z = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0);
				float w = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0);

				float lastX = engine->state.x;
				float lastY = engine->state.y;
				float lastZ = engine->state.z;
				float lastW = engine->state.w;

				float dx = x;
				float dy = -y;

				{	
					INPUT_DATA* data = new INPUT_DATA();
					int dataSize = sizeof(INPUT_DATA);
					memset(data, 0x00, dataSize);

					data->mInputType = INPUT_TYPE::INPUT_AXIS_CAMERA_MOVE;
					data->x = dx * 10;
					data->y = dy * 10;

					gClient.PushPacketWQueue(std::make_unique<Packet>(new CHEADER(COMMAND::COMMAND_INPUT, dataSize), data));
				
				}

	
				float dz = ConvertToRadian(0.25f * static_cast<float>(z - lastZ));
				float dw = ConvertToRadian(0.25f * static_cast<float>(w - lastW));

				{
					INPUT_DATA* data = new INPUT_DATA();
					int dataSize = sizeof(INPUT_DATA);
					memset(data, 0x00, dataSize);

					data->mInputType = INPUT_TYPE::INPUT_AXIS_CAMERA_ROT;
					data->z = dz * 5;
					data->w = dw * 5;

					gClient.PushPacketWQueue(std::make_unique<Packet>(new CHEADER(COMMAND::COMMAND_INPUT, dataSize), data));
				}


				engine->state.x = x;
				engine->state.y = y;
				engine->state.w = w;
				engine->state.z = z;

				__android_log_print(ANDROID_LOG_DEBUG, "Debug", "Down X = %f / Y = %f / Z = %f / W = %f", x, y, z, w);
				
				return 0;
			}


	

		}
	}
	else if (sourceType & AINPUT_SOURCE_GAMEPAD) {

	}


	//0일경우 시스템에서 디폴트 처리
	return 0;
}

/**
* 다음 주 명령을 처리합니다.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_START:
		//소켓
		gClient.Init();	//소켓 초기화
		gClient.Connection();	//소켓 연결

			/*	*/
		if (NetworkSendThread == nullptr) {
			NetworkSendThread = new std::thread([&]() -> void {
				while (canRunning) {
					gClient.PushPacketWQueue(std::make_unique<Packet>(new CHEADER(COMMAND::COMMAND_REQ_FRAME)));

					if (!gClient.SendMSG()) {
						break;
					}
					LOGW("Sending Success...\n");
				}
				});
		}

		if (NetworkRecvThread == nullptr) {
			NetworkRecvThread = new std::thread([&]() -> void {
				while (canRunning) {
					if (!gClient.RecvMSG()) {
						break;
					}
					LOGW("Receiving Success...\n");
				}
				});
		}

		break;
	case APP_CMD_SAVE_STATE:
		// 시스템에서 현재 상태를 저장하도록 요청했습니다. 저장하세요.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// 창이 표시되어 준비를 마쳤습니다.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// 창을 숨기거나 닫아 정리합니다.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// 앱에 포커스가 있으면 가속도계 모니터링을 시작합니다.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// 초당 60개의 이벤트를 가져올 수 있습니다.
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}

		break;
	case APP_CMD_LOST_FOCUS:
		// 앱에서 포커스가 사라지면 가속도계 모니터링이 중지됩니다.
		// 사용하지 않는 동안 배터리를 절약하기 위해 조치입니다.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}

		// 애니메이션도 중지됩니다.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;

	}
}

/**
* android_native_app_glue를 사용하는 네이티브 애플리케이션의
* 주 진입점입니다. 자체 스레드에서 실행되며, 입력 이벤트를
* 받고 다른 작업을 수행하는 자체 이벤트 루프를 포함합니다.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// 가속도계 모니터링을 준비합니다.
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);


	if (state->savedState != NULL) {
		// 이전에 저장된 상태로 시작되며, 이 지점에서 복원됩니다.
		engine.state = *(struct saved_state*)state->savedState;
	}


	engine.animating = 1;


	//수행할 작업을 대기하면서 루프를 실행합니다.
	while (1) {
		// 보류 중인 모든 이벤트를 읽습니다.
		int ident;
		int events;
		struct android_poll_source* source;

		// 애니메이션이 동작하지 않으면 이벤트 대기를 영구적으로 차단합니다.
		// 애니메이션이 동작하면 모든 이벤트를 읽을 때까지 루프를 실행한 다음
		// 계속해서 애니메이션의 다음 프레임을 그립니다.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// 이 이벤트를 처리합니다.
			if (source != NULL) {
				source->process(state, source);
			}

			// 센서에 데이터가 있으면 바로 처리됩니다.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						LOGI("accelerometer: x=%f y=%f z=%f",
							event.acceleration.x, event.acceleration.y,
							event.acceleration.z);
					}
				}
			}

			// 종료 중인지 확인합니다.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// 이벤트를 종료한 후 다음 애니메이션 프레임을 그립니다.
			engine.state.angle += .01f;
			if (engine.state.angle > 1) {
				engine.state.angle = 0;
			}
			
			//여기서 그리기 업데이트
			engine_draw_frame(&engine);
		}
	}
}




