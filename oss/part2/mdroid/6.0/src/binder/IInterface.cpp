/*
 * Copyright (C) 2005 The Android Open Source Project
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
 */

#define LOG_TAG "IInterface"
#include <utils/Log.h>
#include <binder/IInterface.h>

namespace android {

// ---------------------------------------------------------------------------

IInterface::IInterface()
    : RefBase() {
}

IInterface::~IInterface() {
}

// static
sp<IBinder> IInterface::asBinder(const IInterface* iface)
{
    if (iface == NULL) return NULL;
    return const_cast<IInterface*>(iface)->onAsBinder();
}

// static
sp<IBinder> IInterface::asBinder(const sp<IInterface>& iface)
{
    if (iface == NULL) return NULL;
    return iface->onAsBinder();
}


// ---------------------------------------------------------------------------

}; // namespace android

extern "C" {

void _ZN7android10IInterface8asBinderEv(void *retval, void* self) {
    ALOGW("deprecated asBinder call, please update your code");
    //ALOGI("self: %p, retval: %p", self, retval);
    android::sp<android::IBinder> *ret = new(retval) android::sp<android::IBinder>;
    *ret = android::IInterface::asBinder((android::IInterface*)self);
}

void _ZNK7android10IInterface8asBinderEv(void *retval, void *self) {
    ALOGW("deprecated asBinder call, please update your code");
    //ALOGI("self: %p, retval: %p", self, retval);
    android::sp<android::IBinder> *ret = new(retval) android::sp<android::IBinder>;
    *ret = android::IInterface::asBinder((android::IInterface*)self);
}

} // extern "C"
