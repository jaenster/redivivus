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

	.text
	.globl	__ZN9_Wrappers20CRT_ExceptionFilter1EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv
__ZN9_Wrappers20CRT_ExceptionFilter1EP17EHExceptionRecordP18EHRegistrationNodeP8_CONTEXTPv:
	xorl	%eax, %eax
	ret
