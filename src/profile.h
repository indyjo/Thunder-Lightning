#ifndef TNL_PROFILE_H
#define TNL_PROFILE_H

#ifdef ENABLE_PROFILE

#include <map>
#include <fstream>
#include <string>

struct _Profiler {
	struct Step {
		Uint32 tmax, tmin;
		double tavg,tvar;
		std::string name;
		int ncalls;
		Step() : ncalls(0) { }
	};
	typedef std::map<std::string, Step> StepsByName;

	std::ofstream out;
	int niterations;
	Uint32 t_lastbegin,t_begin;

	StepsByName steps;
	
	_Profiler(const char * filename)
		: niterations(0)
		, out(filename)
		, t_begin(SDL_GetTicks())
	{
		out << "Beginning profile session." << std::endl;
	}

	void begin() {
		t_lastbegin = SDL_GetTicks();
		niterations++;
	}

	void finish(const char * stepname) {
		Uint32 t_end = SDL_GetTicks();
		Uint32 duration = t_end - t_lastbegin;
		double dur_double = duration;

		Step & step = steps[stepname];
		if (step.ncalls == 0) {
			step.name = stepname;
			step.tmin = step.tmax = duration;
			step.tvar = 0;
			step.tavg = dur_double;
		} else {
			if (duration < step.tmin) step.tmin = duration;
			if (duration > step.tmax) step.tmax = duration;
			float v = 1.0/step.ncalls, u = 1.0 - v;
			step.tavg = u*step.tavg + v*dur_double;
			step.tvar = u*step.tvar + v*(dur_double - step.tavg)*(dur_double - step.tavg);
		}
		//out << stepname << ": " << step.ncalls << " - " << duration << "ms" << std::endl;
		step.ncalls++;
		t_lastbegin = SDL_GetTicks();
	}

	~_Profiler() {
		Uint32 t_end = SDL_GetTicks();
		out << "---------------------------------------" << std::endl;
		out << "Finished after " << niterations << " iterations." << std::endl;

		double totalsum = 0;
		for(StepsByName::iterator i=steps.begin(); i!=steps.end(); ++i) {
			const Step & step = i->second;
			double total = step.ncalls*step.tavg*0.001;
			totalsum += total;
			out << step.name << ": avg=" << step.tavg << " min=" << step.tmin << " max=" << step.tmax
				<< " stddev=" << sqrt(step.tvar) << " total=" << total
				<< "s in " << step.ncalls << " calls" << std::endl;
		}
		double total_time = (t_end-t_begin)*0.001;
		out << "Total: " << totalsum << "s. This corresponds with " << 100*(totalsum/total_time)
			<< "% of the total running time of " << total_time << "s." << std::endl;
	}
};


#define BEGIN_PROFILE(filename) \
	static _Profiler _profiler(filename); \
	_profiler.begin();

#define FINISH_PROFILE_STEP(step) \
	_profiler.finish(step);

#else
#define BEGIN_PROFILE(x) ;
#define FINISH_PROFILE_STEP(x) ;
#endif


#endif

