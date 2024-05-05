# UNAME - UNIX Toolset

Source code is relatively small, it might not be that hard to extract the kernel information

## Headers used
```c
#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <getopt.h>
# include <sys/systeminfo.h>
```

## Calls to different architecture system control functions

### Sysinfo
```c
sysinfo (SI_ARCHITECTURE, processor, sizeof processor)
```

### Uname
```c
sysctl (mib, 2, processor, &s, 0, 0)
```

## Apple
```c
sysctlbyname ("hw.cputype", &cputype, &s, NULL, 0) == 0
    && (ai = NXGetArchInfoFromCpuType (cputype,
                                        CPU_SUBTYPE_MULTIPLE))
```

## What kind of system information does it print?

### Kernel Release and name
```c
      struct utsname name;

      if (uname (&name) == -1)
        error (EXIT_FAILURE, errno, _("cannot get system name"));
```

### Other hardware information

Using the abovementioned `sysctl` kernel functions, uname program can output
some of the following attributes (denoted by the C Enumeration values)

- `SI_ARCHITECTURE`
- `CTL_HW`
- `HW_MODEL`
- `HW_MACHINE_ARCH`
- `HOST_OPERATING_SYSTEM`

C Structure `utsname` and header `<sys/utsname>` or `<sys/systeminfo.h>`
For OpenBSD `<sys/param.h>`
