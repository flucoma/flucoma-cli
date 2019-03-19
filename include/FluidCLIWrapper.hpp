
#pragma once

#include <clients/common/FluidBaseClient.hpp>
#include <clients/common/OfflineClient.hpp>
#include <clients/common/ParameterSet.hpp>
#include <clients/common/ParameterTypes.hpp>

#include <HISSTools_AudioFile/IAudioFile.h>
#include <HISSTools_AudioFile/OAudioFile.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

namespace fluid {
namespace client {
    
class CLIBufferAdaptor : public BufferAdaptor
{
  
public:
  
  CLIBufferAdaptor(const std::string str)
  : mPath(str), mWrite(false), mAcquired(false), mRank(1)
  {
    // TODO: only read if needed!...
    
    HISSTools::IAudioFile file(mPath);
    
    if (file.isOpen())
    {
      resize(file.getFrames(), file.getChannels(), 1);
      mWrite = false;
      mSamplingRate = file.getSamplingRate();
        
      for (auto i = 0; i < file.getChannels(); i++)
      {
        file.seek();
        file.readChannel(mData[i].data(), file.getFrames(), i);
      }
    }
  }
  
  ~CLIBufferAdaptor()
  {
    if (!mWrite || !numFrames())
      return;
    
    // TODO: file extensions/paths
      
    constexpr auto fileType = HISSTools::BaseAudioFile::kAudioFileWAVE;
    constexpr auto depthType = HISSTools::BaseAudioFile::kAudioFileFloat32;
      
    HISSTools::OAudioFile file(mPath, fileType, depthType, mData.size(), mSamplingRate);
      
    if (file.isOpen())
    {
      for (auto i = 0; i < mData.size(); i++)
      {
        file.seek();
        file.writeChannel(mData[i].data(), static_cast<uint32_t>(numFrames()), i);
      }
    }
  }
  
private:
  
  bool acquire() override   { return !mAcquired && (mAcquired = true); }
  void release() override   { mAcquired = false; }
  
  bool valid() const override { return numFrames(); }
  bool exists() const override  { return true; }
  
  void resize(size_t frames, size_t channels, size_t rank) override
  {
    std::vector<std::vector<float>> newData;
    
    newData.resize(channels * rank);
    for (auto &&c : newData)
      c.resize(frames);
    
    std::swap(newData, mData);
    mWrite = true;
  }
  
  fluid::FluidTensorView<float, 1> samps(size_t channel, size_t rankIdx) override
  {
    return {mData[rankIdx * numChans() + channel].data(), 0, numFrames()};
  }
  
  fluid::FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    size_t length = offset > numFrames() ? 0 : numFrames() - offset;
    return {mData[chanoffset].data() + offset, 0, length};
  }
  
  size_t numFrames() const override { return mData.size() ? mData[0].size() : 0; }
  size_t numChans() const override { return mData.size() / mRank; }
  size_t rank() const override { return mRank; }
  
  std::string mPath;
  bool mWrite;
  bool mAcquired;
  size_t mRank;
  double mSamplingRate = 44100.0;
  std::vector<std::vector<float>> mData;
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
  using ParamDescType = typename ClientType::ParamDescType;
  using ParamSetType = typename ClientType::ParamSetType;
  using ConstString = const std::string;
  
  template <size_t N>
  using ParamArgType = ParamLiteralType<typename ParamDescType::template DescriptorTypeAt<N>>;
  
  static constexpr auto descriptors() { return ClientType::getParameterDescriptors(); }
  
  static constexpr size_t nParams = descriptors().size();
  
  template <size_t N>
  static constexpr size_t paramSize() { return descriptors().template get<N>().fixedSize; }
  
  template <size_t N>
  static constexpr auto paramDefault() { return descriptors().template get<N>().defaultValue; }
  
  template <size_t N>
  static std::string optionName()
  {
    std::string str(descriptors().template name<N>());
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
    
    template<typename T>
    ErrorType checkValues(int& i, int argc, const char* argv[], size_t nArgs, T)
    {
      if (!(i + nArgs < argc))
        return kErrMissingVals;
      
      for (size_t j = 1; j <= nArgs; j++)
        if (!testString(argv[i + j], T()))
          return kErrValType;
      
      i += 1 + nArgs;
      return kErrNone;
    }
    
    ErrorType operator()(int& i, int argc, const char* argv[], FlagsType& flags)
    {
      if (argv[i][0] != '-' || numeric(argv[i]))
        return kErrNoOption;
      
      if (!strcmp(argv[i], optionName<N>().c_str()))
      {
        if (flags[N])
          return kErrAlreadySet;
        flags[N] = true;
        return checkValues(i, argc, argv, paramSize<N>(), ParamArgType<N>());
      }
      
      return ValidateParams<L, N + 1>()(i, argc, argv, flags);
    }
  };
  
  template <size_t N>
  struct ValidateParams<N, N>
  {
    ErrorType operator()(int& i, int argc, const char* argv[], FlagsType& flags)
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

    typename T::type operator()(int argc, const char* argv[])
    {
      for (auto i = 0; i < argc; i++)
      {
        if (!strcmp(argv[i], optionName<N>().c_str()))
        {
          ParamArraySetter<T, paramSize<N>()> a;

          for (auto j = 1; j <= paramSize<N>(); j++)
            a[i] = fromString(argv[i + j], a[0]);
          
          return a.value();
        }
      }
      
      return paramDefault<N>();
    }
  };
  
public:
  
  static void report(const char* str1, const char * str2)
  {
    std::cout << str1 << " " << str2 << "\n";
  }
    
  static int run(int argc, const char* argv[])
  {
    ParamSetType params(descriptors());
    ClientType client(params);
    FlagsType flags;
      
    flags.fill(false);
      
    for (int i = 1; i < argc; )
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
    
    // TODO: figure out what to do if values require constraints
    
    params.template setParameterValues<Setter>(false, argc, argv);
    params.constrainParameterValues();
    
    auto result = client.process();
    
    if (!result.ok())
      std::cout << result.message() << "\n";
    
    return result.ok() ? 0 : -1;
  }
};
    
} // namespace client
} // namespace fluid
