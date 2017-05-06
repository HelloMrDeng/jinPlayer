// $Change: 395037 $
// mpif.h: interface for the mpif class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPIF_H__D294B1A4_32F6_465F_A7E9_37504CB52419__INCLUDED_)
#define AFX_MPIF_H__D294B1A4_32F6_465F_A7E9_37504CB52419__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "columbus_typedefs.h"
#include "columbus_mpif.h"
//#include "ceddkex.h"
#include "ceddk.h"
#include "columbus_base_regs.h"
#include "columbus_chiptop.h"
#include <CEDDK.h>


class Cmpif
{
public:
	void AeonBoot();
	BOOL CarinaInit();
	void MpifReadBuffer(int addr, int len, char *buf);
	void MpifWriteBuffer(int addr, int len, char *buf);
	void load8051(LPCTSTR lpFileName);
	BOOL DataFilePut(LPCTSTR lpFileName, DWORD addr);
	void Carina3ATestMIU();
	void Carina3ATestRIU();
	void Carina3ATest();
	void Carina2ATestF0F0();
	void Carina2ATestXIU();
	void Carina_2A_Test();
	void Carina_1A_Test();
	void MpifStop();
	BOOL MpifTestInit(void);
	MPIF_IOCONTROL_CODE_et m_dwMpifBitMode;
	pmstar_multi_pif_put_st m_hydra_put_2A;
	pmstar_multi_pif_put_st m_hydra_put;
	pmstar_multi_pif_get_st m_hydra_get;
	BOOL m_bFlagExitTest;
	volatile PREG_MPIFDMA_st m_pMPIFDMARegs;
	MPIF_IOCONTROL_CODE_et m_mpifClk;
	UINT8 *m_rx;
	UINT8 *m_tx;
	pmstar_multi_pif_register_st m_hydra_register;
	MPIF_OPTIONS_st m_sOption;
	void MIUInit(void);
	BOOL MpifInit(void);
	Cmpif();
	virtual ~Cmpif();

};

#endif // !defined(AFX_MPIF_H__D294B1A4_32F6_465F_A7E9_37504CB52419__INCLUDED_)
