/*
Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
Copyright 2017-2019 University of Huddersfield.
Licensed under the BSD-3 License.
See license.md file in the project root for full license information.
This project has received funding from the European Research Council (ERC)
under the European Union’s Horizon 2020 research and innovation programme
(grant agreement No 725899).
*/

#pragma once

#include <FluidVersion.hpp>

#include <clients/common/FluidBaseClient.hpp>
#include <clients/common/OfflineClient.hpp>
#include <clients/common/ParameterSet.hpp>
#include <clients/common/ParameterTypes.hpp>

#include <AudioFile/IAudioFile.h>
#include <AudioFile/OAudioFile.h>

#include <cctype>
#include <iostream>
#include <iomanip>
#include <regex>
#include <string>
#include <vector>
#include <utility>

namespace fluid {
namespace client {
    
class CLIBufferAdaptor : public BufferAdaptor
{
  
public:
  
  CLIBufferAdaptor(const std::string str)
  : mPath(str), mAcquired(false), mNumChans(1)
  {
    // TODO: only read if needed!...
    
    HISSTools::IAudioFile file(mPath);
    
    if (file.isOpen())
    {
      resize(file.getFrames(), file.getChannels(), file.getSamplingRate());
        
      for (uint16_t i = 0; i < file.getChannels(); i++)
      {
        file.seek();
        file.readChannel(getChannel(i), file.getFrames(), i);
      }
    }
  }
  
  void write(bool allowCSV)
  {
    if (!numFrames())
      return;
    
    index pathLength = mPath.length();
    
    if (allowCSV && (pathLength > 4) && !mPath.compare(pathLength - 4, 4, ".csv"))
    {
      FluidTensorView<const float, 2> view{mData.data(), 0, numChans(), numFrames()};
      
      std::ofstream file(mPath.c_str());
      
      if (file.is_open())
      {
        file << view;
        file.close();
      }
    }
    else
    {
      // TODO: file extensions/paths
      
      constexpr auto fileType = HISSTools::BaseAudioFile::kAudioFileWAVE;
      constexpr auto depthType = HISSTools::BaseAudioFile::kAudioFileFloat32;
      
      uint16_t chans = static_cast<uint16_t>(
                          std::min(
                            asSigned(std::numeric_limits<uint16_t>::max()),
                            numChans())
                      );
      
      HISSTools::OAudioFile file(mPath, fileType, depthType, chans, mSamplingRate);
      
      if (file.isOpen())
      {
        for (uint16_t i = 0; i < numChans(); i++)
        {
          file.seek();
          file.writeChannel(getChannel(i), static_cast<uint32_t>(numFrames()), i);
        }
      }
    }
  }
  
private:
  
  bool acquire() const override   { return !mAcquired && (mAcquired = true); }
  void release() const override   { mAcquired = false; }
  
  bool valid() const override { return numFrames(); }
  bool exists() const override  { return true; }
  
  double sampleRate() const override { return mSamplingRate; }
  
  const float *getChannel(index channel) const { return mData.data() + numFrames() * channel; }
  float *getChannel(index channel) { return mData.data() + numFrames() * channel; }
  
  const Result resize(index frames, index channels, double sampleRate) override
  {
    std::vector<std::vector<float>> newData;
    
    mNumChans = channels;
    mData.resize(channels * frames);
    mSamplingRate = sampleRate;
    return {};
  }
  
  fluid::FluidTensorView<float, 1> samps(index channel) override
  {
    return {getChannel(channel), 0, numFrames()};
  }
  
  fluid::FluidTensorView<float, 1> samps(index offset, index nframes, index chanoffset) override
  {
    index length = offset > numFrames() ? 0 : numFrames() - offset;
    return {getChannel(chanoffset) + offset, 0, std::min(length, nframes)};
  }
  
  fluid::FluidTensorView<const float, 1> samps(index channel) const override
  {
    return fluid::FluidTensorView<const float, 1>{getChannel(channel), 0, numFrames()};
  }
  
  fluid::FluidTensorView<const float, 1> samps(index offset, index nframes, index chanoffset) const override
  {
    index length = offset > numFrames() ? 0 : numFrames() - offset;
    return fluid::FluidTensorView<const float, 1>{getChannel(chanoffset) + offset, 0, std::min(length, nframes)};
  }
  
  index numFrames() const override { return mData.size() / mNumChans; }
  index numChans() const override { return mNumChans; }
  
  std::string asString() const override { return mPath; }
  
  std::string mPath;
  mutable bool mAcquired;
  double mSamplingRate = 44100.0;
  index mNumChans;
  std::vector<float> mData;
};

template <template <typename T> class Client>
class CLIWrapper
{
  enum ErrorType
  {
    kErrNone,
    kErrNoOption,
    kErrUnknownOption,
    kErrAlreadySet,
    kErrMissingVals,
    kErrValType
  };
  
  using ClientType = Client<float>;
  using ParamSetType = typename ClientType::ParamSetType;
  using ConstString = const std::string;
  
  static constexpr auto& descriptors() { return ClientType::getParameterDescriptors(); }
  static constexpr index nParams = descriptors().size();
  
  template <size_t N>
  static constexpr auto paramDescriptor() { return descriptors().template get<N>(); }
    
  template <size_t N>
  static constexpr index paramSize() { return paramDescriptor<N>().fixedSize; }
  
  template <size_t N>
  static std::string optionName()
  {
    std::string str(paramDescriptor<N>().name);
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    str.insert(0, "-");
    
    return str;
  }
    
  using FlagsType = std::array<bool, nParams>;
  
  template <size_t L, size_t N = 0>
  struct ValidateParams
  {
    bool numeric(ConstString s, bool d = true)
    {
      return s.find_first_not_of("0123456789.", s[0] == '-', d ? 11 : 10) == std::string::npos;
    };
      
