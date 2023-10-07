/*
 * Copyright (C) 2008 The Android Open Source Project
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

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */
#define LOG_TAG "ashmem"

#ifndef __ANDROID_VNDK__
#include <dlfcn.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <linux/memfd.h>
#include <log/log.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

//#include <android-base/properties.h>
#include <cutils/ashmem.h>
//#include <android-base/unique_fd.h>

#define ASHMEM_DEVICE "/dev/ashmem"

/* Will be added to UAPI once upstream change is merged */
#define F_SEAL_FUTURE_WRITE 0x0010

/*
 * The minimum vendor API level at and after which it is safe to use memfd.
 * This is to facilitate deprecation of ashmem.
 */
#define MIN_MEMFD_VENDOR_API_LEVEL 29
#define MIN_MEMFD_VENDOR_API_LEVEL_CHAR 'Q'

/* ashmem identity */
static dev_t __ashmem_rdev;
/*
 * If we trigger a signal handler in the middle of locked activity and the
 * signal handler calls ashmem, we could get into a deadlock state.
 */
static pthread_mutex_t __ashmem_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * has_memfd_support() determines if the device can use memfd. memfd support
 * has been there for long time, but certain things in it may be missing.  We
 * check for needed support in it. Also we check if the VNDK version of
 * libcutils being used is new enough, if its not, then we cannot use memfd
 * since the older copies may be using ashmem so we just use ashmem. Once all
 * Android devices that are getting updates are new enough (ex, they were
 * originally shipped with Android release > P), then we can just use memfd and
 * delete all ashmem code from libcutils (while preserving the interface).
 *
 * NOTE:
 * The sys.use_memfd property is set by default to false in Android
 * to temporarily disable memfd, till vendor and apps are ready for it.
 * The main issue: either apps or vendor processes can directly make ashmem
 * IOCTLs on FDs they receive by assuming they are ashmem, without going
 * through libcutils. Such fds could have very well be originally created with
 * libcutils hence they could be memfd. Thus the IOCTLs will break.
 *
 * Set default value of sys.use_memfd property to true once the issue is
 * resolved, so that the code can then self-detect if kernel support is present
 * on the device. The property can also set to true from adb shell, for
 * debugging.
 */

//STDLIBC_INTEGER START
#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
#endif
//STDLIBC_INTEGER END


/* logistics of getting file descriptor for ashmem */
static int __ashmem_open_locked()
{
    int ret;
    struct stat st;

    int fd = TEMP_FAILURE_RETRY(open(ASHMEM_DEVICE, O_RDWR));
    if (fd < 0) {
        return fd;
    }

    ret = TEMP_FAILURE_RETRY(fstat(fd, &st));
    if (ret < 0) {
        int save_errno = errno;
        close(fd);
        errno = save_errno;
        return ret;
    }
    if (!S_ISCHR(st.st_mode) || !st.st_rdev) {
        close(fd);
        errno = ENOTTY;
        return -1;
    }

    __ashmem_rdev = st.st_rdev;
    return fd;
}

static int __ashmem_open()
{
    int fd;

    pthread_mutex_lock(&__ashmem_lock);
    fd = __ashmem_open_locked();
    pthread_mutex_unlock(&__ashmem_lock);

    return fd;
}

/* Make sure file descriptor references ashmem, negative number means false */
static int __ashmem_is_ashmem(int fd, int fatal)
{
    dev_t rdev;
    struct stat st;

    if (fstat(fd, &st) < 0) {
        return -1;
    }

    rdev = 0; /* Too much complexity to sniff __ashmem_rdev */
    if (S_ISCHR(st.st_mode) && st.st_rdev) {
        pthread_mutex_lock(&__ashmem_lock);
        rdev = __ashmem_rdev;
        if (rdev) {
            pthread_mutex_unlock(&__ashmem_lock);
        } else {
            int fd = __ashmem_open_locked();
            if (fd < 0) {
                pthread_mutex_unlock(&__ashmem_lock);
                return -1;
            }
            rdev = __ashmem_rdev;
            pthread_mutex_unlock(&__ashmem_lock);

            close(fd);
        }

        if (st.st_rdev == rdev) {
            return 0;
        }
    }

    if (fatal) {
        if (rdev) {
            LOG_ALWAYS_FATAL("illegal fd=%d mode=0%o rdev=%d:%d expected 0%o %d:%d",
              fd, st.st_mode, major(st.st_rdev), minor(st.st_rdev),
              S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRGRP,
              major(rdev), minor(rdev));
        } else {
            LOG_ALWAYS_FATAL("illegal fd=%d mode=0%o rdev=%d:%d expected 0%o",
              fd, st.st_mode, major(st.st_rdev), minor(st.st_rdev),
              S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IRGRP);
        }
        /* NOTREACHED */
    }

    errno = ENOTTY;
    return -1;
}

static int __ashmem_check_failure(int fd, int result)
{
    if (result == -1 && errno == ENOTTY) __ashmem_is_ashmem(fd, 1);
    return result;
}

static bool memfd_is_ashmem(int fd) {
    static bool fd_check_error_once = false;

    if (__ashmem_is_ashmem(fd, 0) == 0) {
        if (!fd_check_error_once) {
            ALOGE("memfd: memfd expected but ashmem fd used - please use libcutils.\n");
            fd_check_error_once = true;
        }

        return true;
    }

    return false;
}

int ashmem_valid(int fd)
{
    return __ashmem_is_ashmem(fd, 0) >= 0;
}

/*
 * ashmem_create_region - creates a new ashmem region and returns the file
 * descriptor, or <0 on error
 *
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 */
int ashmem_create_region(const char *name, size_t size)
{
    int ret, save_errno;

    int fd = __ashmem_open();
    if (fd < 0) {
        return fd;
    }

    if (name) {
        char buf[ASHMEM_NAME_LEN] = {0};

        strncpy(buf, name, sizeof(buf));//STDLIBC_INTEGER
        ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_NAME, buf));
        if (ret < 0) {
            goto error;
        }
    }

    ret = TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_SIZE, size));
    if (ret < 0) {
        goto error;
    }

    return fd;

error:
    save_errno = errno;
    close(fd);
    errno = save_errno;
    return ret;
}

int ashmem_set_prot_region(int fd, int prot)
{
    int ret = __ashmem_is_ashmem(fd, 1);
    if (ret < 0) {
        return ret;
    }

    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_SET_PROT_MASK, prot));
}

int ashmem_pin_region(int fd, size_t offset, size_t len)
{
    struct ashmem_pin pin = { offset, len };

    int ret = __ashmem_is_ashmem(fd, 1);
    if (ret < 0) {
        return ret;
    }

    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_PIN, &pin));
}

int ashmem_unpin_region(int fd, size_t offset, size_t len)
{
    struct ashmem_pin pin = { offset, len };

    int ret = __ashmem_is_ashmem(fd, 1);
    if (ret < 0) {
        return ret;
    }

    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_UNPIN, &pin));
}

int ashmem_get_size_region(int fd)
{
    int ret = __ashmem_is_ashmem(fd, 1);
    if (ret < 0) {
        return ret;
    }

    return TEMP_FAILURE_RETRY(ioctl(fd, ASHMEM_GET_SIZE, NULL));
}
