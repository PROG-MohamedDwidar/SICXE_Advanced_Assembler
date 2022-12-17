.data

opNames dq ?
opFormats dq ?
opCodes dq ?
sicAsmCode dq ?
loctr dq ?
endpoint byte '.'
searchLoc qword ?
param1 qword ?
param2 qword ?
inflag byte 0
inflag2 byte 0
store1 qword ?
store2 qword ?
store3 qword ?
offset1 qword 0
offset2 qword 0
indexctr qword 0
equalflag byte 0
len1 byte 0
len2 byte 0
lenstart qword ?
calclenRet byte 0
.code
init proc
	mov opNames, rcx
	mov opFormats, rdx
	mov opCodes,r8
	mov sicAsmCode,r9
   	ret              
init endp 

calclen proc
	
	ret
calclen endp


isEqual proc
	mov lenstart, r14
	call calclen
	mov cl, calclenRet
	mov len1, cl

	mov lenstart, r13
	call calclen
	mov cl, calclenRet
	mov len2, cl


	ret
isEqual endp

getSize proc
	mov r13, opNames;
	mov inflag2, 0
do:
	mov bl, byte ptr[r13]
	cmp bl, endpoint
	je don
	cmp bl, '_'
	je seperator

col:
	cmp inflag2, 0
	jne ncol
	mov inflag2, 1
	mov store2, r13
	call isEqual
	mov r13, store2

ncol:

	jmp endsep
seperator:
	mov inflag2, 0

endsep:
	
	inc r13
	jmp do
don:
	ret
getSize endp

passOne proc
	mov loctr,rcx
	mov r14, sicAsmCode

do: 
	mov al, byte ptr[r14]
	cmp al,endpoint
	je don
	cmp al, '_'
	je seperator

col:
	cmp inflag, 0
	jne noCol
	mov inflag, 1
	mov store1, r14
	call getSize
	mov r14, store1

noCol:
	

	
	jmp endsep
seperator:
	mov inflag, 0
endsep:

	add r14,1

	jmp do

don:
	ret
passOne endp

end