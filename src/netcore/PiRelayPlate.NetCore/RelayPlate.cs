using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;

namespace PiRelayPlate.NetCore
{
    public static class RelayPlate
    {
        private const string LibFileName = "libRelayPlate.so";

        static RelayPlate()
        {
            // Ensure the file has been created / updated in the directory
            var executingDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            var libPath = Path.Combine(executingDirectory, LibFileName);

            if (File.Exists(libPath))
            {
                // attempt to remove the file first
                try
                {
                    File.Delete(libPath);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Could not remove file {LibFileName}. Error: {ex.Message}");
                }
            }

            // Attempt to write the current resource file
            try
            {
                File.WriteAllBytes(libPath, Properties.Resources.libRelayPlate);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Could not add file {LibFileName}. Error: {ex.Message}");
            }
        }


        // ToDo: fix mangled name on c export
        [DllImport(LibFileName, EntryPoint = "_Z11SetPinStatehhh", CallingConvention = CallingConvention.StdCall, PreserveSig = true), SuppressUnmanagedCodeSecurity]
        public static extern int SetPinState(byte boardId, byte pin, byte state);
    }
}
