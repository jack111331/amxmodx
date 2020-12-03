#include "amxmodx.h"

#include "profilerbase.h" // messy platform specific stuff
#include "CVector.h"
#include "CString.h"

#include "amx.h"

#include <time.h>

CVector<TemporaryProfile*> g_ProfileList;

extern "C" void const *amx_opcodelist_jit[];
extern "C" AMX* jit_amxptr;

#define USENAMETABLE(hdr) \
                        ((hdr)->defsize==sizeof(AMX_FUNCSTUBNT))
#define NUMENTRIES(hdr,field,nextfield) \
                        (unsigned)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)
#define GETENTRY(hdr,table,index) \
                        (AMX_FUNCSTUB *)((unsigned char*)(hdr) + (unsigned)(hdr)->table + (unsigned)index*(hdr)->defsize)
#define GETENTRYNAME(hdr,entry) \
                        ( USENAMETABLE(hdr) \
                           ? (char *)((unsigned char*)(hdr) + (unsigned)((AMX_FUNCSTUBNT*)(entry))->nameofs) \
                           : ((AMX_FUNCSTUB*)(entry))->name )

// Plugin is loading
// Look for natives and public functions here
// Unfortunately we cannot look for non-public functions as easily here
// so instead we do it when the JIT is relocating addresses (amx_Browserelocate or whatever)
// we will check for OP_PROC
void PluginProfile::Setup(AMX* amx)
{
		AMX_HEADER* hdr = reinterpret_cast<AMX_HEADER*>(amx->base);
		AMX_FUNCSTUB* func;

		// get the native count
		unsigned int nativecount = NUMENTRIES(hdr, natives, libraries);

		this->m_Natives.resize(nativecount);

		for (unsigned int i = 0; i < nativecount; i++)
		{
			func = GETENTRY(hdr, natives, i);
			const char* name = GETENTRYNAME(hdr, func);

			this->m_Natives[i].setName(name);

			//printf("setup: %s\n", name);

		}

		// do the same for public functions
		unsigned int forwardcount = NUMENTRIES(hdr, publics, natives);

		this->m_Forwards.resize(forwardcount);

		for (unsigned int i = 0; i < forwardcount; i++)
		{
			func = GETENTRY(hdr, publics, i);
			const char* name = GETENTRYNAME(hdr, func);

			this->m_Forwards[i].setName(name);
		}
		

		CPluginMngr::CPlugin* plugin = reinterpret_cast<CPluginMngr::CPlugin*>(amx->userdata[3]);
		this->m_PluginName.assign(plugin->getName());

}
// This will in-turn call PrintProfile(FILE* fp) with a file opened for the
// plugin's profile data to be dumped to.  By default, it opens up
// $amxmodx$/data/profiles/$plugin.amxx$.txt
void PluginProfile::PrintProfile()
{
	const char *data = get_localinfo("amxmodx_datadir", "addons/amxmodx/data");
	char path[255];
	build_pathname_r(path, sizeof(path)-1, "%s/profiles", data);
	
	if (!DirExists(path))
	{
		mkdir(path
#if defined __linux__
			, 0755
#endif
			);
		if (!DirExists(path))
			return;
	}

	char file[512];

	build_pathname_r(file, sizeof(file)-1, "%s/profiles/%s.txt", data, this->m_PluginName.c_str());
	FILE* fp = fopen(file, "a");
//	printf("Opening %s for writing...\n", file);
	if (!fp)
	{
		AMXXLOG_Error("Unable to open %s for writing, cannot output profile data!", file);
	}
	else
	{
		this->PrintProfile(fp);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

// This dumps the plugin's profile information to a file
// To dump to console, pass stdout
void PluginProfile::PrintProfile(FILE* fp)
{
	time_t rawtime;
	time(&rawtime);


	String thetime;

	thetime.assign(asctime(localtime(&rawtime)));
	int loc = thetime.find('\n');
	thetime.erase(loc, 1);
	fprintf(fp, "date: %s map: %s\n", thetime.c_str(), STRING(gpGlobals->mapname));
	fprintf(fp, "%4s | %32s | %10s | %-12s\n", "type", "name", "calls", "time / min / max");
	fprintf(fp, "%4s---%32s---%10s---%12s\n", "----", "--------------------------------", "----------", "------------");

	unsigned int native0 = 0;
	unsigned int public0 = 0;
	unsigned int function0 = 0;
	for (size_t i = 0; i < this->m_Natives.size(); i++)
	{
		NativeProfile* p = &this->m_Natives[i];

		if (p->getCalls() == 0)
		{
			native0++;
		}
		else
		{
			fprintf(fp, "%4s | %32s | %10u | %lf / %lf / %lf\n", "n", p->getName().c_str(), p->getCalls(), p->getTime(), p->getMin(), p->getMax());
		}
	}
	for (size_t i = 0; i < this->m_Forwards.size(); i++)
	{
		ForwardProfile* p = &this->m_Forwards[i];

		if (p->getCalls() == 0)
		{
			public0++;
		}
		else
		{
			fprintf(fp, "%4s | %32s | %10u | %lf / %lf / %lf\n", "p", p->getName().c_str(), p->getCalls(), p->getTime(), p->getMin(), p->getMax());
		}
	}
	for (size_t i = 0; i < this->m_Functions.size(); i++)
	{
		FunctionProfile* p = &this->m_Functions[i];

		if (p->getCalls() == 0)
		{
			function0++;
		}
		else
		{
			fprintf(fp, "%4s | %32s | %10u | %lf / %lf / %lf\n", "f", p->getName().c_str(), p->getCalls(), p->getTime(), p->getMin(), p->getMax());
		}
	}

	if (native0 != 0 ||
		public0 != 0 ||
		function0 != 0)
	{
		fprintf(fp, "%d natives, %d public callbacks, %d function calls were not executed.\n", native0, public0, function0);
	}
}

// This will stop the profile on the top of the profile stack
// Additionally, if there is a profile next in line, it will continue that's profile's execution
void PluginProfile::StopNextProfile()
{
	if (this->m_TempStack.size() == 0)
	{
		// This should NEVER happen
		AMXXLOG_Error("m_TempStack is empty!");
		return;
	}
	this->m_TempStack.front()->End();
	delete this->m_TempStack.front();

	m_TempStack.pop();

	if (this->m_TempStack.size() != 0)
	{
		m_TempStack.front()->Continue();
	}
}


// This will create a TemporaryProfile object with a pointer back to the base profile
// Additionally, this will pause the currently active profile from within the plugin
// and cause push the new temp profile object onto the tempprofile stack
void PluginProfile::StartProfile(BaseProfile* parent)
{
	if (this->m_TempStack.size() != 0)
	{
		this->m_TempStack.front()->Pause();
	}
	TemporaryProfile* p = new TemporaryProfile(parent);

	this->m_TempStack.push(p);
	p->Start();
}

// This will look for a function address (JIT-relocated address)
// and return its address if found.
// If not found (shouldn't happen, but...) it will return -1
int PluginProfile::FindFunctionIndexByAddress(ucell addr)
{
	for (size_t i = 0; i < this->m_Functions.size(); i++)
	{
		if (this->m_Functions[i].getAddress() == addr)
		{
			return i;
		}
	}

	return -1;
}

// Adds a function to the plugin's internal list.  The first
// address field is the JIT-relocated address field.
void PluginProfile::AddFunction(ucell addr, ucell origaddr)
{
	this->m_Functions.push_back(FunctionProfile());

	FunctionProfile* fp = &(this->m_Functions.back());

	fp->setAddress(addr);

	int index = this->LookupFunctionSymbol(origaddr);

	if (index < 0)
	{
		char buff[32];

		snprintf(buff, sizeof(buff) - 1, "unknown-0x%08X", origaddr);

		fp->setName(buff);
	}
	else
	{
		fp->setName(this->m_Funclist[index].name.c_str());
	}
}

// Inserts a function symbol into the plugin's internal list.
// The address provided is the standard address, NOT the JIT-relocated address
void PluginProfile::InsertFunctionSymbol(const char* name, ucell addr)
{
	funcsymbol_t sym;

	sym.name.assign(name);
	sym.addr = addr;

	this->m_Funclist.push_back(sym);


}

// Finds a function symbol by address, if one exists, returns its index.
// If one does not exist, it returns -1.
int PluginProfile::LookupFunctionSymbol(ucell addr)
{
	for (size_t i = 0; i < this->m_Funclist.size(); i++)
	{
		if (this->m_Funclist[i].addr == addr)
		{
			return i;
		}
	}

	return -1;
}

// This is called when a TemporaryProfile is stopped.
// This will take the time that TemporaryProfile was executing
// and add it back to the base profile's time.
void BaseProfile::AnalyzeTemporaryProfile(TemporaryProfile* in)
{
#ifdef _WIN32
	double time;
			
	this->TimeDifference(in->m_Start, in->m_End, time);
			
#else
	int sec;
	int usec;
	
	this->TimeDifference(in->m_Start, in->m_End, sec, usec);
	
	double time;

	time = TIMEVALTODOUBLE(sec, usec);
#endif

	this->m_Calls++;
	// If this is the first call, then set the time for the min and max
	if (this->m_Calls == 1)
	{
		this->m_Max = time;
		this->m_Min = time;			
	}
	else
	{
		if (this->m_Max < time)
		{
			this->m_Max = time;
		}
		else if (this->m_Min > time)
		{
			this->m_Min = time;
		}
	}
	this->m_Time += time;
}

// Callback from within the JIT
// This is called right before an OP_CALL from within the JIT
// Provided is the CPlugin pointer (cast to void because I'm lazy and don't want to fiddle with include files)
// and the function index from within the CPlugin->getProfile() internal function list
void __PROFILER_DECL profilerStart(void* plugin, cell index)
{
	
	reinterpret_cast<CPluginMngr::CPlugin*>(plugin)->getProfile().StartFunction(index);
}

// This is called immediately after an OP_CALL finishes
// The parameters provided here are identical to the prarameters provided in profilerStart()
void __PROFILER_DECL profilerEnd(void* plugin, cell index)
{
	reinterpret_cast<CPluginMngr::CPlugin*>(plugin)->getProfile().StopFunction(index);
}
#define OP_CALL 49
// profilerReloc()'s purpose is essentially to modify the code the JIT will be copying for the CALL opcode
// There are two x86 call functions inserted inside of the CALL opcode now; one before, and one after the actual call
// each of these two functions have two "push dword" instructions before it, with the generic 12345678h parameters
// This needs to be changed to CPlugin pointer, and function index.
int __PROFILER_DECL profilerReloc(AMX* amx, void* jitcode, void* cip)
{
	// We are given an AMX_HEADER pointer (which is (AMX*)amx->base)
	// the pointer to the start of the jit code that needs to be modified
	// a pointer to the cip

	// Lastly, return 0 if we are not profiling this plugin, 1 otherwise
	
	char* cod = reinterpret_cast<char*>(amx->base) + (reinterpret_cast<AMX_HEADER*>(amx->base)->cod);

	unsigned int diff = reinterpret_cast<unsigned int>(cip) - reinterpret_cast<unsigned int>(cod);


	cod += diff;

	// cod now points to the CALL opcode
	// move up the size of a cell to get the address
	ucell op = *reinterpret_cast<ucell*>(cod);

	// ensure that the op is CALL

	if (op != reinterpret_cast<ucell>(amx_opcodelist_jit[OP_CALL]))
	{
		return 0;
	}
	cod += sizeof(cell);
	ucell addr = *reinterpret_cast<ucell*>(cod);

	CPluginMngr::CPlugin* plugin = reinterpret_cast<CPluginMngr::CPlugin*>(amx->userdata[3]);

	if (plugin == NULL)
	{
		return 0;
	}

	if (!plugin->isProfiling())
	{
		return 0;
	}

	int index = plugin->getProfile().FindFunctionIndexByAddress(addr);

	if (index == -1)
	{
		return 0;
	}

#define JCODE(offset) (reinterpret_cast<unsigned char*>(jitcode) + offset)
#define	JPARAM(offset) (reinterpret_cast<ucell*>(reinterpret_cast<unsigned char*>(jitcode) + offset))

	// The way jitcode should look is as follows:
	//   0:   60                      pusha
	//   1:   68 78 56 34 12          push   $0x12345678
	//   6:   68 78 56 34 12          push   $0x12345678
	//   b:   ff 15 00 00 00 00       call   *0x0
	//  11:   83 c4 08                add    $0x8,%esp
	//  14:   61                      popa
	//
	//00000015 <j_call>:
	//  15:   8d 76 fc                lea    0xfffffffc(%esi),%esi
	//  18:   c7 06 00 00 00 00       movl   $0x0,(%esi)
	//
	//0000001e <j_call_e8>:
	//  1e:   e8 00 00 00 00          call   23 <j_call_e8+0x5>
	//  23:   60                      pusha
	//  24:   68 78 56 34 12          push   $0x12345678
	//  29:   68 78 56 34 12          push   $0x12345678
	//  2e:   ff 15 04 00 00 00       call   *0x4
	//  34:   83 c4 08                add    $0x8,%esp
	//  37:   61                      popa


	// For profile start, need to change offset 2 to function index, and offset 7 to CPlugin pointer


	*JPARAM(2) = *reinterpret_cast<ucell*>(&index);
	*JPARAM(7) = reinterpret_cast<ucell>(plugin);

	// For profile end, need to change offset 37 to function index, and offset 42 to CPlugin pointer


	*JPARAM(37) = *reinterpret_cast<ucell*>(&index);
	*JPARAM(42) = reinterpret_cast<ucell>(plugin);


	return 1;

}


// native Profile:StartProfile()
static cell AMX_NATIVE_CALL StartProfile(AMX* amx, cell* params)
{
	TemporaryProfile* p = new TemporaryProfile;

	// Iterate the vector, if any of the entries are NULL use them
	for (size_t i = 0; i < g_ProfileList.size(); i++)
	{
		if (g_ProfileList[i] == NULL)
		{
			g_ProfileList[i] = p;

			p->Start();

			return i + 1;
		}
	}

	// No NULL entries, make a new one
	g_ProfileList.push_back(p);

	p->Start();

	return g_ProfileList.size();
}
// native Float:StopProfile(&Profile:what, const buffer[] = "", len = 0)
static cell AMX_NATIVE_CALL StopProfile(AMX* amx, cell* params)
{
	cell* handle;
	if (amx_GetAddr(amx, params[1], &handle) == AMX_ERR_NONE)
	{
		cell handleid = *handle;

		// handles are automatically incremented by 1 so that 0 would be considered invalid
		if (handleid < 1)
		{
			LogError(amx, AMX_ERR_NATIVE, "Invalid handle passed to StopProfile");
			return 0;
		}
		handleid--;

		if ((unsigned)handleid >= g_ProfileList.size())
		{
			LogError(amx, AMX_ERR_NATIVE, "Invalid handle passed to StopProfile");
			return 0;
		}


		TemporaryProfile* p = g_ProfileList[handleid];

		if (p == NULL)
		{
			LogError(amx, AMX_ERR_NATIVE, "Invalid handle passed to StopProfile");
			return 0;
		}

		p->End();

		// Now, NULL out the vector entry

		g_ProfileList[handleid] = NULL;

		// and reset the handle passed in
		*handle = 0;

#ifdef _WIN32
		unsigned __int64 temp;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&temp));

		p->m_Frequency = 1.0 / (double)temp;

		double time;

		p->TimeDifference(p->m_Start, p->m_End, time);

		// params[2] is buffer, params[3] is length of the buffer
		if (params[3] > 0)
		{
			char buffer[256];

			snprintf(buffer, sizeof(buffer) - 1, "%lf", time);

			set_amxstring(amx, params[2], buffer, params[3]);
		}

		float ret = static_cast<float>(time);
		return amx_ftoc(ret);
#else

		int seconds;
		int useconds;

		double time;

		p->TimeDifference(p->m_Start, p->m_End, seconds, useconds);

		time = TIMEVALTODOUBLE(seconds, useconds);
		// params[2] is buffer, params[3] is length of the buffer
		if (params[3] > 0)
		{
			char buffer[256];

			snprintf(buffer, sizeof(buffer) - 1, "%lf", time);

			set_amxstring(amx, params[2], buffer, params[3]);
		}

		float ret = static_cast<float>(time);
		return amx_ftoc(ret);


#endif


	}
	LogError(amx, AMX_ERR_NATIVE, "Unable to get the handle for StopProfile, cannot continue.");

	return 0;
}

AMX_NATIVE_INFO profile_Natives[] = {

	{ "StartProfile",		StartProfile },
	{ "StopProfile",		StopProfile },

	{ NULL,					NULL }
};
