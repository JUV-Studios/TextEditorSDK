﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProjectCodeEditor.Dialogs
{
    internal interface IModalDialog
    {
        Task<bool> Show();
    }
}
