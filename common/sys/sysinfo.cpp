// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "sysinfo.h"
#include "intrinsics.h"
#include "string.h"

////////////////////////////////////////////////////////////////////////////////
/// All Platforms
////////////////////////////////////////////////////////////////////////////////

namespace embree
{
  std::string getPlatformName() 
  {
#if defined(__LINUX__) && !defined(__X86_64__)
    return "Linux (32bit)";
#elif defined(__LINUX__) && defined(__X86_64__)
    return "Linux (64bit)";
#elif defined(__FREEBSD__) && !defined(__X86_64__)
    return "FreeBSD (32bit)";
#elif defined(__FREEBSD__) && defined(__X86_64__)
    return "FreeBSD (64bit)";
#elif defined(__CYGWIN__) && !defined(__X86_64__)
    return "Cygwin (32bit)";
#elif defined(__CYGWIN__) && defined(__X86_64__)
    return "Cygwin (64bit)";
#elif defined(__WIN32__) && !defined(__X86_64__)
    return "Windows (32bit)";
#elif defined(__WIN32__) && defined(__X86_64__)
    return "Windows (64bit)";
#elif defined(__MACOSX__) && !defined(__X86_64__)
    return "Mac OS X (32bit)";
#elif defined(__MACOSX__) && defined(__X86_64__)
    return "Mac OS X (64bit)";
#elif defined(__UNIX__) && !defined(__X86_64__)
    return "Unix (32bit)";
#elif defined(__UNIX__) && defined(__X86_64__)
    return "Unix (64bit)";
#else
    return "Unknown";
#endif
  }

  std::string getCompilerName()
  {
#if defined(__INTEL_COMPILER)
    int icc_mayor = __INTEL_COMPILER / 100 % 100;
    int icc_minor = __INTEL_COMPILER % 100;
    std::string version = "Intel Compiler ";
    version += toString(icc_mayor);
    version += "." + toString(icc_minor);
#if defined(__INTEL_COMPILER_UPDATE)
    version += "." + toString(__INTEL_COMPILER_UPDATE);
#endif
    return version;
#elif defined(__clang__)
    return "CLANG " __clang_version__;
#elif defined (__GNUC__)
    return "GCC " __VERSION__;
#elif defined(_MSC_VER)
    std::string version = toString(_MSC_FULL_VER);
    version.insert(4,".");
    version.insert(9,".");
    version.insert(2,".");
    return "Visual C++ Compiler " + version;
#else
    return "Unknown Compiler";
#endif
  }

  std::string getCPUVendor()
  {
    return "ARM ARM ARM ";
  }

  CPUModel getCPUModel() 
  {
    return CPU_ARM;

  }

  std::string stringOfCPUModel(CPUModel model)
  {
    switch (model) {
    case CPU_KNC             : return "Knights Corner";
    case CPU_CORE1           : return "Core1";
    case CPU_CORE2           : return "Core2";
    case CPU_CORE_NEHALEM    : return "Nehalem";
    case CPU_CORE_SANDYBRIDGE: return "SandyBridge";
    case CPU_HASWELL         : return "Haswell";
    default                  : return "Unknown CPU";
    }
  }

  /* constants to access destination registers of CPUID instruction */
  static const int EAX = 0;
  static const int EBX = 1;
  static const int ECX = 2;
  static const int EDX = 3;

  /* cpuid[eax=1].ecx */
  static const int CPU_FEATURE_BIT_SSE3   = 1 << 0;
  static const int CPU_FEATURE_BIT_SSSE3  = 1 << 9;
  static const int CPU_FEATURE_BIT_FMA3   = 1 << 12;
  static const int CPU_FEATURE_BIT_SSE4_1 = 1 << 19;
  static const int CPU_FEATURE_BIT_SSE4_2 = 1 << 20;
  //static const int CPU_FEATURE_BIT_MOVBE  = 1 << 22;
  static const int CPU_FEATURE_BIT_POPCNT = 1 << 23;
  //static const int CPU_FEATURE_BIT_XSAVE  = 1 << 26;
  static const int CPU_FEATURE_BIT_OXSAVE = 1 << 27;
  static const int CPU_FEATURE_BIT_AVX    = 1 << 28;
  static const int CPU_FEATURE_BIT_F16C   = 1 << 29;
  static const int CPU_FEATURE_BIT_RDRAND = 1 << 30;

  /* cpuid[eax=1].edx */
  static const int CPU_FEATURE_BIT_SSE  = 1 << 25;
  static const int CPU_FEATURE_BIT_SSE2 = 1 << 26;

  /* cpuid[eax=0x80000001].ecx */
  static const int CPU_FEATURE_BIT_LZCNT = 1 << 5;

