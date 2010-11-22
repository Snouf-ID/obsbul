
#ifndef COMMON_H
#define COMMON_H

#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>
#include <exception>
#include <memory>

using namespace std;

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#ifdef _WIN32
    typedef unsigned int uint;
#endif

#include "util/logger.h"
using namespace util;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#endif
