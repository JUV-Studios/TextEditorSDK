﻿using System.Collections;

namespace ProjectCodeEditor.Services
{
    public static class MemoryService
    {
        public static bool Get(int index, BitArray array)
        {
            if (index > array.Count) return false;
            else return array.Get(index);
        }

        public static void Set(int index, bool value, BitArray array)
        {
            if (index > array.Count) array.Length++;
            array.Set(index, value);
        }
    }
}