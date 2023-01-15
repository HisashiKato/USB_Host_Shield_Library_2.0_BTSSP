/* Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
  
 Modified 18 July 2020 by HisashiKato
 Web      :  http://kato-h.cocolog-nifty.com/khweblog/
 */

#include "BTDSSP.h"
// To enable serial debugging see "settings.h"
//#define EXTRADEBUG // Uncomment to get even more debugging data


const uint8_t BTDSSP::BTDSSP_CONTROL_PIPE = 0;
const uint8_t BTDSSP::BTDSSP_EVENT_PIPE = 1;
const uint8_t BTDSSP::BTDSSP_DATAIN_PIPE = 2;
const uint8_t BTDSSP::BTDSSP_DATAOUT_PIPE = 3;

BTDSSP::BTDSSP(USB *p) :
connectToHIDDevice(false),
pairWithHIDDevice(false),
pairedDevice(false),      ////
linkkeyNotification(false),
pUsb(p), // Pointer to USB class instance - mandatory
bAddress(0), // Device address - mandatory
bNumEP(1), // If config descriptor needs to be parsed
qNextPollTime(0), // Reset NextPollTime
pollInterval(0),
bPollEnable(false) // Don't start polling before dongle is connected
{
        for(uint8_t i = 0; i < BTDSSP_NUM_SERVICES; i++)
                btService[i] = NULL;

        Initialize(); // Set all variables, endpoint structs etc. to default values

        if(pUsb) // Register in USB subsystem
                pUsb->RegisterDeviceClass(this); // Set devConfig[] entry
}

uint8_t BTDSSP::ConfigureDevice(uint8_t parent, uint8_t port, bool lowspeed) {
        const uint8_t constBufSize = sizeof (USB_DEVICE_DESCRIPTOR);
        uint8_t buf[constBufSize];
        USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
        uint8_t rcode;
        UsbDevice *p = NULL;
        EpInfo *oldep_ptr = NULL;

        Initialize(); // Set all variables, endpoint structs etc. to default values

        AddressPool &addrPool = pUsb->GetAddressPool(); // Get memory address of USB device address pool
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nBTDSSP ConfigureDevice"), 0x80);
#endif

        if(bAddress) { // Check if address has already been assigned to an instance
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress in use"), 0x80);
#endif
                return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
        }

        p = addrPool.GetUsbDevicePtr(0); // Get pointer to pseudo device with address 0 assigned
        if(!p) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress not found"), 0x80);
#endif
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
        }

        if(!p->epinfo) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nepinfo is null"), 0x80);
#endif
                return USB_ERROR_EPINFO_IS_NULL;
        }

        oldep_ptr = p->epinfo; // Save old pointer to EP_RECORD of address 0
        p->epinfo = epInfo; // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
        p->lowspeed = lowspeed;
        rcode = pUsb->getDevDescr(0, 0, constBufSize, (uint8_t*)buf); // Get device descriptor - addr, ep, nbytes, data

        p->epinfo = oldep_ptr; // Restore p->epinfo

        if(rcode)
                goto FailGetDevDescr;

        bAddress = addrPool.AllocAddress(parent, false, port); // Allocate new address according to device class

        if(!bAddress) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nOut of address space"), 0x80);
#endif
                return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;
        }

        if (udd->bDeviceClass == 0x09) // Some dongles have an USB hub inside
                goto FailHub;

        epInfo[0].maxPktSize = udd->bMaxPacketSize0; // Extract Max Packet Size from device descriptor
        epInfo[1].epAddr = udd->bNumConfigurations; // Steal and abuse from epInfo structure to save memory

        VID = udd->idVendor;
        PID = udd->idProduct;

        return USB_ERROR_CONFIG_REQUIRES_ADDITIONAL_RESET;

FailHub:
#ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nPlease create a hub instance in your code: \"USBHub Hub1(&Usb);\""), 0x80);
#endif
        pUsb->setAddr(bAddress, 0, 0); // Reset address
        rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;
        Release();
        return rcode;

FailGetDevDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetDevDescr(rcode);
#endif
        if(rcode != hrJERR)
                rcode = USB_ERROR_FailGetDevDescr;
        Release();
        return rcode;
};

uint8_t BTDSSP::Init(uint8_t parent __attribute__((unused)), uint8_t port __attribute__((unused)), bool lowspeed) {
        uint8_t rcode;
        uint8_t num_of_conf = epInfo[1].epAddr; // Number of configurations
        epInfo[1].epAddr = 0;

        AddressPool &addrPool = pUsb->GetAddressPool();
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nBTDSSP Init"), 0x80);
#endif
        UsbDevice *p = addrPool.GetUsbDevicePtr(bAddress); // Get pointer to assigned address record

        if(!p) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress not found"), 0x80);
#endif
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
        }

        delay(300); // Assign new address to the device

        rcode = pUsb->setAddr(0, 0, bAddress); // Assign new address to the device
        if(rcode) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nsetAddr: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
#endif
                p->lowspeed = false;
                goto Fail;
        }
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nAddr: "), 0x80);
        D_PrintHex<uint8_t > (bAddress, 0x80);
#endif

        p->lowspeed = false;

        p = addrPool.GetUsbDevicePtr(bAddress); // Get pointer to assigned address record
        if(!p) {
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nAddress not found"), 0x80);
#endif
                return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
        }

        p->lowspeed = lowspeed;

        rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo); // Assign epInfo to epinfo pointer - only EP0 is known
        if(rcode)
                goto FailSetDevTblEntry;

        for(uint8_t i = 0; i < num_of_conf; i++) {
                ConfigDescParser<USB_CLASS_WIRELESS_CTRL, WI_SUBCLASS_RF, WI_PROTOCOL_BT, CP_MASK_COMPARE_ALL> confDescrParser(this); // Set class id according to the specification
                rcode = pUsb->getConfDescr(bAddress, 0, i, &confDescrParser);
                if(rcode) // Check error code
                        goto FailGetConfDescr;
                if(bNumEP >= BTDSSP_MAX_ENDPOINTS) // All endpoints extracted
                        break;
        }

        if(bNumEP < BTDSSP_MAX_ENDPOINTS)
                goto FailUnknownDevice;

        // Assign epInfo to epinfo pointer - this time all 3 endpoins
        rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);
        if(rcode)
                goto FailSetDevTblEntry;

        // Set Configuration Value
        rcode = pUsb->setConf(bAddress, epInfo[ BTDSSP_CONTROL_PIPE ].epAddr, bConfNum);
        if(rcode)
                goto FailSetConfDescr;

        hci_num_reset_loops = 100; // only loop 100 times before trying to send the hci reset command
        hci_counter = 0;
        hci_state = HCI_INIT_STATE;
        waitingForConnection = false;
        bPollEnable = true;

#ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nBluetooth Dongle Initialized"), 0x80);
#endif
        return 0; // Successful configuration

        /* Diagnostic messages */
FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
        NotifyFailSetDevTblEntry();
        goto Fail;
#endif

FailGetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetConfDescr();
        goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailSetConfDescr();
#endif
        goto Fail;

FailUnknownDevice:
#ifdef DEBUG_USB_HOST
        NotifyFailUnknownDevice(VID, PID);
#endif
        pUsb->setAddr(bAddress, 0, 0); // Reset address
        rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;
Fail:
#ifdef DEBUG_USB_HOST
        Notify(PSTR("\r\nBTDSSP Init Failed, error code: "), 0x80);
        NotifyFail(rcode);
#endif
        Release();
        return rcode;
}

void BTDSSP::Initialize() {
        uint8_t i;
        for(i = 0; i < BTDSSP_MAX_ENDPOINTS; i++) {
                epInfo[i].epAddr = 0;
                epInfo[i].maxPktSize = (i) ? 0 : 8;
                epInfo[i].bmSndToggle = 0;
                epInfo[i].bmRcvToggle = 0;
                epInfo[i].bmNakPower = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
        }
        for(i = 0; i < BTDSSP_NUM_SERVICES; i++) {
                if(btService[i])
                        btService[i]->Reset(); // Reset all Bluetooth services
        }

        connectToHIDDevice = false;
        incomingHIDDevice = false;
        bAddress = 0; // Clear device address
        bNumEP = 1; // Must have to be reset to 1
        qNextPollTime = 0; // Reset next poll time
        pollInterval = 0;
        bPollEnable = false; // Don't start polling before dongle is connected
}