  /* cpuid[eax=7,ecx=0].ebx */
  static const int CPU_FEATURE_BIT_BMI1    = 1 << 3;
  static const int CPU_FEATURE_BIT_AVX2    = 1 << 5;
  static const int CPU_FEATURE_BIT_BMI2    = 1 << 8;
  static const int CPU_FEATURE_BIT_AVX512F = 1 << 16;     // AVX512F  (foundation)
  static const int CPU_FEATURE_BIT_AVX512DQ = 1 << 17;    // AVX512DQ (doubleword and quadword instructions)
  static const int CPU_FEATURE_BIT_AVX512PF = 1 << 26;    // AVX512PF (prefetch gather/scatter instructions)
  static const int CPU_FEATURE_BIT_AVX512ER = 1 << 27;    // AVX512ER (exponential and reciprocal instructions)
  static const int CPU_FEATURE_BIT_AVX512CD = 1 << 28;    // AVX512CD (conflict detection instructions)
  static const int CPU_FEATURE_BIT_AVX512BW = 1 << 30;    // AVX512BW (byte and word instructions)
  static const int CPU_FEATURE_BIT_AVX512VL = 1 << 31;    // AVX512VL (vector length extensions)
  static const int CPU_FEATURE_BIT_AVX512IFMA = 1 << 21;  // AVX512IFMA (integer fused multiple-add instructions)
  
  /* cpuid[eax=7,ecx=0].ecx */
  static const int CPU_FEATURE_BIT_AVX512VBMI = 1 << 1;   // AVX512VBMI (vector bit manipulation instructions)
  static const int CPU_FEATURE_BIT_NEON = 1 << 10;

  int getCPUFeatures()
  {
     return CPU_FEATURE_NEON|CPU_FEATURE_SSE2|CPU_FEATURE_SSE;
  }

  std::string stringOfCPUFeatures(int features)
  {
    std::string str;
    if (features & CPU_FEATURE_SSE   ) str += "SSE ";
    if (features & CPU_FEATURE_SSE2  ) str += "SSE2 ";
    if (features & CPU_FEATURE_SSE3  ) str += "SSE3 ";
    if (features & CPU_FEATURE_SSSE3 ) str += "SSSE3 ";
    if (features & CPU_FEATURE_SSE41 ) str += "SSE4.1 ";
    if (features & CPU_FEATURE_SSE42 ) str += "SSE4.2 ";
    if (features & CPU_FEATURE_POPCNT) str += "POPCNT ";
    if (features & CPU_FEATURE_AVX   ) str += "AVX ";
    if (features & CPU_FEATURE_F16C  ) str += "F16C ";
    if (features & CPU_FEATURE_RDRAND) str += "RDRAND ";
    if (features & CPU_FEATURE_AVX2  ) str += "AVX2 ";
    if (features & CPU_FEATURE_FMA3  ) str += "FMA3 ";
    if (features & CPU_FEATURE_LZCNT ) str += "LZCNT ";
    if (features & CPU_FEATURE_BMI1  ) str += "BMI1 ";
    if (features & CPU_FEATURE_BMI2  ) str += "BMI2 ";
    if (features & CPU_FEATURE_KNC   ) str += "KNC ";
    if (features & CPU_FEATURE_AVX512F) str += "AVX512F ";
    if (features & CPU_FEATURE_AVX512DQ) str += "AVX512DQ ";
    if (features & CPU_FEATURE_AVX512PF) str += "AVX512PF ";
    if (features & CPU_FEATURE_AVX512ER) str += "AVX512ER ";
    if (features & CPU_FEATURE_AVX512CD) str += "AVX512CD ";
    if (features & CPU_FEATURE_AVX512BW) str += "AVX512BW ";
    if (features & CPU_FEATURE_AVX512VL) str += "AVX512VL ";
    if (features & CPU_FEATURE_AVX512IFMA) str += "AVX512IFMA ";
    if (features & CPU_FEATURE_AVX512VBMI) str += "AVX512VBMI ";
    if (features & CPU_FEATURE_NEON) str += "NEON ";
    return str;
  }
  
  std::string stringOfISA (int isa)
  {
    if (isa == SSE) return "SSE";
    if (isa == SSE2) return "SSE2";
    if (isa == SSE3) return "SSE3";
    if (isa == SSSE3) return "SSSE3";
    if (isa == SSE41) return "SSE4.1";
    if (isa == SSE42) return "SSE4.2";
    if (isa == AVX) return "AVX";
    if (isa == AVX2) return "AVX2";
    if (isa == AVX512KNL) return "AVX512KNL";
    if (isa == AVX512SKX) return "AVX512SKX";
    if (isa == KNC) return "KNC";
    if (isa == NEON) return "NEON";
    return "UNKNOWN";
  }

  bool hasISA(int features, int isa) {
    return (features & isa) == isa;
  }
  
