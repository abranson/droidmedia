/*
 * Copyright (C) 2014-2015-2015 Jolla Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authored by: Mohammed Hassan <mohammed.hassan@jolla.com>
 */

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/BinderService.h>
#include <gui/ISurfaceComposer.h>
#include <gui/IDisplayEventConnection.h>
#include <binder/IPermissionController.h>
#include <binder/MemoryHeapBase.h>
#include "allocator.h"

#if ANDROID_MAJOR == 4 && ANDROID_MINOR == 1 && ANDROID_MICRO == 2
#include "services/services_4_1_2.h"
#endif

#if ANDROID_MAJOR == 4 && ANDROID_MINOR == 2 && ANDROID_MICRO == 2
#include "services/services_4_2_2.h"
#endif

#if ANDROID_MAJOR == 4 && ANDROID_MINOR == 4 && ANDROID_MICRO == 4
#include "services/services_4_4_4.h"
#endif

#if ANDROID_MAJOR == 5 && ANDROID_MINOR == 1
#include "services/services_5_1_0.h"
#endif

#if ANDROID_MAJOR == 6 && ANDROID_MINOR == 0
#include "services/services_6_0_0.h"
#endif

#if ANDROID_MAJOR == 7 && ANDROID_MINOR == 1 && ANDROID_MICRO == 1
#include "services/services_7_1_1.h"
#endif

#if (ANDROID_MAJOR == 4 && ANDROID_MINOR == 4) || ANDROID_MAJOR >= 5
#include <binder/AppOpsManager.h>
#include <binder/IAppOpsService.h>
class FakeAppOps : public BinderService<FakeAppOps>,
		   public BnAppOpsService
{
public:
  static char const *getServiceName() {
    return "appops";
  }

  virtual int32_t checkOperation(int32_t, int32_t, const String16&) {
    return android::AppOpsManager::MODE_ALLOWED;
  }

  virtual int32_t noteOperation(int32_t, int32_t, const String16&) {
    return android::AppOpsManager::MODE_ALLOWED;
  }

  virtual int32_t startOperation(const sp<IBinder>&, int32_t, int32_t,
				 const String16&) {
    return android::AppOpsManager::MODE_ALLOWED;
  }

  virtual void finishOperation(const sp<IBinder>&, int32_t, int32_t, const String16&) {
    // Nothing
  }

  virtual void startWatchingMode(int32_t, const String16&, const sp<IAppOpsCallback>&) {
    // Nothing
  }

  void stopWatchingMode(const sp<IAppOpsCallback>&) {
    // Nothing
  }

  virtual sp<IBinder> getToken(const sp<IBinder>&) {
    return NULL;
  }

#if ANDROID_MAJOR >= 6
  virtual int32_t permissionToOpCode(const String16& permission) {
    return 0;
  }
#endif
};

#endif

#if ANDROID_MAJOR >= 6

#include <binder/IProcessInfoService.h>

class FakeProcessInfoService : public BinderService<FakeProcessInfoService>,
                        public BnProcessInfoService
{
public:
    static char const *getServiceName() {
        return "processinfo";
    }

    status_t getProcessStatesFromPids(size_t length, int32_t* pids, int32_t* states) {
    	for (int i=0; i< length; i++)
    		states[i] = 0;
    	return 0;
    }
};

#include <binder/IBatteryStats.h>

class FakeBatteryStats : public BinderService<FakeBatteryStats>,
                                public BnBatteryStats
{
public:
    static char const *getServiceName() {
        return "batterystats";
    }
    void noteStartSensor(int uid, int sensor) {  }
    void noteStopSensor(int uid, int sensor) {  }
    void noteStartVideo(int uid) {  }
    void noteStopVideo(int uid) {  }
    void noteStartAudio(int uid) {  }
    void noteStopAudio(int uid) {  }
    void noteResetVideo() {  }
    void noteResetAudio() {  }
    void noteFlashlightOn(int uid) {  }
    void noteFlashlightOff(int uid) {  }
    void noteStartCamera(int uid) {  }
    void noteStopCamera(int uid) {  }
    void noteResetCamera() {  }
    void noteResetFlashlight() {  }
};


#include <gui/ISensorServer.h>
#include <gui/ISensorEventConnection.h>
#include <gui/Sensor.h>
#include <gui/BitTube.h>

class FakeSensorEventConnection : public BnSensorEventConnection
{
    sp<BitTube> mChannel;
public:
    FakeSensorEventConnection()
    {
        mChannel = new BitTube(0);
    }
    sp<BitTube> getSensorChannel() const {
        return mChannel;
    }
    status_t enableDisable(int handle, bool enabled, nsecs_t samplingPeriodNs,
                            nsecs_t maxBatchReportLatencyNs, int reservedFlags) {
        return 0;
    }
    status_t setEventRate(int handle, nsecs_t ns) {
        return 0;
    }
    status_t flush() {
        return 0;
    }
};
class FakeSensorServer : public BinderService<FakeSensorServer>,
                         public BnSensorServer
{
public:
    static char const *getServiceName() {
        return "sensorservice";
    }

    Vector<Sensor> getSensorList(const String16& opPackageName) {
        return Vector<Sensor>();
    }

    sp<ISensorEventConnection> createSensorEventConnection(const String8& packageName,
                        int mode, const String16& opPackageName) {
        return sp<ISensorEventConnection>(new FakeSensorEventConnection);
    }

    int32_t isDataInjectionEnabled() {
        return 0;
    }
};
#endif

using namespace android;

class FakePermissionController : public BinderService<FakePermissionController>,
                                 public BnPermissionController
{
public:
    static char const *getServiceName() {
        return "permission";
    }

    bool checkPermission(const String16& permission, int32_t, int32_t) {
      if (permission == String16("android.permission.CAMERA")) {
	return true;
      }

      return false;
    }

#if ANDROID_MAJOR >= 6
    void getPackagesForUid(const uid_t uid, Vector<String16> &packages) {
      }

    bool isRuntimePermission(const String16& permission) {
         return false;
      }
#endif
};

int
main(int, char**)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();

    FakePermissionController::instantiate();
    MiniSurfaceFlinger::instantiate();

#if (ANDROID_MAJOR == 4 && ANDROID_MINOR == 4) || ANDROID_MAJOR >= 5
    FakeAppOps::instantiate();
#endif

#if ANDROID_MAJOR >= 6
    FakeProcessInfoService::instantiate();
    FakeBatteryStats::instantiate();
    FakeSensorServer::instantiate();
#endif

    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

    return 0;
}
