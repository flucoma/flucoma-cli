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

#include <AudioFile/IAudioFile.h>
#include <AudioFile/OAudioFile.h>
#include <clients/common/FluidBaseClient.hpp>
#include <clients/common/OfflineClient.hpp>
#include <clients/common/ParameterSet.hpp>
#include <clients/common/ParameterTypes.hpp>
#include <FluidVersion.hpp>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

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
  
    mReadError = file.getErrorFlags();
  
    if (file.isOpen())
    {
      resize(file.getFrames(), file.getChannels(), file.getSamplingRate());
      file.seek();
      file.readInterleaved(mData.data(),file.getFrames());
    }
  }

  void write(bool allowCSV, bool& result)
  {
    if (!numFrames()) return;

    index pathLength = mPath.length();

    if (allowCSV && (pathLength > 4) &&
        !mPath.compare(pathLength - 4, 4, ".csv"))
    {
      FluidTensorView<const float, 2> view{mData.data(), 0, numChans(),
                                           numFrames()};

      std::ofstream file(mPath.c_str());

      if (file.is_open())
      {
        file << std::setprecision(std::numeric_limits<float>::max_digits10)
             << view;
        file.close();
      }
      else
      {
        result = false;
        std::cerr << "Could not open file " << mPath << " for writing\n";
      }
    }
    else
    {
      // TODO: file extensions/paths

      constexpr auto fileType = HISSTools::BaseAudioFile::kAudioFileWAVE;
      constexpr auto depthType = HISSTools::BaseAudioFile::kAudioFileFloat32;

      uint16_t chans = static_cast<uint16_t>(
          std::min(asSigned(std::numeric_limits<uint16_t>::max()), numChans()));

        HISSTools::OAudioFile file(mPath, fileType, depthType, chans,
                                 mSamplingRate);

      if (file.isOpen())
      {
        std::cout << "Writing " << mPath << '\n';
        file.seek();
        file.writeInterleaved(mData.data(), static_cast<uint32_t>(numFrames()));
        
        if(file.getIsError())
        {
          result = false;
          for(auto&& e:file.getErrors()) std::cerr << HISSTools::BaseAudioFile::getErrorString(e) << '\n';
        }
      }
      else
      {
        result = false;
        std::cerr << "Could not open file " << mPath << " for writing\n";
      }
    }
  }
  
  int readError() const { return mReadError; }

private:
  bool acquire() const override { return !mAcquired && (mAcquired = true); }
  void release() const override { mAcquired = false; }

  bool valid() const override { return numFrames(); }
  bool exists() const override { return true; }

  double sampleRate() const override { return mSamplingRate; }

  const float* getChannel(index channel) const
  {
    return mData.data() + numFrames() * channel;
  }
  float* getChannel(index channel)
  {
    return mData.data() + numFrames() * channel;
  }

  const Result resize(index frames, index channels, double sampleRate) override
  {
    mNumChans = channels;
    mData.resize(frames,channels);
    mSamplingRate = sampleRate;
    return {};
  }
  
  fluid::FluidTensorView<float, 2> allFrames() override
  {
      return mData; 
  }
  
  fluid::FluidTensorView<const float, 2> allFrames() const override
  {
      return mData; 
  }
  
  fluid::FluidTensorView<float, 1> samps(index channel) override
  {
      return mData.col(channel); 
  }

  fluid::FluidTensorView<float, 1> samps(index offset, index nframes,
                                         index chanoffset) override
  {
      return mData(Slice(offset, nframes), Slice(chanoffset, 1)).col(0);
  }

  fluid::FluidTensorView<const float, 1> samps(index channel) const override
  {
      return mData.col(channel);
  }

  fluid::FluidTensorView<const float, 1> samps(index offset, index nframes,
                                               index chanoffset) const override
  {
      return mData(Slice(offset, nframes), Slice(chanoffset, 1)).col(0);
  }

  index numFrames() const override { return mData.rows(); }
  index numChans() const override { return mData.cols(); }

  std::string asString() const override { return mPath; }

  std::string          mPath;
  mutable bool         mAcquired;
  double               mSamplingRate = 44100.0;
  index                mNumChans;
  FluidTensor<float,2> mData;
  int                  mReadError;
};

template <class Client>
class CLIWrapper
{
  enum ErrorType {
    kErrNone,
    kErrNoOption,
    kErrUnknownOption,
    kErrAlreadySet,
    kErrMissingVals,
    kErrValType
  };

  using ClientType = Client;
  using ParamSetType = typename ClientType::ParamSetType;
  using ConstString = const std::string;

  static constexpr auto& descriptors()
  {
    return ClientType::getParameterDescriptors();
  }
  static constexpr index nParams = descriptors().size();

  template <size_t N>
  static constexpr auto paramDescriptor()
  {
    return descriptors().template get<N>();
  }

  template <size_t N>
  static constexpr index paramSize()
  {
    return paramDescriptor<N>().fixedSize;
  }

  template <size_t N>
  static std::string optionName()
  {
    std::string str(paramDescriptor<N>().name);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    str.insert(0, "-");

    return str;
  }

  using FlagsType = std::array<bool, nParams>;

  template <size_t L, size_t N = 0>
  struct ValidateParams
  {
    bool numeric(ConstString s, bool d = true)
    {
      return s.find_first_not_of("0123456789.", s[0] == '-', d ? 11 : 10) ==
             std::string::npos;
    };

