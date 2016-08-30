#ifndef _TUVMEDeviceManager_hh_
#define _TUVMEDeviceManager_hh_

#include "TUVMEDevice.hh"
#include "TUVMEDMADevice.hh"
#include "TUVMEDeviceLock.hh"
#ifdef __cplusplus

#include <set> 
#include <vector> 

class TUVMEDeviceManager {
  public:
	static TUVMEDeviceManager* GetDeviceManager();
	TUVMEDevice* GetDevice(uint32_t vmeAddress, uint32_t addressModifier, uint32_t dataWidth, uint32_t sizeOfImage = 0);
	TUVMEDevice* GetDMADevice(uint32_t vmeAddress, uint32_t addressModifier, uint32_t dataWidth, bool autoIncrement);
	void ReleaseDMADevice() {fDMADevice.UnlockDevice();}
	int32_t CloseDevice(TUVMEDevice* device);

	virtual int32_t LockDevice() { return fLock.Lock(); }
	virtual int32_t UnlockDevice() { return fLock.Unlock(); }

  protected:
	TUVMEDeviceManager();
	virtual ~TUVMEDeviceManager();
	std::set<TUVMEDevice*> fAllDevices; 
	std::set<uint32_t> fDevicesRemaining;

	TUVMEDeviceLock fLock;
	TUVMEDMADevice fDMADevice;
	bool fDMADeviceIsOpen;

  private:
	static TUVMEDeviceManager* gUniverseDeviceManager;
};
#endif /*__cplusplus*/

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
extern TUVMEDevice* get_new_device(uint32_t vmeAddress, uint32_t addressModifier, uint32_t dataWidth, uint32_t sizeOfImage);
extern int32_t close_device(TUVMEDevice* device);
extern TUVMEDevice* get_dma_device(uint32_t vmeAddress, uint32_t addressModifier, uint32_t dataWidth, bool autoIncrement);
extern void release_dma_device(void);
extern TUVMEDevice* get_ctl_device(void);
extern void set_hw_byte_swap(bool doSwap);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void lock_device(TUVMEDevice*);
extern void unlock_device(TUVMEDevice*);
extern int32_t setup_device(TUVMEDevice* dev, uint32_t vme_address, uint32_t address_modifier, uint32_t data_width);
extern int32_t read_device(TUVMEDevice*, char* buffer, uint32_t numBytes, uint32_t offset);
extern int32_t write_device(TUVMEDevice*, char* buffer, uint32_t numBytes, uint32_t offset);
#ifdef __cplusplus
}
#endif

#endif
