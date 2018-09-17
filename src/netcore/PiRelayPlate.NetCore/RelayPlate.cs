using System;
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
            if (_platesAvailable == -1)
            {
                _platesAvailable = RelaysAvailable();
            }

            if (_platesAvailable == 0)
            {
                Console.WriteLine("No PiPlates Available");
                return false;
            }

            if (boardId > _platesAvailable)
            {
                Console.WriteLine($"Request for Board {boardId} failed. Only {_platesAvailable} available");
                return false;
            }

            if (pin > 7)
            {
                Console.WriteLine($"Pin requested {pin} is higher than 7");
            }

            return SetPinStateImpl(boardId, pin, state) == 0;
        }

        public static int RelaysAvailable()
        {
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
