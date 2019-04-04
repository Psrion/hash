#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "HashString.h"



HashString::HashString()	
{
	collc = 0;
	errstr = 0;
	m_tablelength = 0;
	nTableLength = MAXTABLELEN;
	
	InitHashTable();
	
}

HashString::~HashString()
{
	if(NULL != m_HashIndexTable)
	{
		free(m_HashIndexTable);
		m_HashIndexTable = NULL;
	}
}

void HashString::InitCryptTable(void)  
{   
    ulong seed = 0x00100001, index1 = 0, index2 = 0;
	uchar i;  
 
    for( index1 = 0; index1 < 0x100; index1++ )  
    {   
        for( index2 = index1, i = 0; i < 5; i++, index2 += 0x100 )  
        {   
            ulong temp1, temp2;  
            seed = (seed * 125 + 3) % 0x2AAAAB;  
            temp1 = (seed & 0xFFFF) << 0x10;  
            seed = (seed * 125 + 3) % 0x2AAAAB;  
            temp2 = (seed & 0xFFFF);  
            cryptTable[index2] = ( temp1 | temp2 );   
        }   
    }   
} 

int HashString::InitHashTable()
{
    int i;
    
    InitCryptTable();  
    m_tablelength = nTableLength;
    
    m_HashIndexTable = (HASHTABLE *)malloc(nTableLength * sizeof(HASHTABLE));
    if (NULL == m_HashIndexTable) {
        return -1;
    }

    for (i = 0; i < nTableLength; i++ )  
    {  
        m_HashIndexTable[i].nHashA = 0;  
        m_HashIndexTable[i].nHashB = 0;  
        m_HashIndexTable[i].bExists = false;
		memset(m_HashIndexTable[i].cFileName, '\0', MAXFILELEN);
    }
    
    return 0;    
}

ulong HashString::CalHashString(char *lpszString, ulong dwHashType)
{
    unsigned char *key = (unsigned char *)lpszString;
    unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
    int ch; 

    while(*key != 0)
    {   
        ch = toupper(*key++);

        seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
        seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
    }   
    return seed1;
}


long HashString::Hashed(char * lpszString)  
{   
    const ulong HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;  
   
    ulong nHash = CalHashString(lpszString, HASH_OFFSET);  
    ulong nHashA = CalHashString(lpszString, HASH_A);  
    ulong nHashB = CalHashString(lpszString, HASH_B);  
    ulong nHashStart = nHash % m_tablelength;  
    ulong nHashPos = nHashStart;  

 
    while (m_HashIndexTable[nHashPos].bExists)  
    {   
        if (m_HashIndexTable[nHashPos].nHashA == nHashA && m_HashIndexTable[nHashPos].nHashB == nHashB)   
            return nHashPos;   
        else  
            nHashPos = (nHashPos + 1) % m_tablelength;  
 
        if (nHashPos == nHashStart)   
            break;   
    }  
    errstr++;
 
    return -1;  
} 

bool HashString::Hash(char * lpszString)
{  
    const ulong HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;  
    ulong nHash = CalHashString(lpszString, HASH_OFFSET);  
    ulong nHashA = CalHashString(lpszString, HASH_A);  
    ulong nHashB = CalHashString(lpszString, HASH_B);  
    ulong nHashStart = nHash % m_tablelength; 
    ulong nHashPos = nHashStart;  

    while (m_HashIndexTable[nHashPos].bExists)  
    {    
        nHashPos = (nHashPos + 1) % m_tablelength;  
		
		//Execute one cycle  
        if (nHashPos == nHashStart) 
        { 
            collc ++; 
            return false;   
        }  
    }  
    m_HashIndexTable[nHashPos].bExists = true;  
    m_HashIndexTable[nHashPos].nHashA = nHashA;  
    m_HashIndexTable[nHashPos].nHashB = nHashB; 

	memset(m_HashIndexTable[nHashPos].cFileName, '\0', MAXFILELEN);
	memcpy(m_HashIndexTable[nHashPos].cFileName, lpszString, strlen(lpszString));

    return true;  
}


int HashString::handleDelFile(const char* pDirName)
{
	if(0 != access(pDirName, F_OK))
		return -1;
	
	DIR *pDirFd = opendir(pDirName);
	
	if(NULL == pDirFd)
	{
		if(0 == remove(pDirName))
		{
			mkdir(pDirName, 0777);
		}
		
		return -1;
	}

	long lHashRet = 0;
	char cFilePath[512] = {'\0'};
	struct dirent *pstDir = NULL;
	
	while(NULL != (pstDir = readdir(pDirFd)))
	{
		if(   0 == strncmp(pstDir->d_name, ".", 1) 
		    || 0 == strncmp(pstDir->d_name, "..", 2) )
		{
			continue;
		}
	
		lHashRet = Hashed(pstDir->d_name);
		if(-1 == lHashRet)
		{
			memset(cFilePath, '\0', 512);
			sprintf(cFilePath, "%s/%s", pDirName, pstDir->d_name);
			remove(cFilePath);			
		}

//		else if(nTableLength > lHashRet)
//		{
//			if(0 != strcmp(m_HashIndexTable[lHashRet].cFileName, pstDir->d_name))
//			{
//				memset(cFilePath, '\0', 512);
//				sprintf(cFilePath, "%s/%s", pDirName, pstDir->d_name);
//				remove(cFilePath);
//			}
//		}

//		else
//		{		
//			closedir(pDirFd);
//			return -1;
//		}

		
	}

	closedir(pDirFd);
	
	return 0;
		
}


