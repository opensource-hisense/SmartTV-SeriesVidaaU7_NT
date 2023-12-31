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

#include <binder/Binder.h>

#include <stdatomic.h> //STDLIBC_INTEGER
#include <cutils/atomic.h> //STDLIBC_INTEGER
//#include <atomic> //STDLIBC_INTEGER
#include <utils/misc.h>
#include <binder/BpBinder.h>
//#include <binder/IInterface.h>
#include <binder/IResultReceiver.h>
#include <binder/IShellCallback.h>
#include <binder/Parcel.h>

#include <stdio.h>

namespace android {

// ---------------------------------------------------------------------------

IBinder::IBinder()
    : RefBase()
{
}

IBinder::~IBinder()
{
}

// ---------------------------------------------------------------------------

sp<IInterface>  IBinder::queryLocalInterface(const String16& /*descriptor*/)
{
    return nullptr;
}

BBinder* IBinder::localBinder()
{
    return nullptr;
}

BpBinder* IBinder::remoteBinder()
{
    return nullptr;
}

bool IBinder::checkSubclass(const void* /*subclassID*/) const
{
    return false;
}


status_t IBinder::shellCommand(const sp<IBinder>& target, int in, int out, int err,
    Vector<String16>& args, const sp<IShellCallback>& callback,
    const sp<IResultReceiver>& resultReceiver)
{
    Parcel send;
    Parcel reply;
    send.writeFileDescriptor(in);
    send.writeFileDescriptor(out);
    send.writeFileDescriptor(err);
    const size_t numArgs = args.size();
    send.writeInt32(numArgs);
    for (size_t i = 0; i < numArgs; i++) {
        send.writeString16(args[i]);
    }
    send.writeStrongBinder(callback != nullptr ? IInterface::asBinder(callback) : nullptr);
    send.writeStrongBinder(resultReceiver != nullptr ? IInterface::asBinder(resultReceiver) : nullptr);
    return target->transact(SHELL_COMMAND_TRANSACTION, send, &reply);
}

status_t IBinder::getExtension(sp<IBinder>* out) {
    BBinder* local = this->localBinder();
    if (local != nullptr) {
        *out = local->getExtension();
        return OK;
    }

    BpBinder* proxy = this->remoteBinder();
    LOG_ALWAYS_FATAL_IF(proxy == nullptr);

    Parcel data;
    Parcel reply;
    status_t status = transact(EXTENSION_TRANSACTION, data, &reply);
    if (status != OK) return status;

    return reply.readNullableStrongBinder(out);
}

status_t IBinder::getDebugPid(pid_t* out) {
    BBinder* local = this->localBinder();
    if (local != nullptr) {
      *out = local->getDebugPid();
      return OK;
    }

    BpBinder* proxy = this->remoteBinder();
    LOG_ALWAYS_FATAL_IF(proxy == nullptr);

    Parcel data;
    Parcel reply;
    status_t status = transact(DEBUG_PID_TRANSACTION, data, &reply);
    if (status != OK) return status;

    int32_t pid;
    status = reply.readInt32(&pid);
    if (status != OK) return status;

    if (pid < 0 || pid > std::numeric_limits<pid_t>::max()) {
        return BAD_VALUE;
    }
    *out = pid;
    return OK;
}

// ---------------------------------------------------------------------------

class BBinder::Extras
{
public:
    // unlocked objects
    bool mRequestingSid = false;
    sp<IBinder> mExtension;

    // for below objects
    Mutex mLock;
    BpBinder::ObjectManager mObjects;
};

// ---------------------------------------------------------------------------

BBinder::BBinder()//STDLIBC_INTEGER // : mExtras(nullptr)
{
    atomic_init(&mExtras, static_cast<uintptr_t>(0));
}//STDLIBC_INTEGER

bool BBinder::isBinderAlive() const
{
    return true;
}

status_t BBinder::pingBinder()
{
    return NO_ERROR;
}

const String16& BBinder::getInterfaceDescriptor() const
{
    // This is a local static rather than a global static,
    // to avoid static initializer ordering issues.
    static String16 sEmptyDescriptor;
    ALOGW("reached BBinder::getInterfaceDescriptor (this=%p)", this);
    return sEmptyDescriptor;
}

// NOLINTNEXTLINE(google-default-arguments)
status_t BBinder::transact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    data.setDataPosition(0);

