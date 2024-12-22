#ifdef PORTDUINO_LINUX_HARDWARE

#include "linux/gpio/LinuxGPIOPin.h"
#include <assert.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <dirent.h>
#include <unistd.h>

const char *consumer = "portduino";

static bool chip_is_gpiochip_device(const char *path) {
  char *realname, *sysfsp, devpath[64];
	struct stat statbuf;
	bool ret = false;
	int rv;

	rv = lstat(path, &statbuf);
	if (rv)
		goto out;

	/*
	 * Is it a symbolic link? We have to resolve it before checking
	 * the rest.
	 */
	realname = S_ISLNK(statbuf.st_mode) ? realpath(path, NULL) :
					      strdup(path);
	if (realname == NULL)
		goto out;

	rv = stat(realname, &statbuf);
	if (rv)
		goto out_free_realname;

	/* Is it a character device? */
	if (!S_ISCHR(statbuf.st_mode)) {
		errno = ENOTTY;
		goto out_free_realname;
	}

	/* Is the device associated with the GPIO subsystem? */
	snprintf(devpath, sizeof(devpath), "/sys/dev/char/%u:%u/subsystem",
		 major(statbuf.st_rdev), minor(statbuf.st_rdev));

	sysfsp = realpath(devpath, NULL);
	if (!sysfsp)
		goto out_free_realname;

	/*
	 * In glibc, if any of the underlying readlink() calls fail (which is
	 * perfectly normal when resolving paths), errno is not cleared.
	 */
	errno = 0;

	if (strcmp(sysfsp, "/sys/bus/gpio") != 0) {
		/* This is a character device but not the one we're after. */
		errno = ENODEV;
		goto out_free_sysfsp;
	}

	ret = true;

out_free_sysfsp:
	free(sysfsp);
out_free_realname:
	free(realname);
out:
	errno = 0;
	return ret;
}

static int chip_dir_filter(const struct dirent *entry)
{
	bool is_chip;
	char *path;
	int ret;

	ret = asprintf(&path, "/dev/%s", entry->d_name);
	if (ret < 0)
		return 0;

	is_chip = chip_is_gpiochip_device(path);
	free(path);
	return !!is_chip;
}

static struct gpiod_chip *chip_open_by_name(const char *name)
{
	struct gpiod_chip *chip;
	char *path;
	int ret;

	ret = asprintf(&path, "/dev/%s", name);
	if (ret < 0)
		return NULL;

	chip = gpiod_chip_open(path);
	free(path);

	return chip;
}

/**
 * Try to find the specified linux gpio line, throw exception if not found
 */
gpiod_line *LinuxGPIOPin::getLine(const char *chipLabel, const char *linuxPinName) {
	std::string path = "/dev/";
	path += chipLabel;
  if (access(path.c_str(), R_OK) == 0) {
	chip = chip_open_by_name(chipLabel);
	if (!chip) {
		throw std::invalid_argument("Error, cannot open GPIO chip");
	}

#if GPIOD_V == 2
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_request *line = NULL;
    offset = gpiod_chip_get_line_offset_from_name(chip, linuxPinName);
	settings = gpiod_line_settings_new();
	gpiod_line_settings_set_direction(settings, GPIOD_LINE_REQUEST_DIRECTION_AS_IS);
	line_cfg = gpiod_line_config_new();
	gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);
	req_cfg = gpiod_request_config_new();
	gpiod_request_config_set_consumer(req_cfg, consumer);
	line = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);
	gpiod_line_config_free(line_cfg);
	gpiod_line_settings_free(settings);
	gpiod_chip_close(chip);
	return line;


#else
	auto line = gpiod_chip_find_line(chip, linuxPinName);
	struct gpiod_line_request_config request = {
		consumer, GPIOD_LINE_REQUEST_DIRECTION_AS_IS, 0};
	auto result = gpiod_line_request(line, &request, 0);
	if(result != 0) {
		throw std::invalid_argument("Error, cannot open GPIO chip");
	}
	return line;
#endif

  }
#if GPIOD_V == 1
  struct dirent **entries;
  int num_chips = scandir("/dev/", &entries, chip_dir_filter, alphasort);
  assert(num_chips > 0); // FIXME, throw exception
  log(SysGPIO, LogDebug, "getLine(%s, %s)", chipLabel, linuxPinName);
  for (int i = 0; i < num_chips; i++) {
    chip = chip_open_by_name(entries[i]->d_name);
    if (!chip) {
      if (errno == EACCES)
        continue; // skip chips we don't have access to

      assert(0); // die_perror("unable to open %s", entries[i]->d_name);
    } else {
      auto label = gpiod_chip_label(chip);
      if (strcmp(label, chipLabel) == 0) {
        auto line = gpiod_chip_find_line(chip, linuxPinName);

        struct gpiod_line_request_config request = {
            consumer, GPIOD_LINE_REQUEST_DIRECTION_AS_IS, 0};
        auto result = gpiod_line_request(line, &request, 0);
        if(result != 0) {
		    throw std::invalid_argument("Error, cannot open GPIO chip");
		}
        return line;
      }
    }
  }
#endif
  assert(0); // FIXME throw
}

