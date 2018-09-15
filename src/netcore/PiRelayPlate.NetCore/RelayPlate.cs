using System;
using System.Runtime.InteropServices;
using System.Security;

namespace RelayPlateCore
{
    public static class RelayPlate
    {
        // ToDo: fix mangled name on c export
        [DllImport("libblinkLib.so", EntryPoint = "_Z11SetPinStatehhh", CallingConvention = CallingConvention.StdCall, PreserveSig = true), SuppressUnmanagedCodeSecurity]
        public static extern int SetPinState(byte boardId, byte pin, byte state);
    }
}
