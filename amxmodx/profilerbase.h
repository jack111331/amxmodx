#ifndef _PROFILERBASE_H_
#define _PROFILERBASE_H_
// BaseProfile is the nasty class with a lot of the OS-dependant features in it for profiling stuff
// TemporaryProfile is the object created for the actual profiling, BaseProfile's start/stop/pause/cont functions are mostly unused now (I dont remember if i removed them, and scrolling down there is so much work)
#define PROFILE_STARTED 0
#define PROFILE_IDLE    1
#define PROFILE_PAUSED  2

#ifdef _WIN32
// Windows uses the stupid QueryPerformanceCounter crap
#	include <windows.h>
#else
// Linux uses gettimeofday
#	include <sys/time.h>
#endif

#define TIMEVALTODOUBLE(_SEC, _USEC) ((double)(_SEC + (_USEC * 0.000001)))
class TemporaryProfile;
class BaseProfile
{
protected:
#ifdef _WIN32
	double m_Frequency;
	
	unsigned __int64 m_Start;
	unsigned __int64 m_End;
	unsigned __int64 m_Pause;
	
#else
	struct timeval m_Start;
	
	// time the function finished
	struct timeval m_End;
	
	// time the function started being paused
	struct timeval m_Pause;
#endif
	double m_Time;

	double m_Min;
	double m_Max;

	int m_State;
	unsigned int m_ActiveCalls;
	unsigned int m_Calls;
#ifdef _WIN32
	void TimeDifference(unsigned __int64& start, unsigned __int64& end, double& time)
	{
		time = (end - start) * this->m_Frequency;
	};
#else
	void TimeDifference(struct timeval& start, struct timeval& end, int& seconds, int& useconds)
	{
		seconds = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		
		if (useconds < 0)
		{
			// If useconds is less than zero, then a full second hasn't really passed, we need to compensate
			
			seconds -= 1; // fix the seconds count
			
			// now fix the microseconds
			
			useconds = 1000000 + useconds; // useconds is negative, so add it
		}

	};
#endif
	
		
	
	void setup()
	{
#ifdef _WIN32
		unsigned __int64 temp;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&temp));

		this->m_Frequency = 1.0 / (double)temp;
#endif
	};
public:
	BaseProfile()
	{
		m_Time = 0.0;
		m_Min = 0.0;
		m_Max = 0.0;

		m_State = PROFILE_IDLE;
		m_ActiveCalls = 0;
		m_Calls = 0;
		setup();
	};

	unsigned int getCalls() const { return this->m_Calls; }
	double getMin() const { return this->m_Min; }
	double getMax() const { return this->m_Max; }
	double getTime() const { return this->m_Time; }

	void Start()
	{
		if (this->m_State == PROFILE_IDLE)
		{
			this->m_State = PROFILE_STARTED;
#ifdef _WIN32
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_Start));
#else
			gettimeofday(&this->m_Start, NULL);
#endif
		}
		this->m_ActiveCalls++;
		this->m_Calls++;
		
	}
	void Pause()
	{
		if (this->m_State != PROFILE_STARTED)
		{
			return;
		}
#ifdef _WIN32
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_Pause));
#else
		gettimeofday(&this->m_Pause, NULL);

#endif
		
		this->m_State = PROFILE_PAUSED;
	}
	void Continue()
	{
		if (this->m_State != PROFILE_PAUSED)
		{
			return;
		}
		this->m_State = PROFILE_STARTED;

#ifdef _WIN32
		unsigned __int64 now;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&now));
		
		
		this->m_Start += (now - this->m_Pause);
#else
		struct timeval now;
		int sec;
		int usec;
		
		gettimeofday(&now, NULL);
		
		this->TimeDifference(this->m_Pause, now, sec, usec);
		
		// Now add the second/usecond count to the Start tracker
		
		this->m_Start.tv_sec += sec;
		this->m_Start.tv_usec += usec;
#endif
		
	}
	
	void End()
	{
		if (this->m_State != PROFILE_STARTED)
		{
			return;
		}
		this->m_ActiveCalls--;

		if (this->m_ActiveCalls == 0)
		{
#ifdef _WIN32
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_End));

			double time;
			
			this->TimeDifference(this->m_Start, this->m_End, time);
			
#else
			gettimeofday(&this->m_End, NULL);
			
			int sec;
			int usec;
			
			this->TimeDifference(this->m_Start, this->m_End, sec, usec);
			
			double time;

			time = TIMEVALTODOUBLE(sec, usec);
#endif

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
			this->m_State = PROFILE_IDLE;
		}
	}
	void AnalyzeTemporaryProfile(TemporaryProfile* in);
};
class TemporaryProfile
{
public:
	BaseProfile *m_Parent;
#ifdef _WIN32
	unsigned __int64 m_Start;
	unsigned __int64 m_End;
	unsigned __int64 m_Pause;
	double m_Frequency;
	
#else
	struct timeval m_Start;
	
	// time the function finished
	struct timeval m_End;
	
	// time the function started being paused
	struct timeval m_Pause;
#endif
#ifdef _WIN32
	void TimeDifference(unsigned __int64& start, unsigned __int64& end, double& time)
	{
		time = (end - start) * this->m_Frequency;
	};
#else
	void TimeDifference(struct timeval& start, struct timeval& end, int& seconds, int& useconds)
	{
		seconds = end.tv_sec - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		
		if (useconds < 0)
		{
			// If useconds is less than zero, then a full second hasn't really passed, we need to compensate
			
			seconds -= 1; // fix the seconds count
			
			// now fix the microseconds
			
			useconds = 1000000 + useconds; // useconds is negative, so add it
		}

	};
#endif
	
		
public:
	TemporaryProfile() { };
	TemporaryProfile(BaseProfile* parent) : m_Parent(parent) { };

	void Start()
	{
#ifdef _WIN32
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_Start));
#else
		gettimeofday(&this->m_Start, NULL);
#endif
	}
	void Pause()
	{
#ifdef _WIN32
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_Pause));
#else
		gettimeofday(&this->m_Pause, NULL);

#endif
	}
	void Continue()
	{
#ifdef _WIN32
		unsigned __int64 now;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&now));
		
		// Now add the second/usecond count to the Start tracker
		
		this->m_Start += (now - this->m_Pause);
#else
		struct timeval now;
		int sec;
		int usec;
		
		gettimeofday(&now, NULL);
		
		this->TimeDifference(this->m_Pause, now, sec, usec);
		
		// Now add the second/usecond count to the Start tracker
		
		this->m_Start.tv_sec += sec;
		this->m_Start.tv_usec += usec;
#endif
		
	}
	
	void End()
	{
#ifdef _WIN32
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&this->m_End));
#else
		gettimeofday(&this->m_End, NULL);
#endif
		if (this->m_Parent != NULL)
		{
			this->m_Parent->AnalyzeTemporaryProfile(this);
		}
	}
};

#endif // _PROFILERBASE_H_
