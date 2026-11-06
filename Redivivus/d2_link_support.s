	.file	"d2_link_support.s"
# Definitions that let the reconstructed tree link into an executable.
#
# Data symbols carry their REAL bytes, read out of Game.exe - see
# docs/data/recovered-data-symbols.tsv. Function entries are stubs that return,
# with the __stdcall ones popping their arguments so the caller's stack
# survives. A stub does nothing: that is a runtime defect, not a link one.
#
# Everything here is temporary. The generator emits real bodies for these once
# the exclusion closure lands; this file shrinks to nothing as it does.

	.section .rdata,"dr"
	.globl	_DWORD_006db8d0
_DWORD_006db8d0:
	.long	3

	.globl	_DWORD_006db8d4
_DWORD_006db8d4:
	.long	5

	.globl	_UNK_006da48c
_UNK_006da48c:
	.long	148

	.globl	_UNK_006dff20
_UNK_006dff20:
	.long	9

	.globl	_UNK_006fcc24
_UNK_006fcc24:
	.long	12

	.text
	.globl	_FID_conflict____CxxFrameHandler3
_FID_conflict____CxxFrameHandler3:
	xorl	%eax, %eax
	ret


	.globl	__ZN9_Wrappers20CRT_ExceptionFilter1EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv
__ZN9_Wrappers20CRT_ExceptionFilter1EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv:
	xorl	%eax, %eax
	ret

	.globl	__ZN9_Wrappers20CRT_ExceptionFilter2EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv
__ZN9_Wrappers20CRT_ExceptionFilter2EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv:
	xorl	%eax, %eax
	ret

	.globl	__ZN9_Wrappers20CRT_ExceptionFilter3EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv
__ZN9_Wrappers20CRT_ExceptionFilter3EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv:
	xorl	%eax, %eax
	ret

	.globl	__ZN9_Wrappers16gCmdLineHelpTextEPtjPh
__ZN9_Wrappers16gCmdLineHelpTextEPtjPh:
	xorl	%eax, %eax
	ret

	.globl	___CIsqrt
___CIsqrt:
	xorl	%eax, %eax
	ret



	.globl	___sqrt_common
___sqrt_common:
	xorl	%eax, %eax
	ret

	.globl	__ZN8compiler23CRT_GetItemDataAndFlagsEPvPm@8
__ZN8compiler23CRT_GetItemDataAndFlagsEPvPm@8:
	xorl	%eax, %eax
	ret	$8

