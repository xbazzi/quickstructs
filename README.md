# QuickLib
Header-only C++ library with useful data structures and utilities for low-latency applications.

# Readiness
Some of the library is production-ready; some is not. Here is a table with the latest completion estimates:


| Directory / File                              | Completion Estimate | Production-Ready?              | Basis                                                                                                         |
| --------------------------------------------- | ------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------- |
| **[SpinMutex][1]**                            | 85%                 | **Beta**                      | Almost ready. Still needs ISA-specific handling. |
| **[SPSCQueue][2]**                                 | 60%                  | **Alpha**                      | Only use this queue to play around. Still needs a few optimizations.                       |
| **[UniquePtr][3]**                                    | 95%                  | **Yes**                      | Ready to go.                                                             |
| **[ThreadPool][3]**                            | 70%                 | **Alpha**     |                Technically ready, but can be made significantly more performant.                                       
| **[memory/][4]**                                  | 85%                 | **Beta**                      | Some of these might be faster than glibc, some might be slower. There is a lot of potential for speedups through vectorization and other optimizations. Avoid for serious projects (for now).                                             |

[1]: https://github.com/xbazzi/quicklib/tree/master/include/quick "quicklib/include/quick at master · xbazzi/quicklib · GitHub"
[2]: https://github.com/xbazzi/quicklib/tree/master/examples "quicklib/examples at master · xbazzi/quicklib · GitHub"
[3]: https://github.com/xbazzi/quicklib/tree/master/tests "quicklib/tests at master · xbazzi/quicklib · GitHub"
[4]: https://github.com/xbazzi/quicklib/tree/master "GitHub - xbazzi/quicklib: Low-latency library"


