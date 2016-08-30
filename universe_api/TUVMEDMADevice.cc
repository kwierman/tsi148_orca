#include "TUVMEDMADevice.hh"
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sstream>
#include <stdlib.h>
#include <cstring>

#define DMA_BUF_SIZE	0x10000000 //256 MB (the same as Universe II)

TUVMEDMADevice::TUVMEDMADevice(): TUVMEDevice((uint32_t)-1)
{
  fUseNoIncrement = false;
}

TUVMEDMADevice::~TUVMEDMADevice()
{
	Close();
}

int32_t TUVMEDMADevice::Open()  
{
	std::ostringstream os;
	if (fDevNumber >= (int32_t)kNumberOfDevices) {
		return -1; //Error
	}

	os << "/dev/"<< GetDeviceStringName();
	      
	if ((fFileNum = open(os.str().c_str(), O_RDWR)) < 0) {
		fIsOpen = false;
		return fFileNum; // Error
	}

	fIsOpen = true;

	for (unsigned int i = 1; i < 16; i++) {
		fDMABuf = mmap((void*) (i * 0x10000000UL), (size_t) DMA_BUF_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_FIXED, -1, (size_t) 0);
		if (fDMABuf == MAP_FAILED) fDMABuf = NULL;
		else break;
	}

	if (fDMABuf == NULL) return -1; //error
	//todo: if it fails, try harder with MAP_32BIT and do the page alignment manualy
	return 0;
}

static void SwapShortBlock(void* p, int32_t n)
{
	int16_t* sp = (int16_t*)p;
	int32_t i;
	for(i=0;i<n;i++){
		int16_t x = *sp;
		*sp =  (((uint16_t)(x)) << 8) |    
			(((uint16_t)(x)) >> 8) ;
		sp++;
	}
}

static void SwapLongBlock(void* p, int32_t n)
{
	int32_t* lp = (int32_t*)p;
	int32_t i;
	for(i=0;i<n;i++){
		int32_t x = *lp;
		*lp = (((uint32_t)(x)) << 24) |    
			(((uint32_t)(x) & (uint32_t)0x0000FF00) <<  8) |    
			(((uint32_t)(x) & (uint32_t)0x00FF0000) >>  8) |    
			(((uint32_t)(x)) >> 24);
		lp++;
	}
}

static void SwapLongLongBlock(void* p, int32_t n)
{
	uint64_t* lp = (uint64_t*)p;
	int32_t i;
	for(i=0;i<n;i++){
		uint64_t x = *lp;
		*lp = (((uint64_t)(x)) << 56) |
		      ((((uint64_t)(x)) & (uint64_t)0x000000000000FF00ULL) << 40) |
		      ((((uint64_t)(x)) & (uint64_t)0x0000000000FF0000ULL) << 24) |
		      ((((uint64_t)(x)) & (uint64_t)0x00000000FF000000ULL) <<  8) |
		      ((((uint64_t)(x)) & (uint64_t)0x000000FF00000000ULL) >>  8) |
		      ((((uint64_t)(x)) & (uint64_t)0x0000FF0000000000ULL) >> 24) |
		      ((((uint64_t)(x)) & (uint64_t)0x00FF000000000000ULL) >> 40) |
		      (((uint64_t)(x)) >> 56);
		lp++;
	}
}

int32_t TUVMEDMADevice::Read(char* buffer, uint32_t numBytes, uint32_t offset)
{
	int rc = 0;

	fDmaDesc.dir = VME_DMA_FROM_DEVICE;
	fDmaDesc.length = numBytes;
	fDmaDesc.novmeinc = fUseNoIncrement;
	switch (fDataWidth) {
		case VME_D8:
		case VME_D16:
			fDmaDesc.src.data_width = VME_D16;
			break;
		case VME_D32:
		case VME_D64:
			fDmaDesc.src.data_width = VME_D32;
			break;
		default:
			break;
	}

	fDmaDesc.src.am = fAddressModifier;

	fDmaDesc.src.addru = 0;
	fDmaDesc.src.addrl = fVMEAddress + offset;

	fDmaDesc.dst.addru = 0;
	fDmaDesc.dst.addrl = (unsigned long) fDMABuf;

	memset(fDMABuf, 0, numBytes);
	rc = ioctl(fFileNum, VME_IOCTL_START_DMA, &fDmaDesc);

        switch (fDataWidth) {
		case VME_D8:
			break;
		case VME_D16:
			SwapShortBlock(fDMABuf, numBytes/2);
			break;
		case VME_D32:
			SwapLongBlock(fDMABuf, numBytes/4);
			break;
		case VME_D64:
			//SwapLongLongBlock(fDMABuf, numBytes/8);
			//this is to enforce UniverseII/ORCA compatibility
			SwapLongBlock(fDMABuf, numBytes/4);
			break;
	}

	memcpy((void*) buffer, fDMABuf, numBytes);
	if (rc == 0) rc = numBytes;

	return rc;
}

int32_t TUVMEDMADevice::Write(char* buffer, uint32_t numBytes, uint32_t offset)
{
	int rc = 0;

	fDmaDesc.dir = VME_DMA_TO_DEVICE;
	fDmaDesc.length = numBytes;
	fDmaDesc.novmeinc = fUseNoIncrement;
	switch (fDataWidth) {
		case VME_D8:
		case VME_D16:
			fDmaDesc.dst.data_width = VME_D16;
			break;
		case VME_D32:
		case VME_D64:
			fDmaDesc.dst.data_width = VME_D32;
			break;
		default:
			break;
	}

	fDmaDesc.dst.am = fAddressModifier;

	fDmaDesc.dst.addru = 0;
	fDmaDesc.dst.addrl = fVMEAddress + offset;

	fDmaDesc.src.addru = 0;
	fDmaDesc.src.addrl = (unsigned long) fDMABuf;

	memcpy(fDMABuf, (void*) buffer, numBytes);
        switch (fDataWidth) {
		case VME_D8:
			break;
		case VME_D16:
			SwapShortBlock(fDMABuf, numBytes/2);
			break;
		case VME_D32:
			SwapLongBlock(fDMABuf, numBytes/4);
			break;
		case VME_D64:
			//SwapLongLongBlock(fDMABuf, numBytes/8);
			//UniverseII/ORCA compatibility
			SwapLongBlock(fDMABuf, numBytes/4);
			break;
	}

	rc = ioctl(fFileNum, VME_IOCTL_START_DMA, &fDmaDesc);
	if (rc == 0) rc = numBytes;

	return rc;
}

int TUVMEDMADevice::Enable()
{
        memset(&fDmaDesc, 0, sizeof(struct vme_dma));

	fDmaDesc.ctrl.pci_block_size = VME_DMA_BSIZE_4096;
	fDmaDesc.ctrl.pci_backoff_time = VME_DMA_BACKOFF_0;
	fDmaDesc.ctrl.vme_block_size = VME_DMA_BSIZE_4096;
	fDmaDesc.ctrl.vme_backoff_time = VME_DMA_BACKOFF_0;

	return 0;
}


void TUVMEDMADevice::Close() 
{
	if (fIsOpen) {
		close(fFileNum);
		if (fDMABuf) munmap(fDMABuf, (size_t) DMA_BUF_SIZE);
		fIsOpen = false;
	}
}

