#include "gtest/gtest.h"
#include "log.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace testing;

class ProgramOptions {
  public:
    ProgramOptions();

    void parse(int argc, char **argv);

    void process();

  protected:
    po::options_description opts;
    po::options_description hiddenOpts;
    po::variables_map vm;
};

ProgramOptions::ProgramOptions()
    : opts("Glogg Options"), hiddenOpts("Hidden options")
{
    opts.add_options()("help,h", "print out program usage (this message)");
    for (std::string s = "dd"; s.length() <= 10; s.append("d"))
        hiddenOpts.add_options()(s.c_str(), "debug");
}

void ProgramOptions::parse(int argc, char **argv)
{
    po::options_description all_options("all options");
    all_options.add(opts).add(hiddenOpts);

    int command_line_style = (((po::command_line_style::unix_style
                                ^ po::command_line_style::allow_guessing)
                               | po::command_line_style::allow_long_disguise)
                              ^ po::command_line_style::allow_sticky);

    po::store(po::command_line_parser(argc, argv)
                  .options(all_options)
                  .style(command_line_style)
                  .run(),
              vm);
    po::notify(vm);
}

void ProgramOptions::process()
{
    if (vm.count("help")) {
        opts.print(std::cout);
        exit(0);
    }
}

class ProgramOptionsWithLog : public ProgramOptions {
public:
    ProgramOptionsWithLog();
    void process();
};

ProgramOptionsWithLog::ProgramOptionsWithLog()
{
    opts.add_options()("log,l", po::value<std::string>(),
                       "Filename to redirect log to")(
        "debug,d", "output more debug (include multiple times for more "
                   "verbosity e.g. -dddd)");
}

void ProgramOptionsWithLog::process()
{
    ProgramOptions::process();

    QString logFile;
    TLogLevel logLevel = logWARNING;
    if (vm.count("log"))
        logFile = QString::fromStdString(vm["log"].as<std::string>());

    for (std::string s = "dd"; s.length() <= 10; s.append("d"))
        if (vm.count(s))
            logLevel = (TLogLevel)(logWARNING + s.length());
    Log::configure(logLevel, logFile);
}

class MyEventListener : public TestEventListener {
public:
  explicit MyEventListener(TestEventListener* theEventListener)
      : eventListener(theEventListener)
  {}

  virtual void OnTestEnd(const TestInfo &test_info) override
  {
      if (test_info.result()->Failed())
          eventListener->OnTestEnd(test_info);
  }

  virtual void OnTestProgramStart(const UnitTest &) override {}
  virtual void OnTestIterationStart(const UnitTest &, int) override {}
  virtual void OnEnvironmentsSetUpStart(const UnitTest &) override {}
  virtual void OnEnvironmentsSetUpEnd(const UnitTest &) override {}
  virtual void OnTestCaseStart(const TestCase &) override {}
  virtual void OnTestCaseEnd(const TestCase &) override {}
  virtual void OnEnvironmentsTearDownStart(const UnitTest &) override {}
  virtual void OnEnvironmentsTearDownEnd(const UnitTest &) override {}
  virtual void OnTestIterationEnd(const UnitTest &, int) override {}
  virtual void OnTestProgramEnd(const UnitTest &) override {}
  virtual void OnTestStart(const TestInfo &) override {}
  virtual void OnTestPartResult(const TestPartResult &) override {}

protected:
  TestEventListener *eventListener;
};

class ProgramOptionsGTest : public ProgramOptionsWithLog {
public:
  ProgramOptionsGTest()
  {
      opts.add_options()("quiet", "Silence Google Test output");
  }
    void process() {
        ProgramOptionsWithLog::process();
        if (!vm.count("quiet"))
            return;
        TestEventListeners &listeners = UnitTest::GetInstance()->listeners();
        auto default_printer
            = listeners.Release(listeners.default_result_printer());
        MyEventListener *listener = new MyEventListener(default_printer);
        listeners.Append(listener);
    }
};

int main(int argc, char **argv)
{
    InitGoogleTest(&argc, argv);

    ProgramOptionsGTest opts;
    opts.parse(argc, argv);
    opts.process();
    return RUN_ALL_TESTS();
}