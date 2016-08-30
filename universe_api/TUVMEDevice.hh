#ifndef _TUVMEDevice_hh_
#define _TUVMEDevice_hh_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#include <vmebus.h>
}
#include "TUVMEDeviceLock.hh"
#include <string>
#include <sys/mman.h>

class TUVMEDevice {

  public:
	TUVMEDevice(uint32_t devNumber);
	virtual ~TUVMEDevice();

	enum ETUVMEDeviceEnum {kNumberOfDevices = 8};

    	enum ETUVMEDeviceDataWidth { kD8 = 1,
                                 kD16 = 2,
                                 kD32 = 4,
                                 kD64 = 8};

	inline int32_t GetDevNumber() {return fDevNumber;}
	void SetSizeOfImage(uint32_t sizeOfImage);
	void SetVMEAddress(uint32_t vmeAddress); 
	int32_t SetWithAddressModifier(uint32_t addressModifier);
	void SetDataWidth(ETUVMEDeviceDataWidth dataWidth);

	int32_t CheckBusError();
	virtual int32_t Read(char* buffer, uint32_t numBytes, uint32_t offset = 0);
	virtual int32_t Write(char* buffer, uint32_t numBytes, uint32_t offset = 0);

	virtual std::string GetDeviceStringName() {return "vme_mwindow";}
	virtual int32_t Open();
	virtual int32_t Enable(); 
	virtual void Close();

	/* Locking functions for thread safety. */
	virtual int32_t LockDevice() { return fLock.Lock(); }
	virtual int32_t UnlockDevice() { return fLock.Unlock(); }

  protected:
	enum vme_address_modifier fAddressModifier;
	enum vme_data_width fDataWidth;
	uint32_t fSizeOfImage;
	uint32_t fVMEAddress;
	int32_t fDevNumber;
	int32_t fFileNum;
	bool fIsOpen;

	volatile void* fMappedAddress;
	bool fModified;
	struct vme_mapping fDesc;

	TUVMEDeviceLock fLock; // lock for this particular slave window
	static TUVMEDeviceLock fSystemLock; // lock shared between all devices. 

	inline void Reset() 
		{fDataWidth=VME_D32; fAddressModifier=VME_A32_USER_DATA_SCT; fSizeOfImage=0x10000; fVMEAddress=0;
			fModified=true; fMappedAddress=NULL;
		}

//		{fPCIOffset=0;
// 		fMode=kData; fType=kNonPrivileged; fUseBLTs=false; fAllowPostedWrites=false;
//		fUseIORemap=false;}
};
#else
typedef struct TUVMEDevice TUVMEDevice;
#endif /* __cplusplus*/
#endif /* TUVMEDevice.hh */
