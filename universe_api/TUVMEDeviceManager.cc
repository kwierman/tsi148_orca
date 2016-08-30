#include "TUVMEDeviceManager.hh"

//#define fSizePerImage	256*1024*1024/8 //maximum limited by UEFI
#define fSizePerImage	0x10000

TUVMEDeviceManager* TUVMEDeviceManager::gUniverseDeviceManager = NULL;
TUVMEDeviceManager* TUVMEDeviceManager::GetDeviceManager()
{
  if (!gUniverseDeviceManager) gUniverseDeviceManager = new TUVMEDeviceManager;
  return gUniverseDeviceManager;
}

TUVMEDeviceManager::TUVMEDeviceManager()
{
	for (uint32_t i=0;i<TUVMEDevice::kNumberOfDevices;i++) {
		fDevicesRemaining.insert(i);
	}

  if (fDMADevice.Open() < 0) {
    /* Error, also clear out the available devices. */
    fDMADeviceIsOpen = false;
    fDevicesRemaining.clear();
  } else {
    /* Setup device. */
    fDMADeviceIsOpen = true;
  }

}

TUVMEDeviceManager::~TUVMEDeviceManager()
{
  std::set<TUVMEDevice*>::iterator iter; 
  LockDevice();
  for (iter = fAllDevices.begin();iter != fAllDevices.end(); iter++) {  
    delete *iter;
  }
  UnlockDevice();
}


TUVMEDevice* TUVMEDeviceManager::GetDevice(uint32_t vmeAddress, 
  uint32_t addressModifier, uint32_t dataWidth, uint32_t sizeOfImage)
{
	LockDevice();
	TUVMEDevice* device = NULL;
	if (fDevicesRemaining.empty()) {
		UnlockDevice(); 
		return NULL;
	}

	if (sizeOfImage > fSizePerImage) {
		UnlockDevice(); 
		return NULL; 
	}

	//grab the first device available
	uint32_t label;
	label = *fDevicesRemaining.begin();

	device = new TUVMEDevice(label);
	if (device->Open() != 0) {
		delete device;
		UnlockDevice();
		return NULL;
	}

	device->SetWithAddressModifier(addressModifier);
	device->SetDataWidth((TUVMEDevice::ETUVMEDeviceDataWidth)dataWidth);
	device->SetVMEAddress(vmeAddress);

	if (sizeOfImage == 0) sizeOfImage = fSizePerImage;
	if ((addressModifier == 0x29 || addressModifier == 0x2c || addressModifier == 0x2d) &&
		sizeOfImage > 0x10000)
		sizeOfImage = 0x10000;
			  
	device->SetSizeOfImage(sizeOfImage);

	if (device->Enable() < 0) {
		UnlockDevice();
		delete device;
		return NULL;
	}

	fDevicesRemaining.erase(label);
	fAllDevices.insert(device);
	UnlockDevice();
	return device;
}


TUVMEDevice* TUVMEDeviceManager::GetDMADevice(uint32_t vmeAddress, 
	uint32_t addressModifier, uint32_t dataWidth, bool autoIncrement)
{
	/* We have to setup the DMA device. */
	if (!fDMADeviceIsOpen) return NULL;
	fDMADevice.LockDevice();
	fDMADevice.SetWithAddressModifier(addressModifier);
	fDMADevice.SetDataWidth((TUVMEDevice::ETUVMEDeviceDataWidth)dataWidth);
	fDMADevice.SetVMEAddress(vmeAddress);
	fDMADevice.SetNoIncrement(!autoIncrement);
	if (fDMADevice.Enable() < 0) return NULL;
	return &fDMADevice;
}


int32_t TUVMEDeviceManager::CloseDevice(TUVMEDevice* device)
{
  LockDevice();

  if (fAllDevices.find(device) == fAllDevices.end()) {
    UnlockDevice();
    return -1;
  }

  fAllDevices.erase(device);
  fDevicesRemaining.insert(device->GetDevNumber());
  delete device;

  UnlockDevice();
  return 0;
}


void lock_device(TUVMEDevice* dev)
{
  dev->LockDevice();
}

void unlock_device(TUVMEDevice* dev)
{
  dev->UnlockDevice();
}

TUVMEDevice* get_new_device(uint32_t vmeAddress, uint32_t addressModifier, 
  uint32_t dataWidth, uint32_t sizeOfImage)
{
  return TUVMEDeviceManager::GetDeviceManager()->GetDevice(vmeAddress, addressModifier, dataWidth, sizeOfImage);
}

int32_t close_device(TUVMEDevice* device)
{
  return TUVMEDeviceManager::GetDeviceManager()->CloseDevice(device);
}

TUVMEDevice* get_dma_device(uint32_t vmeAddress, uint32_t addressModifier, uint32_t dataWidth, bool autoIncrement)
{
  return TUVMEDeviceManager::GetDeviceManager()->GetDMADevice(vmeAddress, addressModifier, dataWidth, autoIncrement);
}

void release_dma_device(void)
{
  TUVMEDeviceManager::GetDeviceManager()->ReleaseDMADevice();
}


int32_t setup_device(TUVMEDevice* dev, uint32_t vme_address, uint32_t address_modifier, uint32_t data_width)
{
    uint32_t base_address = vme_address & 0xFFFF0000;
    dev->SetWithAddressModifier(address_modifier);
    dev->SetVMEAddress(base_address);
    dev->SetDataWidth((TUVMEDevice::ETUVMEDeviceDataWidth)data_width);
    return 0;
}

int32_t read_device(TUVMEDevice* dev, char* buffer, uint32_t numBytes, uint32_t offset)
{
  return dev->Read(buffer, numBytes, offset);
}
int32_t write_device(TUVMEDevice* dev, char* buffer, uint32_t numBytes, uint32_t offset)
{
  return dev->Write(buffer, numBytes, offset);
}

TUVMEDevice* get_ctl_device()
{
	  return NULL;
}

void set_hw_byte_swap(bool doSwap)
{
}


