/* main.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"
#include <iostream>
#include "milhouse.h"
#include "util/sigint.h"
#include "except.h"

int main(int argc, char **argv)
{
    int ret = 0;
    static const std::string errorString("Please file a bug at " PACKAGE_BUGREPORT " if needed");

    try
    {
        signal_handlers::setHandlers();
        Milhouse milhouse;
        ret = milhouse.run(argc, argv);
    }
    catch (const Except &e)  // these are our exceptions
    {
        if (std::string(e.what()).find("INTERRUPTED") not_eq std::string::npos)
        {
            std::cout << "Interrupted" << std::endl;
            ret = 0;
        }
        else
        {
            std::cerr << e.what() << std::endl;
            std::cerr << errorString << std::endl;
            ret = 1;
        }
    }
    catch (const std::exception &e)  // these are other exceptions (not one of our exception classes)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << errorString << std::endl;
        ret = 1;
    }
    return ret;
}