/* Extracts interrupt-IN, bulk-IN, bulk-OUT endpoint information from config descriptor */
void BTDSSP::EndpointXtract(uint8_t conf, uint8_t iface __attribute__((unused)), uint8_t alt, uint8_t proto __attribute__((unused)), const USB_ENDPOINT_DESCRIPTOR *pep) {
        //ErrorMessage<uint8_t>(PSTR("Conf.Val"),conf);
        //ErrorMessage<uint8_t>(PSTR("Iface Num"),iface);
        //ErrorMessage<uint8_t>(PSTR("Alt.Set"),alt);

        if(alt) // Wrong interface - by BT spec, no alt setting
                return;

        bConfNum = conf;
        uint8_t index;

        if((pep->bmAttributes & bmUSB_TRANSFER_TYPE) == USB_TRANSFER_TYPE_INTERRUPT && (pep->bEndpointAddress & 0x80) == 0x80) { // Interrupt In endpoint found
                index = BTDSSP_EVENT_PIPE;
                epInfo[index].bmNakPower = USB_NAK_NOWAIT;
        } else if((pep->bmAttributes & bmUSB_TRANSFER_TYPE) == USB_TRANSFER_TYPE_BULK) // Bulk endpoint found
                index = ((pep->bEndpointAddress & 0x80) == 0x80) ? BTDSSP_DATAIN_PIPE : BTDSSP_DATAOUT_PIPE;
        else
            return;

        // Fill the rest of endpoint data structure
        epInfo[index].epAddr = (pep->bEndpointAddress & 0x0F);
        epInfo[index].maxPktSize = (uint8_t)pep->wMaxPacketSize;
#ifdef EXTRADEBUG
        PrintEndpointDescriptor(pep);
#endif
        if(pollInterval < pep->bInterval) // Set the polling interval as the largest polling interval obtained from endpoints
                pollInterval = pep->bInterval;
        bNumEP++;
}

