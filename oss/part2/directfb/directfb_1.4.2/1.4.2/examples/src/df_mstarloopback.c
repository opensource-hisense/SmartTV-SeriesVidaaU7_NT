//
// How to "Post Event"?
//
static IDirectFB *dfb;
static IDirectFBInputDevice *dfbDevice = NULL;

// Step 1: Add the device callback function in the source code of your application
DFBEnumerationResult input_device_callback(DFBInputDeviceID id, DFBInputDeviceDescription desc, void* data)
{
        if(strcmp(desc.name, "mstarloopback") == 0) // Identify the device
        {
        if(DFB_OK == dfb->GetInputDevice(dfb,id,&dfbDevice))
              {
                   printf(" found loopback device\n");
              }
        }
        return DFENUM_OK;
}

// Step 2: Post Event
void PostEvent()
{
    InputDeviceIoctlData sendkeyparam;
    DFBInputEvent  evt;

    sendkeyparam.request = DFB_DEV_IOC_SEND_LOOPBACK_EVENT;

    //
    // NOTE: "key-press" & "key-release" must be sent in a pair of events
    //

    // Send "key-press" event
    evt.flags      = (DIEF_KEYCODE |DIEF_KEYID |DIEF_KEYSYMBOL);
    evt.type       = DIET_KEYPRESS;
    evt.key_id     = DIKI_B;;
    evt.key_symbol = DIKS_CAPITAL_B;
    evt.key_code   = 0;
    memcpy(sendkeyparam.param, &evt, sizeof(DFBInputEvent));
    dfbDevice->IOCtl(dfbDevice, &sendkeyparam);


    // Send "key-release" event
    evt.type = DIET_KEYRELEASE;
    memcpy(sendkeyparam.param, &evt, sizeof(DFBInputEvent));
    dfbDevice->IOCtl(dfbDevice, &sendkeyparam);
}

int
main( int argc, char *argv[] )
{
    DirectFBInit( &argc, &argv );
    DirectFBCreate( &dfb );
    DFBResult ret;

    ret = dfb->EnumInputDevices( dfb, input_device_callback, NULL );
    if (ret)
      printf( "IDirectFB::EnumInputDevices failed\n" );
    else
      printf("IDirectFB::EnumInputDevices  successful\n");

    PostEvent();

    return 0;
}

// Step 3. Receive the event in another procecss like Netflix
void ReceiveEvent(DFBInputEvent *evt)
{
    if (evt.key_symbol == DIKS_CAPITAL_B)
        printf("================ Got the loopback DIKS_CAPITAL_B key ====================");
}
