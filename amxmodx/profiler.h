#ifndef _PROFILER_H_
#define _PROFILER_H_

#include "amxmodx.h"

#include "profilerbase.h" // messy platform specific stuff
#include "CVector.h"
#include "CString.h"
#include "sh_stack.h"
#include "amx.h"

#ifndef __PROFILER_DECL
#	ifdef _WIN32
#		define __PROFILER_DECL __cdecl
#	else
#		define __PROFILER_DECL __attribute__((cdecl))
#	endif
#endif

// Inserted into the JIT
extern "C" void profilerSetupCallback(void* start, void* end, void* reloc);

// Callbacks from the JIT
int __PROFILER_DECL profilerReloc(AMX* amx, void* jitcode, void* cip); // Called during compilation
void __PROFILER_DECL profilerStart(void* plugin, cell  index); // Called before a profiled function starts
void __PROFILER_DECL profilerEnd(void* plugin, cell index); // Called after a profiled function ends

class NativeProfile : public BaseProfile
{
protected:
	String m_Name;
	
public:
	NativeProfile() : BaseProfile() { }
	NativeProfile(const char* name) : BaseProfile(), m_Name(name) 	{	}
	
	const String& getName() const { return this->m_Name; }
	void setName(const char* name) { this->m_Name.assign(name); }
};

class FunctionProfile : public BaseProfile
{
protected:
	String m_Name;
	ucell m_Address;

public:
	FunctionProfile() : BaseProfile() { }
	FunctionProfile(const char* name) : BaseProfile(), m_Name(name) { }

	const String& getName() const { return this->m_Name; }
	void setName(const char* name) { this->m_Name.assign(name); }

	ucell getAddress() const { return this->m_Address; }
	void setAddress(ucell addr) { this->m_Address = addr; }
};
class ForwardProfile : public BaseProfile
{
protected:
	String m_Name;

public:
	ForwardProfile() : BaseProfile() { }
	ForwardProfile(const char* name) : BaseProfile(), m_Name(name) { }

	const String& getName() { return this->m_Name; }
	void setName(const char* name) { this->m_Name.assign(name); }
};

typedef struct funcsymbol_s
{
	ucell		addr;
	String		name;
} funcsymbol_t;
class PluginProfile
{
protected:
	CVector<NativeProfile> m_Natives;
	CVector<ForwardProfile> m_Forwards;
	CVector<FunctionProfile> m_Functions;
	CVector<funcsymbol_t> m_Funclist;
	String m_PluginName;
	CStack<TemporaryProfile*> m_TempStack;

public:
	PluginProfile() { }
	~PluginProfile()
	{
		while (m_TempStack.size() > 0)
		{
			delete m_TempStack.front();

			m_TempStack.pop();
		}
	}
	void Setup(AMX* amx);

	void StartProfile(BaseProfile* parent);
	void StopNextProfile();

	void StartNative(cell index) { this->StartProfile(&(m_Natives[index])); }
	void StopNative(cell index) { this->StopNextProfile(); }

	void StartForward(cell index) { this->StartProfile(&(m_Forwards[index])); }
	void StopForward(cell index) { this->StopNextProfile(); }

	void StartFunction(cell index) { this->StartProfile(&(m_Functions[index])); }
	void StopFunction(cell index) { this->StopNextProfile(); }

	void PrintProfile(FILE* fp);
	void PrintProfile();
	void AddFunction(ucell addr, ucell origaddr);

	int FindFunctionIndexByAddress(ucell addr);

	void InsertFunctionSymbol(const char* name, ucell addr);
	int LookupFunctionSymbol(ucell addr);
};

// This needs to be extern to clear it from stale profiles each map change
extern CVector<TemporaryProfile*> g_ProfileList;
extern AMX_NATIVE_INFO profile_Natives[];

#endif // _PROFILER_H_
