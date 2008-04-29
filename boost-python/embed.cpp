#include <boost/python.hpp>
#include <stdio.h>
#include <iostream>

#include "hello.h"

using namespace boost::python;


BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}

int main(int argc, char *argv[])
{
    object imp;
    
    PyImport_AppendInittab((char*)"hello",&inithello);
    Py_Initialize();

    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");

    try
    {
        if(main_module)
        {
            exec("import readline",main_namespace,main_namespace);
            exec("import hello; from hello import *",main_namespace,main_namespace);
            exec("from code import InteractiveConsole",main_namespace,main_namespace);      
            exec("i = InteractiveConsole(globals())",main_namespace,main_namespace);
            exec("i.interact()",main_namespace,main_namespace);
        }
        else
            printf("err\n");

    }
    catch(error_already_set const &)
    {
        PyErr_Print();

    }
}

// c++ embed.cpp hello.cpp -I /usr/include/python2.5 -I /usr/include/boost/python/ /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0 -o embed