    bool testString(ConstString s, LongT::type)   { return numeric(s, false); }
    bool testString(ConstString s, FloatT::type)  { return numeric(s) && s.find(".") == s.find_last_of("."); }
    bool testString(ConstString s, BufferT::type) { return s[0] != '-'; }
    bool testString(ConstString s, InputBufferT::type) { return s[0] != '-'; }

    template<typename T>
    ErrorType checkValues(index& i, index argc, const char* argv[], index nArgs, T)
    {
      if (!(i + nArgs < argc))
        return kErrMissingVals;
      
      for (index j = 1; j <= nArgs; j++)
        if (!testString(argv[i + j], T()))
          return kErrValType;
      
      i += 1 + nArgs;
      return kErrNone;
    }
    
    ErrorType operator()(index& i, index argc, const char* argv[], FlagsType& flags)
    {
      using T = typename ClientType::ParamDescType::template ParamType<N>;
      using ArgType = typename ParamLiteralConvertor<T, paramSize<N>()>::LiteralType;
        
      if (argv[i][0] != '-' || numeric(argv[i]))
        return kErrNoOption;
      
      if (!strcmp(argv[i], optionName<N>().c_str()))
      {
        if (flags[N])
          return kErrAlreadySet;
        flags[N] = true;
        return checkValues(i, argc, argv, paramSize<N>(), ArgType());
      }
      
      return ValidateParams<L, N + 1>()(i, argc, argv, flags);
    }
  };
  
  template <size_t N>
  struct ValidateParams<N, N>
  {
    ErrorType operator()(index&, index, const char*[], FlagsType&)
    {
      return kErrUnknownOption;
    }
  };
  
  template <size_t N, typename T>
  struct Setter
  {
    auto fromString(ConstString s, LongT::type) { return std::stol(s); }
    auto fromString(ConstString s, FloatT::type) { return std::stod(s); }
    auto fromString(ConstString s, BufferT::type) { return BufferT::type(new CLIBufferAdaptor(s)); }
    auto fromString(ConstString s, InputBufferT::type) { return InputBufferT::type(new CLIBufferAdaptor(s)); }

    typename T::type operator()(index argc, const char* argv[])
    {
      for (index i = 0; i < argc; ++i)
      {
        if (!strcmp(argv[i], optionName<N>().c_str()))
        {
          ParamLiteralConvertor<T, paramSize<N>()> a;

          for (index j = 0; j < paramSize<N>(); ++j)
            a[j] = fromString(argv[i + j + 1], a[0]);
          
          return a.value();
        }
      }
      
      return paramDescriptor<N>().defaultValue;
    }
  };
  
  template <size_t N, typename T>
  struct Help
  {
    void operator()(T descriptor)
    {
      std::cout << std::setw(25) << std::setfill(' ');
      std::cout.unsetf(std::ios::right);
      std::cout.setf(std::ios::left);
      std::cout << optionName<N>();
      std::cout.unsetf(std::ios::left);
      std::cout.setf(std::ios::right);
      std::cout.unsetf(std::ios::right);
      std::cout << descriptor.displayName << "\n";
    }
  };
   
  template<size_t N, typename T>
  struct WriteFiles
  {
    void operator()(typename T::type& param, bool allowCSV)
    {
      if (param)
        static_cast<CLIBufferAdaptor *>(param.get())->write(allowCSV);
    }
  };
  
public:
  
  static void report(const char* str1, const char * str2)
  {
    std::cout << str1 << " " << str2 << "\n";
  }
    
  static int run(index argc, const char* argv[])
  {
    ParamSetType params(descriptors());
    FlagsType flags;
    flags.fill(false);
    
    const std::regex help("(-*)h(elp)?");
    const std::regex version("(-*)v(ersion)?");
    std::cmatch m;   
    if (argc > 1 && std::regex_match(argv[1],m,help))      
    {
      std::cout << "Fluid Corpus Manipulation Toolkit, version " << fluidVersion() << '\n';
      std::cout << "Part of the Fluid Corpus Manipulation Project - http:://www.flucoma.org/\n";
      std::cout << "For a more detailed description of the available options than given below, please see the accompanying HTML documentation.\n"; 
      std::cout << "Call with these options:\n";
      descriptors().template iterate<Help>();
      return 0;
    }
    
    if (argc > 1 && std::regex_match(argv[1],m,version))
    {
      std::cout << "Fluid Corpus Manipulation Toolkit, version " << fluidVersion() << '\n';
      return 0;
    }
                            
    for (index i = 1; i < argc; )
    {
      switch (ValidateParams<nParams>()(i, argc, argv, flags))
      {
        case kErrNone:            break;
        case kErrNoOption:        report("Expected option, but found", argv[i]);    return -2;
        case kErrUnknownOption:   report("Unknown option", argv[i]);                return -3;
        case kErrAlreadySet:      report("More than one use of option", argv[i]);   return -4;
        case kErrMissingVals:     report("Missing values for option", argv[i]);     return -5;
        case kErrValType:         report("Values wrong type for option", argv[i]);  return -6;
      }
    }
      
    params.template setParameterValues<Setter>(false, argc, argv);
    params.constrainParameterValues();
    
    // Create client after all parameters are set
    FluidContext context;
    ClientType client(params);
    auto result = client.process(context);
    
    if (!result.ok())
    {
      // Output error
      
      std::cerr << result.message() << "\n";
    }
    else
    {
      // Write files
      
      bool allowCSV = true;
      params.template forEachParamType<BufferT, WriteFiles>(allowCSV);
    }
      
    return result.ok() ? 0 : -1;
  }
};
    
} // namespace client
} // namespace fluid
