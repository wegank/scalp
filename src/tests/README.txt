
= How to add a new test:

Simply put the corresponding *.cpp file in this directory.

The tests should expect the name of the used Solver as the first command line argument and you are forced to use the Dynamic-Backend, however if it works through the Dynamic-Backend it works without too.

If the test is independent of the Solver-Backend simply put "_ALL" at the end of the filename, e. g. "TermGeneration_ALL.cpp".


= How to build the tests:

  - run cmake with "-DBUILD_TESTS=ON"
  - run "make"


= How to run the tests

The tests are generated for all configured Solvers.

  - "make test" or "ctest" (for all tests)
  - "ctest -R $NAME" (for tests with name $NAME)