/**
 * Try to find the specified linux gpio line, throw exception if not found
 */
gpiod_line *LinuxGPIOPin::getLine(const char *chipLabel, const int linuxPinNum) {
	std::string path = "/dev/";
	path += chipLabel;
  if (access(path.c_str(), R_OK) == 0) {
	chip = chip_open_by_name(chipLabel);
	if (!chip) {
		throw std::invalid_argument("Error, cannot open GPIO chip");
	}
#if GPIOD_V == 2
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_request_config *req_cfg = NULL;
	struct gpiod_line_request *line = NULL;
    offset = linuxPinNum;
	settings = gpiod_line_settings_new();
	gpiod_line_settings_set_direction(settings, GPIOD_LINE_REQUEST_DIRECTION_AS_IS);
	line_cfg = gpiod_line_config_new();
	gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);
	req_cfg = gpiod_request_config_new();
	gpiod_request_config_set_consumer(req_cfg, consumer);
	line = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);
	gpiod_line_config_free(line_cfg);
	gpiod_line_settings_free(settings);
	gpiod_chip_close(chip);
	return line;
#else
	auto line = gpiod_chip_get_line(chip, linuxPinNum);

	struct gpiod_line_request_config request = {
		consumer, GPIOD_LINE_REQUEST_DIRECTION_AS_IS, 0};
	auto result = gpiod_line_request(line, &request, 0);
	if(result != 0) {
		throw std::invalid_argument("Error, cannot open GPIO chip");
	}
	return line;
#endif
  }
#if GPIOD_V == 1
  struct dirent **entries;
  int num_chips = scandir("/dev/", &entries, chip_dir_filter, alphasort);
  assert(num_chips > 0); // FIXME, throw exception
  log(SysGPIO, LogDebug, "getLine(%s, %d)", chipLabel, linuxPinNum);
  for (int i = 0; i < num_chips; i++) {
    chip = chip_open_by_name(entries[i]->d_name);
    if (!chip) {
      if (errno == EACCES)
        continue; // skip chips we don't have access to

      assert(0); // die_perror("unable to open %s", entries[i]->d_name);
    } else {
      auto label = gpiod_chip_label(chip);
      if (strcmp(label, chipLabel) == 0) {
        auto line = gpiod_chip_get_line(chip, linuxPinNum);

        struct gpiod_line_request_config request = {
            consumer, GPIOD_LINE_REQUEST_DIRECTION_AS_IS, 0};
        auto result = gpiod_line_request(line, &request, 0);
        if(result != 0) {
		    throw std::invalid_argument("Error, cannot open GPIO chip");
		}
        return line;
      }
    }
  }
#endif
  assert(0); // FIXME throw
}

/**
 * Create a pin given a linux chip label and pin name
 */
LinuxGPIOPin::LinuxGPIOPin(pin_size_t n, const char *chipLabel,
                           const char *linuxPinName,
                           const char *portduinoPinName)
    : GPIOPin(n, portduinoPinName ? portduinoPinName : linuxPinName) {
  line = getLine(chipLabel, linuxPinName);
}

LinuxGPIOPin::LinuxGPIOPin(pin_size_t n, const char *chipLabel,
                           const int linuxPinNum,
                           const char *portduinoPinName)
    : GPIOPin(n, portduinoPinName) {
  line = getLine(chipLabel, linuxPinNum);
}

LinuxGPIOPin::~LinuxGPIOPin() { 
    gpiod_line_release(line); 
    gpiod_chip_close(chip);
}

/// Read the low level hardware for this pin
PinStatus LinuxGPIOPin::readPinHardware() {
    int res = gpiod_line_get_value(line);
    assert(res == 0 || res == 1); // FIXME throw instead

    // log(SysGPIO, LogDebug, "readPinHardware(%s, %d)", getName(), res); 
    return (PinStatus) res;
}

void LinuxGPIOPin::writePin(PinStatus s) {
  // some libraries have been observed failing to set the pin mode to output.
  if (GPIOPin::getPinMode() != OUTPUT)
	setPinMode(OUTPUT);
  GPIOPin::writePin(s); // update status

  int res = gpiod_line_set_value(line, s);
  assert(res == 0);
}

void LinuxGPIOPin::setPinMode(PinMode m) {
  GPIOPin::setPinMode(m);
#if GPIOD_V == 1
  // The gpiod call below does not play well with an already claimed GPIO
  // So we release it first.
  gpiod_line_release(line);

  // Set direction, if output use the current pinstate as the output value
  if (m == OUTPUT) {
    gpiod_line_request_output(line, consumer, readPin());
  }
  else {
    gpiod_line_request_input(line, consumer);
  }
#else
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	int ret;
	settings = gpiod_line_settings_new();
	if (m == OUTPUT) {
		gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
		gpiod_line_settings_set_output_value(settings, (gpiod_line_value) readPin());
	} else {
		gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
	}
	line_cfg = gpiod_line_config_new();
	ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,settings);
	ret = gpiod_line_request_reconfigure_lines(line, line_cfg);

	gpiod_line_config_free(line_cfg);
	gpiod_line_settings_free(settings);


#endif
}


#endif
