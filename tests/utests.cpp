#include "gtest/gtest.h"
#include "log.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ProgramOptions {
public:
  ProgramOptions(int argc, char **argv)
      : opts("Glogg Options"), hiddenOpts("Hidden options")
  {
      opts.add_options()("help,h", "print out program usage (this message)")(
          "log,l", po::value<std::string>(), "Filename to redirect log to")(
          "debug,d", "output more debug (include multiple times for more "
                     "verbosity e.g. -dddd)");
      for (std::string s = "dd"; s.length() <= 10; s.append("d"))
          hiddenOpts.add_options()(s.c_str(), "debug");

        po::options_description all_options("all options");
        all_options.add(opts).add(hiddenOpts);

        int command_line_style = (((po::command_line_style::unix_style ^
                po::command_line_style::allow_guessing) |
                po::command_line_style::allow_long_disguise) ^
                po::command_line_style::allow_sticky);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(all_options).
                style(command_line_style).run(),
                vm);
        po::notify(vm);

        if (vm.count("help")) {
            opts.print(std::cout);
            exit(0);
        }

        if (vm.count("log"))
            logFile = QString::fromStdString(vm["log"].as<std::string>());

        for (std::string s = "dd"; s.length() <= 10; s.append("d"))
            if (vm.count(s))
                logLevel = (TLogLevel)(logWARNING + s.length());
  }

    po::options_description opts;
    po::options_description hiddenOpts;

    QString logFile;
    TLogLevel logLevel = logWARNING;

private:
    po::options_description allOpts;
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ProgramOptions opts(argc, argv);
    Log::configure(opts.logLevel, opts.logFile);
    return RUN_ALL_TESTS();
}