  std::string supportedTargetList (int features)
  {
    std::string v;
    if (hasISA(features,SSE)) v += "SSE ";
    if (hasISA(features,SSE2)) v += "SSE2 ";
    if (hasISA(features,SSE3)) v += "SSE3 ";
    if (hasISA(features,SSSE3)) v += "SSSE3 ";
    if (hasISA(features,SSE41)) v += "SSE4.1 ";
    if (hasISA(features,SSE42)) v += "SSE4.2 ";
    if (hasISA(features,AVX)) v += "AVX ";
    if (hasISA(features,AVXI)) v += "AVXI ";
    if (hasISA(features,AVX2)) v += "AVX2 ";
    if (hasISA(features,AVX512KNL)) v += "AVX512KNL ";
    if (hasISA(features,AVX512SKX)) v += "AVX512SKX ";
    if (hasISA(features,KNC)) v += "KNC ";
    if (hasISA(features,NEON)) v += "NEON";
    return v;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Windows Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace embree
{
  std::string getExecutableFileName() {
    char filename[1024];
    if (!GetModuleFileName(nullptr, filename, sizeof(filename))) return std::string();
    return std::string(filename);
  }

  unsigned int getNumberOfLogicalThreads() 
  {
    static int nThreads = -1;
    if (nThreads != -1) return nThreads;

    typedef WORD (WINAPI *GetActiveProcessorGroupCountFunc)();
    typedef DWORD (WINAPI *GetActiveProcessorCountFunc)(WORD);
    HMODULE hlib = LoadLibrary("Kernel32");
    GetActiveProcessorGroupCountFunc pGetActiveProcessorGroupCount = (GetActiveProcessorGroupCountFunc)GetProcAddress(hlib, "GetActiveProcessorGroupCount");
    GetActiveProcessorCountFunc      pGetActiveProcessorCount      = (GetActiveProcessorCountFunc)     GetProcAddress(hlib, "GetActiveProcessorCount");

    if (pGetActiveProcessorGroupCount && pGetActiveProcessorCount) 
    {
      int groups = pGetActiveProcessorGroupCount();
      int totalProcessors = 0;
      for (int i = 0; i < groups; i++) 
        totalProcessors += pGetActiveProcessorCount(i);
      nThreads = totalProcessors;
    }
    else
    {
      SYSTEM_INFO sysinfo;
      GetSystemInfo(&sysinfo);
      nThreads = sysinfo.dwNumberOfProcessors;
    }
    return nThreads;
  }

  int getTerminalWidth() 
  {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) return 80;
    CONSOLE_SCREEN_BUFFER_INFO info = { { 0 } };
    GetConsoleScreenBufferInfo(handle, &info);
    return info.dwSize.X;
  }

  double getSeconds() 
  {
    LARGE_INTEGER freq, val;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&val);
    return (double)val.QuadPart / (double)freq.QuadPart;
  }

  void sleepSeconds(double t) {
    Sleep(DWORD(1000.0*t));
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Linux Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX__)

#include <stdio.h>
#include <unistd.h>

namespace embree
{
  std::string getExecutableFileName() 
  {
    char pid[32]; sprintf(pid, "/proc/%d/exe", getpid());
    char buf[1024];
    int bytes = readlink(pid, buf, sizeof(buf)-1);
    if (bytes != -1) buf[bytes] = '\0';
    return std::string(buf);
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// FreeBSD Platform
////////////////////////////////////////////////////////////////////////////////

#if defined (__FreeBSD__)

#include <sys/sysctl.h>

namespace embree
{
  std::string getExecutableFileName()
  {
    const int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    char buf[1024];
    size_t len = sizeof(buf);
    if (sysctl(mib, 4, buf, &len, 0x0, 0) == -1) *buf = '\0';
    return std::string(buf);
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// Mac OS X Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__MACOSX__)

#include <mach-o/dyld.h>

namespace embree
{
  std::string getExecutableFileName()
  {
    char buf[1024];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) return std::string();
    return std::string(buf);
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// Unix Platform
////////////////////////////////////////////////////////////////////////////////

#if defined(__UNIX__)

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>

namespace embree
{
  unsigned int getNumberOfLogicalThreads() 
  {
    static int nThreads = -1;
    if (nThreads == -1) nThreads = sysconf(_SC_NPROCESSORS_CONF);
    return nThreads;
  }

  int getTerminalWidth() 
  {
    struct winsize info;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &info) < 0) return 80;
    return info.ws_col;
  }

  double getSeconds() {
    struct timeval tp; gettimeofday(&tp,nullptr);
    return double(tp.tv_sec) + double(tp.tv_usec)/1E6;
  }

  void sleepSeconds(double t) {
    usleep(1000000.0*t);
  }
}
#endif

