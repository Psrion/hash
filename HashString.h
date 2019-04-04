#ifndef __HASHSTRING_H__
#define __HASHSTRING_H__

#define MAXCRYPTLEN   0x500
#define MAXTABLELEN   2048
#define MAXFILELEN    258

typedef unsigned char uchar;
typedef unsigned long ulong;

typedef struct  _HASHTABLE
{  
    long nHashA;  
    long nHashB;  
    bool bExists; 
	char cFileName[MAXFILELEN];
}HASHTABLE, *PHASHTABLE;



class HashString 
{

public:
	HashString();
	~HashString();
	
	bool Hash(char * lpszString);
	int handleDelFile(const char* pDirName);
	
private:	

	int InitHashTable(void);
	void InitCryptTable(void);
	ulong CalHashString(char *lpszString, ulong dwHashType);
	long Hashed(char * lpszString);
	
	ulong nTableLength;
	ulong m_tablelength;    // 哈希索引表长度  
	ulong cryptTable[MAXCRYPTLEN];
	HASHTABLE *m_HashIndexTable;
	
	int collc;
	int errstr;
};


#endif //__HASHSTRING_H__