void BTDSSP::PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr __attribute__((unused))) {

#ifdef EXTRADEBUG
        Notify(PSTR("\r\nEndpoint descriptor:"), 0x80);
        Notify(PSTR("\r\nLength:\t\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bLength, 0x80);
        Notify(PSTR("\r\nType:\t\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bDescriptorType, 0x80);
        Notify(PSTR("\r\nAddress:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bEndpointAddress, 0x80);
        Notify(PSTR("\r\nAttributes:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bmAttributes, 0x80);
        Notify(PSTR("\r\nMaxPktSize:\t"), 0x80);
        D_PrintHex<uint16_t > (ep_ptr->wMaxPacketSize, 0x80);
        Notify(PSTR("\r\nPoll Intrv:\t"), 0x80);
        D_PrintHex<uint8_t > (ep_ptr->bInterval, 0x80);
#endif

}

/* Performs a cleanup after failed Init() attempt */
uint8_t BTDSSP::Release() {
        Initialize(); // Set all variables, endpoint structs etc. to default values
        pUsb->GetAddressPool().FreeAddress(bAddress);
        return 0;
}

uint8_t BTDSSP::Poll() {
        if(!bPollEnable)
                return 0;
        if((int32_t)((uint32_t)millis() - qNextPollTime) >= 0L) { // Don't poll if shorter than polling interval
                qNextPollTime = (uint32_t)millis() + pollInterval; // Set new poll time
                HCI_event_task(); // Poll the HCI event pipe
                HCI_task(); // HCI state machine
                ACL_event_task(); // Poll the ACL input pipe too
                memset(hcibuf, 0, BULK_MAXPKTSIZE); // Clear hcibuf
                memset(hcioutbuf, 0, BULK_MAXPKTSIZE); // Clear hcioutbuf
        }
        return 0;
}

void BTDSSP::disconnect() {
        for(uint8_t i = 0; i < BTDSSP_NUM_SERVICES; i++)
                if(btService[i])
                        btService[i]->disconnect();
};

void BTDSSP::HCI_event_task() {
        uint16_t length = BULK_MAXPKTSIZE; // Request more than 16 bytes anyway, the inTransfer routine will take care of this
        uint8_t rcode = pUsb->inTransfer(bAddress, epInfo[ BTDSSP_EVENT_PIPE ].epAddr, &length, hcibuf, pollInterval); // Input on endpoint 1

        if(!rcode || rcode == hrNAK) { // Check for errors
#ifdef EXTRADEBUG
                if( (length > 0 ) && (hcibuf[0] != 0 ) ){
                        Notify(PSTR("\r\nHCIEVT "), 0x80);
                        for(uint16_t i = 0; i < length; i++) {
                                Notify(PSTR(":"), 0x80);
                                D_PrintHex<uint8_t > (hcibuf[i], 0x80);
                        }
                }
#endif
                switch(hcibuf[0]) { // Switch on event type
                        case EV_COMMAND_COMPLETE:
                                if(!hcibuf[5]) { // Check if command succeeded
                                        hci_set_flag(HCI_FLAG_CMD_COMPLETE); // Set command complete flag
                                }
                                break;

                        case EV_COMMAND_STATUS:
                                if(hcibuf[2]) { // Show status on serial if not OK
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nHCI Command Failed: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);

                                        Notify(PSTR("\r\nNum HCI Command Packets: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[3], 0x80);
                                        Notify(PSTR("\r\nCommand Opcode: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[4], 0x80);
                                        Notify(PSTR(" "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[5], 0x80);

#endif
                                }
                                break;

                        case EV_INQUIRY_COMPLETE: //HCI_Inquiry command has been completed.
                                if(inquiry_counter >= 5 && pairWithHIDDevice) {
                                        inquiry_counter = 0;
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nCouldn't find HID device"), 0x80);
#endif
                                        connectToHIDDevice = false;
                                        pairWithHIDDevice = true;
//                                        pairWithHIDDevice = false;
                                        hci_state = HCI_CHECK_DEVICE_SERVICE;
//                                        hci_state = HCI_SCANNING_STATE;
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nEV_INQUIRY_COMPLETE >>> hci_state = HCI_CHECK_DEVICE_SERVICE"), 0x80);
//                                        Notify(PSTR("\r\nEV_INQUIRY_COMPLETE >>> hci_state = HCI_SCANNING_STATE"), 0x80);
#endif
                                }
                                inquiry_counter++;
#ifdef EXTRADEBUG
/*
                                Notify(PSTR("\r\nEV_INQUIRY_COMPLETE: "), 0x80);
                                Notify(PSTR("\r\ninquiry_counter "), 0x80);
                                Notify(inquiry_counter, 0x80);
*/
#endif
                                break;

                        case EV_INQUIRY_RESULT:
                                if(hcibuf[2]) { // Check that there is more than zero responses
#ifdef EXTRADEBUG
/*
                                        Notify(PSTR("\r\nNumber of responses: "), 0x80);
                                        Notify(hcibuf[2], 0x80);
*/
#endif
                                        for(uint8_t i = 0; i < hcibuf[2]; i++) {
// ADD-------------
                                                if (connectAddressIsSet){

                                                        for(uint8_t k = 0; k < 6; k++) {
                                                                responded_bdaddr[k] = hcibuf[k + 3 + 6 * i];
                                                        }
#ifdef EXTRADEBUG
                                                        Notify(PSTR("\r\nCheck the Device BD address."), 0x80);
                                                                Notify(PSTR("\r\nconnect_bdaddr: "), 0x80);
                                                                for(int8_t n = 5; n > 0; n--) {
                                                                        D_PrintHex<uint8_t > (connect_bdaddr[n], 0x80);
                                                                        Notify(PSTR(":"), 0x80);
                                                                }
                                                                D_PrintHex<uint8_t > (connect_bdaddr[0], 0x80);
                                                                Notify(PSTR("\r\nresponded_bdaddr: "), 0x80);
                                                                for(int8_t n = 5; n > 0; n--) {
                                                                        D_PrintHex<uint8_t > (responded_bdaddr[n], 0x80);
                                                                        Notify(PSTR(":"), 0x80);
                                                                }
                                                                D_PrintHex<uint8_t > (responded_bdaddr[0], 0x80);
#endif
                                                        if (memcmp(responded_bdaddr, connect_bdaddr, sizeof(responded_bdaddr)) == 0) {
                                                                for(uint8_t r = 0; r < 6; r++) {
                                                                        disc_bdaddr[r] = responded_bdaddr[r];
                                                                }
#ifdef DEBUG_USB_HOST
                                                                Notify(PSTR("\r\nBD_ADDR: "), 0x80);
                                                                for(int8_t n = 5; n > 0; n--) {
                                                                        D_PrintHex<uint8_t > (disc_bdaddr[n], 0x80);
                                                                        Notify(PSTR(":"), 0x80);
                                                                }
                                                                D_PrintHex<uint8_t > (disc_bdaddr[0], 0x80);
#endif
                                                                hci_set_flag(HCI_FLAG_HID_DEVICE_FOUND);
                                                                break;

                                                        } else {
#ifdef DEBUG_USB_HOST
                                                                Notify(PSTR("\r\nDevice BD address not match."), 0x80);
#endif
                                                        }
                                                         


                                                } else {

                                                        uint8_t offset = 8 * hcibuf[2] + 3 * i;

                                                        for(uint8_t j = 0; j < 3; j++) {
                                                                classOfDevice[j] = hcibuf[j + 4 + offset];
                                                        }
#ifdef EXTRADEBUG
                                                        Notify(PSTR("\r\nClass of device: "), 0x80);
                                                        D_PrintHex<uint8_t > (classOfDevice[2], 0x80);
                                                        Notify(PSTR(" "), 0x80);
                                                        D_PrintHex<uint8_t > (classOfDevice[1], 0x80);
                                                        Notify(PSTR(" "), 0x80);
                                                        D_PrintHex<uint8_t > (classOfDevice[0], 0x80);
#endif

                                                        if(pairWithHIDDevice && ((classOfDevice[1] & 0x0F) == 0x05) && (classOfDevice[0] & 0xCC)) { // Check if it is a mouse, keyboard, gamepad or joystick - see: http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
#ifdef DEBUG_USB_HOST
                                                                if(classOfDevice[0] & 0x80)
                                                                        Notify(PSTR("\r\nMouse found"), 0x80);
                                                                if(classOfDevice[0] & 0x40)
                                                                        Notify(PSTR("\r\nKeyboard found"), 0x80);
                                                                if(classOfDevice[0] & 0x08)
                                                                        Notify(PSTR("\r\nGamepad found"), 0x80);
                                                                if(classOfDevice[0] & 0x04)
                                                                        Notify(PSTR("\r\nJoystick found"), 0x80);
#endif
                                                                for(uint8_t k = 0; k < 6; k++) {
                                                                        disc_bdaddr[k] = hcibuf[k + 3 + 6 * i];
                                                                }
#ifdef DEBUG_USB_HOST
                                                                Notify(PSTR("\r\nBD_ADDR: "), 0x80);
                                                                for(int8_t n = 5; n > 0; n--) {
                                                                        D_PrintHex<uint8_t > (disc_bdaddr[n], 0x80);
                                                                        Notify(PSTR(":"), 0x80);
                                                                }
                                                                D_PrintHex<uint8_t > (disc_bdaddr[0], 0x80);
#endif
                                                                hci_set_flag(HCI_FLAG_HID_DEVICE_FOUND);
                                                                break;
                                                        }
                                                }
                                        }
                                }
                                break;

                        case EV_CONNECTION_COMPLETE:
                                hci_set_flag(HCI_FLAG_CONNECT_EVENT);
                                if(!hcibuf[2]) { // Check if connected OK
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nEV_CONNECTION_COMPLETE:Connection established"), 0x80);
#endif
                                        hci_handle = hcibuf[3] | ((hcibuf[4] & 0x0F) << 8); // Store the handle for the ACL connection
                                        hci_set_flag(HCI_FLAG_CONNECT_COMPLETE); // Set connection complete flag
                                } else {

#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nConnection Failed: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
#endif
                                }
                                break;

                        case EV_DISCONNECTION_COMPLETE:
                                if(!hcibuf[2]) { // Check if disconnected OK
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nEV_DISCONNECTION_COMPLETE:"), 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[3], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[4], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[5], 0x80);
#endif
                                        hci_set_flag(HCI_FLAG_DISCONNECT_COMPLETE); // Set disconnect command complete flag
                                        hci_clear_flag(HCI_FLAG_CONNECT_COMPLETE); // Clear connection complete flag
                                }
                                break;

                        case EV_REMOTE_NAME_REQUEST_COMPLETE:
                                if(!hcibuf[2]) { // Check if reading is OK
                                        for(uint8_t i = 0; i < min(sizeof (remote_name), sizeof (hcibuf) - 9); i++) {
                                                remote_name[i] = hcibuf[9 + i];
                                                if(remote_name[i] == '\0') // End of string
                                                        break;
                                        }
                                        // TODO: Always set '\0' in remote name!
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nRemote Name: "), 0x80);
                                        for(uint8_t i = 0; i < strlen(remote_name); i++){
                                                Notifyc(remote_name[i], 0x80);
                                        }
#endif
                                        hci_set_flag(HCI_FLAG_REMOTE_NAME_COMPLETE);
                                }
                                break;


                        case EV_CONNECTION_REQUEST:

                                for(uint8_t i = 0; i < 6; i++) {
                                        disc_bdaddr[i] = hcibuf[i + 2];
                                }

                                for(uint8_t i = 0; i < 3; i++) {
                                        classOfDevice[i] = hcibuf[i + 8];
                                }
#ifdef EXTRADEBUG
                                Notify(PSTR("\r\nEV_CONNECTION_REQUEST"), 0x80);
                                Notify(PSTR("\r\nClass of device: "), 0x80);
                                D_PrintHex<uint8_t > (classOfDevice[2], 0x80);
                                Notify(PSTR(" "), 0x80);
                                D_PrintHex<uint8_t > (classOfDevice[1], 0x80);
                                Notify(PSTR(" "), 0x80);
                                D_PrintHex<uint8_t > (classOfDevice[0], 0x80);
#endif
                                if(((classOfDevice[1] & 0x0F) == 0x05) && (classOfDevice[0] & 0xCC)) { // Check if it is a mouse, keyboard, gamepad or joystick
#ifdef DEBUG_USB_HOST
                                        if(classOfDevice[0] & 0x80)
                                                Notify(PSTR("\r\nMouse is connecting"), 0x80);
                                        if(classOfDevice[0] & 0x40)
                                                Notify(PSTR("\r\nKeyboard is connecting"), 0x80);
                                        if(classOfDevice[0] & 0x08)
                                                Notify(PSTR("\r\nGamepad is connecting"), 0x80);
                                        if(classOfDevice[0] & 0x04)
                                                Notify(PSTR("\r\nJoystick is connecting"), 0x80);
#endif
                                        incomingHIDDevice = true;
                                }

                                hci_set_flag(HCI_FLAG_INCOMING_REQUEST);

                                break;


                        case EV_RETURN_LINK_KEYS:
/*
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nReturn_Link_Keys"), 0x80);
#endif

#ifdef EXTRADEBUG
                                Notify(PSTR("\r\nNum_Keys: "), 0x80);
                                        Notify(PSTR(" "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
                                Notify(PSTR("\r\nBD_ADDR: "), 0x80);
                                for(uint8_t i = 5; i > 0; i--) {
                                        Notify(PSTR(" "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[3 + i], 0x80);
                                }
                                D_PrintHex<uint8_t > (hcibuf[3], 0x80);
#endif
*/
                                break;


                        case EV_PIN_CODE_REQUEST:
                                break;


                        case EV_LINK_KEY_REQUEST:// < UseSimplePairing
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nEV_LINK_KEY_REQUEST "), 0x80);
                                Notify(PSTR("\r\npairWithHIDDevice = "), 0x80);
                                D_PrintHex<uint8_t > (pairWithHIDDevice, 0x80);
#endif
                                if( (!pairWithHIDDevice) || (incomingHIDDevice) ){
                                        for(uint8_t i = 0; i < 16; i++) {
                                                link_key[i] = paired_link_key[i];
                                        }
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nPaired Link Key: "), 0x80);
                                        for(uint8_t i = 15; i > 0; i--) {
                                                D_PrintHex<uint8_t > (link_key[i], 0x80);
                                                Notify(PSTR(":"), 0x80);
                                        }
                                        D_PrintHex<uint8_t > (link_key[0], 0x80);
#endif
                                        hci_link_key_request_reply();
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nhci_link_key_request_reply"), 0x80);
#endif
                                } else {
                                        hci_link_key_request_negative_reply();
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nhci_link_key_request_negative_reply"), 0x80);
                                        Notify(PSTR("\r\nStart Simple Pairing"), 0x80);
#endif
                                }
                                break;

                        case EV_IO_CAPABILITY_REQUEST:// < UseSimplePairing
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nReceived IO Capability Request: "), 0x80);
#endif
                                hci_io_capability_request_reply();
                                break;

                        case EV_IO_CAPABILITY_RESPONSE:// < UseSimplePairing

#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nReceived IO Capability Response: "), 0x80);
#endif

                                break;

                        case EV_USER_CONFIRMATION_REQUEST:
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nUser confirmation Request: "), 0x80);
#endif
#ifdef EXTRADEBUG
                                Notify(PSTR("\r\nNumeric value: "), 0x80);
                                for(uint8_t i = 3; i > 0; i--) {
                                        D_PrintHex<uint8_t > (hcibuf[8 + i], 0x80);
                                        Notify(PSTR(" "), 0x80);
                                }
                                D_PrintHex<uint8_t > (hcibuf[8], 0x80);
#endif
                                hci_user_confirmation_request_reply();
                                break;

                        case EV_SIMPLE_PAIRING_COMPLETE:// < UseSimplePairing
                                if(!hcibuf[2]) { // Check if pairing was Complete

#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nSimple Pairing Complete"), 0x80);
#endif

                                } else {
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nPairing Failed: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
#endif
                                        hci_disconnect(hci_handle);
                                        hci_state = HCI_DISCONNECT_STATE;
                                }
                                break;

                        case EV_LINK_KEY_NOTIFICATION:// < UseSimplePairing
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nLink Key Notification"), 0x80);
#endif
                                for(uint8_t i = 0; i < 16; i++) {
                                        link_key[i] = hcibuf[8 + i];
                                }

                                memcpy(connect_bdaddr, disc_bdaddr, 6);
                                memcpy(paired_link_key, link_key, 16);

                                linkkeyNotification = true;
#ifdef EXTRADEBUG
                                Notify(PSTR("\r\nBD_ADDR: "), 0x80);
                                for(int8_t i = 5; i > 0; i--) {
                                        D_PrintHex<uint8_t > (hcibuf[2 + i], 0x80);
                                        Notify(PSTR(" "), 0x80);
                                }
                                D_PrintHex<uint8_t > (hcibuf[2], 0x80);
                                Notify(PSTR("\r\nLink Key for the associated BD_ADDR: "), 0x80);
                                for(uint8_t i = 15; i > 0; i--) {
                                        D_PrintHex<uint8_t > (hcibuf[8 + i], 0x80);
                                        Notify(PSTR(" "), 0x80);
                                }
                                D_PrintHex<uint8_t > (hcibuf[8], 0x80);
#endif
                                break;


                        case EV_AUTHENTICATION_COMPLETE:
                                if(!hcibuf[2]) { // Check if pairing was successful
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nEV_AUTHENTICATION_COMPLETE"), 0x80);
//                                        Notify(PSTR("\r\npairWithHIDDevice:"), 0x80);
//                                        D_PrintHex<uint8_t > (pairWithHIDDevice, 0x80);
//                                        Notify(PSTR("\r\nconnectToHIDDevice:"), 0x80);
//                                        D_PrintHex<uint8_t > (connectToHIDDevice, 0x80);
#endif
                                        if(pairWithHIDDevice && !connectToHIDDevice) { //pairWithHIDDevice:01 connectToHIDDevice:00
#ifdef DEBUG_USB_HOST
                                                Notify(PSTR("\r\nPairing successful with HID device"), 0x80);
#endif
                                                connectToHIDDevice = true; // Used to indicate to the BTHID service, that it should connect to this device
#ifdef DEBUG_USB_HOST
                                                Notify(PSTR("\r\npairWithHIDDevice:"), 0x80);
                                                D_PrintHex<uint8_t > (pairWithHIDDevice, 0x80);
                                                Notify(PSTR("\r\nconnectToHIDDevice:"), 0x80);
                                                D_PrintHex<uint8_t > (connectToHIDDevice, 0x80);
#endif
                                        }

                                        hci_set_connection_encryption(hci_handle);
                                
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nhci_set_connection_encryption(ON)"), 0x80);
                                        D_PrintHex<uint16_t > (hci_handle, 0x80);
#endif
                               
                                        hci_state = HCI_DONE_STATE;
                                } else {
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nPairing Failed: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
#endif
                                        hci_disconnect(hci_handle);
                                        hci_state = HCI_DISCONNECT_STATE;
                                }
                                break;


                        case EV_ENCRYPTION_CHANGE:// < UseSimplePairing
                                if(!hcibuf[2]) { // Check Encryption Changed
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nEV_Encryption Changed"), 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[2], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[3], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[4], 0x80);
//                                        D_PrintHex<uint8_t > (hcibuf[5], 0x80);
#endif
                                }
                                break;


                               /* We will just ignore the following events */
                        case EV_MAX_SLOTS_CHANGE:
                                break;

                        case EV_NUMBER_OF_COMPLETED_PACKETS:
                                break;
                        case EV_ROLE_CHANGED:
                                break;

                        case EV_PAGE_SCAN_REPETITION_MODE_CHANGE:
                                break;


                        case EV_LOOPBACK_COMMAND:
                        case EV_DATA_BUFFER_OVERFLOW:
                        case EV_CHANGE_CONNECTION_LINK_KEY_COMPLETE:
                        case EV_QOS_SETUP_COMPLETE:
                        case EV_READ_REMOTE_VERSION_INFORMATION_COMPLETE:
#ifdef EXTRADEBUG
                                if(hcibuf[0] != 0x00) {
                                        Notify(PSTR("\r\nIgnore HCI Event: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[0], 0x80);
                                }
#endif
                                break;
#ifdef EXTRADEBUG
                        default:
                                if(hcibuf[0] != 0x00) {
                                        Notify(PSTR("\r\nUnmanaged HCI Event: "), 0x80);
                                        D_PrintHex<uint8_t > (hcibuf[0], 0x80);

                                        Notify(PSTR(", data: "), 0x80);
                                        for(uint16_t i = 0; i < hcibuf[1]; i++) {
                                                D_PrintHex<uint8_t > (hcibuf[2 + i], 0x80);
                                                Notify(PSTR(" "), 0x80);
                                        }
                                }
                                break;
#endif
                } // Switch
        }
#ifdef EXTRADEBUG
        else {
                Notify(PSTR("\r\nHCI event error: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
        }
#endif
}

/* Poll Bluetooth and print result */
void BTDSSP::HCI_task() {
        switch(hci_state) {
                case HCI_INIT_STATE:
                        hci_counter++;
                        if(hci_counter > hci_num_reset_loops) { // wait until we have looped x times to clear any old events
                                hci_reset();
                                hci_state = HCI_RESET_STATE;
                                hci_counter = 0;
                        }
                        break;

                case HCI_RESET_STATE:
                        hci_counter++;
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
                                hci_counter = 0;
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nHCI Reset complete"), 0x80);
#endif
                                hci_write_class_of_device();
                                hci_state = HCI_CLASS_OF_DEVICE_STATE;

                        } else if(hci_counter > hci_num_reset_loops) {
                                hci_num_reset_loops *= 10;
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nhci_num_reset_loops: "), 0x80);
                                        D_PrintHex<uint8_t > (hci_num_reset_loops, 0x80);
#endif
                                if(hci_num_reset_loops > 2000)
                                        hci_num_reset_loops = 2000;
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nNo response to HCI Reset"), 0x80);
#endif
                                hci_state = HCI_INIT_STATE;
                                hci_counter = 0;
                        }
                        break;


                case HCI_CLASS_OF_DEVICE_STATE:
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nWrite class of device"), 0x80);
#endif
                                if(btdName != NULL) {
                                        hci_write_local_name(btdName);
                                        hci_state = HCI_WRITE_NAME_STATE;
                                } else {
                                        hci_write_simple_pairing_mode();
                                        hci_state = HCI_WRITE_SIMPLE_PAIRING_STATE;
                                }

                        }
                        break;



                case HCI_WRITE_NAME_STATE:
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nThe name was set to: "), 0x80);
                                NotifyStr(btdName, 0x80);
#endif
                                hci_write_simple_pairing_mode();
                                hci_state = HCI_WRITE_SIMPLE_PAIRING_STATE;
                        }
                        break;


//// SSP

                case HCI_WRITE_SIMPLE_PAIRING_STATE:
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nSimple pairing was enabled"), 0x80);
#endif
                                hci_set_event_mask();
                                hci_state = HCI_SET_EVENT_MASK_STATE;
                        }
                        break;



                case HCI_SET_EVENT_MASK_STATE:
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nSet Event Mask"), 0x80);
#endif
                                hci_state = HCI_CHECK_DEVICE_SERVICE;
                        }
                        break;


//-------------------------------------------------------------------------

                case HCI_AUTHENTICATION_REQUESTED_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {

#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nAuthentication Requested"), 0x80);
#endif
                        }
*/
                        break;

                case HCI_LINK_KEY_REQUEST_REPLY_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nLink Key Request Reply"), 0x80);
#endif
                        }
*/
                        break;

                case HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nStart Simple Pairing"), 0x80);
#endif
                        }
 */
                       break;

                case HCI_IO_CAPABILITY_REQUEST_REPLY_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nIO Capability Request Reply"), 0x80);
#endif
                        }
 */
                       break;

                case HCI_USER_CONFIRMATION_REQUEST_REPLY_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nNumeric Comparison"), 0x80);
#endif
                        }
*/
                        break;

                case HCI_SET_CONNECTION_ENCRYPTION_STATE:
/*
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nSet Connection Encryption"), 0x80);
#endif
                        }
*/
                        break;

//-------------------------------------------------------------------------


                case HCI_CHECK_DEVICE_SERVICE:
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nHCI_CHECK_DEVICE_SERVICE"), 0x80);
#endif
//// check addr & key
                        if (memcmp(connect_bdaddr, zero_bdaddr, sizeof(connect_bdaddr)) != 0) {
                                connectAddressIsSet = 1;
                        }

                        if (memcmp(paired_link_key, zero_link_key, sizeof(paired_link_key)) == 0) {
                                pairWithHIDDevice = 1; //pairing mode
                        }
///
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nconnectAddressIsSet = "), 0x80);
                        D_PrintHex<uint8_t > (connectAddressIsSet, 0x80);
                        Notify(PSTR("\r\npairWithHIDDevice = "), 0x80);
                        D_PrintHex<uint8_t > (pairWithHIDDevice, 0x80);
#endif
                        if(pairWithHIDDevice) {//pairing mode
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nPlease enable discovery of your device."), 0x80);
#endif
                                hci_inquiry();
                                hci_state = HCI_INQUIRY_STATE;

                        } else {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nCHECK_DEVICE_SERVICE >>> HCI_SCANNING_STATE "), 0x80);
#endif
                                hci_state = HCI_SCANNING_STATE;
                        }
                        break;



                case HCI_REMOTE_NAME_STATE:  // <<< EV_REMOTE_NAME_REQUEST_COMPLETE: <<< hci_remote_name_request()
                        if(hci_check_flag(HCI_FLAG_REMOTE_NAME_COMPLETE)) {
#ifdef DEBUG_USB_HOST
//                                Notify(PSTR("\r\nRemote Name: "), 0x80);
//                                for(uint8_t i = 0; i < strlen(remote_name); i++)
//                                        Notifyc(remote_name[i], 0x80);
#endif
                                if(pairWithHIDDevice){//pairing mode

                                        hci_state = HCI_CONNECT_DEVICE_STATE;
                                    
                                } else {
                                    
                                        hci_state = HCI_CHECK_DISC_BDADDR_STATE;

                                }
                        }
                        break;



                case HCI_INQUIRY_STATE:
                        if(hci_check_flag(HCI_FLAG_HID_DEVICE_FOUND)) {
                                hci_inquiry_cancel(); // Stop inquiry
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nHID device found"), 0x80);
#endif
                                hci_remote_name_request();
                                hci_state = HCI_REMOTE_NAME_STATE;

                        }
                        break;


                case HCI_CONNECT_DEVICE_STATE:
                        if(hci_check_flag(HCI_FLAG_CMD_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nConnecting to HID device"), 0x80);
#endif
                                hci_connect();
                                hci_state = HCI_CONNECTED_DEVICE_STATE;
                        }
                        break;


                case HCI_CONNECTED_DEVICE_STATE:
                        if(hci_check_flag(HCI_FLAG_CONNECT_EVENT)) {
                                if(hci_check_flag(HCI_FLAG_CONNECT_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nConnected to HID device"), 0x80);
#endif
                                        hci_authentication_requested(hci_handle);
                                        hci_state = HCI_AUTHENTICATION_REQUESTED_STATE;
                                } else {
#ifdef DEBUG_USB_HOST
                                        Notify(PSTR("\r\nTrying to connect one more time..."), 0x80);
#endif
                                        hci_connect(); // Try to connect one more time
                                }
                        }
                        break;


                case HCI_SCANNING_STATE:
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nHCI_SCANNING_STATE:"), 0x80);
                        Notify(PSTR("\r\nconnectToHIDDevice:"), 0x80);
                        D_PrintHex<uint8_t > (connectToHIDDevice, 0x80);
                        Notify(PSTR("\r\npairWithHIDDevice:"), 0x80);
                        D_PrintHex<uint8_t > (pairWithHIDDevice, 0x80);
#endif
                        if(!connectToHIDDevice && !pairWithHIDDevice) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nWait For Incoming Connection Request"), 0x80);
#endif
                                waitingForConnection = true;
//                              hci_write_scan_enable();

//------
                                hci_clear_flag(HCI_FLAG_INCOMING_REQUEST);
                                if(btdName != NULL){
                                        // Inquiry Scan enabled. Page Scan enabled.
                                        hci_write_scan_enable(INQUIRY_SCAN_ENABLED_PAGE_SCAN_ENABLED); //0x03
#ifdef EXTRADEBUG
                                        Notify(PSTR("\r\nBluetooth Local Name is set."), 0x80);
                                        Notify(PSTR("\r\nInquiry Scan enabled. Page Scan enabled."), 0x80);
#endif
                                } else {
                                        // Inquiry Scan disabled. Page Scan enabled.
                                        hci_write_scan_enable(INQUIRY_SCAN_DISABLED_PAGE_SCAN_ENABLED); //0x02
#ifdef EXTRADEBUG
                                        Notify(PSTR("\r\nBluetooth Local Name is NULL."), 0x80);
                                        Notify(PSTR("\r\nInquiry Scan disabled. Page Scan enabled."), 0x80);
#endif
                                }
//------
                                hci_state = HCI_CONNECT_IN_STATE;
                        }
                        break;



                case HCI_CONNECT_IN_STATE:
                        if(hci_check_flag(HCI_FLAG_INCOMING_REQUEST)) {
                                waitingForConnection = false;
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nHCI_CONNECT_IN_STATE:"), 0x80);
                                Notify(PSTR("\r\nIncoming Connection Request"), 0x80);
#endif
                                hci_remote_name_request();
                                hci_state = HCI_REMOTE_NAME_STATE;
                        }
                        break;


                case HCI_CHECK_DISC_BDADDR_STATE:
#ifdef EXTRADEBUG
                        Notify(PSTR("\r\ndisc_bdaddr: "), 0x80);
                        for(int8_t i = 5; i > 0; i--) {
                                D_PrintHex<uint8_t > (disc_bdaddr[i], 0x80);
                                Notify(PSTR(":"), 0x80);
                        }
                        D_PrintHex<uint8_t > (disc_bdaddr[0], 0x80);

                        Notify(PSTR("\r\nconnect_bdaddr: "), 0x80);
                        for(int8_t i = 5; i > 0; i--) {
                                D_PrintHex<uint8_t > (connect_bdaddr[i], 0x80);
                                Notify(PSTR(":"), 0x80);
                        }
                        D_PrintHex<uint8_t > (connect_bdaddr[0], 0x80);
#endif
////////Check if the device is already paired.
                        if (memcmp(disc_bdaddr, connect_bdaddr, sizeof(disc_bdaddr)) == 0) {
                                pairedDevice = true;
                        } else {
                                pairedDevice = false;
                        }

                        if(!pairedDevice){
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nConnection request device is not paired."), 0x80);
#endif
                                hci_reject_connection();
                                hci_state = HCI_SCANNING_STATE;

                        } else {                

                                hci_accept_connection();
                                hci_state = HCI_CONNECTED_STATE;
                        }
                        break;




                case HCI_CONNECTED_STATE:
                        if(hci_check_flag(HCI_FLAG_CONNECT_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nConnected to Device: "), 0x80);
                                for(int8_t i = 5; i > 0; i--) {
                                        D_PrintHex<uint8_t > (disc_bdaddr[i], 0x80);
                                        Notify(PSTR(":"), 0x80);
                                }
                                D_PrintHex<uint8_t > (disc_bdaddr[0], 0x80);
#endif
                                connectToHIDDevice = true;
                                hci_set_connection_encryption(hci_handle);
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nhci_set_connection_encryption(ON)"), 0x80);
                                D_PrintHex<uint16_t > (hci_handle, 0x80);
#endif
                                // Clear these flags for a new connection
                                l2capConnectionClaimed = false;
                                sdpConnectionClaimed = false;
                                rfcommConnectionClaimed = false;

                                hci_event_flag = 0;

                                hci_state = HCI_DONE_STATE;
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nHCI_CONNECTED_STATE: >>> HCI_DONE_STATE:"), 0x80);
#endif
                        }
                        break;

                case HCI_DONE_STATE:
                        if(hci_check_flag(HCI_FLAG_DISCONNECT_COMPLETE)) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nhci_check_flag HCI_FLAG_DISCONNECT_COMPLETE"), 0x80);
                                Notify(PSTR("\r\nHCI_DONE_STATE: >>> HCI_DISCONNECTED_STATE:"), 0x80);
#endif
                                hci_state = HCI_DISCONNECTED_STATE;
                        }

                        hci_counter++;
                        if(hci_counter > 1000) { // Wait until we have looped 1000 times to make sure that the L2CAP connection has been started
                                hci_counter = 0;
                                hci_state = HCI_DISCONNECT_STATE;
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nHCI_DONE_STATE: >>> HCI_DISCONNECT_STATE:(stay unless disconnected)"), 0x80);
#endif
                        }
                        break;


                case HCI_DISCONNECT_STATE:
                        if(hci_check_flag(HCI_FLAG_DISCONNECT_COMPLETE)) { // stay until disconnect
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nhci_check_flag HCI_FLAG_DISCONNECT_COMPLETE"), 0x80);
                                Notify(PSTR("\r\nHCI_DISCONNECT_STATE: >>> HCI_DISCONNECTED_STATE:"), 0x80);
#endif
                                hci_state = HCI_DISCONNECTED_STATE;
                        }
                        break;

                case HCI_DISCONNECTED_STATE:
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nHCI Disconnected from Device"), 0x80);
#endif
                        disconnect(); // kore wo yaranai to damemitai

                        hci_event_flag = 0; // Clear all flags

                        // Reset all buffers
                        memset(hcibuf, 0, BULK_MAXPKTSIZE);
                        memset(l2capinbuf, 0, BULK_MAXPKTSIZE);

                        connectToHIDDevice = incomingHIDDevice = pairWithHIDDevice = false;
                        linkkeyNotification = false;

                        hci_state = HCI_SCANNING_STATE;
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nHCI_DISCONNECTED_STATE: >>> HCI_SCANNING_STATE:"), 0x80);
#endif
                        break;
                default:
                        break;
        }
}

void BTDSSP::ACL_event_task() {
        uint16_t length = BULK_MAXPKTSIZE;
        uint8_t rcode = pUsb->inTransfer(bAddress, epInfo[ BTDSSP_DATAIN_PIPE ].epAddr, &length, l2capinbuf, pollInterval); // Input on endpoint 2

        if(!rcode) { // Check for errors
                if(length > 0) { // Check if any data was read
                        for(uint8_t i = 0; i < BTDSSP_NUM_SERVICES; i++) {
                                if(btService[i])
                                        btService[i]->ACLData(l2capinbuf);
                        }
                }
        }
#ifdef EXTRADEBUG
        else if(rcode != hrNAK) {
                Notify(PSTR("\r\nACL data in error: "), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
        }
#endif
        for(uint8_t i = 0; i < BTDSSP_NUM_SERVICES; i++)
                if(btService[i])
                        btService[i]->Run();
}

/************************************************************/
/*                    HCI Commands                        */

/************************************************************/
void BTDSSP::HCI_Command(uint8_t* data, uint16_t nbytes) {
        hci_clear_flag(HCI_FLAG_CMD_COMPLETE);
#ifdef EXTRADEBUG
        Notify(PSTR("\r\nHCICMD "), 0x80);
        for(uint16_t i = 0; i < nbytes; i++) {
                Notify(PSTR(":"), 0x80);
                D_PrintHex<uint8_t > (data[i], 0x80);
        }
#endif
        pUsb->ctrlReq(bAddress, epInfo[ BTDSSP_CONTROL_PIPE ].epAddr, bmREQ_HCI_OUT, 0x00, 0x00, 0x00, 0x00, nbytes, nbytes, data, NULL);
}

void BTDSSP::hci_reset() {
        hci_event_flag = 0; // Clear all the flags

        hcioutbuf[0] = 0x03; // HCI OCF = 3
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x00;

        HCI_Command(hcioutbuf, 3);
}

/*
void BTDSSP::hci_write_scan() {
        hci_clear_flag(HCI_FLAG_INCOMING_REQUEST);
        if(btdName != NULL){
#ifdef EXTRADEBUG
                Notify(PSTR("\r\nBluetooth Local Name is set."), 0x80);
#endif
                // Inquiry Scan enabled. Page Scan enabled.
                hci_write_scan_enable(INQUIRY_SCAN_ENABLED_PAGE_SCAN_ENABLED); //0x03
#ifdef EXTRADEBUG
                Notify(PSTR("\r\nInquiry Scan enabled. Page Scan enabled."), 0x80);
#endif
        } else {
                // Inquiry Scan disabled. Page Scan enabled.
                hci_write_scan_enable(INQUIRY_SCAN_DISABLED_PAGE_SCAN_ENABLED); //0x02
#ifdef EXTRADEBUG
                Notify(PSTR("\r\nInquiry Scan disabled. Page Scan enabled."), 0x80);
#endif
        }
}

void BTDSSP::hci_write_scan_disable() {
        hci_write_scan_enable(NO_SCANS_ENABLED); //0x00
}

*/


void BTDSSP::hci_write_scan_enable(uint8_t scan_enable) {
        hcioutbuf[0] = 0x1A; // HCI OCF = 1A
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x01; // parameter length = 1
        hcioutbuf[3] = scan_enable; // configuration parameter

        HCI_Command(hcioutbuf, 4);
}


/*
void BTDSSP::hci_write_scan_enable() {
        hci_clear_flag(HCI_FLAG_INCOMING_REQUEST);

        hcioutbuf[0] = 0x1A; // HCI OCF = 1A
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x01; // parameter length = 1
        if(btdName != NULL){
                hcioutbuf[3] = 0x03; // Inquiry Scan enabled. Page Scan enabled.
#ifdef EXTRADEBUG
                Notify(PSTR("\r\nInquiry Scan enabled. Page Scan enabled."), 0x80);
#endif
        }else{
                hcioutbuf[3] = 0x02; // Inquiry Scan disabled. Page Scan enabled.
#ifdef EXTRADEBUG
                Notify(PSTR("\r\nInquiry Scan disabled. Page Scan enabled."), 0x80);
#endif
        }
        HCI_Command(hcioutbuf, 4);
}

void BTDSSP::hci_write_scan_disable() {
        hcioutbuf[0] = 0x1A; // HCI OCF = 1A
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x01; // parameter length = 1
        hcioutbuf[3] = 0x00; // Inquiry Scan disabled. Page Scan disabled.

        HCI_Command(hcioutbuf, 4);
}
*/


void BTDSSP::hci_accept_connection() {
        hci_clear_flag(HCI_FLAG_CONNECT_COMPLETE);

        hcioutbuf[0] = 0x09; // HCI OCF = 0x09 HCI_Accept_Connection_Request command
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x07; // parameter length 7
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
        hcioutbuf[9] = 0x00; // Switch role to master

        HCI_Command(hcioutbuf, 10);
}

void BTDSSP::hci_reject_connection() {
//        hci_clear_flag(HCI_FLAG_CONNECT_COMPLETE);

        hcioutbuf[0] = 0x0A; // HCI OCF = 0x0A HCI_Reject_Connection_Request command
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x07; // parameter length 7
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
        hcioutbuf[9] = 0x0F; // CONNECTION REJECTED DUE TO UNACCEPTABLE BD_ADDR

        HCI_Command(hcioutbuf, 10);
}


void BTDSSP::hci_remote_name_request() {
//void BTDSSP::hci_remote_name() {
        hci_clear_flag(HCI_FLAG_REMOTE_NAME_COMPLETE);

        hcioutbuf[0] = 0x19; // HCI OCF = 19
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x0A; // parameter length = 10
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
        hcioutbuf[9] = 0x01; // Page Scan Repetition Mode
        hcioutbuf[10] = 0x00; // Reserved
        hcioutbuf[11] = 0x00; // Clock offset - low byte
        hcioutbuf[12] = 0x00; // Clock offset - high byte

        HCI_Command(hcioutbuf, 13);
}



void BTDSSP::hci_write_local_name(const char* name) {
        hcioutbuf[0] = 0x13; // HCI OCF = 13
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = strlen(name) + 1; // parameter length = the length of the string + end byte
        uint8_t i;
        for(i = 0; i < strlen(name); i++)
                hcioutbuf[i + 3] = name[i];
        hcioutbuf[i + 3] = 0x00; // End of string

        HCI_Command(hcioutbuf, 4 + strlen(name));
}


void BTDSSP::hci_set_event_mask() {
        hcioutbuf[0] = 0x01; // HCI OCF = 01
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x08;
        hcioutbuf[3] = 0xFF;
        hcioutbuf[4] = 0xFF;
        hcioutbuf[5] = 0xFF;
        hcioutbuf[6] = 0xFF;
        hcioutbuf[7] = 0xFF;
        hcioutbuf[8] = 0x1F;
        hcioutbuf[9] = 0xFF;
        hcioutbuf[10] = 0x00;

        HCI_Command(hcioutbuf, 11);
}

void BTDSSP::hci_write_simple_pairing_mode() {
        hcioutbuf[0] = 0x56; // HCI OCF = 56
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x01; // parameter length = 1
        hcioutbuf[3] = 0x01; // enable = 1
//        hcioutbuf[3] = enable ? 0x00 : 0x01; // 0x00=OFF 0x01=ON

        HCI_Command(hcioutbuf, 4);
}

void BTDSSP::hci_authentication_requested(uint16_t handle) {
    hcioutbuf[0] = 0x11; // HCI OCF = 11
    hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
    hcioutbuf[2] = 0x02; // Parameter Length = 2
    hcioutbuf[3] = (uint8_t)(handle & 0xFF);//connection handle - low byte
    hcioutbuf[4] = (uint8_t)((handle >> 8) & 0x0F);//connection handle - high byte
    
    HCI_Command(hcioutbuf, 5);
}



void BTDSSP::hci_link_key_request_reply() {
        hcioutbuf[0] = 0x0B; // HCI OCF = 0B
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x16; // parameter length 22
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
        for(uint8_t i = 0; i < 16; i++) hcioutbuf[i + 9] = link_key[i]; // 16 octet link_key
//        hcibuf[9] = link_key[0]; // 16 octet link_key
//        hcibuf[10] = link_key[1];
//        hcibuf[11] = link_key[2];
//        hcibuf[12] = link_key[3];
//        hcibuf[13] = link_key[4];
//        hcibuf[14] = link_key[5];
//        hcibuf[15] = link_key[6];
//        hcibuf[16] = link_key[7];
//        hcibuf[17] = link_key[8];
//        hcibuf[18] = link_key[9];
//        hcibuf[19] = link_key[10];
//        hcibuf[20] = link_key[11];
//        hcibuf[21] = link_key[12];
//        hcibuf[22] = link_key[13];
//        hcibuf[23] = link_key[14];
//        hcibuf[24] = link_key[15];

        HCI_Command(hcioutbuf, 25);
}


void BTDSSP::hci_link_key_request_negative_reply() {
        hcioutbuf[0] = 0x0C; // HCI OCF = 0C
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x06; // parameter length 6
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];

        HCI_Command(hcioutbuf, 9);
}


void BTDSSP::hci_io_capability_request_reply() {
        hcioutbuf[0] = 0x2B; // HCI OCF = 2B
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x09;
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
        hcioutbuf[9] = 0x03; // NoInputNoOutput
        hcioutbuf[10] = 0x00;
        hcioutbuf[11] = 0x00;

        HCI_Command(hcioutbuf, 12);
}

void BTDSSP::hci_user_confirmation_request_reply() {
    hcioutbuf[0] = 0x2C; // HCI OCF = 2C
    hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
    hcioutbuf[2] = 0x06; // Parameter Length = 6
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = disc_bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = disc_bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = disc_bdaddr[1];
//        hcibuf[5] = disc_bdaddr[2];
//        hcibuf[6] = disc_bdaddr[3];
//        hcibuf[7] = disc_bdaddr[4];
//        hcibuf[8] = disc_bdaddr[5];
    
    HCI_Command(hcioutbuf, 9);
}

void BTDSSP::hci_set_connection_encryption(uint16_t handle) {
    hcioutbuf[0] = 0x13; // HCI OCF = 13
    hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
    hcioutbuf[2] = 0x03; // Parameter Length = 9
    hcioutbuf[3] = (uint8_t)(handle & 0xFF); //Connection_Handle - low byte
    hcioutbuf[4] = (uint8_t)((handle >> 8) & 0x0F); //Connection_Handle - high byte
    hcioutbuf[5] = 0x01; // 0x00=OFF 0x01=ON 
    
    HCI_Command(hcioutbuf, 6);
}

void BTDSSP::hci_inquiry() {
        hci_clear_flag(HCI_FLAG_HID_DEVICE_FOUND);
        hcioutbuf[0] = 0x01;
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x05; // Parameter Total Length = 5
        hcioutbuf[3] = 0x33; // LAP: Genera/Unlimited Inquiry Access Code (GIAC = 0x9E8B33) - see https://www.bluetooth.org/Technical/AssignedNumbers/baseband.htm
        hcioutbuf[4] = 0x8B;
        hcioutbuf[5] = 0x9E;
        hcioutbuf[6] = 0x30; // Inquiry time = 61.44 sec (maximum)
        hcioutbuf[7] = 0x0A; // 10 number of responses

        HCI_Command(hcioutbuf, 8);
}

void BTDSSP::hci_inquiry_cancel() {
        hcioutbuf[0] = 0x02;
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x00; // Parameter Total Length = 0

        HCI_Command(hcioutbuf, 3);
}

void BTDSSP::hci_connect() {
        hci_connect(disc_bdaddr); // Use last discovered device
}

void BTDSSP::hci_connect(uint8_t *bdaddr) {
        hci_clear_flag(HCI_FLAG_CONNECT_COMPLETE | HCI_FLAG_CONNECT_EVENT);

        hcioutbuf[0] = 0x05; // HCI_Create_Connection
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x0D; // parameter Total Length = 13
        for(uint8_t i = 0; i < 6; i++) hcioutbuf[i + 3] = bdaddr[i]; // 6 octet bdaddr
//        hcibuf[3] = bdaddr[0]; // 6 octet bdaddr
//        hcibuf[4] = bdaddr[1];
//        hcibuf[5] = bdaddr[2];
//        hcibuf[6] = bdaddr[3];
//        hcibuf[7] = bdaddr[4];
//        hcibuf[8] = bdaddr[5];
        hcioutbuf[9] = 0x18; // DM1 or DH1 may be used
        hcioutbuf[10] = 0xCC; // DM3, DH3, DM5, DH5 may be used
        hcioutbuf[11] = 0x01; // Page repetition mode R1
        hcioutbuf[12] = 0x00; // Reserved
        hcioutbuf[13] = 0x00; // Clock offset
        hcioutbuf[14] = 0x00; // Invalid clock offset
        hcioutbuf[15] = 0x00; // Do not allow role switch

        HCI_Command(hcioutbuf, 16);
}


void BTDSSP::hci_disconnect(uint16_t handle) { // This is called by the different services
        hci_clear_flag(HCI_FLAG_DISCONNECT_COMPLETE);
        hcioutbuf[0] = 0x06; // HCI OCF = 6
        hcioutbuf[1] = 0x01 << 2; // HCI OGF = 1
        hcioutbuf[2] = 0x03; // parameter length = 3
        hcioutbuf[3] = (uint8_t)(handle & 0xFF); //connection handle - low byte
        hcioutbuf[4] = (uint8_t)((handle >> 8) & 0x0F); //connection handle - high byte
        hcioutbuf[5] = 0x13; // reason

        HCI_Command(hcioutbuf, 6);
}

void BTDSSP::hci_write_class_of_device() { // See http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
        hcioutbuf[0] = 0x24; // HCI OCF = 24
        hcioutbuf[1] = 0x03 << 2; // HCI OGF = 3
        hcioutbuf[2] = 0x03; // parameter length = 3
        hcioutbuf[3] = 0x04; // Robot // Minor Device Class
        hcioutbuf[4] = 0x08; // Toy   // Major Device Class
        hcioutbuf[5] = 0x00;          // Major Service Class

        HCI_Command(hcioutbuf, 6);
}
/*******************************************************************
 *                                                                 *
 *                        HCI ACL Data Packet                      *
 *                                                                 *
 *   buf[0]          buf[1]          buf[2]          buf[3]
 *   0       4       8    11 12      16              24            31 MSB
 *  .-+-+-+-+-+-+-+-|-+-+-+-|-+-|-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.
 *  |      HCI Handle       |PB |BC |       Data Total Length       |   HCI ACL Data Packet
 *  .-+-+-+-+-+-+-+-|-+-+-+-|-+-|-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.
 *
 *   buf[4]          buf[5]          buf[6]          buf[7]
 *   0               8               16                            31 MSB
 *  .-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.
 *  |            Length             |          Channel ID           |   Basic L2CAP header
 *  .-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.
 *
 *   buf[8]          buf[9]          buf[10]         buf[11]
 *   0               8               16                            31 MSB
 *  .-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.
 *  |     Code      |  Identifier   |            Length             |   Control frame (C-frame)
 *  .-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-.   (signaling packet format)
 */
/************************************************************/
/*                    L2CAP Commands                        */

/************************************************************/
void BTDSSP::L2CAP_Command(uint16_t handle, uint8_t* data, uint8_t nbytes, uint8_t channelLow, uint8_t channelHigh) {
        uint8_t buf[8 + nbytes];
        buf[0] = (uint8_t)(handle & 0xff); // HCI handle with PB,BC flag
        buf[1] = (uint8_t)(((handle >> 8) & 0x0f) | 0x20);
        buf[2] = (uint8_t)((4 + nbytes) & 0xff); // HCI ACL total data length
        buf[3] = (uint8_t)((4 + nbytes) >> 8);
        buf[4] = (uint8_t)(nbytes & 0xff); // L2CAP header: Length
        buf[5] = (uint8_t)(nbytes >> 8);
        buf[6] = channelLow;
        buf[7] = channelHigh;

        for(uint16_t i = 0; i < nbytes; i++) // L2CAP C-frame
                buf[8 + i] = data[i];

        uint8_t rcode = pUsb->outTransfer(bAddress, epInfo[ BTDSSP_DATAOUT_PIPE ].epAddr, (8 + nbytes), buf);
        if(rcode) {
                delay(100); // This small delay prevents it from overflowing if it fails
#ifdef DEBUG_USB_HOST
                Notify(PSTR("\r\nError sending L2CAP message: 0x"), 0x80);
                D_PrintHex<uint8_t > (rcode, 0x80);
                Notify(PSTR(" - Channel ID: "), 0x80);
                D_PrintHex<uint8_t > (channelHigh, 0x80);
                Notify(PSTR(" "), 0x80);
                D_PrintHex<uint8_t > (channelLow, 0x80);
#endif
        }
}

void BTDSSP::l2cap_connection_request(uint16_t handle, uint8_t rxid, uint8_t* scid, uint16_t psm) {
        l2capoutbuf[0] = L2CAP_CMD_CONNECTION_REQUEST; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x04; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = (uint8_t)(psm & 0xff); // PSM
        l2capoutbuf[5] = (uint8_t)(psm >> 8);
        l2capoutbuf[6] = scid[0]; // Source CID
        l2capoutbuf[7] = scid[1];

        L2CAP_Command(handle, l2capoutbuf, 8);
}

void BTDSSP::l2cap_connection_response(uint16_t handle, uint8_t rxid, uint8_t* dcid, uint8_t* scid, uint8_t result) {
        l2capoutbuf[0] = L2CAP_CMD_CONNECTION_RESPONSE; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x08; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = dcid[0]; // Destination CID
        l2capoutbuf[5] = dcid[1];
        l2capoutbuf[6] = scid[0]; // Source CID
        l2capoutbuf[7] = scid[1];
        l2capoutbuf[8] = result; // Result: Pending or Success
        l2capoutbuf[9] = 0x00;
        l2capoutbuf[10] = 0x00; // No further information
        l2capoutbuf[11] = 0x00;

        L2CAP_Command(handle, l2capoutbuf, 12);
}

void BTDSSP::l2cap_config_request(uint16_t handle, uint8_t rxid, uint8_t* dcid) {
        l2capoutbuf[0] = L2CAP_CMD_CONFIG_REQUEST; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x08; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = dcid[0]; // Destination CID
        l2capoutbuf[5] = dcid[1];
        l2capoutbuf[6] = 0x00; // Flags
        l2capoutbuf[7] = 0x00;
        l2capoutbuf[8] = 0x01; // Config Opt: type = MTU (Maximum Transmission Unit) - Hint
        l2capoutbuf[9] = 0x02; // Config Opt: length
        l2capoutbuf[10] = 0xFF; // MTU
        l2capoutbuf[11] = 0xFF;

        L2CAP_Command(handle, l2capoutbuf, 12);
}

void BTDSSP::l2cap_config_response(uint16_t handle, uint8_t rxid, uint8_t* scid) {
        l2capoutbuf[0] = L2CAP_CMD_CONFIG_RESPONSE; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x0A; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = scid[0]; // Source CID
        l2capoutbuf[5] = scid[1];
        l2capoutbuf[6] = 0x00; // Flag
        l2capoutbuf[7] = 0x00;
        l2capoutbuf[8] = 0x00; // Result
        l2capoutbuf[9] = 0x00;
        l2capoutbuf[10] = 0x01; // Config
        l2capoutbuf[11] = 0x02;
        l2capoutbuf[12] = 0xA0;
        l2capoutbuf[13] = 0x02;

        L2CAP_Command(handle, l2capoutbuf, 14);
}

void BTDSSP::l2cap_disconnection_request(uint16_t handle, uint8_t rxid, uint8_t* dcid, uint8_t* scid) {
        l2capoutbuf[0] = L2CAP_CMD_DISCONNECT_REQUEST; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x04; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = dcid[0];
        l2capoutbuf[5] = dcid[1];
        l2capoutbuf[6] = scid[0];
        l2capoutbuf[7] = scid[1];

        L2CAP_Command(handle, l2capoutbuf, 8);
}

void BTDSSP::l2cap_disconnection_response(uint16_t handle, uint8_t rxid, uint8_t* dcid, uint8_t* scid) {
        l2capoutbuf[0] = L2CAP_CMD_DISCONNECT_RESPONSE; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x04; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = dcid[0];
        l2capoutbuf[5] = dcid[1];
        l2capoutbuf[6] = scid[0];
        l2capoutbuf[7] = scid[1];

        L2CAP_Command(handle, l2capoutbuf, 8);
}

void BTDSSP::l2cap_information_response(uint16_t handle, uint8_t rxid, uint8_t infoTypeLow, uint8_t infoTypeHigh) {
        l2capoutbuf[0] = L2CAP_CMD_INFORMATION_RESPONSE; // Code
        l2capoutbuf[1] = rxid; // Identifier
        l2capoutbuf[2] = 0x08; // Length
        l2capoutbuf[3] = 0x00;
        l2capoutbuf[4] = infoTypeLow;
        l2capoutbuf[5] = infoTypeHigh;
        l2capoutbuf[6] = 0x00; // Result = success
        l2capoutbuf[7] = 0x00; // Result = success
        l2capoutbuf[8] = 0x00;
        l2capoutbuf[9] = 0x00;
        l2capoutbuf[10] = 0x00;
        l2capoutbuf[11] = 0x00;

        L2CAP_Command(handle, l2capoutbuf, 12);
}
