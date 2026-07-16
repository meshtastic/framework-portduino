#include "Arduino.h"
#include "PortduinoFS.h"
#include "PortduinoGPIO.h"
#include <argp.h>
#include <stdio.h>
#include <cerrno>
#include <iostream>
#include <cstring>

#ifdef _WIN32
// No <ftw.h> on Windows; _mkdir()/_execv() come from the CRT headers below.
#include <direct.h>
#include <process.h>
#include <filesystem>
#include <sys/stat.h>
#else
#include <ftw.h>
#include <sys/stat.h>
#endif

// Windows has no POSIX permission bits, so _mkdir() takes no mode.
#ifdef _WIN32
#define portduino_mkdir(path, mode) _mkdir(path)
#else
#define portduino_mkdir(path, mode) mkdir(path, mode)
#endif

/** # msecs to sleep each loop invocation.  FIXME - make this controlable via
 * config file or command line flags.
 */
static long loopDelay = 100;

/** Store pointer to argv for restart
*/
char **progArgv;

/** apps run under portduino can optionally define a portduinoSetup() to
 * use portduino specific init code (such as gpioBind) to setup portduino on
 * their host machine, before running 'arduino' code.
 */
void __attribute__((weak)) portduinoSetup() {
  printf("No portduinoSetup() found, using default settings...\n");
}

void __attribute__((weak)) portduinoCustomInit() {}

// FIXME - move into app client (out of lib) and use real name
// FIXME - add app specific options as child options
// http://www.gnu.org/software/libc/manual/html_node/Argp.html
const char *argp_program_bug_address =
    "https://github.com/meshtastic/Meshtastic-device";
static char doc[] = "An application written with portduino";
static char args_doc[] = "...";

static struct argp_option options[] = {
    {"erase", 'e', 0, 0, "Erase virtual filesystem before use"},
    {"fsdir", 'd', "DIR", 0, "The directory to use as the virtual filesystem"},
    {0}};

struct TopArguments {
  bool erase;
  char *fsDir;
};

// In bss (inited to zero)
TopArguments portduinoArguments;

static struct argp_child children[2] = {{NULL}, {NULL}};

static void *childArguments;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  auto args = (TopArguments *)state->input;
  switch (key) {
  case ARGP_KEY_INIT:
    if (children[0].argp)
      state->child_inputs[0] = childArguments;
    break;
  case 'e':
    args->erase = true;
    break;
  case 'd':
    args->fsDir = arg;
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/*
 * Functions to remove contents of directory
 * Adapted from: https://stackoverflow.com/a/5467788
 */
#ifdef _WIN32
// No nftw() on Windows. Deleting each entry under the root rather than the root
// itself matches the POSIX path's `0 < ftwbuf->level` test.
int rmrf(char *path)
{
    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator(path, ec)) {
        std::error_code rmEc;
        std::filesystem::remove_all(entry.path(), rmEc);
        if (rmEc) {
            fprintf(stderr, "%s: %s\n", entry.path().string().c_str(), rmEc.message().c_str());
            return -1;
        }
    }
    if (ec) {
        fprintf(stderr, "%s: %s\n", path, ec.message().c_str());
        return -1;
    }
    return 0;
}
#else
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = 0;
    if (0 < ftwbuf->level)
      rv = remove(fpath);
    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}
#endif // _WIN32

static struct argp argp = {options, parse_opt, args_doc, doc, children, 0, 0};

/**
 * call from portuinoCustomInit() if you want to add custom command line
 * arguments
 */
void portduinoAddArguments(const struct argp_child &child,
                           void *_childArguments) {
  // We only support one child for now
  children[0] = child;
  childArguments = _childArguments;
}

void portduinoSetOptions(portduinoOptions options) {
  realHardware = options.realHardware;
}

void reboot() {
#ifdef _WIN32
  // _execv() spawns a replacement and terminates this process rather than
  // replacing the image in place, but the effect is the same restart.
  int err = _execv(progArgv[0], progArgv);
  printf("_execv() returned %i!\n", err);
#else
  int err = execv(progArgv[0], progArgv);
  printf("execv() returned %i!\n", err);
#endif
  std::cout << "error: " << std::strerror(errno) << '\n';
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

  progArgv = (char**) malloc((argc + 1) * sizeof(char*)); // New pointer array, argc + 1 to hold the final null
  int j = 0;
  for (int i = 0; i < argc; i++) { // iterate through the arguments, stripping out the erase command, to avoid erase on reboot()
    if (strcmp(argv[i], "-e") != 0 && strcmp(argv[i], "--erase") != 0  ) {
      progArgv[j] = argv[i];
      j++;
    }
  }
  progArgv[j] = NULL;

  portduinoCustomInit();

  auto *args = &portduinoArguments;

  auto parseResult = argp_parse(&argp, argc, argv, 0, 0, args);
  if (parseResult == 0) {
    String fsRoot;

    if (!args->fsDir) {
      // create a default dir

      const char *homeDir = getenv("HOME");
#ifdef _WIN32
      // Windows doesn't set $HOME outside of MSYS/Cygwin shells.
      if (!homeDir)
        homeDir = getenv("USERPROFILE");
#endif
      assert(homeDir);

      fsRoot += homeDir + String("/.portduino");
      portduino_mkdir(fsRoot.c_str(), 0700);

      const char *instanceName = "default";
      fsRoot += "/" + String(instanceName);
    } else
      fsRoot += args->fsDir;

    printf("Portduino is starting, VFS root at %s\n", fsRoot.c_str());

    int status = portduino_mkdir(fsRoot.c_str(), 0700);
    if (status != 0 && errno == EEXIST && args->erase) {
      // Remove contents of existing VFS root directory
      std::cout << "Erasing virtual Filesystem!" << std::endl;
      rmrf(const_cast<char*>(fsRoot.c_str())); 
    }

    portduinoVFS->mountpoint(fsRoot.c_str());

    gpioInit();
    portduinoSetup();
    setup();

    while (true) {
      gpioIdle(); // FIXME, do this someplace better
      loop();

      // Even if the Arduino code doesn't want to sleep, ensure we don't  burn
      // too much CPU
      if (!realHardware)
        delay(loopDelay);
    }
    return 0;
  } else
    return parseResult;
}
