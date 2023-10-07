/* Copyright (c) 2010-2011 MStar Semiconductor, Inc.  All rights reserved.
 */
#define LOG_TAG "binderclient"
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <binder/IBinder.h>
#include <sys/system_properties.h>
#include <cutils/properties.h>
using namespace android;
#define SERVICE_NAME "mstar.multimedia"
extern int __system_properties_init(void);
int main(int argc, char **argv)
{
    __system_properties_init();
    {
        char property[92];
        property_get("ro.product.brand", property, NULL);
        ALOGI("******* property_get *********** : %s", property);
    }
    {
        char *key;
        char *value;
        __system_property_get(key, value);
    }
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;
    binder = sm->getService(String16(SERVICE_NAME));
    ALOGI("getService(%s) = %p\n", SERVICE_NAME, binder.get());
    if(binder == NULL)
    {
        ALOGE("Can't get %s service!\n", SERVICE_NAME);
        return -1;
    }
    return 0;
}
