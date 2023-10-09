using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
using PiRelayPlate.NetCore.Resources;

namespace PiRelayPlate.NetCore
{
    public static class RelayPlate
    {
        private const string LibFileName = "libRelayPlate";

        private static int _platesAvailable = -1;

        static RelayPlate()
        {
            try
            {
                EmbeddedResources.ExtractAll();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error extracting required resources. Error: {ex.Message}");
            }
        }

        public const byte MaxPinsPerRelayBoard = 7;

        /// <summary>
        /// Its important that we don't call this method while its currently executing another
        /// command. Because of this, we have made this a synchronized call. 
        /// If there are other API calls that need to be made, a locking component will 
        /// be required to synchronize all calls. 
        /// </summary>
        /// <param name="pin"></param>
        /// <param name="state"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.Synchronized)]
        public static bool SetPinState(byte pin, byte state)
        {
            var boardId = pin / MaxPinsPerRelayBoard;
            var correctPin = pin % MaxPinsPerRelayBoard;

            // Because the modulus of correctPin may be 0 (indicating a perfect)
            // division, we know we are on the last pin of the desired board.
            // therefor we will set the pin to the last pin ID,
            // and de-increment the boardId.
            if (correctPin == 0)
            {
                correctPin = MaxPinsPerRelayBoard;
                boardId--;
            }

            return SetPinState((byte)boardId, (byte)correctPin, state);
        }

        public static bool SetPinState(byte boardId, byte pin, byte state)
        {
            // before calling the native code, do a quick check to see if there are files in /dev
            // that control SPI - if not, the native code may not exit correctly causing 
            // an app crash
            if (Directory.GetFiles("/dev", "spidev*").Length == 0)
            {
                Console.WriteLine("No SPI Devices in /dev!");
                _platesAvailable = 0;
                return false;
            }
            
            // this will init the Relays (if there are any attached)
            if (_platesAvailable == -1)
            {
                _platesAvailable = RelaysAvailable();
            }

            // If there are no relays detected, return false
            if (_platesAvailable == 0)
            {
                Console.WriteLine("No PiPlates Available");
                return false;
            }

            // validate the board being requested to ensure it exists before we can send a command to it.
            if (boardId > _platesAvailable)
            {
                Console.WriteLine($"Request for Board {boardId} failed. Only {_platesAvailable} available");
                return false;
            }

            // Ensure that we don't send pins outside 1-7
            if (pin > 7 || pin == 0)
            {
                Console.WriteLine($"Pin requested {pin} is higher than 7");
                return false;
            }

            return SetPinStateImpl(boardId, pin, state) == 0;
        }

        public static int RelaysAvailable()
        {
            // before calling the native code, do a quick check to see if there are files in /dev
            // that control SPI - if not, the native code may not exit correctly causing 
            // an app crash
            if (Directory.GetFiles("/dev", "spidev*").Length == 0)
            {
                Console.WriteLine("No SPI Devices in /dev!");
                return 0;
            }

            _platesAvailable = RelaysAvailableImpl();
            return _platesAvailable;
        }
    
        // TODO: fix mangled name on c export
        [DllImport(LibFileName, EntryPoint = "_Z11SetPinStatehhh", CallingConvention = CallingConvention.StdCall, PreserveSig = true), SuppressUnmanagedCodeSecurity]
        private static extern int SetPinStateImpl(byte boardId, byte pin, byte state);

        [DllImport(LibFileName, EntryPoint = "_Z15RelaysAvailablev", CallingConvention = CallingConvention.StdCall, PreserveSig = true), SuppressUnmanagedCodeSecurity]
        private static extern int RelaysAvailableImpl();
    }
}