    status_t err = NO_ERROR;
    switch (code) {
        case PING_TRANSACTION:
            err = pingBinder();
            break;
        case EXTENSION_TRANSACTION:
            err = reply->writeStrongBinder(getExtension());
            break;
        case DEBUG_PID_TRANSACTION:
            err = reply->writeInt32(getDebugPid());
            break;
        default:
            err = onTransact(code, data, reply, flags);
            break;
    }

    // In case this is being transacted on in the same process.
    if (reply != nullptr) {
        reply->setDataPosition(0);
    }

    return err;
}

// NOLINTNEXTLINE(google-default-arguments)
status_t BBinder::linkToDeath(
    const sp<DeathRecipient>& /*recipient*/, void* /*cookie*/,
    uint32_t /*flags*/)
{
    return INVALID_OPERATION;
}

// NOLINTNEXTLINE(google-default-arguments)
status_t BBinder::unlinkToDeath(
    const wp<DeathRecipient>& /*recipient*/, void* /*cookie*/,
    uint32_t /*flags*/, wp<DeathRecipient>* /*outRecipient*/)
{
    return INVALID_OPERATION;
}

status_t BBinder::dump(int /*fd*/, const Vector<String16>& /*args*/)
{
    return NO_ERROR;
}

void BBinder::attachObject(
    const void* objectID, void* object, void* cleanupCookie,
    object_cleanup_func func)
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER

    if (!e) {
        e = new Extras;
        Extras* expected = nullptr;
        if (!atomic_compare_exchange_strong_explicit(
                                             &mExtras, &expected,
                                             reinterpret_cast<uintptr_t>(e),
                                             memory_order_release,
                                             memory_order_acquire)) {
           delete e;
        e = reinterpret_cast<Extras*>(expected);  // Filled in by CAS
        }//STDLIBC_INTEGER

        if (e == 0) return; // out of memory
    }

    AutoMutex _l(e->mLock);
    e->mObjects.attach(objectID, object, cleanupCookie, func);
}
static inline uintptr_t load_const_atomic(const atomic_uintptr_t* p,
                                          memory_order mo) {
    atomic_uintptr_t* non_const_p = const_cast<atomic_uintptr_t*>(p);
    return atomic_load_explicit(non_const_p, mo);
}//STDLIBC_INTEGER
void* BBinder::findObject(const void* objectID) const
{
    Extras* e = reinterpret_cast<Extras*>(
                load_const_atomic(&mExtras, memory_order_acquire));//STDLIBC_INTEGER
    if (!e) return NULL;

    AutoMutex _l(e->mLock);
    return e->mObjects.find(objectID);
}

void BBinder::detachObject(const void* objectID)
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER
    if (!e) return;

    AutoMutex _l(e->mLock);
    e->mObjects.detach(objectID);
}

BBinder* BBinder::localBinder()
{
    return this;
}

bool BBinder::isRequestingSid()
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER

    return e && e->mRequestingSid;
}

void BBinder::setRequestingSid(bool requestingSid)
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER

    if (!e) {
        // default is false. Most things don't need sids, so avoiding allocations when possible.
        if (!requestingSid) {
            return;
        }

        e = getOrCreateExtras();
        if (!e) return; // out of memory
    }

    e->mRequestingSid = requestingSid;
}

sp<IBinder> BBinder::getExtension() {
    //STDLIBC_INTEGER  Extras* e = mExtras.load(std::memory_order_acquire);
   Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER
    if (e == nullptr) return nullptr;
    return e->mExtension;
}

pid_t BBinder::getDebugPid() {
    return getpid();
}

void BBinder::setExtension(const sp<IBinder>& extension) {
    Extras* e = getOrCreateExtras();
    e->mExtension = extension;
}

