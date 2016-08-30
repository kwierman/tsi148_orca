#ifndef _TUVMEDMADevice_hh_
#define _TUVMEDMADevice_hh_

#ifdef __cplusplus
extern "C" {
#include <vmebus.h>
}
#include "TUVMEDevice.hh"
#include <string>

class TUVMEDMADevice: public TUVMEDevice {

  public:
    TUVMEDMADevice();
    virtual ~TUVMEDMADevice();

	virtual std::string GetDeviceStringName() {return "vme_dma";}    
	virtual int32_t Open();
	virtual int32_t Enable();
 	virtual int32_t Read(char* buffer, uint32_t numBytes, uint32_t offset = 0);
 	virtual int32_t Write(char* buffer, uint32_t numBytes, uint32_t offset = 0);
	virtual void Close();

	void SetNoIncrement(bool noInc = true) {fUseNoIncrement = noInc;}

  protected:
	bool fUseNoIncrement;
	void* fDMABuf;
	struct vme_dma fDmaDesc;
};

#endif /* __cplusplus */
#endif /* TUVMEDMADevice.hh */

