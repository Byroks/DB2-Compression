DB2 Programming Task SoSe2022                         {#mainpage}
============

# Introduction
This is the documentation of the programming project for the exercises in the Lecture 'Databases Implementation Techniques' (DB2) in summer term 2022.
You can find general information about the lecture [here](https://www.dbse.ovgu.de/Lehre/Sommersemester/Bachelor+und+Master/DB2.html).

# Tasks
The purpose of the programming tasks is to deepen your knowledge in selected aspects of the lecture. 
This year, we decided to set this focus on compression techniques in column oriented database management systems. 
Furthermore, we choose C++ as programming language, because it is the most frequently used programming language for database management systems (except for C).
The task is to implement compression techniques in our framework. 
We provide a set of classes as presetting, where you have to include an implementation w.r.t. an interface. 
You can download or clone the sources [here](https://git.iti.cs.ovgu.de/dbse-teaching/db2_programming_project). 
A set of unit tests will help you during the development process to identify errors. 
The same unit tests will be used at the end of the term to validate your solution. 
A working implementation is a necessary prerequisite to participate in the exam!

You may choose between the following compression techniques (you may suggest other compression techniques as well):
- Run Length Encoding
- Delta Coding
- Bit-Vector Encoding
- Dictionary Encoding
- Frequency Partitioning

All compression techniques are explained in the lecture. You can find the slides [here](https://elearning.ovgu.de/course/view.php?id=12434#section-5).

# Organization

Students will form teams of two students each. In case the exercise has an uneven number of participants, one team may consist of three students.
Please register your team until the **12.05.2022** via [Moodle](https://elearning.ovgu.de/mod/choicegroup/view.php?id=333678).

Solutions are to be submitted via [Moodle](https://elearning.ovgu.de/mod/assign/view.php?id=333676) until the **6.07.2022 23:39**.
Note that the deadline is strict, there will be **no** deadline extension.

Solutions will be presented and discussed by each team in the exercise.

Teams consisting of bachelor students have to implement **two** compression techniques and will receive 5 credit points when they pass the exam.
Teams consisting of master students have to implement **three** compression techniques, because they will receive 6 credit points when they pass the exam.

We will check the quality of your submitted solution. 
It has to pass the unit tests, implement the compression technique it represents, and may not be a copy of a solution submitted by another team or any third party implementation. 
Solutions who fail to fulfill only one of these requirements will not be able to participate in the exam.


# Setup and Tools
As for every project, there are some points to mention on how to successfully build the project and how to get started.

## Prerequisites
- C++ compiler with C++17 support (gcc >= 8 clang >= 8 msvc >= 2017.3.5)
- CMake
- Make, Ninja or another build management tool
- Git to download further dependencies
- Doxygen to build the documentation

## Setup
If you do not have all dependencies fulfilled, install them. Afterwards, the project can already be configured. 
Therefore, you can use either the GUI frontend from CMake, open the project in an IDE, or open a terminal in the project folder and type `$ cmake -Bbuild -H.`. 
CMake will now check for needed features, download further dependencies and configure the project.

## Documentation
To generate the documentation for the project, please run `cmake --build . --target documentation`.
This generates the html documentation in the doc folder. 
You can read the documentation by simply opening the index.htm file in the html folder with your favourite browser.

## Building the Project
In order to build the binary from your source files, you can just call the command `cmake --build . --target main`.

## Tests
To run the tests, just run `$ ctest .` in the build directory, when the binary is already build. This will build the project and run available tests.
If you want to build and run the tests in one command, you can use `$ ctest --build-and-test source_directory build_directory --build-generator generator` with generator being "Unix Makefiles", "Ninja" or "Visual Studio".
You also have to replace source_directory with the root directory of the project and build directory with the cmake configured build path.

# Getting Started
To implement your selected compression technique, you have to inherit from the base class @ref CoGaDB::CompressedColumn and implement it's pure virtual methods (similar to an abstract method in Java).
You can test your class by adding it to the list of column types to test in @ref main.cpp. The tests are now automatically instantiated for this class and use its implemented functionality.

If you need an example for the structure of a compressed column, we prepared an example in the project.
The @ref CoGaDB::DictionaryCompressedColumn, which is stored in the file @ref dictionary_compressed_column.hpp.

# C++ Background
You should familiarize yourself with the following features of the C++ language:
- pointers, references, and [smart pointers](https://en.cppreference.com/w/cpp/memory).
- create objects on the heap with new
- call by value and call by reference
- public inheritance
- basic STL containers, such as std::vector, std::list and std::map
- basic templates and how to use them

You can find a lot of useful examples in the framework code, e.g., the uncompressed column implementation @ref include/core/column.hpp.
Recommended (selected) sources of information about C++ are:
- [https://en.cppreference.com/w/](https://en.cppreference.com/w/)
- Bjarne Stroustrup. The C++ Programming Language. Addison-Wesley, 4th edition, 2013
- Scott Meyers. Effective C++: 55 Specific Ways to Improve Your Programs and Designs, Addison-Wesley, 3rd edition, 2005

# Submission
To prepare your solution for submission, it has to be packaged. You can do this with the command `$ cmake --build . --target package_source`.
This generates an archive containing the entire project source code, including your implementation in the build directory.
To submit you solution, you just have to upload the archive [here](https://elearning.ovgu.de/mod/assign/view.php?id=333676).