    bool testString(ConstString s, LongT::type) { return numeric(s, false); }
    bool testString(ConstString s, FloatT::type)
    {
      return numeric(s) && s.find(".") == s.find_last_of(".");
    }
    bool testString(ConstString s, BufferT::type) { return s[0] != '-'; }
    bool testString(ConstString s, InputBufferT::type) { return s[0] != '-'; }

    template <typename T>
    ErrorType checkValues(index& i, index argc, const char* argv[], index nArgs,
                          T)
    {
      if (!(i + nArgs < argc)) return kErrMissingVals;

      for (index j = 1; j <= nArgs; j++)
        if (!testString(argv[i + j], T())) return kErrValType;

      i += 1 + nArgs;
      return kErrNone;
    }

    ErrorType operator()(index& i, index argc, const char* argv[],
                         FlagsType& flags)
    {
      using T = typename ClientType::ParamDescType::template ParamType<N>;
      using ArgType =
          typename ParamLiteralConvertor<T, paramSize<N>()>::LiteralType;

      if (argv[i][0] != '-' || numeric(argv[i])) return kErrNoOption;

      if (!strcmp(argv[i], optionName<N>().c_str()))
      {
        if (flags[N]) return kErrAlreadySet;
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
    auto fromString(ConstString s, BufferT::type)
    {
      return BufferT::type(new CLIBufferAdaptor(s));
    }
    auto fromString(ConstString s, InputBufferT::type)
    {
      return InputBufferT::type(new CLIBufferAdaptor(s));
    }

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

  template <size_t N, typename T>
  struct WriteFiles
  {
    void operator()(typename T::type& param, bool allowCSV, bool& result)
    {
      if (param) static_cast<CLIBufferAdaptor*>(param.get())->write(allowCSV, result);
    }
  };
  
  template <size_t N, typename T>
  struct CheckRead
  {
    void operator()(typename T::type& param, bool& result)
    {
      if(param)
      {
        const CLIBufferAdaptor* ifile = static_cast<const CLIBufferAdaptor*>(param.get());
        if(ifile->readError())
        {
          std::cout << ifile->readError() << '\n'; 
          using Audio = HISSTools::BaseAudioFile;
          result = false;
          std::vector<Audio::Error> errors = Audio::extractErrorsFromFlags(ifile->readError());
          for(auto&& e:errors) std::cerr << Audio::getErrorString(e) << '\n';
        }
      }
    }
  };
  

public:
  static void report(const char* str1, const char* str2)
  {
    std::cout << str1 << " " << str2 << "\n";
  }

  static int run(index argc, const char* argv[])
  {
    ParamSetType params(descriptors());
    FlagsType    flags;
    flags.fill(false);

    const std::regex help("(-*)h(elp)?");
    const std::regex version("(-*)v(ersion)?");
    std::cmatch      m;
    if (argc > 1 && std::regex_match(argv[1], m, help))
    {
      std::cout << "Fluid Corpus Manipulation Toolkit, version "
                << fluidVersion() << '\n';
      std::cout << "Part of the Fluid Corpus Manipulation Project - "
                   "http:://www.flucoma.org/\n";
      std::cout
          << "For a more detailed description of the available options than "
             "given below, please see the accompanying HTML documentation.\n";
      std::cout << "Call with these options:\n";
      descriptors().template iterate<Help>();
      return 0;
    }

    if (argc > 1 && std::regex_match(argv[1], m, version))
    {
      std::cout << "Fluid Corpus Manipulation Toolkit, version "
                << fluidVersion() << '\n';
      return 0;
    }

    for (index i = 1; i < argc;)
    {
      switch (ValidateParams<nParams>()(i, argc, argv, flags))
      {
      case kErrNone: break;
      case kErrNoOption:
        report("Expected option, but found", argv[i]);
        return -2;
      case kErrUnknownOption: report("Unknown option", argv[i]); return -3;
      case kErrAlreadySet:
        report("More than one use of option", argv[i]);
        return -4;
      case kErrMissingVals:
        report("Missing values for option", argv[i]);
        return -5;
      case kErrValType:
        report("Values wrong type for option", argv[i]);
        return -6;
      }
    }

    params.template setParameterValues<Setter>(false, argc, argv);
    params.constrainParameterValues();
    
    bool readSuccess{true};
    params.template forEachParamType<InputBufferT, CheckRead>(readSuccess);
    if(!readSuccess) return -1; 

    // Create client after all parameters are set

    ClientType   client(params);
    Result result;

    client.enqueue(params);
    result = client.process();
    
    double progress = 0.0;

    while(result.ok())
    {
        ProcessState state = client.checkProgress(result);
      
        if (state == ProcessState::kDone || state == ProcessState::kDoneStillProcessing) {
          std::cout << "100%\n";
          break;
        }
        if (state != ProcessState::kDone) {
          double newProgress = client.progress();
          if (newProgress - progress >=0.01)
          {
            std::cout << std::setw(3)  << static_cast<int>(100 * newProgress) << "%\r" << std::flush;
            progress = newProgress;
          }
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(20ms); 
          continue; 
        }
    }

    if (!result.ok())
    {
      // Output error
      
      std::cerr << result.message() << "\n";
    }
    else
    {
      // Write files
      
      bool allowCSV = true;
      bool fileWriteResult = true;
      params.template forEachParamType<BufferT, WriteFiles>(allowCSV, fileWriteResult);
      if(!fileWriteResult) return -1; 
    }

    return result.ok() ? 0 : -1;
  }
};

} // namespace client
} // namespace fluid
