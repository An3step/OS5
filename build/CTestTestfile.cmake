# CMake generated Testfile for 
# Source directory: C:/Apps/VS Code/CPP Projects/OS5T
# Build directory: C:/Apps/VS Code/CPP Projects/OS5T/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(ReceiverTests "C:/Apps/VS Code/CPP Projects/OS5T/build/Debug/ReceiverTests.exe")
  set_tests_properties(ReceiverTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;27;add_test;C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(ReceiverTests "C:/Apps/VS Code/CPP Projects/OS5T/build/Release/ReceiverTests.exe")
  set_tests_properties(ReceiverTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;27;add_test;C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(ReceiverTests "C:/Apps/VS Code/CPP Projects/OS5T/build/MinSizeRel/ReceiverTests.exe")
  set_tests_properties(ReceiverTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;27;add_test;C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(ReceiverTests "C:/Apps/VS Code/CPP Projects/OS5T/build/RelWithDebInfo/ReceiverTests.exe")
  set_tests_properties(ReceiverTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;27;add_test;C:/Apps/VS Code/CPP Projects/OS5T/CMakeLists.txt;0;")
else()
  add_test(ReceiverTests NOT_AVAILABLE)
endif()