BBinder::~BBinder()
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_relaxed));//STDLIBC_INTEGER
    if (e) delete e;
}


// NOLINTNEXTLINE(google-default-arguments)
status_t BBinder::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t /*flags*/)
{
    switch (code) {
        case INTERFACE_TRANSACTION:
            reply->writeString16(getInterfaceDescriptor());
            return NO_ERROR;

        case DUMP_TRANSACTION: {
            int fd = data.readFileDescriptor();
            int argc = data.readInt32();
            Vector<String16> args;
            for (int i = 0; i < argc && data.dataAvail() > 0; i++) {
               args.add(data.readString16());
            }
            return dump(fd, args);
        }

        case SHELL_COMMAND_TRANSACTION: {
            int in = data.readFileDescriptor();
            int out = data.readFileDescriptor();
            int err = data.readFileDescriptor();
            int argc = data.readInt32();
            Vector<String16> args;
            for (int i = 0; i < argc && data.dataAvail() > 0; i++) {
               args.add(data.readString16());
            }
            sp<IShellCallback> shellCallback = IShellCallback::asInterface(
                    data.readStrongBinder());
            sp<IResultReceiver> resultReceiver = IResultReceiver::asInterface(
                    data.readStrongBinder());

            // XXX can't add virtuals until binaries are updated.
            //return shellCommand(in, out, err, args, resultReceiver);
            (void)in;
            (void)out;
            (void)err;

            if (resultReceiver != nullptr) {
                resultReceiver->send(INVALID_OPERATION);
            }

            return NO_ERROR;
        }

        case SYSPROPS_TRANSACTION: {
            report_sysprop_change();
            return NO_ERROR;
        }

        default:
            return UNKNOWN_TRANSACTION;
    }
}

BBinder::Extras* BBinder::getOrCreateExtras()
{
    Extras* e = reinterpret_cast<Extras*>(
                    atomic_load_explicit(&mExtras, memory_order_acquire));//STDLIBC_INTEGER

    if (!e) {
        e = new Extras;
        Extras* expected = nullptr;
        if (!atomic_compare_exchange_strong_explicit(
                                             &mExtras, &expected,
                                             reinterpret_cast<uintptr_t>(e),
                                             memory_order_release,
                                             memory_order_acquire)) {
           delete e;
        e = reinterpret_cast<Extras*>(expected);  // Filled in by CAS
        }//STDLIBC_INTEGER

        if (e == nullptr) return nullptr; // out of memory
    }



    return e;
}

// ---------------------------------------------------------------------------

enum {
    // This is used to transfer ownership of the remote binder from
    // the BpRefBase object holding it (when it is constructed), to the
    // owner of the BpRefBase object when it first acquires that BpRefBase.
    kRemoteAcquired = 0x00000001
};

BpRefBase::BpRefBase(const sp<IBinder>& o)
    : mRemote(o.get()), mRefs(nullptr), mState(0)
{
    extendObjectLifetime(OBJECT_LIFETIME_WEAK);

    if (mRemote) {
        mRemote->incStrong(this);           // Removed on first IncStrong().
        mRefs = mRemote->createWeak(this);  // Held for our entire lifetime.
    }
}

BpRefBase::~BpRefBase()
{
    if (mRemote) {
        if (!(mState&kRemoteAcquired)) {//STDLIBC_INTEGER
            mRemote->decStrong(this);
        }
        mRefs->decWeak(this);
    }
}

void BpRefBase::onFirstRef()
{
    android_atomic_or(kRemoteAcquired, &mState);//STDLIBC_INTEGER
}

void BpRefBase::onLastStrongRef(const void* /*id*/)
{
    if (mRemote) {
        mRemote->decStrong(this);
    }
}

bool BpRefBase::onIncStrongAttempted(uint32_t /*flags*/, const void* /*id*/)
{
    return mRemote ? mRefs->attemptIncStrong(this) : false;
}

// ---------------------------------------------------------------------------

} // namespace android
