﻿#include "HomePage.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::Develop::implementation
{
    HomePage::HomePage()
    {
        InitializeComponent();
    }
}
