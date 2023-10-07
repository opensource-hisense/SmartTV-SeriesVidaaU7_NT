/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma once

/*
 * See documentation in RefBase.h
 */

//#include <atomic>
#include <cutils/atomic.h>//STDLIBC_INTEGER
#include <sys/types.h>

namespace android {

class ReferenceRenamer;

template <class T>
class LightRefBase
{
public:
    inline LightRefBase() : mCount(0) { }
    inline void incStrong(__attribute__((unused)) const void* id) const {
        android_atomic_inc(&mCount);//STDLIBC_INTEGER
        //mCount.fetch_add(1, std::memory_order_relaxed);//STDLIBC_INTEGER
    }
    inline void decStrong(__attribute__((unused)) const void* id) const {
        if (android_atomic_dec(&mCount) == 1) {
            delete static_cast<const T*>(this);

        }//STDLIBC_INTEGER
        //if (mCount.fetch_sub(1, std::memory_order_release) == 1) {
            //std::atomic_thread_fence(std::memory_order_acquire);
            //delete static_cast<const T*>(this);//STDLIBC_INTEGER
       // }
    }
    //! DEBUGGING ONLY: Get current strong ref count.
    inline int32_t getStrongCount() const {
        //return mCount.load(std::memory_order_relaxed);//STDLIBC_INTEGER
        return mCount;//STDLIBC_INTEGER
    }

protected:
    inline ~LightRefBase() { }

private:
    friend class ReferenceMover;
    inline static void renameRefs(size_t /*n*/, const ReferenceRenamer& /*renamer*/) { }
    inline static void renameRefId(T* /*ref*/, const void* /*old_id*/ , const void* /*new_id*/) { }

private:
    //mutable std::atomic<int32_t> mCount;//STDLIBC_INTEGER
    mutable volatile int32_t mCount;//STDLIBC_INTEGER
    //Because of the GCC 4.7 doesn't fully implement the atomic functions, we use stdatomic to replace it.
    //The parameter type is volatile int32_t.
};


// This is a wrapper around LightRefBase that simply enforces a virtual
// destructor to eliminate the template requirement of LightRefBase
class VirtualLightRefBase : public LightRefBase<VirtualLightRefBase> {
public:
    virtual ~VirtualLightRefBase() = default;
};

}  // namespace android
