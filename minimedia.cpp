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
#include <CameraService.h>
#include <binder/MemoryHeapBase.h>
#include <media/IMediaPlayerService.h>
#include <media/IMediaCodecList.h>
#include <media/IMediaRecorder.h>
#include <media/ICrypto.h>
#include <media/IDrm.h>
#include <media/IHDCP.h>
#include <media/IRemoteDisplay.h>
#include <media/IDrm.h>
#include <media/stagefright/MediaCodecList.h>
#include <OMX.h>
#if ANDROID_MAJOR >= 6
#include <binder/BinderService.h>
#if ANDROID_MAJOR < 7
#include <camera/ICameraService.h>
#else
#include <android/hardware/ICameraService.h>
#endif
#include <binder/IInterface.h>
#include <cutils/multiuser.h>
#endif

#define LOG_TAG "MinimediaService"

// echo "persist.camera.shutter.disable=1" >> /system/build.prop

using namespace android;

#define BINDER_SERVICE_CHECK_INTERVAL 500000


#if ANDROID_MAJOR >= 6

#include <camera/ICameraServiceProxy.h>

class FakeCameraServiceProxy : public BinderService<FakeCameraServiceProxy>,
                        public BnCameraServiceProxy
{
public:
    static char const *getServiceName() {
        return "media.camera.proxy";
    }

    void pingForUserUpdate() {
    }

    void notifyCameraState(String16 cameraId, CameraState newCameraState) {
    }
};


#endif

class FakeMediaPlayerService : public BinderService<FakeMediaPlayerService>,
                        public BnMediaPlayerService
{
private:
    sp<IOMX>                    mOMX;
public:
    static char const *getServiceName() {
        return "media.player";
    }
    sp<IMediaRecorder> createMediaRecorder(const String16 &opPackageName) {
        return NULL;
    }
    sp<IMediaMetadataRetriever> createMetadataRetriever() {
        return NULL;
    }
    sp<IMediaPlayer> create(const sp<IMediaPlayerClient>& client, int audioSessionId = 0) {
        return NULL;
    }

    sp<IOMX> getOMX() {
        if (mOMX.get() == NULL) {
            mOMX = new OMX;
        }
        return mOMX;
    }
    sp<ICrypto>         makeCrypto() {
        return NULL;
    }
    sp<IDrm>            makeDrm() {
        return NULL;
    }
    sp<IHDCP>           makeHDCP(bool createEncryptionModule) {
        return NULL;
    }
    sp<IMediaCodecList> getCodecList() const {
        return MediaCodecList::getLocalInstance();
    }
    sp<IRemoteDisplay> listenForRemoteDisplay(const String16 &opPackageName,
            const sp<IRemoteDisplayClient>& client, const String8& iface){
        return NULL;
    }
    void addBatteryData(uint32_t params) { }
    status_t pullBatteryData(Parcel* reply) { return 0; }
};

int
main(int, char**)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
#if ANDROID_MAJOR >= 6
    FakeCameraServiceProxy::instantiate();
#endif
    FakeMediaPlayerService::instantiate();
    CameraService::instantiate();

#if ANDROID_MAJOR >= 6
    // Camera service needs to be told which users may use the camera
    sp<IBinder> binder;
    do {
        binder = sm->getService(String16("media.camera"));
        if (binder != NULL) {
            break;
        }
        ALOGW("Camera service is not yet available, waiting...");
        usleep(BINDER_SERVICE_CHECK_INTERVAL);
    } while (true);
    ALOGD("Allowing use of the camera for users root and bin");
#if ANDROID_MAJOR >= 7
    sp<hardware::ICameraService> gCameraService = interface_cast<hardware::ICameraService>(binder);
    std::vector<int32_t> users = {0, 1};
    gCameraService->notifySystemEvent(1, users);
#else
    sp<ICameraService> gCameraService = interface_cast<ICameraService>(binder);
    int32_t users[2];
    users[0] = 0; users[1] = 1;
    gCameraService->notifySystemEvent(1, users, 2);
#endif
#endif

    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

    return 0;
}
